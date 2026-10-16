// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "endurance_writer.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <set>
#include <stdexcept>
#include <iostream>
using namespace bytebeat::endurance;
static unsigned checks=0;
static void require(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
static std::string read(const std::filesystem::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int main(){try{
    Ring<4> small;Event e{};
    for(unsigned i=0;i<4;++i){e.call=i;require(small.push(e),"Ring admits capacity");}
    require(!small.push(e),"Full ring refuses overwrite");
    for(unsigned i=0;i<4;++i){require(small.pop(e)&&e.call==i,"Ring preserves ownership/FIFO");}
    require(!small.pop(e),"Empty ring bounded");
    for(unsigned i=0;i<100000;++i){e.call=i;require(small.push(e)&&small.pop(e)&&e.call==i,"Repeated wrap");}
    Ring<256> concurrent;std::atomic<unsigned> done{0},admitted{0},rejected{0};std::set<uint64_t> seen;
    std::vector<std::thread> producers;
    for(unsigned t=0;t<4;++t)producers.emplace_back([&,t]{for(unsigned i=0;i<25000;++i){Event r{};r.call=uint64_t(t)*25000+i;r.generation=r.call^0xfeedbeef;if(concurrent.push(r))++admitted;else ++rejected;}++done;});
    while(done.load()!=4){while(concurrent.pop(e)){require(e.generation==(e.call^0xfeedbeef)&&seen.insert(e.call).second,"Concurrent complete/no duplicate payload");}std::this_thread::yield();}
    for(auto& p:producers)p.join();while(concurrent.pop(e)){require(e.generation==(e.call^0xfeedbeef)&&seen.insert(e.call).second,"Final concurrent drain");}
    require(seen.size()==admitted&&admitted+rejected==100000,"Every admitted record retained; rejection accounted");
    Frames f{};FrameInfo missing{};missing.streaming=true;missing.absence=3;
    for(unsigned i=0;i<5;++i)f.observe(missing);
    auto available=missing;available.available=available.consumed=true;f.observe(available);
    for(unsigned i=0;i<3;++i)f.observe(missing);
    require(f.attempted==9&&f.empty==8&&f.prefix==5&&f.suffix==3&&f.maximum==5,"Exact per-frame burst shape");
    missing.absence=1;f.observe(missing);require(f.inactive==1&&f.empty==8&&f.suffix==0,"Inactive not queue starvation");
    missing.absence=2;f.observe(missing);require(f.unhealthy==1&&f.empty==8,"Failure not queue starvation");
    missing.absence=4;f.observe(missing);require(f.revision_empty==1&&f.empty==9,"Stale revision retirement identified");
    available.consumed=false;f.observe(available);require(f.stale==1,"Stale available not empty");
    Merge m;e={};e.call=1;e.stream=5;e.generation=9;e.frames=4;e.first_clock=0;e.last_clock=3;e.pcm.attempted=e.pcm.empty=e.pcm.prefix=e.pcm.suffix=e.pcm.maximum=4;e.pcm.first_revision=e.pcm.last_revision=2;
    m.accept(e,0);e.call=2;e.first_clock=4;e.last_clock=7;m.accept(e,0);require(m.maximum==8,"Cross-provider consecutive frames");
    e.call=3;e.first_clock=8;e.last_clock=11;m.accept(e,1);require(m.maximum==8&&m.run==4,"Dropped evidence breaks continuity");
    e.exceptional=true;m.accept(e,1);require(!m.known&&!m.run,"Exception breaks continuity");
    e.exceptional=false;e.produced=0;e.frames=0;e.pcm={};m.accept(e,1);require(!m.known&&!m.run,"Stop/native zero is not PCM empty");
    auto root=std::filesystem::path("fixtures/telemetry");std::filesystem::create_directories(root);
    auto legacy_path=root/"legacy.csv";HANDLE legacy=CreateFileW(legacy_path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,0,nullptr);require(legacy!=INVALID_HANDLE_VALUE,"Owned legacy file");
    Writer writer;require(writer.open(legacy),"Worker writer open");auto start=writer.start_clock();Health h{};h.active=h.healthy=true;h.snapshot=true;h.track=1;h.serial=1;h.revision=1;h.length=6552000;
    constexpr unsigned seconds=4*3600+30*60,rate=40,frames=600;
    uint64_t expected_empty=0;
    for(unsigned second=1;second<=seconds;++second){
        for(unsigned call=0;call<rate;++call){e={};e.call=uint64_t(second-1)*rate+call;e.begin=e.call*600;e.end=e.begin+600;e.stream=11;e.generation=1;e.requested=e.produced=e.frames=frames;e.first_clock=uint32_t(e.call*600);e.last_clock=e.first_clock+599;
            FrameInfo info{};info.streaming=true;info.available=info.consumed=true;info.snapshot=true;info.revision=1;info.value={1,1,e.call*600,6552000,1};
            bool empty=second%3600==0&&call<9;if(empty){info.available=info.consumed=false;info.absence=3;expected_empty+=frames;}
            for(unsigned frame=0;frame<frames;++frame)e.pcm.observe(info);
            publish(e);writer.drain();
        }
        h.pcm=totals.attempted.load()-totals.empty.load();h.position=h.pcm;h.ui_now=uint64_t(second)*60;h.titles=h.times=h.ui_now;
        writer.heartbeat(h,true,false,start+uint64_t(second)*1000);
    }
    require(totals.calls==648000&&totals.attempted==388800000&&totals.empty==expected_empty&&totals.dropped==0,"4h30m equivalent full callback/frame accounting without drops");
    require(writer.health_ok()&&writer.event_ok(),"4h30m logs within independent disk caps");writer.close();CloseHandle(legacy);
    auto content=read(root/"supermoon-endurance-health.csv");require(content.find("16200000,")!=std::string::npos&&content.find("native_drained=unknown")!=std::string::npos,"Full-session coverage and truthful end");
    require(content.find("start_qpc=")!=std::string::npos&&content.find("start_tick_ms=")!=std::string::npos&&content.find("UTC=")!=std::string::npos,"Cross-file timestamp anchors");
    require(std::filesystem::file_size(root/"supermoon-endurance-health.csv")<64*1024*1024&&std::filesystem::file_size(root/"supermoon-endurance-events.csv")<32*1024*1024,"Actual bounded output sizes");
    Output limited;limited.file=CreateFileW((root/"budget.txt").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);limited.limit=4;limited.line("1234");limited.line("5");require(limited.limited&&limited.bytes==4,"Budget failure explicit/no growth");limited.close();
    Output disk_failure;disk_failure.limit=100;disk_failure.file=CreateFileW((root/"readonly.txt").c_str(),GENERIC_READ,0,nullptr,CREATE_ALWAYS,0,nullptr);disk_failure.line("write must fail");require(!disk_failure.ok,"Permission/disk write failure explicit");disk_failure.close();
    auto nonexistent=root/"missing"/"legacy.csv";HANDLE invalid=CreateFileW(nonexistent.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);Writer failed;require(!failed.open(invalid),"Missing path safely refused");failed.close();
    printf("PASS endurance checks=%u simulated_seconds=16200 callbacks=648000 completed_frames=388800000 empty_frames=%llu ring_dropped=0 concurrent_admitted=%u rejected_accounted=%u\n",checks,(unsigned long long)expected_empty,admitted.load(),rejected.load());return 0;
}catch(const std::exception& e){fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
