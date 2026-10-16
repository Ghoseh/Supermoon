// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "windows_keyboard.hpp"
#include <cwchar>

namespace bytebeat::playlist {
bool dispatch_key(Engine& engine,KeyAction action) noexcept {
    constexpr Command commands[]={Command::previous,Command::next,Command::toggle_shuffle,Command::shuffle};
    return unsigned(action)<4&&engine.command(commands[unsigned(action)]);
}
WindowsKeyboard::WindowsKeyboard(Engine& engine,KeyboardConfiguration config):engine_(engine),config_(config){
    if(!config_.enabled)return;
    worker_=std::thread([this]{
        KeyboardEdges edges;uint64_t last_pcm=0,last_activity=0;
        while(!stop_.load(std::memory_order_acquire)&&!engine_.stop_requested()){
            auto tick=GetTickCount64(),pcm=engine_.stats().pcm_frames;
            if(pcm!=last_pcm){last_pcm=pcm;last_activity=tick;}
            auto down=[](int key){return (GetAsyncKeyState(key)&0x8000)!=0;}; // Only current-state high bit.
            InputContext context{};DWORD pid{};auto window=GetForegroundWindow();auto thread=window?GetWindowThreadProcessId(window,&pid):0;
            context.foreground=window&&pid==GetCurrentProcessId();context.captured=true;
            if(context.foreground){
                GUITHREADINFO gui{};gui.cbSize=sizeof(gui);wchar_t name[64]{};
                if(thread&&GetGUIThreadInfo(thread,&gui)&&GetClassNameW(window,name,64)){
                    context.captured=(gui.flags&(GUI_INMENUMODE|GUI_INMOVESIZE|GUI_SYSTEMMENUMODE|GUI_POPUPMENUMODE))||
                        gui.hwndCaret||!wcscmp(name,L"#32770");
                    if(gui.hwndFocus&&gui.hwndFocus!=window){
                        DWORD focus_pid{};GetWindowThreadProcessId(gui.hwndFocus,&focus_pid);
                        wchar_t focus[64]{};GetClassNameW(gui.hwndFocus,focus,64);
                        if(focus_pid!=GetCurrentProcessId()||wcsstr(focus,L"Edit")||wcsstr(focus,L"ComboBox"))context.captured=true;
                    }
                }
            }
            // Selected-carrier activity plus recently consumed prepared PCM. A stopped
            // or silent unavailable native voice expires conservatively after100ms.
            // Does not infer playing state from the carrier's clock or read NMS memory.
            context.carrier=engine_.active()&&engine_.healthy()&&last_activity&&tick-last_activity<=100;
            context.modifiers=(down(VK_CONTROL)?1u:0u)|(down(VK_SHIFT)?2u:0u)|(down(VK_MENU)?4u:0u)|((down(VK_LWIN)||down(VK_RWIN))?8u:0u);
            std::array<bool,4> keys{};for(unsigned i=0;i<4;++i)keys[i]=config_.bindings[i].key&&down(int(config_.bindings[i].key));
            edges.poll(config_,context,keys,[this](KeyAction action){
                if(dispatch_key(engine_,action))admitted_.fetch_add(1);else rejected_.fetch_add(1);
            });
            Sleep(10); // Control worker only; never audio/main/DllMain.
        }
    });
}
WindowsKeyboard::~WindowsKeyboard(){stop_.store(true,std::memory_order_release);if(worker_.joinable())worker_.join();}
}
