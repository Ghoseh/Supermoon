// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "endurance.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <cstring>
namespace bytebeat::endurance {
// Worker-thread only. Byte limits are separate so detailed-event exhaustion
// cannot silently stop periodic whole-session aggregates/coverage.
struct Output {
    HANDLE file{INVALID_HANDLE_VALUE};uint64_t bytes{},limit{};bool ok{true},limited{};
    void line(const char* s) noexcept {if(!ok||limited||file==INVALID_HANDLE_VALUE)return;auto size=DWORD(strlen(s));
        if(bytes+size>limit){limited=true;return;}DWORD written{};
        if(!WriteFile(file,s,size,&written,nullptr)||written!=size)ok=false;bytes+=written;}
    void close() noexcept {if(file!=INVALID_HANDLE_VALUE){if(!FlushFileBuffers(file))ok=false;CloseHandle(file);file=INVALID_HANDLE_VALUE;}}
};
struct Health {
    uint64_t pcm{},underruns{},completed{},failed{},serial{},revision{},position{},length{};
    uint64_t ui_now{},ui_rows{},ui_dropped{},titles{},times{},fallbacks{};
    uint32_t track{UINT32_MAX};bool snapshot{},active{},healthy{},stop{},worker_exited{},worker_joined{},worker_join_failed{};
};
class Writer {
    Output health_{INVALID_HANDLE_VALUE,0,64ull*1024*1024},events_{INVALID_HANDLE_VALUE,0,32ull*1024*1024};
    Merge merge_{};uint64_t began_{},last_{},event_count_{},sampled_{},last_serial_{},last_revision_{};
public:
    bool open(HANDLE legacy) noexcept {
        wchar_t path[32768]{};auto n=GetFinalPathNameByHandleW(legacy,path,32768,FILE_NAME_NORMALIZED);
        if(!n||n>=32768)return false;auto slash=wcsrchr(path,L'\\');if(!slash||size_t(slash-path)+64>=32768)return false;
        wcscpy(slash+1,L"supermoon-endurance-health.csv");health_.file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        wcscpy(slash+1,L"supermoon-endurance-events.csv");events_.file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        health_.ok=health_.file!=INVALID_HANDLE_VALUE;events_.ok=events_.file!=INVALID_HANDLE_VALUE;
        began_=GetTickCount64();last_=began_;LARGE_INTEGER f{},qpc{};QueryPerformanceFrequency(&f);QueryPerformanceCounter(&qpc);SYSTEMTIME utc{};GetSystemTime(&utc);char text[600]{};
        snprintf(text,sizeof(text),"# endurance observation only; qpc_frequency=%llu start_qpc=%llu start_tick_ms=%llu UTC=%04u-%02u-%02uT%02u:%02u:%02u.%03uZ health_cap=67108864 event_cap=33554432 ring=4096 time_limit=none; counters independently sampled; no native-drain/clean claim\n",static_cast<unsigned long long>(f.QuadPart),static_cast<unsigned long long>(qpc.QuadPart),static_cast<unsigned long long>(began_),utc.wYear,utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds);health_.line(text);
        health_.line("elapsed_ms,calls,exceptions,zero_produced,native_frames,custom_attempted,empty_pcm,stale_available,faulted,ring_dropped,max_empty_within_call,max_empty_observed_contiguous,event_records,event_io,event_limit,health_io,health_limit,active,healthy,stop,worker_exited,worker_joined,worker_join_failed,pcm,underruns,completed,failed,snapshot,track,serial,revision,position,length,ui_now,ui_rows,ui_legacy_dropped,title_replaced,time_replaced,ui_fallback,legacy_io,legacy_limit\n");
        events_.line("call,begin_qpc,end_qpc,stream_token,generation,requested,produced,frames,faults,exception,custom_attempted,empty_pcm,inactive,unhealthy,revision_empty,stale_available,prefix_empty,suffix_empty,max_empty,first_valid,first_track,first_serial,first_revision,first_position,last_valid,last_track,last_serial,last_revision,last_position,last_length\n");
        return health_.ok&&events_.ok;
    }
    void drain() noexcept {
        Event e{};for(unsigned i=0;i<256&&ring.pop(e);++i){++event_count_;merge_.accept(e,totals.dropped.load(std::memory_order_relaxed));
            bool changed=e.pcm.last_valid&&(e.pcm.last.serial!=last_serial_||e.pcm.last_revision!=last_revision_);
            if(e.pcm.last_valid){last_serial_=e.pcm.last.serial;last_revision_=e.pcm.last_revision;}
            if(!changed&&!e.pcm.empty&&!e.pcm.inactive&&!e.pcm.unhealthy&&!e.pcm.stale&&!e.exceptional&&!e.faults&&e.produced&&++sampled_%64)continue;
            char s[1800]{};
            snprintf(s,sizeof(s),"%llu,%llu,%llu,%llx,%llu,%u,%u,%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%u,%u,%llu,%llu,%llu,%u,%u,%llu,%llu,%llu,%llu\n",
                (unsigned long long)e.call,(unsigned long long)e.begin,(unsigned long long)e.end,(unsigned long long)e.stream,(unsigned long long)e.generation,e.requested,e.produced,e.frames,e.faults,unsigned(e.exceptional),
                (unsigned long long)e.pcm.attempted,(unsigned long long)e.pcm.empty,(unsigned long long)e.pcm.inactive,(unsigned long long)e.pcm.unhealthy,(unsigned long long)e.pcm.revision_empty,(unsigned long long)e.pcm.stale,
                (unsigned long long)e.pcm.prefix,(unsigned long long)e.pcm.suffix,(unsigned long long)e.pcm.maximum,unsigned(e.pcm.first_valid),e.pcm.first.track,(unsigned long long)e.pcm.first.serial,(unsigned long long)e.pcm.first_revision,(unsigned long long)e.pcm.first.position,
                unsigned(e.pcm.last_valid),e.pcm.last.track,(unsigned long long)e.pcm.last.serial,(unsigned long long)e.pcm.last_revision,(unsigned long long)e.pcm.last.position,(unsigned long long)e.pcm.last.length);events_.line(s);
        }
    }
    // Optional clock argument makes long-session file behavior testable without
    // waiting4h; production passes the real GetTickCount64 value.
    void heartbeat(const Health& h,bool legacy_io,bool legacy_limit,uint64_t now) noexcept {
        if(now-last_<1000)return;last_=now;char s[1800]{};
        snprintf(s,sizeof(s),"%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%llu,%llu,%llu,%llu,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%u,%u\n",
            (unsigned long long)(now-began_),(unsigned long long)totals.calls.load(),(unsigned long long)totals.exceptions.load(),(unsigned long long)totals.zero_produced.load(),(unsigned long long)totals.native_frames.load(),
            (unsigned long long)totals.attempted.load(),(unsigned long long)totals.empty.load(),(unsigned long long)totals.stale.load(),(unsigned long long)totals.faulted.load(),(unsigned long long)totals.dropped.load(),
            (unsigned long long)totals.max_within_call.load(),(unsigned long long)merge_.maximum,(unsigned long long)event_count_,unsigned(events_.ok),unsigned(events_.limited),unsigned(health_.ok),unsigned(health_.limited),unsigned(h.active),unsigned(h.healthy),unsigned(h.stop),unsigned(h.worker_exited),unsigned(h.worker_joined),unsigned(h.worker_join_failed),
            (unsigned long long)h.pcm,(unsigned long long)h.underruns,(unsigned long long)h.completed,(unsigned long long)h.failed,unsigned(h.snapshot),h.track,(unsigned long long)h.serial,(unsigned long long)h.revision,(unsigned long long)h.position,(unsigned long long)h.length,
            (unsigned long long)h.ui_now,(unsigned long long)h.ui_rows,(unsigned long long)h.ui_dropped,(unsigned long long)h.titles,(unsigned long long)h.times,(unsigned long long)h.fallbacks,unsigned(legacy_io),unsigned(legacy_limit));health_.line(s);
        if((now-began_)/1000%10==0){if(health_.file!=INVALID_HANDLE_VALUE&&!FlushFileBuffers(health_.file))health_.ok=false;if(events_.file!=INVALID_HANDLE_VALUE&&!FlushFileBuffers(events_.file))events_.ok=false;}
    }
    uint64_t start_clock() const noexcept {return began_;}
    bool health_ok()const noexcept{return health_.ok&&!health_.limited;}
    bool event_ok()const noexcept{return events_.ok&&!events_.limited;}
    void close() noexcept {health_.line("# capture_end worker_only; native_drained=unknown clean_session=unknown\n");health_.close();events_.close();}
};
}
