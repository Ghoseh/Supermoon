// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "startup_policy.hpp"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <cstdlib>
#include <new>
static thread_local bool audited;
static unsigned allocations;
void* operator new(size_t n){if(audited)++allocations;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete(void* p,size_t) noexcept{std::free(p);}
int main(int argc,char** argv){
    if(argc!=2)return 2;
    std::ifstream manifest(argv[1]);std::string line;unsigned checks=0,failures=0;
    while(std::getline(manifest,line)){
        auto a=line.find('\t'),b=line.find('\t',a+1);if(a==line.npos||b==line.npos)return 2;
        auto name=line.substr(0,a),path=line.substr(b+1);bool expected=line[a+1]=='1';
        std::ifstream input(path,std::ios::binary);if(!input)return 2;
        std::string xml{std::istreambuf_iterator<char>(input),{}};
        SYSTEM_INFO si{};GetSystemInfo(&si);size_t pages=(xml.size()+si.dwPageSize-1)/si.dwPageSize;if(!pages)pages=1;
        auto base=static_cast<char*>(VirtualAlloc(nullptr,(pages+2)*si.dwPageSize,MEM_RESERVE|MEM_COMMIT,PAGE_NOACCESS));if(!base)return 2;
        DWORD old{};if(!VirtualProtect(base+si.dwPageSize,pages*si.dwPageSize,PAGE_READWRITE,&old))return 2;
        auto at=base+(pages+1)*si.dwPageSize-xml.size();memcpy(at,xml.data(),xml.size());
        audited=true;bool authorized=bytebeat::startup::mods_enabled(std::string_view(at,xml.size()));audited=false;
        VirtualFree(base,0,MEM_RELEASE);++checks;
#if EXPECT_F1_CORRECTED
        if(authorized!=expected)++failures;
#endif
        printf("{\"case\":\"%s\",\"authorized\":%s,\"expected\":%s}\n",name.c_str(),authorized?"true":"false",expected?"true":"false");
    }
    printf("{\"checks\":%u,\"failures\":%u,\"parser_allocations\":%u,\"guard_pages\":true}\n",checks,failures,allocations);
    return failures||allocations||!checks?1:0;
}
