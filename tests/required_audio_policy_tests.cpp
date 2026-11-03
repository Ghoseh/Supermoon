// SPDX-License-Identifier: MIT
#include "startup_policy.hpp"
#include "audio_policy_fixtures.hpp"
#include "remediation_harness.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <cstring>
#include <new>
static bool allocation_audit{};
static unsigned allocations{};
void* operator new(size_t n){if(allocation_audit)++allocations;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,size_t) noexcept {std::free(p);}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,size_t) noexcept {std::free(p);}
using namespace bytebeat::startup;
using namespace audio_fixtures;
using remediation_test::expect;
namespace {
void policy(const std::string& xml,AudioPolicy expected,const char* reason){
    allocation_audit=true;auto actual=inspect_audio_settings(xml);allocation_audit=false;
    std::printf("POLICY actual=%s expected=%s\n",audio_policy_reason(actual),audio_policy_reason(expected));
    expect(actual==expected,reason);expect(allocations==0,"bounded settings policy allocates no memory");
}
void normal(){policy(settings(),AudioPolicy::allowed,"normal desktop registration metadata accepted");}
void desktop_flag(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","false")),AudioPolicy::allowed,"desktop Enabled independent of EnabledVR");}
void mode_disabled(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","false","true")),AudioPolicy::required_entry_disabled,"VR enablement cannot substitute for desktop Enabled");}
void absent_vr(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","")),AudioPolicy::allowed,"desktop does not require an optional VR flag");}
void no_index(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","false","")),AudioPolicy::allowed,"entry identity does not require an array index");}
void another_index(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","true","4294967295")),AudioPolicy::allowed,"index is metadata, not identity or precedence");}
void others(){policy(settings(record("OTHER","true","false","4")+record()+record("UNRELATED","false","true","8")),AudioPolicy::allowed,"unrelated enabled/disabled mods accepted; required flag retained after later record");}
void unicode(){policy(settings(record("\xce\x94 &amp; audio","true","false","3")+record()),AudioPolicy::allowed,"unrelated Unicode/entity names accepted");}
void dependency_metadata(){auto other=record("OTHER","true","false","7");auto at=other.find("<Property name='Dependencies'/>");
    other.replace(at,std::strlen("<Property name='Dependencies'/>"),"<Property name='Dependencies'><Property name='Data' value='GcModDependency'>"+property("Name","SUPERMOON_MENUAUDIO_TEST")+"</Property></Property>");
    policy(settings(other+record()),AudioPolicy::allowed,"unrelated dependency metadata is not a duplicate required mod entry");
}
void entities(){policy(settings(record("Supermoon_MenuAudio_T&#69;st","tr&#117;e","false")),AudioPolicy::allowed,"required name and boolean decode references once");}
void duplicate_name(){policy(settings(record()+record("supermoon_menuaudio_test","true","false","1")),AudioPolicy::ambiguous_required_entry,"duplicate required names refuse");}
void contradictory(){policy(settings(record()+record("SUPERMOON_MENUAUDIO_T&#69;ST","false","true","1")),AudioPolicy::ambiguous_required_entry,"contradictory aliased required entries refuse");}
void duplicate_index(){policy(settings(record()+record("OTHER","true","true","00")),AudioPolicy::ambiguous_required_entry,"duplicate required index with later record refuses");}
void duplicate_index_before(){policy(settings(record("OTHER","false","true","0")+record()),AudioPolicy::ambiguous_required_entry,"duplicate required index with earlier record refuses");}
void duplicate_enabled(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","true","0",property("Enabl&#101;d","false"))),AudioPolicy::ambiguous_required_entry,"duplicate logical Enabled field refuses");}
void duplicate_field_name(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","true","0",property("Name","OTHER"))),AudioPolicy::ambiguous_required_entry,"duplicate Name cannot hide the required entry");}
void duplicate_vr(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","true","false","0",property("EnabledVR","true"))),AudioPolicy::ambiguous_required_entry,"duplicate VR flag is ambiguous metadata");}
void invalid_bool(){policy(settings(record("SUPERMOON_MENUAUDIO_TEST","TRUE")),AudioPolicy::ambiguous_required_entry,"unsupported Enabled value refuses");}
void malformed(){policy(settings()+"<Property",AudioPolicy::malformed_or_global_disabled,"malformed suffix cannot authorize an earlier valid entry");}
void global_disabled(){policy(settings(record(),false),AudioPolicy::malformed_or_global_disabled,"global disable remains authoritative");}
void absent(){policy(settings(""),AudioPolicy::registration_pending,"missing registration is pending, not enabled");expect(!mods_enabled(settings("")),"original missing-entry assertion remains false");}
void wrong_name(){policy(settings(record("OTHER")),AudioPolicy::registration_pending,"unrelated entry cannot establish required registration");}
void misplaced(){auto body=property("DisableAllMods","false")+property("Name","SUPERMOON_MENUAUDIO_TEST")+property("Enabled","true");policy("<Data template='GcModSettings'>"+body+"</Data>",AudioPolicy::ambiguous_required_entry,"misplaced required fields cannot authorize");}
void double_encoded(){policy(settings(record("SUPERM&amp;#79;ON_MENUAUDIO_TEST")),AudioPolicy::registration_pending,"double encoding never creates a required-name alias");}
void large(){auto xml=settings();xml.insert(xml.size()-7,65536-xml.size(),' ');policy(xml,AudioPolicy::allowed,"64 KiB supported limit accepted");xml.insert(xml.size()-7,1,' ');policy(xml,AudioPolicy::malformed_or_global_disabled,"over-limit XML refuses");}
void deep(){auto xml=settings();std::string nested;for(unsigned i=0;i<34;++i)nested+="<Property name='Meta'>";for(unsigned i=0;i<34;++i)nested+="</Property>";xml.insert(xml.size()-7,nested);policy(xml,AudioPolicy::malformed_or_global_disabled,"depth limit preserved");}
void guard_pages(){auto xml=settings();SYSTEM_INFO info{};GetSystemInfo(&info);auto page=info.dwPageSize;
    auto memory=static_cast<char*>(VirtualAlloc(nullptr,3*page,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));remediation_test::require(memory&&xml.size()<page,"owned guard-page memory");
    DWORD old{};remediation_test::require(VirtualProtect(memory,page,PAGE_NOACCESS,&old)&&VirtualProtect(memory+2*page,page,PAGE_NOACCESS,&old),"guard protection");
    for(size_t offset:{size_t(0),size_t(page)-xml.size()}){auto at=memory+page+offset;memcpy(at,xml.data(),xml.size());allocation_audit=true;bool allowed=mods_enabled(std::string_view(at,xml.size()));allocation_audit=false;expect(allowed,"strict required policy at guard boundary");}
    VirtualFree(memory,0,MEM_RELEASE);expect(allocations==0,"strict guard-page parser has zero allocations");
}
void registration(){AudioRegistrationGate gate;
    for(unsigned n=0;n<10;++n)expect(!gate.update(AudioPolicy::registration_pending,Readiness::ready),"native-ready alone never admits missing entry");
    expect(!gate.update(AudioPolicy::allowed,Readiness::waiting),"entry alone never bypasses native readiness");
    expect(!gate.update(AudioPolicy::allowed,Readiness::ready),"first joint confirmation waits");
    expect(gate.update(AudioPolicy::allowed,Readiness::ready),"registered enabled entry and native readiness admit after two observations");
    expect(!gate.update(AudioPolicy::required_entry_disabled,Readiness::ready),"disable resets confirmation");
    expect(!audio_registration_can_wait(AudioPolicy::required_entry_disabled),"known disabled is a refusal, not pending registration");
}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"normal_desktop_metadata",normal},{"desktop_enabled_vr_disabled",desktop_flag},{"desktop_disabled_vr_enabled",mode_disabled},
    {"optional_vr_flag_absent",absent_vr},{"unindexed_registration",no_index},{"index_not_identity",another_index},
    {"unrelated_mods_preserved",others},{"unicode_unrelated_names",unicode},{"unrelated_dependency_metadata",dependency_metadata},{"decoded_required_identity",entities},
    {"duplicate_required_name",duplicate_name},{"contradictory_required_entries",contradictory},{"duplicate_required_index_after",duplicate_index},
    {"duplicate_required_index_before",duplicate_index_before},{"duplicate_enabled_alias",duplicate_enabled},{"duplicate_name_field",duplicate_field_name},
    {"duplicate_vr_field",duplicate_vr},{"malformed_enabled",invalid_bool},{"malformed_document",malformed},{"global_disable",global_disabled},
    {"missing_entry_pending_not_enabled",absent},{"unrelated_entry_not_required",wrong_name},{"misplaced_required_name",misplaced},
    {"single_entity_decode",double_encoded},{"size_limit",large},{"depth_limit",deep},{"strict_policy_guard_pages",guard_pages},
    {"registration_admission_sequence",registration}};
    return remediation_test::main(argc,argv,cases);
}
