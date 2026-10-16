// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "playlist/notifications.hpp"
#include "playlist/normalization.hpp"
#include <cstdio>
#include <fstream>
#include <stdexcept>
using namespace bytebeat::playlist;
int main(){try{
    auto root=std::filesystem::path("fixtures/notification-peak");std::filesystem::create_directories(root);
    unsigned cases=0;double max_peak=0;
    for(double amplitude:{.10,.841395,.88105,.94406})for(bool on:{false,true}){
        Notifications mixer(true);mixer.publish(on,mixer.epoch());const auto& cue=on?on_cue:off_cue;
        auto path=root/(std::to_string(cases)+".pcm");std::ofstream out(path,std::ios::binary);
        for(unsigned i=0;i<cue.frames;++i){float song=float(amplitude*std::sin(i*2*3.141592653589793*997/24000));
            float mixed=song+mixer.sample(song,amplitude+.00001,1,1);mixer.consume();
            if(!std::isfinite(mixed)||std::abs(mixed)>=1)throw std::runtime_error("Sample clipping");out.write(reinterpret_cast<char*>(&mixed),4);}
        out.close();auto peak=measure_pcm(path,cue.frames).true_peak;max_peak=std::max(max_peak,peak);
        if(peak>=1)throw std::runtime_error("Measured true peak clipping");++cases;
        printf("mix amplitude=%.6f on=%d true_peak=%.9f\n",amplitude,int(on),peak);
    }
    for(unsigned mode=0;mode<2;++mode){
        Notifications mixer(true);auto path=root/("rapid"+std::to_string(mode)+".pcm");std::ofstream out(path,std::ios::binary);
        constexpr unsigned frames=96000;
        for(unsigned i=0;i<frames;++i){if(i%800==0)mixer.publish(bool((i/800)%2),mixer.epoch());if(mode&&i%3853==0)mixer.cancel();
            float song=float(.841395*std::sin(i*2*3.141592653589793*997/24000));
            float mixed=song+mixer.sample(song,.841405,1,1);mixer.consume();
            if(std::abs(mixed)>=1)throw std::runtime_error("Rapid toggle sample clipping");out.write(reinterpret_cast<char*>(&mixed),4);}
        out.close();auto peak=measure_pcm(path,frames).true_peak;max_peak=std::max(max_peak,peak);if(peak>=1)throw std::runtime_error("Rapid toggle true peak clipping");++cases;
        printf("rapid mode=%u true_peak=%.9f\n",mode,peak);
    }
    printf("PASS notification true_peak cases=%u maximum=%.9f unchanged_resident_meter\n",cases,max_peak);return 0;
}catch(const std::exception& e){fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
