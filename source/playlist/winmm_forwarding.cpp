// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
// Resolve the OS module once through GetSystemDirectoryW: production bootstrap
// preflights on its worker before mod callbacks; an ordinary cold export may
// also initialize forwarding. DllMain itself does not resolve exports.
// ABI-preserving x64 tail stubs retain stack arguments.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include "winmm_exports.hpp"
extern "C" {void* winmm_targets[181]{};}
#ifdef WINMM_FORWARD_SELFTEST
// Own-process fixture only; not defined in either production DLL build.
extern "C" {void* winmm_test_target{};unsigned winmm_test_seen=UINT32_MAX;unsigned winmm_test_fault{};}
#endif
namespace {
INIT_ONCE once=INIT_ONCE_STATIC_INIT;
unsigned failure_reason{};DWORD failure_error{};bool complete{};
BOOL CALLBACK initialize(PINIT_ONCE,void*,void**) noexcept {
    DWORD saved=GetLastError();wchar_t path[32768]{};auto n=GetSystemDirectoryW(path,32768);
#ifdef WINMM_FORWARD_SELFTEST
    if(winmm_test_fault==1){n=0;SetLastError(ERROR_INSUFFICIENT_BUFFER);}
#endif
    // Initialize once even on failure: do not repeatedly load from export calls.
    if(!n||n>=32757){failure_reason=1;failure_error=GetLastError();return TRUE;}
    wcscat(path,L"\\winmm.dll");
    // Explicit absolute OS path plus restricted dependency search prevents
    // recursive local-proxy loading and current-directory dependency hijacking.
    auto system=LoadLibraryExW(path,nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
#ifdef WINMM_FORWARD_SELFTEST
    if(winmm_test_fault==2){if(system)FreeLibrary(system);system=nullptr;SetLastError(ERROR_MOD_NOT_FOUND);}
#endif
    if(!system){failure_reason=2;failure_error=GetLastError();return TRUE;}
    void* targets[181]{};
    complete=true;
    for(unsigned i=0;i<181;++i){auto address=GetProcAddress(system,winmm_names[i]?winmm_names[i]:MAKEINTRESOURCEA(winmm_ordinals[i]));
#ifdef WINMM_FORWARD_SELFTEST
        if((winmm_test_fault==3&&i==1)||(winmm_test_fault==4&&!winmm_names[i])){address=nullptr;SetLastError(ERROR_PROC_NOT_FOUND);}
#endif
        if(!address){complete=false;failure_reason=3;failure_error=ERROR_PROC_NOT_FOUND;}targets[i]=reinterpret_cast<void*>(address);}
    // Deliberately retain system module for process lifetime. No detach/free.
    for(unsigned i=0;i<181;++i)InterlockedExchangePointer(&winmm_targets[i],targets[i]);
    SetLastError(saved);return TRUE;
}
BOOL initialize_once() noexcept {
#ifdef WINMM_FORWARD_SELFTEST
    if(winmm_test_fault==5){SetLastError(ERROR_GEN_FAILURE);return FALSE;}
#endif
    return InitOnceExecuteOnce(&once,initialize,nullptr,nullptr);
}
[[noreturn]] void unavailable(unsigned reason,DWORD error,unsigned index) noexcept {
#ifdef WINMM_FORWARD_SELFTEST
    // Evidence pipe owned by the disposable fixture; no diagnostic I/O in DLL.
    char line[128]{};unsigned published=0;for(auto target:winmm_targets)if(target)++published;
    auto length=snprintf(line,sizeof(line),"FAILURE reason=%u error=%lu index=%u published=%u\n",reason,error,index,published);
    DWORD written{};WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),line,DWORD(length),&written,nullptr);
#endif
    // Custom exception identifies forwarding failure, rather than reporting
    // Windows' stack-corruption status for a missing audio entry point.
    EXCEPTION_RECORD record{};record.ExceptionCode=0xE0534D01;record.ExceptionFlags=EXCEPTION_NONCONTINUABLE;
    record.NumberParameters=3;record.ExceptionInformation[0]=reason;record.ExceptionInformation[1]=error;record.ExceptionInformation[2]=index;
#ifdef WINMM_FORWARD_SELFTEST
    if(winmm_test_fault!=6)
#endif
    RaiseFailFastException(&record,nullptr,0);
    TerminateProcess(GetCurrentProcess(),127); // fallback if OS unexpectedly returns
    for(;;){} // never return a null jump target
}
}
// Worker preflight is nonfatal. A partially available OS surface is usable for
// ordinary forwarding, but cannot enable the mod's real-time native callbacks.
extern "C" bool InitializeWinmm() noexcept {
    DWORD saved=GetLastError();auto ok=initialize_once();
    auto ready=ok&&complete;SetLastError(saved);return ready;
}
extern "C" void* ResolveWinmm(unsigned index) noexcept {
    DWORD saved=GetLastError();
#ifdef WINMM_FORWARD_SELFTEST
    if(winmm_test_target&&index<181){winmm_test_seen=index;InterlockedExchangePointer(&winmm_targets[index],winmm_test_target);SetLastError(saved);return winmm_test_target;}
#endif
    if(index>=181)unavailable(4,ERROR_INVALID_PARAMETER,index);
    if(!initialize_once())unavailable(5,GetLastError(),index);
    auto target=winmm_targets[index];
    if(!target)unavailable(failure_reason,failure_error,index);
    SetLastError(saved);return target;
}
