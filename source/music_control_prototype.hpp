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
    uint64_t last_word_{};bool last_desired_{},last_valid_{};
    struct Decision {uint64_t word;bool desired;};
    Decision decision() noexcept {
        auto word=intent.snapshot();bool desired=intent.suppress(word);
        if(desired&&!live_(PlaybackIntent::source_index(word))){intent.invalidate(word);desired=false;}
        return {word,desired};
    }
    bool usable() noexcept {
        if(initialized_())return true;
        intent.invalidate(intent.snapshot());last_valid_=false;return false;
    }
    int apply(Decision command,int32_t duration) noexcept {
        int result=set_(parameter_,command.desired?-35.f:-48.f,UINT64_MAX,duration,4,false);
        if(log_)log_(command.word,command.desired,result);
        last_valid_=false; // A failed or superseded call is never acknowledged.
        return result;
    }
    void acknowledge(Decision command,Decision current) noexcept {
        if(command.word==current.word&&command.desired==current.desired){
            last_word_=command.word;last_desired_=command.desired;last_valid_=true;
        }
    }
public:
    // Set up once before publishing a host-update wrapper. Never from audio.
    void prepare(uint32_t parameter,Set set,Initialized initialized,Live live,Log log) noexcept {
        parameter_=parameter;set_=set;initialized_=initialized;live_=live;log_=log;
    }
    void enable() noexcept {if(parameter_&&set_&&initialized_&&live_)enabled_.store(true,std::memory_order_release);}
    void pump() noexcept {
        if(!enabled_.load(std::memory_order_acquire)||pumping_.test_and_set(std::memory_order_acquire))return;
        struct Release {std::atomic_flag& flag;~Release(){flag.clear(std::memory_order_release);}} release{pumping_};
        if(!usable())return;
        auto command=decision();
        if(last_valid_&&last_word_==command.word&&last_desired_==command.desired)return;
        // Decisions are short-lived values. No retained native object/gain
        // pointer, audio-thread call, background SDK pump or old-command retry.
        int result=apply(command,command.desired?150:750);
        if(result!=1)intent.invalidate(command.word);
        if(!usable())return; // Never compensate through an unusable SDK.
        auto current=decision();
        if(result==1)acknowledge(command,current);
        // One immediate neutral compensation for a failed/obsolete enable.
        // Reentrant pumps do not spin; this owner rechecks their changed intent.
        // At most two SDK calls per pump. Failed release stays uncached for the
        // next qualified control opportunity; final-callback service is unproved.
        if(command.desired&&(result!=1||!current.desired)){
            Decision neutral{current.word,false};
            if(apply(neutral,0)==1&&usable())acknowledge(neutral,decision());
        }
    }
};
}
