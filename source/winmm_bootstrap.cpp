// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
// Native bootstrap only: no PCM, synthesis, UI, catalog or duration changes.
// All 181 WINMM exports are ABI-preserving tail stubs to the runtime OS
// module, including the unnamed ordinal. DllMain never resolves exports.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include <cstdio>
#include <cwchar>
#include <cstring>
#include <cstdint>
#include "pinned_build.hpp"
#include "startup_policy.hpp"
#include "playlist/bootstrap_compatibility.hpp"
#include "winmm_bootstrap_config.hpp"
#include "supermoon_adapter_api.hpp"

extern "C" bool InitializeWinmm() noexcept;
namespace {
using namespace bytebeat::bootstrap_config;
HANDLE journal=INVALID_HANDLE_VALUE;
template<class T> T resolve(HMODULE module,const char* name) noexcept {
    auto address=GetProcAddress(module,name);T result{};
    static_assert(sizeof(address)==sizeof(result));memcpy(&result,&address,sizeof(result));return result;
}
void event(const char* state, unsigned long code=0) noexcept {
    if(journal==INVALID_HANDLE_VALUE)return;
    char line[256]; SYSTEMTIME utc{};GetSystemTime(&utc);
    int n=snprintf(line,sizeof(line),"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ pid=%lu state=%s code=%lu\n",
        utc.wYear,utc.wMonth,utc.wDay,utc.wHour,utc.wMinute,utc.wSecond,utc.wMilliseconds,
        GetCurrentProcessId(),state,code);
    DWORD written{};
    if(n>0 && size_t(n)<sizeof(line)){WriteFile(journal,line,DWORD(n),&written,nullptr);FlushFileBuffers(journal);}
}
struct File {
    HANDLE handle=INVALID_HANDLE_VALUE;
    explicit File(const wchar_t* path):handle(CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr)){}
    ~File(){if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);}
};
bool pinned_file(HANDLE file,const uint8_t* expected,uint64_t expected_bytes) noexcept {
    if(file==INVALID_HANDLE_VALUE)return false;
    BCRYPT_ALG_HANDLE alg{};BCRYPT_HASH_HANDLE hash{};
    bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptCreateHash(alg,&hash,nullptr,0,nullptr,0,0)>=0;
    uint8_t buffer[65536],digest[32]{};DWORD bytes{};uint64_t count{};
    while(ok){if(!ReadFile(file,buffer,sizeof(buffer),&bytes,nullptr)){ok=false;break;}if(!bytes)break;count+=bytes;ok=BCryptHashData(hash,buffer,bytes,0)>=0;}
    if(ok)ok=BCryptFinishHash(hash,digest,32,0)>=0;
    if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);
    return ok && count==expected_bytes && !memcmp(expected,digest,32);
}
bool setting_allows_startup() noexcept {
    File xml(settings_path);if(xml.handle==INVALID_HANDLE_VALUE)return false;
    char data[65537];DWORD bytes{};
    if(!ReadFile(xml.handle,data,sizeof(data),&bytes,nullptr) || bytes>65536)return false;
    return bytebeat::startup::mods_enabled(std::string_view(data,bytes));
}
bool opt_in() noexcept {
    File marker(enable_path);if(marker.handle==INVALID_HANDLE_VALUE)return false;
    char data[64];DWORD bytes{};
    return ReadFile(marker.handle,data,sizeof(data),&bytes,nullptr) &&
           std::string_view(data,bytes)=="ByteBeat automatic startup v1\r\n";
}
bool image_read(uintptr_t base,uint32_t rva,void* out,size_t size) noexcept {
    MEMORY_BASIC_INFORMATION info{};
    if(!VirtualQuery(reinterpret_cast<void*>(base+rva),&info,sizeof(info)) || info.State!=MEM_COMMIT ||
       info.Type!=MEM_IMAGE || reinterpret_cast<uintptr_t>(info.AllocationBase)!=base ||
       info.Protect&(PAGE_GUARD|PAGE_NOACCESS) ||
       base+rva+size>reinterpret_cast<uintptr_t>(info.BaseAddress)+info.RegionSize)return false;
    SIZE_T got{};
    return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(base+rva),out,size,&got) && got==size;
}
bytebeat::startup::Readiness readiness(uintptr_t base) noexcept {
    uintptr_t values[3]{};
    if(!image_read(base,0x6ED93C0,values,sizeof(values)))return bytebeat::startup::Readiness::conflict;
    return bytebeat::startup::classify(base,values[0],values[1],values[2]);
}
bool readiness_code_matches(uintptr_t base) noexcept {
    uint8_t bytes[32]{},digest[32]{};
    if(!image_read(base,0x31A85A0,bytes,sizeof(bytes)))return false;
    BCRYPT_ALG_HANDLE alg{};
    if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;
    bool ok=BCryptHash(alg,nullptr,0,bytes,sizeof(bytes),digest,sizeof(digest))>=0;
    BCryptCloseAlgorithmProvider(alg,0);
    return ok && !memcmp(digest,readiness_setter_sha256,32);
}
bool original_dispatch(uintptr_t base) noexcept {
    for(unsigned i=0;i<5;++i){uintptr_t value{};if(!image_read(base,dispatch_slots[i],&value,sizeof(value)) || value!=base+dispatch_targets[i])return false;}
    return true;
}
bool published_dispatch(uintptr_t base,HMODULE adapter) noexcept {
    for(unsigned i=0;i<5;++i){
        uintptr_t value{};MEMORY_BASIC_INFORMATION info{};
        if(!image_read(base,dispatch_slots[i],&value,sizeof(value)) || value==base+dispatch_targets[i] ||
           !VirtualQuery(reinterpret_cast<void*>(value),&info,sizeof(info)) || info.State!=MEM_COMMIT ||
           info.Protect&(PAGE_GUARD|PAGE_NOACCESS) || !(info.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE)))return false;
#ifdef ADAPTER_RECOVERY_BUILD
        // Provider and the owned-source cleanup wrapper are in the pinned DLL.
        // Update/pool drivers remain private executable clones. Do not accept
        // a foreign image or broaden any other slot's publication contract.
        const bool image_slot=i==0||i==2;
#else
        const bool image_slot=i==0;
#endif
        if(image_slot ? (info.Type!=MEM_IMAGE || info.AllocationBase!=adapter) : info.Type!=MEM_PRIVATE)return false;
    }
    return true;
}
bool initialize_paths(HMODULE self) noexcept {
    wchar_t executable[32768]{},proxy[32768]{};
    auto n=GetModuleFileNameW(nullptr,executable,32768),p=GetModuleFileNameW(self,proxy,32768);
    if(!n||n>=32768||!p||p>=32768)return false;
    auto slash=wcsrchr(executable,L'\\');if(!slash||_wcsicmp(slash+1,L"NMS.exe"))return false;
    if(n+128>=32768)return false;
    wcscpy(game_path,executable);slash[1]=0;
    swprintf(proxy_path,32768,L"%lswinmm.dll",executable);
    if(_wcsicmp(proxy_path,proxy))return false;
    swprintf(adapter_path,32768,L"%lsSupermoon\\supermoon_playlist.dll",executable);
    swprintf(log_directory,32768,L"%lsSupermoon",executable);
    swprintf(settings_path,32768,L"%lsSETTINGS\\GCMODSETTINGS.MXML",executable);
    swprintf(enable_path,32768,L"%lsbytebeat-playlist-experimental.enabled",executable);
    return true;
}
DWORD run_impl(HMODULE self) {
    if(!initialize_paths(self))return 2;
    // New-thread entry is delivered only after serialized DLL startup completes.
    // Pin outside DllMain. Static NMS import keeps the DLL loaded until this
    // worker can pin it. Dynamic unload before worker entry is unsupported.
    HMODULE pin{};
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                         reinterpret_cast<LPCWSTR>(&run_impl),&pin))return 1;
    wchar_t path[32768]{};
    swprintf(path,32768,L"%ls\\bytebeat-startup-latest.log",log_directory);
    journal=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    // An unobservable startup is refused, but WINMM still forwards normally.
    if(journal==INVALID_HANDLE_VALUE)return 3;
    event("worker_started_outside_DllMain");
    // Nonfatal preflight, with a pinned proxy and diagnostics, before any mod
    // publication. Native PCM callbacks can never trigger lazy OS loading.
    if(!InitializeWinmm()){event("refused_incomplete_system_winmm");return 20;}
    if(!GetModuleFileNameW(self,path,32768) || _wcsicmp(path,proxy_path)){
        event("refused_proxy_installation_path");return 2;
    }
    if(!opt_in()){event("disabled_or_missing_enable_marker");return 5;}
    auto module=GetModuleHandleW(nullptr);
    if(!module || !GetModuleFileNameW(module,path,32768) || _wcsicmp(path,game_path)){event("refused_game_identity");return 4;}
    try{if(!bytebeat::compatibility::allow_startup(path,std::filesystem::path(log_directory)/"State")){event("compatibility_declined_or_no_native_profile");return 4;}}
    catch(...){event("compatibility_state_refused");return 4;}
    if(!bytebeat::pinned_executable_file(path)){event("refused_native_profile_identity");return 4;}
    if(!setting_allows_startup()){event("disabled_or_unknown_mod_settings");return 6;}
    File adapter_file(adapter_path);
    if(!pinned_file(adapter_file.handle,adapter_sha256,adapter_bytes)){event("refused_adapter_identity");return 7;}
    if(GetModuleHandleW(L"supermoon_playlist.dll") || GetModuleHandleW(L"supermoon_playback.dll") || GetModuleHandleW(L"supermoon_adapter.dll") || GetModuleHandleW(L"player_probe.dll")) {
        event("refused_existing_adapter_or_probe");return 8;
    }
    auto base=reinterpret_cast<uintptr_t>(module);
    if(!readiness_code_matches(base)){event("refused_readiness_code_identity");return 9;}
    if(!original_dispatch(base)){event("refused_dispatch_conflict");return 9;}
    event("waiting_native_audio_registration");
    const uint64_t deadline=GetTickCount64()+120000;
    unsigned confirmations=0;
    while(confirmations<2) {
        if(GetTickCount64()>=deadline){event("refused_readiness_timeout");return 10;}
        auto status=readiness(base);
        if(status==bytebeat::startup::Readiness::conflict){event("refused_audio_registration_conflict");return 11;}
        confirmations=status==bytebeat::startup::Readiness::ready ? confirmations+1 : 0;
        if(confirmations<2)Sleep(25); // polling cadence, never a readiness criterion
    }
    // Recheck policy and dispatch immediately before loading the payload. The
    // adapter repeats executable/function/dispatch/PCM validation before writing.
    if(!opt_in() || !setting_allows_startup() || !original_dispatch(base)){
        event("refused_final_preflight");return 12;
    }
    event("native_audio_registration_ready");
    HMODULE adapter=LoadLibraryExW(adapter_path,nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!adapter){event("refused_adapter_load",GetLastError());return 13;}
    // Never unload/retry, including nonzero activator results: existing activator
    // status 10 can mean partial publication. Process exit is the only rollback.
    auto activate=resolve<DWORD(WINAPI*)(void*)>(adapter,"AdapterActivatePlayback");
    auto count=resolve<uint32_t(*)()>(adapter,"AdapterCount");
    auto read=resolve<uint32_t(*)(uint32_t,AdapterRecord*)>(adapter,"AdapterRead");
    if(!activate || !count || !read){event("refused_adapter_exports");return 14;}
    bytebeat::compatibility::Store recovery(std::filesystem::path(log_directory)/"State");
    // Legacy records are advisory; no claim that the previous exit was clean.
    if(recovery.unexpected_previous_active())event("legacy_session_unresolved_advisory");
    auto previous=recovery.previous_attempt();
    if(previous==bytebeat::compatibility::AttemptState::live){event("refused_live_attempt_owner");return 18;}
    if(previous==bytebeat::compatibility::AttemptState::exited)event("previous_attempt_owner_exited_not_clean_verdict");
    recovery.begin_attempt(bytebeat::compatibility::executable_hash(game_path));
    event("activation_attempt_once");
    DWORD result=activate(nullptr);event("activation_return",result);
    recovery.record_activation_return(result); // Includes rejection/partial publication; no retry
    if(result){event(result==10 ? "partial_publication_possible_exit_required" : "activation_refused_no_retry",result);return 15;}
    if(!published_dispatch(base,adapter)){event("publication_verification_failed_exit_required");return 16;}
    event("five_dispatch_redirects_verified");
    event("awaiting_callback_and_commit_evidence_PCM_requires_CSV");
    // Accepted C3: only result 10 explicitly names partial publication;
    // result 16/later preparation refusals may also leave resident wrappers.
    // All such refusals require normal exit, never retry or hot unloading.
    // Read only the payload's immutable journal. A successful LoadLibrary or
    // even five redirects does not prove execution or audible replacement.
    bool callback=false,commit=false;uint32_t cursor=0;
    while(!(callback && commit)) {
        uint32_t end=count();
        for(;cursor<end;++cursor) {
            AdapterRecord record{};if(!read(cursor,&record))break;
            if(record.faults){event("adapter_recorded_fault",record.faults);return 17;}
            if(!callback && record.kind==1){callback=true;event("native_provider_callback_observed");}
            if(!commit && record.kind==4 && record.generation){commit=true;event("successful_native_commit_observed");}
        }
        if(!(callback && commit))Sleep(100);
    }
    event("hook_execution_observed_PCM_requires_CSV");return 0;
}
DWORD run(HMODULE self) noexcept {try{return run_impl(self);}catch(...){event("refused_state_or_storage_exception");return 19;}}
DWORD WINAPI worker(void* module) noexcept {
    auto result=run(static_cast<HMODULE>(module));
    if(journal!=INVALID_HANDLE_VALUE){CloseHandle(journal);journal=INVALID_HANDLE_VALUE;}
    return result;
}
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH){
        // Minimal deferred-worker design. No waiting, loading, hashing, CRT
        // work, native reads, hooks or forwarding setup inside loader lock.
        // Failure to schedule leaves ordinary system WINMM behavior intact.
        HANDLE thread=CreateThread(nullptr,0,worker,module,0,nullptr);
        if(thread)CloseHandle(thread);
    }
    return TRUE;
}
