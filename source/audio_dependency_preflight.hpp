// SPDX-License-Identifier: MIT
#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <filesystem>
#include <string>
#include <cstring>
#include "required_audio_policy.hpp"

namespace bytebeat::startup {
struct AudioPreflight {AudioPolicy policy;bool loose_scan_limited{};};
namespace audio_file_detail {
struct File {
    HANDLE handle;
    explicit File(const std::filesystem::path& path) noexcept:
        handle(CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr)){}
    ~File(){if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);}
};
inline bool equivalent_banks(const std::filesystem::path& first,const std::filesystem::path& second) noexcept {
    File a(first),b(second);LARGE_INTEGER sa{},sb{};
    if(a.handle==INVALID_HANDLE_VALUE||b.handle==INVALID_HANDLE_VALUE||
       !GetFileSizeEx(a.handle,&sa)||!GetFileSizeEx(b.handle,&sb)||sa.QuadPart!=sb.QuadPart||
       sa.QuadPart<0||sa.QuadPart>65536)return false;
    unsigned char left[4096],right[4096];
    for(uint64_t offset=0;offset<uint64_t(sa.QuadPart);){
        DWORD want=DWORD((uint64_t(sa.QuadPart)-offset)>sizeof(left)?sizeof(left):uint64_t(sa.QuadPart)-offset),got_a{},got_b{};
        if(!ReadFile(a.handle,left,want,&got_a,nullptr)||!ReadFile(b.handle,right,want,&got_b,nullptr)||
           got_a!=want||got_b!=want||memcmp(left,right,want))return false;
        offset+=want;
    }
    return true;
}
inline bool component_name(std::string_view raw,std::wstring& decoded) {
    size_t p=0;
    while(p<raw.size()){
        unsigned code{};if(!next_xml_code(raw,p,code)||code<32||code=='/'||code=='\\'||
           code==':'||code=='"'||code=='<'||code=='>'||code=='|'||code=='*'||code=='?')return false;
        if(code<=0xffff)decoded.push_back(wchar_t(code));
        else{code-=0x10000;decoded.push_back(wchar_t(0xd800+(code>>10)));decoded.push_back(wchar_t(0xdc00+(code&1023)));}
    }
    return !decoded.empty()&&decoded!=L"."&&decoded!=L".."&&decoded.back()!=L'.'&&decoded.back()!=L' ';
}
struct LooseConflictCheck {
    const std::filesystem::path& mods;
    bool limited{};
    bool operator()(std::string_view raw_name) noexcept {
        bool resource_found=false;
        try{
            std::wstring name;if(!component_name(raw_name,name)){limited=true;return false;}
            auto folder=mods/name;auto attributes=GetFileAttributesW(folder.c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES){auto error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)limited=true;return false;}
            if(!(attributes&FILE_ATTRIBUTE_DIRECTORY)||(attributes&FILE_ATTRIBUTE_REPARSE_POINT)){limited=true;return false;}
            for(const auto* part:{L"audio",L"windows"}){
                folder/=part;attributes=GetFileAttributesW(folder.c_str());
                if(attributes==INVALID_FILE_ATTRIBUTES){auto error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)limited=true;return false;}
                if(!(attributes&FILE_ATTRIBUTE_DIRECTORY)||(attributes&FILE_ATTRIBUTE_REPARSE_POINT)){limited=true;return false;}
            }
            auto other=folder/L"init.bnk";attributes=GetFileAttributesW(other.c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES){auto error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)limited=true;return false;}
            if(attributes&FILE_ATTRIBUTE_DIRECTORY)return false;
            resource_found=true;
            // Only a known-enabled record with this exact loose resource is a
            // competing override. Unrelated mods/packages are never refused.
            if(attributes&FILE_ATTRIBUTE_REPARSE_POINT)return true;
            return !equivalent_banks(other,mods/L"Supermoon_MenuAudio_Test/audio/windows/init.bnk");
        }catch(...){limited=true;return resource_found;}
    }
};
}
// Settings and known loose-conflict preconditions only. The existing executable,
// payload, native-site and exact required-bank hash gates remain mandatory.
inline AudioPreflight audio_dependency_preflight(const wchar_t* executable) noexcept {
    try{
        if(!executable||!*executable)return {AudioPolicy::settings_unreadable,false};
        auto binaries=std::filesystem::path(executable).parent_path();
        char data[65537];DWORD bytes{};
        {
        audio_file_detail::File file(binaries/L"SETTINGS/GCMODSETTINGS.MXML");
        if(file.handle==INVALID_HANDLE_VALUE){auto error=GetLastError();
            return {error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND||error==ERROR_SHARING_VIOLATION?
                    AudioPolicy::registration_pending:AudioPolicy::settings_unreadable,false};}
        if(GetFileType(file.handle)!=FILE_TYPE_DISK)return {AudioPolicy::settings_unreadable,false};
        BY_HANDLE_FILE_INFORMATION before{},after{};
        if(!GetFileInformationByHandle(file.handle,&before)||!ReadFile(file.handle,data,sizeof(data),&bytes,nullptr)||
           !GetFileInformationByHandle(file.handle,&after))return {AudioPolicy::settings_unreadable,false};
        if(bytes>65536||before.nFileSizeHigh)return {AudioPolicy::malformed_or_global_disabled,false};
        if(!bytes||before.nFileSizeLow!=bytes||before.nFileSizeLow!=after.nFileSizeLow||
           before.nFileSizeHigh!=after.nFileSizeHigh||CompareFileTime(&before.ftLastWriteTime,&after.ftLastWriteTime))
            return {AudioPolicy::registration_pending,false};
        } // Close the read handle before parsing/scanning; never pin settings.
        auto mods=binaries.parent_path()/L"GAMEDATA/MODS";
        audio_file_detail::LooseConflictCheck check{mods};
        auto policy=inspect_audio_settings(std::string_view(data,bytes),check);
        return {policy,check.limited};
    }catch(...){return {AudioPolicy::settings_unreadable,false};}
}
}
