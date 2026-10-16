// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "windows_controls.hpp"
#include <chrono>
#include <stdexcept>
namespace bytebeat::playlist {
bool valid_keys(KeyConfiguration c) noexcept {
    if(!c.enabled)return true;
    auto valid=[](unsigned k){return (k>='A'&&k<='Z')||(k>=VK_F1&&k<=VK_F12);};
    return valid(c.next)&&valid(c.previous)&&c.next!=c.previous;
}
WindowsControls::WindowsControls(Engine& e,KeyConfiguration c):engine_(e),config_(c){
    if(!valid_keys(c))throw std::runtime_error("Unsafe/duplicate track key configuration");if(!c.enabled)return;
    worker_=std::thread([this]{InputEdges edges;while(!stop_.load(std::memory_order_acquire)){
        DWORD foreground_pid{};auto window=GetForegroundWindow();if(window)GetWindowThreadProcessId(window,&foreground_pid);
        auto down=[](int key){return (GetAsyncKeyState(key)&0x8000)!=0;};
        bool foreground=foreground_pid==GetCurrentProcessId()&&!down(VK_CONTROL)&&!down(VK_MENU);
        edges.poll(foreground,engine_.active()&&engine_.healthy(),down(VK_SHIFT),down(int(config_.next)),down(int(config_.previous)),[this](Command command){engine_.command(command);});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }});
}
WindowsControls::~WindowsControls(){stop_.store(true,std::memory_order_release);if(worker_.joinable())worker_.join();}
}
