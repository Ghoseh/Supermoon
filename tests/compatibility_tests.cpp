// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "playlist/compatibility.hpp"
#include "playlist/compatibility_dialogs.hpp" // Compile UI helper; never show a dialog offline.
#include "playlist/bootstrap_compatibility.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace bytebeat::compatibility;
void check(bool v,const char* m){if(!v)throw std::runtime_error(m);}
int main(){try{
    auto root=std::filesystem::path("fixtures/compatibility-fixtures")/std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto a=std::string(64,'a'),b=std::string(64,'b');
    {Store s(root);check(!s.known(a)&&!s.unexpected_previous_active(),"Initial state");s.remember(a,Decision::no);s.remember(b,Decision::yes);
     bool locked=false;try{Store second(root);}catch(...){locked=true;}check(locked,"Concurrent instance refusal");s.mark_active(a);}
    {Store s(root);check(s.get(a)==Decision::no&&s.get(b)==Decision::yes&&s.unexpected_previous_active(),"Persist build decisions/recovery");s.mark_clean();check(!s.unexpected_previous_active(),"Clean shutdown");s.reset();check(!s.known(a),"Reset");}
    for(unsigned mask=0;mask<32;++mask)for(auto d:{Decision::no,Decision::yes}){
        Inputs i{bool(mask&1),bool(mask&2),bool(mask&4),bool(mask&8),bool(mask&16)};auto result=evaluate(i,d);
        if(result==Result::activate)check((i.verified_build||d==Decision::yes)&&i.native_profile_available&&i.sites_valid&&(!i.unexpected_previous_active||i.recovery_allowed),"Unsafe activation");
        if(d==Decision::no)check(result==Result::disabled,"Saved No decision disables even if later verified");
    }
    {std::ofstream f(root/"decisions.txt",std::ios::binary);f<<"SPDEC1\n"<<a<<" X\n";}bool invalid=false;try{Store s(root);}catch(...){invalid=true;}check(invalid,"Corrupt decisions refusal");
    {std::ofstream f(root/"decisions.txt",std::ios::binary);f<<"SPDEC1\n";} {Store s(root);s.remember(a,Decision::yes);} // Failed constructor released lock.
    wchar_t executable[32768]{};check(GetModuleFileNameW(nullptr,executable,32768)>0,"Fixture executable path");auto identity=executable_hash(executable);
    auto bootstrap=root/"bootstrap";{Store s(bootstrap);s.remember(identity,Decision::no);}check(!allow_startup(executable,bootstrap),"Actual bootstrap helper bypassed saved No");
    {Store s(bootstrap);s.remember(identity,Decision::yes);}check(!allow_startup(executable,bootstrap),"Actual bootstrap helper bypassed missing native profile");
    std::cout<<"PASS compatibility: per-build Yes/No persistence/reset, native validation mandatory, all policy combinations, suspected active-session recovery, exclusive ownership, corrupt-state refusal; no Windows dialog or game activation\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
