// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "playlist/stream_engine.hpp"
#include "playlist/windows_controls.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <cstdlib>
#include <new>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
thread_local bool callback_audit=false;
thread_local unsigned callback_allocations=0;
void* operator new(size_t size){if(callback_audit)++callback_allocations;if(auto p=std::malloc(size?size:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,size_t) noexcept {std::free(p);}
void* operator new[](size_t size){return ::operator new(size);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,size_t) noexcept {std::free(p);}
using namespace bytebeat::playlist;
namespace {
void check(bool v,const char* m){if(!v)throw std::runtime_error(m);}
Engine::Frame frame(Engine& e){auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(std::chrono::steady_clock::now()<end){callback_audit=true;auto f=e.peek();callback_audit=false;if(f.pcm)return f;check(e.healthy(),"Worker failed");std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("PCM timeout");}
Engine::Frame transition(Engine& e,uint64_t serial){auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(std::chrono::steady_clock::now()<end){auto f=e.peek();if(f.pcm&&f.serial!=serial)return f;check(e.healthy(),"Transition worker failed");std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Transition timeout");}
Prepared cache(const std::filesystem::path& p,uint64_t frames,float sample){std::ofstream file(p,std::ios::binary);for(uint64_t n=0;n<frames;++n)file.write(reinterpret_cast<char*>(&sample),4);file.close();check(bool(file),"Fixture write");return {p,frames,0,1,"fixture"};}
}
int main(){try{
    auto root=std::filesystem::path("build/playlist-lufs/stream-fixtures")/std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());std::filesystem::create_directories(root);
    std::vector<Prepared> tracks;for(unsigned i=0;i<5;++i)tracks.push_back(cache(root/(std::to_string(i)+".pcm"),257+i*13,.1f+float(i)*.1f));
    {
    Engine e(tracks,81);check(!e.peek().pcm&&!e.command(Command::next),"Inactive scope");e.set_active(true);
    check(valid_keys({})&&valid_keys({true,'T','Y'})&&!valid_keys({true,'T','T'})&&!valid_keys({true,16,'T'}),"Key configuration validation");WindowsControls disabled(e);
    std::set<uint32_t> cycle;uint32_t prior=UINT32_MAX;uint64_t count=0;
    for(unsigned song=0;song<100;++song){auto first=frame(e);check(first.position==0&&first.sample==0,"Song start ramp");auto id=first.track;auto serial=first.serial;
        if(song%5==0){cycle.clear();check(id!=prior,"Cycle immediate repeat");}check(cycle.insert(id).second,"Cycle duplicate");
        for(uint64_t pos=0;pos<tracks[id].frames;++pos){auto f=frame(e);check(f.track==id&&f.serial==serial&&f.position==pos&&f.last==(pos+1==tracks[id].frames),"PCM cursor/EOF");
            float gain=float(std::min<uint64_t>(64,std::min(pos,tracks[id].frames-1-pos)))/64.f;
            check(std::abs(f.sample-(.1f+float(id)*.1f)*gain)<1e-7,"PCM corrupt/ramp");callback_audit=true;bool consumed=e.consume(f),double_consumed=e.consume(f);callback_audit=false;check(consumed&&!double_consumed,"Double consume guard");++count;}
        prior=id;
    }
    check(e.stats().completed==100&&e.stats().pcm_frames==count,"Actual PCM accounting");
    auto a=frame(e);check(e.consume(a),"Manual first");check(e.command(Command::next),"Next enqueue");auto b=transition(e,a.serial);check(b.position==0&&b.track!=a.track,"Next follows audible history");check(e.consume(b),"Next consume");
    check(e.command(Command::previous),"Previous enqueue");auto c=transition(e,b.serial);check(c.position==0&&c.track==a.track,"Previous actual history");check(e.consume(c),"Previous consume");
    auto stale=frame(e);e.restart();check(!e.consume(stale),"Generation stale consume");auto d=transition(e,c.serial);check(d.track==c.track&&d.position==0,"Native commit restart current");check(e.consume(d),"Restart consume");
    e.set_active(false);check(!e.peek().pcm&&!e.command(Command::previous),"Switch away scope");e.restart();e.set_active(true);auto returned=transition(e,d.serial);check(returned.position==0&&returned.track==d.track,"Reentry contract");check(e.consume(returned),"Reentry consume");
    for(unsigned i=0;i<10000;++i)e.command(i&1?Command::previous:Command::next);check(e.stats().rejected>0,"Command capacity");
    for(unsigned i=0;i<5000;++i){auto f=frame(e);check(e.consume(f),"Rapid command state");}check(e.healthy(),"Rapid command worker");
    auto bad=cache(root/"nan.pcm",1,std::numeric_limits<float>::quiet_NaN());auto missing=bad;missing.path=root/"missing.pcm";
    Engine skip({missing,bad,tracks[0]},0,0);skip.set_active(true);auto good=frame(skip);check(good.track==2,"Invalid prepared skip");check(skip.consume(good),"Skip consume");check(skip.stats().failed>=2,"Failure diagnostics");
    Engine invalid({missing},0);invalid.set_active(true);for(unsigned i=0;i<100&&invalid.healthy();++i)std::this_thread::sleep_for(std::chrono::milliseconds(1));check(!invalid.healthy()&&!invalid.peek().pcm,"All invalid safe");
    bool empty=false;try{Engine empty_engine({});}catch(...){empty=true;}check(empty,"Empty rejection");
    Engine single({tracks[0]},0,0);single.set_active(true);for(unsigned i=0;i<2000;++i){auto f=frame(single);check(f.track==0&&single.consume(f),"Single song repeat");}
    std::atomic<bool> done{false};std::thread controls([&]{for(unsigned i=0;i<20000;++i){if(i%19==0)single.restart();else single.command(i&1?Command::next:Command::previous);}done.store(true);});
    struct Join {std::thread& thread;~Join(){if(thread.joinable())thread.join();}}join{controls};
    unsigned consumed=0;while(!done.load()||consumed<2000){auto f=single.peek();if(f.pcm&&single.consume(f))++consumed;else std::this_thread::yield();check(single.healthy(),"Concurrent worker failed");}controls.join();
    } // All unrelated workers joined; handle fixture starts from an idle baseline.
    check(callback_allocations==0,"Audio API allocated memory");DWORD handles_before{},handles_after{};check(GetProcessHandleCount(GetCurrentProcess(),&handles_before)!=0,"Handle count before");
    for(unsigned i=0;i<50;++i){Engine short_lived({tracks[0]},0,0);short_lived.set_active(true);auto f=frame(short_lived);check(short_lived.consume(f),"Repeated worker lifecycle");}
    check(GetProcessHandleCount(GetCurrentProcess(),&handles_after)!=0,"Handle count after");std::cout<<"handle_snapshot before="<<handles_before<<" after="<<handles_after<<"\n";check(handles_after==handles_before,"Worker/file handle leak");
    std::cout<<"PASS stream engine: 100 real EOF songs/20 shuffle cycles, exact samples, audible history, stale generations, scoped activity, invalid caches, bounded commands, concurrent controls; zero audited callback allocations, 50 worker lifecycles/no handle leak; PCM queue="<<Engine::block_frames*Engine::block_count*4<<" bytes\n";
    return 0;
}catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';return 1;}}
