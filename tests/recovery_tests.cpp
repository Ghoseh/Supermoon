// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "playlist/compatibility.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <array>
using namespace bytebeat::compatibility;
namespace bytebeat::compatibility {extern bool compatibility_test_owner_denied;}
void check(bool v,const char* m){if(!v)throw std::runtime_error(m);}
void launch(const std::filesystem::path& root,const wchar_t* mode){wchar_t exe[32768]{};check(GetModuleFileNameW(nullptr,exe,32768)>0,"Self path");auto command=L"\""+std::wstring(exe)+L"\" "+mode+L" \""+root.wstring()+L"\"";STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};check(CreateProcessW(exe,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Recovery child");CloseHandle(process.hThread);auto wait=WaitForSingleObject(process.hProcess,10000);if(wait!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,99);WaitForSingleObject(process.hProcess,10000);}DWORD code{};GetExitCodeProcess(process.hProcess,&code);CloseHandle(process.hProcess);check(wait==WAIT_OBJECT_0,"Recovery timeout");check(code==(!wcscmp(mode,L"--terminate")||!wcscmp(mode,L"--attempt-terminate")?72u:0u),"Child exit status");}
int wmain(int argc,wchar_t** argv){try{
    auto hash=std::string(64,'a');
    if(argc==3){Store s(argv[2]);
        if(!wcscmp(argv[1],L"--attempt-live-check")){check(s.previous_attempt()==AttemptState::live,"Other live process owner");bool blocked=false;try{s.begin_attempt(hash);}catch(...){blocked=true;}check(blocked,"Other process duplicate refused");return 0;}
        if(std::wstring(argv[1]).rfind(L"--attempt-",0)==0){s.begin_attempt(hash);
            if(!wcscmp(argv[1],L"--attempt-terminate"))TerminateProcess(GetCurrentProcess(),72);
            if(!wcscmp(argv[1],L"--attempt-reject"))s.record_activation_return(2);
            if(!wcscmp(argv[1],L"--attempt-partial"))s.record_activation_return(10);
            if(!wcscmp(argv[1],L"--attempt-success"))s.record_activation_return(0);
            return 0; // no clean token or destructor clearing
        }
        s.mark_active(hash);if(!wcscmp(argv[1],L"--terminate"))TerminateProcess(GetCurrentProcess(),72);
        // Child simulates an explicit audited orderly token. The production
        // native integration has no such proven signal and NEVER calls this.
        if(!wcscmp(argv[1],L"--orderly"))s.mark_clean();return 0;}
    auto root=std::filesystem::path("fixtures/recovery")/std::to_string(GetCurrentProcessId())/std::to_string(GetTickCount64());
    launch(root,L"--terminate");{Store s(root);check(s.unexpected_previous_active(),"Terminated child persists unresolved marker");s.mark_clean();}
    launch(root,L"--orderly");{Store s(root);check(!s.unexpected_previous_active(),"Explicit orderly-token storage");}
    for(auto bytes:{"","SPACTIVE", "SPACTIVEinvalid\n"}){{std::ofstream f(root/"active.txt",std::ios::binary);f<<bytes;}bool refused=false;try{Store s(root);s.unexpected_previous_active();}catch(...){refused=true;}check(refused,"Zero/truncated/corrupt active marker refuses");}
    {Store s(root);s.mark_active(hash);s.mark_clean();s.remember(hash,Decision::no);}
    {std::ofstream f(root/"decisions.txt",std::ios::binary);}bool truncated=false;try{Store s(root);}catch(...){truncated=true;}check(truncated,"Zero-byte decisions refuse");
    {std::ofstream f(root/"decisions.txt",std::ios::binary);f<<"SPDEC1\n";}
    for(unsigned i=0;i<100;++i){Store s(root);check(!s.unexpected_previous_active(),"Repeat clean initialization");s.mark_active(hash);check(s.unexpected_previous_active(),"Repeated active publication");s.mark_clean();}
    {std::ofstream f(root/"active.txt.pending",std::ios::binary);f<<"interrupted";}{Store s(root);s.mark_active(hash);check(s.unexpected_previous_active(),"Interrupted pending overwrite recovery");s.mark_clean();}
    auto journal=root/"attempts";
    for(auto mode:{L"--attempt-terminate",L"--attempt-returnless",L"--attempt-reject",L"--attempt-partial",L"--attempt-success"}){
        launch(journal,mode);{Store s(journal);check(s.previous_attempt()==AttemptState::exited,"Exited owner permits fresh process without clean verdict");}
        std::ifstream f(journal/"attempt.txt",std::ios::binary);std::string bytes{std::istreambuf_iterator<char>(f),{}};
        auto end=!wcscmp(mode,L"--attempt-reject")?" 1 2\n":!wcscmp(mode,L"--attempt-partial")?" 1 10\n":!wcscmp(mode,L"--attempt-success")?" 1 0\n":" 0 0\n";
        check(bytes.size()>=strlen(end)&&bytes.substr(bytes.size()-strlen(end))==end,"Persist exact attempted/returned status");
    }
    {Store s(journal);s.mark_active(hash);check(s.unexpected_previous_active(),"Legacy unresolved marker retained advisory");
        s.begin_attempt(hash);check(s.previous_attempt()==AttemptState::live,"Current live PID+creation owner");
        bool duplicate=false;try{s.begin_attempt(hash);}catch(...){duplicate=true;}check(duplicate,"Same-process duplicate attempt refused");
        compatibility_test_owner_denied=true;bool unknown=false;try{s.previous_attempt();}catch(...){unknown=true;}compatibility_test_owner_denied=false;check(unknown,"Unqueryable owner fails closed");
        HANDLE held=CreateFileW((journal/"attempt.txt.pending").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);check(held!=INVALID_HANDLE_VALUE,"Return-write sharing fixture");
        bool failed=false;try{s.record_activation_return(10);}catch(...){failed=true;}CloseHandle(held);check(failed&&s.previous_attempt()==AttemptState::live,"Failed return publication retains attempt identity");
        s.record_activation_return(10);bool repeated=false;try{s.record_activation_return(0);}catch(...){repeated=true;}check(repeated,"Activation return cannot be overwritten");}
    launch(journal,L"--attempt-live-check");
    {Store s(root/"before-call-failure");HANDLE held=CreateFileW((root/"before-call-failure/attempt.txt.pending").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);check(held!=INVALID_HANDLE_VALUE,"Attempt publication sharing fixture");
        bool failed=false;try{s.begin_attempt(hash);}catch(...){failed=true;}CloseHandle(held);check(failed&&s.previous_attempt()==AttemptState::none,"Failed begin cannot authorize or leave published attempt");}
    {FILETIME c{},e{},k{},u{};check(GetProcessTimes(GetCurrentProcess(),&c,&e,&k,&u)!=0,"PID reuse fixture identity");auto created=(uint64_t(c.dwHighDateTime)<<32)|c.dwLowDateTime;
        std::ofstream f(journal/"attempt.txt",std::ios::binary);f<<"SPATTEMPT2\n"<<hash<<'\n'<<GetCurrentProcessId()<<' '<<created+1<<" 0 0\n";f.close();Store s(journal);check(s.previous_attempt()==AttemptState::exited,"Simulated PID reuse does not match creation time");}
    for(auto bytes:std::array<std::string,4>{"","SPATTEMPT2\n", "SPATTEMPT2\ninvalid\n", "SPATTEMPT2\n"+hash+"\n0 1 0 0\n"}){
        std::ofstream f(journal/"attempt.txt",std::ios::binary);f<<bytes;f.close();bool bad=false;try{Store s(journal);s.previous_attempt();}catch(...){bad=true;}check(bad,"Malformed attempt state refuses");}
    std::cout<<"PASS recovery: actual child termination AND normal exit without clean token; activation rejection/partial/success/returnless exits permit next process; legacy advisory; live duplicate and unqueryable owner refusal; PID reuse identity; failed return writes; corrupt state; 100 initializations. Native clean exit still unproven\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
