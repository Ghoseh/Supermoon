// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "title_test.hpp"
#include "time.hpp"
#include <cstring>
namespace supermoon::ui {
TitleBinding title_binding;
bool substitute_test_title(const CallContext& c,const unsigned char* key,char* output) noexcept {
    if(!title_binding.enabled.load(std::memory_order_acquire)||c.return_site!=journal.now_return||
        !title_binding.core||!journal.engine||c.key_list!=title_binding.personal_keys||
        c.playlist_member<0x7d5c||c.frame_base>UINTPTR_MAX-0xf0||
        reinterpret_cast<uintptr_t>(output)!=c.frame_base+0xf0)return false;
    const auto k=reinterpret_cast<uintptr_t>(key);
    if(k<c.key_list||k-c.key_list>=256||((k-c.key_list)&15))return false;
    const auto component=c.playlist_member-0x7d5c;
    if(component>UINTPTR_MAX-0x28)return false;
    const auto stream=component+0x28;
    // Read-only atomics, no shared core guard: UI cannot contend with PCM delivery.
    if(title_binding.core->ui_bound_stream()!=stream)return false;
    if(memcmp(key,journal.carrier,16)!=0)return false;
    bytebeat::playlist::PlaybackSnapshot before{},after{};
    if(!journal.engine->now_playing_snapshot(before)||!before.serial||!before.revision||
        before.track==UINT32_MAX||!before.length||before.position>before.length)return false;
    // Preserve the verified native carrier identity before filename substitution.
    // Different localization/author/unknown or unterminated output keeps native text.
    constexpr char expected[]="Hello Games - Supermoon";
    // Do not read indeterminate stack bytes after an early NUL (e.g. native
    // lookup failure leaves only output[0] initialized). Bound the scan first.
    size_t length=0;while(length<sizeof(expected)&&output[length])++length;
    if(length!=sizeof(expected)-1||memcmp(output,expected,length)!=0)return false;
    const auto* names=title_binding.filenames;
    if(!names||before.track>=names->size())return false;
    const auto& text=(*names)[before.track];
    if(!text.length||text.length>255||text.bytes[text.length]!=0)return false;
    if(!journal.engine->now_playing_snapshot(after)||before.serial!=after.serial||
        before.revision!=after.revision||before.track!=after.track||before.length!=after.length||
        title_binding.core->ui_bound_stream()!=stream)return false;
    // Same native-owned fixed-string object, pointer and lifetime. No new string
    // storage, pointer substitution, length/header field or widget invocation.
    memcpy(output,text.bytes.data(),size_t(text.length)+1);title_binding.replaced.fetch_add(1,std::memory_order_relaxed);return true;
}
extern "C" void SupermoonTitleTestHandler(void* object,const unsigned char* key,char* output,const CallContext* c) {
    journal.original(object,key,output);
    time::substitute_title(*c,key,output);
    journal.observe(object,key,output,c->return_site);
}
}
