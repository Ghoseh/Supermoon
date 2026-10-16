// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "normalization.hpp"
#include "ebur128.h"
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <locale>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <cwchar>
namespace bytebeat::playlist {
namespace {
void need(bool b,const char* message){if(!b)throw std::runtime_error(message);}
// Production links the separately pinned ebur128_peak.c, not the unchanged vendor
// reference C file. Its low-rate peak filter and allocation cleanup are qualified
// inputs. Retain the mutex: vendor initialization rewrites static histogram tables.
std::mutex meter_mutex;
struct Meter {ebur128_state* state{};~Meter(){if(state)ebur128_destroy(&state);}};
}
void validate_normalization(const NormalizationSettings& s){
    need(std::isfinite(s.target_lufs)&&s.target_lufs>=-60&&s.target_lufs<=0,"TargetLUFS must be finite -60..0");
    need(std::isfinite(s.ceiling_dbtp)&&s.ceiling_dbtp>=-20&&s.ceiling_dbtp<=0,"TruePeakDBTP must be finite -20..0");
}
std::string normalization_identity(const NormalizationSettings& s){
    validate_normalization(s);std::ostringstream out;out.imbue(std::locale::classic());
    out<<"LUFS1/libebur128-67b33abe/histogram/24kmono/8x257TPpatch1/reflected-and-zero-edges1/margin0.5/"<<s.enabled<<'/'<<std::setprecision(17)<<s.target_lufs<<'/'<<s.ceiling_dbtp;return out.str();
}
NormalizationSettings normalization_configuration(const std::filesystem::path& mod){
    NormalizationSettings s;auto path=mod/"playlist.ini";wchar_t value[128]{};
    GetPrivateProfileStringW(L"Normalization",L"Enabled",L"1",value,128,path.c_str());
    need(!wcscmp(value,L"0")||!wcscmp(value,L"1"),"Normalization Enabled must be 0 or 1");s.enabled=value[0]==L'1';
    auto number=[&](const wchar_t* key,const wchar_t* fallback){auto n=GetPrivateProfileStringW(L"Normalization",key,fallback,value,128,path.c_str());need(n&&n<127,"Invalid normalization setting length");wchar_t* end{};double v=wcstod(value,&end);need(end!=value&&!*end,"Invalid normalization number");return v;};
    s.target_lufs=number(L"TargetLUFS",L"-16");s.ceiling_dbtp=number(L"TruePeakDBTP",L"-1");validate_normalization(s);return s;
}
LoudnessMeasurement measure_pcm(const std::filesystem::path& path,uint64_t frames){
    std::lock_guard<std::mutex> lock(meter_mutex); // Off audio, serializes all vendor access.
    need(frames&&frames<=uint64_t(INT64_MAX)/4&&std::filesystem::file_size(path)==frames*4,"Invalid measurement PCM size");
    std::ifstream input(path,std::ios::binary);need(bool(input),"Measurement PCM unavailable");
    Meter m;m.state=ebur128_init(1,24000,EBUR128_MODE_I|EBUR128_MODE_TRUE_PEAK|EBUR128_MODE_HISTOGRAM);need(m.state,"Loudness meter allocation failed");
    need(ebur128_set_channel(m.state,0,EBUR128_CENTER)==EBUR128_SUCCESS,"Mono channel map failed");
    // Boundary reconstruction differs between resamplers. Protect both zero
    // extension and reflection without changing PCM or loudness integration.
    Meter edges;edges.state=ebur128_init(1,24000,EBUR128_MODE_TRUE_PEAK);need(edges.state,"Boundary peak meter allocation failed");
    std::array<float,64> first{},last{},extension{};size_t edge_count=size_t(std::min<uint64_t>(64,frames));
    input.read(reinterpret_cast<char*>(first.data()),std::streamsize(edge_count*4));need(size_t(input.gcount())==edge_count*4,"Truncated peak boundary");input.clear();input.seekg(0);
    size_t reflect_count=edge_count-1;
    for(size_t i=0;i<reflect_count;++i)extension[i]=first[reflect_count-i];
    need(ebur128_add_frames_float(edges.state,extension.data(),reflect_count)==EBUR128_SUCCESS,"Initial boundary peak failed");
    std::array<float,4096> buffer{};
    for(uint64_t at=0;at<frames;){size_t n=size_t(std::min<uint64_t>(buffer.size(),frames-at));input.read(reinterpret_cast<char*>(buffer.data()),std::streamsize(n*4));need(size_t(input.gcount())==n*4,"Truncated measurement PCM");
        for(size_t i=0;i<n;++i)need(std::isfinite(buffer[i]),"Non-finite measurement PCM");
        need(ebur128_add_frames_float(m.state,buffer.data(),n)==EBUR128_SUCCESS,"Loudness measurement failed");
        need(ebur128_add_frames_float(edges.state,buffer.data(),n)==EBUR128_SUCCESS,"Boundary peak measurement failed");
        for(size_t i=0;i<n;++i)last[size_t((at+i)%last.size())]=buffer[i];at+=n;}
    double loudness{};need(ebur128_loudness_global(m.state,&loudness)==EBUR128_SUCCESS,"Integrated loudness failed");
    // Query loudness before the short zero tail; flush interpolation endpoint only.
    std::array<float,256> zero{};need(ebur128_add_frames_float(m.state,zero.data(),zero.size())==EBUR128_SUCCESS,"Peak endpoint flush failed");
    double peak{};need(ebur128_true_peak(m.state,0,&peak)==EBUR128_SUCCESS&&std::isfinite(peak)&&peak>=0,"True peak measurement failed");
    for(size_t i=0;i<reflect_count;++i)extension[i]=last[size_t((frames-2-i)%last.size())];
    need(ebur128_add_frames_float(edges.state,extension.data(),reflect_count)==EBUR128_SUCCESS&&ebur128_add_frames_float(edges.state,zero.data(),zero.size())==EBUR128_SUCCESS,"Final boundary peak failed");
    double reflected{};need(ebur128_true_peak(edges.state,0,&reflected)==EBUR128_SUCCESS&&std::isfinite(reflected)&&reflected>=0,"Boundary peak read failed");peak=std::max(peak,reflected);
    return {frames>=9600&&std::isfinite(loudness),std::isfinite(loudness)?loudness:0,peak};
}
}
