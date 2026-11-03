// SPDX-License-Identifier: MIT
// Actual exported stage reader + owned injected cold-stage failures only.
#define ADAPTER_SELFTEST_BUILD
#include "../source/supermoon_adapter.cpp"
#include "remediation_harness.hpp"
#include <fstream>
extern "C" void FixtureFrameReturn(){}
using remediation_test::expect;
namespace {
void stages(){
    using bytebeat::activation::Stage;
    for(auto s:{Stage::preflight,Stage::lifecycle_exposed,Stage::core_ready,Stage::publication_started,Stage::published}){
        activation_stage.store(uint32_t(s),std::memory_order_release);
        auto reported=AdapterActivationStage(nullptr);
        expect(reported==uint32_t(s),"actual exported reader returns injected stage");
        expect(std::strstr(bytebeat::activation::name(reported),"activation_stage_"),"stage name is explicit");
        expect(bool(std::strstr(bytebeat::activation::failure(reported),"exit_normally"))==(s!=Stage::preflight),"all possibly exposed failures require normal exit/no retry");
    }
}
void numeric_results(){
    using bytebeat::activation::Stage;
    struct Injection {unsigned result;Stage stage;};
    for(auto test:{Injection{1,Stage::preflight},Injection{2,Stage::preflight},Injection{16,Stage::lifecycle_exposed},Injection{12,Stage::lifecycle_exposed},Injection{7,Stage::lifecycle_exposed},Injection{9,Stage::lifecycle_exposed},Injection{11,Stage::core_ready},Injection{10,Stage::publication_started},Injection{18,Stage::publication_started}}){
        activation_stage.store(uint32_t(test.stage));auto observed=AdapterActivationStage(nullptr);
        auto message=bytebeat::activation::failure(observed,test.result);
        std::printf("injected_result=%u observed_stage=%s failure=%s\n",test.result,bytebeat::activation::name(observed),message);
        expect(bool(std::strstr(message,"exit_normally"))==(test.stage!=Stage::preflight||test.result==1),"exposed or concurrent/repeated activation refusal requires normal exit, independent of result-10 mapping");
    }
    expect(std::strstr(bytebeat::activation::failure(99),"exit_normally"),"unknown stage is conservative");
}
void exception(){
    activation_stage.store(uint32_t(bytebeat::activation::Stage::core_ready));
    try{throw 1;}catch(...){expect(AdapterActivationStage(nullptr)==uint32_t(bytebeat::activation::Stage::core_ready),"injected preparation/publication exception does not erase last stage");}
}
void writer_capacity(){
    for(unsigned n=0;n<bytebeat::recovery::source_capacity;++n)remediation_test::require(bytebeat::recovery::registry.allocate_source(),"owned cumulative births");
    {std::ofstream f("capacity.pcm",std::ios::binary);float value=0;for(unsigned n=0;n<16;++n)f.write(reinterpret_cast<char*>(&value),4);}
    bytebeat::playlist::Engine engine({{"capacity.pcm",16,0,1,"owned PCM"}},17,0);
    auto thread=AdapterTestStartWriter(engine,L"capacity.csv");remediation_test::require(thread,"actual diagnostic writer start");
    expect(AdapterTestStopWriter(thread),"actual writer joins in owned fixture");
    std::ifstream f("capacity.csv");std::string text{std::istreambuf_iterator<char>(f),{}};
    expect(text.find("# recovery_capacity cumulative_not_songs sources=4096/4096")!=std::string::npos&&text.find("refused_tracked=")!=std::string::npos&&text.find("exit_normally_restart_no_slot_reuse")!=std::string::npos,"real writer publishes capacity limits, tracked counters and restart/degradation guidance");
}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={{"exported_stage_reporting",stages},{"all_refusal_codes_after_exposure",numeric_results},{"exception_preserves_stage",exception},{"actual_writer_capacity_visibility",writer_capacity}};return remediation_test::main(argc,argv,cases);}
