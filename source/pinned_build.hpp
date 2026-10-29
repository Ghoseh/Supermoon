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
    const unsigned char expected[32]={0xd3,0x93,0xc6,0x78,0x2d,0x29,0xe1,0x91,0x58,0x54,0xc0,0xd5,0xcf,0x42,0x50,0x5c,0x80,0x93,0x74,0x0c,0x05,0xdc,0x53,0x70,0x5d,0xbe,0xd7,0xe0,0xd1,0xd4,0xac,0x37};
    ok=ok && total==89489480 && !memcmp(digest,expected,32);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return ok;
}
}
