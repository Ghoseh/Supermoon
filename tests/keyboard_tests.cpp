// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "playlist/stream_engine.hpp"
#include "playlist/keyboard_settings.hpp"
#include "playlist/windows_keyboard.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <new>
#include <set>

using namespace bytebeat::playlist;
std::atomic<unsigned> checks{},normal_new{};thread_local bool audio_audit{};
void* operator new(size_t n){if(audio_audit)++normal_new;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept{std::free(p);}void operator delete(void* p,size_t) noexcept{std::free(p);}
void check(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL %s\n",why);std::abort();}}
template<class F> void wait(F f){auto limit=std::chrono::steady_clock::now()+std::chrono::seconds(8);while(!f()){if(std::chrono::steady_clock::now()>limit)check(false,"Own fixture timeout");Sleep(1);}}
template<class F> void refused(F f){bool caught=false;try{f();}catch(...){caught=true;}check(caught,"Expected invalid configuration/state refusal");}
std::string read_file(const std::filesystem::path& path){std::ifstream in(path,std::ios::binary);return {(std::istreambuf_iterator<char>(in)),{}};}
void text(const std::filesystem::path& path,const std::string& value){std::ofstream out(path,std::ios::binary);out<<value;out.close();check(bool(out),"Fixture text write");}
Engine::Frame ready(Engine& e){Engine::Frame f;wait([&]{audio_audit=true;f=e.peek();audio_audit=false;return f.pcm;});return f;}
void consume(Engine& e,uint64_t count){for(uint64_t i=0;i<count;++i){auto f=ready(e);audio_audit=true;auto used=e.consume(f);audio_audit=false;check(used,"Exactly consumed own PCM frame");}}
Engine::Frame change(Engine& e,uint64_t revision){Engine::Frame f;wait([&]{f=e.peek();return f.pcm&&f.revision!=revision;});return f;}

void handles(const char* stage){DWORD n{};GetProcessHandleCount(GetCurrentProcess(),&n);std::fprintf(stderr,"HANDLES %s %lu\n",stage,n);}
void inputs(){
    KeyboardConfiguration cfg;InputContext c{true,true,false,3};std::array<bool,4> keys{};KeyboardEdges edges;unsigned sent[4]{};
    auto send=[&](KeyAction a){++sent[unsigned(a)];};
    keys[0]=true;edges.poll(cfg,c,keys,send);check(sent[0]==0,"Initial held primary not armed");
    keys[0]=false;edges.poll(cfg,c,keys,send);keys[0]=true;edges.poll(cfg,c,keys,send);
    for(unsigned i=0;i<100;++i)edges.poll(cfg,c,keys,send);check(sent[0]==1,"Held key sends one command");
    c.foreground=false;edges.poll(cfg,c,keys,send);c.foreground=true;edges.poll(cfg,c,keys,send);check(sent[0]==1,"Held Alt-Tab return never replays");
    keys={};edges.poll(cfg,c,keys,send);c.carrier=false;keys[1]=true;edges.poll(cfg,c,keys,send);c.carrier=true;edges.poll(cfg,c,keys,send);check(sent[1]==0,"Inactive Dreamers edge not replayed on return");
    keys={};edges.poll(cfg,c,keys,send);c.captured=true;keys[2]=true;edges.poll(cfg,c,keys,send);c.captured=false;edges.poll(cfg,c,keys,send);check(sent[2]==0,"Captured/menu/text edge not replayed");
    keys={};edges.poll(cfg,c,keys,send);c.modifiers=7;keys[3]=true;edges.poll(cfg,c,keys,send);c.modifiers=3;edges.poll(cfg,c,keys,send);check(sent[3]==0,"Extra Alt then modifier change while held refuses");
    keys={};edges.poll(cfg,c,keys,send);keys[0]=keys[1]=true;edges.poll(cfg,c,keys,send);check(sent[0]==1&&sent[1]==0,"Ambiguous simultaneous primary keys refuse");
    for(unsigned i=0;i<4;++i){keys={};edges.poll(cfg,c,keys,send);keys[i]=true;edges.poll(cfg,c,keys,send);check(sent[i]==(i?1u:2u),"All four independent input actions");}
    cfg.enabled=false;keys={};edges.poll(cfg,c,keys,send);keys[0]=true;edges.poll(cfg,c,keys,send);check(sent[0]==2,"Disabled controls refuse");
}
void order_cases(){
    Order normal({10,20,30},1,false);check(normal.next()==10&&normal.next()==20,"Normal library sequence");
    normal.set_shuffle(true);check(normal.current()==20,"Toggle never advances current identity");auto shuffled=normal.next();check(shuffled!=20,"Enabled mode next shuffled without immediate repeat");
    normal.set_shuffle(false);check(normal.current()==shuffled,"Disable preserves current identity");auto expected=shuffled==30?10:shuffled+10;check(normal.next()==expected,"Disable resumes normal order from current");
    auto jumped=normal.shuffle_next();check(jumped!=expected&&!normal.shuffled(),"Shuffle-and-advance independent of persistent mode");
    auto next=jumped==30?10:jumped+10;check(normal.next()==next,"Normal order resumes after one-shot shuffle");
    Order history({0,1,2,3},4);auto a=history.next();auto b=history.next();auto d=history.shuffle_next();check(d!=b,"Shuffle action chooses different playable song");
    check(history.previous()==b&&history.previous()==a,"Actual shuffle history navigation");check(history.next()==b&&history.next()==d,"Forward history remains coherent");
    Order single({7},5,false);check(single.next()==7&&single.previous()==7&&single.shuffle_next()==7&&!single.shuffled(),"Single-song mode/actions safe");
    Order empty({},0);check(empty.empty(),"Empty order recognized");refused([&]{empty.next();});refused([&]{empty.shuffle_next();});
    for(unsigned mode=0;mode<2;++mode){Order cycle({0,1,2,3},34,bool(mode));uint32_t last=99;for(unsigned n=0;n<20;++n){std::set<uint32_t> seen;for(unsigned j=0;j<4;++j){auto id=cycle.next();if(!j&&n)check(id!=last,"No cross-cycle immediate repeat");seen.insert(id);last=id;}check(seen.size()==4,"Complete no-replacement/sequential cycle");}}
}
void settings(const std::filesystem::path& root){
    auto cfg=keyboard_configuration(root);check(cfg.enabled&&cfg.bindings[0].key==0x75&&cfg.bindings[3].key==0x78,"Approved four default keys");
    text(root/"playlist.ini","[Keyboard]\nEnabled=1\nPrevious=Ctrl+Shift+F1\nNext=Ctrl+Shift+F2\nToggleShuffle=Ctrl+Shift+F3\nShuffleAndAdvance=None\n");
    cfg=keyboard_configuration(root);check(cfg.bindings[0].key==0x70&&cfg.bindings[2].key==0x72&&!cfg.bindings[3].key,"All shortcuts configurable/individually disabled");
    for(auto value:{"Next=Ctrl+Shift+F6","Next=F7","Next=Ctrl+Shift+F12","Enabled=maybe","Next=Ctrl+Shift+F25"}){text(root/"playlist.ini",std::string("[Keyboard]\n")+value+"\n");refused([&]{keyboard_configuration(root);});}
    auto dir=root/"State";
    {ShuffleStore store(dir);check(store.available()&&store.mode(),"Missing state preserves prior shuffle-on default");check(store.save(false)&&!store.mode(),"Persist shuffle off");ShuffleStore busy(dir);check(!busy.available()&&!busy.save(true),"Exclusive preference ownership");}
    {ShuffleStore store(dir);check(store.available()&&!store.mode(),"Restore preference across launches");auto before=read_file(dir/"shuffle-mode.txt");
        HANDLE hold=CreateFileW((dir/"shuffle-mode.txt").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);check(hold!=INVALID_HANDLE_VALUE,"Own sharing failure fixture");
        check(!store.save(true)&&!store.mode()&&read_file(dir/"shuffle-mode.txt")==before,"Failed atomic publication keeps old preference");CloseHandle(hold);check(store.save(true),"Preference failure recoverable");}
    text(dir/"shuffle-mode.txt.pending","interrupted");{ShuffleStore store(dir);check(store.available()&&store.mode(),"Interrupted pending file never becomes preference");check(store.save(false),"Pending state safely replaced on next save");}
    text(dir/"shuffle-mode.txt","corrupt");{ShuffleStore store(dir);check(!store.available()&&!store.save(false)&&read_file(dir/"shuffle-mode.txt")=="corrupt","Corrupt preference retained and not guessed/overwritten");}
}
std::vector<Prepared> make_tracks(const std::filesystem::path& root,unsigned count,uint64_t length){
    std::vector<Prepared> result;for(unsigned n=0;n<count;++n){auto path=root/(std::to_string(n)+".pcm");std::ofstream out(path,std::ios::binary);for(uint64_t f=0;f<length;++f){float sample=.1f+float(n)*.1f+float(f%512)*.000001f;out.write(reinterpret_cast<char*>(&sample),4);}out.close();check(bool(out),"Own PCM publication");result.push_back({path,length,0,1,"fixture"});}return result;
}
void engine_cases(const std::filesystem::path& root){
    auto tracks=make_tracks(root,4,100000);bool saved=false;unsigned writes=0;
    Engine engine(tracks,9,0,false,[&](bool mode){saved=mode;++writes;return true;});engine.set_active(true);
    auto f=ready(engine);check(f.track==0&&f.position==0,"Sequential initial song");consume(engine,1);
    auto revision=f.revision;check(engine.command(Command::toggle_shuffle),"Toggle admitted");wait([&]{return engine.shuffle_enabled();});f=ready(engine);check(f.revision==revision,"Pure toggle preserves queued revision");
    check(f.track==0&&f.position==1&&engine.shuffle_enabled()&&saved&&writes==1,"Toggle preserves WAV position and persists");consume(engine,1);
    check(engine.command(Command::toggle_shuffle),"Disable toggle admitted");wait([&]{return !engine.shuffle_enabled();});f=ready(engine);check(f.revision==revision,"Pure disable preserves queued revision");
    check(f.track==0&&f.position==2&&!engine.shuffle_enabled()&&!saved&&writes==2,"Disable preserves current WAV position");
    check(engine.command(Command::next),"Next admitted");f=change(engine,f.revision);check(f.track==1&&f.position==0,"Next follows current library order");
    consume(engine,72000);check(engine.command(Command::previous),"Exactly3sec Previous admitted");f=change(engine,f.revision);check(f.track==0&&f.position==0,"At3sec Previous navigates");
    check(engine.command(Command::next),"Next to test threshold");f=change(engine,f.revision);check(f.track==1,"Threshold test song");
    consume(engine,72001);check(engine.command(Command::previous),"Beyond3sec Previous admitted");f=change(engine,f.revision);check(f.track==1&&f.position==0,"Beyond3sec Previous restarts current WAV");
    consume(engine,1);check(engine.command(Command::shuffle),"Shuffle-and-advance admitted");f=change(engine,f.revision);check(f.track!=1&&!engine.shuffle_enabled()&&writes==2,"One-shot shuffle leaves persistent mode untouched");
    consume(engine,1);auto track=f.track;check(engine.command(Command::next),"Next after one-shot admitted");f=change(engine,f.revision);check(f.track==(track+1)%4,"Off-mode Next resumes from randomly selected current song");
    engine.set_active(false);check(!engine.command(Command::next)&&!engine.peek().pcm,"Inactive/native selection rejects external commands");
    engine.restart();engine.set_active(true);f=change(engine,f.revision);check(f.position==0,"Native return retains current-WAV restart contract");
    engine.request_stop();check(!engine.active()&&!engine.command(Command::toggle_shuffle),"Retired resident worker rejects commands");
}
void delayed_persistence(const std::vector<Prepared>& tracks){
    std::atomic<bool> entered{false},release{false};
    Engine engine(tracks,14,0,false,[&](bool){entered=true;wait([&]{return release.load();});return true;});engine.set_active(true);
    auto f=ready(engine);consume(engine,1000);check(engine.command(Command::toggle_shuffle),"Slow toggle admitted");wait([&]{return entered.load();});
    auto revision=f.revision;consume(engine,5000);release=true;wait([&]{return engine.shuffle_enabled();});f=ready(engine);check(f.track==0&&f.position==6000&&f.revision==revision,"Slow persistence preserves queued PCM without revision/reseek");
}
void scoped_queue(const std::vector<Prepared>& tracks){
    std::atomic<bool> entered{false},release{false};
    Engine engine(tracks,14,0,false,[&](bool){entered=true;wait([&]{return release.load();});return true;});engine.set_active(true);
    auto f=ready(engine);consume(engine,1);check(dispatch_key(engine,KeyAction::toggle_shuffle),"Key dispatcher reaches toggle");wait([&]{return entered.load();});
    check(dispatch_key(engine,KeyAction::next),"Queued Next before selection away");engine.set_active(false);engine.restart();engine.set_active(true);release=true;
    f=change(engine,f.revision);check(f.track==0&&f.position==0&&engine.stats().rejected>=1,"Old queued key cannot cross native selection/restart epoch");
    consume(engine,1);check(dispatch_key(engine,KeyAction::next),"Key dispatcher reaches Next");f=change(engine,f.revision);auto previous=f.track;consume(engine,1);
    check(dispatch_key(engine,KeyAction::shuffle_advance),"Key dispatcher reaches shuffle-and-advance");f=change(engine,f.revision);check(f.track!=previous,"Dispatch advances a different song");consume(engine,1);
    check(dispatch_key(engine,KeyAction::previous),"Key dispatcher reaches Previous");f=change(engine,f.revision);check(f.track==previous,"Dispatch Previous uses actual shuffle history");
    check(!dispatch_key(engine,KeyAction(255)),"Invalid key action refused");
}
void producers(const std::vector<Prepared>& tracks){
    Engine engine(tracks,17,0,false);engine.set_active(true);consume(engine,1);
    std::atomic<unsigned> accepted{},rejected{},done{};std::thread threads[4];
    for(unsigned p=0;p<4;++p)threads[p]=std::thread([&,p]{for(unsigned i=0;i<1000;++i){if(dispatch_key(engine,KeyAction((i+p)%4)))++accepted;else ++rejected;}++done;});
    wait([&]{auto frame=engine.peek();if(frame.pcm){audio_audit=true;engine.consume(frame);audio_audit=false;}return done==4;});
    for(auto& thread:threads)thread.join();consume(engine,10);
    check(accepted+rejected==4000&&accepted>0&&engine.healthy()&&engine.stats().failed==0,"Concurrent bounded control admission and PCM consumption");
}
void child_persistence(const std::filesystem::path& root){
    wchar_t exe[32768]{};check(GetModuleFileNameW(nullptr,exe,32768)!=0,"Own child executable path");
    auto path=root/"ChildState";std::wstring command=L"\""+std::wstring(exe)+L"\" --write-off \""+path.wstring()+L"\"";
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    check(CreateProcessW(exe,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Own separate-process preference write");
    check(WaitForSingleObject(process.hProcess,10000)==WAIT_OBJECT_0,"Own preference child joined");DWORD code{};GetExitCodeProcess(process.hProcess,&code);CloseHandle(process.hThread);CloseHandle(process.hProcess);
    check(code==0,"Own preference child passed");ShuffleStore store(path);check(store.available()&&!store.mode(),"Preference restored from another process");
}
int wmain(int argc,wchar_t** argv){
    if(argc==3&&!wcscmp(argv[1],L"--write-off")){ShuffleStore store(argv[2]);return store.available()&&store.save(false)?0:2;}
    try{
    DWORD before{},after{};
    auto root=std::filesystem::absolute(std::filesystem::path("fixtures/fixtures")/std::to_string(GetCurrentProcessId()));std::filesystem::create_directories(root);
    handles("before child first use");child_persistence(root);handles("after child first use");
    check(GetProcessHandleCount(GetCurrentProcess(),&before)!=0,"Initial handles after child-launch API initialization");
    for(unsigned i=0;i<3;++i){child_persistence(root);DWORD n{};GetProcessHandleCount(GetCurrentProcess(),&n);check(n==before,"Repeated child launch closes exact handles");}
    inputs();order_cases();settings(root);handles("settings");
    auto pcm=root/"pcm";std::filesystem::create_directories(pcm);engine_cases(pcm);handles("engine");
    delayed_persistence(make_tracks(pcm,4,100000));handles("delayed");
    scoped_queue(make_tracks(pcm,4,100000));handles("scoped");producers(make_tracks(pcm,4,100000));handles("producers");child_persistence(root);handles("child");
    refused([&]{Engine empty({});});
    {auto single=make_tracks(pcm,1,17);Engine e(single,0,0,false);e.set_active(true);consume(e,34);check(e.stats().completed==2,"Single-song EOF repetition");}
    {Engine e(make_tracks(pcm,1,100000),0,0,false,[](bool){return false;});e.set_active(true);ready(e);consume(e,1);check(e.command(Command::toggle_shuffle),"Failed store toggle queued");wait([&]{return e.shuffle_failures()==1;});check(!e.shuffle_enabled()&&e.healthy()&&e.stats().failed==0,"Persistence failure leaves mode/audio intact, no quarantine");}
    {Engine e(make_tracks(pcm,1,100000),0,0);KeyboardConfiguration cfg;cfg.enabled=false;WindowsKeyboard controls(e,cfg);e.request_stop();}
    check(normal_new==0,"Zero audited callback allocations");check(GetProcessHandleCount(GetCurrentProcess(),&after)!=0,"Final handle sample");
    std::fprintf(stderr,"OWN handle comparison before=%lu after=%lu\n",before,after);check(before==after,"Exact handles after private workers/store ownership");
    std::printf("PASS keyboard checks=%u handles=%lu->%lu callbacks_new=0 four_edges=PASS config=PASS persistence=PASS shuffle_mode=PASS previous3sec=PASS continuity=PASS\n",checks.load(),before,after);
    std::puts("LIMITS: owned fixtures and deterministic input contexts; no NMS focus/overlay/binding/live keyboard verification. LUFS is qualified separately by the consolidated cache/audio test.");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"FAIL exception %s\n",e.what());return 1;}}
