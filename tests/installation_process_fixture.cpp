// SPDX-License-Identifier: MIT
// Owned offline process fixture only. Never contains or runs NMS instructions.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cwchar>
int main(){
    wchar_t path[32768]{};GetModuleFileNameW(nullptr,path,32768);
    auto name=wcsrchr(path,L'\\');name=name?name+1:path;
    if(_wcsicmp(name,L"tasklist.exe")==0){
        auto h=CreateFileW(L"HIJACK_EXECUTED",GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);return 0;
    }
    std::puts("OWNED PROCESS FIXTURE READY; no game code");std::fflush(stdout);
    Sleep(60000);return 0;
}
