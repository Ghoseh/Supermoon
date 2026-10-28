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
        check(classify(base,base+0x2EA2750,0,0)==Readiness::waiting);
        check(classify(base,0,base+0x2EA27D0,0)==Readiness::waiting);
        check(classify(base,base+0x2EA2750,base+0x2EA27D0,0)==Readiness::ready);
        check(classify(base,base+0x2EA2750,base+0x2EA27D0,1)==Readiness::conflict);
        check(classify(base,1,0,0)==Readiness::conflict);
        check(classify(base,0,1,0)==Readiness::conflict);
        check(classify(base,base+0x2EA2750+1,base+0x2EA27D0,0)==Readiness::conflict);
    }
    auto wrap=[](const std::string& property){return "<?xml version=\"1.0\"?><Data template=\"GcModSettings\">"+property+"</Data>";};
    const std::string enabled="<Property name=\"DisableAllMods\" value=\"false\" />";
    check(mods_enabled(wrap(enabled)));
    check(mods_enabled(wrap("<Property value = 'false' name = 'DisableAllMods'/>")));
    check(mods_enabled(wrap("<Property\nname=\"DisableAllMods\"\tvalue=\"false\"/>")));
    for(const std::string& value:{"true","False","FALSE","0","","unknown"," false","false "})
        check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\""+value+"\" />")));
    check(!mods_enabled(""));
    check(!mods_enabled(enabled));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" />")));
    check(!mods_enabled(wrap(enabled+enabled)));
    check(!mods_enabled(wrap("<!--"+enabled+"-->")));
    check(mods_enabled(wrap("<!--<Property name=\"DisableAllMods\" value=\"true\"/>-->"+enabled)));
    check(!mods_enabled(wrap("<!--unterminated"+enabled)));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false\" value=\"true\"/>")));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" name=\"Other\" value=\"false\"/>")));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false/>")));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\"value=\"false\"/>")));
    check(!mods_enabled(wrap("<Property name=\"DisableAllMods\" value=\"false\"")));
    check(!mods_enabled(wrap(enabled+std::string(1,'\0'))));
    check(!mods_enabled(wrap(enabled+std::string(65536,' '))));
    check(!mods_enabled("<!DOCTYPE Data>"+wrap(enabled)));
    check(!mods_enabled("<!--<Data template=\"GcModSettings\">-->"+enabled+"</Data>"));
    check(!mods_enabled("<Data template=\"GcModSettings\">"+enabled));
    check(!mods_enabled(wrap("")+enabled));
    check(!mods_enabled(enabled+wrap("")));
    printf("PASS %d startup readiness/disable-policy checks; no game loaded\n",checks);
}
