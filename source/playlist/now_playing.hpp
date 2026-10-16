// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace bytebeat::playlist {
struct PlaybackSnapshot {
    uint64_t serial{}, revision{}, position{}, length{};
    uint32_t track{UINT32_MAX};
};
// One serialized PCM writer; multiple off-callback readers. Atomic data members
// keep concurrent reads defined by C++, with a bounded sequence check. A read
// never returns a mixed track/position/duration. No strings or pointers cross it.
class PlaybackPublication {
    std::atomic<uint64_t> sequence_{0}, serial_{0}, revision_{0}, position_{0}, length_{0};
    std::atomic<uint32_t> track_{UINT32_MAX};
public:
    void publish(PlaybackSnapshot s) noexcept {
        auto n=sequence_.load(std::memory_order_seq_cst);
        if(n>UINT64_MAX-2)return; // Refuse wrap/ABA rather than publish ambiguity.
        sequence_.store(n+1,std::memory_order_seq_cst);
        serial_.store(s.serial,std::memory_order_seq_cst);revision_.store(s.revision,std::memory_order_seq_cst);
        position_.store(s.position,std::memory_order_seq_cst);length_.store(s.length,std::memory_order_seq_cst);
        track_.store(s.track,std::memory_order_seq_cst);
        sequence_.store(n+2,std::memory_order_seq_cst);
    }
    bool read(PlaybackSnapshot& result) const noexcept {
        for(unsigned attempt=0;attempt<3;++attempt){
            auto a=sequence_.load(std::memory_order_seq_cst);if(a&1)continue;
            PlaybackSnapshot s{serial_.load(std::memory_order_seq_cst),revision_.load(std::memory_order_seq_cst),
                position_.load(std::memory_order_seq_cst),length_.load(std::memory_order_seq_cst),track_.load(std::memory_order_seq_cst)};
            auto b=sequence_.load(std::memory_order_seq_cst);
            if(a==b&&!(b&1)){result=s;return true;}
        }
        return false;
    }
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
struct NowPlayingView {std::wstring filename,elapsed,duration;double fraction{};bool valid{},playing{};};
inline std::wstring frame_time(uint64_t frames){
    auto seconds=frames/24000;auto minutes=seconds/60;
    auto remainder=seconds%60;
    return std::to_wstring(minutes)+L":"+(remainder<10?L"0":L"")+std::to_wstring(remainder);
}
// UI/control thread only. Filename strings are immutable and owned by the caller.
// This pure view helper consumes a caller-supplied native-playing flag. The
// production filename/TIME/progress binding lives in time.cpp; this helper neither
// reads game objects nor treats native carrier clock as prepared-PCM position.
inline NowPlayingView now_playing(PlaybackSnapshot s,const std::vector<std::wstring>& names,bool native_playing){
    if(s.track>=names.size()||!s.serial||!s.length||s.position>s.length)return {};
    return {names[s.track],frame_time(s.position),frame_time(s.length),double(s.position)/double(s.length),true,native_playing};
}
}
