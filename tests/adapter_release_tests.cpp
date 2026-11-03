// SPDX-License-Identifier: MIT
// Include unchanged adapter so tests use its actual callbacks, Core and intent.
// Fixture setup replaces activation/publication, SDK and native originals only.
#define ADAPTER_SELFTEST_BUILD
#include "../source/supermoon_adapter.cpp"
#include "remediation_harness.hpp"
#include <chrono>
#include <fstream>
extern "C" void FixtureFrameReturn(){} // Live-native unwind checking is not enabled.
extern "C" uint32_t RecoverySourceTerm(void*,void*);
using remediation_test::expect;
using remediation_test::require;
namespace {
float applied=-48.f;
unsigned sdk_calls{},unlocks{};
unsigned fail_sdk{};
bool in_provider{},sdk_on_provider{};
int sdk(uint32_t,float value,uint64_t,int32_t,uint32_t,bool){++sdk_calls;sdk_on_provider|=in_provider;if(fail_sdk){--fail_sdk;return 2;}applied=value;return 1;}
bool sdk_ready(){return true;}
void native_unlock(){++unlocks;}
struct Fixture {
    alignas(16) unsigned char component[128]{};
    bytebeat::CompositionKey key{};
    bytebeat::playlist::Engine engine;
    bytebeat::recovery::Source* source{};
    uint32_t index{};
    static std::vector<bytebeat::playlist::Prepared> tracks(){
        std::vector<bytebeat::playlist::Prepared> result;
        for(unsigned n=0;n<2;++n){auto path=std::filesystem::path(n?"audible.pcm":"silence.pcm");std::ofstream file(path,std::ios::binary);float value=n?.25f:0.f;for(unsigned i=0;i<16;++i)file.write(reinterpret_cast<const char*>(&value),4);file.close();require(bool(file),"synthetic PCM write");result.push_back({path,16,0,1,"owned synthetic PCM"});}
        return result;
    }
    Fixture():engine(tracks(),17,0){
        AdapterTestConfigureEngine(nullptr,engine,0,key);
        engine_log.store(&engine);core->endpoint_validator(&endpoint_live);
        floor_carrier_key=key;
        source=bytebeat::recovery::registry.allocate_source();require(source,"owned source");
        auto event=bytebeat::recovery::registry.begin_event(reinterpret_cast<uintptr_t>(stream()));require(event,"owned event");
        source->id.store(41);source->phase.store(bytebeat::recovery::ready);
        bytebeat::recovery::registry.publish(*event,41);bytebeat::recovery::registry.bind(*source);
        index=uint32_t(source-bytebeat::recovery::registry.sources);
        bytebeat::recovery::set_enabled();
        floor_control.prepare(658155624,sdk,sdk_ready,&floor_live,nullptr);floor_control.enable();
        FloorNativeUnlock=reinterpret_cast<void*>(&native_unlock);
        stream()[0x14]=stream()[0x15]=1;
    }
    ~Fixture(){engine_ready.store(false);engine_log=nullptr;delete core;core=&initial_core;}
    unsigned char* stream(){return component+0x28;}
    void commit(bool custom=true){
        auto selected=key;if(!custom)selected[0]=1;
        pending={reinterpret_cast<uintptr_t>(component),123,selected,true,true};
        AdapterCommit(component,reinterpret_cast<void*>(uintptr_t(123)));
        expect(floor_control.intent.suppress()==custom,"actual AdapterCommit publishes expected logical intent");
    }
    void update(){const auto before=unlocks;FloorControlUnlockBridge();expect(unlocks==before+1,"actual control bridge calls original unlock exactly once");}
    void observe_stop(){
        stream()[0x14]=0;
        CallbackContext context{};context.stream=stream();context.endpoint=reinterpret_cast<uintptr_t>(source);
        current=&context;in_provider=true;AdapterObserveState(stream());in_provider=false;current=nullptr;
        expect(!sdk_on_provider,"no SDK call in provider observation");
    }
    void state(bool logical,float parameter,const char* message){std::printf("STATE logical=%d applied=%.0f sdk_calls=%u unlocks=%u\n",floor_control.intent.suppress(),applied,sdk_calls,unlocks);expect(floor_control.intent.suppress()==logical&&applied==parameter,message);}
};
void nominal(){Fixture f;f.commit();f.update();f.state(true,-35,"actual adapter custom Play applies suppression");f.observe_stop();f.update();f.state(false,-48,"actual provider Stop plus control unlock applies neutral");f.commit();f.update();f.commit(false);f.update();f.state(false,-48,"actual native-track switch applies neutral");}
void last_stop(){Fixture f;f.commit();f.update();f.observe_stop();
    // Deliberately no arbitrary future pump. Provider traffic and redirected
    // control unlocks cease here. A safe independent release service is absent.
    f.state(false,-48,"A1-M2 final provider Stop must receive a safe release opportunity");}
void last_retirement(){Fixture f;f.commit();f.update();require(RecoverySourceTerm(f.source->native,nullptr)==1,"actual Term wrapper");
    expect(!bytebeat::recovery::Registry::live(f.source),"last source is retired");
    f.state(false,-48,"A1-M2 actual retirement without later callbacks must release applied suppression");}
void retirement_with_update(){Fixture f;f.commit();f.update();RecoverySourceTerm(f.source->native,nullptr);f.update();f.state(false,-48,"later real control bridge repairs retirement");}
void last_release_fails(){Fixture f;f.commit();f.update();f.observe_stop();fail_sdk=1;f.update();
    f.state(false,-48,"A1-M2 failed final release needs a represented safe retry after provider cessation");}
void eof_and_silence(){Fixture f;f.commit();f.update();
    f.state(true,-35,"suppression active before worker refill/PCM availability");
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    bool silent=false,audible=false;unsigned empty=0;
    while(f.engine.stats().completed<4&&std::chrono::steady_clock::now()<deadline){
        auto frame=f.engine.peek();
        if(frame.pcm){silent|=frame.sample==0;audible|=frame.sample!=0;require(f.engine.consume(frame),"actual Engine consumes synthetic frame");}
        else {++empty;std::this_thread::yield();}
        // Song EOF and refill are internal to Engine: do not synthesize a native
        // Stop or new adapter commit at these points.
        FloorControlUnlockBridge();
        require(floor_control.intent.suppress()&&applied==-35,"EOF/refill retained controller intent and applied parameter");
    }
    expect(f.engine.stats().completed>=4&&silent&&audible,"actual Engine crossed silent and audible song EOFs");
    std::printf("STREAM completed=%llu empty_peeks=%u\n",static_cast<unsigned long long>(f.engine.stats().completed),empty);
    f.state(true,-35,"continuous custom carrier retains suppression through EOFs and silence");
    f.observe_stop();f.update();f.state(false,-48,"native Stop remains distinct from internal EOF");
}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"actual_adapter_nominal",nominal},{"final_provider_stop_no_release_opportunity",last_stop},
    {"last_source_retired_no_release_opportunity",last_retirement},{"retirement_with_later_control_unlock",retirement_with_update},
    {"failed_final_release_no_later_callback",last_release_fails},
    {"actual_engine_silence_eof_refill",eof_and_silence}};
    return remediation_test::main(argc,argv,cases);
}
