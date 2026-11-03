// Owned playback intent: verified custom commits enable; observations only clear.
#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <limits>
namespace supermoon::floor_prototype {
// ticket is an append-only owned Source slot, never a native stream address.
// generation is the monotonically increasing successful Core commit generation.
// Production integration must capture a word BEFORE reading native state.
// Observation never turns playback on; only a strictly newer commit may do so.
class PlaybackIntent {
public:
    static constexpr uint32_t capacity=4096;
    static constexpr uint64_t max_generation=(uint64_t(1)<<50)-1;
private:
    std::atomic<uint64_t> word_{0};
    std::array<std::atomic<bool>,capacity> retired_{};
    std::atomic<bool> disabled_{false};
    static constexpr uint64_t active_bit=1;
    static uint64_t generation(uint64_t word) noexcept {return word>>14;}
    static uint32_t ticket(uint64_t word) noexcept {return uint32_t((word>>2)&4095);}
public:
    bool commit(uint32_t source,uint64_t gen,bool custom) noexcept {
        if(source>=capacity||!gen||gen>max_generation){disabled_.store(true,std::memory_order_release);return false;}
        if(disabled_.load(std::memory_order_acquire)||retired_[source].load(std::memory_order_acquire))return false;
        auto current=word_.load(std::memory_order_acquire);
        const auto next=(gen<<14)|(uint64_t(source)<<2)|(custom?active_bit:0);
        for(unsigned attempt=0;attempt<8;++attempt){
            if(gen<=generation(current))return false;
            if(word_.compare_exchange_weak(current,next,std::memory_order_acq_rel))return true;
        }
        disabled_.store(true,std::memory_order_release);return false;
    }
    uint64_t capture(uint32_t source) const noexcept {
        if(source>=capacity||disabled_.load(std::memory_order_acquire)||retired_[source].load(std::memory_order_acquire))return 0;
        auto word=word_.load(std::memory_order_acquire);
        return generation(word)&&ticket(word)==source?word:0;
    }
    // true means current observation accepted/no release needed; false can also
    // mean superseded. Callers must not retry using an old borrowed observation.
    bool observe(uint32_t source,uint64_t captured,uint8_t enabled,uint8_t initialized) noexcept {
        if(source>=capacity||!captured||ticket(captured)!=source)return false;
        if(enabled==1&&initialized==1)return true; // no store, no resurrection
        if(!(captured&active_bit))return false;
        return invalidate(captured);
    }
    // Recoverable engine/SDK loss cancels this generation only. Keep its
    // generation so replay needs a strictly newer validated commit. A newer
    // commit published during a failed setter is never cleared by that failure.
    bool invalidate(uint64_t captured) noexcept {
        return (captured&active_bit)&&word_.compare_exchange_strong(captured,captured&~active_bit,std::memory_order_acq_rel);
    }
    void retire(uint32_t source) noexcept {
        if(source<capacity)retired_[source].store(true,std::memory_order_release);
    }
    void disable() noexcept {disabled_.store(true,std::memory_order_release);}
    uint64_t snapshot() const noexcept {return word_.load(std::memory_order_acquire);}
    static uint32_t source_index(uint64_t word) noexcept {return ticket(word);}
    bool suppress(uint64_t word) const noexcept {
        return !disabled_.load(std::memory_order_acquire)&&generation(word)&&(word&active_bit)&&!retired_[ticket(word)].load(std::memory_order_acquire);
    }
    bool suppress() const noexcept {
        return suppress(snapshot());
    }
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(std::atomic<bool>::is_always_lock_free);
}
