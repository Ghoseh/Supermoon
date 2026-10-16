// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <cstdint>
#include "playlist/stream_engine.hpp"
#include "supermoon_core.hpp"
#include "filename_title.hpp"

namespace supermoon::ui {
using Formatter=void(*)(void*,const unsigned char*,char*);
constexpr uint32_t capacity=8192;
struct Record {
    uint64_t call{},ticks{},return_site{},function_token{},output_token{};
    bytebeat::playlist::PlaybackSnapshot snapshot{};
    uint32_t thread{},route{},length{};
    bool terminated{},snapshot_valid{},carrier{};
    unsigned char key[16]{},output[256]{};
};
struct Slot {Record value{};std::atomic<bool> ready{false};};
struct Journal {
    Slot slots[capacity]{};
    std::atomic<uint64_t> calls[3]{},reserved{0},dropped{0},last_serial[2]{},last_revision[2]{};
    std::atomic<bool> enabled{false};
    Formatter original{};
    uintptr_t now_return{},row_return{};
    bytebeat::playlist::Engine* engine{}; // Mod-owned, permanently resident after publication.
    unsigned char carrier[16]{};
    void observe(void*,const unsigned char*,const char*,uintptr_t) noexcept;
};
extern Journal journal;
// Deliberately not noexcept: original exceptions must unwind normally.
extern "C" __declspec(noinline) void SupermoonObserveTitle(void*,const unsigned char*,char*);
enum class Redirect {refused,changed,protection_failure};
Redirect redirect(void* volatile* cell,void* expected,void* replacement) noexcept;
// Only called after the original exact-version activation and resident adoption.
bool start(unsigned char* base,bytebeat::playlist::Engine*,const unsigned char* carrier,bytebeat::SupermoonCore*,const FilenameTitles*) noexcept;
#ifdef UI_OBSERVER_SELFTEST_BUILD
HANDLE test_writer(const wchar_t* path) noexcept;
#endif
}
