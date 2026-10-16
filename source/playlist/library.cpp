// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "library.hpp"
#include <stdexcept>
namespace bytebeat::playlist {
Library prepare_library(const std::filesystem::path& music,const std::filesystem::path& root,uint64_t budget){
    Library result;auto found=discover(music);result.diagnostics=std::move(found.diagnostics);
    if(found.paths.empty())return result;
    std::filesystem::create_directories(root);
    for(unsigned attempt=0;attempt<1024;++attempt){auto run=root/("run-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+"-"+std::to_string(attempt));
        if(CreateDirectoryW(run.c_str(),nullptr)){result.cache_directory=run;break;}
        if(GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("Cannot create private playlist cache");}
    if(result.cache_directory.empty())throw std::runtime_error("Cannot reserve unique playlist cache");
    uint64_t used=0;
    for(size_t i=0;i<found.paths.size();++i){auto& source=found.paths[i];
        try{
            uint64_t required;
            {Wav header(source);auto& f=header.format();auto frames=(f.frames*24000+f.rate-1)/f.rate;required=frames*4;}
            // Raw/final PCM coexist; peak/loudness verification can add scans.
            if(required>budget-used||required>(budget-used)/2){result.diagnostics.push_back(source.filename().u8string()+": cache budget exceeded");continue;}
            auto prepared=prepare(source,result.cache_directory/(std::to_string(i)+".pcm"));
            if(prepared.frames*4!=required){std::filesystem::remove(prepared.path);throw std::runtime_error("Source changed during preparation");}
            used+=required;result.tracks.push_back(std::move(prepared));result.sources.push_back(source);
        }catch(const std::exception& error){result.diagnostics.push_back(source.filename().u8string()+": "+error.what());}
    }
    return result;
}
}
