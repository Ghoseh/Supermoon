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
    const unsigned char expected[32]={0x09,0x22,0x36,0x96,0x2e,0xe0,0x50,0xe4,0x19,0x42,0x46,0x99,0xb5,0xc3,0x43,0xa0,0x17,0x3f,0x77,0x73,0xd5,0xad,0x08,0x66,0xb7,0xec,0x36,0x6d,0x33,0x4c,0x07,0xd4};
    ok=ok && total==89540680 && !memcmp(digest,expected,32);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return ok;
}
}
