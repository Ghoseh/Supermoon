// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "recovery_lifecycle.hpp"
namespace bytebeat::recovery {
// Setup is off-callback and precedes lifecycle publication. publish() exposes
// 15 cells individually, with the owning allocator after its 14 dependencies; false is
// never permission to retry, unload wrappers or reclaim callback-visible state.
void initialize(uintptr_t base,void* const* clone_functions) noexcept;
bool validate(unsigned char* base,HMODULE module) noexcept;
bool publish(unsigned char* base) noexcept;
void* allocation(void*,size_t,const char*,uintptr_t,uintptr_t caller);
void retire_stream(uintptr_t) noexcept;
// A current control lookup; callbacks instead carry their owned Source lease.
Source* control_endpoint(uintptr_t stream) noexcept;
bool enabled() noexcept;
void set_enabled() noexcept;
}
extern "C" void RecoveryPosted(uint32_t id) noexcept;
extern "C" void RecoveryPostedBridge();
extern "C" void AdapterRecoveryRetire(uintptr_t ticket) noexcept;
extern "C" void AdapterRecoverySource(uintptr_t ticket,uint32_t id,uint32_t phase,uintptr_t stream) noexcept;
