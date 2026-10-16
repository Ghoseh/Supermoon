// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#define DR_MP3_ONLY_MP3
#define DR_MP3_FLOAT_OUTPUT
#include "vendor/dr_mp3/dr_mp3.h"
#include "mp3_reader.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <memory>
#include <stdexcept>
namespace bytebeat::playlist {
namespace {
std::atomic<uint64_t> calls{};
void need(bool value,const char* message){if(!value)throw std::runtime_error(message);}
uint32_t be32(const unsigned char* b){return uint32_t(b[0])<<24|uint32_t(b[1])<<16|uint32_t(b[2])<<8|b[3];}
uint32_t le32(const unsigned char* b){return uint32_t(b[0])|uint32_t(b[1])<<8|uint32_t(b[2])<<16|uint32_t(b[3])<<24;}
struct Header {uint32_t rate,channels,bytes,samples,version;};
Header header(const unsigned char* h){
    need(h[0]==255&&(h[1]&224)==224,"MP3 frame sync missing/corrupted");
    auto version=(h[1]>>3)&3,layer=(h[1]>>1)&3,bitrate=h[2]>>4,rate=(h[2]>>2)&3;
    need(version!=1&&layer==1&&bitrate&&bitrate!=15&&rate!=3,"Unsupported MP3 header (Layer III/non-free-format required)");
    static constexpr unsigned rates[]={44100,48000,32000};
    static constexpr unsigned br1[]={0,32,40,48,56,64,80,96,112,128,160,192,224,256,320};
    static constexpr unsigned br2[]={0,8,16,24,32,40,48,56,64,80,96,112,128,144,160};
    auto hz=rates[rate]/(version==3?1:version==2?2:4);
    auto bytes=(version==3?144000:72000)*(version==3?br1[bitrate]:br2[bitrate])/hz+((h[2]>>1)&1);
    need(bytes>=24&&bytes<=2048,"MP3 frame byte bounds");
    return {hz,(h[3]>>6)==3?1u:2u,bytes,version==3?1152u:576u,uint32_t(version)};
}
}
bool is_mp3(const std::filesystem::path& p){auto extension=p.extension().wstring();std::transform(extension.begin(),extension.end(),extension.begin(),[](wchar_t c){return std::towlower(c);});return extension==L".mp3";}
uint64_t mp3_decode_calls() noexcept{return calls.load(std::memory_order_relaxed);}
struct Mp3::State {
    HANDLE pin=INVALID_HANDLE_VALUE;std::ifstream file;drmp3dec decoder{};
    std::array<unsigned char,2048> encoded{};std::array<float,2304> decoded{};
    uint64_t start{},end{},next{},position{},length{},delay{},padding{},remaining_delay{};
    Header type{};uint32_t available{},offset{};bool gapless{};
    ~State(){if(pin!=INVALID_HANDLE_VALUE)CloseHandle(pin);}
    void read(uint64_t at,void* destination,size_t n){
        need(at<=end&&n<=end-at,"MP3 bounded input read");file.clear();file.seekg(std::streamoff(at));need(bool(file),"MP3 source seek failed");
        file.read(static_cast<char*>(destination),std::streamsize(n));need(size_t(file.gcount())==n,"Truncated MP3 source");
    }
    void decode(){
        need(next<end&&end-next>=4,"Unexpected MP3 audio EOF");read(next,encoded.data(),4);auto h=header(encoded.data());
        need(h.rate==type.rate&&h.channels==type.channels&&h.version==type.version&&h.bytes<=end-next,"MP3 format changed/truncated frame");
        read(next,encoded.data(),h.bytes);next+=h.bytes;
        // Scratch is temporary, not overlap/reservoir state. Do not allow stale
        // uninitialized scratch to influence malformed-frame decoding.
        std::memset(&decoder.scratch,0,sizeof(decoder.scratch));decoded.fill(0);
        // The vendored patch propagates sticky mandatory-granule/bounds errors as
        // zero samples and clears failed-frame PCM. Demand the complete count and
        // format before publishing available samples; successful reservoir/QMF
        // state is retained across frames. CRC authenticity is not established.
        drmp3dec_frame_info info{};calls.fetch_add(1,std::memory_order_relaxed);
        int n=drmp3dec_decode_frame(&decoder,encoded.data(),int(h.bytes),decoded.data(),&info);
        need(n==int(h.samples)&&info.frame_bytes==int(h.bytes)&&info.channels==int(h.channels)&&info.sample_rate==int(h.rate)&&info.layer==3,"Malformed MP3 compressed frame");
        for(unsigned i=0;i<unsigned(n)*h.channels;++i)need(std::isfinite(decoded[i])&&std::abs(decoded[i])<=16,"Non-finite/extreme MP3 PCM");
        available=unsigned(n);offset=0;
    }
};
Mp3::Mp3(const std::filesystem::path& path){
    auto s=std::make_unique<State>();s->pin=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,nullptr);
    need(s->pin!=INVALID_HANDLE_VALUE,"MP3 source unavailable or being modified");s->file.open(path,std::ios::binary);need(bool(s->file),"MP3 source open failed");
    auto bytes=std::filesystem::file_size(path);need(bytes>=4&&bytes<=512ull*1024*1024,"MP3 file size outside supported 4B..512MiB");s->end=bytes;
    std::array<unsigned char,2048> data{};s->read(0,data.data(),size_t(std::min<uint64_t>(10,bytes)));
    if(bytes>=10&&!memcmp(data.data(),"ID3",3)){
        unsigned version=data[3];need(version>=2&&version<=4&&data[4]!=255,"Unsupported ID3 version");
        need(!(data[6]&128)&&!(data[7]&128)&&!(data[8]&128)&&!(data[9]&128),"Invalid ID3 synchsafe length");
        uint64_t size=uint64_t(data[6])<<21|uint64_t(data[7])<<14|uint64_t(data[8])<<7|data[9];
        need(size<=16*1024*1024&&10+size<=bytes,"ID3 tag oversized/truncated");s->start=10+size;
        if(version==4&&(data[5]&16)){need(s->start+10<=bytes,"Truncated ID3 footer");s->read(s->start,data.data(),10);need(!memcmp(data.data(),"3DI",3),"Invalid ID3 footer");s->start+=10;}
    }
    if(s->end>=128){s->read(s->end-128,data.data(),3);if(!memcmp(data.data(),"TAG",3))s->end-=128;}
    if(s->end>=32){s->read(s->end-32,data.data(),32);if(!memcmp(data.data(),"APETAGEX",8)){
        auto size=le32(data.data()+12),flags=le32(data.data()+20);need((le32(data.data()+8)==1000||le32(data.data()+8)==2000)&&size>=32&&size<=16*1024*1024&&size<=s->end-s->start,"Invalid APE footer");
        s->end-=size;if(flags&0x80000000u){need(s->end>=32,"Truncated APE header");s->read(s->end-32,data.data(),8);need(!memcmp(data.data(),"APETAGEX",8),"Invalid APE header");s->end-=32;}
    }}
    need(s->start<s->end&&s->end-s->start>=4,"MP3 has no audio");
    uint64_t at=s->start,frames=0,metadata_frames=0;uint32_t declared=0;uint64_t encoded_delay=0,encoded_padding=0;
    while(at<s->end){need(++frames<=1000000&&s->end-at>=4,"MP3 frame count/truncated tail");s->read(at,data.data(),4);auto h=header(data.data());
        if(frames==1)s->type=h;else need(h.rate==s->type.rate&&h.channels==s->type.channels&&h.version==s->type.version,"Changing MP3 rate/channel/version unsupported");
        need(h.bytes<=s->end-at,"Truncated final MP3 frame");
        if(frames==1){s->read(at,data.data(),h.bytes);size_t xing=4+((data[1]&1)?0:2)+(h.version==3?(h.channels==1?17:32):(h.channels==1?9:17));
            if(xing+8<=h.bytes&&(!memcmp(data.data()+xing,"Xing",4)||!memcmp(data.data()+xing,"Info",4))){
                metadata_frames=1;auto flags=be32(data.data()+xing+4);need((flags&~15u)==0,"Unsupported Xing flags");size_t p=xing+8;
                auto field=[&](size_t n){need(p+n<=h.bytes,"Truncated Xing metadata");auto old=p;p+=n;return old;};
                if(flags&1)declared=be32(data.data()+field(4));if(flags&2){auto count=be32(data.data()+field(4));need(!count||count==s->end-s->start,"Xing byte count mismatches source");}
                if(flags&4)field(100);if(flags&8)field(4);
                if(p+36<=h.bytes&&(!memcmp(data.data()+p,"LAME",4)||!memcmp(data.data()+p,"Lavc",4)||!memcmp(data.data()+p,"Lavf",4))){
                    encoded_delay=uint64_t(data[p+21])<<4|data[p+22]>>4;encoded_padding=uint64_t(data[p+22]&15)<<8|data[p+23];
                    s->delay=encoded_delay+529;s->padding=encoded_padding>=529?encoded_padding-529:0;s->gapless=true;
                }
                s->start=at+h.bytes;
            }
        }
        at+=h.bytes;
    }
    need(frames>metadata_frames,"MP3 contains metadata only");auto audio_frames=frames-metadata_frames;
    need(!declared||declared==audio_frames,"Xing frame count mismatches physical MP3 frames");
    auto decoded_frames=audio_frames*s->type.samples;
    need(!s->gapless||(encoded_padding>=529&&s->delay+s->padding<decoded_frames),"Invalid gapless delay/padding");
    s->length=decoded_frames-s->delay-s->padding;need(s->length&&s->length<=uint64_t(INT64_MAX)/8,"Invalid MP3 PCM duration");
    format_={s->type.rate,uint16_t(s->type.channels),32,3,uint16_t(4*s->type.channels),0,s->length*4*s->type.channels,s->length};
    state_=s.release();restart();
}
Mp3::~Mp3(){delete state_;}
void Mp3::restart(){auto& s=*state_;s.decoder={};drmp3dec_init(&s.decoder);s.next=s.start;s.position=0;s.remaining_delay=s.delay;s.available=s.offset=0;}
bool Mp3::gapless()const noexcept{return state_->gapless;}
size_t Mp3::read_mono(float* out,size_t capacity){
    need(capacity<=4096&&(out||!capacity),"MP3 mono buffer contract");auto& s=*state_;size_t n=size_t(std::min<uint64_t>(capacity,s.length-s.position));
    for(size_t i=0;i<n;++i){
        while(s.offset==s.available||s.remaining_delay){if(s.offset==s.available)s.decode();auto skip=uint32_t(std::min<uint64_t>(s.remaining_delay,s.available-s.offset));s.offset+=skip;s.remaining_delay-=skip;}
        auto at=s.offset++*s.type.channels;float left=s.decoded[at];out[i]=s.type.channels==2?float(.5f*left+.5f*s.decoded[at+1]):left;
    }
    s.position+=n;return n;
}
}
