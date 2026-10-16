// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

namespace bytebeat::playlist {
struct Cue {const float* pcm;uint32_t frames;double peak;};
extern const Cue on_cue,off_cue;
bool notification_configuration(const std::filesystem::path&);

// One worker publishes a lock-free latest-commit mailbox. Exactly one validated
// audio consumer owns all cursor fields. No allocation, IO, waits or borrowed data.
// Rapid commits may coalesce; committed() is not a count of audibly finished cues.
class Notifications {
    const bool enabled_;
    std::atomic<uint64_t> event_{0},epoch_{1},committed_{0};
    uint32_t sequence_{}; // Disk worker only; no wrap or ABA permitted.
    uint64_t seen_{},consumer_epoch_{};uint32_t offset_{};const Cue* cue_{};
public:
    explicit Notifications(bool enabled=false):enabled_(enabled){}
    uint64_t epoch()const noexcept{return epoch_.load(std::memory_order_acquire);}
    void cancel()noexcept{epoch_.fetch_add(1,std::memory_order_acq_rel);}
    void publish(bool on,uint64_t epoch)noexcept {
        if(!enabled_||epoch>0x7fffffffu||epoch!=this->epoch()||sequence_==UINT32_MAX)return;
        auto token=(uint64_t(++sequence_)<<32)|(epoch<<1)|uint64_t(on);
        event_.store(token,std::memory_order_release);committed_.fetch_add(1,std::memory_order_relaxed);
    }
    uint64_t committed()const noexcept{return committed_.load(std::memory_order_acquire);}
    // Main music is unchanged. Per-frame cue scale uses the track peak bound
    // and current native envelope/gain; high native gain or invalid metadata can mute
    // a cue rather than change music or introduce additional clipping.
    float sample(float music,double bound,float envelope,float gain)noexcept {
        auto epoch=this->epoch();
        if(epoch!=consumer_epoch_){consumer_epoch_=epoch;cue_=nullptr;offset_=0;}
        auto token=event_.load(std::memory_order_acquire);
        if(token!=seen_){seen_=token;cue_=nullptr;offset_=0;
            if(enabled_&&((token>>1)&0x7fffffffu)==epoch)cue_=(token&1)?&on_cue:&off_cue;}
        if(!cue_||offset_>=cue_->frames)return 0;
        double factor=double(envelope)*gain;
        if(!std::isfinite(music)||!std::isfinite(bound)||bound<0||bound>1.00001||
           !std::isfinite(factor)||factor<0||!cue_->pcm||cue_->peak<=0)return 0;
        double limit=0.98/std::max(1.,factor);
        double scale=std::min(1.,std::max(0.,limit-bound)/cue_->peak);
        float value=float(double(cue_->pcm[offset_])*scale);
        // Fail closed on inconsistent bounds or rounding, not a limiter. Do not
        // modify the song if the candidate additive sample cannot fit safely.
        if(!std::isfinite(value)||std::abs(double(music)+value)>limit)return 0;
        return value;
    }
    void consume()noexcept{if(cue_&&offset_<cue_->frames)++offset_;}
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
}
