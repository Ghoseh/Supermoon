// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "wav_pipeline.hpp"
namespace bytebeat::playlist {
bool is_mp3(const std::filesystem::path&);
// Source-format/decoder identity applies to MP3 caches ONLY. WAV recipe stays exact.
inline constexpr const char* mp3_recipe="MP3/dr_mp3-0.7.4/51e61d308dde6b437fce0c5fabb32cd86b40f4d7/bounds2-granule-errors/strict-layer3-gapless1";
uint64_t mp3_decode_calls() noexcept; // Off-callback qualification/diagnostic counter.
class Mp3 {
    struct State;State* state_{};Format format_{};
public:
    explicit Mp3(const std::filesystem::path&);
    ~Mp3();Mp3(const Mp3&)=delete;Mp3& operator=(const Mp3&)=delete;
    const Format& format()const noexcept{return format_;}
    size_t read_mono(float*,size_t);
    void restart();
    bool gapless()const noexcept;
};
}
