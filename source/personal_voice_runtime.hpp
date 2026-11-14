// SPDX-License-Identifier: MIT
#pragma once
#include "personal_voice_gain.hpp"
#include "personal_voice_gain_profile.hpp"
#include "recovery_native.hpp"
#include <filesystem>
#include <stdexcept>
#include <cstdio>
namespace bytebeat::personal_gain {
template<class T>inline T load(const void* p,size_t n) noexcept {T v;memcpy(&v,static_cast<const unsigned char*>(p)+n,sizeof(v));return v;}
struct Runtime {
    struct Cookie {recovery::Event* event{};uint64_t object{};std::atomic<uint32_t> id{0};std::atomic<recovery::Source*> source{nullptr};std::atomic<uintptr_t> connection{0};};
    Setting setting;uintptr_t base{};Targets targets;std::atomic<bool> activation_ready{false};
    Cookie cookies[recovery::event_capacity];std::atomic<uint32_t> count{0};
    std::atomic<uint64_t> registered{0},post_failed{0},authorized{0},inactive{0};
    std::atomic<uint64_t> outcomes[7]{};
    void configure(const std::filesystem::path& mod) {
        wchar_t value[128]{};auto ini=mod/L"playlist.ini";
        auto attrs=GetFileAttributesW(ini.c_str());
        if(attrs==INVALID_FILE_ATTRIBUTES){
            auto error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND){setting={0,1,false};return;}
        }else{
            if(attrs&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY)){setting={0,1,false};return;}
            auto probe=CreateFileW(ini.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
            if(probe==INVALID_HANDLE_VALUE){setting={0,1,false};return;}CloseHandle(probe);
        }
        auto n=GetPrivateProfileStringW(L"Audio",L"VoiceGainDb",L"3.0",value,128,ini.c_str());
        setting=n&&n<127?parse(value):Setting{0,1,false};
    }
    Cookie* cookie(void* ptr) noexcept {
        auto a=reinterpret_cast<uintptr_t>(ptr),start=reinterpret_cast<uintptr_t>(cookies);
        if(a<start||a-start>=sizeof(cookies)||(a-start)%sizeof(Cookie))return nullptr;
        auto n=(a-start)/sizeof(Cookie);return n<count.load(std::memory_order_acquire)?&cookies[n]:nullptr;
    }
    Cookie* reserve(recovery::Event* e,uint64_t object) noexcept {
        auto n=count.load(std::memory_order_acquire);
        for(unsigned attempt=0;attempt<8;++attempt){if(n>=recovery::event_capacity)return nullptr;
            if(count.compare_exchange_weak(n,n+1,std::memory_order_acq_rel)){auto& c=cookies[n];c.event=e;c.object=object;return &c;}}
        return nullptr;
    }
    void authorize(recovery::Source* s) noexcept {
        if(!setting.db||!recovery::Registry::live(s))return;
        auto e=s->event.load(std::memory_order_acquire);auto end=count.load(std::memory_order_acquire);
        for(uint32_t n=0;n<end;++n){auto& c=cookies[n];auto id=c.id.load(std::memory_order_acquire);
            if(id&&c.event==e&&id==s->id.load(std::memory_order_acquire)){
                recovery::Source* empty=nullptr;if(c.source.compare_exchange_strong(empty,s,std::memory_order_acq_rel))authorized.fetch_add(1,std::memory_order_relaxed);
            }}
    }
    void activate() noexcept {activation_ready.store(setting.valid&&setting.db>0,std::memory_order_release);}
    void callback(uint32_t type,Info* p) noexcept {
        if(!setting.db||!activation_ready.load(std::memory_order_acquire)||type!=0x10||!p)return;
        auto c=cookie(p->cookie);if(!c)return;
        auto id=c->id.load(std::memory_order_acquire);auto s=c->source.load(std::memory_order_acquire);
        if(!id||!s||!recovery::Registry::live(s)||s->event.load(std::memory_order_acquire)!=c->event){inactive.fetch_add(1,std::memory_order_relaxed);return;}
        auto reject=[&](){outcomes[unsigned(Outcome::identity)].fetch_add(1,std::memory_order_relaxed);};
        if(p->playing!=id||p->event!=personal_event||p->object!=c->object||s->id.load()!=id||!p->context||!p->mixer){reject();return;}
        if(load<uintptr_t>(p->context,0)!=base+0x365c368||load<uintptr_t>(p->mixer,0)!=base+0x3662748){reject();return;}
        auto conn=load<unsigned char*>(p->context,8);auto pbi=load<void*>(p->context,16);
        if(!conn||!pbi||p->base!=reinterpret_cast<float*>(conn+4)||p->emitter!=reinterpret_cast<float*>(conn+12)||load<uint32_t>(conn,0x1a4)!=0){reject();return;}
        auto voice=load<void*>(conn,0x138);auto mix=load<unsigned char*>(conn,0x140);
        if(!voice||!mix||load<void*>(voice,8)!=pbi||mix+0xa0!=p->mixer){reject();return;}
        using ID=uint32_t(*)(void*);
        if(reinterpret_cast<ID>(base+0x3221e90)(pbi)!=sound_id||reinterpret_cast<ID>(base+0x32cefb0)(p->mixer)!=bus_id){reject();return;}
        auto address=reinterpret_cast<uintptr_t>(conn),first=c->connection.load(std::memory_order_acquire);
        if(!first){c->connection.compare_exchange_strong(first,address,std::memory_order_acq_rel);first=c->connection.load(std::memory_order_acquire);}
        if(first!=address){reject();return;}
        auto result=targets.apply(address,p->base,setting.factor);outcomes[unsigned(result)].fetch_add(1,std::memory_order_relaxed);
    }
    bool diagnostics(char* buffer,size_t size,uint64_t now) noexcept {
        static uint64_t last=0;if(last&&now-last<1000)return false;last=now?now:1;
        std::snprintf(buffer,size,"# personal_voice_gain experimental_schema=1 db=%.3f valid=%u ready=%u registered=%llu post_failed=%llu authorized=%llu inactive=%llu applied=%llu rejected_identity=%llu ambiguous_target=%llu insufficient_headroom=%llu overlap=%llu capacity=%llu; native_personal_inheritance_accepted; no_forced_ramp_reset\n",setting.db,unsigned(setting.valid),unsigned(activation_ready.load()),registered.load(),post_failed.load(),authorized.load(),inactive.load(),outcomes[1].load(),outcomes[2].load(),outcomes[3].load(),outcomes[4].load(),outcomes[5].load(),outcomes[6].load());return true;
    }
};
inline Runtime runtime;
extern "C" inline void PersonalVoiceGainCallback(uint32_t type,Info* p) noexcept {runtime.callback(type,p);}
using Post=uint32_t(*)(uint32_t,uint64_t,uint32_t,void(*)(uint32_t,Info*),void*,uint32_t,void*,uint32_t);
extern "C" inline uint32_t PersonalVoiceGainPost(uint32_t event,uint64_t object,uint32_t flags,void(*cb)(uint32_t,Info*),void* cookie,uint32_t count,void* external,uint32_t playing) {
    auto& r=runtime;auto original=reinterpret_cast<Post>(r.base+0x3215650);
    auto e=recovery::constructing;
    if(r.setting.db&&event==personal_event&&!flags&&!cb&&!cookie&&!count&&!external&&!playing&&e){
        if(auto c=r.reserve(e,object)){
            auto id=original(event,object,0x10,&PersonalVoiceGainCallback,c,0,nullptr,0);
            c->id.store(id,std::memory_order_release);if(id)r.registered.fetch_add(1,std::memory_order_relaxed);else r.post_failed.fetch_add(1,std::memory_order_relaxed);return id;
        }
    }
    return original(event,object,flags,cb,cookie,count,external,playing);
}
}
