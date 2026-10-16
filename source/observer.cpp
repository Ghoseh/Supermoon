// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "observer.hpp"
#include <cstring>

namespace supermoon::ui {
Journal journal;
static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(std::atomic<bool>::is_always_lock_free);
void Journal::observe(void* object,const unsigned char* key,const char* output,uintptr_t site) noexcept {
    auto route=site==now_return?1u:site==row_return?2u:0u;
    if(!route||!enabled.load(std::memory_order_acquire))return;
    auto call=calls[route].fetch_add(1,std::memory_order_relaxed)+1;
    bytebeat::playlist::PlaybackSnapshot snap{};
    bool valid=engine&&engine->now_playing_snapshot(snap);
    // Approximate change sampling across UI threads, not a complete transition log.
    bool changed=false;
    if(valid){changed=last_serial[route-1].exchange(snap.serial)!=snap.serial;
        changed=(last_revision[route-1].exchange(snap.revision)!=snap.revision)||changed;}
    if(call>32&&call%64&&!changed)return;
    auto n=reserved.fetch_add(1,std::memory_order_relaxed);
    if(n>=capacity){dropped.fetch_add(1,std::memory_order_relaxed);return;}
    Record r{};r.call=call;r.thread=GetCurrentThreadId();r.route=route;r.return_site=site;
    LARGE_INTEGER time{};QueryPerformanceCounter(&time);r.ticks=uint64_t(time.QuadPart);
    r.function_token=reinterpret_cast<uintptr_t>(object);r.output_token=reinterpret_cast<uintptr_t>(output);
    r.snapshot_valid=valid;if(valid)r.snapshot=snap;
    // Only the known reference contracts are read, after the original returns.
    // Unknown call sites never cause a borrowed-pointer read. No widget access.
    memcpy(r.key,key,16);r.carrier=memcmp(key,carrier,16)==0;
    for(uint32_t i=0;i<256;++i){auto b=static_cast<unsigned char>(output[i]);
        if(!b){r.terminated=true;break;}r.output[r.length++]=b;}
    slots[n].value=r;slots[n].ready.store(true,std::memory_order_release);
}
extern "C" __declspec(noinline) void SupermoonObserveTitle(void* object,const unsigned char* key,char* output) {
    const auto site=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    journal.original(object,key,output);
    journal.observe(object,key,output,site);
}
Redirect redirect(void* volatile* cell,void* expected,void* replacement) noexcept {
    if(!cell||!expected||!replacement||(reinterpret_cast<uintptr_t>(cell)&7))return Redirect::refused;
    MEMORY_BASIC_INFORMATION info{};
    if(!VirtualQuery(const_cast<void**>(cell),&info,sizeof(info))||info.State!=MEM_COMMIT||
        info.Protect&(PAGE_GUARD|PAGE_NOACCESS)||
        reinterpret_cast<uintptr_t>(cell)+sizeof(void*)>reinterpret_cast<uintptr_t>(info.BaseAddress)+info.RegionSize)return Redirect::refused;
    DWORD old{},ignored{};
    if(!VirtualProtect(const_cast<void**>(cell),sizeof(void*),PAGE_READWRITE,&old))return Redirect::refused;
    auto before=InterlockedCompareExchangePointer(cell,replacement,expected);
    if(!VirtualProtect(const_cast<void**>(cell),sizeof(void*),old,&ignored))return Redirect::protection_failure;
    return before==expected?Redirect::changed:Redirect::refused;
}
}
