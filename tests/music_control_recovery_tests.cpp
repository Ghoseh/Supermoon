// SPDX-License-Identifier: MIT
// Additional Phase 1 schedules; original Phase 0 regression files stay unchanged.
#include "music_control_prototype.hpp"
#include "remediation_harness.hpp"
using supermoon::floor_prototype::MusicControl;
using supermoon::floor_prototype::PlaybackIntent;
using remediation_test::expect;
using remediation_test::require;
namespace {
MusicControl* running{};
bool ready=true,live=true;
unsigned calls{},action{};
float applied=-48.f;
uint64_t generation=1;
enum Action {none,new_custom_failed_enable,stop_failed_compensation,lose_init_on_enable,
             lose_init_on_compensation,churn,new_custom_failed_release,new_native_failed_enable};
bool initialized(){return ready;}
bool alive(uint32_t) noexcept {return live;}
int setter(uint32_t id,float value,uint64_t object,int32_t ms,uint32_t curve,bool bypass){
    ++calls;
    expect(id==658155624&&object==UINT64_MAX&&curve==4&&!bypass&&ms>=0,"SDK argument contract");
    int result=1;
    if(value==-35.f){
        if(action==new_custom_failed_enable||action==new_native_failed_enable){
            const bool custom=action==new_custom_failed_enable;action=none;
            expect(running->intent.commit(1,++generation,custom),"newer validated generation published during failed enable");
            running->pump();result=2;
        }else if(action==stop_failed_compensation||action==lose_init_on_compensation){
            expect(running->intent.observe(0,running->intent.capture(0),0,1),"Stop inside enable");
            running->pump();
        }else if(action==lose_init_on_enable){action=none;ready=false;}
        else if(action==churn){expect(running->intent.commit(1,++generation,false),"churn publishes native generation");running->pump();}
    }else{
        if(action==stop_failed_compensation){action=none;result=2;}
        else if(action==lose_init_on_compensation){action=none;ready=false;}
        else if(action==new_custom_failed_release){action=none;expect(running->intent.commit(1,++generation,true),"new custom generation during failed neutral");running->pump();result=2;}
        else if(action==churn){expect(running->intent.commit(1,++generation,true),"churn publishes custom generation");running->pump();}
    }
    if(result==1)applied=value;
    std::printf("SDK call=%u request=%.0f result=%d applied=%.0f logical=%d\n",calls,value,result,applied,running->intent.suppress());
    return result;
}
struct Fixture {
    MusicControl control;
    Fixture(){running=&control;control.prepare(658155624,setter,initialized,alive,nullptr);control.enable();}
    void play(uint32_t source=0){require(control.intent.commit(source,generation,true),"validated custom commit");}
    void state(bool logical,float parameter,const char* message){
        std::printf("STATE logical=%d applied=%.0f expected_logical=%d expected_applied=%.0f\n",control.intent.suppress(),applied,logical,parameter);
        expect(control.intent.suppress()==logical&&applied==parameter,message);
    }
};
void first_unusable_commit(){Fixture f;f.play();ready=false;f.control.pump();
    expect(calls==0,"no SDK call before first usable initialization");
    ready=true;f.control.pump();f.state(false,-48,"pre-initialization request never resurrects");
    ++generation;f.play(1);f.control.pump();f.state(true,-35,"new validated generation works after initial unavailability");
}
void first_ready_commit(){Fixture f;f.play();f.control.pump();ready=false;f.control.pump();
    ready=true;++generation;f.play(1);f.control.pump();
    f.state(true,-35,"fresh usable-engine commit works without an intermediate healthy pump");
}
void failed_old_enable_new_custom(){Fixture f;f.play();action=new_custom_failed_enable;f.control.pump();
    f.state(true,-48,"failed old enable plus fallback preserves newer custom intent");
    f.control.pump();f.state(true,-35,"new custom generation applies on next explicit control opportunity");
    expect(calls==3,"one failed enable, one fallback, one fresh enable");
}
void failed_old_enable_new_native(){Fixture f;f.play();action=new_native_failed_enable;f.control.pump();
    f.state(false,-48,"new native generation remains neutral after old enable failure");
    ++generation;f.play(1);f.control.pump();f.state(true,-35,"later custom playback still recovers");
}
void failed_compensation(){Fixture f;f.play();action=stop_failed_compensation;f.control.pump();
    expect(calls==2,"failed compensation does not loop");f.state(false,-35,"failed compensation does not falsify applied neutral");
    f.control.pump();f.state(false,-48,"uncached failed compensation retried on explicit control opportunity");
    expect(calls==3,"neutral retry was not skipped");
}
void lost_init_during_enable(){Fixture f;f.play();action=lose_init_on_enable;f.control.pump();
    expect(calls==1,"initialization loss prevents immediate SDK compensation");f.state(false,-35,"successful enable is canceled logically when SDK becomes unusable");
    ready=true;f.control.pump();f.state(false,-48,"usable engine receives neutral without resurrecting old generation");
    ++generation;f.play(1);f.control.pump();f.state(true,-35,"fresh playback recovers after in-flight engine loss");
}
void lost_init_during_compensation(){Fixture f;f.play();action=lose_init_on_compensation;f.control.pump();
    expect(calls==2,"no extra SDK work after compensation loses initialization");f.state(false,-48,"applied neutral remains distinct from unusable epoch");
    ready=true;++generation;f.play(1);f.control.pump();f.state(true,-35,"fresh generation recovers after compensation-time loss");
}
void stale_invalidation(){Fixture f;f.play();const auto old=f.control.intent.snapshot();
    ++generation;f.play(1);expect(!f.control.intent.invalidate(old),"old failure cannot invalidate newer source/generation");
    f.control.pump();f.state(true,-35,"new source remains enabled");
    const auto current=f.control.intent.snapshot();expect(f.control.intent.invalidate(current),"current recoverable generation cancels");
    expect(!f.control.intent.commit(1,generation,true),"same generation cannot revive canceled playback");
    f.control.pump();f.state(false,-48,"canceled generation releases");
    ++generation;f.play(1);f.control.pump();f.state(true,-35,"strictly newer generation restores playback");
}
void lost_liveness(){Fixture f;f.play();f.control.pump();live=false;f.control.pump();
    f.state(false,-48,"lost live identity cancels current generation");live=true;f.control.pump();
    f.state(false,-48,"liveness recovery alone never resurrects old playback");
    ++generation;f.play(1);f.control.pump();f.state(true,-35,"new validated live source recovers");
}
void failed_old_release_new_custom(){Fixture f;f.play();f.control.pump();
    require(f.control.intent.observe(0,f.control.intent.capture(0),0,1),"Stop observation");
    action=new_custom_failed_release;f.control.pump();
    f.state(true,-35,"failed old release does not clear newer custom commit or falsify applied parameter");
    const auto before=calls;f.control.pump();expect(calls==before+1,"failed neutral not cached as fresh enable");
    f.state(true,-35,"fresh custom command explicitly applied after failed release");
}
void bounded_churn(){Fixture f;f.play();action=churn;
    for(unsigned n=0;n<4;++n){const auto before=calls;f.control.pump();expect(calls==before+2,"each changing-intent pump limited to primary plus compensation");f.state(true,-48,"new custom remains pending after neutral compensation");}
    action=none;f.control.pump();f.state(true,-35,"latest validated generation applies after churn stops");
}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"unusable_before_first_init_cancels_commit",first_unusable_commit},
    {"fresh_commit_on_first_usable_update",first_ready_commit},
    {"new_custom_during_failed_old_enable",failed_old_enable_new_custom},
    {"new_native_during_failed_old_enable",failed_old_enable_new_native},
    {"failed_immediate_compensation_retry",failed_compensation},
    {"initialization_lost_inside_setter",lost_init_during_enable},
    {"initialization_lost_inside_compensation",lost_init_during_compensation},
    {"exact_generation_failure_invalidation",stale_invalidation},
    {"liveness_return_needs_new_commit",lost_liveness},
    {"new_custom_during_failed_old_release",failed_old_release_new_custom},
    {"bounded_work_under_generation_churn",bounded_churn}};
    return remediation_test::main(argc,argv,cases);
}
