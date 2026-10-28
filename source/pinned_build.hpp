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
    const unsigned char expected[32]={0xd1,0xc7,0x15,0x57,0x64,0x27,0x1e,0xff,0x07,0x33,0xc3,0x7d,0x18,0x49,0xe6,0x85,0xa1,0x1d,0xa3,0xd5,0x4a,0x41,0xd6,0x85,0xd7,0x89,0x8f,0xc0,0xaa,0xd8,0x3a,0x87};
    ok=ok && total==89491528 && !memcmp(digest,expected,32);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return ok;
}
}
