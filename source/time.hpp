// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "title_test.hpp"
#include "time_caption.hpp"
#include "ui_profile.hpp"
namespace supermoon::ui::time {
// Per-render copied tuple, valid only within the matching original simulation
// call. Nested/reentrant calls taint the outer slot; TimeSimulation __finally
// clears/restores TLS even when the original C++/SEH exception escapes.
constexpr unsigned max_depth=4;
struct Context {uintptr_t site{},member{},keys{},frame{},source{};};
struct Frozen {bytebeat::playlist::PlaybackSnapshot snapshot{};FilenameTitle filename{};std::array<char,32> caption{};uint8_t length{};};
struct Slot {uintptr_t frame{},member{},keys{},root{},stream{};bool valid{},tainted{};unsigned phase{},titles{};Frozen value{};};
extern std::atomic<bool> enabled;
extern std::atomic<uint64_t> title_replaced,time_replaced,fallbacks;
extern uintptr_t image,now_return,time_return,keys_address;
extern bytebeat::playlist::Engine* engine;
extern bytebeat::SupermoonCore* core;
extern const FilenameTitles* filenames;
extern unsigned char carrier_key[16];
using Read=BOOL(WINAPI*)(HANDLE,LPCVOID,LPVOID,SIZE_T,SIZE_T*);
extern Read read_memory;
bool safe_read(uintptr_t,void*,size_t) noexcept;
bool substitute_title(const CallContext&,const unsigned char*,char*) noexcept;
bool prepare(unsigned char*,bytebeat::playlist::Engine*,bytebeat::SupermoonCore*,const unsigned char*,const FilenameTitles*) noexcept;
bool publish(unsigned char*) noexcept;
void cancel() noexcept;
#ifdef UI_TIME_TEST
using SnapshotReader=bool(*)(bytebeat::playlist::PlaybackSnapshot&);
extern SnapshotReader test_snapshot;
Slot test_slot() noexcept;
unsigned test_depth() noexcept;
#endif
}
extern "C" {
extern void* TimeOriginals[2];
void TimeSimulation(void*);
void TimeSetterBridge();
const char* TimeSelect(void*,const supermoon::ui::time::Context*) noexcept;
}
