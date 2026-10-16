// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "audio_core.hpp"
#include "endurance.hpp"
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>

namespace bytebeat {
using CompositionKey = std::array<uint8_t,16>;
// Borrowed completed-frame values are read under the native synthesis lock.
// The carrier clock is continuity evidence, not track identity or custom position.
// Recovery supplies an owned source ticket in stream; UI keeps the native address
// separately. Consumed prepared PCM determines playlist position.
struct CompletedFrame {
    uintptr_t stream{};
    uint32_t clock{}, index{}, requested{}, produced{};
    float envelope{}, gain{};
    float* output{};
};
enum class FrameResult : uint32_t { native=0, substituted=1, eof=2, invalid=3, contention=4 };
// One resident personal-playback sidecar; its try-once guard serializes state.
// Production borrows a streaming Engine, not a whole immutable song vector.
// Qualified frame calls do not allocate, wait or chase retired native pointers.
class SupermoonCore {
    AudioCore audio_;
    CompositionKey carrier_{};
    bool key_known_{}, armed_{}, bound_{}, clock_known_{}, lifecycle_closed_{};
    uintptr_t stream_{};
    std::atomic<uintptr_t> owner_stream_{0};
    std::atomic<uintptr_t> callback_stream_{0}; // Fast reject before shared guard, no native dereference.
    std::atomic<uintptr_t> endpoint_ui_stream_{0};
    bool (*endpoint_live_)(uintptr_t) noexcept {};
    uint32_t next_clock_{};
    uint64_t generation_{};
    std::atomic_flag guard_=ATOMIC_FLAG_INIT;
    std::atomic<bool> fault_{false}, retired_{true};
    struct Guard {
        // Try once instead of waiting on the audio thread. The separate native
        // lock protects NMS synthesis; this guard protects our sidecar state.
        std::atomic_flag& flag; bool held;
        explicit Guard(std::atomic_flag& f):flag(f),held(!f.test_and_set(std::memory_order_acquire)){}
        ~Guard(){if(held)flag.clear(std::memory_order_release);}
    };
public:
    explicit SupermoonCore(const std::vector<float>* song):audio_(song){}
    explicit SupermoonCore(playlist::Engine& engine):audio_(engine){}
    // Installed once before engine_ready enables Core access. It reads resident
    // owned endpoint state, never follows a native owner/context pointer.
    void endpoint_validator(bool (*check)(uintptr_t) noexcept) noexcept {endpoint_live_=check;}
    bool commit_endpoint(uintptr_t ticket,uintptr_t native_stream,const CompositionKey& key,uint32_t clock,uint64_t* generation=nullptr) noexcept {
        Guard guard(guard_);if(!guard.held||!endpoint_live_||!endpoint_live_(ticket)||!native_stream)return false;
        if(generation_==UINT64_MAX){invalidate();return false;}
        // An immutable owned source ticket is a new binding. Legacy commit()
        // still refuses terminal native-pointer rebinding, unchanged below.
        if(owner_stream_.load(std::memory_order_acquire)!=ticket)fault_.store(false,std::memory_order_release);
        if(fault_.load(std::memory_order_acquire))return false;
        ++generation_;stream_=ticket;owner_stream_.store(ticket,std::memory_order_release);next_clock_=clock;clock_known_=true;bound_=true;
        retired_.store(false,std::memory_order_release);endpoint_ui_stream_.store(native_stream,std::memory_order_release);
        if(generation)*generation=generation_;
        bool committed=audio_.commit(generation_,armed_&&key_known_&&key==carrier_);
        callback_stream_.store(committed?ticket:0,std::memory_order_release);return committed;
    }
    // Production retirement pauses custom consumption without shutting down
    // the resident worker. A later validated native Play may commit a new ticket.
    bool retire_endpoint(uintptr_t ticket) noexcept {
        Guard guard(guard_);if(!guard.held)return false;
        if(owner_stream_.load(std::memory_order_acquire)!=ticket)return true;
        callback_stream_.store(0,std::memory_order_release);endpoint_ui_stream_.store(0,std::memory_order_release);
        audio_.disable();retired_.store(true,std::memory_order_release);bound_=false;return true;
    }
    // Configure before Core admission. Diagnostic-only builds never arm PCM.
    void configure(const CompositionKey& key,bool known,SafetyEvidence evidence,bool pcm) noexcept {
        carrier_=key; key_known_=known; audio_.evidence(evidence); armed_=pcm&&known&&evidence.ready();
    }
    // Legacy native-pointer path, retained for non-recovery builds/tests.
    // Native setter succeeded; the caller holds the native synthesis lock.
    bool commit(uintptr_t stream,const CompositionKey& key,bool personal,uint32_t clock,uint64_t* generation=nullptr) noexcept {
        // stream is an identity token here, not memory owned by this class.
        // A generation represents a successful native reset, not a track index.
        Guard guard(guard_);
        if(!guard.held){fault_.store(true,std::memory_order_release);return false;}
        // Unrecognized ownership cannot leave an old mapping active.
        if(!personal || !stream){callback_stream_.store(0,std::memory_order_release);audio_.disable();bound_=false;retired_.store(true,std::memory_order_release);return false;}
        // This legacy path has no incarnation ticket, so terminal native-
        // pointer retirement stays closed. Production uses commit_endpoint().
        if(lifecycle_closed_)return false;
        if(generation_==UINT64_MAX){fault_.store(true);return false;}
        ++generation_; stream_=stream;owner_stream_.store(stream,std::memory_order_release); next_clock_=clock; clock_known_=true; bound_=true;
        if(generation)*generation=generation_;
        retired_.store(false,std::memory_order_release);
        bool committed=audio_.commit(generation_,armed_&&key_known_&&key==carrier_);
        callback_stream_.store(committed?stream:0,std::memory_order_release);return committed;
    }
    // Legacy terminal retirement shuts down the worker; never reopen it by
    // pointer equality. Production lifecycle retirement uses retire_endpoint().
    void retire(uintptr_t stream=0) noexcept {
        // Unrelated owners cannot disable this worker through guard contention.
        if(stream&&stream!=owner_stream_.load(std::memory_order_acquire))return;
        Guard guard(guard_);
        if(!guard.held){invalidate();audio_.shutdown();retired_.store(true,std::memory_order_release);return;}
        if(stream&&stream!=stream_)return; // unrelated native owner
        callback_stream_.store(0,std::memory_order_release);audio_.disable();audio_.shutdown();
        if(stream_)lifecycle_closed_=true;
        retired_.store(true,std::memory_order_release);
    }
    void invalidate() noexcept {fault_.store(true,std::memory_order_release);callback_stream_.store(0,std::memory_order_release);audio_.disable();}
    static float processed(float sample,float envelope,float gain) noexcept {
        // Match native scalar SSE rounding/order and min/max, not reassociation/FMA.
        __m128 factor=_mm_mul_ss(_mm_set_ss(envelope),_mm_set_ss(gain));
        __m128 value=_mm_mul_ss(factor,_mm_set_ss(sample));
        value=_mm_max_ss(value,_mm_set_ss(-1.f));
        return _mm_cvtss_f32(_mm_min_ss(value,_mm_set_ss(1.f)));
    }
    FrameResult frame(const CompletedFrame& f,uint64_t* generation=nullptr,endurance::FrameInfo* diagnostic=nullptr) noexcept {
        if(diagnostic)*diagnostic={};
        if(f.stream!=callback_stream_.load(std::memory_order_acquire))return FrameResult::native;
        // Returning native leaves the already-calculated output untouched.
        // Continuity is measured in actual produced samples, not buffer capacity
        // or wall-clock time; stopped/empty callbacks never reach this function.
        Guard guard(guard_);
        if(!guard.held){invalidate();return FrameResult::contention;}
        if(fault_.load(std::memory_order_acquire)||retired_.load(std::memory_order_acquire)||!bound_||f.stream!=stream_) return FrameResult::native;
        if(!f.output || f.index>=f.requested || f.produced!=f.index || !std::isfinite(f.gain) || !std::isfinite(f.envelope) || f.gain<0 || f.envelope<0 || f.envelope>1 || !clock_known_ || f.clock!=next_clock_) {
            invalidate();return FrameResult::invalid;
        }
        ++next_clock_; // unsigned wrap matches native dword clock
        if(generation)*generation=generation_;
        if(!armed_) return FrameResult::native;
        auto plan=audio_.begin_frame(true);
        // AudioCore peeks the bounded Engine queue (legacy builds use a vector).
        // EOF provides a zero sample while native clocks and availability carry
        // on. No allocation, native Next request or duration change occurs here.
        if(!plan.replace) return FrameResult::native;
        if(diagnostic&&audio_.streaming()){
            diagnostic->streaming=true;diagnostic->available=plan.streaming.pcm;
            diagnostic->absence=plan.streaming.absence;
            if(plan.streaming.pcm){auto& p=plan.streaming;diagnostic->value={p.serial,p.revision,p.position,p.length,p.track};diagnostic->revision=p.revision;diagnostic->snapshot=true;}
            else diagnostic->snapshot=audio_.diagnostic_snapshot(diagnostic->value,diagnostic->revision);
        }
        float cue=audio_.notification_sample(plan,f.envelope,f.gain);
        // Keep exact baseline SSE behavior when no cue contributes. Main PCM,
        // clocks and publication remain untouched by the additive resident layer.
        float value=processed(cue==0.f?plan.sample:plan.sample+cue,f.envelope,f.gain);
        // finish is before the unconditional native produced/clock increments.
        // No failure/retry/branch to nonproduction exists between site and increments.
        if(!audio_.finish_frame(plan,true)){invalidate();return FrameResult::invalid;}
        f.output[f.index]=value;
        if(diagnostic)diagnostic->consumed=plan.cursor!=audio_.cursor();
        return plan.cursor==audio_.cursor()?FrameResult::eof:FrameResult::substituted;
    }
    // Called at the native progress comparison, under its synthesis lock.
    // Does not change clock, native state, WAV cursor or manual control flags.
    bool carrier_policy(uintptr_t stream,uint32_t clock) noexcept {
        if(stream!=callback_stream_.load(std::memory_order_acquire))return false;
        Guard guard(guard_);
        if(!guard.held){invalidate();return false;}
        if(!armed_||fault_.load(std::memory_order_acquire)||retired_.load(std::memory_order_acquire)
           ||!bound_||stream!=stream_||!clock_known_)return false;
        if(clock!=next_clock_){invalidate();return false;}
        return audio_.carrier_active();
    }
    // Only for initialization/offline tests or under guard; not asynchronous telemetry.
    uint64_t cursor_for_test() const noexcept{return audio_.cursor();}
    uint64_t generation_for_test() const noexcept{return generation_;}
    // UI-only identity read; no native dereference, mutation or audio guard.
    uintptr_t ui_bound_stream() const noexcept {
        auto stream=callback_stream_.load(std::memory_order_acquire);
        if(endpoint_live_){
            if(!stream||!endpoint_live_(stream)||fault_.load(std::memory_order_acquire)||retired_.load(std::memory_order_acquire))return 0;
            auto native=endpoint_ui_stream_.load(std::memory_order_acquire);
            return callback_stream_.load(std::memory_order_acquire)==stream?native:0;
        }
        if(!stream||fault_.load(std::memory_order_acquire)||retired_.load(std::memory_order_acquire)
            ||stream!=owner_stream_.load(std::memory_order_acquire))return 0;
        return callback_stream_.load(std::memory_order_acquire)==stream?stream:0;
    }
    bool faulted() const noexcept{return fault_.load(std::memory_order_acquire);}
};
}
