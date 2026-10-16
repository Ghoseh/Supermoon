// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "library.hpp"
#include <memory>
#include <functional>
namespace bytebeat::playlist {
// Container/preparation recipe: 24 kHz float mono, 64-tap sinc and reconstructed
// peak headroom. Entry identity also hashes normalization settings/measurement
// recipe and, for MP3 only, mp3_recipe. Keep all identities aligned with DSP changes.
inline constexpr unsigned cache_recipe=3;
struct CacheOptions {
    NormalizationSettings normalization;
    uint64_t bytes=3072ull*1024*1024;
    // Offline failure injection only. Production leaves this empty.
    std::function<void(const char*)> checkpoint;
};
struct CachedLibrary {
    Library library;
    // Deny write/delete for EVERY referenced PCM, including queued/future songs.
    // Prepared copies also hold the lease, so Engine lifetime is independent.
    std::vector<std::shared_ptr<void>> leases;
    uint64_t reused{},rebuilt{},removed{},disk_bytes{},hashed_bytes{};
};
// Nonblocking exclusive manager lock; busy/inaccessible roots return diagnostics.
// All hashes, preparation, eviction and file handles are off native callbacks.
CachedLibrary prepare_cached_library(const std::filesystem::path& music,
                                    const std::filesystem::path& root,
                                    const CacheOptions& options={});
struct CacheConfiguration {std::filesystem::path music;CacheOptions cache;};
// Optional playlist.ini: [Playlist] CacheMiB=16..65536, Music=<path>.
// Relative Music paths are resolved against the mod directory, never cwd.
CacheConfiguration cache_configuration(const std::filesystem::path& mod);
}
