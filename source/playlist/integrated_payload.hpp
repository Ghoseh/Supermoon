// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
// Retained initial preparation experiment, not the production payload entry.
// supermoon_playback_payload.hpp selects persistent cache, normalization and keys.
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include "supermoon_core.hpp"
#include "playlist/library.hpp"
#include <memory>
#include <stdexcept>

namespace bytebeat::playback {
struct Configuration {bool live_identity_reviewed{};CompositionKey carrier{};SafetyEvidence evidence{};};
inline std::filesystem::path executable_path(){wchar_t path[32768]{};auto n=GetModuleFileNameW(nullptr,path,32768);if(!n||n>=32768)throw std::runtime_error("Executable path unavailable");return path;}
inline bool location_matches(const wchar_t* path){auto expected=executable_path().parent_path()/"Supermoon"/"supermoon_playlist.dll";return !_wcsicmp(expected.c_str(),path);}
struct Payload {
    playlist::Library library;
    playlist::Engine engine;
    SupermoonCore core;
    explicit Payload(playlist::Library&& input,uint32_t seed):library(std::move(input)),engine(library.tracks,seed),core(engine){}
    Payload(const Payload&)=delete;Payload& operator=(const Payload&)=delete;
};
inline std::unique_ptr<Payload> prepare(const Configuration& config){
    if(!config.live_identity_reviewed||!config.evidence.ready())throw std::runtime_error("Carrier identity/profile not reviewed");
    bool nonzero=false;for(auto b:config.carrier)nonzero|=b!=0;if(!nonzero)throw std::runtime_error("Missing carrier key");
    auto base=executable_path().parent_path()/"Supermoon";
    // This legacy path creates a fresh private run cache. Production incremental
    // cache reuse is implemented separately in prepare_cached_library().
    auto library=playlist::prepare_library(base/"Music",base/"Cache",256ull*1024*1024);
    auto log=base/("preparation-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+".log");
    std::ofstream journal(log,std::ios::binary);if(!journal)throw std::runtime_error("Preparation diagnostics unavailable");
    journal<<"Experimental internal preparation: 256 MiB peak new-cache budget\n";
    for(auto& message:library.diagnostics)journal<<message<<'\n';
    for(size_t i=0;i<library.tracks.size();++i){auto& t=library.tracks[i];journal<<"track_id="<<i<<" filename="<<library.sources[i].filename().u8string()<<" frames="<<t.frames<<" gain="<<t.gain<<" source_sha256="<<t.source_sha256<<'\n';}
    journal.close();if(!journal)throw std::runtime_error("Preparation diagnostics flush failed");
    if(library.tracks.empty())throw std::runtime_error("No valid songs within experimental cache budget");
    uint32_t seed{};if(BCryptGenRandom(nullptr,reinterpret_cast<PUCHAR>(&seed),sizeof(seed),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0)throw std::runtime_error("Shuffle seed unavailable");
    auto payload=std::make_unique<Payload>(std::move(library),seed);
    payload->core.configure(config.carrier,true,config.evidence,true);return payload;
}
}
