// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "observer.hpp"
#include "supermoon_core.hpp"
#include "filename_title.hpp"
namespace supermoon::ui {
struct CallContext {uintptr_t return_site{},playlist_member{},key_list{},frame_base{};};
static_assert(sizeof(CallContext)==32);
struct TitleBinding {
    std::atomic<bool> enabled{false};
    bytebeat::SupermoonCore* core{};
    uintptr_t personal_keys{};
    const FilenameTitles* filenames{}; // Immutable, owned by resident Payload.
    std::atomic<uint64_t> replaced{0};
};
extern TitleBinding title_binding;
bool substitute_test_title(const CallContext&,const unsigned char*,char*) noexcept;
extern "C" void SupermoonTitleTestBridge();
// Do not translate exceptions from the original native formatter.
extern "C" void SupermoonTitleTestHandler(void*,const unsigned char*,char*,const CallContext*);
}
