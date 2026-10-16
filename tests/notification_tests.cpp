// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "supermoon_core.hpp"
#include "playlist/keyboard_settings.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <new>
#include <thread>
using namespace bytebeat;using namespace bytebeat::playlist;
static uint64_t checks{};static std::atomic<uint64_t> allocations{};static thread_local bool audit{};
void* operator new(size_t n){if(audit)allocations++;if(auto p=malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{free(p);}void operator delete(void* p,size_t)noexcept{free(p);}
void check(bool b,const char* why){++checks;if(!b){fprintf(stderr,"FAIL %s\n",why);abort();}}
template<class F>void wait(F f){auto end=std::chrono::steady_clock::now()+std::chrono::seconds(8);while(!f()){check(std::chrono::steady_clock::now()<end,"Bounded own fixture wait");Sleep(1);}}
Engine::Frame ready(Engine& e){Engine::Frame f;wait([&]{audit=true;f=e.peek();audit=false;return f.pcm;});return f;}
void consume(Engine& e,unsigned n){for(unsigned i=0;i<n;++i){auto f=ready(e);audit=true;check(e.consume(f),"PCM consumption exact");audit=false;}}
static double budget(float music,double bound,float envelope,float gain,const Cue& cue,unsigned frame){
    double scale=std::min(1.,std::max(0.,.98/std::max(1.,double(envelope)*gain)-bound)/cue.peak);
    float v=float(double(cue.pcm[frame])*scale);
    return std::abs(double(music)+v)>.98/std::max(1.,double(envelope)*gain)?0:v;
}
void unit(){
    check(on_cue.frames==48126&&off_cue.frames==48126,"Embedded original duration preserved to output sample");
    Notifications n(true),disabled(false);float music=.1f;const double bound=.2;
    check(n.sample(music,bound,1,1)==0&&n.committed()==0,"Startup silent");
    for(bool on:{true,false}){
        n.publish(on,n.epoch());const Cue& cue=on?on_cue:off_cue;
        for(unsigned i=0;i<cue.frames;++i){audit=true;float s=n.sample(music,bound,1,1);float again=n.sample(music,bound,1,1);check(s==again,"Repeated peek doesn't advance cue");
            check(std::abs(double(s)-budget(music,bound,1,1,cue,i))<1e-8,"Correct ON/OFF embedded sample");n.consume();audit=false;}
        check(n.sample(music,bound,1,1)==0,"Cue ends without replacement/repetition");
    }
    check(n.committed()==2,"Two successful publication events");
    for(float music_bound:{.1f,.8414f,.8811f,.9441f,1.f}){
        for(bool on:{false,true}){
            n.cancel();n.publish(on,n.epoch());const Cue& cue=on?on_cue:off_cue;
            for(unsigned i=0;i<cue.frames;++i){float contribution=n.sample(music_bound,music_bound,1,1);
                check(std::abs(double(music_bound)+contribution)<=1.000001,"Normalized/off/max-ceiling headroom stress");n.consume();}
        }
    }
    disabled.publish(true,disabled.epoch());check(disabled.committed()==0&&disabled.sample(music,bound,1,1)==0,"Disabled silence");
    auto old=n.epoch();auto count=n.committed();n.publish(true,old);n.cancel();check(n.sample(music,bound,1,1)==0,"Stop cancels active/pending cue");n.publish(false,old);check(n.committed()==count+1,"Old epoch not published");
    n.publish(true,n.epoch());n.publish(false,n.epoch());
    for(unsigned i=0;i<1000;++i){check(std::abs(double(n.sample(music,bound,1,1))-budget(music,bound,1,1,off_cue,i))<1e-8,"Latest rapid committed toggle wins");n.consume();}
    n.cancel();n.publish(true,n.epoch());check(n.sample(NAN,bound,1,1)==0&&n.sample(music,NAN,1,1)==0&&n.sample(music,bound,1,INFINITY)==0,"Invalid mixing falls back");
    n.cancel();n.publish(false,n.epoch());
    for(unsigned i=0;i<off_cue.frames;++i){for(float gain:{0.f,.2f,1.f,2.f,10.f}){
        float s=n.sample(.84f,.85,1,gain);double mixed=double(.84f)+s;
        check(std::isfinite(s)&&std::abs(mixed)<=1.,"Finite no extra dry clipping");
        if(gain<=1)check(std::abs(mixed*gain)<=.980001,"Bounded native-gain output");
        else check(s==0,"No extra clipping when original native factor already exceeds budget");}
        n.consume();}
    Notifications concurrent(true);std::atomic<bool> done{};
    std::thread worker([&]{for(unsigned i=0;i<10000;++i)concurrent.publish(i%2,concurrent.epoch());done=true;});
    for(unsigned i=0;i<100000||!done;++i){audit=true;float x=concurrent.sample(.1f,.2,1,1);check(std::isfinite(x)&&std::abs(.1f+x)<=.980001,"Concurrent mailbox immutable assets and bounds");concurrent.consume();audit=false;if(i%79==0)concurrent.cancel();}
    worker.join();
}
void integration(const std::filesystem::path& root){
    std::vector<Prepared> tracks;for(unsigned i=0;i<3;++i){auto path=root/("track"+std::to_string(i)+".pcm");std::ofstream out(path,std::ios::binary);float v=.1f*(i+1);for(unsigned n=0;n<144000;++n)out.write(reinterpret_cast<char*>(&v),4);out.close();tracks.emplace_back(path,144000,v,1,"test");}
    std::atomic<bool> persist_ok{true};std::atomic<unsigned> saves{};
    Engine e(tracks,7,0,false,[&](bool){saves++;return persist_ok.load();},true);
    CompositionKey key{},dream{};key[0]=1;dream[0]=2;SupermoonCore core(e);core.configure(key,true,{true,true,true,true,true,true},true);
    check(core.commit(0x1234,key,true,0),"Carrier commit");uint32_t clock=0;
    auto step=[&]{auto f=ready(e);float out=123;CompletedFrame c{0x1234,clock++,0,1,0,1,1,&out};audit=true;auto r=core.frame(c);audit=false;check(r==FrameResult::substituted||r==FrameResult::eof,"Actual guarded delivery");check(std::isfinite(out)&&std::abs(out)<1,"Actual mix never clips");return std::pair<Engine::Frame,float>{f,out};};
    for(unsigned i=0;i<1000;++i){auto v=step();check(v.second==v.first.sample,"No startup notification; baseline bit-exact");}
    auto before=ready(e);check(e.command(Command::toggle_shuffle),"Toggle admitted");wait([&]{return e.shuffle_enabled()&&e.notification_commits()==1;});
    auto after=ready(e);check(after.revision==before.revision,"Pure toggle preserves queued revision");
    check(after.track==before.track&&after.position>=before.position,"Pure toggle never navigates/restarts music");
    unsigned audible=0;for(unsigned i=0;i<on_cue.frames;++i){auto v=step();if(v.second!=v.first.sample)audible++;PlaybackSnapshot snap{};check(e.now_playing_snapshot(snap)&&snap.position==v.first.position+1&&snap.track==v.first.track,"Cue doesn't alter WAV position/UI identity");}
    check(audible>0,"Actual audio callback mixes ON cue");
    auto commits=e.notification_commits();persist_ok=false;check(e.command(Command::toggle_shuffle),"Failed persistence attempt admitted");wait([&]{return e.shuffle_failures()==1;});check(e.shuffle_enabled()&&e.notification_commits()==commits,"Failed commit emits no notification");
    persist_ok=true;check(e.command(Command::shuffle),"F9 command admitted");wait([&]{return ready(e).track!=after.track;});check(e.shuffle_enabled()&&e.notification_commits()==commits,"F9 no toggle sound/preference change");
    check(core.commit(0x1234,dream,true,clock),"Dreamers native selection committed");check(!e.command(Command::toggle_shuffle),"Dreamers rejects controls");float untouched=.123f;check(core.frame({0x1234,clock,0,1,0,1,1,&untouched})==FrameResult::native&&untouched==.123f,"Dreamers native PCM untouched");
    check(core.commit(0x1234,key,true,0),"Stop/Play carrier reentry");clock=0;for(unsigned i=0;i<100;++i){auto v=step();check(v.second==v.first.sample,"No stale cue after selection/restart");}
    auto off_before=ready(e);check(e.command(Command::toggle_shuffle),"OFF toggle admitted");wait([&]{return !e.shuffle_enabled()&&e.notification_commits()==commits+1;});
    check(ready(e).revision==off_before.revision,"Pure OFF toggle preserves queued revision");
    unsigned audible_off=0;for(unsigned i=0;i<off_cue.frames;++i){auto v=step();if(v.second!=v.first.sample)audible_off++;
        check(std::abs(double(v.second)-double(v.first.sample)-budget(v.first.sample,tracks[v.first.track].measured_reconstructed_peak+.00001,1,1,off_cue,i))<1e-6,"Actual guarded OFF cue sample selection");}
    check(audible_off>0,"Actual audio callback mixes OFF cue");
    auto off_commits=e.notification_commits();check(e.command(Command::shuffle),"F9 while mode OFF admitted");
    wait([&]{auto f=e.peek();return f.pcm&&f.revision!=off_before.revision;});
    check(!e.shuffle_enabled()&&e.notification_commits()==off_commits,"F9 also preserves OFF preference without cue");
    core.retire(0x1234);check(!e.command(Command::toggle_shuffle),"Retired owner rejects toggle");wait([&]{return e.worker_exited();});check(e.join_finished_worker(),"Disk worker safely joined outside callback");
    check(saves==3,"Only successful/failed F8 attempts call persistence");
    Engine off(tracks,1,0,false,{},false);off.set_active(true);consume(off,2);check(off.command(Command::toggle_shuffle),"Disabled sound still toggles");wait([&]{return off.shuffle_enabled();});check(off.notification_commits()==0,"Config-disabled no event");
    std::atomic<bool> pending{},release{};
    Engine blocked(tracks,1,0,false,[&](bool){pending=true;while(!release.load())Sleep(1);return true;},true);
    blocked.set_active(true);consume(blocked,2);check(blocked.command(Command::toggle_shuffle),"Slow persistence admitted");wait([&]{return pending.load();});
    check(blocked.notification_commits()==0,"Notification not published before persistence succeeds");
    blocked.set_active(false);release=true;wait([&]{return blocked.shuffle_enabled();});check(blocked.notification_commits()==0,"Deactivated during persistence produces no delayed sound");blocked.set_active(true);
    auto f=ready(blocked);check(blocked.notification_sample(f,1,1)==0,"No stale notification on returning to Supermoon");
    Engine edges_engine(tracks,2,0,false,{},true);edges_engine.set_active(true);consume(edges_engine,2);
    KeyboardEdges edges;KeyboardConfiguration cfg;InputContext ctx{true,true,false,3};std::array<bool,4> held{};
    unsigned sent=0;auto send=[&](KeyAction a){check(a==KeyAction::toggle_shuffle,"Correct F8 action");sent++;check(edges_engine.command(Command::toggle_shuffle),"Keyboard edge admitted");};
    edges.poll(cfg,ctx,held,send);held[2]=true;for(unsigned i=0;i<1000;++i)edges.poll(cfg,ctx,held,send);
    wait([&]{return edges_engine.notification_commits()==1;});check(sent==1,"Held-key repeated polls yield one committed notification");
    for(unsigned i=0;i<50;++i){auto target=edges_engine.notification_commits()+1;check(edges_engine.command(Command::toggle_shuffle),"Rapid intentional toggle accepted");wait([&]{return edges_engine.notification_commits()==target;});}
    check(edges_engine.notification_commits()==51&&edges_engine.shuffle_enabled(),"Rapid toggle commits preserve parity/latest mode");
    edges_engine.restart();auto next=ready(edges_engine);check(edges_engine.notification_sample(next,1,1)==0,"Native restart cancels outstanding notification");
    std::atomic<bool> queue_pending{},queue_release{};
    Engine queued(tracks,3,0,false,[&](bool){queue_pending=true;while(!queue_release.load())Sleep(1);return true;},true);
    queued.set_active(true);consume(queued,2);check(queued.command(Command::toggle_shuffle),"Initial queued toggle admitted");wait([&]{return queue_pending.load();});
    unsigned accepted=1,rejected=0;for(unsigned i=0;i<64;++i){if(queued.command(Command::toggle_shuffle))accepted++;else rejected++;}
    check(accepted<=Engine::command_count&&rejected>0&&queued.notification_commits()==0,"Full command queue rejects excess without notification");
    queue_release=true;wait([&]{return queued.notification_commits()==accepted;});check(queued.shuffle_enabled()==bool(accepted%2),"Exactly successful queued toggles publish; rejected commands don't");
}
void configuration(const std::filesystem::path& root){
    auto ini=root/"playlist.ini";check(notification_configuration(root),"Older config defaults ON");
    for(auto value:{"0","1","bad","11"}){std::ofstream out(ini);out<<"[Notifications]\nShuffleSounds="<<value<<'\n';out.close();WritePrivateProfileStringW(nullptr,nullptr,nullptr,ini.c_str());bool thrown=false,result=false;try{result=notification_configuration(root);}catch(...){thrown=true;}
        if(value[0]=='0')check(!thrown&&!result,"Disabled parsed");else if(value[0]=='1'&&value[1]==0)check(!thrown&&result,"Enabled parsed");else check(thrown,"Malformed config safely rejected");}
    auto music=root/"Music";std::filesystem::create_directory(music);for(auto n:{"Shuffle On.wav","sHUfFlE oFF.WAV","song.wav"}){std::ofstream f(music/n);f<<"fixture";}
    auto d=discover(music);check(d.paths.size()==1&&d.paths[0].filename()=="song.wav"&&d.diagnostics.size()==2,"Reserved notifications excluded case-insensitively before decoding/cache/history");
}
int main(){auto root=std::filesystem::path("fixtures/notifications")/std::to_string(GetCurrentProcessId());std::filesystem::create_directories(root);unit();integration(root);configuration(root);check(allocations==0,"No callback C++ allocations");printf("PASS notification checks=%llu audited_allocations=%llu latest_commit_bounded actual_core_mix playlist_cursor_intact fallback config exclusion\n",static_cast<unsigned long long>(checks),static_cast<unsigned long long>(allocations.load()));}
