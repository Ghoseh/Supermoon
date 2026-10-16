// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "supermoon_adapter_api.hpp"
#include "native_clone.hpp"
#include "wav_reader.hpp"
#include "pinned_build.hpp"
#include <array>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <set>

void AdapterTestConfigureEngine(AdapterProviderFn,bytebeat::playlist::Engine&,uintptr_t,const bytebeat::CompositionKey&);

extern "C" void FixtureControl(uint32_t,void*,void*,void*,const void*);
namespace {
using namespace bytebeat;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
template<class T> void put(void* p,size_t n,T v){memcpy(static_cast<unsigned char*>(p)+n,&v,sizeof(v));}
template<class T> T get(const void* p,size_t n){T v;memcpy(&v,static_cast<const unsigned char*>(p)+n,sizeof(v));return v;}
std::filesystem::path fixture_executable;
CompositionKey moon{'T','E','S','T','0','1'},dream{'T','E','S','T','0','2'};
thread_local unsigned lock_depth{};
extern "C" void fixture_lock(void* p,uint32_t){EnterCriticalSection(static_cast<CRITICAL_SECTION*>(p));++lock_depth;}
extern "C" void fixture_unlock(void* p){require(lock_depth==1,"Native lock pairing failed");--lock_depth;LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(p));}

struct Harness {
    HANDLE file{},mapping{};unsigned char* image{};NativeClone clone;
    std::vector<unsigned char> object=std::vector<unsigned char>(0x8000),catalog=std::vector<unsigned char>(0x320);
    AdapterProviderFn native{};
    Harness(){
        file=CreateFileW(fixture_executable.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        require(file!=INVALID_HANDLE_VALUE,"Fixture executable unavailable");
        mapping=CreateFileMappingW(file,nullptr,PAGE_READONLY|SEC_IMAGE,0,0,nullptr);
        image=static_cast<unsigned char*>(MapViewOfFile(mapping,FILE_MAP_READ,0,0,0));require(image,"SEC_IMAGE fixture failed");
        // Private copy-on-write data values, not disk writes or a game process.
        DWORD old{},ignored{};require(VirtualProtect(image+0x4f77568,8,PAGE_WRITECOPY,&old)!=0,"Fixture globals COW failed");
        put<float>(image,0x4f77568,.125f);put<uint32_t>(image,0x4f7756c,4);
        require(VirtualProtect(image+0x4f77568,8,old,&ignored)!=0,"Fixture global protection restore failed");
        void* bridges[]={reinterpret_cast<void*>(&AdapterFrameBridge),reinterpret_cast<void*>(&AdapterBeforeBridge),reinterpret_cast<void*>(&AdapterAfterBridge),reinterpret_cast<void*>(&AdapterCommitBridge),reinterpret_cast<void*>(&AdapterRetireBridge),reinterpret_cast<void*>(&AdapterCarrierBridge)};
        clone.prepare(image,bridges);
        // Fixture only: replace the three native lock calls in private clone
        // with Windows critical-section stubs. All timing/fade/production and
        // completed-frame instructions remain the actual pinned native code.
        auto mem=const_cast<unsigned char*>(clone.memory());require(VirtualProtect(mem,layout::allocation_size,PAGE_READWRITE,&old)!=0,"Fixture clone write failed");
        uint32_t stub=layout::allocation_size-32;
        for(unsigned k=0;k<2;++k){auto p=mem+stub+k*16;p[0]=0xff;p[1]=0x25;memset(p+2,0,4);void* fn=k?reinterpret_cast<void*>(&fixture_unlock):reinterpret_cast<void*>(&fixture_lock);memcpy(p+6,&fn,8);}
        for(const auto& i:layout::instructions)if(i.rva==0xf6c686||i.rva==0xf6c699||i.rva==0xf6d6bc){int32_t rel=int32_t(stub+(i.rva==0xf6c686?0:16)-i.offset-i.size);put<int32_t>(mem,i.offset+i.field,rel);}
        require(VirtualProtect(mem,layout::allocation_size,old,&ignored)!=0,"Fixture clone RX failed");FlushInstructionCache(GetCurrentProcess(),mem,layout::allocation_size);
        auto fn=clone.function(0);memcpy(&native,&fn,sizeof(native));
        InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object.data()+0x7d30));
    }
    ~Harness(){DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object.data()+0x7d30));UnmapViewOfFile(image);CloseHandle(mapping);CloseHandle(file);}
    void* stream(){return object.data()+0x28;}
    void configure(const std::vector<float>& song,bool pcm=true){AdapterTestResetJournal();AdapterTestConfigure(native,&song,reinterpret_cast<uintptr_t>(image),moon,pcm);}
    void commit(CompositionKey key,uint32_t clock=0,bool personal=true){
        put<uint32_t>(stream(),0x10,clock);put<uint8_t>(stream(),0x14,1);put<uint8_t>(stream(),0x15,1);put<uint8_t>(stream(),0x834,120);put<float>(stream(),0x18,1);put<uint32_t>(stream(),0x1c,0);put<uint8_t>(stream(),0x20,0);
        memcpy(catalog.data()+0x20,key.data(),16);
        auto controller=object.data()+(personal?0x7d5c:0x7d50);
        FixtureControl(0,object.data(),controller,reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(image)+0x6eb1430),key.data());
        FixtureControl(1,object.data(),controller,nullptr,catalog.data());FixtureControl(2,object.data(),controller,nullptr,catalog.data()+0x40);
    }
    uint32_t call(float* out,uint32_t n){uint32_t produced=0;AdapterTestProvider(stream(),out,n,&produced);require(!lock_depth,"Lock leaked");return produced;}
};
void test_streaming_provider(Harness& h){
    using namespace bytebeat::playlist;
    auto root=std::filesystem::path("build/playlist-lufs/native-stream-fixtures")/std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());std::filesystem::create_directories(root);
    std::vector<Prepared> tracks;
    for(unsigned id=0;id<4;++id){auto p=root/(std::to_string(id)+".pcm");std::ofstream f(p,std::ios::binary);float sample=.1f+float(id)*.1f;uint64_t frames=1333+id*31;for(uint64_t i=0;i<frames;++i)f.write(reinterpret_cast<const char*>(&sample),4);f.close();require(bool(f),"Stream fixture write");tracks.push_back({p,frames,0,1,"fixture"});}
    Engine engine(tracks,51);AdapterTestResetJournal();AdapterTestConfigureEngine(h.native,engine,reinterpret_cast<uintptr_t>(h.image),moon);
    h.commit(moon,6143990);std::set<uint32_t> cycle;unsigned completed=0;uint64_t pcm=0;uint32_t prior=UINT32_MAX;
    auto wait=[&]{auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(std::chrono::steady_clock::now()<deadline){auto f=engine.peek();if(f.pcm)return f;require(engine.healthy(),"Native stream worker failed");std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Native stream timeout");};
    while(completed<12){auto frame=wait();if(frame.position==0){if(completed%4==0){cycle.clear();require(prior!=frame.track,"Native stream boundary repeat");}require(cycle.insert(frame.track).second,"Native stream duplicate cycle");}
        float output=99;require(h.call(&output,1)==1&&output==SupermoonCore::processed(frame.sample,1,1),"Actual native streamed PCM/envelope");++pcm;
        require(engine.stats().pcm_frames==pcm,"Native streamed cursor not consumed exactly once");if(frame.last){++completed;prior=frame.track;}}
    require(get<uint32_t>(h.stream(),0x10)==6143990+pcm&&!get<uint8_t>(h.stream(),0x20),"Native playlist clock/carrier lifetime");
    // Provider capacity is the requested frame count, including slots touched
    // before a partial-production exit. A one-float buffer for request=2 was a
    // latent fixture overrun masked by the previous compiler's stack layout.
    std::array<float,3> output_buffer{99,99,77};float& output=output_buffer[0];
    auto before=engine.stats().pcm_frames;put<uint8_t>(h.stream(),0x14,0);require(!h.call(&output,1)&&engine.stats().pcm_frames==before,"Native stop consumed queue");
    h.commit(dream,6144000);require(!engine.active()&&!engine.command(Command::next)&&h.call(&output,2)==1&&output_buffer[2]==77,"Native Dreamers scope/gate/capacity");
    h.commit(moon,UINT32_MAX-3);auto frame=wait();require(frame.position==0,"Native successful commit reset song");for(unsigned i=0;i<8;++i){frame=wait();require(h.call(&output,1)==1&&output==SupermoonCore::processed(frame.sample,1,1),"Native stream uint32 wrap PCM");}require(get<uint32_t>(h.stream(),0x10)==4,"Native stream uint32 wrap clock");
    auto serial=frame.serial;require(engine.command(Command::next),"Native Next command");auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    do{frame=wait();if(frame.serial!=serial)break;std::this_thread::sleep_for(std::chrono::milliseconds(1));}while(std::chrono::steady_clock::now()<deadline);
    require(frame.serial!=serial&&frame.position==0,"Native Next generation");put<float>(h.stream(),0x18,.25f);require(h.call(&output,1)==1&&output==SupermoonCore::processed(frame.sample,1,.25f),"Native streamed gain");
    // Two actual providers use independent native fixture locks. The unbound
    // stream must not contend on our sidecar or consume the bound playlist.
    Harness foreign;put<uint32_t>(foreign.stream(),0x10,0);put<uint8_t>(foreign.stream(),0x14,1);put<uint8_t>(foreign.stream(),0x15,1);put<uint8_t>(foreign.stream(),0x834,120);put<float>(foreign.stream(),0x18,1);
    std::atomic<bool> failed{false};std::thread unrelated([&]{try{for(unsigned i=0;i<10000;++i){float out=99;require(foreign.call(&out,1)==1&&out==0,"Foreign stream substituted");}}catch(...){failed.store(true);}});
    struct Join {std::thread& thread;~Join(){if(thread.joinable())thread.join();}}join{unrelated};
    for(unsigned i=0;i<10000;++i){frame=wait();require(h.call(&output,1)==1&&output==SupermoonCore::processed(frame.sample,1,.25f),"Bound concurrent native PCM");}
    unrelated.join();require(!failed.load(),"Concurrent foreign stream failed");
    FixtureControl(3,h.object.data(),nullptr,nullptr,nullptr);require(!engine.active()&&!engine.command(Command::next),"Retirement retains stale stream input");
    auto retire_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!engine.worker_exited()&&std::chrono::steady_clock::now()<retire_deadline)std::this_thread::sleep_for(std::chrono::milliseconds(1));
    require(engine.worker_exited(),"Retire bridge must request worker exit");
    auto retired_pcm=engine.stats().pcm_frames;
    require(h.call(&output,1)==1&&output==0&&engine.stats().pcm_frames==retired_pcm,"Native fallback after retirement consumes no custom PCM");
    for(uint32_t i=0;i<AdapterCount();++i){AdapterRecord r{};require(AdapterRead(i,&r)&&!r.faults,"Stream integration adapter fault");}
    std::cout<<"actual_native_streaming=PASS twelve actual EOF transitions/three shuffle cycles across clock 6144000, exact queue PCM/native gain, stop/switch/reentry/Next/retirement, uint32 wrap, simultaneous bound/unrelated native callbacks; fixture locks and disabled synthesis lanes\n";
}
}
LONG WINAPI fixture_exception(EXCEPTION_POINTERS* e){
    MEMORY_BASIC_INFORMATION info{};VirtualQuery(reinterpret_cast<void*>(e->ContextRecord->Rip),&info,sizeof(info));
    char message[512]{};snprintf(message,sizeof(message),"fixture_exception code=%lx rip=%llx rva=%llx image=%llx address=%llx rax=%llx rcx=%llx r15=%llx\n",e->ExceptionRecord->ExceptionCode,
        static_cast<unsigned long long>(e->ContextRecord->Rip),static_cast<unsigned long long>(e->ContextRecord->Rip-reinterpret_cast<uintptr_t>(info.AllocationBase)),static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(info.AllocationBase)),
        static_cast<unsigned long long>(e->ExceptionRecord->ExceptionInformation[1]),
        static_cast<unsigned long long>(e->ContextRecord->Rax),static_cast<unsigned long long>(e->ContextRecord->Rcx),static_cast<unsigned long long>(e->ContextRecord->R15));
    DWORD written{};WriteFile(GetStdHandle(STD_ERROR_HANDLE),message,DWORD(strlen(message)),&written,nullptr);return EXCEPTION_EXECUTE_HANDLER;
}
int wmain(int argc,wchar_t** argv){try{
    SetUnhandledExceptionFilter(fixture_exception);
    require(argc==2,"Usage: native_stream_tests.exe <owned pinned NMS.exe path>");fixture_executable=argv[1];
    require(bytebeat::pinned_executable_file(fixture_executable.c_str()),"Unsupported native fixture layout");
    Harness h;test_streaming_provider(h);
    std::cout<<"PASS new cache candidate native streaming contract; fixture locks/disabled lanes; game image mapped as data, NMS not launched\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
