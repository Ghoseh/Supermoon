// SPDX-License-Identifier: MIT
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "wav_pipeline.hpp"
namespace bytebeat::playlist {
// Cold discovery only. Production uses the Win32 functions; owned fixtures can
// inject each error without relying on the standard library's iterator adapter.
struct DiscoveryIO {
    decltype(&FindFirstFileW) first=&FindFirstFileW;
    decltype(&FindNextFileW) next=&FindNextFileW;
    decltype(&GetFileAttributesW) attributes=&GetFileAttributesW;
    decltype(&FindClose) close=&FindClose;
    decltype(&GetLastError) error=&GetLastError;
};
Discovery discover(const std::filesystem::path&,const DiscoveryIO&);
}
