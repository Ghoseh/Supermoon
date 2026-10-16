// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "order.hpp"
#include "wav_pipeline.hpp"
#include "now_playing.hpp"
#include "notifications.hpp"
#include <array>
#include <atomic>
#include <thread>
#include <functional>

namespace bytebeat::playlist {
// One serialized audio consumer, bounded nonblocking control producers, one owned disk worker.
// Construction/destruction, commands and joining occur outside native callbacks.
// The consumer alone owns read_/offset_; the worker owns write_, readers and Order.
// Release/acquire pairs publish filled blocks and reclaim storage after consumption.
// Published objects are resident; worker join does not establish native draining.
class Engine {
public:
    static constexpr size_t block_frames=1024, block_count=16, command_count=32;
    struct Frame {
        float sample{}; uint32_t track{};
        uint64_t position{},length{},serial{},revision{},block{},offset{};
        bool pcm{},last{};
        // Observation only: exact unavailable branch, never a playback decision.
        // 0 PCM/unspecified,1 inactive,2 unhealthy,3 empty,4 stale blocks retired.
        uint8_t absence{};
    };
    struct Stats {uint64_t pcm_frames{},underruns{},completed{},rejected{},failed{};};
    explicit Engine(std::vector<Prepared>,uint32_t seed=0,uint32_t edge_ramp_frames=64,bool shuffle=true,std::function<bool(bool)> persist={},bool notifications=false);
    ~Engine();
    Engine(const Engine&)=delete;Engine& operator=(const Engine&)=delete;
    void set_active(bool value) noexcept {if(active_.exchange(value,std::memory_order_acq_rel)!=value){activity_epoch_.fetch_add(1,std::memory_order_acq_rel);notifications_.cancel();}}
    bool active() const noexcept {return !stop_.load(std::memory_order_acquire)&&active_.load(std::memory_order_acquire);}
    void request_stop() noexcept {stop_.store(true,std::memory_order_release);active_.store(false,std::memory_order_release);notifications_.cancel();}
    bool worker_exited() const noexcept {return worker_exited_.load(std::memory_order_acquire);}
    // Off-callback diagnostic owner only. Published Engine stays resident;
    // no destructor may race this method. Scope exit alone is not thread join.
    bool join_finished_worker() noexcept;
    bool worker_joined() const noexcept {return worker_joined_.load(std::memory_order_acquire);}
    bool worker_join_failed() const noexcept {return worker_join_failed_.load(std::memory_order_acquire);}
    bool now_playing_snapshot(PlaybackSnapshot& value) const noexcept {
        return active()&&healthy()&&publication_.read(value)&&value.revision==revision_.load(std::memory_order_acquire);
    }
    bool stop_requested() const noexcept{return stop_.load(std::memory_order_acquire);}
    bool shuffle_enabled() const noexcept{return shuffle_.load(std::memory_order_acquire);}
    uint64_t shuffle_failures() const noexcept{return shuffle_failures_.load();}
    bool healthy() const noexcept {return healthy_.load(std::memory_order_acquire);}
    uint64_t diagnostic_revision() const noexcept {return revision_.load(std::memory_order_acquire);}
    bool command(Command) noexcept; // Serialized try-once producers; off audio thread.
    void restart() noexcept {restart_epoch_.fetch_add(1,std::memory_order_acq_rel);revision_.fetch_add(1,std::memory_order_acq_rel);notifications_.cancel();}
    float notification_sample(const Frame& f,float envelope,float gain) noexcept;
    uint64_t notification_commits()const noexcept{return notifications_.committed();}
    Frame peek() noexcept; // Never allocates, locks, reads disk or waits.
    bool consume(const Frame&) noexcept; // Exactly once per produced PCM frame.
    Stats stats() const noexcept;
    uint32_t heard_track() const noexcept {return heard_track_.load(std::memory_order_acquire);}
private:
    struct Block {std::array<float,block_frames> samples{};uint64_t start{},length{},serial{},revision{};uint32_t track{},count{};};
    std::vector<Prepared> tracks_;
    std::array<Block,block_count> blocks_{};
    alignas(64) std::atomic<uint64_t> read_{0};
    alignas(64) std::atomic<uint64_t> write_{0};
    uint32_t offset_{}; // Consumer only.
    struct Control {Command action;uint64_t epoch,restart_epoch,serial,position;};
    std::array<Control,command_count> commands_{};
    std::atomic<uint64_t> activity_epoch_{0},restart_epoch_{0},shuffle_failures_{0};
    std::atomic<bool> shuffle_{true};
    std::function<bool(bool)> persist_;
    Notifications notifications_;
    std::atomic<uint64_t> command_read_{0},command_write_{0},revision_{1};
    std::atomic_flag command_producer_=ATOMIC_FLAG_INIT;
    std::atomic<bool> stop_{false},healthy_{true},active_{false};
    std::atomic<uint64_t> pcm_{0},underruns_{0},completed_{0},rejected_{0},failed_{0},audible_serial_{0};
    std::atomic<uint32_t> heard_track_{UINT32_MAX};
    PlaybackPublication publication_;
    std::atomic<bool> worker_exited_{false},worker_joined_{false},worker_join_failed_{false};
    std::atomic_flag join_owner_=ATOMIC_FLAG_INIT;
    uint32_t seed_,ramp_;
    std::thread worker_;
    void run() noexcept;
};
static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(std::atomic<bool>::is_always_lock_free);
}
