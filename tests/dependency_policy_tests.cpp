// SPDX-License-Identifier: MIT
// Actual settings parser; desired dependency behavior is intentionally red on v1.0.3.
#include "startup_policy.hpp"
#include "remediation_harness.hpp"
#include <string>
using remediation_test::expect;
namespace {
std::string settings(bool global,bool enabled,bool vr,bool entry=true){
    std::string xml="<Data template='GcModSettings'><Property name='DisableAllMods' value='";
    xml+=global?"false":"true";xml+="'/>";
    if(entry){xml+="<Property name='Data'><Property name='Data' value='GcModSettingsInfo' _index='0'><Property name='Name' value='SUPERMOON_MENUAUDIO_TEST'/><Property name='Enabled' value='";xml+=enabled?"true":"false";xml+="'/><Property name='EnabledVR' value='";xml+=vr?"true":"false";xml+="'/></Property></Property>";}
    return xml+"</Data>";
}
void enabled(){expect(bytebeat::startup::mods_enabled(settings(true,true,true)),"well-formed enabled required entry accepted");}
void global_disabled(){expect(!bytebeat::startup::mods_enabled(settings(false,true,true)),"global disable refuses");}
void malformed(){expect(!bytebeat::startup::mods_enabled("<Data>"),"malformed settings refuse");}
void disabled(){expect(!bytebeat::startup::mods_enabled(settings(true,false,false)),"A1-M3 explicitly disabled required entry must refuse");}
void missing(){expect(!bytebeat::startup::mods_enabled(settings(true,false,false,false)),"missing required entry must not establish dependency admission");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"enabled_required_entry",enabled},{"global_disable",global_disabled},{"malformed_settings",malformed},
    {"disabled_required_entry",disabled},{"missing_required_entry",missing}};
    return remediation_test::main(argc,argv,cases);
}
