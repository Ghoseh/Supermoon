// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "keyboard_policy.hpp"
#include <filesystem>
#include <string>

namespace bytebeat::playlist {
KeyboardConfiguration keyboard_configuration(const std::filesystem::path& mod);
// Off-thread bounded, process-owned preference. Independent of cache/recovery.
class ShuffleStore {
    std::filesystem::path path_;
    void* lock_{};
    bool mode_{true},available_{};
    std::string error_;
public:
    explicit ShuffleStore(const std::filesystem::path& state);
    ~ShuffleStore();
    ShuffleStore(const ShuffleStore&)=delete;
    bool mode() const noexcept{return mode_;}
    bool save(bool) noexcept;
    bool available() const noexcept{return available_;}
    const std::string& error() const noexcept{return error_;}
};
}
