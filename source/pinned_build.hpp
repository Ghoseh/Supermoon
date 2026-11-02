// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <cstring>

namespace bytebeat {
// Worker-only whole-file identity gate used by bootstrap and activation. Loaded
// function/dispatch checks remain separate; a matching disk file alone is insufficient.
inline bool pinned_executable_file(const wchar_t* path) {
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE) return false;
    BCRYPT_ALG_HANDLE algorithm{}; BCRYPT_HASH_HANDLE hash{};
    bool ok=BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok) ok=BCryptCreateHash(algorithm,&hash,nullptr,0,nullptr,0,0)>=0;
    unsigned char buffer[65536],digest[32]{}; DWORD bytes=0; uint64_t total=0;
    while(ok){if(!ReadFile(file,buffer,sizeof(buffer),&bytes,nullptr)){ok=false;break;} if(!bytes)break; total+=bytes; ok=BCryptHashData(hash,buffer,bytes,0)>=0;}
    if(ok) ok=BCryptFinishHash(hash,digest,32,0)>=0;
    const unsigned char expected[32]={0xa7,0x7f,0xc2,0x98,0x77,0x78,0x9c,0x3d,0xa9,0x18,0x1a,0xfe,0xb5,0x94,0x1b,0x9b,0x85,0xa2,0x7d,0x3d,0xf2,0x49,0x19,0xa2,0x44,0x03,0xcf,0x8e,0x19,0x4c,0x61,0x09};
    ok=ok && total==89535560 && !memcmp(digest,expected,32);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return ok;
}
}
