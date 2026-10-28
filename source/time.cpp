// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "time.hpp"
#include <cstring>
#include <bcrypt.h>
#include "register_value.hpp"
#include "register_profile.hpp"
extern "C" {void* TimeOriginals[2]{};}
namespace supermoon::ui::time {
std::atomic<bool> enabled{false};
std::atomic<uint64_t> title_replaced{0},time_replaced{0},fallbacks{0};
uintptr_t image{},now_return{},time_return{},keys_address{};
bytebeat::playlist::Engine* engine{};bytebeat::SupermoonCore* core{};const FilenameTitles* filenames{};
unsigned char carrier_key[16]{};Read read_memory=ReadProcessMemory;
static thread_local Slot slots[max_depth]{};
static thread_local unsigned depth=0;
#ifdef UI_TIME_TEST
SnapshotReader test_snapshot{};
Slot test_slot() noexcept {return slots[0];}
unsigned test_depth() noexcept {return depth;}
#endif
static bool snapshot(bytebeat::playlist::PlaybackSnapshot& value) noexcept {
#ifdef UI_TIME_TEST
    if(test_snapshot)return test_snapshot(value);
#endif
    return engine&&engine->now_playing_snapshot(value);
}
bool safe_read(uintptr_t p,void* out,size_t n) noexcept {
    if(!p||!n||n>256||p>UINTPTR_MAX-n||!read_memory)return false;
    SIZE_T copied{};
    __try {return read_memory(GetCurrentProcess(),reinterpret_cast<void*>(p),out,n,&copied)&&copied==n;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool field(uintptr_t p,size_t off,void* out,size_t n) noexcept {return p<=UINTPTR_MAX-off&&safe_read(p+off,out,n);}
static Slot* eligible() noexcept {
    if(!enabled.load(std::memory_order_acquire)||depth!=1||slots[0].tainted)return nullptr;
    return &slots[0];
}
// Called only after the original formatter. Keep the pinned native-title, key,
// owner, output-object and basename contracts, then commit one owned tuple.
bool substitute_title(const CallContext& c,const unsigned char* key,char* output) noexcept {
    auto s=eligible();if(!s||c.return_site!=now_return)return false;
    if(s->titles++){s->valid=false;s->tainted=true;return false;}
    if(!core||!filenames||c.key_list!=keys_address||c.playlist_member<0x7d5c||
       c.frame_base>UINTPTR_MAX-0xf0||reinterpret_cast<uintptr_t>(output)!=c.frame_base+0xf0)return false;
    auto k=reinterpret_cast<uintptr_t>(key);
    if(k<keys_address||k-keys_address>=256||((k-keys_address)&15))return false;
    auto component=c.playlist_member-0x7d5c;
    if(component>UINTPTR_MAX-0x28)return false;auto stream=component+0x28;
    if(core->ui_bound_stream()!=stream)return false;
    unsigned char copied[16]{};
    if(!safe_read(k,copied,16)||memcmp(copied,carrier_key,16))return false;
    constexpr char expected[]="Hello Games - Supermoon";
    size_t length=0;while(length<sizeof(expected)&&output[length])++length;
    if(length!=sizeof(expected)-1||memcmp(output,expected,length))return false;
    Frozen value{};if(!snapshot(value.snapshot)||!progress::valid(value.snapshot)||value.snapshot.track>=filenames->size())return false;
    value.filename=(*filenames)[value.snapshot.track];
    if(!value.filename.length||value.filename.length>255||value.filename.bytes[value.filename.length]||
       !progress::caption(value.snapshot,value.caption,value.length))return false;
    uintptr_t root{};
    if(c.frame_base<0x20||!safe_read(c.frame_base-0x20,&root,8)||!root)return false;
    // Recheck identity, not position: the owned tuple is the source of truth for
    // this rendering invocation even if PCM advances immediately afterwards.
    bytebeat::playlist::PlaybackSnapshot after{};
    if(!snapshot(after)||!progress::valid(after)||after.serial!=value.snapshot.serial||after.revision!=value.snapshot.revision||
       after.track!=value.snapshot.track||after.length!=value.snapshot.length||core->ui_bound_stream()!=stream)return false;
    s->frame=c.frame_base;s->member=c.playlist_member;s->keys=c.key_list;s->root=root;s->stream=stream;s->value=value;s->valid=true;s->phase=1;
    memcpy(output,value.filename.bytes.data(),size_t(value.filename.length)+1);
    title_binding.replaced.fetch_add(1,std::memory_order_relaxed);title_replaced.fetch_add(1,std::memory_order_relaxed);return true;
}
static const char* select(void* object,const Context& c) noexcept {
    auto original=reinterpret_cast<const char*>(c.source);auto s=eligible();
    if(!s||!s->valid||s->phase!=1||c.site!=time_return||c.frame!=s->frame||c.member!=s->member||c.keys!=s->keys||
       s->frame>UINTPTR_MAX-0x68||c.source!=s->frame+0x68||!core||core->ui_bound_stream()!=s->stream){fallbacks++;return original;}
    // No stored widget pointers: inspect only the object in this setter call.
    uintptr_t vtable{},definition{},parent{};unsigned char role[16]{};
    constexpr unsigned char expected[16]={'T','I','M','E'};
    auto p=reinterpret_cast<uintptr_t>(object);
    if(!field(p,0,&vtable,8)||vtable!=image+0x4B09130||!field(p,0x48,&definition,8)||
       !field(definition,0x48,role,16)||memcmp(role,expected,16)||!field(p,0x40,&parent,8)){fallbacks++;return original;}
    uintptr_t visited[8]{};bool matched=false;
    for(unsigned i=0;i<8&&parent;++i){bool cycle=false;for(unsigned j=0;j<i;++j)cycle|=visited[j]==parent;if(cycle)break;
        visited[i]=parent;if(parent==s->root){matched=true;break;}if(!field(parent,0x40,&parent,8))break;}
    if(!matched||core->ui_bound_stream()!=s->stream){fallbacks++;return original;}
    // Consume once. This owned caption remains alive through the native copying
    // setter, including its reentrant calls; nested scopes never overwrite it.
    s->phase=2;time_replaced.fetch_add(1,std::memory_order_relaxed);return s->value.caption.data();
}
// Owned data copied from a caller-live catalog record; no native pointers
// survive this function. SHA checks the known pinned 120-BPM composition.
static uint64_t progress_token(const Context& c,const char* selected) noexcept {
    auto slot=eligible();if(!slot||!slot->valid||slot->phase!=2||selected!=slot->value.caption.data()||c.site!=time_return||c.frame!=slot->frame||c.member!=slot->member||c.keys!=slot->keys||!core||core->ui_bound_stream()!=slot->stream)return 0;
    uintptr_t record{};char catalog[64]{};int loops{},again{};unsigned char tempo{},hash[32]{};
    if(c.frame<0x60||!safe_read(c.frame-0x60,&record,8)||record>UINTPTR_MAX-0x40||!field(record,0x40,catalog,sizeof(catalog))||catalog[60]||!safe_read(image+0x5045E1C,&loops,4)||!field(slot->stream,0x834,&tempo,1)||tempo!=120)return 0;
    BCRYPT_ALG_HANDLE alg{};BCRYPT_HASH_HANDLE digest{};
    bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptCreateHash(alg,&digest,nullptr,0,nullptr,0,0)>=0;
    if(ok)ok=BCryptHashData(digest,reinterpret_cast<unsigned char*>(catalog),60,0)>=0;
    if(ok)ok=BCryptFinishHash(digest,hash,32,0)>=0;
    if(digest)BCryptDestroyHash(digest);if(alg)BCryptCloseAlgorithmProvider(alg,0);
    float elapsed{};
    if(!ok||memcmp(hash,register_profile::catalog_sha,32)||!safe_read(image+0x5045E1C,&again,4)||again!=loops||!eligible()||core->ui_bound_stream()!=slot->stream||!register_progress::value(slot->value.snapshot.position,slot->value.snapshot.length,loops,elapsed))return 0;
    return register_progress::token(elapsed);
}
bool prepare(unsigned char* base,bytebeat::playlist::Engine* e,bytebeat::SupermoonCore* c,const unsigned char* key,const FilenameTitles* names) noexcept {
    if(enabled.load()||TimeOriginals[0]||!base||!e||!c||!key||!names)return false;
    constexpr uint32_t cells[]={0x4B13108,0x4B091C0};constexpr uint32_t targets[]={0x341690,0x1FEF50};
    for(unsigned i=0;i<2;++i)if(*reinterpret_cast<void**>(base+cells[i])!=base+targets[i])return false;
    image=reinterpret_cast<uintptr_t>(base);now_return=image+profile::now_return;time_return=image+0xF89053;keys_address=image+profile::personal_keys;
    engine=e;core=c;filenames=names;memcpy(carrier_key,key,16);
    for(unsigned i=0;i<2;++i)TimeOriginals[i]=base+targets[i];return true;
}
bool publish(unsigned char* base) noexcept {
    constexpr uint32_t cells[]={0x4B13108,0x4B091C0};void* wrappers[]={reinterpret_cast<void*>(TimeSimulation),reinterpret_cast<void*>(TimeSetterBridge)};
    for(unsigned i=0;i<2;++i)if(redirect(reinterpret_cast<void* volatile*>(base+cells[i]),TimeOriginals[i],wrappers[i])!=Redirect::changed){cancel();return false;}
    enabled.store(true,std::memory_order_release);return true;
}
void cancel() noexcept {enabled.store(false,std::memory_order_release);} // Resident exact-forwarding on partial publication; never unhook/reclaim.
}
extern "C" const char* TimeSelect(void* object,const supermoon::ui::time::Context* context) noexcept {
    auto error=GetLastError();auto result=supermoon::ui::time::select(object,*context);SetLastError(error);return result;
}
extern "C" void TimeSimulation(void* object) {
    using namespace supermoon::ui::time;
    auto error=GetLastError();auto level=depth++;
    if(level&&level<=max_depth){slots[level-1].tainted=true;slots[level-1].valid=false;}
    if(level<max_depth)slots[level]={};
    SetLastError(error);
    __try {reinterpret_cast<void(*)(void*)>(TimeOriginals[0])(object);}
    __finally {auto saved=GetLastError();if(level<max_depth)slots[level]={};depth=level;SetLastError(saved);}
}

extern "C" uint64_t TimeProgressToken(const supermoon::ui::time::Context* c,const char* selected) noexcept {auto e=GetLastError();auto t=supermoon::ui::time::progress_token(*c,selected);SetLastError(e);return t;}
