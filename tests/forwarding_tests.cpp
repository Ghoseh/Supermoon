// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <filesystem>
#include "winmm_exports.hpp"
extern "C" void* ResolveWinmm(unsigned) noexcept;
extern "C" void* winmm_targets[181];
extern "C" void* winmm_test_target;
extern "C" unsigned winmm_test_seen;
extern "C" unsigned winmm_test_fault;
extern "C" bool InitializeWinmm() noexcept;
void check(bool v,const char* why){if(!v)throw std::runtime_error(why);}
template<class Fn> Fn fn(void (*p)()){return reinterpret_cast<Fn>(p);}
template<class Fn> Fn resolve(HMODULE module,const char* name){auto p=GetProcAddress(module,name);Fn f{};memcpy(&f,&p,sizeof(f));return f;}
// Test every tail stub's exact ABI independently of OS side effects. Replace
// resolved targets only in this own-process executable, never in the DLL.
extern "C" uint64_t probe(uint64_t a,double b,uint64_t c,double d,uint64_t e,double f,uint64_t g,double h){return a+uint64_t(b)*3+c*5+uint64_t(d)*7+e*11+uint64_t(f)*13+g*17+uint64_t(h)*19;}
void failure_child(unsigned fault,unsigned index){
    char exe[32768]{};check(GetModuleFileNameA(nullptr,exe,32768)>0,"Failure self path");
    SECURITY_ATTRIBUTES security{sizeof(security),nullptr,TRUE};HANDLE input{},output{};
    check(CreatePipe(&input,&output,&security,0)!=0,"Failure evidence pipe");
    check(SetHandleInformation(input,HANDLE_FLAG_INHERIT,0)!=0,"Pipe inheritance");
    std::string command="\""+std::string(exe)+"\" --failure "+std::to_string(fault)+" "+std::to_string(index);
    STARTUPINFOA startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESTDHANDLES;startup.hStdOutput=output;startup.hStdError=output;startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION p{};check(CreateProcessA(exe,command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&p)!=0,"Failure child creation");
    CloseHandle(output);CloseHandle(p.hThread);
    auto wait=WaitForSingleObject(p.hProcess,10000);if(wait!=WAIT_OBJECT_0){TerminateProcess(p.hProcess,99);WaitForSingleObject(p.hProcess,10000);}
    DWORD code{};check(GetExitCodeProcess(p.hProcess,&code)!=0,"Failure child exit query");CloseHandle(p.hProcess);
    char buffer[2048]{};DWORD bytes{};ReadFile(input,buffer,sizeof(buffer)-1,&bytes,nullptr);CloseHandle(input);
    check(wait==WAIT_OBJECT_0&&code==(fault==6?127u:0xE0534D01u),"Own-child fail-fast status or termination fallback");
    unsigned reason=fault==0||fault==6?4:fault==5?5:fault<3?fault:3;
    DWORD error=fault==0||fault==6?ERROR_INVALID_PARAMETER:fault==5?ERROR_GEN_FAILURE:fault==1?ERROR_INSUFFICIENT_BUFFER:fault==2?ERROR_MOD_NOT_FOUND:ERROR_PROC_NOT_FOUND;
    auto expected="FAILURE reason="+std::to_string(reason)+" error="+std::to_string(error)+" index="+std::to_string(index)+" published="+std::to_string(fault==3||fault==4?180:0);
    check(std::string(buffer,bytes).find(expected)!=std::string::npos,"Failure reason/error/index/partial publication evidence");
}
int main(int argc,char** argv){try{
    if(argc==4&&std::string(argv[1])=="--failure"){
        SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);winmm_test_fault=unsigned(std::stoul(argv[2]));auto index=unsigned(std::stoul(argv[3]));
        if(winmm_test_fault&&winmm_test_fault<6){SetLastError(0x9876);check(!InitializeWinmm()&&GetLastError()==0x9876,"Worker failure is nonfatal and preserves LastError");
            check(!InitializeWinmm(),"Repeat failed preflight remains nonfatal");
            if(winmm_test_fault==3||winmm_test_fault==4){unsigned available=index==0?1:0;check(ResolveWinmm(available)!=nullptr,"Available exports remain usable after incomplete preflight");
                unsigned clock=0;while(clock<181&&(!winmm_names[clock]||strcmp(winmm_names[clock],"timeGetTime")))++clock;check(clock<181,"Available clock fixture");
                using Clock=DWORD(WINAPI*)();check(uint32_t(GetTickCount()-fn<Clock>(winmm_stubs[clock])())<1000,"Real available export forwards after incomplete preflight");}
        }
        if(index<181)winmm_stubs[index]();else ResolveWinmm(index);
        return 80; // failure path must never return here
    }
    unsigned unnamed=0;while(unnamed<181&&winmm_names[unnamed])++unnamed;check(unnamed<181,"Unnamed ordinal fixture");
    check(winmm_names[1]!=nullptr,"Named failure fixture");
    failure_child(1,0);failure_child(2,0);failure_child(3,1);failure_child(4,unnamed);failure_child(0,181);
    failure_child(5,0);failure_child(6,181);
    std::atomic<bool> failed{false};std::vector<std::thread> threads;
    for(unsigned n=0;n<16;++n)threads.emplace_back([&]{for(unsigned i=0;i<181;++i){SetLastError(0x1234);if(!ResolveWinmm(i)||GetLastError()!=0x1234)failed=true;}});
    for(auto& t:threads)t.join();check(!failed,"Concurrent lazy resolution/LastError");
    SetLastError(0x3456);check(InitializeWinmm()&&GetLastError()==0x3456,"Complete worker preflight positive contract");
    wchar_t path[32768]{};auto n=GetSystemDirectoryW(path,32768);check(n&&n<32757,"Dynamic OS root");wcscat(path,L"\\winmm.dll");auto native=LoadLibraryExW(path,nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);check(native,"System WINMM");
    for(unsigned i=0;i<181;++i)check(ResolveWinmm(i)==reinterpret_cast<void*>(GetProcAddress(native,winmm_names[i]?winmm_names[i]:MAKEINTRESOURCEA(winmm_ordinals[i]))),"181 exact OS export targets");
    using Probe=uint64_t(*)(uint64_t,double,uint64_t,double,uint64_t,double,uint64_t,double);
    auto expected=probe(1,2,3,4,5,6,7,8);
    for(unsigned i=0;i<181;++i){InterlockedExchangePointer(&winmm_targets[i],reinterpret_cast<void*>(&probe));check(fn<Probe>(winmm_stubs[i])(1,2,3,4,5,6,7,8)==expected,"Export tail-call GPR/XMM/stack ABI");}
    winmm_test_target=reinterpret_cast<void*>(&probe);
    for(unsigned i=0;i<181;++i){winmm_test_seen=UINT32_MAX;InterlockedExchangePointer(&winmm_targets[i],nullptr);SetLastError(0x2468);
        check(fn<Probe>(winmm_stubs[i])(1,2,3,4,5,6,7,8)==expected&&winmm_test_seen==i&&GetLastError()==0x2468,"Every slow bridge index/GPR/XMM/stack/LastError");}
    winmm_test_target=nullptr;
    if(argc==2&&std::string(argv[1])=="--without-proxy"){
        std::cout<<"PASS WINMM own-process subset: seven failure children, 181 OS targets, hot/slow ABI stubs and concurrent resolvers; separate proxy DLL load intentionally skipped\n";return 0;
    }
    // A fresh DLL exercises the slow resolver bridge with real stack arguments.
    wchar_t self[32768]{};check(GetModuleFileNameW(nullptr,self,32768)>0,"Own forwarding fixture path");
    auto proxy_path=std::filesystem::path(self).parent_path()/"winmm.dll";auto proxy=LoadLibraryW(proxy_path.c_str());check(proxy,"Proxy wrong-host load");
    using Wave=unsigned(WINAPI*)(void**,unsigned,const void*,uintptr_t,uintptr_t,unsigned);
    auto wave=resolve<Wave>(proxy,"waveOutOpen");auto original=resolve<Wave>(native,"waveOutOpen");
    check(wave&&wave(nullptr,0,nullptr,0,0,0)==original(nullptr,0,nullptr,0,0,0),"Cold proxy six-argument forwarding");
    using Time=DWORD(WINAPI*)();auto clock=resolve<Time>(proxy,"timeGetTime");check(clock&&uint32_t(GetTickCount()-clock())<1000,"Hot proxy clock");
    using Error=unsigned(WINAPI*)(unsigned,wchar_t*,unsigned);auto error=resolve<Error>(proxy,"waveOutGetErrorTextW");wchar_t a[256]{},b[256]{};auto os_error=resolve<Error>(native,"waveOutGetErrorTextW");check(error(0,a,256)==os_error(0,b,256)&&!wcscmp(a,b),"Output buffer forwarding");
    std::cout<<"PASS portable WINMM: 7 disposable failure children (path/load/named/ordinal/index/InitOnce/fallback), nonfatal/repeated and positive preflight, real partial available forwarding, exact fail-fast status/reason and termination fallback; all 181 dynamic system targets; 181 hot AND slow ABI tail stubs (index/GPR/XMM/stack/LastError), 16 concurrent resolvers, real proxy exports; wrong host inert\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
