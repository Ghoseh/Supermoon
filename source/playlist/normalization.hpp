// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <filesystem>
#include <string>
#include <cstdint>
namespace bytebeat::playlist {
struct NormalizationSettings {bool enabled=true;double target_lufs=-16,ceiling_dbtp=-1;};
struct NormalizationResult {
    bool enabled{},defined{},limited{};
    double input_lufs{},output_lufs{},true_peak{};
};
struct LoudnessMeasurement {bool defined{};double lufs{},true_peak{};};
void validate_normalization(const NormalizationSettings&);
std::string normalization_identity(const NormalizationSettings&);
NormalizationSettings normalization_configuration(const std::filesystem::path&);
// Prepared mono float PCM at 24 kHz; bounded buffers/histogram, worker only.
LoudnessMeasurement measure_pcm(const std::filesystem::path&,uint64_t frames);
}
