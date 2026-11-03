// SPDX-License-Identifier: MIT
#include "audio_dependency_preflight.hpp"
#include "audio_policy_fixtures.hpp"
#include "remediation_harness.hpp"
#include <fstream>
using namespace bytebeat::startup;
using namespace audio_fixtures;
using remediation_test::expect;
using remediation_test::require;
namespace {
struct Fixture {
    std::filesystem::path root=std::filesystem::absolute("owned-install"),exe=root/"Binaries/NMS.exe";
    std::filesystem::path xml=root/"Binaries/SETTINGS/GCMODSETTINGS.MXML",mods=root/"GAMEDATA/MODS";
    static void write(const std::filesystem::path& path,const std::string& bytes){std::filesystem::create_directories(path.parent_path());std::ofstream f(path,std::ios::binary);f.write(bytes.data(),std::streamsize(bytes.size()));f.close();require(bool(f),"owned fixture write");}
    Fixture(){std::filesystem::create_directories(exe.parent_path());std::filesystem::create_directories(mods);}
    AudioPreflight read(){return audio_dependency_preflight(exe.c_str());}
    void config(const std::string& bytes=settings()){write(xml,bytes);}
    void bank(const std::string& name,const std::string& bytes){write(mods/std::filesystem::u8path(name)/"audio/windows/init.bnk",bytes);}
};
void normal(){Fixture f;f.config();auto before=std::filesystem::last_write_time(f.xml);expect(f.read().policy==AudioPolicy::allowed,"ordinary no-argument desktop configuration accepted");expect(std::filesystem::last_write_time(f.xml)==before,"preflight never writes settings");}
void first_install(){Fixture f;AudioRegistrationGate gate;
    expect(f.read().policy==AudioPolicy::registration_pending,"first install without settings is pending");
    expect(!std::filesystem::exists(f.xml),"preflight does not create settings");
    f.config(settings(""));expect(!gate.update(f.read().policy,Readiness::ready),"NMS-created empty registry remains non-admitting");
    f.config();expect(!gate.update(f.read().policy,Readiness::ready),"NMS registration written; first confirmation waits");
    expect(gate.update(f.read().policy,Readiness::ready),"first installation can admit after NMS registers enabled mod");
}
void shared_writer(){Fixture f;f.config(settings(""));auto writer=CreateFileW(f.xml.c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(writer!=INVALID_HANDLE_VALUE,"owned exclusive NMS-like writer");
    expect(f.read().policy==AudioPolicy::registration_pending,"in-progress native writer is pending, not permanent disable");CloseHandle(writer);
    f.config();expect(f.read().policy==AudioPolicy::allowed,"normal completed registration becomes eligible");
}
void disabled_registration(){Fixture f;f.config(settings(record("SUPERMOON_MENUAUDIO_TEST","false","true")));expect(f.read().policy==AudioPolicy::required_entry_disabled,"registered disabled mod is not auto-enabled");}
void malformed(){Fixture f;f.config("<Data>");expect(f.read().policy==AudioPolicy::malformed_or_global_disabled,"malformed registered settings refuse");}
void unrelated(){Fixture f;f.config(settings(record()+record("OTHER","true","false","1")));Fixture::write(f.mods/"OTHER/textures/other.bin","unrelated");expect(f.read().policy==AudioPolicy::allowed,"enabled unrelated loose mod allowed");}
void conflict(){Fixture f;f.config(settings(record()+record("OTHER","true","false","1")));f.bank("Supermoon_MenuAudio_Test","owned required synthetic bytes");f.bank("OTHER","different synthetic bank");expect(f.read().policy==AudioPolicy::conflicting_bank,"known enabled different override of exact bank resource refuses");}
void identical(){Fixture f;f.config(settings(record()+record("OTHER","true","true","1")));f.bank("Supermoon_MenuAudio_Test","identical synthetic bank");f.bank("OTHER","identical synthetic bank");expect(f.read().policy==AudioPolicy::allowed,"identical competing bytes do not presume incompatible content");}
void disabled_other(){Fixture f;f.config(settings(record()+record("OTHER","false","true","1")));f.bank("OTHER","different synthetic bank");expect(f.read().policy==AudioPolicy::allowed,"desktop-disabled other bank does not block desktop admission");}
void unregistered_other(){Fixture f;f.config();f.bank("UNREGISTERED","synthetic bank");expect(f.read().policy==AudioPolicy::allowed,"unknown on-disk folder alone does not prove enabled conflicting override");}
void packaged(){Fixture f;f.config(settings(record()+record("OTHER.pak","true","true","1")));Fixture::write(f.mods/"OTHER.pak","opaque unrelated package");auto result=f.read();expect(result.policy==AudioPolicy::allowed&&result.loose_scan_limited,"opaque package is a documented inspection limit, not blanket refusal");}
void encoded_other(){Fixture f;f.config(settings(record()+record("Other &amp; \xce\x94","true","true","1")));f.bank("Supermoon_MenuAudio_Test","synthetic own");f.bank("Other & \xce\x94","synthetic other");expect(f.read().policy==AudioPolicy::conflicting_bank,"Unicode/entity names map once to a known enabled loose resource");}
void unsafe_name(){Fixture f;f.config(settings(record()+record("../OUTSIDE","true","true","1")));auto r=f.read();expect(r.policy==AudioPolicy::allowed&&r.loose_scan_limited,"unrelated unsupported path name never traverses outside MODS");}
void cold_missing_bank(){Fixture f;f.config();expect(f.read().policy==AudioPolicy::allowed,"settings check alone never claims bank identity");expect(!std::filesystem::exists(f.mods/"Supermoon_MenuAudio_Test/audio/windows/init.bnk"),"bank still absent: existing mandatory payload identity gate remains required");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"ordinary_desktop_read_only",normal},{"first_install_registration",first_install},{"native_registration_writer",shared_writer},
    {"registered_disabled",disabled_registration},{"malformed_settings",malformed},{"unrelated_enabled_mod",unrelated},
    {"known_loose_bank_conflict",conflict},{"identical_bank_bytes",identical},{"disabled_other_bank",disabled_other},
    {"unregistered_folder_not_enabled_proof",unregistered_other},{"opaque_package_not_blanket_refused",packaged},
    {"unicode_entity_conflict_name",encoded_other},{"no_name_path_traversal",unsafe_name},{"settings_not_bank_identity",cold_missing_bank}};
    return remediation_test::main(argc,argv,cases);
}
