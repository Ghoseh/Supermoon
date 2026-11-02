// Music-only soundtrack control; SDK calls occur on native control paths.
#pragma once
#include "playback_intent_prototype.hpp"
namespace supermoon::floor_prototype {
class MusicControl {
public:
    using Set=int(*)(uint32_t,float,uint64_t,int32_t,uint32_t,bool);
    using Initialized=bool(*)();
    using Live=bool(*)(uint32_t) noexcept;
    using Log=void(*)(uint64_t,bool,int) noexcept;
    PlaybackIntent intent;
private:
    Set set_{};Initialized initialized_{};Live live_{};Log log_{};
    uint32_t parameter_{};std::atomic<bool> enabled_{false};
    std::atomic_flag pumping_=ATOMIC_FLAG_INIT;
    uint64_t last_word_{};bool last_desired_{},last_valid_{},seen_initialized_{};
public:
    // Set up once before publishing a host-update wrapper. Never from audio.
    void prepare(uint32_t parameter,Set set,Initialized initialized,Live live,Log log) noexcept {
        parameter_=parameter;set_=set;initialized_=initialized;live_=live;log_=log;
    }
    void enable() noexcept {if(parameter_&&set_&&initialized_&&live_)enabled_.store(true,std::memory_order_release);}
    void pump() noexcept {
        if(!enabled_.load(std::memory_order_acquire)||pumping_.test_and_set(std::memory_order_acquire))return;
        struct Release {std::atomic_flag& flag;~Release(){flag.clear(std::memory_order_release);}} release{pumping_};
        if(!initialized_()){if(seen_initialized_)intent.disable();last_valid_=false;return;}
        seen_initialized_=true;
        auto word=intent.snapshot();bool desired=intent.suppress(word)&&live_(PlaybackIntent::source_index(word));
        if(last_valid_&&last_word_==word&&last_desired_==desired)return;
        // Decisions are short-lived values. No retained native object/gain
        // pointer, audio-thread call, background SDK pump or old-command retry.
        int result=set_(parameter_,desired?-35.f:-48.f,UINT64_MAX,desired?150:750,4,false);
        if(log_)log_(word,desired,result);
        if(result==1){last_word_=word;last_desired_=desired;last_valid_=true;return;}
        intent.disable();last_valid_=false;
        // A failed enabling command cannot leave the controller requesting on.
        // Release is retried from fresh state on the next normal host update.
        if(desired){int neutral=set_(parameter_,-48.f,UINT64_MAX,0,4,false);if(log_)log_(word,false,neutral);if(neutral==1){last_word_=word;last_desired_=false;last_valid_=true;}}
    }
};
}
