// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "compatibility.hpp"
#include "compatibility_dialogs.hpp"
#include "pinned_build.hpp"
#include <bcrypt.h>
#include <array>
namespace bytebeat::compatibility {
inline std::string executable_hash(const wchar_t* path){
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
    if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot pin executable for compatibility");
    BCRYPT_ALG_HANDLE algorithm{};BCRYPT_HASH_HANDLE hash{};std::array<unsigned char,65536> bytes{};unsigned char digest[32]{};DWORD n{};
    bool ok=BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptCreateHash(algorithm,&hash,nullptr,0,nullptr,0,0)>=0;
    while(ok){if(!ReadFile(file,bytes.data(),DWORD(bytes.size()),&n,nullptr)){ok=false;break;}if(!n)break;ok=BCryptHashData(hash,bytes.data(),n,0)>=0;}
    if(ok)ok=BCryptFinishHash(hash,digest,32,0)>=0;if(hash)BCryptDestroyHash(hash);if(algorithm)BCryptCloseAlgorithmProvider(algorithm,0);CloseHandle(file);
    if(!ok)throw std::runtime_error("Executable SHA256 failed");std::string out;for(auto b:digest){out.push_back("0123456789abcdef"[b>>4]);out.push_back("0123456789abcdef"[b&15]);}return out;
}
// Called only by the deferred bootstrap worker AFTER explicit enable-marker
// checks. No dialog can bypass the pinned profile or subsequent site checks.
inline bool allow_startup(const wchar_t* executable,const std::filesystem::path& state){
    auto identity=executable_hash(executable);bool verified=bytebeat::pinned_executable_file(executable);
    Store store(state);Decision decision=Decision::yes;
    if(store.known(identity))decision=store.get(identity);
    else if(!verified){decision=Dialogs().unverified_build(identity);store.remember(identity,decision);}
    return evaluate({verified,verified,true,false,true},decision)==Result::activate;
}
}
