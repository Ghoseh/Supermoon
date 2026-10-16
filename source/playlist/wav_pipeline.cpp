// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include "wav_pipeline.hpp"
#include "mp3_reader.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace bytebeat::playlist {
namespace {
void check(bool v,const char* why){if(!v)throw std::runtime_error(why);}
uint16_t u16(const unsigned char* p){return uint16_t(p[0])|uint16_t(p[1])<<8;}
uint32_t u32(const unsigned char* p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
void exact(std::istream& f,void* p,size_t n){f.read(static_cast<char*>(p),std::streamsize(n));check(size_t(f.gcount())==n,"Truncated WAV/cache read");}
constexpr double pi=3.14159265358979323846;
constexpr unsigned taps=64;
std::array<double,taps> kernel(double phase,double cutoff){
    std::array<double,taps> k{};double sum=0;
    for(unsigned i=0;i<taps;++i){double x=double(int(i)-31)-phase;
        double window=.42+.5*std::cos(pi*x/32)+.08*std::cos(2*pi*x/32);
        if(std::abs(x)>=32)window=0;
        k[i]=(std::abs(x)<1e-14?cutoff:std::sin(pi*cutoff*x)/(pi*x))*window;sum+=k[i];}
    for(auto& v:k)v/=sum;return k;
}
class MonoWindow {
    Wav& wav_;std::array<float,4096> ring_{};uint64_t decoded_{};
public:
    explicit MonoWindow(Wav& wav):wav_(wav){}
    float at(int64_t index){
        if(index<0||uint64_t(index)>=wav_.format().frames)return 0;
        while(uint64_t(index)>=decoded_){std::array<float,1024> b{};auto n=wav_.read_mono(b.data(),b.size());check(n>0,"Unexpected source EOF");for(size_t i=0;i<n;++i)ring_[size_t((decoded_+i)%ring_.size())]=b[i];decoded_+=n;}
        check(decoded_-uint64_t(index)<=ring_.size(),"Resampler window contract failed");return ring_[size_t(uint64_t(index)%ring_.size())];
    }
};
class Resampler {
    Wav& wav_;MonoWindow window_;uint64_t index_{};uint32_t denominator_,step_;
    std::vector<std::array<double,taps>> phases_;
public:
    explicit Resampler(Wav& w):wav_(w),window_(w){uint32_t gcd=std::gcd(w.format().rate,24000u);denominator_=24000/gcd;step_=w.format().rate/gcd;
        if(w.format().rate!=24000){phases_.reserve(denominator_);double cutoff=.94*std::min(1.,24000./w.format().rate);
            for(uint32_t p=0;p<denominator_;++p)phases_.push_back(kernel(double(p)/denominator_,cutoff));}}
    uint64_t frames() const{return (wav_.format().frames*24000+wav_.format().rate-1)/wav_.format().rate;}
    float next(){check(index_<frames(),"Resampler EOF misuse");if(wav_.format().rate==24000)return window_.at(int64_t(index_++));
        uint64_t numerator=index_*step_,center=numerator/denominator_;auto& k=phases_[size_t(numerator%denominator_)];double y=0;
        for(unsigned i=0;i<taps;++i)y+=k[i]*window_.at(int64_t(center)+int(i)-31);++index_;return float(y);}
};
double peak_file(const std::filesystem::path& p,uint64_t frames){
    std::ifstream f(p,std::ios::binary);check(bool(f),"Private cache unavailable");
    std::array<float,4096> ring{};uint64_t loaded=0;
    auto at=[&](int64_t i){if(i<0||uint64_t(i)>=frames)return 0.f;
        while(uint64_t(i)>=loaded){size_t n=size_t(std::min<uint64_t>(1024,frames-loaded));std::array<float,1024>b{};exact(f,b.data(),n*4);for(size_t j=0;j<n;++j)ring[size_t((loaded+j)%ring.size())]=b[j];loaded+=n;}return ring[size_t(uint64_t(i)%ring.size())];};
    std::array<std::array<double,taps>,3> phases{kernel(.25,1),kernel(.5,1),kernel(.75,1)};
    double peak=0;
    // Include filter support outside the file to account for endpoint ringing.
    for(int64_t n=-32;n<int64_t(frames)+32;++n){peak=std::max(peak,std::abs(double(at(n))));
        for(auto& k:phases){double y=0;for(unsigned t=0;t<taps;++t)y+=k[t]*at(n+int(t)-31);peak=std::max(peak,std::abs(y));}}
    return peak;
}
std::string hash_file(HANDLE f){
    BCRYPT_ALG_HANDLE a{};BCRYPT_HASH_HANDLE h{};uint8_t digest[32]{};std::array<uint8_t,65536>b{};DWORD n{};
    check(BCryptOpenAlgorithmProvider(&a,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0,"SHA provider failed");
    bool ok=BCryptCreateHash(a,&h,nullptr,0,nullptr,0,0)>=0;
    while(ok){if(!ReadFile(f,b.data(),DWORD(b.size()),&n,nullptr)){ok=false;break;}if(!n)break;ok=BCryptHashData(h,b.data(),n,0)>=0;}
    if(ok)ok=BCryptFinishHash(h,digest,32,0)>=0;if(h)BCryptDestroyHash(h);BCryptCloseAlgorithmProvider(a,0);check(ok,"Source hash failed");
    std::string s;for(auto v:digest){s.push_back("0123456789abcdef"[v>>4]);s.push_back("0123456789abcdef"[v&15]);}return s;
}
}
struct Wav::Pin {HANDLE handle;explicit Pin(const std::filesystem::path& p):handle(CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,nullptr)){check(handle!=INVALID_HANDLE_VALUE,"WAV unavailable or being modified");}~Pin(){CloseHandle(handle);}};
Wav::Wav(const std::filesystem::path& p){
    if(is_mp3(p)){auto m=std::make_unique<Mp3>(p);format_=m->format();mp3_=m.release();return;}
    auto owned=std::make_unique<Pin>(p);file_.open(p,std::ios::binary);check(bool(file_),"Cannot read WAV");
    file_.seekg(0,std::ios::end);auto length=file_.tellg();check(length>=12&&uint64_t(length)<=uint64_t(UINT32_MAX)+8,"Expected bounded RIFF WAV (RF64 unsupported)");file_.seekg(0);
    std::array<unsigned char,40>b{};exact(file_,b.data(),12);check(!memcmp(b.data(),"RIFF",4)&&!memcmp(b.data()+8,"WAVE",4),"Expected little-endian RIFF/WAVE");
    uint64_t end=8ull+u32(b.data()+4);check(end==uint64_t(length),"RIFF length mismatch");bool fmt=false,data=false;unsigned chunks=0;
    for(uint64_t pos=12;pos<end;){check(++chunks<=4096&&end-pos>=8,"Invalid WAV chunk count/header");file_.seekg(std::streamoff(pos));exact(file_,b.data(),8);uint64_t n=u32(b.data()+4),next=pos+8+n+(n&1);check(next<=end,"Chunk exceeds WAV bounds");
        if(!memcmp(b.data(),"fmt ",4)){check(!fmt&&n>=16,"Duplicate/short fmt");fmt=true;exact(file_,b.data(),size_t(std::min<uint64_t>(n,b.size())));
            auto& s=format_;s.tag=u16(b.data());s.channels=u16(b.data()+2);s.rate=u32(b.data()+4);s.alignment=u16(b.data()+12);s.bits=u16(b.data()+14);
            if(s.tag==0xfffe){check(n>=40&&u16(b.data()+16)>=22&&18ull+u16(b.data()+16)<=n&&u16(b.data()+18)==s.bits,"Unsupported extensible valid bits");
                const uint8_t tail[]={0,0,16,0,128,0,0,170,0,56,155,113};check(!memcmp(b.data()+28,tail,12)&&u16(b.data()+26)==0,"Unsupported extensible GUID");s.tag=u16(b.data()+24);
                uint32_t mask=u32(b.data()+20);check(mask==0||(s.channels==1&&mask==4)||(s.channels==2&&mask==3),"Unsupported WAV channel mask");}
            else check(n==16||(n>=18&&18ull+u16(b.data()+16)<=n),"Invalid fmt extension");
            check((s.channels==1||s.channels==2)&&s.rate>=8000&&s.rate<=384000,"Supported rates 8–384 kHz; mono/stereo only");
            check((s.tag==1&&(s.bits==8||s.bits==16||s.bits==24||s.bits==32))||(s.tag==3&&(s.bits==32||s.bits==64)),"Unsupported WAV encoding");
            check(s.alignment==s.channels*(s.bits/8)&&u32(b.data()+8)==uint64_t(s.rate)*s.alignment,"Invalid WAV block alignment/rate");
        }else if(!memcmp(b.data(),"data",4)){check(!data&&n>0,"Duplicate/empty data");data=true;format_.offset=pos+8;format_.bytes=n;}
        pos=next;
    }
    check(fmt&&data&&format_.bytes%format_.alignment==0,"Missing/unaligned WAV PCM");format_.frames=format_.bytes/format_.alignment;pin_=owned.release();restart();
}
Wav::~Wav(){delete mp3_;delete pin_;}
void Wav::restart(){if(mp3_){mp3_->restart();return;}file_.clear();file_.seekg(std::streamoff(format_.offset));check(bool(file_),"WAV seek failed");position_=0;}
size_t Wav::read_mono(float* out,size_t capacity){
    if(mp3_)return mp3_->read_mono(out,capacity);
    check(capacity<=4096&&(out||!capacity),"WAV decode buffer contract");size_t n=size_t(std::min<uint64_t>(capacity,format_.frames-position_));std::array<uint8_t,65536>b{};if(n)exact(file_,b.data(),n*format_.alignment);
    auto decode=[&](const uint8_t* p){double v=0;if(format_.tag==3){if(format_.bits==32){float x;memcpy(&x,p,4);v=x;}else memcpy(&v,p,8);}
        else if(format_.bits==8)v=(int(p[0])-128)/128.;else if(format_.bits==16)v=int16_t(u16(p))/32768.;
        else if(format_.bits==24){int32_t x=int32_t(uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16);if(x&0x800000)x|=int32_t(0xff000000);v=x/8388608.;}
        else v=int32_t(u32(p))/2147483648.;check(std::isfinite(v)&&std::abs(v)<=16,"Non-finite or extreme float WAV sample");return float(v);};
    for(size_t i=0;i<n;++i){auto p=b.data()+i*format_.alignment;float left=decode(p);out[i]=format_.channels==2?float(.5f*left+.5f*decode(p+format_.bits/8)):left;}position_+=n;return n;
}
Prepared prepare(const std::filesystem::path& source,const std::filesystem::path& output,const NormalizationSettings& settings){
    validate_normalization(settings);
    check(!std::filesystem::exists(output)&&source!=output,"Refusing existing cache output");auto raw=output;raw+=L".raw";auto temp=output;temp+=L".tmp";
    check(!std::filesystem::exists(raw)&&!std::filesystem::exists(temp),"Refusing cache temporary collision");
    struct Cleanup{std::filesystem::path a,b;~Cleanup(){std::error_code ec;std::filesystem::remove(a,ec);std::filesystem::remove(b,ec);}}cleanup{raw,temp};
    Wav wav(source); // Denies source writes/deletes through both passes.
    HANDLE hash=CreateFileW(source.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);check(hash!=INVALID_HANDLE_VALUE,"Hash source pin failed");
    std::string source_hash;try{source_hash=hash_file(hash);}catch(...){CloseHandle(hash);throw;}CloseHandle(hash);
    Resampler resampler(wav);uint64_t frames=resampler.frames();std::ofstream first(raw,std::ios::binary);check(bool(first),"Cannot create preparation cache");
    std::array<float,1024>b{};for(uint64_t pos=0;pos<frames;){size_t n=size_t(std::min<uint64_t>(b.size(),frames-pos));for(size_t i=0;i<n;++i)b[i]=resampler.next();first.write(reinterpret_cast<char*>(b.data()),std::streamsize(n*4));check(bool(first),"Cache disk write failed");pos+=n;}first.close();
    check(bool(first),"Cache flush failed");
    double peak=peak_file(raw,frames),target=std::pow(10.,-1.10/20.);double gain=peak>target?target/peak:1;
    NormalizationResult normalization;normalization.enabled=settings.enabled;
    if(settings.enabled){
        auto meter=measure_pcm(raw,frames);normalization.defined=meter.defined;normalization.input_lufs=meter.lufs;
        peak=std::max(peak,meter.true_peak);
        double desired=meter.defined?std::pow(10.,(settings.target_lufs-meter.lufs)/20.):1;
        // 0.5 dB conservative measurement/float-rounding margin, no dynamics processing.
        double ceiling=std::pow(10.,(settings.ceiling_dbtp-.5)/20.);
        gain=peak>0?std::min(desired,ceiling/peak):desired;
        normalization.limited=meter.defined&&gain<desired*(1-1e-9);
    }
    std::ifstream input(raw,std::ios::binary);std::ofstream final(temp,std::ios::binary);check(bool(input)&&bool(final),"Cannot finalize cache");
    for(uint64_t pos=0;pos<frames;){size_t n=size_t(std::min<uint64_t>(b.size(),frames-pos));exact(input,b.data(),n*4);for(size_t i=0;i<n;++i)b[i]=float(double(b[i])*gain);final.write(reinterpret_cast<char*>(b.data()),std::streamsize(n*4));check(bool(final),"Cache final write failed");pos+=n;}final.close();
    check(bool(final)&&std::filesystem::file_size(temp)==frames*4,"Incomplete finalized cache");
    Prepared result{output,frames,peak,gain,source_hash};
    if(settings.enabled){
        auto meter=measure_pcm(temp,frames);normalization.output_lufs=meter.lufs;normalization.true_peak=meter.true_peak;
        check(meter.true_peak<=std::pow(10.,settings.ceiling_dbtp/20.),"Normalized true peak exceeds configured ceiling");
    }
    std::filesystem::rename(temp,output);result.normalization=normalization;return result;
}
struct CacheReader::Pin {HANDLE handle;explicit Pin(const std::filesystem::path& p):handle(CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,nullptr)){check(handle!=INVALID_HANDLE_VALUE,"PCM cache unavailable or being modified");}~Pin(){CloseHandle(handle);}};
CacheReader::CacheReader(const Prepared& p):frames_(p.frames){
    auto owned=std::make_unique<Pin>(p.path);file_.open(p.path,std::ios::binary);
    check(bool(file_)&&frames_&&frames_<=uint64_t(INT64_MAX)/4&&std::filesystem::file_size(p.path)==frames_*4,"Invalid PCM cache size");pin_=owned.release();
}
CacheReader::~CacheReader(){delete pin_;}
size_t CacheReader::read(float* out,size_t count){check(out||!count,"Null PCM buffer");size_t n=size_t(std::min<uint64_t>(count,frames_-position_));if(n)exact(file_,out,n*4);for(size_t i=0;i<n;++i)check(std::isfinite(out[i])&&std::abs(out[i])<=1,"Corrupt PCM cache sample");position_+=n;return n;}
void CacheReader::seek(uint64_t frame){check(frame<=frames_&&frame<=uint64_t(INT64_MAX)/4,"Cache continuation bounds");file_.clear();file_.seekg(std::streamoff(frame*4));check(bool(file_),"Cache continuation seek failed");position_=frame;}
void CacheReader::restart(){file_.clear();file_.seekg(0);check(bool(file_),"Cache seek failed");position_=0;}
// Accepted audit C1: interrupted enumeration can end without a diagnostic
// in the pinned filesystem implementation. A partial path list is therefore not
// proof of complete discovery; prepare_cached_library may evict omitted caches.
// Source audio is never repaired/deleted here. The runtime defect is deferred.
Discovery discover(const std::filesystem::path& directory){Discovery result;std::error_code ec;
    if(!std::filesystem::is_directory(directory,ec)){result.diagnostics.push_back("Music folder missing/unreadable");return result;}
    std::filesystem::directory_iterator it(directory,ec),end;if(ec){result.diagnostics.push_back("Music folder enumeration failed");return result;}
    for(;it!=end;it.increment(ec)){if(ec){result.diagnostics.push_back("Music folder enumeration interrupted");break;}auto p=it->path();auto ext=p.extension().wstring();std::transform(ext.begin(),ext.end(),ext.begin(),[](wchar_t c){return std::towlower(c);});
        auto name=p.filename().wstring();std::transform(name.begin(),name.end(),name.begin(),[](wchar_t c){return std::towlower(c);});
        if(name==L"shuffle on.wav"||name==L"shuffle off.wav"){result.diagnostics.push_back("Developer shuffle notification excluded from playlist");continue;}
        if((ext!=L".wav"&&ext!=L".mp3")||!it->is_regular_file(ec))continue;if(result.paths.size()==10000){result.diagnostics.push_back("Music folder limit: 10000 files");break;}result.paths.push_back(p);}
    std::sort(result.paths.begin(),result.paths.end());return result;
}
}
