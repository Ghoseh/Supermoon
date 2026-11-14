// SPDX-License-Identifier: MIT
#include "personal_voice_runtime.hpp"
#include <fstream>
#include <limits>
#include <cstdio>
namespace bytebeat::recovery {Registry registry;thread_local Event* constructing{};}
using namespace bytebeat::personal_gain;
static unsigned checks{};
void need(bool value,const char* text){++checks;if(!value){std::fprintf(stderr,"FAIL %s\n",text);ExitProcess(1);}}
int main(){
    auto root=std::filesystem::absolute(std::filesystem::path("fixtures/gain")/std::to_string(GetCurrentProcessId()));std::filesystem::create_directories(root);
    Runtime config;config.configure(root);need(config.setting.valid&&config.setting.db==3&&std::abs(config.setting.factor-1.41253754f)<1e-7f,"missing INI +3 default");
    auto path=root/"playlist.ini";
    for(const char* body:{"[Playlist]\nMusic=Music\n","[Audio]\n; no key\n"}){
        {std::ofstream out(path);out<<body;}config.configure(root);need(config.setting.valid&&config.setting.db==3,"existing INI missing key +3 default");
    }
    for(const wchar_t* value:{L"0",L"0.0",L"1.5",L"3.0",L"4.5",L"6.0",L"2.25",L" 3.0 "}){auto s=parse(value);need(s.valid&&s.db>=0&&s.db<=6,"supported finite decimal setting");}
    for(const wchar_t* value:{L"",L"NaN",L"inf",L"-1",L"6.01",L"1e0",L"3 dB",L"1.2.3",L"0x3",L"+3",L"999999999999999999999999999999999999999999"}){auto s=parse(value);need(!s.valid&&s.db==0&&s.factor==1,"invalid setting neutral");}
    for(const char* text:{"0.0","1.5","3.0","4.5","6.0","NaN","inf","6.01",""}){
        auto contents=std::string("[Audio]\nVoiceGainDb=")+text+"\n; personal preference\n";
        {std::ofstream out(path,std::ios::binary);out<<contents;}
        config.configure(root);std::ifstream in(path,std::ios::binary);std::string after((std::istreambuf_iterator<char>(in)),{});need(after==contents,"startup never rewrites existing INI");
        if(!strcmp(text,"0.0"))need(config.setting.valid&&config.setting.db==0&&config.setting.factor==1,"explicit neutral overrides release default");
        if(!strcmp(text,"6.01")||!strcmp(text,"NaN")||!strcmp(text,"inf")||!*text)need(!config.setting.valid&&config.setting.db==0,"malformed existing preference neutral");
    }
    auto hold=CreateFileW(path.c_str(),GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);need(hold!=INVALID_HANDLE_VALUE,"own unreadable INI fixture");
    config.configure(root);need(!config.setting.valid&&config.setting.db==0,"unreadable existing INI refuses positive default");CloseHandle(hold);
    auto bad=root/"directory-ini";std::filesystem::create_directories(bad/"playlist.ini");config.configure(bad);need(!config.setting.valid&&config.setting.db==0,"directory INI refuses positive default");
    Targets targets;float sample=.112201847f,original=sample;auto setting=parse(L"3");need(targets.apply(16,&sample,setting.factor)==Outcome::applied,"gain applied once");
    auto boosted=sample;for(unsigned n=0;n<10000;++n)need(targets.apply(16,&sample,setting.factor)==Outcome::ambiguous&&sample==boosted,"repeated callback never compounds");
    need(std::abs(boosted-original*setting.factor)<1e-7f,"correct +3 linear target");
    sample=.9f;need(targets.apply(32,&sample,setting.factor)==Outcome::headroom&&sample==.9f,"insufficient target headroom leaves native value");
    sample=0;need(targets.apply(48,&sample,setting.factor)==Outcome::neutral&&sample==0,"mute preserved");
    sample=original;need(targets.apply(64,&sample,1)==Outcome::neutral&&sample==original,"0 dB unchanged");
    sample=std::numeric_limits<float>::max();need(targets.apply(80,&sample,setting.factor)==Outcome::headroom,"huge native target rejected before multiplication");
    std::printf("PASS gain_settings checks=%u default_db=3 explicit_neutral=PASS INI_preserved=PASS nonaccumulation=PASS no_game_or_DLL_loaded\n",checks);
}
