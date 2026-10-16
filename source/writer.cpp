// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#include "observer.hpp"
#include "title_test.hpp"
#include "ui_profile.hpp"
#include "time.hpp"
#include "time_profile.hpp"
#include "register_profile.hpp"
#include <bcrypt.h>
#include <cstdio>
#include <cstring>

namespace supermoon::ui {
namespace {
HANDLE file=INVALID_HANDLE_VALUE;
std::atomic<uint32_t> publication{0}; // 0 private,1 resident/armed,2 refused,3 partial protection failure
constexpr uint64_t byte_limit=32*1024*1024,detail_limit=4*1024*1024,time_limit=6*60*60*1000;
bool digest(const unsigned char* data,uint32_t bytes,const unsigned char* expected) noexcept {
    BCRYPT_ALG_HANDLE alg{};BCRYPT_HASH_HANDLE hash{};unsigned char out[32]{};
    bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptCreateHash(alg,&hash,nullptr,0,nullptr,0,0)>=0;
    if(ok)ok=BCryptHashData(hash,const_cast<unsigned char*>(data),bytes,0)>=0;
    if(ok)ok=BCryptFinishHash(hash,out,32,0)>=0;
    if(hash)BCryptDestroyHash(hash);
    if(alg)BCryptCloseAlgorithmProvider(alg,0);
    return ok&&memcmp(out,expected,32)==0;
}
bool image_range(unsigned char* base,uint32_t rva,uint32_t size) noexcept {
    uintptr_t p=reinterpret_cast<uintptr_t>(base)+rva,end=p+size;
    while(p<end){MEMORY_BASIC_INFORMATION i{};
        if(!VirtualQuery(reinterpret_cast<void*>(p),&i,sizeof(i))||i.State!=MEM_COMMIT||i.Type!=MEM_IMAGE||i.AllocationBase!=base||i.Protect&(PAGE_GUARD|PAGE_NOACCESS))return false;
        auto protection=i.Protect&0xff;
        if(protection!=PAGE_READONLY&&protection!=PAGE_READWRITE&&protection!=PAGE_WRITECOPY&&
            protection!=PAGE_EXECUTE_READ&&protection!=PAGE_EXECUTE_READWRITE&&protection!=PAGE_EXECUTE_WRITECOPY)return false;
        auto next=reinterpret_cast<uintptr_t>(i.BaseAddress)+i.RegionSize;
        if(next<=p)return false;p=next;
    }return true;
}
void hex(const unsigned char* in,uint32_t n,char* out) noexcept {
    constexpr const char* digits="0123456789abcdef";
    for(uint32_t i=0;i<n;++i){out[2*i]=digits[in[i]>>4];out[2*i+1]=digits[in[i]&15];}out[2*n]=0;
}
DWORD WINAPI writer(void*) {
    uint64_t bytes=0;bool io=true,limit=false,details_done=false;auto began=GetTickCount64();uint32_t next=0;
    auto write=[&](const char* s){auto n=DWORD(strlen(s));if(bytes+n>byte_limit){limit=true;return;}
        DWORD done{};if(!WriteFile(file,s,n,&done,nullptr)||done!=n)io=false;bytes+=done;};
    LARGE_INTEGER frequency{};QueryPerformanceFrequency(&frequency);char line[1800]{};
    snprintf(line,sizeof(line),"# filename_only_prototype=1 context_register_guard=1 qpc_frequency=%llu capacity=%u sampling=first32_then64_plus_snapshot_changes max_bytes=%llu max_ms=%llu\n",
        static_cast<unsigned long long>(frequency.QuadPart),capacity,static_cast<unsigned long long>(byte_limit),static_cast<unsigned long long>(time_limit));write(line);
    write("route,call,thread,qpc,return_site,function_token,output_token,key_hex,carrier,snapshot_valid,track,serial,revision,position,length,terminated,output_length,output_hex\n");
    auto last=began;
    for(;;){
        auto state=publication.load(std::memory_order_acquire);
        if(state&&state!=1)break;
        auto count=journal.reserved.load(std::memory_order_acquire);
        while(next<capacity&&next<count&&journal.slots[next].ready.load(std::memory_order_acquire)&&io&&!limit&&!details_done){
            const auto& r=journal.slots[next++].value;char key[33]{},out[513]{};hex(r.key,16,key);hex(r.output,r.length,out);
            snprintf(line,sizeof(line),"%u,%llu,%u,%llu,%llx,%llx,%llx,%s,%u,%u,%u,%llu,%llu,%llu,%llu,%u,%u,%s\n",r.route,
                static_cast<unsigned long long>(r.call),r.thread,static_cast<unsigned long long>(r.ticks),
                static_cast<unsigned long long>(r.return_site),static_cast<unsigned long long>(r.function_token),static_cast<unsigned long long>(r.output_token),key,
                unsigned(r.carrier),unsigned(r.snapshot_valid),r.snapshot.track,static_cast<unsigned long long>(r.snapshot.serial),
                static_cast<unsigned long long>(r.snapshot.revision),static_cast<unsigned long long>(r.snapshot.position),static_cast<unsigned long long>(r.snapshot.length),unsigned(r.terminated),r.length,out);write(line);
            if(bytes>=detail_limit){details_done=true;write("# UI detailed budget reached; aggregate coverage continues\n");}
        }
        auto now=GetTickCount64();
        if(now-last>=1000){snprintf(line,sizeof(line),"# counts now=%llu rows=%llu reserved=%llu dropped=%llu publication=%u replaced=%llu\n",
            static_cast<unsigned long long>(journal.calls[1].load()),static_cast<unsigned long long>(journal.calls[2].load()),
            static_cast<unsigned long long>(journal.reserved.load()),static_cast<unsigned long long>(journal.dropped.load()),state,static_cast<unsigned long long>(title_binding.replaced.load()));write(line);last=now;}
        if(!io||limit||now-began>=time_limit||(journal.engine&&journal.engine->stop_requested()))break;
        Sleep(100);
    }
    journal.enabled.store(false,std::memory_order_release);
    snprintf(line,sizeof(line),"# capture_end publication=%u io_ok=%u byte_limit=%u elapsed_ms=%llu engine_stop=%u now=%llu rows=%llu consumed=%u dropped=%llu native_drained=unknown clean_session=unknown\n",publication.load(),unsigned(io),unsigned(limit),
        static_cast<unsigned long long>(GetTickCount64()-began),unsigned(journal.engine&&journal.engine->stop_requested()),
        static_cast<unsigned long long>(journal.calls[1].load()),static_cast<unsigned long long>(journal.calls[2].load()),next,static_cast<unsigned long long>(journal.dropped.load()));
    if(io&&!limit)write(line);CloseHandle(file);file=INVALID_HANDLE_VALUE;return 0;
}
}
bool start(unsigned char* base,bytebeat::playlist::Engine* engine,const unsigned char* carrier,bytebeat::SupermoonCore* core,const FilenameTitles* filenames) noexcept {
    // Existing activation has already verified the entire executable file. Also
    // pin loaded renderer/formatter bytes and the isolated aligned callable cell.
    for(const auto& p:profile::ranges)if(!image_range(base,p.rva,p.size)||!digest(base+p.rva,p.size,p.sha))return false;
    for(const auto& p:time_profile::ranges)if(!image_range(base,p.rva,p.size)||!digest(base+p.rva,p.size,p.sha))return false;
    for(const auto& p:register_profile::ranges)if(!image_range(base,p.rva,p.size)||!digest(base+p.rva,p.size,p.sha))return false;
    for(auto rva:{0x4a50bd0u,0x4a47228u})if(!image_range(base,rva,8))return false;
    if(!image_range(base,profile::slot,8)||!image_range(base,profile::library_slot,8))return false;
    auto cell=reinterpret_cast<void* volatile*>(base+profile::slot);
    auto expected=base+profile::formatter;
    if(*cell!=expected||*reinterpret_cast<void**>(base+profile::library_slot)!=expected)return false;
    if(!image_range(base,profile::personal_keys,256))return false;
    title_binding.filenames=filenames;
    title_binding.core=core;title_binding.personal_keys=reinterpret_cast<uintptr_t>(base)+profile::personal_keys;
    journal.original=reinterpret_cast<Formatter>(expected);journal.engine=engine;
    journal.now_return=reinterpret_cast<uintptr_t>(base)+profile::now_return;
    journal.row_return=reinterpret_cast<uintptr_t>(base)+profile::row_return;memcpy(journal.carrier,carrier,16);
    wchar_t path[32768]{};DWORD n=GetModuleFileNameW(nullptr,path,32768);
    if(!n||n>=32768)return false;auto slash=wcsrchr(path,L'\\');if(!slash)return false;
    constexpr wchar_t suffix[]=L"Supermoon\\supermoon-ui-filename-latest.csv";
    if(size_t(slash-path)+1+sizeof(suffix)/sizeof(wchar_t)>32768)return false;wcscpy(slash+1,suffix);
    file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    // Enable before starting the writer: an immediate writer failure may disable
    // capture and must not be undone by a later startup store.
    journal.enabled.store(true,std::memory_order_release);
    HANDLE thread=CreateThread(nullptr,0,writer,nullptr,0,nullptr);
    if(!thread){journal.enabled=false;CloseHandle(file);file=INVALID_HANDLE_VALUE;return false;}
    // All state remains resident even if publication/protection restoration fails.
    if(!time::prepare(base,engine,core,carrier,filenames)){journal.enabled=false;publication=2;CloseHandle(thread);return false;}
    auto result=redirect(cell,expected,reinterpret_cast<void*>(&SupermoonTitleTestBridge));
    if(result==Redirect::changed){if(!time::publish(base))result=Redirect::refused;}else time::cancel();
    publication.store(result==Redirect::changed?1:result==Redirect::refused?2:3,std::memory_order_release);
    title_binding.enabled.store(result==Redirect::changed,std::memory_order_release);
    if(result!=Redirect::changed)journal.enabled.store(false,std::memory_order_release);
    CloseHandle(thread);return result==Redirect::changed;
}
#ifdef UI_OBSERVER_SELFTEST_BUILD
HANDLE test_writer(const wchar_t* path) noexcept {
    if(file!=INVALID_HANDLE_VALUE)return nullptr;
    file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return nullptr;
    publication=1;
    auto thread=CreateThread(nullptr,0,writer,nullptr,0,nullptr);
    if(!thread){CloseHandle(file);file=INVALID_HANDLE_VALUE;}
    return thread;
}
#endif
}
