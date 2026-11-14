// SPDX-License-Identifier: MIT
// Private experimental personal-ByteBeat connection gain. No PCM processing.
#pragma once
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
namespace bytebeat::personal_gain {
inline constexpr uint32_t personal_event=0x1df2f9b5,sound_id=127184786,bus_id=1035353295;
struct Setting {double db{};float factor{1};bool valid{true};};
inline Setting parse(const wchar_t* text) noexcept {
    if(!text)return {0,1,false};
    while(*text==L' '||*text==L'\t')++text;
    const wchar_t* begin=text;bool digit=false,point=false;
    for(;*text&&*text!=L' '&&*text!=L'\t';++text){
        if(*text>=L'0'&&*text<=L'9'){digit=true;continue;}
        if(*text==L'.'&&!point){point=true;continue;}
        return {0,1,false};
    }
    const wchar_t* end=text;while(*text==L' '||*text==L'\t')++text;
    if(*text||!digit||end-begin>32)return {0,1,false};
    wchar_t* parsed{};double db=std::wcstod(begin,&parsed);
    if(parsed!=end||!std::isfinite(db)||db<0||db>6)return {0,1,false};
    return {db,float(std::pow(10.,db/20.)),true};
}
struct Info {void* cookie;uint64_t object;uint32_t playing,event;float* matrix;uint32_t input,output;float* base;float* emitter;void* context;void* mixer;};
static_assert(sizeof(Info)==72&&offsetof(Info,base)==40&&offsetof(Info,context)==56);
enum class Outcome:uint32_t {neutral,applied,identity,ambiguous,headroom,overlap,capacity};
// Addresses are opaque keys only. Rows never free/recycle or dereference them.
// Retaining the previous write across address/event reuse is conservative: an
// indistinguishable incoming target is skipped rather than multiplied again.
class Targets {
    struct Row {std::atomic<uintptr_t> address{0};std::atomic_flag busy=ATOMIC_FLAG_INIT;float last{};bool written{};};
    static constexpr unsigned capacity=256;Row rows_[capacity];
public:
    Outcome apply(uintptr_t address,float* target,float factor) noexcept {
        if(factor==1)return Outcome::neutral;
        if(!address||!target||!std::isfinite(factor)||factor<1||factor>1.995263f)return Outcome::identity;
        auto start=unsigned((address>>4)%capacity);Row* row=nullptr;
        for(unsigned n=0;n<capacity;++n){auto& r=rows_[(start+n)%capacity];auto key=r.address.load(std::memory_order_acquire);
            if(!key&&r.address.compare_exchange_strong(key,address,std::memory_order_acq_rel))key=address;
            if(key==address){row=&r;break;}}
        if(!row)return Outcome::capacity;
        if(row->busy.test_and_set(std::memory_order_acquire))return Outcome::overlap;
        float raw=*target;Outcome result=Outcome::ambiguous;
        if(!std::isfinite(raw)||raw<0)result=Outcome::identity;
        else if(row->written&&raw==row->last)result=Outcome::ambiguous;
        else if(raw>1.f/factor)result=Outcome::headroom; // Reject before any overflowing product.
        else {
            float next=raw*factor;
            // Reject insufficient headroom; do not clamp/limit/compress a signal.
            if(!std::isfinite(next)||next>1)result=Outcome::headroom;
            else if(raw==0)result=Outcome::neutral;
            else {*target=next;row->last=next;row->written=true;result=Outcome::applied;}
        }
        row->busy.clear(std::memory_order_release);return result;
    }
};
}
