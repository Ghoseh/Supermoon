// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "wav_pipeline.hpp"
namespace bytebeat::playlist {
struct Library {std::vector<Prepared> tracks;std::vector<std::filesystem::path> sources;std::vector<std::string> diagnostics;std::filesystem::path cache_directory;};
// Off callback only; source audio remains untouched. Total new PCM cache bytes
// bounded by caller budget. Fresh private output avoids trusting stale caches.
Library prepare_library(const std::filesystem::path& music,const std::filesystem::path& cache_root,uint64_t byte_budget=2ull*1024*1024*1024);
}
