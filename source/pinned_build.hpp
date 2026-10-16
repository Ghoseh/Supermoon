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
    const unsigned char expected[32]={0x13,0xd5,0x06,0x0d,0x4e,0xfb,0x9d,0x2a,0x6a,0x6b,0x1b,0x34,0x9b,0xc4,0x25,0x72,0x31,0x05,0x6c,0xc2,0xa0,0x55,0xdf,0x4b,0xb1,0x5d,0x81,0x62,0x62,0xcc,0x34,0x99};
    ok=ok && total==88560712 && !memcmp(digest,expected,32);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return ok;
}
}
