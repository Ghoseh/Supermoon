// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#ifdef ADAPTER_BUILD
#define ADAPTER_API extern "C" __declspec(dllexport)
#else
#define ADAPTER_API extern "C"
#endif
using AdapterProviderFn=void(*)(void*,float*,uint32_t,uint32_t*);
struct AdapterRecord {
    uint64_t sequence{},qpc{},end_qpc{},call{},stream{},component{},generation{};
    uint32_t thread{},kind{},requested{},produced{},frames{},first_clock{},last_clock{},faults{};
    float min_envelope{},max_envelope{},min_gain{},max_gain{};
    uint8_t key[16]{};
};
#ifdef ADAPTER_PLAYBACK_BUILD
ADAPTER_API DWORD WINAPI AdapterActivatePlayback(void*);
#else
ADAPTER_API DWORD WINAPI AdapterActivateDiagnostics(void*);
#endif
ADAPTER_API DWORD WINAPI AdapterFinishCapture(void*);
ADAPTER_API uint32_t AdapterRead(uint32_t,AdapterRecord*);
ADAPTER_API uint32_t AdapterCount();
ADAPTER_API uint64_t AdapterDropped();
// Offline native-context fixture invokes helpers/bridges in its OWN process.
// Not exported by production DLL; never a target-process control API.
extern "C" void AdapterFrame(void*,const unsigned char*) noexcept;
extern "C" void AdapterBefore(void*,void*,void*,const void*) noexcept;
extern "C" void AdapterAfter(void*,void*) noexcept;
extern "C" void AdapterCommit(void*,void*) noexcept;
extern "C" void AdapterRetire(void*) noexcept;
extern "C" void AdapterFrameBridge();
extern "C" void AdapterBeforeBridge();
extern "C" void AdapterAfterBridge();
extern "C" void AdapterCommitBridge();
extern "C" void AdapterRetireBridge();
#ifdef ADAPTER_SELFTEST_BUILD
#include "supermoon_core.hpp"
void AdapterTestConfigure(AdapterProviderFn,const std::vector<float>*,uintptr_t,const bytebeat::CompositionKey&,bool);
uint64_t AdapterTestCursor();
void AdapterTestProvider(void*,float*,uint32_t,uint32_t*);
void AdapterTestResetJournal();
void AdapterTestSetUnwindCheck(bool);
bool AdapterTestReadCatalogLabel(uint32_t,uint8_t*,uint8_t*,uint8_t*);
#endif

extern "C" bool AdapterCarrier(void*) noexcept;
extern "C" void AdapterCarrierBridge();

#ifdef ADAPTER_SELFTEST_BUILD
HANDLE AdapterTestStartWriter(bytebeat::playlist::Engine&,const wchar_t*);
bool AdapterTestStopWriter(HANDLE);
#endif
