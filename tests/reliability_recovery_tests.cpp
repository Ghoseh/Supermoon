// SPDX-License-Identifier: MIT
#include "recovery_native.hpp"
#include "recovery_diagnostics.hpp"
#include "remediation_harness.hpp"
#include <cstring>
using namespace bytebeat::recovery;
using remediation_test::expect;using remediation_test::require;
extern "C" uint32_t RecoverySourceInit(void*,void*,void*,void*,void*);
extern "C" void RecoverySourceExecute(void*,void*);
extern "C" void RecoveryOwnerInit(void*);
extern "C" void RecoveryFree(void*,void*);
extern "C" void RecoveryTestOriginal(unsigned,void*);
extern "C" void AdapterRecoveryRetire(uintptr_t) noexcept{}
extern "C" void AdapterRecoverySource(uintptr_t,uint32_t,uint32_t,uintptr_t) noexcept{}
namespace {
unsigned mode{},native_calls{},foreign_calls{};alignas(16) unsigned char owner[128]{},foreign[64]{};
void* table[16]{},*subtable[2]{};void** context=table;void** subcontext=subtable;
void fault(){RaiseException(0xe1234567,0,0,nullptr);}
void* query(void*){if(mode==3)fault();return &subcontext;}
uint32_t id(void*){if(mode==4)fault();return 7;}
uint32_t native_init(void*,void*,void*,void*,void*){++native_calls;if(mode==5)fault();return mode==6?2:1;}
void native_execute(void*,void*){++native_calls;}
void* foreign_allocate(void*,size_t,const char*,uintptr_t){++foreign_calls;return foreign;}
Source* setup(){table[15]=reinterpret_cast<void*>(&query);subtable[1]=reinterpret_cast<void*>(&id);auto s=registry.allocate_source();require(s,"owned source");auto e=registry.begin_event(reinterpret_cast<uintptr_t>(owner)+0x28);require(e,"owned event");registry.publish(*e,7);RecoveryTestOriginal(4,reinterpret_cast<void*>(&native_init));return s;}
__declspec(noinline) unsigned guarded(void* source,void* ctx,void* params){__try {RecoverySourceInit(source,nullptr,ctx,params,nullptr);return 0;}__except(EXCEPTION_EXECUTE_HANDLER){return GetExceptionCode();}}
void exceptional(unsigned where){
    auto s=setup();auto previous=registry.allocate_source();auto previous_lease=registry.begin_event(123);invocation=previous;invocation_lease=previous_lease;
    auto region=static_cast<unsigned char*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(region,"guard allocation");DWORD old{};require(VirtualProtect(region+4096,4096,PAGE_NOACCESS,&old)!=0,"guard page");
    void* params=nullptr;void* ctx=&context;
    if(where==0)params=region+4096;
    if(where==1){params=region+4096-8;void* vtable=nullptr;memcpy(params,&vtable,8);}
    if(where==2)ctx=region+4096;
    mode=where;unsigned code=guarded(s->native,ctx,params);
    expect(code==(where<=2?EXCEPTION_ACCESS_VIOLATION:0xe1234567),"original SEH exception propagates unchanged");
    expect(invocation==previous&&invocation_lease==previous_lease,"prior TLS invocation and lease restored");
    expect(s->phase.load()==retired,"failed Init is terminal");bool latch=s->executing.test_and_set();expect(!latch,"execution latch restored");s->executing.clear();
    auto e=registry.by_id(7);expect(e&&!(e->admission.load()&count_mask),"every acquired event lease released");
    VirtualFree(region,0,MEM_RELEASE);
}
void params_vtable(){exceptional(0);}void params_gain(){exceptional(1);}void context_read(){exceptional(2);}void query_throw(){exceptional(3);}void id_throw(){exceptional(4);}void body_throw(){exceptional(5);}
void valid(){auto s=setup();alignas(16) unsigned char params[16]{};uint32_t gain=0x3f800000;memcpy(params+8,&gain,4);expect(RecoverySourceInit(s->native,nullptr,&context,params,nullptr)==1,"valid Init result retained");expect(s->phase.load()==ready&&Registry::live(s)&&!invocation&&!invocation_lease,"valid Init binds and restores TLS");expect(s->external_parameters.load()==reinterpret_cast<uintptr_t>(params)&&s->shadow_parameters.gain.load()==gain,"valid parameter mirror retained");}
void native_fail(){auto s=setup();mode=6;expect(RecoverySourceInit(s->native,nullptr,&context,nullptr,nullptr)==2&&s->phase.load()==retired,"native failed Init remains terminal");expect(!invocation&&!invocation_lease&&!(registry.by_id(7)->admission.load()&count_mask),"failed result releases state");}
void source_capacity_test(){
    RecoveryTestOriginal(0,reinterpret_cast<void*>(&foreign_allocate));void* first=nullptr;
    for(unsigned n=0;n<source_capacity;++n){auto p=allocation(nullptr,64,nullptr,0,0x31ac65c);require(p,"owned factory allocation");if(!n)first=p;}
    expect(!allocation(nullptr,64,nullptr,0,0x31ac65c)&&!foreign_calls,"source exhaustion never falls back to native allocator");RecoveryFree(nullptr,first);
    expect(!allocation(nullptr,64,nullptr,0,0x31ac65c),"retirement does not recycle source identity");auto s=registry.snapshot();expect(s.sources==source_capacity&&s.source_refused==2,"cumulative source usage/refusal snapshot");
    expect(allocation(nullptr,63,nullptr,0,0x31ac65c)==foreign&&foreign_calls==1,"unqualified size retains foreign forwarding");
}
void parameter_capacity_test(){RecoveryTestOriginal(0,reinterpret_cast<void*>(&foreign_allocate));for(unsigned n=0;n<parameter_capacity;++n)require(allocation(nullptr,16,nullptr,0,n%2?0x31ac6bc:0x31acd37),"owned parameter/Clone");expect(!allocation(nullptr,16,nullptr,0,0x31acd37)&&!foreign_calls,"parameter exhaustion refuses without fallback");auto s=registry.snapshot();expect(s.parameters==parameter_capacity&&s.parameter_refused==1,"parameter capacity telemetry");}
void owner_without_event(void*){auto s=registry.allocate_source();require(s,"source remains owned");expect(RecoverySourceInit(s->native,nullptr,&context,nullptr,nullptr)==2&&s->phase.load()==retired,"missing event refuses source initialization");}
void event_capacity_test(){table[15]=reinterpret_cast<void*>(&query);subtable[1]=reinterpret_cast<void*>(&id);for(unsigned n=0;n<event_capacity;++n)require(registry.begin_event(n+1000),"owned event");RecoveryTestOriginal(8,reinterpret_cast<void*>(&owner_without_event));RecoveryOwnerInit(owner);auto s=registry.snapshot();expect(s.events==event_capacity&&s.event_refused==1&&!native_calls&&!constructing,"event exhaustion remains bounded and custom Init cannot enter native body");}
void reused_id(){auto s=setup();s->phase.store(ready);s->id.store(7);registry.bind(*s);auto second=registry.begin_event(123);registry.publish(*second,7);expect(!Registry::live(s)&&!registry.by_id(7),"ID reuse poisons both identities without aliasing");RecoveryTestOriginal(5,reinterpret_cast<void*>(&native_execute));unsigned char buffer[32]{};RecoverySourceExecute(s->native,buffer);expect(!native_calls&&registry.snapshot().id_reuse==1,"stale Execute refuses and ID reuse remains visible");}
void diagnostics(){Registry::Snapshot s{};Reporter reporter;char out[768]{};expect(reporter.sample(s,0,out,sizeof(out))&&strstr(out,"cumulative_not_songs")&&strstr(out,"sources=0/4096"),"initial cumulative capacity row");s.sources=source_capacity;s.source_refused=1;expect(!reporter.sample(s,999,out,sizeof(out)),"writer cadence bounded");expect(reporter.sample(s,1000,out,sizeof(out))&&strstr(out,"exit_normally_restart_no_slot_reuse")&&strstr(out,"source_refused=1"),"degraded refusal and normal-restart guidance");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"borrowed_parameter_vtable_fault",params_vtable},{"borrowed_parameter_gain_fault",params_gain},{"borrowed_context_fault",context_read},{"context_query_exception",query_throw},{"context_id_exception",id_throw},{"native_init_exception",body_throw},{"valid_init",valid},{"failed_init_result",native_fail},
    {"source_factory_exhaustion",source_capacity_test},{"parameter_clone_exhaustion",parameter_capacity_test},{"owner_event_exhaustion",event_capacity_test},{"numeric_id_reuse",reused_id},{"capacity_diagnostic_row",diagnostics}};
    return remediation_test::main(argc,argv,cases);
}
