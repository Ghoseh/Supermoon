// SPDX-License-Identifier: MIT
#pragma once
#include "startup_policy.hpp"

namespace bytebeat::startup {
// Desktop admission only. VR is unsupported; no launch arguments are required
// or inspected. A missing entry is never enabled, but may still be registering.
enum class AudioPolicy : uint32_t {allowed,registration_pending,malformed_or_global_disabled,
    ambiguous_required_entry,required_entry_disabled,conflicting_bank,settings_unreadable};
inline const char* audio_policy_reason(AudioPolicy policy) noexcept {
    switch(policy){
    case AudioPolicy::allowed:return "audio_entry_enabled_not_loaded_bank_proof";
    case AudioPolicy::registration_pending:return "waiting_required_audio_mod_registration";
    case AudioPolicy::malformed_or_global_disabled:return "refused_malformed_or_globally_disabled_mods";
    case AudioPolicy::ambiguous_required_entry:return "refused_ambiguous_required_audio_entry";
    case AudioPolicy::required_entry_disabled:return "refused_required_audio_entry_disabled";
    case AudioPolicy::conflicting_bank:return "refused_competing_audio_windows_init_bank";
    case AudioPolicy::settings_unreadable:return "refused_audio_settings_unreadable";
    }
    return "refused_unknown_audio_policy";
}
inline bool next_xml_code(std::string_view raw,size_t& p,unsigned& code) noexcept {
    if(p==raw.size())return false;
    code=static_cast<unsigned char>(raw[p++]);
    if(code=='&'){
        auto end=raw.find(';',p);if(end==raw.npos||!xml_reference(raw.substr(p,end-p),code))return false;
        p=end+1;return true;
    }
    if(code<128)return xml_character(code);
    unsigned n=0,minimum=0;
    if(code>=0xc2&&code<=0xdf){code&=31;n=1;minimum=0x80;}
    else if(code>=0xe0&&code<=0xef){code&=15;n=2;minimum=0x800;}
    else if(code>=0xf0&&code<=0xf4){code&=7;n=3;minimum=0x10000;}else return false;
    if(n>raw.size()-p)return false;
    while(n--){auto b=static_cast<unsigned char>(raw[p++]);if((b&0xc0)!=0x80)return false;code=(code<<6)|(b&63);}
    return code>=minimum&&xml_character(code);
}
inline bool required_audio_name(std::string_view raw) noexcept {
    constexpr std::string_view expected="supermoon_menuaudio_test";
    size_t p=0,n=0;
    while(p<raw.size()){
        unsigned code{};if(!next_xml_code(raw,p,code))return false;
        if(code>='A'&&code<='Z')code+='a'-'A';
        if(n==expected.size()||code!=static_cast<unsigned char>(expected[n++]))return false;
    }
    return n==expected.size();
}
namespace audio_policy_detail {
struct NoConflictCheck {bool operator()(std::string_view) noexcept {return false;}};
template<class ConflictCheck> struct RequiredEntry {
    enum Node {other,list,entry,leaf};
    std::array<Node,33> nodes{};
    // More than can fit as complete typed records in the 64 KiB grammar.
    std::array<uint32_t,1024> indices{};
    unsigned lists{},indexed_records{},required{},names{},enabled_fields{},vr_fields{};
    uint32_t row_index{},required_index{};
    bool indexed{},required_indexed{},row_bad{},row_required{},row_enabled{},required_enabled{},conflict{},ambiguous{};
    std::string_view row_name;
    ConflictCheck& check;
    explicit RequiredEntry(ConflictCheck& c) noexcept:check(c){}
    static bool is(std::string_view raw,std::string_view value) noexcept {return xml_attribute_equals(raw,value);}
    void open(unsigned depth,const ModSettingsToken& token) noexcept {
        auto parent=depth?nodes[depth-1]:other;nodes[depth]=other;
        if(parent==leaf){if(depth==2)ambiguous=true;else row_bad=true;}
        if(depth==0)return;
        bool name=is(token.property,"Name"),enabled=is(token.property,"Enabled"),vr=is(token.property,"EnabledVR");
        if(depth==1&&is(token.property,"DisableAllMods")){
            nodes[depth]=leaf;if(token.attributes!=2||token.indexed||!token.type.empty())ambiguous=true;
        }else if(depth==1&&is(token.property,"Data")){
            nodes[depth]=list;if(++lists!=1||token.valued||token.indexed||!token.type.empty())ambiguous=true;
        }else if(depth==2&&parent==list&&is(token.property,"Data")&&is(token.value,"GcModSettingsInfo")){
            nodes[depth]=entry;names=enabled_fields=vr_fields=0;row_name={};
            row_bad=!token.type.empty();row_required=row_enabled=false;
            indexed=token.indexed;row_index=token.index;
        }else if(parent==entry&&depth==3&&(name||enabled||vr)){
            nodes[depth]=leaf;
            if(!token.valued||token.indexed||!token.type.empty())row_bad=true;
            if(name){++names;row_name=token.value;row_required|=required_audio_name(token.value);}
            else{
                bool yes=is(token.value,"true"),no=is(token.value,"false");
                if(!yes&&!no)row_bad=true;
                if(enabled){++enabled_fields;row_enabled=yes;}else ++vr_fields;
            }
        }else if(depth<=3&&name&&required_audio_name(token.value))ambiguous=true;
    }
    void close(unsigned depth) noexcept {
        if(nodes[depth]!=entry)return;
        if(row_required){
            ++required;required_indexed=indexed;required_index=row_index;required_enabled=row_enabled;
            if(row_bad||names!=1||enabled_fields!=1||vr_fields>1)ambiguous=true;
            if(indexed)for(unsigned n=0;n<indexed_records;++n)if(indices[n]==row_index)ambiguous=true;
        }else{
            if(required_indexed&&indexed&&row_index==required_index)ambiguous=true;
            if(names==1&&enabled_fields==1&&row_enabled&&!row_bad)conflict|=check(row_name);
        }
        if(indexed){if(indexed_records==indices.size())ambiguous=true;else indices[indexed_records++]=row_index;}
    }
    AudioPolicy result() const noexcept {
        if(ambiguous||required>1)return AudioPolicy::ambiguous_required_entry;
        if(!required)return AudioPolicy::registration_pending;
        if(!required_enabled)return AudioPolicy::required_entry_disabled;
        return conflict?AudioPolicy::conflicting_bank:AudioPolicy::allowed;
    }
};
}
template<class ConflictCheck> inline AudioPolicy inspect_audio_settings(std::string_view xml,ConflictCheck& check) noexcept {
    audio_policy_detail::RequiredEntry<ConflictCheck> observer(check);
    if(!parse_mod_settings(xml,observer))return AudioPolicy::malformed_or_global_disabled;
    return observer.result();
}
inline AudioPolicy inspect_audio_settings(std::string_view xml) noexcept {
    audio_policy_detail::NoConflictCheck check;return inspect_audio_settings(xml,check);
}
inline bool mods_enabled(std::string_view xml) noexcept {return inspect_audio_settings(xml)==AudioPolicy::allowed;}
inline bool audio_registration_can_wait(AudioPolicy policy) noexcept {
    return policy==AudioPolicy::allowed||policy==AudioPolicy::registration_pending;
}
struct AudioRegistrationGate {
    unsigned confirmations{};
    bool update(AudioPolicy policy,Readiness native) noexcept {
        if(policy==AudioPolicy::allowed&&native==Readiness::ready){if(confirmations<2)++confirmations;}
        else confirmations=0;
        return confirmations>=2;
    }
};
}
