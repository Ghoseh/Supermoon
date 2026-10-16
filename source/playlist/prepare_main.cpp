// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "library.hpp"
#include <iostream>
int wmain(int argc,wchar_t** argv){try{
    wchar_t exe[32768]{};auto n=GetModuleFileNameW(nullptr,exe,32768);if(!n||n>=32768)throw std::runtime_error("Cannot locate executable");
    auto base=std::filesystem::path(exe).parent_path();
    if(argc>3){std::cerr<<"Usage: playlist_prepare.exe [Music directory] [private cache root]\n";return 2;}
    auto library=bytebeat::playlist::prepare_library(argc>1?std::filesystem::path(argv[1]):base/"Music",argc>2?std::filesystem::path(argv[2]):base/"Cache");
    for(auto& d:library.diagnostics)std::cerr<<d<<'\n';
    if(library.tracks.empty()){std::cerr<<"No valid songs prepared\n";return 1;}
    std::ofstream manifest(library.cache_directory/"manifest.tsv",std::ios::binary);manifest<<"SUPERMOON_PRIVATE_CACHE_V1\n";
    for(size_t i=0;i<library.tracks.size();++i){auto& t=library.tracks[i];manifest<<t.path.filename().u8string()<<'\t'<<t.frames<<'\t'<<t.source_sha256<<'\n';std::cout<<library.sources[i].filename().u8string()<<" frames="<<t.frames<<" gain="<<t.gain<<'\n';}
    manifest.close();if(!manifest)throw std::runtime_error("Manifest flush failed");
    std::cout<<"Prepared "<<library.tracks.size()<<" songs; cache="<<library.cache_directory.u8string()<<"\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
