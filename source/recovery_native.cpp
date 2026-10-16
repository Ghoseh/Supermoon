// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "recovery_native.hpp"
#include <intrin.h>
#include <initializer_list>
namespace bytebeat::recovery {
Registry registry;thread_local Source* invocation{};thread_local Event* constructing{};thread_local Event* invocation_lease{};
namespace {
uintptr_t image{};void* original[15]{};std::atomic<bool> active{false};
void* context_vtable[16]{};void* sub_vtable[2]{};
constexpr uint32_t cells[]={0x35a97f8,0x35a9800,0x3595798,0x35957a0,0x35957c8,0x35957f8,0x6e06308,0x6e06300,0x4ab8a78,0x4ab8a80,0x3595870,0x3595890,0x3595878,0x3595888,0x3595898};
constexpr uint32_t targets[]={0x321d590,0x321d5d0,0x3134ea0,0x2e37650,0x3135480,0x3134f70,0x2e2f750,0x2e2f6d0,0xf6b8f0,0xf6c1b0,0x3135570,0x2e37650,0x3135610,0x31355f0,0x3135650};
template<class T>T fn(unsigned n) noexcept {T f{};memcpy(&f,&original[n],sizeof(f));return f;}
template<class T>T read(const void* p,size_t off) noexcept {T v;memcpy(&v,static_cast<const unsigned char*>(p)+off,sizeof(v));return v;}
void stop_buffer(void* buffer) noexcept {if(buffer){uint16_t zero=0;uint32_t state=2;memcpy(static_cast<unsigned char*>(buffer)+0x12,&zero,2);memcpy(static_cast<unsigned char*>(buffer)+0xc,&state,4);}}
void retire(Source* s) noexcept {if(s&&s->phase.exchange(retired,std::memory_order_acq_rel)!=retired){auto e=s->event.load(std::memory_order_acquire);AdapterRecoverySource(reinterpret_cast<uintptr_t>(s),s->id.load(),retired,e?e->stream:0);AdapterRecoveryRetire(reinterpret_cast<uintptr_t>(s));}}
void* cached_context(void* object) noexcept {auto& s=*static_cast<Interface*>(object)->source;return &s.subcontext;}
uint32_t cached_id(void* object) noexcept {auto& s=*static_cast<Interface*>(object)->source;return s.phase.load(std::memory_order_acquire)==retired?0:s.id.load(std::memory_order_acquire);}
void close_and_drain(Event* e) noexcept {
    if(!e)return;Registry::close(e);
    // This is the native OWNER CLEANUP/INIT control path, never Execute,
    // Format, a frame helper, source Term or DllMain. The admission is ours:
    // admitted Source scopes include their lookup->provider gap. Closing
    // prevents further admission through that event and delays our original
    // cleanup call until those scopes leave; it does not drain all upstream NMS
    // work. U1 lock/phase assumptions remain unproven; this wait has no timeout.
    if((e->admission.load(std::memory_order_acquire)&count_mask)!=0)registry.cleanup_waits.fetch_add(1);
    while(e->admission.load(std::memory_order_acquire)&count_mask)Sleep(1);
    if(auto s=e->endpoint.load(std::memory_order_acquire))retire(s);
}
void close_stream(uintptr_t stream) noexcept {
    auto end=registry.event_count.load(std::memory_order_acquire);if(end>event_capacity)end=event_capacity;
    for(uint32_t n=0;n<end;++n){auto& e=registry.events[n];if(e.prepared.load(std::memory_order_acquire)&&e.stream==stream)close_and_drain(&e);}
}
void mirror(void* p) noexcept {if(p)registry.mirror(p,read<uint32_t>(p,8));}
}
extern "C" __declspec(noinline) void* RecoveryAllocate(void* allocator,size_t bytes,const char* file,uintptr_t line) {
    return allocation(allocator,bytes,file,line,reinterpret_cast<uintptr_t>(_ReturnAddress()));
}
// Only the pinned factory/Clone callers and exact sizes receive owned slots.
// Other allocation/Free paths keep the original allocator ownership.
void* allocation(void* allocator,size_t bytes,const char* file,uintptr_t line,uintptr_t caller) {
    if(caller==image+0x3134eec&&bytes==64){auto s=registry.allocate_source();return s?static_cast<void*>(s->native):nullptr;}
    if((caller==image+0x3134f4c||caller==image+0x31355c7)&&bytes==16){auto p=registry.allocate_parameter();return p?static_cast<void*>(p->native):nullptr;}
    return fn<void*(*)(void*,size_t,const char*,uintptr_t)>(0)(allocator,bytes,file,line);
}
extern "C" void RecoveryFree(void* allocator,void* object) {
    if(auto s=registry.source(object)){retire(s);return;}
    if(auto p=registry.parameter(object)){p->retired.store(true,std::memory_order_release);return;}
    fn<void(*)(void*,void*)>(1)(allocator,object);
}
extern "C" void* RecoverySourceDestructor(void* object,uint32_t flags) {
    if(auto s=registry.source(object)){retire(s);return object;}
    return fn<void*(*)(void*,uint32_t)>(2)(object,flags);
}
extern "C" uint32_t RecoverySourceTerm(void* object,void* allocator) {
    if(auto s=registry.source(object)){retire(s);return 1;}
    return fn<uint32_t(*)(void*,void*)>(3)(object,allocator);
}
extern "C" uint32_t RecoverySourceInit(void* object,void* allocator,void* context,void* params,void* format) {
    auto s=registry.source(object);if(!s)return fn<uint32_t(*)(void*,void*,void*,void*,void*)>(4)(object,allocator,context,params,format);
    if(s->executing.test_and_set(std::memory_order_acquire)){registry.refused.fetch_add(1);return 2;}
    auto phase=s->phase.load(std::memory_order_acquire);
    if((phase!=allocated&&phase!=ready)||!s->phase.compare_exchange_strong(phase,initializing,std::memory_order_acq_rel)){s->executing.clear(std::memory_order_release);registry.refused.fetch_add(1);return 2;}
    auto old_id=s->id.load(std::memory_order_acquire);auto previous=invocation;auto previous_lease=invocation_lease;Event* init_lease=nullptr;invocation=s;invocation_lease=nullptr;uint32_t result=2;bool complete=false;
    // Borrowed parameters must be valid here. Accepted audit C2: these loads
    // precede __try, so a synthetic invalid parameter fault bypasses its cleanup.
    // No valid-caller occurrence was established; behavior is unchanged in v1.0.
    void* stable_params=params;
    if(params&&!registry.parameter(params)){
        s->shadow_parameters.vtable=read<void*>(params,0);s->shadow_parameters.gain.store(read<uint32_t>(params,8),std::memory_order_relaxed);
        s->external_parameters.store(reinterpret_cast<uintptr_t>(params),std::memory_order_release);stable_params=&s->shadow_parameters;
    }
    __try {
        // Borrow the native context ONLY in Init's caller-live scope. The
        // subsequent original Init/Execute/gain bodies query owned interfaces.
        if(context){
            auto query=reinterpret_cast<void*(*)(void*)>(read<void**>(context,0)[15]);auto sub=query(context);
            auto get_id=sub?reinterpret_cast<uint32_t(*)(void*)>(read<void**>(sub,0)[1]):nullptr;
            auto id=get_id?get_id(sub):0;auto event=registry.by_id(id);
            if(id&&(!old_id||old_id==id)&&registry.acquire_event(event)){
                init_lease=event;invocation_lease=event;s->id.store(id,std::memory_order_release);s->event.store(event,std::memory_order_release);
                s->context={context_vtable,s};s->subcontext={sub_vtable,s};
                result=fn<uint32_t(*)(void*,void*,void*,void*,void*)>(4)(object,allocator,&s->context,stable_params,format);
                uint32_t expected=initializing;if(result==1&&s->phase.compare_exchange_strong(expected,ready,std::memory_order_acq_rel)){registry.bind(*s);AdapterRecoverySource(reinterpret_cast<uintptr_t>(s),id,ready,event->stream);}else retire(s);
            }else retire(s);
        }else retire(s);
        complete=true;
    } __finally {if(!complete)retire(s);invocation=previous;invocation_lease=previous_lease;Registry::release(init_lease);s->executing.clear(std::memory_order_release);}
    return result;
}
extern "C" void RecoveryFormat(uint32_t id,void* format) {
    if(invocation&&!invocation_lease)return;
    fn<void(*)(uint32_t,void*)>(6)(id,format);
}
extern "C" void RecoveryGlobalExecute(uint32_t id,void* buffer) {
    if(invocation&&(!invocation_lease||invocation->phase.load(std::memory_order_acquire)==retired||!Registry::open(invocation_lease))){stop_buffer(buffer);return;}
    fn<void(*)(uint32_t,void*)>(7)(id,buffer);
}
extern "C" void RecoverySourceExecute(void* object,void* buffer) {
    auto s=registry.source(object);if(!s){fn<void(*)(void*,void*)>(5)(object,buffer);return;}
    if(s->phase.load(std::memory_order_acquire)!=ready){registry.late_calls.fetch_add(1);stop_buffer(buffer);return;}
    if(s->executing.test_and_set(std::memory_order_acquire)){registry.overlaps.fetch_add(1);stop_buffer(buffer);return;}
    Event* held=nullptr;auto e=s->event.load(std::memory_order_acquire);
    if(!e||!registry.acquire(*s,held)){s->executing.clear(std::memory_order_release);stop_buffer(buffer);return;}
    auto previous=invocation;auto previous_lease=invocation_lease;invocation=s;invocation_lease=held;
    __try {if(s->phase.load(std::memory_order_acquire)==ready)fn<void(*)(void*,void*)>(5)(object,buffer);else stop_buffer(buffer);}
    __finally {invocation=previous;invocation_lease=previous_lease;Registry::release(held);s->executing.clear(std::memory_order_release);}
}
extern "C" void RecoveryOwnerInit(void* object) {
    auto stream=reinterpret_cast<uintptr_t>(object)+0x28;close_stream(stream);auto previous=constructing;constructing=registry.begin_event(stream);
    __try {fn<void(*)(void*)>(8)(object);}
    __finally {if(AbnormalTermination())close_and_drain(constructing);constructing=previous;}
}
extern "C" void RecoveryOwnerCleanup(void* object) {close_stream(reinterpret_cast<uintptr_t>(object)+0x28);fn<void(*)(void*)>(9)(object);}
extern "C" void* RecoveryParameterDestructor(void* object,uint32_t flags) {
    if(auto p=registry.parameter(object)){p->retired.store(true,std::memory_order_release);return object;}
    return fn<void*(*)(void*,uint32_t)>(10)(object,flags);
}
extern "C" uint32_t RecoveryParameterTerm(void* object,void* allocator) {
    if(auto p=registry.parameter(object)){p->retired.store(true,std::memory_order_release);return 1;}
    return fn<uint32_t(*)(void*,void*)>(11)(object,allocator);
}
extern "C" uint32_t RecoveryParameterSet(void* object,uint32_t id,void* value,uint32_t size) {
    if(auto p=registry.parameter(object);p&&p->retired.load(std::memory_order_acquire))return 0x1f;
    auto result=fn<uint32_t(*)(void*,uint32_t,void*,uint32_t)>(12)(object,id,value,size);mirror(object);return result;
}
extern "C" uint32_t RecoveryParameterInit(void* object,void* allocator,void* data,uint32_t size) {
    if(auto p=registry.parameter(object);p&&p->retired.load(std::memory_order_acquire))return 0x1f;
    auto result=fn<uint32_t(*)(void*,void*,void*,uint32_t)>(13)(object,allocator,data,size);mirror(object);return result;
}
extern "C" uint32_t RecoveryParameterBlock(void* object,void* data,uint32_t size) {
    if(auto p=registry.parameter(object);p&&p->retired.load(std::memory_order_acquire))return 0x1f;
    auto result=fn<uint32_t(*)(void*,void*,uint32_t)>(14)(object,data,size);mirror(object);return result;
}
void initialize(uintptr_t base,void* const* clones) noexcept {
    image=base;for(unsigned n=0;n<15;++n)original[n]=reinterpret_cast<void*>(base+targets[n]);
    original[4]=clones[9];original[5]=clones[10];original[8]=clones[7];original[9]=clones[4];context_vtable[15]=reinterpret_cast<void*>(&cached_context);sub_vtable[1]=reinterpret_cast<void*>(&cached_id);
    // Initialization is off callback and precedes every pool publication.
    auto pages=reinterpret_cast<volatile unsigned char*>(&registry);for(size_t n=0;n<sizeof(registry);n+=4096){auto byte=pages[n];pages[n]=byte;}
}
bool validate(unsigned char* base,HMODULE module) noexcept {
    for(unsigned n=0;n<15;++n){MEMORY_BASIC_INFORMATION i{};auto cell=base+cells[n];if(!VirtualQuery(cell,&i,sizeof(i))||i.State!=MEM_COMMIT||i.Type!=MEM_IMAGE||i.AllocationBase!=module||(i.Protect&(PAGE_GUARD|PAGE_NOACCESS))||*reinterpret_cast<void**>(cell)!=base+targets[n])return false;}return true;
}
bool publish(unsigned char* base) noexcept {
    void* replacements[]={reinterpret_cast<void*>(&RecoveryAllocate),reinterpret_cast<void*>(&RecoveryFree),reinterpret_cast<void*>(&RecoverySourceDestructor),reinterpret_cast<void*>(&RecoverySourceTerm),reinterpret_cast<void*>(&RecoverySourceInit),reinterpret_cast<void*>(&RecoverySourceExecute),reinterpret_cast<void*>(&RecoveryFormat),reinterpret_cast<void*>(&RecoveryGlobalExecute),reinterpret_cast<void*>(&RecoveryOwnerInit),reinterpret_cast<void*>(&RecoveryOwnerCleanup),reinterpret_cast<void*>(&RecoveryParameterDestructor),reinterpret_cast<void*>(&RecoveryParameterTerm),reinterpret_cast<void*>(&RecoveryParameterSet),reinterpret_cast<void*>(&RecoveryParameterInit),reinterpret_cast<void*>(&RecoveryParameterBlock)};
    // Publish owning allocation LAST: every possible owned destruction route
    // must already be hooked, with its page protection restored. Partial failure
    // before this point keeps genuine native allocation/foreign forwarding.
    // If the allocator exchange succeeds but its own restore fails, all dependent
    // wrappers already exist. Return false and retain residency; do not roll back
    // their ownership routes or treat failed activation as safe to hot-retry.
    for(unsigned n:{9u,1u,2u,3u,4u,5u,6u,7u,10u,11u,12u,13u,14u,8u,0u}){auto cell=reinterpret_cast<void* volatile*>(base+cells[n]);DWORD old{},ignored{};if(!VirtualProtect(const_cast<void**>(cell),8,PAGE_READWRITE,&old))return false;bool ok=InterlockedCompareExchangePointer(cell,replacements[n],base+targets[n])==base+targets[n];bool restored=VirtualProtect(const_cast<void**>(cell),8,old,&ignored)!=0;if(!ok||!restored)return false;}return true;
}
void retire_stream(uintptr_t stream) noexcept {if(auto e=registry.control(stream)){Registry::close(e);if(auto s=e->endpoint.load(std::memory_order_acquire))retire(s);}}
Source* control_endpoint(uintptr_t stream) noexcept {auto e=registry.control(stream);auto s=e?e->endpoint.load(std::memory_order_acquire):nullptr;return Registry::live(s)?s:nullptr;}
bool enabled() noexcept{return active.load(std::memory_order_acquire);}void set_enabled() noexcept{active.store(true,std::memory_order_release);}
}
extern "C" void RecoveryPosted(uint32_t id) noexcept {auto saved=GetLastError();if(auto e=bytebeat::recovery::constructing)bytebeat::recovery::registry.publish(*e,id);SetLastError(saved);}
#ifdef RECOVERY_SELFTEST_BUILD
extern "C" void RecoveryTestOriginal(unsigned index,void* value){bytebeat::recovery::original[index]=value;}
#endif
