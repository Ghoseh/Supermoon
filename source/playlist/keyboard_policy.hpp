// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>

namespace bytebeat::playlist {
enum class KeyAction:uint8_t {previous,next,toggle_shuffle,shuffle_advance};
struct Shortcut {unsigned key{},modifiers{3};}; // Ctrl=1, Shift=2, Alt=4, Win=8
struct KeyboardConfiguration {
    bool enabled{true};
    std::array<Shortcut,4> bindings{{{0x75,3},{0x76,3},{0x77,3},{0x78,3}}};
};
struct InputContext {bool foreground{},carrier{},captured{};unsigned modifiers{};};
class KeyboardEdges {
    std::array<bool,4> armed_{};
public:
    template<class Send> void poll(const KeyboardConfiguration& config,InputContext context,
                                 const std::array<bool,4>& down,Send send) noexcept {
        bool eligible=config.enabled&&context.foreground&&context.carrier&&!context.captured;
        unsigned pressed=0;for(auto value:down)pressed+=value;
        for(unsigned i=0;i<4;++i){
            if(!eligible||!config.bindings[i].key){armed_[i]=false;continue;}
            if(!down[i]){armed_[i]=true;continue;}
            if(armed_[i]){
                armed_[i]=false; // A failed/modified edge is consumed until release too.
                if(pressed==1&&context.modifiers==config.bindings[i].modifiers)send(KeyAction(i));
            }
        }
    }
};
}
