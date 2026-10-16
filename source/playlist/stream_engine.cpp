// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "stream_engine.hpp"
#include <chrono>
#include <deque>
#include <memory>
#include <stdexcept>

namespace bytebeat::playlist {
Engine::Engine(std::vector<Prepared> tracks,uint32_t seed,uint32_t ramp,bool shuffle,std::function<bool(bool)> persist,bool notifications):tracks_(std::move(tracks)),shuffle_(shuffle),persist_(std::move(persist)),notifications_(notifications),seed_(seed),ramp_(std::min(ramp,1024u)){
    if(tracks_.empty()||tracks_.size()>10000)throw std::runtime_error("Playlist requires 1..10000 prepared tracks");
    worker_=std::thread([this]{run();});
}
Engine::~Engine(){request_stop();if(worker_.joinable())worker_.join();}
bool Engine::join_finished_worker() noexcept {
    if(worker_joined())return true;
    if(!worker_exited()||worker_join_failed())return false;
    if(join_owner_.test_and_set(std::memory_order_acquire))return false;
    struct Release {std::atomic_flag& flag;~Release(){flag.clear(std::memory_order_release);}} release{join_owner_};
    if(worker_joined())return true;
    try {
        if(worker_.get_id()==std::this_thread::get_id())return false;
        // May wait for the runtime epilogue; only this diagnostic worker calls
        // it, never an audio/native callback, DllMain or the game main thread.
        if(worker_.joinable())worker_.join();
        worker_joined_.store(true,std::memory_order_release);return true;
    }catch(...){worker_join_failed_.store(true,std::memory_order_release);return false;}
}
bool Engine::command(Command c) noexcept {
    if(!active()||!healthy()||(c!=Command::next&&c!=Command::previous&&c!=Command::restart&&c!=Command::shuffle&&c!=Command::toggle_shuffle))return false;
    // Native UI and an optional key worker would be separate producers.
    // Serialize admission by trying once, never wait or overwrite another slot.
    if(command_producer_.test_and_set(std::memory_order_acquire)){rejected_.fetch_add(1,std::memory_order_relaxed);return false;}
    struct Release {std::atomic_flag& flag;~Release(){flag.clear(std::memory_order_release);}} release{command_producer_};
    if(!active()||!healthy())return false;
    auto w=command_write_.load(std::memory_order_relaxed);
    if(w-command_read_.load(std::memory_order_acquire)==command_count){rejected_.fetch_add(1,std::memory_order_relaxed);return false;}
    auto epoch=activity_epoch_.load(std::memory_order_acquire),revision=revision_.load(std::memory_order_acquire),restart=restart_epoch_.load(std::memory_order_acquire);
    PlaybackSnapshot snapshot{};bool present=publication_.read(snapshot)&&snapshot.revision==revision;
    if(!active()||activity_epoch_.load(std::memory_order_acquire)!=epoch)return false;
    commands_[w%command_count]={c,epoch,restart,present?snapshot.serial:0,present?snapshot.position:0};
    command_write_.store(w+1,std::memory_order_release);return true;
}
Engine::Frame Engine::peek() noexcept {
    if(!active()){Frame f{};f.absence=1;return f;}
    if(!healthy()){Frame f{};f.absence=2;return f;}
    auto revision=revision_.load(std::memory_order_acquire);
    bool stale=false;
    // Bounded retirement of stale blocks; only consumer releases queue slots.
    for(size_t attempt=0;attempt<block_count;++attempt){
        auto r=read_.load(std::memory_order_relaxed);
        if(r==write_.load(std::memory_order_acquire))break;
        auto& b=blocks_[r%block_count];
        if(b.revision!=revision){stale=true;offset_=0;read_.store(r+1,std::memory_order_release);continue;}
        auto pos=b.start+offset_;
        return {b.samples[offset_],b.track,pos,b.length,b.serial,b.revision,r,offset_,true,pos+1==b.length};
    }
    underruns_.fetch_add(1,std::memory_order_relaxed);Frame f{};f.absence=stale?4:3;return f;
}
float Engine::notification_sample(const Frame& f,float envelope,float gain) noexcept {
    if(!f.pcm||!active()||!healthy()||f.track>=tracks_.size()||f.revision!=revision_.load(std::memory_order_acquire)||
       f.block!=read_.load(std::memory_order_relaxed)||f.offset!=offset_)return 0;
    const auto& track=tracks_[f.track];
    double bound=std::min(1.,track.measured_reconstructed_peak*track.gain+0.00001);
    return notifications_.sample(f.sample,bound,envelope,gain);
}
bool Engine::consume(const Frame& f) noexcept {
    if(!f.pcm||!active()||!healthy()||f.revision!=revision_.load(std::memory_order_acquire)||
       f.block!=read_.load(std::memory_order_relaxed)||f.offset!=offset_)return false;
    audible_serial_.store(f.serial,std::memory_order_release);heard_track_.store(f.track,std::memory_order_release);pcm_.fetch_add(1,std::memory_order_relaxed);
    publication_.publish({f.serial,f.revision,f.position+1,f.length,f.track});
    notifications_.consume();
    if(f.last)completed_.fetch_add(1,std::memory_order_relaxed);
    if(++offset_==blocks_[f.block%block_count].count){offset_=0;read_.store(f.block+1,std::memory_order_release);}return true;
}
Engine::Stats Engine::stats() const noexcept {return {pcm_.load(),underruns_.load(),completed_.load(),rejected_.load(),failed_.load()};}
void Engine::run() noexcept {
    // Created before readers/state so the completion latch is published only
    // after worker-owned files have closed. This is not native callback drain.
    struct Completion {std::atomic<bool>& exited;~Completion(){exited.store(true,std::memory_order_release);}} completion{worker_exited_};
    try {
        std::vector<uint32_t> valid;
        for(uint32_t i=0;i<tracks_.size();++i){if(stop_.load(std::memory_order_acquire))return;try{CacheReader test(tracks_[i]);valid.push_back(i);}catch(...){failed_.fetch_add(1);}}
        if(valid.empty()){healthy_.store(false,std::memory_order_release);return;}
        struct State {uint64_t serial;Order order;};
        Order order(valid,seed_,shuffle_enabled());uint32_t track=order.next();uint64_t serial=1,position=0,revision=revision_.load();
        std::deque<State> states;states.push_back({serial,order});std::unique_ptr<CacheReader> reader;
        auto select=[&]{reader=std::make_unique<CacheReader>(tracks_[track]);position=0;};
        while(!stop_.load(std::memory_order_acquire)){
            try {
                if(!reader)select();
                auto heard=audible_serial_.load(std::memory_order_acquire);
                while(states.size()>1&&states[1].serial<=heard)states.pop_front();
                auto target_revision=revision_.load(std::memory_order_acquire);
                auto cr=command_read_.load(std::memory_order_relaxed),cw=command_write_.load(std::memory_order_acquire);
                if(target_revision!=revision||cr!=cw){
                    auto found=std::find_if(states.begin(),states.end(),[&](auto& state){return state.serial==heard;});
                    Order choice=found!=states.end()?found->order:states.front().order;
                    uint32_t chosen=choice.current();
                    bool refresh=target_revision!=revision,navigation=refresh,changed=false;
                    while(cr!=cw){
                        auto control=commands_[cr++%command_count];
                        if(!active()||control.epoch!=activity_epoch_.load(std::memory_order_acquire)||control.restart_epoch!=restart_epoch_.load(std::memory_order_acquire)){rejected_.fetch_add(1);continue;}
                        if(control.action==Command::toggle_shuffle){
                            auto notification_epoch=notifications_.epoch();
                            bool mode=!choice.shuffled();bool saved=true;
                            try{if(persist_)saved=persist_(mode);}catch(...){saved=false;}
                            if(!saved){shuffle_failures_.fetch_add(1);continue;} // Never quarantine an audio track for preference I/O.
                            choice.set_shuffle(mode);shuffle_.store(mode,std::memory_order_release);changed=true;
                            if(active()&&control.epoch==activity_epoch_.load(std::memory_order_acquire)&&
                               control.restart_epoch==restart_epoch_.load(std::memory_order_acquire))
                                notifications_.publish(mode,notification_epoch);
                        }else{
                            if(control.action==Command::next)chosen=choice.next();
                            else if(control.action==Command::shuffle)chosen=choice.shuffle_next();
                            else if(control.action==Command::previous){
                                // Request-time consumed 24 kHz PCM position; never carrier time.
                                if(navigation||control.serial!=heard||control.position<=uint64_t(3)*24000)chosen=choice.previous();
                            }
                            navigation=true;changed=true;
                        }
                    }
                    command_read_.store(cr,std::memory_order_release);
                    if(refresh||changed){
                        // A pure toggle changes future order without retiring queued
                        // PCM or reconstructing a reader from a moving consumer.
                        // Already published lookahead retains its track sequence,
                        // serial and revision (up to 16,384 frames at 24 kHz).
                        // Navigation/restart still invalidates stale revisions;
                        // paused consumption can extend the wall-clock mode delay.
                        target_revision=revision_.load(std::memory_order_acquire);
                        if(target_revision!=revision)navigation=true;
                        if(!navigation){
                            order.set_shuffle(shuffle_enabled());
                            for(auto& state:states)state.order.set_shuffle(shuffle_enabled());
                            // EOF may already have selected an unpublished next
                            // song while the ring was full. Rechoose only that
                            // uncommitted selection from the last queued song.
                            if(!position&&states.size()>1){
                                states.pop_back();order=states.back().order;track=order.next();
                                states.push_back({serial,order});reader.reset();select();
                            }
                        }else{
                            order=std::move(choice);track=chosen;
                            if(target_revision==revision)target_revision=revision_.fetch_add(1,std::memory_order_acq_rel)+1;
                            revision=target_revision;states.clear();states.push_back({++serial,order});select();
                        }
                    }
                }
                auto w=write_.load(std::memory_order_relaxed);
                if(w-read_.load(std::memory_order_acquire)==block_count){std::this_thread::sleep_for(std::chrono::milliseconds(1));continue;}
                auto& block=blocks_[w%block_count];
                block.start=position;block.length=reader->frames();block.serial=states.back().serial;block.revision=revision;block.track=track;
                block.count=uint32_t(reader->read(block.samples.data(),block_frames));
                if(!block.count)throw std::runtime_error("Unexpected private-cache EOF");
                auto ramp=std::min<uint64_t>(ramp_,block.length/2);
                if(ramp)for(uint32_t i=0;i<block.count;++i){auto p=position+i;float gain=float(std::min<uint64_t>(ramp,std::min(p,block.length-1-p)))/float(ramp);block.samples[i]*=gain;}
                position+=block.count;
                if(revision!=revision_.load(std::memory_order_acquire))continue; // Unpublished data may be discarded.
                write_.store(w+1,std::memory_order_release);
                if(position==reader->frames()){track=order.next();states.push_back({++serial,order});reader.reset();}
            } catch(...) {
                // Quarantine a failed prepared track. Rebuild eligible order rather
                // than repeatedly selecting a broken cache; history resets here.
                failed_.fetch_add(1);valid.erase(std::remove(valid.begin(),valid.end(),track),valid.end());
                if(valid.empty())throw;
                order=Order(valid,seed_,shuffle_enabled());track=order.next();reader.reset();
                revision=revision_.fetch_add(1,std::memory_order_acq_rel)+1;
                states.clear();states.push_back({++serial,order});
            }
        }
    } catch(...) {healthy_.store(false,std::memory_order_release);}
}
}
