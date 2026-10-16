// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>
namespace supermoon::ui {
// Prepared before UI publication from owned Library.sources. Immutable during UI
// use; no path operations, allocation, disk reads or native pointers in rendering.
struct FilenameTitle {std::array<char,256> bytes{};uint16_t length{};bool fallback{},truncated{};};
using FilenameTitles=std::vector<FilenameTitle>;
inline FilenameTitle filename_title(std::wstring_view filename) noexcept {
    FilenameTitle result;
    size_t n=filename.size();
    auto lower=[](wchar_t c){return c>=L'A'&&c<=L'Z'?c+32:c;};
    if(n>=4&&filename[n-4]==L'.'&&((lower(filename[n-3])==L'w'&&lower(filename[n-2])==L'a'&&lower(filename[n-1])==L'v')||
       (lower(filename[n-3])==L'm'&&lower(filename[n-2])==L'p'&&lower(filename[n-1])==L'3')))n-=4;
    for(size_t i=0;i<n;++i){
        uint32_t cp=static_cast<uint32_t>(filename[i]);
        if(cp>=0xd800&&cp<=0xdbff){
            if(i+1<n&&filename[i+1]>=0xdc00&&filename[i+1]<=0xdfff)
                cp=0x10000+((cp-0xd800)<<10)+(static_cast<uint32_t>(filename[++i])-0xdc00);
            else{cp='?';result.fallback=true;}
        }else if((cp>=0xdc00&&cp<=0xdfff)||cp>0x10ffff||cp<32||cp==127){cp='?';result.fallback=true;}
        char b[4];size_t count;
        if(cp<128){b[0]=static_cast<char>(cp);count=1;}
        else if(cp<2048){b[0]=static_cast<char>(0xc0|(cp>>6));b[1]=static_cast<char>(0x80|(cp&63));count=2;}
        else if(cp<65536){b[0]=static_cast<char>(0xe0|(cp>>12));b[1]=static_cast<char>(0x80|((cp>>6)&63));b[2]=static_cast<char>(0x80|(cp&63));count=3;}
        else{b[0]=static_cast<char>(0xf0|(cp>>18));b[1]=static_cast<char>(0x80|((cp>>12)&63));b[2]=static_cast<char>(0x80|((cp>>6)&63));b[3]=static_cast<char>(0x80|(cp&63));count=4;}
        if(result.length+count>255){result.truncated=true;break;}
        for(size_t j=0;j<count;++j)result.bytes[result.length++]=b[j];
    }
    // Empty basename has no displayable filename: native formatter is retained.
    return result;
}
}
