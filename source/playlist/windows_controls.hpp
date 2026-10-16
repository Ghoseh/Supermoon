// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "stream_engine.hpp"
namespace bytebeat::playlist {
// Legacy observer; production uses WindowsKeyboard and configured chords.
// Disabled default. Do not assign Shift+T until native/user binding conflicts
// have been checked. This observes keys without intercepting normal game input.
struct KeyConfiguration {bool enabled{};unsigned next{},previous{};};
bool valid_keys(KeyConfiguration) noexcept;
class WindowsControls {
    Engine& engine_;KeyConfiguration config_;std::atomic<bool> stop_{false};std::thread worker_;
public:
    WindowsControls(Engine&,KeyConfiguration={});
    ~WindowsControls();
    WindowsControls(const WindowsControls&)=delete;WindowsControls& operator=(const WindowsControls&)=delete;
};
}
