// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#ifdef ADAPTER_RECOVERY_BUILD
#include "recovery_layout.hpp"
#else
#include "native_layout.hpp"
#endif

namespace bytebeat {
// Off-audio initialization only. No original instruction stream is modified.
// Once publication may occur, retain private code/unwind table and the pinned
// DLL through process exit, including partial failure. No hot-unload/drain proof.
class NativeClone {
    unsigned char* memory_{};
    std::vector<RUNTIME_FUNCTION> table_;
    bool registered_{},published_{};
    static void rel32(unsigned char* field,uintptr_t target,uintptr_t end) {
        // x64 relative branches/RIP references encode a signed 32-bit distance
        // from the end of the instruction. Refuse out-of-range destinations.
        int64_t distance=int64_t(target)-int64_t(end);
        if(distance<INT32_MIN||distance>INT32_MAX)throw std::runtime_error("Clone outside relative-address range");
        int32_t value=int32_t(distance);memcpy(field,&value,4);
    }
public:
    ~NativeClone(){if(!published_){if(registered_)RtlDeleteFunctionTable(table_.data());if(memory_)VirtualFree(memory_,0,MEM_RELEASE);}}
    NativeClone()=default; NativeClone(const NativeClone&)=delete;
    template<size_t Count>
    void prepare(unsigned char* base,void* const (&bridges)[Count]) {
        static_assert(Count==sizeof(layout::stubs)/sizeof(layout::stubs[0]));
        if(memory_)throw std::runtime_error("No repeated clone preparation");
        // Reserve private memory near the host, never replace an existing mapping.
        uintptr_t center=reinterpret_cast<uintptr_t>(base)&~uintptr_t(65535);
        for(uintptr_t step=65536;step<0x40000000&&!memory_;step+=65536) {
            for(int sign:{1,-1}) {
                uintptr_t target=sign>0?center+step:center-step;
                MEMORY_BASIC_INFORMATION info{};
                if(VirtualQuery(reinterpret_cast<void*>(target),&info,sizeof(info))&&info.State==MEM_FREE&&info.RegionSize>=layout::allocation_size)
                    memory_=static_cast<unsigned char*>(VirtualAlloc(reinterpret_cast<void*>(target),layout::allocation_size,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
                if(memory_)break;
            }
        }
        if(!memory_)throw std::runtime_error("No near private allocation");
        memset(memory_,0xCC,layout::allocation_size);
        for(const auto& i:layout::instructions) {
            // Metadata describes each original instruction and its new address.
            // Internal targets follow the relocated copy; external native calls
            // still target NMS. Short branches are widened when necessary.
            auto output=memory_+i.offset;
            if(i.hook>=0){auto call=output-5;call[0]=0xE8;rel32(call+1,reinterpret_cast<uintptr_t>(memory_+layout::stubs[i.hook]),reinterpret_cast<uintptr_t>(call+5));}
            memcpy(output,base+i.rva,i.original_size);
            uint32_t field=i.field;
            if(i.kind==2){output[1]=uint8_t(0x80+(output[0]&15));output[0]=0x0F;field=2;}
            if(i.kind==3){output[0]=0xE9;field=1;}
            if(i.kind){uintptr_t target=i.target_offset==UINT32_MAX?reinterpret_cast<uintptr_t>(base+i.target):reinterpret_cast<uintptr_t>(memory_+i.target_offset);rel32(output+field,target,reinterpret_cast<uintptr_t>(output+i.size));}
        }
        for(size_t n=0;n<Count;++n){auto stub=memory_+layout::stubs[n];stub[0]=0xFF;stub[1]=0x25;memset(stub+2,0,4);memcpy(stub+6,&bridges[n],8);}
        // Windows needs unwind metadata to walk through private machine code.
        // The table's vector storage must stay at a stable address after it is
        // registered, and both table and code must outlive every native caller.
        for(const auto& u:layout::unwind){memcpy(memory_+u.offset,u.bytes,u.size);table_.push_back({u.begin,u.end,u.offset});}
        std::sort(table_.begin(),table_.end(),[](auto a,auto b){return a.BeginAddress<b.BeginAddress;});
        for(size_t n=1;n<table_.size();++n)if(table_[n-1].EndAddress>table_[n].BeginAddress)throw std::runtime_error("Overlapping unwind ranges");
        if(!RtlAddFunctionTable(table_.data(),DWORD(table_.size()),reinterpret_cast<DWORD64>(memory_)))throw std::runtime_error("Unwind registration failed");
        registered_=true;
        // Build as writable data, then make it read/execute before any native
        // pointer can reference it. Flush the CPU's instruction cache as well.
        DWORD old{};
        if(!VirtualProtect(memory_,layout::allocation_size,PAGE_EXECUTE_READ,&old)||!FlushInstructionCache(GetCurrentProcess(),memory_,layout::allocation_size))throw std::runtime_error("Clone RX publication failed");
    }
    void* function(size_t index) const {return memory_+layout::functions[index].offset;}
    const unsigned char* memory() const {return memory_;}
    const std::vector<RUNTIME_FUNCTION>& table() const{return table_;}
    void published() noexcept{published_=true;}
};
}
