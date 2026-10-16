// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "filename_title.hpp"
#include "playlist/now_playing.hpp"
#include <array>
namespace supermoon::ui::progress {
inline bool valid(bytebeat::playlist::PlaybackSnapshot s) noexcept {
    return s.serial&&s.revision&&s.track!=UINT32_MAX&&s.length&&s.position<=s.length;
}
inline bool caption(bytebeat::playlist::PlaybackSnapshot s,std::array<char,32>& output,uint8_t& length) noexcept {
    if(!valid(s))return false;
    std::array<char,32> result{};size_t used=0;
    auto character=[&](char c){if(used==31)return false;result[used++]=c;return true;};
    auto time=[&](uint64_t frames,bool total){
        uint64_t seconds=frames/24000,minutes=seconds/60;char digits[20];unsigned n=0;
        if(total&&minutes<10&&!character(' '))return false; // Native %2d minimum width.
        do{digits[n++]=char('0'+minutes%10);minutes/=10;}while(minutes);
        while(n)if(!character(digits[--n]))return false;
        return character(':')&&character(char('0'+(seconds%60)/10))&&character(char('0'+seconds%10));
    };
    if(!time(s.position,false)||!character(' ')||!character('/')||!character(' ')||!time(s.length,true))return false;
    output=result;length=static_cast<uint8_t>(used);return true;
}
}
