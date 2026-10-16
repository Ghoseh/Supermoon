// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace bytebeat::playlist {
// Worker-only order/history. Existing prepared-library order is authoritative.
class Order {
    std::vector<uint32_t> eligible_,bag_,history_;
    size_t next_{},history_position_{};
    std::mt19937 random_;
    uint64_t cycles_{};
    bool shuffle_{true};
    void refill(){
        bag_=eligible_;std::shuffle(bag_.begin(),bag_.end(),random_);next_=0;++cycles_;
        if(bag_.size()>1&&!history_.empty()&&bag_[0]==current())
            std::swap(bag_[0],*std::find_if(bag_.begin()+1,bag_.end(),[&](auto v){return v!=current();}));
    }
    uint32_t remember(uint32_t id){
        if(!history_.empty())history_.resize(history_position_+1);
        history_.push_back(id);history_position_=history_.size()-1;
        if(history_.size()>1024){history_.erase(history_.begin());--history_position_;}
        return id;
    }
    uint32_t normal(bool forward) const {
        auto at=std::find(eligible_.begin(),eligible_.end(),current());
        if(at==eligible_.end())throw std::runtime_error("Missing current playlist identity");
        auto index=size_t(at-eligible_.begin());
        // Conservative existing beginning-of-history behavior: no invented
        // Previous wrap at the first library entry. Next/EOF repeats normally.
        return forward?eligible_[(index+1)%eligible_.size()]:eligible_[index?index-1:0];
    }
public:
    explicit Order(std::vector<uint32_t> ids,uint32_t seed=0,bool shuffle=true):eligible_(std::move(ids)),random_(seed),shuffle_(shuffle){
        auto sorted=eligible_;std::sort(sorted.begin(),sorted.end());
        if(std::adjacent_find(sorted.begin(),sorted.end())!=sorted.end())throw std::runtime_error("Duplicate playlist identity");
    }
    bool empty() const{return eligible_.empty();}
    bool shuffled() const noexcept{return shuffle_;}
    void set_shuffle(bool value){
        if(value==shuffle_)return;
        shuffle_=value;bag_.clear();next_=0;
        if(!history_.empty())history_.resize(history_position_+1);
    }
    uint32_t next(){
        if(empty())throw std::runtime_error("Empty playlist");
        if(!shuffle_)return remember(history_.empty()?eligible_[0]:normal(true));
        if(!history_.empty()&&history_position_+1<history_.size())return history_[++history_position_];
        if(next_==bag_.size())refill();return remember(bag_[next_++]);
    }
    uint32_t shuffle_next(){
        if(empty())throw std::runtime_error("Empty playlist");
        if(!history_.empty())history_.resize(history_position_+1);
        bag_.clear();next_=0;refill();return remember(bag_[next_++]);
    }
    uint32_t previous(){
        if(history_.empty())return next();
        if(!shuffle_){auto id=normal(false);return id==current()?id:remember(id);}
        if(history_position_)--history_position_;return current();
    }
    uint32_t current() const{if(history_.empty())throw std::runtime_error("No current song");return history_[history_position_];}
    uint64_t cycles() const{return cycles_;}
};
enum class Command:uint8_t {next,previous,restart,shuffle,toggle_shuffle};
// Retained compatibility for existing focused regression fixtures, not the new
// integration. New runtime input uses KeyboardEdges with exact modifiers.
struct InputEdges {
    bool next_down{},previous_down{},eligible_before{};
    template<class Send> void poll(bool foreground,bool mapped,bool shift,bool next,bool previous,Send send){
        bool eligible=foreground&&mapped;bool n=shift&&next,p=shift&&previous;
        if(eligible&&eligible_before){if(n&&!next_down)send(Command::next);if(p&&!previous_down)send(Command::previous);}
        next_down=n;previous_down=p;eligible_before=eligible;
    }
};
}
