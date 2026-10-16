// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "stream_engine.hpp"
#include "keyboard_policy.hpp"

namespace bytebeat::playlist {
bool dispatch_key(Engine&,KeyAction) noexcept;
// Separate off-audio observer, no hooks, registered global hotkeys or key capture.
class WindowsKeyboard {
    Engine& engine_;KeyboardConfiguration config_;std::atomic<bool> stop_{false};std::thread worker_;
    std::atomic<uint64_t> admitted_{0},rejected_{0};
public:
    WindowsKeyboard(Engine&,KeyboardConfiguration);
    ~WindowsKeyboard();
    WindowsKeyboard(const WindowsKeyboard&)=delete;
    uint64_t admitted() const noexcept{return admitted_.load();}
    uint64_t rejected() const noexcept{return rejected_.load();}
};
}
