// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "wav_reader.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bytebeat {
static_assert(sizeof(float)==4 && std::numeric_limits<float>::is_iec559);
static constexpr uint64_t limit = 128ull*1024*1024;
static void require(bool ok, const char* reason) { if(!ok) throw std::runtime_error(reason); }
static uint16_t u16(const unsigned char* p) { return uint16_t(p[0]) | uint16_t(p[1])<<8; }
static uint32_t u32(const unsigned char* p) { return uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24; }
static void exact(std::ifstream& f, void* p, size_t n) {
    f.read(static_cast<char*>(p),static_cast<std::streamsize>(n));
    require(size_t(f.gcount())==n,"Truncated WAV/read failure");
}
WavInfo inspect_wav(const std::filesystem::path& path) {
    std::ifstream f(path,std::ios::binary);
    require(bool(f),"Cannot open WAV");
    f.seekg(0,std::ios::end); auto end=f.tellg();
    require(end>=12 && uint64_t(end)<=limit,"Invalid/oversized WAV (128 MiB limit)");
    f.seekg(0); std::array<unsigned char,40> b{}; exact(f,b.data(),12);
    require(!memcmp(b.data(),"RIFF",4) && !memcmp(b.data()+8,"WAVE",4),"Expected little-endian RIFF/WAVE");
    uint64_t riff_end=8ull+u32(b.data()+4);
    require(riff_end==uint64_t(end),"RIFF length mismatch/trailing data");
    bool fmt=false,data=false; WavInfo out{}; unsigned chunks=0;
    for(uint64_t pos=12;pos<riff_end;) {
        require(++chunks<=4096 && riff_end-pos>=8,"Invalid chunk header/count");
        f.seekg(static_cast<std::streamoff>(pos)); exact(f,b.data(),8);
        uint64_t n=u32(b.data()+4), next=pos+8+n+(n&1);
        require(next<=riff_end,"Chunk exceeds RIFF bounds/missing padding");
        if(!memcmp(b.data(),"fmt ",4)) {
            require(!fmt && n>=16,"Duplicate/short fmt chunk"); fmt=true;
            exact(f,b.data(),size_t(std::min<uint64_t>(n,b.size())));
            uint16_t tag=u16(b.data());
            require(u16(b.data()+2)==1 && u32(b.data()+4)==24000 && u32(b.data()+8)==96000 && u16(b.data()+12)==4 && u16(b.data()+14)==32,"Expected 24000 Hz mono float32 (align 4/rate 96000)");
            if(tag==0xfffe) {
                const unsigned char guid[16]={3,0,0,0,0,0,16,0,128,0,0,170,0,56,155,113};
                require(n>=40 && u16(b.data()+16)>=22 && 18ull+u16(b.data()+16)<=n && u16(b.data()+18)==32 && (u32(b.data()+20)==0 || u32(b.data()+20)==4) && !memcmp(b.data()+24,guid,16),"Invalid extensible float format");
            } else require(tag==3 && (n==16 || (n>=18 && 18ull+u16(b.data()+16)<=n)),"Expected IEEE float format");
        } else if(!memcmp(b.data(),"data",4)) {
            require(!data && n>0 && n%4==0,"Duplicate/empty/unaligned data chunk");
            data=true; out={pos+8,n,n/4};
        }
        pos=next;
    }
    require(fmt && data,"Missing fmt/data"); return out;
}
ChunkReader::ChunkReader(const std::filesystem::path& path):file_(path,std::ios::binary),info_(inspect_wav(path)) { restart(); }
void ChunkReader::restart() { file_.clear(); file_.seekg(static_cast<std::streamoff>(info_.offset)); require(bool(file_),"WAV seek failed"); position_=0; }
size_t ChunkReader::read(float* output,size_t capacity) {
    size_t n=size_t(std::min<uint64_t>(capacity,info_.frames-position_));
    if(n) { require(output!=nullptr,"Null read buffer"); exact(file_,output,n*4); }
    for(size_t i=0;i<n;++i) require(std::isfinite(output[i]),"Non-finite WAV sample");
    position_+=n; return n;
}
std::vector<float> preload_wav(const std::filesystem::path& path) {
    ChunkReader r(path); std::vector<float> samples(size_t(r.info().frames));
    require(r.read(samples.data(),samples.size())==samples.size(),"Incomplete preload"); return samples;
}
}
