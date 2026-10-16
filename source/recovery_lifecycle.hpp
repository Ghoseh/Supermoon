// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cstddef>

namespace bytebeat::recovery {
// Process-lifetime stores cover the intercepted native factory sites, not
// just the selected Supermoon voice. Slots/counters never recycle; exhaustion
// or repeated reserve contention refuses new work instead of aliasing old tickets.
constexpr uint32_t source_capacity=4096,parameter_capacity=8192,event_capacity=4096;
constexpr uint64_t closed_bit=uint64_t(1)<<63,count_mask=~closed_bit;
enum Phase:uint32_t {unused,allocated,initializing,ready,retired};
struct Source;
struct Event {
    // A unique append-only record for one observed native PostEvent lifecycle.
    // Stream addresses locate current CONTROL state, never callback identity.
    // This association does not prove closure of upstream native delayed work.
    std::atomic<uint64_t> admission{closed_bit};
    std::atomic<uint32_t> id{0},published{0},poisoned{0},terminal{0},prepared{0};
    uintptr_t stream{};
    std::atomic<Source*> endpoint{nullptr};
};
struct Interface {void** vtable{};Source* source{};};
struct alignas(16) ParameterMirror {void* vtable{};std::atomic<uint32_t> gain{0};uint32_t padding{};};
static_assert(offsetof(ParameterMirror,gain)==8&&sizeof(ParameterMirror)==16);
struct alignas(64) Source {
    alignas(16) unsigned char native[64]{};
    Interface context{},subcontext{};
    std::atomic<uint32_t> phase{unused},id{0};
    std::atomic<Event*> event{nullptr};
    std::atomic_flag executing=ATOMIC_FLAG_INIT;
    std::atomic<uintptr_t> external_parameters{0};
    ParameterMirror shadow_parameters{};
};
struct alignas(16) Parameter {unsigned char native[16]{};std::atomic<bool> retired{false};};
static_assert(offsetof(Source,native)==0&&offsetof(Parameter,native)==0);
struct Registry {
    Source sources[source_capacity];Parameter parameters[parameter_capacity];Event events[event_capacity];
    std::atomic<uint32_t> source_count{0},parameter_count{0},event_count{0};
    std::atomic<uint64_t> refused{0},id_reuse{0},late_calls{0},overlaps{0},cleanup_waits{0};
    uint32_t reserve(std::atomic<uint32_t>& counter,uint32_t limit) noexcept {
        auto n=counter.load(std::memory_order_acquire);
        for(unsigned attempt=0;attempt<8;++attempt){if(n>=limit)break;if(counter.compare_exchange_weak(n,n+1,std::memory_order_acq_rel))return n;}
        refused.fetch_add(1);return UINT32_MAX;
    }
    Source* allocate_source() noexcept {
        auto n=reserve(source_count,source_capacity);if(n==UINT32_MAX)return nullptr;
        auto& s=sources[n];s.phase.store(allocated,std::memory_order_release);return &s;
    }
    Parameter* allocate_parameter() noexcept {
        auto n=reserve(parameter_count,parameter_capacity);if(n==UINT32_MAX)return nullptr;return &parameters[n];
    }
    Source* source(void* p) noexcept {
        auto a=reinterpret_cast<uintptr_t>(p),base=reinterpret_cast<uintptr_t>(sources);
        if(a<base||a-base>=sizeof(sources)||(a-base)%sizeof(Source))return nullptr;
        auto n=(a-base)/sizeof(Source);return n<source_count.load(std::memory_order_acquire)?&sources[n]:nullptr;
    }
    Parameter* parameter(void* p) noexcept {
        auto a=reinterpret_cast<uintptr_t>(p),base=reinterpret_cast<uintptr_t>(parameters);
        if(a<base||a-base>=sizeof(parameters)||(a-base)%sizeof(Parameter))return nullptr;
        auto n=(a-base)/sizeof(Parameter);return n<parameter_count.load(std::memory_order_acquire)?&parameters[n]:nullptr;
    }
    Event* begin_event(uintptr_t stream) noexcept {
        auto n=reserve(event_count,event_capacity);if(n==UINT32_MAX)return nullptr;
        auto& e=events[n];e.stream=stream;e.prepared.store(1,std::memory_order_release);return &e;
    }
    static bool open(const Event* e) noexcept {return e&&e->published.load(std::memory_order_acquire)&&!e->terminal.load(std::memory_order_acquire)&&!(e->admission.load(std::memory_order_acquire)&closed_bit)&&!e->poisoned.load(std::memory_order_acquire);}
    static bool live(const Source* s) noexcept {return s&&s->phase.load(std::memory_order_acquire)==ready&&open(s->event.load(std::memory_order_acquire));}
    Event* by_id(uint32_t id) noexcept {
        if(!id)return nullptr;auto end=event_count.load(std::memory_order_acquire);if(end>event_capacity)end=event_capacity;
        Event* result=nullptr;
        for(uint32_t n=0;n<end;++n){auto& e=events[n];if(e.published.load(std::memory_order_acquire)&&e.id.load(std::memory_order_relaxed)==id){if(result)return nullptr;result=&e;}}
        return result;
    }
    Event* control(uintptr_t stream) noexcept {
        auto end=event_count.load(std::memory_order_acquire);if(end>event_capacity)end=event_capacity;
        while(end){auto& e=events[--end];if(e.published.load(std::memory_order_acquire)&&e.stream==stream&&open(&e))return &e;}return nullptr;
    }
    void bind(Source& s) noexcept {
        if(s.phase.load(std::memory_order_acquire)!=ready)return;
        auto e=by_id(s.id.load(std::memory_order_acquire));if(!open(e))return;
        Source* old=e->endpoint.load(std::memory_order_acquire);
        // One source claim per observed event. A second source sharing its
        // numeric ID is ambiguous, even after the first source retired.
        if(old&&old!=&s){e->poisoned.store(1);e->admission.fetch_or(closed_bit);refused.fetch_add(1);return;}
        if(s.event.load(std::memory_order_acquire)&&s.event.load()!=e){s.phase.store(retired);refused.fetch_add(1);return;}
        s.event.store(e,std::memory_order_release);
        if(!e->endpoint.compare_exchange_strong(old,&s,std::memory_order_acq_rel)&&old!=&s){e->poisoned.store(1);e->admission.fetch_or(closed_bit);refused.fetch_add(1);}
    }
    void publish(Event& e,uint32_t id) noexcept {
        if(!id){e.poisoned.store(1);e.published.store(1,std::memory_order_release);return;}
        e.id.store(id,std::memory_order_relaxed);e.admission.store(0,std::memory_order_release);e.published.store(1,std::memory_order_release);
        // Reused numeric IDs are permanently ambiguous for this process.
        // Never recycle the old row or infer a fresh incarnation from the value.
        auto end=event_count.load(std::memory_order_acquire);if(end>event_capacity)end=event_capacity;
        for(uint32_t n=0;n<end;++n){auto& previous=events[n];if(&previous!=&e&&previous.published.load(std::memory_order_acquire)&&previous.id.load()==id){previous.poisoned.store(1);previous.admission.fetch_or(closed_bit);e.poisoned.store(1);e.admission.fetch_or(closed_bit);id_reuse.fetch_add(1);}}
        end=source_count.load(std::memory_order_acquire);if(end>source_capacity)end=source_capacity;
        for(uint32_t n=0;n<end;++n)if(sources[n].id.load(std::memory_order_acquire)==id)bind(sources[n]);
    }
    bool acquire_event(Event* held) noexcept {
        if(!open(held)){late_calls.fetch_add(1);return false;}
        auto state=held->admission.load(std::memory_order_acquire);
        for(unsigned n=0;n<8;++n){if((state&closed_bit)||(state&count_mask)==count_mask)return false;if(held->admission.compare_exchange_weak(state,state+1,std::memory_order_acq_rel)){
            if(!held->poisoned.load(std::memory_order_acquire)&&!held->terminal.load(std::memory_order_acquire))return true;
            held->admission.fetch_sub(1,std::memory_order_release);return false;}}
        refused.fetch_add(1);return false; // Audio path never waits/spins indefinitely.
    }
    bool acquire(Source& s,Event*& held) noexcept {
        held=s.event.load(std::memory_order_acquire);if(!live(&s)){late_calls.fetch_add(1);return false;}
        if(!acquire_event(held))return false;
        if(s.phase.load(std::memory_order_acquire)==ready)return true;
        held->admission.fetch_sub(1,std::memory_order_release);return false;
    }
    static void release(Event* e) noexcept {if(e)e->admission.fetch_sub(1,std::memory_order_release);}
    static void close(Event* e) noexcept {if(e){e->terminal.store(1,std::memory_order_release);e->admission.fetch_or(closed_bit,std::memory_order_acq_rel);}}
    void mirror(void* parameter,uint32_t bits) noexcept {
        auto end=source_count.load(std::memory_order_acquire);if(end>source_capacity)end=source_capacity;
        for(uint32_t n=0;n<end;++n){auto& s=sources[n];if(s.phase.load(std::memory_order_acquire)!=retired&&s.external_parameters.load(std::memory_order_acquire)==reinterpret_cast<uintptr_t>(parameter)){
            // Native parameter writes/readbacks are naturally aligned dwords.
            // The shadow stays resident even after its original native object dies.
            s.shadow_parameters.gain.store(bits,std::memory_order_release);
        }}
    }
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(sizeof(Registry)<4*1024*1024);
extern Registry registry;
extern thread_local Source* invocation;
extern thread_local Event* constructing;
extern thread_local Event* invocation_lease;
inline Source* ticket(uintptr_t stream) noexcept {auto s=invocation;auto e=s?s->event.load(std::memory_order_acquire):nullptr;return Registry::live(s)&&e&&e==invocation_lease&&e->stream==stream?s:nullptr;}
}
