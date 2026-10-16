// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "keyboard_settings.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace bytebeat::playlist {
namespace {
void need(bool value,const char* error){if(!value)throw std::runtime_error(error);}
void regular(const std::filesystem::path& path){auto a=GetFileAttributesW(path.c_str());need(a==INVALID_FILE_ATTRIBUTES||!(a&FILE_ATTRIBUTE_REPARSE_POINT),"Shuffle state/config reparse point refused");}
uint32_t checksum(const std::string& text){uint32_t result=2166136261u;for(unsigned char c:text){result^=c;result*=16777619u;}return result;}
std::string record(bool value){std::string text=value?"SupermoonShuffle1\n1\n":"SupermoonShuffle1\n0\n";char tail[16]{};std::snprintf(tail,sizeof(tail),"%08x\n",checksum(text));return text+tail;}
Shortcut binding(std::wstring value){
    value.erase(std::remove_if(value.begin(),value.end(),[](wchar_t c){return c==L' '||c==L'\t';}),value.end());
    std::transform(value.begin(),value.end(),value.begin(),[](wchar_t c){return wchar_t(towupper(c));});
    if(value==L"NONE")return {0,3};
    const std::wstring prefix=L"CTRL+SHIFT+F";need(value.rfind(prefix,0)==0,"Binding requires Ctrl+Shift+F1..F24 or None");
    auto suffix=value.substr(prefix.size());need(!suffix.empty()&&suffix.size()<=2,"Invalid shortcut function key");
    unsigned n=0;for(auto c:suffix){need(c>=L'0'&&c<=L'9',"Invalid shortcut function key");n=n*10+unsigned(c-L'0');}
    need(n>=1&&n<=24&&n!=12,"Unsafe shortcut key: F12 reserved for debugger/Steam screenshot; choose another key");
    return {0x70+n-1,3};
}
}
KeyboardConfiguration keyboard_configuration(const std::filesystem::path& mod){
    KeyboardConfiguration result;auto path=mod/"playlist.ini";regular(path);wchar_t value[128]{};
    GetPrivateProfileStringW(L"Keyboard",L"Enabled",L"1",value,128,path.c_str());
    need(!wcscmp(value,L"0")||!wcscmp(value,L"1"),"Keyboard Enabled must be 0 or 1");result.enabled=value[0]==L'1';
    const wchar_t* keys[]={L"Previous",L"Next",L"ToggleShuffle",L"ShuffleAndAdvance"};
    const wchar_t* defaults[]={L"Ctrl+Shift+F6",L"Ctrl+Shift+F7",L"Ctrl+Shift+F8",L"Ctrl+Shift+F9"};
    for(unsigned i=0;i<4;++i){auto n=GetPrivateProfileStringW(L"Keyboard",keys[i],defaults[i],value,128,path.c_str());need(n<127,"Shortcut value too long");result.bindings[i]=binding(value);}
    for(unsigned i=0;i<4;++i)for(unsigned j=i+1;j<4;++j)need(!result.bindings[i].key||result.bindings[i].key!=result.bindings[j].key,"Duplicate keyboard shortcuts refused; no automatic substitution");
    return result;
}
ShuffleStore::ShuffleStore(const std::filesystem::path& state):path_(state/"shuffle-mode.txt"){
    try{
        regular(state);std::filesystem::create_directories(state);regular(path_);auto lock=state/"shuffle-mode.lock";regular(lock);
        auto h=CreateFileW(lock.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        need(h!=INVALID_HANDLE_VALUE,"Shuffle preference is inaccessible or owned by another process");lock_=h;
        if(std::filesystem::exists(path_)){
            need(std::filesystem::file_size(path_)<=64,"Corrupted shuffle preference size");std::ifstream file(path_,std::ios::binary);need(bool(file),"Shuffle preference read failed");
            std::string data((std::istreambuf_iterator<char>(file)),{});need(data==record(false)||data==record(true),"Corrupted shuffle preference; retained without overwrite");mode_=data==record(true);
        }
        available_=true;
    }catch(const std::exception& e){error_=e.what();}
}
ShuffleStore::~ShuffleStore(){if(lock_)CloseHandle(lock_);}
bool ShuffleStore::save(bool value) noexcept {
    if(!available_)return false;
    HANDLE file=INVALID_HANDLE_VALUE;
    try{
        auto pending=path_;pending+=L".pending";regular(pending);regular(path_);
        auto data=record(value);file=CreateFileW(pending.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);need(file!=INVALID_HANDLE_VALUE,"Shuffle preference write failed");
        DWORD count{};bool ok=WriteFile(file,data.data(),DWORD(data.size()),&count,nullptr)&&count==data.size()&&FlushFileBuffers(file);CloseHandle(file);file=INVALID_HANDLE_VALUE;need(ok,"Shuffle preference durable flush failed");
        need(MoveFileExW(pending.c_str(),path_.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Shuffle preference atomic publication failed");mode_=value;error_.clear();return true;
    }catch(const std::exception& e){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);error_=e.what();return false;}
}
}
