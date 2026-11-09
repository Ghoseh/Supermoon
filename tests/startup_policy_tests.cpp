// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "startup_policy.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
using namespace bytebeat::startup;
int checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL check %d\n",checks);exit(1);}}
int main(){
    for(uintptr_t base:{uintptr_t(0x140000000),uintptr_t(0x7FF600000000)}) {
        check(classify(base,0,0,0)==Readiness::waiting);
        check(classify(base,base+0x2ea7540,0,0)==Readiness::waiting);
        check(classify(base,0,base+0x2ea75c0,0)==Readiness::waiting);
        check(classify(base,base+0x2ea7540,base+0x2ea75c0,0)==Readiness::ready);
        check(classify(base,base+0x2ea7540,base+0x2ea75c0,1)==Readiness::conflict);
        check(classify(base,1,0,0)==Readiness::conflict);
        check(classify(base,0,1,0)==Readiness::conflict);
        check(classify(base,base+0x2ea7540+1,base+0x2ea75c0,0)==Readiness::conflict);
    }
    auto wrap=[](const std::string& property){return "<?xml version=\"1.0\"?><Data template=\"GcModSettings\">"+property+"</Data>";};
    const std::string enabled="<Property name=\"DisableAllMods\" value=\"false\" />";
    check(global_mods_enabled(wrap(enabled)));
    check(global_mods_enabled(wrap("<Property value = 'false' name = 'DisableAllMods'/>")));
    check(global_mods_enabled(wrap("<Property\nname=\"DisableAllMods\"\tvalue=\"false\"/>")));
    for(const std::string& value:{"true","False","FALSE","0","","unknown"," false","false "})
        check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\""+value+"\" />")));
    check(!global_mods_enabled(""));
    check(!global_mods_enabled(enabled));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" />")));
    check(!global_mods_enabled(wrap(enabled+enabled)));
    check(!global_mods_enabled(wrap("<!--"+enabled+"-->")));
    check(global_mods_enabled(wrap("<!--<Property name=\"DisableAllMods\" value=\"true\"/>-->"+enabled)));
    check(!global_mods_enabled(wrap("<!--unterminated"+enabled)));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false\" value=\"true\"/>")));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" name=\"Other\" value=\"false\"/>")));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false/>")));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\"value=\"false\"/>")));
    check(!global_mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false\"")));
    check(!global_mods_enabled(wrap(enabled+std::string(1,'\0'))));
    check(!global_mods_enabled(wrap(enabled+std::string(65536,' '))));
    check(!global_mods_enabled("<!DOCTYPE Data>"+wrap(enabled)));
    check(!global_mods_enabled("<!--<Data template=\"GcModSettings\">-->"+enabled+"</Data>"));
    check(!global_mods_enabled("<Data template=\"GcModSettings\">"+enabled));
    check(!global_mods_enabled(wrap("")+enabled));
    check(!global_mods_enabled(enabled+wrap("")));
    const std::string mod="<Property name='Data'><Property name='Data' value='GcModSettingsInfo' _index='0'><Property name='Name' value='MENU_AUDIO_TEST'/><Property name='Enabled' value='true'/><Property name='Dependencies'/></Property></Property>";
    check(global_mods_enabled(wrap(enabled+mod)));
    check(!global_mods_enabled(wrap("<Property name='DisableAllMods' value='true'/>"+mod)));
    check(!global_mods_enabled(wrap(enabled+"<Property name='Data' value='GcModSettingsInfo' _index='0'><Property name='DisableAllMod&#115;' value='true'/></Property>")));
    for(const std::string& index:{"","-1","+1","1.0","4294967296","00000000000","&#48;"})
        check(!global_mods_enabled(wrap(enabled+"<Property name='Data' value='GcModSettingsInfo' _index='"+index+"'/>")));
    check(global_mods_enabled(wrap(enabled+"<Property name='Data' value='GcModSettingsInfo' _index='4294967295'/>")));
    check(!global_mods_enabled(wrap(enabled+"<Property name='Data' _index='0' _index='1'/>")));
    printf("PASS %d startup readiness/disable-policy checks; no game loaded\n",checks);
}
