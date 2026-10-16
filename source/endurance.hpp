// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "playlist/now_playing.hpp"
#include <atomic>
#include <array>
#include <cstdint>
#include <algorithm>

namespace bytebeat::endurance {
// Owned scalar copies only. The native provider scope supplies no widget/file
// pointers. No allocation, syscall, lock wait or decoder in these methods.
struct FrameInfo {
    playlist::PlaybackSnapshot value{};
    uint64_t revision{};
    bool streaming{},available{},consumed{},snapshot{};
    uint8_t absence{};
};
struct Frames {
    uint64_t attempted{},empty{},stale{},prefix{},suffix{},maximum{},inactive{},unhealthy{},revision_empty{};
    uint64_t first_revision{},last_revision{};
    playlist::PlaybackSnapshot first{},last{};
    bool first_valid{},last_valid{};
    void observe(const FrameInfo& f) noexcept {
        if(!f.streaming)return;
        if(!attempted){first=f.value;first_valid=f.snapshot;first_revision=f.revision;}
        last=f.value;last_valid=f.snapshot;last_revision=f.revision;++attempted;
        if(!f.available&&(f.absence==3||f.absence==4)){++empty;++suffix;if(empty==attempted)++prefix;maximum=std::max(maximum,suffix);if(f.absence==4)++revision_empty;}
        else if(!f.available){suffix=0;if(f.absence==1)++inactive;else ++unhealthy;}
        else {suffix=0;if(!f.consumed)++stale;}
    }
};
struct Event {
    uint64_t call{},begin{},end{},stream{},generation{};
    uint32_t requested{},produced{},frames{},first_clock{},last_clock{},faults{};
    bool exceptional{};
    Frames pcm{};
};
// Bounded MPSC/single-reader ring. Per-cell sequence grants exclusive storage
// ownership until the consumer releases it. A paused producer may hold up the
// reader; producers try at most8 times and drop rather than wait/overwrite.
template<size_t N> class Ring {
    static_assert(N>1&&(N&(N-1))==0);
    struct Cell {std::atomic<uint64_t> sequence{};Event value{};};
    std::array<Cell,N> cells_{};
    std::atomic<uint64_t> write_{0};uint64_t read_{};
public:
    Ring() noexcept {for(size_t i=0;i<N;++i)cells_[i].sequence.store(i,std::memory_order_relaxed);}
    bool push(const Event& e) noexcept {
        auto pos=write_.load(std::memory_order_relaxed);
        for(unsigned tries=0;tries<8;++tries){auto& c=cells_[pos&(N-1)];auto seq=c.sequence.load(std::memory_order_acquire);
            if(seq==pos){if(write_.compare_exchange_weak(pos,pos+1,std::memory_order_relaxed)){c.value=e;c.sequence.store(pos+1,std::memory_order_release);return true;}}
            else {if(seq<pos)return false;pos=write_.load(std::memory_order_relaxed);}
        }return false;
    }
    bool pop(Event& e) noexcept {auto& c=cells_[read_&(N-1)];if(c.sequence.load(std::memory_order_acquire)!=read_+1)return false;
        e=c.value;c.sequence.store(read_+N,std::memory_order_release);++read_;return true;}
};
struct Counters {
    std::atomic<uint64_t> calls{},exceptions{},zero_produced{},native_frames{},attempted{},empty{},stale{},faulted{},dropped{},max_within_call{};
};
inline Ring<4096> ring;
inline Counters totals;
inline void publish(const Event& e) noexcept {
    totals.calls.fetch_add(1,std::memory_order_relaxed);
    totals.exceptions.fetch_add(e.exceptional,std::memory_order_relaxed);
    totals.zero_produced.fetch_add(!e.exceptional&&!e.produced,std::memory_order_relaxed);
    totals.native_frames.fetch_add(e.frames,std::memory_order_relaxed);
    totals.attempted.fetch_add(e.pcm.attempted,std::memory_order_relaxed);
    totals.empty.fetch_add(e.pcm.empty,std::memory_order_relaxed);
    totals.stale.fetch_add(e.pcm.stale,std::memory_order_relaxed);
    totals.faulted.fetch_add(e.faults!=0,std::memory_order_relaxed);
    auto prior=totals.max_within_call.load(std::memory_order_relaxed);
    // Single accepted PCM consumer normally wins immediately. Bounded attempts;
    // record loss of max publication explicitly rather than spin on callback.
    for(unsigned i=0;e.pcm.maximum>prior&&i<8;++i)
        if(totals.max_within_call.compare_exchange_weak(prior,e.pcm.maximum,std::memory_order_relaxed))break;
    if(e.pcm.maximum>totals.max_within_call.load(std::memory_order_relaxed))totals.dropped.fetch_add(1,std::memory_order_relaxed);
    if(!ring.push(e))totals.dropped.fetch_add(1,std::memory_order_relaxed);
}
struct Merge {
    uint64_t run{},maximum{},stream{},generation{},revision{},last_call{},drop_epoch{};
    uint32_t next_clock{};bool known{};
    void accept(const Event& e,uint64_t drops) noexcept {
        bool contiguous=known&&drop_epoch==drops&&e.stream==stream&&e.generation==generation&&
          e.pcm.first_revision==revision&&e.first_clock==next_clock&&e.call==last_call+1;
        if(!contiguous)run=0;
        if(e.exceptional||e.faults||e.pcm.attempted!=e.frames||!e.frames){known=false;run=0;return;}
        maximum=std::max(maximum,e.pcm.maximum);
        maximum=std::max(maximum,run+e.pcm.prefix);
        run=e.pcm.empty==e.pcm.attempted?run+e.pcm.suffix:e.pcm.suffix;
        known=true;stream=e.stream;generation=e.generation;revision=e.pcm.last_revision;
        next_clock=e.last_clock+1;last_call=e.call;drop_epoch=drops;
    }
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
}
