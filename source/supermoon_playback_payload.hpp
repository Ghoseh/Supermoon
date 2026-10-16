// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include "supermoon_core.hpp"
#include "filename_title.hpp"
#include "playlist/persistent_cache.hpp"
#include "playlist/keyboard_settings.hpp"
#include "playlist/windows_keyboard.hpp"
#include <memory>
#include <stdexcept>

namespace bytebeat::playback {
struct Configuration {bool live_identity_reviewed{};CompositionKey carrier{};SafetyEvidence evidence{};};
inline std::filesystem::path executable_path(){wchar_t path[32768]{};auto n=GetModuleFileNameW(nullptr,path,32768);if(!n||n>=32768)throw std::runtime_error("Executable path unavailable");return path;}
inline bool location_matches(const wchar_t* path){auto expected=executable_path().parent_path()/"Supermoon"/"supermoon_playlist.dll";return !_wcsicmp(expected.c_str(),path);}
// Optional UI preparation failure leaves audio preparation usable. No metadata IO.
inline supermoon::ui::FilenameTitles prepare_titles(const playlist::Library& library) noexcept {
    try {
        if(library.sources.size()!=library.tracks.size()||library.sources.size()>10000)return {};
        supermoon::ui::FilenameTitles result;result.reserve(library.sources.size());
        for(const auto& source:library.sources)
            result.push_back(supermoon::ui::filename_title(source.filename().native()));
        return result;
    }catch(...){return {};}
}
struct Payload {
    playlist::ShuffleStore shuffle_store;
    playlist::CachedLibrary cached;
    playlist::Library& library;
    const supermoon::ui::FilenameTitles filenames;
    playlist::Engine engine;
    SupermoonCore core;
    playlist::WindowsKeyboard keyboard;
    explicit Payload(playlist::CachedLibrary&& input,uint32_t seed,const std::filesystem::path& mod,playlist::KeyboardConfiguration keys,bool notifications=true):shuffle_store(mod/"State"),cached(std::move(input)),library(cached.library),filenames(prepare_titles(library)),engine(library.tracks,seed,64,shuffle_store.mode(),[this](bool mode){return shuffle_store.save(mode);},notifications),core(engine),keyboard(engine,keys){}
    Payload(const Payload&)=delete;Payload& operator=(const Payload&)=delete;
};
inline std::unique_ptr<Payload> prepare(const Configuration& config){
    if(!config.live_identity_reviewed||!config.evidence.ready())throw std::runtime_error("Carrier identity/profile not reviewed");
    bool nonzero=false;for(auto b:config.carrier)nonzero|=b!=0;if(!nonzero)throw std::runtime_error("Missing carrier key");
    auto base=executable_path().parent_path()/"Supermoon";
    auto settings=playlist::cache_configuration(base);
    playlist::KeyboardConfiguration keys;std::string keyboard_error;
    try{keys=playlist::keyboard_configuration(base);}catch(const std::exception& e){keys.enabled=false;keyboard_error=e.what();}
    bool notifications=false;std::string notification_error;
    try{notifications=playlist::notification_configuration(base);}catch(const std::exception& e){notification_error=e.what();}
    auto cached=playlist::prepare_cached_library(settings.music,base/"Cache",settings.cache);
    auto& library=cached.library;
    auto log=base/"preparation-latest.log";
    std::ofstream journal(log,std::ios::binary);if(!journal)throw std::runtime_error("Preparation diagnostics unavailable");
    journal<<"Validated persistent cache recipe=3 quota_bytes="<<settings.cache.bytes<<" reused="<<cached.reused<<" rebuilt="<<cached.rebuilt<<" disk_bytes="<<cached.disk_bytes<<" hashed_bytes="<<cached.hashed_bytes<<"\n";
    journal<<"keyboard_enabled="<<keys.enabled<<" keyboard_configuration_error="<<keyboard_error<<'\n';
    journal<<"shuffle_notifications_enabled="<<notifications<<" notification_configuration_error="<<notification_error<<'\n';
    for(auto& message:library.diagnostics)journal<<message<<'\n';
    for(size_t i=0;i<library.tracks.size();++i){auto& t=library.tracks[i];journal<<"track_id="<<i<<" filename="<<library.sources[i].filename().u8string()<<" frames="<<t.frames<<" gain="<<t.gain<<" source_sha256="<<t.source_sha256<<'\n';}
    journal<<"normalization_settings="<<playlist::normalization_identity(settings.cache.normalization)<<'\n';
    for(size_t i=0;i<library.tracks.size();++i){auto& n=library.tracks[i].normalization;
        journal<<"normalization track_id="<<i<<" enabled="<<n.enabled<<" loudness_defined="<<n.defined<<" input_lufs="<<n.input_lufs<<" output_lufs="<<n.output_lufs<<" true_peak="<<n.true_peak<<" peak_limited="<<n.limited<<'\n';}
    journal.close();if(!journal)throw std::runtime_error("Preparation diagnostics flush failed");
    if(library.tracks.empty())throw std::runtime_error("No valid songs within experimental cache budget");
    uint32_t seed{};if(BCryptGenRandom(nullptr,reinterpret_cast<PUCHAR>(&seed),sizeof(seed),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0)throw std::runtime_error("Shuffle seed unavailable");
    auto payload=std::make_unique<Payload>(std::move(cached),seed,base,keys,notifications);
    std::ofstream status(log,std::ios::app);
    if(status)status<<"shuffle_initial="<<payload->engine.shuffle_enabled()<<" shuffle_persistence_available="<<payload->shuffle_store.available()<<" shuffle_persistence_error="<<payload->shuffle_store.error()<<'\n';
    payload->core.configure(config.carrier,true,config.evidence,true);return payload;
}
}
