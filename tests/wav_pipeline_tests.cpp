// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "playlist/wav_pipeline.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace bytebeat::playlist;
namespace {
void check(bool v,const char* m){if(!v)throw std::runtime_error(m);}
void p16(std::vector<uint8_t>& b,uint16_t v){b.push_back(uint8_t(v));b.push_back(uint8_t(v>>8));}
void p32(std::vector<uint8_t>& b,uint32_t v){for(unsigned i=0;i<4;++i)b.push_back(uint8_t(v>>(8*i)));}
void word(std::vector<uint8_t>& b,const char* s){b.insert(b.end(),s,s+4);}
std::vector<uint8_t> wave(uint16_t tag,uint16_t bits,uint16_t channels,uint32_t rate,const std::vector<uint8_t>& data,bool ext=false){
    std::vector<uint8_t>b;word(b,"RIFF");p32(b,0);word(b,"WAVE");word(b,"fmt ");p32(b,ext?40:16);p16(b,ext?0xfffe:tag);p16(b,channels);p32(b,rate);p32(b,rate*channels*(bits/8));p16(b,channels*(bits/8));p16(b,bits);
    if(ext){p16(b,22);p16(b,bits);p32(b,channels==1?4:3);p32(b,tag);p16(b,0);p16(b,16);uint8_t tail[]={128,0,0,170,0,56,155,113};b.insert(b.end(),tail,tail+8);}
    word(b,"data");p32(b,uint32_t(data.size()));b.insert(b.end(),data.begin(),data.end());if(data.size()&1)b.push_back(0);uint32_t n=uint32_t(b.size()-8);memcpy(b.data()+4,&n,4);return b;
}
void write(const std::filesystem::path& p,const std::vector<uint8_t>& b){std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));check(bool(f),"Fixture write failed");}
std::vector<uint8_t> floats(const std::vector<float>& samples){std::vector<uint8_t>b(samples.size()*4);memcpy(b.data(),samples.data(),b.size());return b;}
void reject(const std::filesystem::path& p){bool bad=false;try{Wav w(p);std::array<float,4096>b{};while(w.read_mono(b.data(),b.size())){}}catch(...){bad=true;}check(bad,"Malformed WAV accepted");}
}
int main(){try{
    auto root=std::filesystem::path("fixtures/legacy/wav-fixtures");std::filesystem::create_directories(root);
    // Fixture outputs use a distinct new run directory; no existing output overwrite.
    auto run=root/std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());std::filesystem::create_directory(run);
    for(uint16_t bits:{8,16,24,32}){std::vector<uint8_t> data;
        for(unsigned n=0;n<65;++n){if(bits==8)data.push_back(192);else{uint32_t v=bits==16?16384:bits==24?4194304:1073741824;for(unsigned i=0;i<bits/8;++i)data.push_back(uint8_t(v>>(8*i)));}}
        auto p=run/(std::to_string(bits)+".wav");write(p,wave(1,bits,1,24000,data));Wav wav(p);std::array<float,4096>b{};check(wav.read_mono(b.data(),b.size())==65,"PCM count failed");for(unsigned n=0;n<65;++n)check(b[n]==.5f,"PCM decoding failed");}
    std::vector<uint8_t> stereo;for(unsigned n=0;n<65;++n){p16(stereo,32767);p16(stereo,32768);}auto s=run/"stereo.wav";write(s,wave(1,16,2,48000,stereo));Wav w(s);std::array<float,4096>b{};w.read_mono(b.data(),b.size());check(b[0]==-1.f/65536,"Float-before-average contract failed");
    for(bool ext:{false,true}){auto p=run/(ext?"ext.wav":"float.wav");write(p,wave(3,32,1,24000,floats(std::vector<float>(73,.4f)),ext));auto prepared=prepare(p,run/(ext?"ext.pcm":"float.pcm"));CacheReader r(prepared);check(prepared.frames==73&&prepared.gain==1&&r.read(b.data(),b.size())==73,"Direct 24k cache failed");for(unsigned n=0;n<73;++n)check(b[n]==.4f,"Direct 24k float changed");}
    std::vector<uint8_t> d64(11*8);double sample=.25;for(unsigned n=0;n<11;++n)memcpy(d64.data()+n*8,&sample,8);auto f64=run/"64.wav";write(f64,wave(3,64,1,24000,d64));Wav f(f64);check(f.read_mono(b.data(),b.size())==11&&b[0]==.25f,"float64 failed");
    auto bad=wave(1,16,1,24000,std::vector<uint8_t>(2));bad[4]++;write(run/"bad.wav",bad);reject(run/"bad.wav");
    write(run/"empty.wav",wave(1,16,1,24000,{}));reject(run/"empty.wav");write(run/"compressed.wav",wave(2,16,1,24000,{0,0}));reject(run/"compressed.wav");
    write(run/"nan.wav",wave(3,32,1,24000,floats({NAN})));reject(run/"nan.wav");write(run/"inf.wav",wave(3,32,1,24000,floats({INFINITY})));reject(run/"inf.wav");reject(run/"missing.wav");
    // Test actual compiled FIR rejection, not a waveform-free resampling model.
    double alias_rms=0,pass_rms=0;
    for(unsigned hz:{1000,18000}){std::vector<float> tone(48000);for(size_t n=0;n<tone.size();++n)tone[n]=float(.5*std::sin(2*3.14159265358979323846*hz*n/48000));auto p=run/(std::to_string(hz)+".wav");write(p,wave(3,32,1,48000,floats(tone)));auto prepared=prepare(p,run/(std::to_string(hz)+".pcm"));check(prepared.frames==24000,"Resampling output count wrong");CacheReader cache(prepared);uint64_t pos=0;double energy=0;uint64_t count=0;while(auto n=cache.read(b.data(),b.size())){for(size_t i=0;i<n;++i)if(pos+i>100&&pos+i<23900){energy+=double(b[i])*b[i];++count;}pos+=n;}double rms=std::sqrt(energy/count);if(hz==18000)alias_rms=rms;else pass_rms=rms;}
    check(pass_rms>.35&&pass_rms<.36&&alias_rms<.0001,"FIR passband/anti-alias regression");
    auto loud=run/"loud.wav";write(loud,wave(3,32,1,24000,floats({1.2f,-1.2f,1.2f,-1.2f})));auto prepared=prepare(loud,run/"loud.pcm");check(prepared.gain<1&&prepared.gain>0,"Negative-only headroom missing");CacheReader lc(prepared);lc.read(b.data(),4);for(unsigned n=0;n<4;++n)check(std::abs(b[n])<.9f,"Prepared sample clamp risk");bool refused=false;try{prepare(loud,run/"loud.pcm");}catch(...){refused=true;}check(refused,"Cache overwrite accepted");
    check(discover(run/"missing").paths.empty()&&!discover(run/"missing").diagnostics.empty(),"Missing Music diagnostics failed");check(discover(run).paths.size()>=15,"WAV discovery failed");
    std::cout<<"PASS internal WAV pipeline PCM8/16/24/32 float32/64 extensible mono/stereo, strict RIFF, exact counts, direct24k, float average, negative-only reconstructed-peak headroom, source pins, cache refusal; passband_RMS="<<pass_rms<<" alias_RMS="<<alias_rms<<" bounded disk-backed preparation\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
