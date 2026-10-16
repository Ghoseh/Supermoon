// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include "normalization.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

namespace bytebeat::playlist {
struct Format {uint32_t rate{};uint16_t channels{},bits{},tag{},alignment{};uint64_t offset{},bytes{},frames{};};
class Mp3;
class Wav {
    struct Pin;
    Pin* pin_{};
    Mp3* mp3_{}; // Preparation-only optional source decoder; never audio callback.
    std::ifstream file_;
    Format format_{};
    uint64_t position_{};
public:
    explicit Wav(const std::filesystem::path&);
    ~Wav();
    Wav(const Wav&)=delete;Wav& operator=(const Wav&)=delete;
    const Format& format() const{return format_;}
    size_t read_mono(float*,size_t);
    void restart();
};
struct Prepared {
    std::filesystem::path path;
    uint64_t frames{};
    double measured_reconstructed_peak{},gain{1};
    std::string source_sha256;
    // Persistent-cache ownership follows every copied Engine track, independent
    // of the caller's Library lifetime. Destruction is never on audio callbacks.
    std::shared_ptr<void> lease;
    NormalizationResult normalization;
    Prepared()=default;
    Prepared(std::filesystem::path p,uint64_t n,double peak,double scale,std::string hash,
             std::shared_ptr<void> hold={}):path(std::move(p)),frames(n),measured_reconstructed_peak(peak),gain(scale),source_sha256(std::move(hash)),lease(std::move(hold)){}
};
// Worker only. Bounded sample buffers and disk-backed intermediate/final PCM;
// peak and optional loudness verification add scans beyond decode/gain passes.
// Existing outputs are never overwritten, including accidental source==output.
Prepared prepare(const std::filesystem::path& source,const std::filesystem::path& output,const NormalizationSettings& settings={false});
class CacheReader {
    struct Pin;
    Pin* pin_{};
    std::ifstream file_;
    uint64_t frames_{},position_{};
public:
    explicit CacheReader(const Prepared&);
    ~CacheReader();
    CacheReader(const CacheReader&)=delete;CacheReader& operator=(const CacheReader&)=delete;
    size_t read(float*,size_t);
    void restart();
    void seek(uint64_t frame); // Worker-only bounded repositioning; pure toggles do not seek.
    uint64_t frames() const{return frames_;}
};
struct Discovery {std::vector<std::filesystem::path> paths;std::vector<std::string> diagnostics;};
Discovery discover(const std::filesystem::path& directory);
}
