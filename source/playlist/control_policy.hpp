// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>

namespace bytebeat::playlist {
enum class PlaylistAction:uint8_t {none,next,previous,shuffle};
// Legacy control experiment; production uses KeyboardEdges/WindowsKeyboard.
// This helper requires explicit binding, foreground, active carrier,
// no text entry/menu and exact modifier match. Native controls are never consumed.
// Conflict-free installation still requires a verified active-game binding map.
struct ControlContext {bool foreground{},carrier{},text_entry{},menu{};unsigned modifiers{};};
struct KeyBinding {unsigned key{},modifiers{};PlaylistAction action{};};
inline bool allowed_binding(KeyBinding b) noexcept {
    // Default-disabled policy; Ctrl+Shift+F6..F8 opt-in candidates only.
    return b.key>=0x75&&b.key<=0x77&&b.modifiers==3&&
        (b.action==PlaylistAction::next||b.action==PlaylistAction::previous||b.action==PlaylistAction::shuffle);
}
struct ControlEdge {
    bool previous_down{},previous_eligible{};
    PlaylistAction poll(KeyBinding b,ControlContext c,bool down,bool known_conflict) noexcept {
        bool eligible=allowed_binding(b)&&!known_conflict&&c.foreground&&c.carrier&&!c.text_entry&&!c.menu&&c.modifiers==b.modifiers;
        auto action=eligible&&previous_eligible&&down&&!previous_down?b.action:PlaylistAction::none;
        previous_down=down;previous_eligible=eligible;return action;
    }
};
}
