// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "compatibility.hpp"
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace bytebeat::compatibility {
#ifdef COMPATIBILITY_SELFTEST
bool compatibility_test_owner_denied{};
#endif
namespace {
void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
bool hash(const std::string& s){return s.size()==64&&s.find_first_not_of("0123456789abcdef")==std::string::npos;}
void atomic_write(const std::filesystem::path& p,const std::string& bytes){
    auto temp=p;temp+=L".pending";
    HANDLE f=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    require(f!=INVALID_HANDLE_VALUE,"Cannot write compatibility state");DWORD n{};
    bool ok=WriteFile(f,bytes.data(),DWORD(bytes.size()),&n,nullptr)&&n==bytes.size()&&FlushFileBuffers(f);CloseHandle(f);
    require(ok,"Compatibility state flush failed");
    require(MoveFileExW(temp.c_str(),p.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Compatibility state publication failed");
}
std::string read(const std::filesystem::path& p){if(!std::filesystem::exists(p))return {};
    require(std::filesystem::file_size(p)<=32768,"Oversized compatibility state");std::ifstream f(p,std::ios::binary);
    require(bool(f),"Cannot read compatibility state");return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
struct Attempt {std::string build;DWORD pid{};uint64_t created{};unsigned phase{},result{};};
uint64_t creation(HANDLE process){FILETIME c{},e{},k{},u{};require(GetProcessTimes(process,&c,&e,&k,&u)!=0,"Cannot query recovery owner identity");return (uint64_t(c.dwHighDateTime)<<32)|c.dwLowDateTime;}
std::string serialize(const Attempt& a){return "SPATTEMPT2\n"+a.build+"\n"+std::to_string(a.pid)+" "+std::to_string(a.created)+" "+std::to_string(a.phase)+" "+std::to_string(a.result)+"\n";}
Attempt attempt(const std::filesystem::path& p){auto bytes=read(p);std::istringstream in(bytes);std::string magic,extra;Attempt a;
    in>>magic>>a.build>>a.pid>>a.created>>a.phase>>a.result;
    require(bool(in)&&!(in>>extra)&&magic=="SPATTEMPT2"&&hash(a.build)&&a.pid&&a.created&&a.phase<=1&&(a.phase||!a.result)&&bytes==serialize(a),"Malformed activation-attempt journal");return a;}
}
Result evaluate(Inputs i,Decision d) noexcept {
    if(d==Decision::no)return Result::disabled;
    if(i.unexpected_previous_active&&!i.recovery_allowed)return Result::recovery_declined;
    if(!i.native_profile_available||!i.sites_valid)return Result::validation_failed;
    return Result::activate;
}
Store::Store(const std::filesystem::path& directory):directory_(directory){
    std::filesystem::create_directories(directory_);
    HANDLE lock=CreateFileW((directory_/"session.lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    require(lock!=INVALID_HANDLE_VALUE,"Compatibility store already in use/unavailable");lock_=lock;
    try{
        auto bytes=read(directory_/"decisions.txt");if(bytes.empty()){require(!std::filesystem::exists(directory_/"decisions.txt"),"Truncated compatibility state");return;}
        require(bytes.rfind("SPDEC1\n",0)==0,"Unknown compatibility state format");
        for(size_t pos=7;pos<bytes.size();){require(decisions_.size()<256&&bytes.size()-pos>=67,"Malformed compatibility decision");auto key=bytes.substr(pos,64);char value=bytes[pos+65];
            require(hash(key)&&bytes[pos+64]==' '&&(value=='Y'||value=='N')&&bytes[pos+66]=='\n'&&!decisions_.count(key),"Malformed/duplicate compatibility decision");decisions_[key]=value=='Y'?Decision::yes:Decision::no;pos+=67;}
    }catch(...){CloseHandle(lock);lock_=nullptr;throw;}
}
Store::~Store(){if(lock_)CloseHandle(static_cast<HANDLE>(lock_));} // No implied clean shutdown.
bool Store::known(const std::string& key) const {return hash(key)&&decisions_.count(key);}
Decision Store::get(const std::string& key) const {return decisions_.at(key);}
void Store::remember(const std::string& key,Decision value){require(hash(key),"Invalid executable SHA256");auto next=decisions_;next[key]=value;require(next.size()<=256,"Compatibility decision capacity reached; reset required");
    std::string bytes="SPDEC1\n";for(auto& item:next)bytes+=item.first+" "+(item.second==Decision::yes?"Y\n":"N\n");atomic_write(directory_/"decisions.txt",bytes);decisions_=std::move(next);}
void Store::reset(){atomic_write(directory_/"decisions.txt","SPDEC1\n");decisions_.clear();}
bool Store::unexpected_previous_active() const {auto path=directory_/"active.txt";if(!std::filesystem::exists(path))return false;auto bytes=read(path);require(bytes.size()==73&&bytes.rfind("SPACTIVE",0)==0&&hash(bytes.substr(8,64))&&bytes[72]=='\n',"Malformed active-session marker");return true;}
void Store::mark_active(const std::string& key){require(hash(key),"Invalid active build");atomic_write(directory_/"active.txt","SPACTIVE"+key+"\n");}
void Store::mark_clean(){std::error_code error;std::filesystem::remove(directory_/"active.txt",error);require(!error,"Cannot clear active-session marker");}
AttemptState Store::previous_attempt() const {
    auto path=directory_/"attempt.txt";if(!std::filesystem::exists(path))return AttemptState::none;
    auto a=attempt(path);
#ifdef COMPATIBILITY_SELFTEST
    require(!compatibility_test_owner_denied,"Injected inaccessible attempt owner");
#endif
    HANDLE owner=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,a.pid);
    if(!owner){require(GetLastError()==ERROR_INVALID_PARAMETER,"Cannot determine activation-attempt owner lifetime");return AttemptState::exited;}
    uint64_t created{};try{created=creation(owner);}catch(...){CloseHandle(owner);throw;}
    auto wait=WaitForSingleObject(owner,0);CloseHandle(owner);
    require(wait==WAIT_OBJECT_0||wait==WAIT_TIMEOUT,"Cannot determine activation-attempt owner state");
    return created!=a.created||wait==WAIT_OBJECT_0?AttemptState::exited:AttemptState::live;
}
void Store::begin_attempt(const std::string& build){
    require(hash(build),"Invalid attempt build");require(previous_attempt()!=AttemptState::live,"Activation attempt already owned by a live process");
    atomic_write(directory_/"attempt.txt",serialize({build,GetCurrentProcessId(),creation(GetCurrentProcess()),0,0}));
}
void Store::record_activation_return(unsigned result){auto a=attempt(directory_/"attempt.txt");
    require(a.pid==GetCurrentProcessId()&&a.created==creation(GetCurrentProcess())&&a.phase==0,"Activation return does not own current attempt");
    a.phase=1;a.result=result;atomic_write(directory_/"attempt.txt",serialize(a));
}
}
