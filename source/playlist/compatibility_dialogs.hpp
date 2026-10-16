// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include "compatibility.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>
namespace bytebeat::compatibility {
// Explicit worker-only helper. Never constructed/called by DllMain or callback.
class Dialogs {
    DWORD worker_=GetCurrentThreadId();
    void check_worker() const {if(GetCurrentThreadId()!=worker_)throw std::runtime_error("Compatibility dialog called on wrong thread");}
public:
    Decision unverified_build(const std::string& hash) const {
        check_worker();if(hash.size()!=64||hash.find_first_not_of("0123456789abcdef")!=std::string::npos)throw std::runtime_error("Invalid build identity");
        std::wstring text=L"This NMS.exe build has not been verified for Supermoon Playlist.\nBuild SHA-256: ";text.append(hash.begin(),hash.end());
        text+=L"\n\nAttempt compatibility validation? Yes never bypasses native hook validation. No disables the playback mod for this build; normal WINMM forwarding continues.";
        return MessageBoxW(nullptr,text.c_str(),L"Supermoon Playlist — unverified game build",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)==IDYES?Decision::yes:Decision::no;
    }
    bool recovery() const {
        check_worker();return MessageBoxW(nullptr,L"The previous session ended without a clean completion record while the playback mod was active. This does not establish that the mod caused a crash.\n\nAttempt playback activation again after validation?",L"Supermoon Playlist — recovery",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)==IDYES;
    }
};
}
