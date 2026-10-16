// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace bytebeat {
// Retained 24 kHz mono-float WAV utility, not the production mixed-format reader.
// Production uses playlist/wav_pipeline.hpp and mp3_reader.hpp during preparation.
// Conversion/decoding belongs outside the audio thread.
struct WavInfo { uint64_t offset{}, bytes{}, frames{}; };
WavInfo inspect_wav(const std::filesystem::path& path);
std::vector<float> preload_wav(const std::filesystem::path& path);
// Benchmark/worker reader. Never call from a real-time callback.
class ChunkReader {
    std::ifstream file_;
    WavInfo info_;
    uint64_t position_{};
public:
    explicit ChunkReader(const std::filesystem::path& path);
    size_t read(float* output, size_t capacity);
    void restart();
    const WavInfo& info() const { return info_; }
};
}
