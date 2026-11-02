// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
// Payload activation is explicit; the proxy schedules it on its worker.
// Production recovery publishes lifecycle/forwarding hooks before cold preparation.
// PCM/Core access remains gated until the prepared resident Engine is adopted.
#define ADAPTER_BUILD
#include "supermoon_adapter_api.hpp"
extern "C" void AdapterStateLockBridge();
extern "C" { void* StateNativeLock=nullptr; }

#include "supermoon_core.hpp"
#include "music_control_prototype.hpp"
extern "C" void FloorControlUnlockBridge();
extern "C" { void* FloorNativeUnlock=nullptr; }

#include "native_clone.hpp"
#include "pinned_build.hpp"
#include <atomic>
#include <cstdio>
#include <cwchar>
#include <new>
#include "playlist/resident_handoff.hpp"
#include "endurance_writer.hpp"
#ifdef ADAPTER_RECOVERY_BUILD
#include "recovery_native.hpp"
#include "recovery_pins.hpp"
#endif
#ifdef ADAPTER_SELFTEST_BUILD
namespace bytebeat::playback {inline bool location_matches(const wchar_t*){return false;}}
#endif
#ifdef ADAPTER_PLAYBACK_BUILD
#include "supermoon_playback_payload.hpp"
#include "observer.hpp"
#include "time.hpp"
#endif

namespace {
constexpr uint32_t capacity=262144;
supermoon::floor_prototype::MusicControl floor_control;
bytebeat::CompositionKey floor_carrier_key{};
HANDLE floor_bank_handle=INVALID_HANDLE_VALUE;
uint32_t floor_index(uintptr_t token) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    auto s=bytebeat::recovery::registry.source(reinterpret_cast<void*>(token));
    if(s)return uint32_t(s-bytebeat::recovery::registry.sources);
#else
    (void)token;
#endif
    return UINT32_MAX;
}

enum Kind:uint32_t { callback=1,selection=2,resolved=3,commit_event=4,retire_event=5 };
enum Fault:uint32_t { context_fault=1,key_fault=2,scope_fault=4,clock_fault=8,reentry_fault=16,production_fault=32,binding_fault=64 };
struct Slot {AdapterRecord value{};std::atomic<bool> ready{false};};
static_assert(sizeof(Slot)==128); // fixed 32 MiB journal, no wrap/reuse
Slot records[capacity];
struct PlaybackCounts {uint32_t substituted{},eof{},native{};};
#ifdef ADAPTER_PLAYBACK_BUILD
PlaybackCounts playback_counts[capacity]; // published by the same ready flag
#endif
// Small immutable label journal: copy fixed strings while the resolver's native
// record is valid, then let the worker serialize them. No later pointer chasing
// or audio-thread file access. Labels explain observed keys; they never arm PCM.
struct CatalogLabel {uint8_t key[16]{},name[32]{},locid[32]{};std::atomic<bool> ready{false};};
CatalogLabel catalog_labels[64];
std::atomic<uint32_t> catalog_label_count{0},catalog_label_dropped{0};
// Producers reserve a unique slot, fill it, then publish ready with release
// ordering. The writer's acquire read sees the complete record. A slow producer
// may temporarily hold up the readable prefix; slots are never recycled.
std::atomic<uint64_t> sequence{0},dropped{0},call_id{0};
std::atomic<uint32_t> activation{0};
std::atomic<bool> finished{false};
bytebeat::SupermoonCore initial_core(nullptr);
bytebeat::SupermoonCore* core=&initial_core; // Set at adoption; engine_ready gates early hooks.
std::atomic<bytebeat::playlist::Engine*> engine_log{nullptr};
AdapterProviderFn provider{}; // immutable before publication
uintptr_t native_base{};
#ifdef ADAPTER_RECOVERY_BUILD
std::atomic<uintptr_t> deferred_retirement{0};
std::atomic<bool> engine_ready{false};
bool endpoint_live(uintptr_t token) noexcept {return bytebeat::recovery::Registry::live(bytebeat::recovery::registry.source(reinterpret_cast<void*>(token)));}
#endif
HANDLE log_file=INVALID_HANDLE_VALUE,writer_ready{};
bytebeat::NativeClone clone;
struct Pending {uintptr_t component{},payload{};bytebeat::CompositionKey key{};bool personal{},valid{};};
struct CallbackContext {void* stream{};float* buffer{};uint32_t request{};uint32_t* produced{};AdapterRecord summary{};uintptr_t endpoint{};PlaybackCounts playback{};bytebeat::endurance::Frames continuity{};};
// TLS means each native call carries its own borrowed arguments. Pending holds
// copied key bytes and opaque address values only; neither TLS object owns NMS
// memory. Native pointers may be read only in their reviewed execution context.
thread_local Pending pending;
thread_local CallbackContext* current;
static_assert(std::atomic<uint64_t>::is_always_lock_free);
template<class T> T read(const void* object,size_t offset) noexcept{T value;memcpy(&value,static_cast<const unsigned char*>(object)+offset,sizeof(value));return value;}
#ifdef ADAPTER_SELFTEST_BUILD
bool check_live_unwind{};
extern "C" void FixtureFrameReturn();
// Own-process fixture only, never compiled into the production DLL.
__declspec(noinline) void verify_live_unwind(void* stream,const unsigned char* native_stack) noexcept {
    CONTEXT context{};RtlCaptureContext(&context);bool ok=false;
    for(unsigned depth=0;depth<6;++depth){
        if(context.Rip==reinterpret_cast<DWORD64>(&FixtureFrameReturn)) {ok=context.Rsp==reinterpret_cast<DWORD64>(native_stack)&&context.R15==reinterpret_cast<DWORD64>(stream)&&context.Rbp==read<uint64_t>(native_stack,0x1A8);break;}
        DWORD64 image_base{};auto entry=RtlLookupFunctionEntry(context.Rip,&image_base,nullptr);
        if(!entry)break;PVOID data{};DWORD64 frame{};
        RtlVirtualUnwind(UNW_FLAG_NHANDLER,image_base,context.Rip,entry,&context,&data,&frame,nullptr);
    }
    if(!ok){fputs("FAIL live bridge unwind\n",stderr);abort();}
}
#endif
uint64_t ticks() noexcept{LARGE_INTEGER value{};QueryPerformanceCounter(&value);return uint64_t(value.QuadPart);}
void append(AdapterRecord value,const PlaybackCounts* counts=nullptr) noexcept {
    auto n=sequence.fetch_add(1,std::memory_order_relaxed);
    if(n>=capacity){dropped.fetch_add(1,std::memory_order_relaxed);return;}
    value.sequence=n;value.qpc=value.qpc?value.qpc:ticks();value.thread=GetCurrentThreadId();
#ifdef ADAPTER_PLAYBACK_BUILD
    playback_counts[n]=counts?*counts:PlaybackCounts{};
#else
    (void)counts;
#endif
    records[n].value=value;records[n].ready.store(true,std::memory_order_release);
}
void observed_fault(uint32_t fault) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    core->invalidate();if(current)current->summary.faults|=fault;
}
void adapter_provider(void* stream,float* buffer,uint32_t request,uint32_t* produced) {
#ifdef ADAPTER_RECOVERY_BUILD
    auto source=bytebeat::recovery::invocation;
    auto ticket=bytebeat::recovery::ticket(reinterpret_cast<uintptr_t>(stream));
    if(source&&!ticket){if(produced)*produced=0;return;}
#endif
    // With recovery enabled, the owned Source/lease above attributes this call;
    // a missing/retired ticket is refused before the original provider runs.
    // The admitted wrapper surrounds one native call. Frame bridges inside that call
    // accumulate evidence while the native synthesis lock is held. Afterwards
    // we read only the caller's produced count, never follow the stream object.
    CallbackContext context{};context.stream=stream;context.buffer=buffer;context.request=request;context.produced=produced;
#ifdef ADAPTER_RECOVERY_BUILD
    context.endpoint=bytebeat::recovery::enabled()?reinterpret_cast<uintptr_t>(ticket):0;
    context.summary.component=context.endpoint;
#endif
    context.summary.call=call_id.fetch_add(1,std::memory_order_relaxed);context.summary.qpc=ticks();
    context.summary.stream=reinterpret_cast<uintptr_t>(stream);context.summary.kind=callback;context.summary.requested=request;
    auto previous=current;current=&context;
    bool completed=false;
    // Win64 SEH cleanup, tested under the pinned compiler for C++ and Windows
    // exceptions. Never catch/swallow/retry the original provider or fix output.
    __try {
        if(previous)observed_fault(reentry_fault);
        provider(stream,buffer,request,produced); // exactly once; native buffers/counts unchanged in diagnostics
        context.summary.end_qpc=ticks();context.summary.produced=produced?*produced:0;
        if(!produced||context.summary.produced>request||context.summary.frames!=context.summary.produced)observed_fault(production_fault);
        append(context.summary,&context.playback);
        completed=true;
    } __finally {
        current=previous;
        auto& s=context.summary;
        bytebeat::endurance::publish({s.call,s.qpc,completed?s.end_qpc:ticks(),s.stream,s.generation,
          s.requested,s.produced,s.frames,s.first_clock,s.last_clock,s.faults,!completed,context.continuity});
    }
}
bool memory_digest(unsigned char* bytes,uint32_t length,const uint8_t* expected) {
    BCRYPT_ALG_HANDLE algorithm{};BCRYPT_HASH_HANDLE hash{};uint8_t digest[32]{};
    bool ok=BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptCreateHash(algorithm,&hash,nullptr,0,nullptr,0,0)>=0;
    if(ok)ok=BCryptHashData(hash,bytes,length,0)>=0&&BCryptFinishHash(hash,digest,32,0)>=0;
    if(hash)BCryptDestroyHash(hash);if(algorithm)BCryptCloseAlgorithmProvider(algorithm,0);
    return ok&&!memcmp(digest,expected,32);
}

bool floor_live(uint32_t index) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    auto engine=engine_log.load(std::memory_order_acquire);
    return engine_ready.load(std::memory_order_acquire)&&index<bytebeat::recovery::source_capacity&&
        bytebeat::recovery::Registry::live(&bytebeat::recovery::registry.sources[index])&&
        !core->faulted()&&engine&&engine->healthy();
#else
    (void)index;return false;
#endif
}
void floor_log(uint64_t word,bool desired,int result) noexcept {
    AdapterRecord r{};r.kind=9;r.generation=word>>14;r.component=supermoon::floor_prototype::PlaybackIntent::source_index(word)+1;
    r.requested=desired;r.produced=uint32_t(result);append(r);
}
bool floor_preflight(unsigned char* base,HMODULE module,const wchar_t* executable) {
    (void)module;
    {const uint8_t expected[]={0xff,0xac,0x6b,0xf1,0xc0,0x1f,0xae,0x4c,0x3a,0x2d,0xf2,0x0e,0x05,0xc2,0x0c,0x1d,0xe1,0xaa,0x97,0x60,0x25,0x1c,0x52,0xa6,0x45,0xbf,0xcf,0x8f,0x4e,0xc7,0xdb,0x76};if(!memory_digest(base+46634544,7,expected))return false;}
    {const uint8_t expected[]={0x34,0xf4,0x80,0x00,0x31,0xd1,0xe0,0xa4,0x21,0xde,0x8f,0xbf,0x9d,0xe0,0x1f,0x95,0xa2,0xda,0x09,0x70,0xf5,0x7d,0x34,0x1d,0x28,0x34,0x0e,0x95,0x8b,0x31,0xf3,0xeb};if(!memory_digest(base+52538576,39,expected))return false;}
    {const uint8_t expected[]={0x27,0x04,0xcd,0x72,0x22,0x4b,0x88,0x4b,0xe7,0x1b,0x80,0xb8,0x9f,0x1b,0x26,0x94,0x47,0x61,0x2f,0x9e,0x42,0xc4,0xdd,0x97,0x86,0x1a,0xa9,0x35,0x30,0xc0,0x0c,0x2e};if(!memory_digest(base+52549312,254,expected))return false;}
    {const uint8_t expected[]={0xe6,0x3f,0x8b,0xe6,0x36,0xbf,0x0b,0xc2,0x0d,0x0f,0x5b,0xd9,0xa4,0x47,0xbf,0x15,0x44,0x8e,0x84,0x22,0x02,0x3c,0x73,0x15,0x00,0xa6,0xd8,0x5a,0x5a,0xab,0x56,0x2b};if(!memory_digest(base+52508160,8,expected))return false;}

    auto bank=std::filesystem::path(executable).parent_path().parent_path()/L"GAMEDATA/MODS/Supermoon_MenuAudio_Test/audio/windows/init.bnk";
    HANDLE file=CreateFileW(bank.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    LARGE_INTEGER size{};DWORD got{};std::vector<unsigned char> data(36017);
    const uint8_t expected[]={ 0x65,0xbc,0xbd,0xd3,0xcd,0x9a,0xaa,0xe5,0x13,0xd2,0x18,0x65,0xe5,0x04,0xf1,0x94,0xaf,0xba,0x58,0x4e,0xe0,0x12,0xf9,0xd3,0xd5,0x58,0xe9,0xc0,0x4f,0x71,0xa5,0x4f };
    bool ok=GetFileSizeEx(file,&size)&&size.QuadPart==36017&&ReadFile(file,data.data(),DWORD(data.size()),&got,nullptr)&&got==data.size()&&memory_digest(data.data(),DWORD(data.size()),expected);
    if(!ok){CloseHandle(file);return false;}
    floor_bank_handle=file; // retain a read-only handle until ordinary process exit
#ifdef ADAPTER_PLAYBACK_BUILD
    floor_carrier_key=bytebeat::playback::configuration.carrier;
#endif
    FloorNativeUnlock=base+0x2c79630;
    floor_control.prepare(658155624,reinterpret_cast<supermoon::floor_prototype::MusicControl::Set>(base+0x321acd0),reinterpret_cast<supermoon::floor_prototype::MusicControl::Initialized>(base+0x3213600),&floor_live,&floor_log);
    return true;
}
bool floor_publish(unsigned char* base) noexcept {
    (void)base;floor_control.enable();return true;
}
uint64_t diagnostic_bytes{};bool diagnostic_limit{},diagnostic_io=true;
void write_line(const char* line) {
    constexpr uint64_t limit=64ull*1024*1024;
    DWORD written{},size=DWORD(strlen(line));
    if(diagnostic_limit)return;
    if(diagnostic_bytes+size>limit-1024){
        const char* note="# diagnostic byte limit reached; later records omitted; not shutdown proof\n";
        WriteFile(log_file,note,DWORD(strlen(note)),&written,nullptr);diagnostic_limit=true;return;
    }
    if(!WriteFile(log_file,line,size,&written,nullptr)||written!=size)diagnostic_io=false;
    diagnostic_bytes+=written;
}
DWORD WINAPI writer(void*) {
    // Disk access and polling belong to this worker, never the audio callback.
    // A footer means capture stopped; it does not establish native teardown.
    LARGE_INTEGER frequency{};QueryPerformanceFrequency(&frequency);SYSTEMTIME utc{};GetSystemTime(&utc);
    char line[768];
    bytebeat::endurance::Writer endurance;
    endurance.open(log_file); // worker-owned files; failures reported, not audio changes
#ifdef ADAPTER_PLAYBACK_BUILD
    constexpr const char* mode="playback; PCM prepared";
#else
    constexpr const char* mode="diagnostics only; PCM disabled";
#endif
    snprintf(line,sizeof(line),"# Supermoon adapter %s; capacity=%u; qpc_frequency=%lld; UTC=%04u-%02u-%02uT%02u:%02u:%02u.%03uZ\nsequence,qpc,end_qpc,call,stream_opaque,component_opaque,generation,thread,kind,requested,produced,frames,first_clock,last_clock,faults,min_envelope,max_envelope,min_gain,max_gain,key_hex\n",mode,capacity,frequency.QuadPart,utc.wYear,utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);
    write_line(line);FlushFileBuffers(log_file);SetEvent(writer_ready);
    uint32_t cursor=0,label_cursor=0,last_track=UINT32_MAX;uint64_t last_completed=0,last_position=0,last_serial=0;
    bool worker_scope_reported=false,worker_join_reported=false;uint64_t last_shuffle_failures=UINT64_MAX;int last_shuffle=-1;
    for(;;){
#ifdef ADAPTER_RECOVERY_BUILD
        if(auto token=deferred_retirement.exchange(0,std::memory_order_acq_rel))if(!core->retire_endpoint(token))deferred_retirement.store(token,std::memory_order_release);
#endif
        bytebeat::endurance::Health health{};
        if(auto engine=engine_log.load(std::memory_order_acquire)){
            auto health_stats=engine->stats();bytebeat::playlist::PlaybackSnapshot snapshot{};
            health.pcm=health_stats.pcm_frames;health.underruns=health_stats.underruns;health.completed=health_stats.completed;health.failed=health_stats.failed;
            health.active=engine->active();health.healthy=engine->healthy();health.stop=engine->stop_requested();
            health.worker_exited=engine->worker_exited();health.worker_joined=engine->worker_joined();health.worker_join_failed=engine->worker_join_failed();
            health.snapshot=engine->now_playing_snapshot(snapshot);
            if(health.snapshot){health.track=snapshot.track;health.serial=snapshot.serial;health.revision=snapshot.revision;health.position=snapshot.position;health.length=snapshot.length;}
            auto mode=int(engine->shuffle_enabled());auto failures=engine->shuffle_failures();
            if(mode!=last_shuffle||failures!=last_shuffle_failures){
                snprintf(line,sizeof(line),"# keyboard_shuffle enabled=%d persistence_failures=%llu\n",mode,static_cast<unsigned long long>(failures));write_line(line);last_shuffle=mode;last_shuffle_failures=failures;
            }
            if(engine->worker_exited()&&!worker_scope_reported){
                snprintf(line,sizeof(line),"# playlist_worker_scope_end qpc=%llu; not thread join or native drain\n",static_cast<unsigned long long>(ticks()));write_line(line);worker_scope_reported=true;
            }
            if(engine->worker_exited()&&!worker_join_reported){
                if(engine->join_finished_worker()){
                    snprintf(line,sizeof(line),"# playlist_worker_joined qpc=%llu; std_thread_join_completed; not native callback drain\n",static_cast<unsigned long long>(ticks()));write_line(line);worker_join_reported=true;
                }else if(engine->worker_join_failed()){
                    write_line("# playlist_worker_join_failed; resident state retained; no retry or native clean verdict\n");worker_join_reported=true;
                }
            }
            bytebeat::playlist::PlaybackSnapshot progress;
            if(engine->now_playing_snapshot(progress)&&progress.serial&&
               (progress.serial!=last_serial||progress.position>=last_position+24000||(progress.position==progress.length&&progress.position!=last_position))){
                // Consumed prepared PCM, not native carrier time or downstream
                // speaker position. Tuple is coherent; no filenames/pointers.
                snprintf(line,sizeof(line),"# playlist_position track_id=%u serial=%llu revision=%llu position=%llu length=%llu\n",progress.track,
                    static_cast<unsigned long long>(progress.serial),static_cast<unsigned long long>(progress.revision),
                    static_cast<unsigned long long>(progress.position),static_cast<unsigned long long>(progress.length));write_line(line);
                last_serial=progress.serial;last_position=progress.position;
            }
            auto track=engine->heard_track();auto stats=engine->stats();
            if(track!=UINT32_MAX&&(track!=last_track||stats.completed!=last_completed)){
                // Polling telemetry is approximate and can omit very short tracks.
                // Counts come from consumed PCM, not silence or native clock.
                snprintf(line,sizeof(line),"# playlist_heard track_id=%u pcm_frames=%llu completed=%llu underruns=%llu failed=%llu\n",track,
                    static_cast<unsigned long long>(stats.pcm_frames),static_cast<unsigned long long>(stats.completed),
                    static_cast<unsigned long long>(stats.underruns),static_cast<unsigned long long>(stats.failed));write_line(line);
                last_track=track;last_completed=stats.completed;
            }
        }
        while(label_cursor<64&&catalog_labels[label_cursor].ready.load(std::memory_order_acquire)) {
            const auto& label=catalog_labels[label_cursor];char key[33]{},name[65]{},locid[65]{};
            for(size_t i=0;i<16;++i)snprintf(key+i*2,3,"%02x",label.key[i]);
            for(size_t i=0;i<32;++i){snprintf(name+i*2,3,"%02x",label.name[i]);snprintf(locid+i*2,3,"%02x",label.locid[i]);}
            snprintf(line,sizeof(line),"# catalog_key=%s name_hex=%s locid_hex=%s\n",key,name,locid);write_line(line);++label_cursor;
        }
        AdapterRecord value{};
        while(AdapterRead(cursor,&value)) {
            char key[33]{};for(size_t i=0;i<16;++i)snprintf(key+i*2,3,"%02x",value.key[i]);
            snprintf(line,sizeof(line),"%llu,%llu,%llu,%llu,%llx,%llx,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%.9g,%.9g,%.9g,%.9g,%s\n",static_cast<unsigned long long>(value.sequence),static_cast<unsigned long long>(value.qpc),static_cast<unsigned long long>(value.end_qpc),static_cast<unsigned long long>(value.call),static_cast<unsigned long long>(value.stream),static_cast<unsigned long long>(value.component),static_cast<unsigned long long>(value.generation),value.thread,value.kind,value.requested,value.produced,value.frames,value.first_clock,value.last_clock,value.faults,value.min_envelope,value.max_envelope,value.min_gain,value.max_gain,key);
            write_line(line);
#ifdef ADAPTER_PLAYBACK_BUILD
            if(value.kind==callback){const auto& counts=playback_counts[cursor];snprintf(line,sizeof(line),"# pcm_call=%llu substituted=%u eof=%u native=%u\n",static_cast<unsigned long long>(value.call),counts.substituted,counts.eof,counts.native);write_line(line);}
#endif
            ++cursor;
        }
#ifdef ADAPTER_PLAYBACK_BUILD
        health.ui_now=supermoon::ui::journal.calls[1].load();health.ui_rows=supermoon::ui::journal.calls[2].load();health.ui_dropped=supermoon::ui::journal.dropped.load();
        health.titles=supermoon::ui::time::title_replaced.load();health.times=supermoon::ui::time::time_replaced.load();health.fallbacks=supermoon::ui::time::fallbacks.load();
#endif
        endurance.drain();endurance.heartbeat(health,diagnostic_io,diagnostic_limit,GetTickCount64());
        if(finished.load(std::memory_order_acquire))break;
        Sleep(25); // worker only
    }
    snprintf(line,sizeof(line),"# capture_end prefix=%u reserved=%u dropped=%llu; not native callback-drain proof\n",cursor,AdapterCount(),static_cast<unsigned long long>(AdapterDropped()));write_line(line);
    endurance.drain();endurance.close();
    FlushFileBuffers(log_file);CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;return 0;
}
}
// Private passive diagnostic. Runs only after the original native lock call.
// Reads current invocation's borrowed stream while its native lock is held.
// Kind 8: requested=enable byte, produced=initialized byte. No audio writes,
// retained native pointers or gain writes; samples can only clear owned intent.
extern "C" void FloorBeforeAudioUpdate() noexcept {
    auto saved=GetLastError();
    static std::atomic<uint64_t> entries{0};[[maybe_unused]] auto entry=entries.fetch_add(1,std::memory_order_relaxed);
#ifdef ADAPTER_RECOVERY_BUILD
    if(entry==0){AdapterRecord r{};r.kind=10;r.requested=engine_ready.load(std::memory_order_acquire);append(r);}
    if(engine_ready.load(std::memory_order_acquire))floor_control.pump();
#endif
    SetLastError(saved);
}
extern "C" void AdapterObserveState(void* stream) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    if(!current||current->stream!=stream)return;
    AdapterRecord record{};record.kind=8;record.call=current->summary.call;
    record.stream=reinterpret_cast<uintptr_t>(stream);
    record.component=current->endpoint;
    auto index=floor_index(current->endpoint);
    auto captured=floor_control.intent.capture(index); // before the native read
    record.requested=read<uint8_t>(stream,0x14);
    record.produced=read<uint8_t>(stream,0x15);
    (void)floor_control.intent.observe(index,captured,uint8_t(record.requested),uint8_t(record.produced));
#ifdef ADAPTER_STATE_FIXTURE
    extern void StateFixtureProbe(uint32_t,uint32_t);
    StateFixtureProbe(record.requested,record.produced);
#endif
    append(record);
}
extern "C" bool AdapterCarrier(void* stream) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return false;
#endif
    if(!current||current->stream!=stream||!core)return false;
#ifdef ADAPTER_RECOVERY_BUILD
    auto ticket=bytebeat::recovery::ticket(reinterpret_cast<uintptr_t>(stream));
    return bytebeat::recovery::enabled()&&ticket&&current->endpoint==reinterpret_cast<uintptr_t>(ticket)&&core->carrier_policy(current->endpoint,read<uint32_t>(stream,0x10));
#else
    return core->carrier_policy(reinterpret_cast<uintptr_t>(stream),read<uint32_t>(stream,0x10));
#endif
}
extern "C" void AdapterFrame(void* stream,const unsigned char* stack) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
#ifdef ADAPTER_SELFTEST_BUILD
    if(check_live_unwind)verify_live_unwind(stream,stack);
#endif
    if(!current||current->stream!=stream||!stack){observed_fault(context_fault);return;}
    // The completed-frame site is reached only after a frame's native processing.
    // Offsets describe this exact pinned executable's stack frame, not a public
    // NMS API. Check them against the wrapper's arguments before using output.
    // Read the sample clock here, while synthesis owns its lock. A disabled
    // provider can advance that clock without producing audio, so a requested
    // buffer size or an outside-lock clock read cannot locate a WAV sample.
    // Native loop/fade settings belong to shared GcAudioGlobals; they are not
    // per-Supermoon seconds fields and this helper must never rewrite them.
    auto index=read<uint64_t>(stack,0x80);auto buffer=read<float*>(stack,0x188);
    auto request=read<uint32_t>(stack,0x190);auto produced=read<uint32_t*>(stack,0x198);
    if(index>UINT32_MAX||buffer!=current->buffer||request!=current->request||produced!=current->produced||!produced||index>=request||*produced!=index){observed_fault(context_fault);return;}
    bytebeat::CompletedFrame frame{reinterpret_cast<uintptr_t>(stream),read<uint32_t>(stream,0x10),uint32_t(index),request,*produced,read<float>(stack,0x50),read<float>(stream,0x18),buffer};
    auto& s=current->summary;
    if(s.frames==0){s.first_clock=frame.clock;s.min_envelope=s.max_envelope=frame.envelope;s.min_gain=s.max_gain=frame.gain;}
    else{if(frame.clock!=s.last_clock+1)observed_fault(clock_fault);s.min_envelope=std::min(s.min_envelope,frame.envelope);s.max_envelope=std::max(s.max_envelope,frame.envelope);s.min_gain=std::min(s.min_gain,frame.gain);s.max_gain=std::max(s.max_gain,frame.gain);}
    ++s.frames;s.last_clock=frame.clock;
    uint64_t frame_generation{};bytebeat::endurance::FrameInfo diagnostic{};
#ifdef ADAPTER_RECOVERY_BUILD
    auto ticket=bytebeat::recovery::ticket(reinterpret_cast<uintptr_t>(stream));
    if(!bytebeat::recovery::enabled()||!ticket||current->endpoint!=reinterpret_cast<uintptr_t>(ticket))return;
    frame.stream=current->endpoint;
#endif
    auto result=core->frame(frame,&frame_generation,&diagnostic);
    current->continuity.observe(diagnostic);
#ifdef ADAPTER_PLAYBACK_BUILD
    // Count actual completed output writes, rather than inferring replacement
    // from elapsed time. The writer serializes these counters off the audio path.
    if(result==bytebeat::FrameResult::substituted)++current->playback.substituted;
    else if(result==bytebeat::FrameResult::eof)++current->playback.eof;
    else ++current->playback.native;
#endif
    if(s.frames==1)s.generation=frame_generation;
    else if(s.generation!=frame_generation)observed_fault(binding_fault);
    if(result==bytebeat::FrameResult::invalid||result==bytebeat::FrameResult::contention||core->faulted())s.faults|=binding_fault;
}
extern "C" void AdapterBefore(void* component,void* controller,void* playlist,const void* key) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    // A display title alone cannot authorize substitution. Establish personal
    // playlist provenance at the resolver call and copy its actual 16-byte key.
    AdapterRecord r{};r.kind=selection;r.component=reinterpret_cast<uintptr_t>(component);r.stream=r.component+0x28;
    if(pending.valid){observed_fault(reentry_fault);r.faults|=reentry_fault;}
    pending={};pending.component=r.component;
    pending.personal=component&&key&&reinterpret_cast<uintptr_t>(controller)==r.component+0x7D5C&&reinterpret_cast<uintptr_t>(playlist)==native_base+0x6f8ef30;
    // Selected key is a native resolver argument: valid borrowed 16-byte identifier.
    if(key){memcpy(pending.key.data(),key,16);memcpy(r.key,key,16);pending.valid=true;}
    else r.faults|=key_fault;
    if(!pending.personal)r.faults|=scope_fault;
    append(r);
}
extern "C" void AdapterAfter(void* component,void* record) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    // This bridge still runs in the resolver's native call chain. Confirm that
    // its result describes the selected key before accepting payload identity.
    AdapterRecord r{};r.kind=resolved;r.component=reinterpret_cast<uintptr_t>(component);r.stream=r.component+0x28;
    bool ok=pending.valid&&pending.component==r.component&&record;
    if(ok){memcpy(r.key,pending.key.data(),16);ok=memcmp(static_cast<unsigned char*>(record)+0x20,pending.key.data(),16)==0;}
    if(ok){
        pending.payload=reinterpret_cast<uintptr_t>(record)+0x40;
        // GcByteBeatLibraryData reflection proves these are embedded 32-byte
        // Name (+0x300) and LocID (+0) fields in the same resolved record. Copy
        // bounded bytes; no assumption of a terminator, encoding or UI title.
        auto index=catalog_label_count.fetch_add(1,std::memory_order_relaxed);
        if(index<64){auto& label=catalog_labels[index];memcpy(label.key,pending.key.data(),16);memcpy(label.name,static_cast<unsigned char*>(record)+0x300,32);memcpy(label.locid,record,32);label.ready.store(true,std::memory_order_release);}
        else catalog_label_dropped.fetch_add(1,std::memory_order_relaxed);
    }
    else{r.faults|=key_fault;pending={};
#ifdef ADAPTER_RECOVERY_BUILD
        if(auto endpoint=bytebeat::recovery::control_endpoint(r.stream))AdapterRecoveryRetire(reinterpret_cast<uintptr_t>(endpoint));
#else
        core->retire(r.stream);
#endif
    }
    append(r);
}
extern "C" void AdapterCommit(void* component,void* payload) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    // The setter has reset synthesis state but has not released its lock yet.
    // Publishing the generation here prevents the audio thread from consuming
    // new native state with stale prepared-PCM state. The recovery path also
    // requires a live owned endpoint; native manual Play supplies the commit.
    AdapterRecord r{};r.kind=commit_event;r.component=reinterpret_cast<uintptr_t>(component);r.stream=r.component+0x28;
    bool matched=pending.valid&&pending.component==r.component&&pending.payload==reinterpret_cast<uintptr_t>(payload);
    if(matched)memcpy(r.key,pending.key.data(),16);
#ifdef ADAPTER_RECOVERY_BUILD
    auto endpoint=bytebeat::recovery::control_endpoint(r.stream);
    bool accepted=bytebeat::recovery::enabled()&&matched&&pending.personal&&endpoint&&core->commit_endpoint(reinterpret_cast<uintptr_t>(endpoint),r.stream,pending.key,read<uint32_t>(component,0x38),&r.generation);
    if(!accepted){r.faults|=scope_fault;if(endpoint)AdapterRecoveryRetire(reinterpret_cast<uintptr_t>(endpoint));}
    if(endpoint&&r.generation)(void)floor_control.intent.commit(floor_index(reinterpret_cast<uintptr_t>(endpoint)),r.generation,accepted&&pending.key==floor_carrier_key);
#else
    if(!matched||!pending.personal||!core->commit(r.stream,pending.key,matched&&pending.personal,read<uint32_t>(component,0x38),&r.generation)) {r.faults|=scope_fault;core->retire(r.stream);}
#endif
    pending={};append(r);
}
extern "C" void AdapterRetire(void* component) noexcept {
#ifdef ADAPTER_RECOVERY_BUILD
    if(!engine_ready.load(std::memory_order_acquire))return;
#endif
    // Entry before removal/lock deletion. No native dereference or hot-state freeing.
    AdapterRecord r{};r.kind=retire_event;r.component=reinterpret_cast<uintptr_t>(component);r.stream=r.component+0x28;
#ifdef ADAPTER_RECOVERY_BUILD
    bytebeat::recovery::retire_stream(r.stream);
#else
    core->retire(r.stream);
#endif
    if(pending.component==r.component)pending={};append(r);
}
uint32_t AdapterRead(uint32_t index,AdapterRecord* output) {
    if(!output||index>=capacity||!records[index].ready.load(std::memory_order_acquire))return 0;
    *output=records[index].value;return 1;
}
uint32_t AdapterCount(){auto n=sequence.load(std::memory_order_relaxed);return uint32_t(std::min<uint64_t>(n,capacity));}
uint64_t AdapterDropped(){return dropped.load(std::memory_order_relaxed);}
DWORD WINAPI AdapterFinishCapture(void*){finished.store(true,std::memory_order_release);return 0;}
#ifdef ADAPTER_PLAYBACK_BUILD
#define ADAPTER_ACTIVATE AdapterActivatePlayback
#else
#define ADAPTER_ACTIVATE AdapterActivateDiagnostics

#endif
DWORD WINAPI ADAPTER_ACTIVATE(void*) {
    // Explicit, one-shot activation. Loading the DLL alone never reaches here.
    // Refusals are not invitations to retry: later failures can occur after a
    // live pointer has already changed. Normal process exit is the rollback.
    uint32_t idle=0;if(!activation.compare_exchange_strong(idle,1))return 1;
    wchar_t path[32768]{};auto module=GetModuleHandleW(nullptr);
    if(!module||!GetModuleFileNameW(module,path,32768)||_wcsicmp(std::filesystem::path(path).filename().c_str(),L"NMS.exe")||!bytebeat::pinned_executable_file(path))return 2;
    try {
        struct PublicationGuard {
            bool exposed{};
            ~PublicationGuard(){if(!exposed)finished.store(true,std::memory_order_release);}
        } publication;
        auto base=reinterpret_cast<unsigned char*>(module);native_base=reinterpret_cast<uintptr_t>(base);
        for(const auto& f:bytebeat::layout::functions)if(!memory_digest(base+f.rva,f.original_size,f.sha256))return 3;
#ifdef ADAPTER_RECOVERY_BUILD
        for(const auto& pin:bytebeat::recovery::native_pins)if(!memory_digest(base+pin.rva,pin.size,pin.sha))return 14;
        if(!bytebeat::recovery::validate(base,module))return 15;
#endif
        if(!floor_preflight(base,module,path))return 17;
        const uint32_t slots[]={0x4b845b0,0x4b844f8,0x4b84578,0x4b83cb0,0x4b83cb8};const uint32_t targets[]={0xf8abb0,0xf8a110,0xf8a720,0xfbf1f0,0xfbf150};
        DWORD slot_protection{};
        for(size_t n=0;n<5;++n){MEMORY_BASIC_INFORMATION info{};auto slot=reinterpret_cast<void* volatile*>(base+slots[n]);if(!VirtualQuery(const_cast<void**>(slot),&info,sizeof(info))||info.State!=MEM_COMMIT||info.Type!=MEM_IMAGE||info.AllocationBase!=module||(info.Protect&(PAGE_GUARD|PAGE_NOACCESS))||*slot!=base+targets[n])return 4;if(n==0)slot_protection=info.Protect;else if(info.Protect!=slot_protection)return 4;}
        HMODULE self{};
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&ADAPTER_ACTIVATE),&self))return 5;
        if(!GetModuleFileNameW(self,path,32768)||!bytebeat::playback::location_matches(path))return 6;
        void* bridges[]={reinterpret_cast<void*>(&AdapterFrameBridge),reinterpret_cast<void*>(&AdapterBeforeBridge),reinterpret_cast<void*>(&AdapterAfterBridge),reinterpret_cast<void*>(&AdapterCommitBridge),reinterpret_cast<void*>(&AdapterRetireBridge),reinterpret_cast<void*>(&AdapterCarrierBridge)
#ifdef ADAPTER_RECOVERY_BUILD
            ,reinterpret_cast<void*>(&RecoveryPostedBridge)
#endif
            ,reinterpret_cast<void*>(&AdapterStateLockBridge),reinterpret_cast<void*>(&FloorControlUnlockBridge)
        };
        StateNativeLock=base+0x2c772f0;
        clone.prepare(base,bridges);void* original=clone.function(0);memcpy(&provider,&original,sizeof(provider));
#ifdef ADAPTER_RECOVERY_BUILD
        // Capture owner/source births BEFORE potentially long cold preparation.
        // Only lifecycle/forwarding runs here; early wrappers return before
        // touching Core/PCM while engine_ready is false. UI starts later.
        void* native_functions[12]{};for(size_t n=0;n<12;++n)native_functions[n]=clone.function(n);
        bytebeat::recovery::initialize(reinterpret_cast<uintptr_t>(base),native_functions);
        publication.exposed=true;clone.published();
        if(!bytebeat::recovery::publish(base))return 16;
#endif
#ifdef ADAPTER_PLAYBACK_BUILD
        // Refuse unreviewed identity before adopting the prepared payload.
        // Recovery lifecycle hooks may already be exposed here: a preparation
        // refusal is still no-retry/exit-required, even without that log wording.
        // Payload allocation stays local until adoption; adopted Core/PCM and
        // previously published native wrappers remain resident on later failure.
        if(!bytebeat::playback::configuration.live_identity_reviewed)return 12;
        bytebeat::playlist::ResidentHandoff<bytebeat::playback::Payload> payload(bytebeat::playback::prepare(bytebeat::playback::configuration));
#else
        initial_core.configure({},false,{},false);
#endif
        wchar_t* slash=wcsrchr(path,L'\\');if(!slash)return 6;slash[1]=0;
#ifdef ADAPTER_PLAYBACK_BUILD
        constexpr const wchar_t* log_mode=L"playback";
#else
        constexpr const wchar_t* log_mode=L"diagnostic";
#endif
        wchar_t filename[128]{};swprintf(filename,128,L"supermoon-%ls-latest.csv",log_mode);
        if(wcslen(path)+wcslen(filename)>=32768)return 6;wcscat(path,filename);
        log_file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(log_file==INVALID_HANDLE_VALUE)return 7;
        writer_ready=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!writer_ready){CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;return 7;}
        HANDLE thread=CreateThread(nullptr,0,writer,nullptr,0,nullptr);if(!thread){CloseHandle(writer_ready);writer_ready=nullptr;CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;return 7;}CloseHandle(thread);
        auto ready=WaitForSingleObject(writer_ready,10000);
        if(ready==WAIT_OBJECT_0){CloseHandle(writer_ready);writer_ready=nullptr;}
        // Timeout may leave a late SetEvent in flight; retain that one handle
        // and pinned DLL rather than close a potentially reused handle token.
        if(ready!=WAIT_OBJECT_0||finished.load())return 8;
        // The five pointers occupy two adjacent .rdata pages. Require identical
        // original protections above so restoring one saved protection cannot
        // accidentally change a page that previously had different permissions.
        // Only the pointer cells are written; native instruction bytes stay intact.
        DWORD old{};auto first=base+0x4b83cb0;constexpr size_t span=0x908;
        if(!VirtualProtect(first,span,PAGE_READWRITE,&old))return 9;
        void* replacements[]={reinterpret_cast<void*>(&adapter_provider),clone.function(1),clone.function(4),clone.function(5),clone.function(6)};
        // A component vtable redirect does not catch a pool's direct CALL.
        // Redirect both pool dispatch slots to private driver copies as well.
        // Their relocated CALLs reach our private update/transition/setter chain,
        // preserving native list iteration and synchronization. This closes the
        // identified static gap; actual NMS key/commit coverage still needs proof.
        // Mark resident before any publication, even if a later compare fails.
        // Each aligned exchange is atomic, not a publication transaction.
        // Recovery already owns cleanup via the early lifecycle hook; publish
        // update/pool drivers before provider. The legacy branch also publishes
        // cleanup here. Any failure retains all callback-visible state.
#ifdef ADAPTER_PLAYBACK_BUILD
        auto resident=payload.adopt();core=&resident->core;
        engine_log.store(&resident->engine,std::memory_order_release);
#endif
        publication.exposed=true; // All later failures retain callback state.
        clone.published();bool ok=true;
#ifdef ADAPTER_RECOVERY_BUILD
        core->endpoint_validator(&endpoint_live);
        engine_ready.store(true,std::memory_order_release);
        for(size_t n:{size_t(1),size_t(3),size_t(4),size_t(0)})if(ok&&
#else
        for(size_t n:{size_t(2),size_t(1),size_t(3),size_t(4),size_t(0)})if(
#endif
            InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(base+slots[n]),replacements[n],base+targets[n])!=base+targets[n]){ok=false;break;}
        DWORD ignored{};bool restored=VirtualProtect(first,span,old,&ignored)!=0;
        if(!ok||!restored)return 10;
#ifdef ADAPTER_RECOVERY_BUILD
        bytebeat::recovery::set_enabled();
#endif // possible partial activation: no retry/hot unload
#ifdef ADAPTER_PLAYBACK_BUILD
        // Optional diagnostic failure leaves successful playback activation intact.
        supermoon::ui::start(base,&resident->engine,bytebeat::playback::configuration.carrier.data(),&resident->core,&resident->filenames);
#endif
        if(!floor_publish(base))return 18;
        return 0;
    }catch(...){return 11;} // Local payload RAII joins its worker; earlier exposed hooks/adopted state stay resident.
}
#ifdef ADAPTER_SELFTEST_BUILD
void AdapterTestConfigure(AdapterProviderFn original,const std::vector<float>* song,uintptr_t base,const bytebeat::CompositionKey& key,bool pcm) {
    // Own-process tests only; no publication and no live NMS access.
    static bytebeat::SupermoonCore* owned{};
    delete owned;owned=new bytebeat::SupermoonCore(song);core=owned;provider=original;native_base=base;pending={};current=nullptr;
    core->configure(key,true,{true,true,true,true,true,true},pcm);
#ifdef ADAPTER_RECOVERY_BUILD
    engine_ready.store(true,std::memory_order_release);
#endif
}
uint64_t AdapterTestCursor(){return core->cursor_for_test();}
void AdapterTestProvider(void* stream,float* output,uint32_t requested,uint32_t* produced){adapter_provider(stream,output,requested,produced);}
void AdapterTestResetJournal(){for(uint32_t i=0;i<AdapterCount();++i)records[i].ready.store(false);for(auto& label:catalog_labels)label.ready.store(false);catalog_label_count=0;catalog_label_dropped=0;sequence=0;dropped=0;call_id=0;}
bool AdapterTestReadCatalogLabel(uint32_t index,uint8_t* key,uint8_t* name,uint8_t* locid){if(index>=64||!key||!name||!locid||!catalog_labels[index].ready.load(std::memory_order_acquire))return false;memcpy(key,catalog_labels[index].key,16);memcpy(name,catalog_labels[index].name,32);memcpy(locid,catalog_labels[index].locid,32);return true;}
void AdapterTestSetUnwindCheck(bool value){check_live_unwind=value;}
#endif

#ifdef ADAPTER_SELFTEST_BUILD
void AdapterTestConfigureEngine(AdapterProviderFn original,bytebeat::playlist::Engine& engine,uintptr_t base,const bytebeat::CompositionKey& key){
    provider=original;native_base=base;core=new bytebeat::SupermoonCore(engine);
    core->configure(key,true,{true,true,true,true,true,true},true);
#ifdef ADAPTER_RECOVERY_BUILD
    engine_ready.store(true,std::memory_order_release);
#endif
}
#endif

#ifdef ADAPTER_SELFTEST_BUILD
// Test process owns the Engine until this writer is explicitly joined.
HANDLE AdapterTestStartWriter(bytebeat::playlist::Engine& engine,const wchar_t* path){
    if(log_file!=INVALID_HANDLE_VALUE||engine_log.load())return nullptr;
    log_file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(log_file==INVALID_HANDLE_VALUE)return nullptr;
    writer_ready=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    if(!writer_ready){CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;return nullptr;}
    finished=false;diagnostic_bytes=0;diagnostic_limit=false;diagnostic_io=true;engine_log.store(&engine,std::memory_order_release);
    HANDLE thread=CreateThread(nullptr,0,writer,nullptr,0,nullptr);
    if(!thread){engine_log=nullptr;CloseHandle(writer_ready);writer_ready=nullptr;CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;return nullptr;}
    if(WaitForSingleObject(writer_ready,5000)!=WAIT_OBJECT_0){finished=true;return thread;}
    CloseHandle(writer_ready);writer_ready=nullptr;return thread;
}
bool AdapterTestStopWriter(HANDLE thread){
    finished=true;if(WaitForSingleObject(thread,5000)!=WAIT_OBJECT_0)return false;
    CloseHandle(thread);engine_log=nullptr;
    if(writer_ready){CloseHandle(writer_ready);writer_ready=nullptr;}
    return true;
}
#endif

#ifdef ADAPTER_RECOVERY_BUILD
extern "C" void AdapterRecoveryRetire(uintptr_t ticket) noexcept {
    if(!engine_ready.load(std::memory_order_acquire))return;
    auto index=floor_index(ticket);auto captured=floor_control.intent.capture(index);
    (void)floor_control.intent.observe(index,captured,0,1);
    if(!core->retire_endpoint(ticket))deferred_retirement.store(ticket,std::memory_order_release);
}
extern "C" void AdapterRecoverySource(uintptr_t ticket,uint32_t id,uint32_t phase,uintptr_t stream) noexcept {
    if(phase==bytebeat::recovery::retired)floor_control.intent.retire(floor_index(ticket));
    AdapterRecord value{};value.kind=phase==bytebeat::recovery::ready?6u:7u;value.component=ticket;value.stream=stream;value.requested=id;value.produced=phase;append(value);
}
#endif
