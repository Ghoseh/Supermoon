// SPDX-License-Identifier: MIT
// Fixed-behavior assertions against the actual headers. Each case is a new process.
#include "music_control_prototype.hpp"
#include "remediation_harness.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
using namespace supermoon::floor_prototype;
using remediation_test::expect;
using remediation_test::require;
namespace {
constexpr float neutral=-48.f,suppressing=-35.f;
MusicControl* active{};
bool ready=true,live[PlaybackIntent::capacity]{true,true,true};
bool sdk_contract=true,compensated_new_commit=false;
unsigned calls{},fail_next{},action{};
float applied=neutral;
HANDLE setter_entered{},stop_completed{};
// Application is deliberately recorded AFTER the injected interleaving. A failed
// SDK call never updates applied. Logical intent is always sampled separately.
bool initialized(){return ready;}
bool alive(uint32_t ticket) noexcept {return ticket<PlaybackIntent::capacity&&live[ticket];}
int setter(uint32_t id,float value,uint64_t object,int32_t ms,uint32_t curve,bool bypass) {
    ++calls;
    sdk_contract&=id==658155624&&object==UINT64_MAX&&curve==4&&!bypass&&
        ((value==suppressing&&ms==150)||(value==neutral&&(ms==750||ms==0)));
    if(value==suppressing&&action>=1&&action<=4) {
        auto a=action;action=a==4?5:0;
        if(a==1||a==4)expect(active->intent.observe(0,active->intent.capture(0),0,1),"Stop accepted inside setter");
        if(a==2)active->intent.retire(0);
        if(a==3)expect(active->intent.commit(1,2,false),"new native commit accepted inside setter");
        active->pump(); // Deterministic skipped nested pump; no sleep or timing luck.
    } else if(value==neutral&&action==5) {
        action=0;compensated_new_commit=active->intent.commit(1,2,true);
        active->pump();
    } else if(value==suppressing&&action==6) {
        action=0;SetEvent(setter_entered);
        expect(WaitForSingleObject(stop_completed,2000)==WAIT_OBJECT_0,"latched concurrent Stop completes before SDK apply");
    }
    const int result=fail_next?(--fail_next,2):1;
    if(result==1)applied=value;
    std::printf("SDK call=%u requested=%.0f result=%d applied=%.0f logical=%d\n",
                calls,value,result,applied,active->intent.suppress());
    return result;
}
struct Fixture {
    MusicControl control;
    Fixture(){active=&control;control.prepare(658155624,setter,initialized,alive,nullptr);control.enable();}
    void play(uint32_t ticket=0,uint64_t generation=1){require(control.intent.commit(ticket,generation,true),"initial validated commit");}
    void pump(){control.pump();expect(sdk_contract,"SDK packet contract");}
    void state(bool logical,float parameter,const char* message){
        std::printf("STATE logical=%d applied=%.0f expected_logical=%d expected_applied=%.0f\n",control.intent.suppress(),applied,logical,parameter);
        expect(control.intent.suppress()==logical&&applied==parameter,message);
    }
    void stop(){require(control.intent.observe(0,control.intent.capture(0),0,1),"Stop observation");}
    void fresh(){expect(control.intent.commit(1,2,true),"A1-M1 fresh validated commit recovers");pump();state(true,suppressing,"fresh playback has logical intent AND applied suppression");}
};
void initial_not_ready(){Fixture f;ready=false;f.pump();expect(calls==0,"unusable SDK not called");ready=true;f.play();f.pump();f.state(true,suppressing,"first usable engine accepts Play");}
void engine_loss(){Fixture f;f.play();f.pump();ready=false;f.pump();expect(calls==1,"transient uninitialized engine is not called");expect(!f.control.intent.suppress(),"old playback intent invalidated on engine loss");ready=true;f.pump();f.state(false,neutral,"old request does not resurrect after reset");f.fresh();}
void failed_enable(){Fixture f;f.play();fail_next=1;f.pump();f.state(false,neutral,"failed enable receives successful neutral fallback");f.fresh();}
void failed_release(){Fixture f;f.play();f.pump();f.stop();fail_next=1;f.pump();f.state(false,suppressing,"failed release is NOT recorded as applied neutral");const auto before=calls;f.pump();expect(calls==before+1,"next explicit control opportunity retries failed release");f.state(false,neutral,"release repaired");f.fresh();}
void both_fail(){Fixture f;f.play();fail_next=2;f.pump();expect(calls==2,"enable and neutral fallback both attempted and failed");f.state(false,neutral,"failed commands preserve last successful parameter");const auto before=calls;f.pump();expect(calls==before+1,"failed fallback remains actionable at next opportunity");f.state(false,neutral,"later neutral success without stale re-enable");f.fresh();}
void race(unsigned mode){Fixture f;f.play();action=mode;f.pump();f.state(false,neutral,"A1-M2 obsolete enable must be neutral at OUTER pump return");expect(calls<=8,"compensation work bounded");}
void stop_race(){race(1);}
void retirement_race(){race(2);}
void native_race(){race(3);}
void threaded_stop(){Fixture f;f.play();
    setter_entered=CreateEventW(nullptr,TRUE,FALSE,nullptr);stop_completed=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    require(setter_entered&&stop_completed,"owned scheduling events");
    action=6;std::thread stop([]{
        if(WaitForSingleObject(setter_entered,2000)==WAIT_OBJECT_0){
            expect(active->intent.observe(0,active->intent.capture(0),0,1),"concurrent Stop accepted");
            active->pump();SetEvent(stop_completed);
        }
    });
    f.pump();stop.join();CloseHandle(setter_entered);CloseHandle(stop_completed);
    f.state(false,neutral,"A1-M2 concurrent Stop cannot finish with obsolete suppression");
}
void newer_during_compensation(){Fixture f;f.play();action=4;f.pump();expect(compensated_new_commit,"new custom commit actually delivered during neutral compensation");expect(f.control.intent.suppress(),"compensation preserves newer custom intent");f.pump();f.state(true,suppressing,"new custom generation survives repair and reaches SDK");expect(calls<=8,"bounded repair with new commit");}
void later_repair(){Fixture f;f.play();action=1;f.pump();f.pump();f.state(false,neutral,"documented later independent pump repairs stale Stop");}
void stale_callback(){Fixture f;f.play();f.pump();auto old=f.control.intent.capture(0);require(f.control.intent.commit(1,2,true),"new source commit");expect(!f.control.intent.observe(0,old,0,1),"old provider Stop rejected");f.pump();f.state(true,suppressing,"stale callback cannot release newer source");f.control.intent.retire(0);expect(!f.control.intent.commit(0,3,true),"retired ticket cannot restart");f.pump();f.state(true,suppressing,"retiring old ticket cannot release current source");}
void same_ticket_stale(){Fixture f;f.play();const auto old=f.control.intent.capture(0);require(f.control.intent.commit(0,2,true),"new same-ticket generation");expect(!f.control.intent.observe(0,old,0,1),"old same-ticket Stop cannot clear newer commit");f.pump();f.state(true,suppressing,"new same-ticket generation stays active");}
void terminal_rejection(){Fixture f;f.play();f.pump();expect(!f.control.intent.commit(PlaybackIntent::capacity,2,true),"invalid identity refused");f.pump();f.state(false,neutral,"terminal identity invalidates suppression");expect(!f.control.intent.commit(1,3,true),"terminal identity remains closed");f.pump();f.state(false,neutral,"no terminal resurrection");}
void nominal(){Fixture f;f.play();f.pump();f.state(true,suppressing,"nominal Play");f.stop();f.pump();f.state(false,neutral,"nominal Stop");require(f.control.intent.commit(1,2,true),"second Play");f.pump();require(f.control.intent.commit(1,3,false),"native composition");f.pump();f.state(false,neutral,"nominal native switch");}
void live_loss(){Fixture f;f.play();f.pump();live[0]=false;f.pump();expect(applied==neutral,"liveness false neutralizes applied audio even with an active logical word");}
void quiet_carrier(){Fixture f;f.play();f.pump();for(unsigned i=0;i<1000;++i)f.control.pump();f.state(true,suppressing,"quiet/EOF/refill does not imply native Stop; controller has no amplitude input");expect(calls==1,"unchanged custom carrier deduplicated");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"initial_engine_not_ready",initial_not_ready},{"transient_engine_loss",engine_loss},
    {"failed_enable_successful_fallback",failed_enable},{"failed_release_then_fresh_play",failed_release},
    {"failed_enable_and_fallback",both_fail},{"stop_inside_setter",stop_race},
    {"retire_inside_setter",retirement_race},{"native_commit_inside_setter",native_race},
    {"latched_concurrent_stop",threaded_stop},
    {"new_custom_during_compensation",newer_during_compensation},{"later_independent_pump_repairs",later_repair},
    {"stale_provider_and_retired_ticket",stale_callback},{"same_ticket_stale_observation",same_ticket_stale},
    {"terminal_identity_stays_closed",terminal_rejection},{"nominal_play_stop_native",nominal},
    {"live_loss_with_control_opportunity",live_loss},{"quiet_carrier_intent",quiet_carrier}};
    return remediation_test::main(argc,argv,cases);
}
