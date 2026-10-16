// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "notifications.hpp"
#include <cwchar>
#include <stdexcept>
namespace bytebeat::playlist {
#include "notification_assets_data.inc"
bool notification_configuration(const std::filesystem::path& mod){
    auto ini=mod/"playlist.ini";auto attrs=GetFileAttributesW(ini.c_str());
    if(attrs!=INVALID_FILE_ATTRIBUTES&&(attrs&FILE_ATTRIBUTE_REPARSE_POINT))throw std::runtime_error("Notification config reparse point refused");
    wchar_t text[16]{};auto n=GetPrivateProfileStringW(L"Notifications",L"ShuffleSounds",L"1",text,16,ini.c_str());
    if(n>=15||(!wcscmp(text,L"0")?false:wcscmp(text,L"1")!=0))throw std::runtime_error("Notifications ShuffleSounds must be 0 or 1");
    return text[0]==L'1';
}
}
