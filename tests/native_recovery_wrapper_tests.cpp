// SPDX-License-Identifier: MIT
// Links actual recovery_native.cpp; no copied native code or real native objects.
#include "recovery_native.hpp"
#include "remediation_harness.hpp"
using namespace bytebeat::recovery;
using remediation_test::expect;
using remediation_test::require;
extern "C" void RecoverySourceExecute(void*,void*);
extern "C" uint32_t RecoverySourceTerm(void*,void*);
extern "C" void RecoveryOwnerCleanup(void*);
extern "C" void RecoveryTestOriginal(unsigned,void*);
extern "C" void AdapterRecoveryRetire(uintptr_t) noexcept{}
extern "C" void AdapterRecoverySource(uintptr_t,uint32_t,uint32_t,uintptr_t) noexcept{}
namespace {
alignas(16) unsigned char owner[128]{};
unsigned executed{},cleaned{};
void cleanup(void*){++cleaned;}
void execute(void*,void*){++executed;expect(invocation&&invocation_lease,"Execute holds actual TLS and lease");}
void reentrant(void*,void*){std::puts("REACHED owned admitted Execute calling same-owner cleanup");std::fflush(stdout);RecoveryOwnerCleanup(owner);}
void fault(void*,void*){RaiseException(0xe1234567,0,0,nullptr);}
unsigned guarded_execute(void* source){__try {RecoverySourceExecute(source,nullptr);return 0;}__except(EXCEPTION_EXECUTE_HANDLER){return GetExceptionCode();}}
Source* setup(){auto s=registry.allocate_source();require(s,"owned source allocation");auto e=registry.begin_event(reinterpret_cast<uintptr_t>(owner)+0x28);require(e,"owned event allocation");s->id.store(7);s->phase.store(ready);registry.publish(*e,7);registry.bind(*s);RecoveryTestOriginal(5,reinterpret_cast<void*>(&execute));RecoveryTestOriginal(9,reinterpret_cast<void*>(&cleanup));return s;}
void normal(){auto s=setup();RecoverySourceExecute(s->native,nullptr);expect(executed==1&&!invocation&&!invocation_lease,"Execute restores TLS");expect((s->event.load()->admission.load()&count_mask)==0,"Execute releases admission");RecoveryOwnerCleanup(owner);expect(cleaned==1&&!Registry::live(s),"owner cleanup drains and calls original");unsigned char buffer[32]{};RecoverySourceExecute(s->native,buffer);expect(executed==1,"late Execute never calls native original");uint32_t state{};memcpy(&state,buffer+0xc,4);expect(state==2,"late Execute returns stopped buffer");}
void retired_source(){auto s=setup();expect(RecoverySourceTerm(s->native,nullptr)==1,"Term wrapper succeeds");RecoverySourceExecute(s->native,nullptr);expect(!executed&&!Registry::live(s),"retired ticket cannot execute");}
void exception(){auto s=setup();RecoveryTestOriginal(5,reinterpret_cast<void*>(&fault));expect(guarded_execute(s->native)==0xe1234567,"original Execute exception propagated");expect(!invocation&&!invocation_lease&&(s->event.load()->admission.load()&count_mask)==0,"exception finally restores TLS and releases admission");RecoveryTestOriginal(5,reinterpret_cast<void*>(&execute));RecoverySourceExecute(s->native,nullptr);expect(executed==1,"exception clears execution latch");}
void self_wait(){auto s=setup();RecoveryTestOriginal(5,reinterpret_cast<void*>(&reentrant));RecoverySourceExecute(s->native,nullptr);expect(cleaned==1,"same-invocation cleanup completes safely (A1-H1 conditional schedule)");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"execute_cleanup_late_callback",normal},{"retired_source_late_execute",retired_source},
    {"execute_exception_finally",exception},{"same_invocation_cleanup_liveness",self_wait}};
    return remediation_test::main(argc,argv,cases);
}
