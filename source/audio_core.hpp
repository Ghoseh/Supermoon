// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "playlist/stream_engine.hpp"

namespace bytebeat {
// Profile evidence supplied before publication, not live telemetry or a proof of
// callback draining. Offline fixtures may supply their own owned-context evidence.
// pre_gain_boundary is a legacy field name: an equivalent completed-frame
// adapter must also prove that it applies the exact native gain/envelope.
struct SafetyEvidence {
    bool identity{}, pre_gain_boundary{}, native_envelope{}, single_consumer{}, lifetime{}, duration{};
    bool ready() const { return identity && pre_gain_boundary && native_envelope && single_consumer && lifetime && duration; }
};
struct FramePlan { bool replace{}; float sample{}; uint64_t generation{}, cursor{}; playlist::Engine::Frame streaming{}; };
// One resident Core/Engine binding, gated by the current owned source ticket;
// neither a composition key nor a numeric PlayingID is an owner incarnation.
// Commit/frame/cursor state requires serialized sidecar access; SupermoonCore
// supplies its guard and validated native call sites. disable()/shutdown() only
// signal Engine atomics; configuration occurs before publication.
// The legacy backend borrows a resident vector; production borrows the resident
// Engine and calls its bounded peek/consume/notification APIs. Only published PCM
// blocks are immutable until consumption; the worker streams the remaining cache.
// No allocation, file I/O, decoding, command dispatch or ownership transfer here.
class AudioCore {
    const std::vector<float>* song_{};
    playlist::Engine* engine_{};
    SafetyEvidence evidence_{};
    uint64_t generation_{}, cursor_{};
    bool carrier_{},carrier_started_{};
public:
    explicit AudioCore(const std::vector<float>* resident_song):song_(resident_song) {}
    explicit AudioCore(playlist::Engine& engine):engine_(&engine) {}
    void disable() noexcept {if(engine_)engine_->set_active(false);}
    void shutdown() noexcept {if(engine_)engine_->request_stop();}
    bool carrier_active() const noexcept {return carrier_&&evidence_.ready()&&(engine_?engine_->healthy():(song_&&!song_->empty()));}
    void evidence(SafetyEvidence evidence) { evidence_=evidence; }
    // Call only for a successful composition commit UNDER the native lock.
    // Repeated same composition still gets a fresh monotonic generation.
    bool commit(uint64_t generation, bool verified_carrier) noexcept {
        if(generation==0 || generation<=generation_) return false;
        generation_=generation; cursor_=0; carrier_=verified_carrier;
        if(engine_){
            // Preserve the first prefetched queue; no custom audio/commands could
            // have been consumed before initial activation. Later Play restarts.
            if(verified_carrier){if(carrier_started_)engine_->restart();carrier_started_=true;}
            engine_->set_active(verified_carrier);
        }
        return true;
    }
    FramePlan begin_frame(bool native_will_produce) const noexcept {
        if(engine_){bool use=native_will_produce&&carrier_active();
            auto frame=use?engine_->peek():playlist::Engine::Frame{};
            return {use,frame.pcm?frame.sample:0.f,generation_,cursor_,frame};}
        bool use=native_will_produce && carrier_ && evidence_.ready() && song_ && !song_->empty();
        return {use,use && cursor_<song_->size()?(*song_)[size_t(cursor_)]:0.f,generation_,cursor_};
    }
    // Historical portable candidate only; superseded by supermoon_core.hpp.
    // Disabled native lanes can skip this contribution, so it is not an adapter.
    // Candidate integration: replace dry lane contribution, BEFORE native gain,
    // envelope and clamping. Use song once on lane 0 and zero lanes 1..7.
    // This is NOT an installed hook, and the internal lane contract is unverified.
    static float lane(const FramePlan& plan,unsigned lane,float native_sample) noexcept {
        return plan.replace && lane<8 ? (lane==0?plan.sample:0.f) : native_sample;
    }
    bool finish_frame(const FramePlan& plan,bool native_produced) noexcept {
        if(plan.generation!=generation_ || plan.cursor!=cursor_) return false;
        if(engine_){
            // A control generation may change after peek. One captured frame
            // may finish; stale queue state is never consumed or faulted.
            if(plan.replace&&native_produced&&plan.streaming.pcm&&engine_->consume(plan.streaming))++cursor_;
        } else if(plan.replace && native_produced && cursor_<song_->size()) ++cursor_;
        return true;
    }
    uint64_t cursor() const noexcept { return cursor_; }
    bool streaming() const noexcept {return engine_!=nullptr;}
    bool diagnostic_snapshot(playlist::PlaybackSnapshot& value,uint64_t& revision) const noexcept {
        if(!engine_)return false;revision=engine_->diagnostic_revision();return engine_->now_playing_snapshot(value);
    }
    float notification_sample(const FramePlan& plan,float envelope,float gain) noexcept {
        return engine_&&plan.replace?engine_->notification_sample(plan.streaming,envelope,gain):0.f;
    }
    uint64_t generation() const noexcept { return generation_; }
};
}
