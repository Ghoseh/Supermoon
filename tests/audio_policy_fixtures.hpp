// SPDX-License-Identifier: MIT
#pragma once
#include <string>
namespace audio_fixtures {
inline std::string property(const std::string& name,const std::string& value){return "<Property name='"+name+"' value='"+value+"'/>";}
inline std::string record(const std::string& name="SUPERMOON_MENUAUDIO_TEST",const std::string& enabled="true",
                          const std::string& vr="true",const std::string& index="0",const std::string& extra=""){
    return "<Property name='Data' value='GcModSettingsInfo'"+(index.empty()?std::string{}:" _index='"+index+"'")+">"+
        property("Name",name)+property("Author","")+property("ID","0")+property("AuthorID","0")+
        property("LastUpdated","0")+property("ModPriority","0")+property("Enabled",enabled)+
        (vr.empty()?std::string{}:property("EnabledVR",vr))+"<Property name='Dependencies'/>"+extra+"</Property>";
}
inline std::string settings(const std::string& records=record(),bool globally_enabled=true){
    return "<?xml version='1.0' encoding='utf-8'?><Data template='GcModSettings'>"+
        property("DisableAllMods",globally_enabled?"false":"true")+"<Property name='Data'>"+records+"</Property></Data>";
}
}
