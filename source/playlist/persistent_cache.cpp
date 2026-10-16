// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include "persistent_cache.hpp"
#include "mp3_reader.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <set>
#include <stdexcept>
namespace bytebeat::playlist {
namespace {
void need(bool v,const char* m){if(!v)throw std::runtime_error(m);}
struct Handle {
    HANDLE h=INVALID_HANDLE_VALUE;
    explicit Handle(HANDLE value):h(value){need(h!=INVALID_HANDLE_VALUE,"Cache/source unavailable or in use");}
    ~Handle(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}
};
struct Hash {
    BCRYPT_ALG_HANDLE a{};BCRYPT_HASH_HANDLE h{};
    Hash(){need(BCryptOpenAlgorithmProvider(&a,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0,"Hash provider unavailable");
        if(BCryptCreateHash(a,&h,nullptr,0,nullptr,0,0)<0){BCryptCloseAlgorithmProvider(a,0);a=nullptr;throw std::runtime_error("Hash creation failed");}}
    ~Hash(){if(h)BCryptDestroyHash(h);if(a)BCryptCloseAlgorithmProvider(a,0);}
    void add(const void* p,size_t n){need(n<=UINT32_MAX&&BCryptHashData(h,reinterpret_cast<PUCHAR>(const_cast<void*>(p)),ULONG(n),0)>=0,"Hash update failed");}
    std::string finish(){unsigned char b[32]{};need(BCryptFinishHash(h,b,32,0)>=0,"Hash finish failed");std::string s;for(auto v:b){s+="0123456789abcdef"[v>>4];s+="0123456789abcdef"[v&15];}return s;}
};
std::string hash_bytes(const std::string& s){Hash h;h.add(s.data(),s.size());return h.finish();}
std::string hash_handle(HANDLE f,uint64_t& counted){LARGE_INTEGER zero{};need(SetFilePointerEx(f,zero,nullptr,FILE_BEGIN)!=0,"Hash seek failed");Hash h;std::array<unsigned char,65536> b{};DWORD n{};
    for(;;){need(ReadFile(f,b.data(),DWORD(b.size()),&n,nullptr)!=0,"Hash read failed");if(!n)break;h.add(b.data(),n);counted+=n;}return h.finish();}
std::shared_ptr<Handle> pin(const std::filesystem::path& p){return std::make_shared<Handle>(CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,nullptr));}
bool digest(const std::string& s){return s.size()==64&&s.find_first_not_of("0123456789abcdef")==std::string::npos;}
struct Identity {uint64_t size{},time{},id{};uint32_t volume{};};
Identity identity(HANDLE f){BY_HANDLE_FILE_INFORMATION i{};need(GetFileInformationByHandle(f,&i)!=0,"Source identity unavailable");return {(uint64_t(i.nFileSizeHigh)<<32)|i.nFileSizeLow,(uint64_t(i.ftLastWriteTime.dwHighDateTime)<<32)|i.ftLastWriteTime.dwLowDateTime,(uint64_t(i.nFileIndexHigh)<<32)|i.nFileIndexLow,i.dwVolumeSerialNumber};}
bool same(Identity a,Identity b){return a.size==b.size&&a.time==b.time&&a.id==b.id&&a.volume==b.volume;}
struct Meta {unsigned recipe{};Identity source;uint64_t frames{};double peak{},gain{};std::string path_hash,source_hash,pcm_hash,settings_hash;NormalizationResult normalization;};
std::string serialize(const Meta& m){std::ostringstream f;f<<std::setprecision(17)<<"SPCACHE "<<m.recipe<<'\n'<<m.path_hash<<'\n'<<m.source.size<<' '<<m.source.time<<' '<<m.source.id<<' '<<m.source.volume<<'\n'<<m.source_hash<<'\n'<<m.frames<<' '<<m.peak<<' '<<m.gain<<'\n'<<m.pcm_hash<<'\n'<<m.settings_hash<<'\n'<<m.normalization.enabled<<' '<<m.normalization.defined<<' '<<m.normalization.limited<<' '<<m.normalization.input_lufs<<' '<<m.normalization.output_lufs<<' '<<m.normalization.true_peak<<'\n';auto s=f.str();return s+hash_bytes(s)+'\n';}
Meta parse(const std::filesystem::path& p){need(std::filesystem::file_size(p)<=1024,"Oversized cache metadata");std::ifstream f(p,std::ios::binary);need(bool(f),"Cache metadata unavailable");std::string s{std::istreambuf_iterator<char>(f),{}};
    need(s.size()>=65&&s.back()=='\n',"Truncated cache metadata");auto body=s.substr(0,s.size()-65);need(hash_bytes(body)==s.substr(s.size()-65,64),"Corrupt cache metadata");
    std::istringstream in(body);std::string magic,extra;Meta m;in>>magic>>m.recipe>>m.path_hash>>m.source.size>>m.source.time>>m.source.id>>m.source.volume>>m.source_hash>>m.frames>>m.peak>>m.gain>>m.pcm_hash>>m.settings_hash>>m.normalization.enabled>>m.normalization.defined>>m.normalization.limited>>m.normalization.input_lufs>>m.normalization.output_lufs>>m.normalization.true_peak;
    need(bool(in)&&!(in>>extra)&&magic=="SPCACHE"&&m.recipe==cache_recipe&&digest(m.path_hash)&&digest(m.source_hash)&&digest(m.pcm_hash)&&m.frames&&m.frames<=uint64_t(INT64_MAX)/4&&std::isfinite(m.peak)&&m.peak>=0&&std::isfinite(m.gain)&&m.gain>0&&m.gain<=1000000&&digest(m.settings_hash)&&std::isfinite(m.normalization.input_lufs)&&std::isfinite(m.normalization.output_lufs)&&std::isfinite(m.normalization.true_peak)&&m.normalization.true_peak>=0,"Incompatible cache metadata");return m;}
void publish(const std::filesystem::path& p,const std::string& s){auto temp=p;temp+=L".pending";
    Handle f(CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));DWORD n{};
    need(WriteFile(f.h,s.data(),DWORD(s.size()),&n,nullptr)&&n==s.size()&&FlushFileBuffers(f.h),"Cache metadata write/flush failed");
    // Move needs the write handle closed first.
    CloseHandle(f.h);f.h=INVALID_HANDLE_VALUE;
    need(MoveFileExW(temp.c_str(),p.c_str(),MOVEFILE_WRITE_THROUGH)!=0,"Cache metadata publication failed");}
bool owned(const std::filesystem::path& p){auto name=p.filename().u8string();return name.size()>65&&digest(name.substr(0,64))&&(name.substr(64)==".pcm"||name.substr(64)==".meta"||name.substr(64)==".pcm.raw"||name.substr(64)==".pcm.tmp"||name.substr(64)==".meta.pending");}
bool remove_file(const std::filesystem::path& p,CachedLibrary& out){std::error_code ec;if(std::filesystem::remove(p,ec)){++out.removed;return true;}return false;}
struct Entry {std::filesystem::path pcm,meta;uint64_t bytes{};std::filesystem::file_time_type age;};
void checkpoint(const CacheOptions& o,const char* s){if(o.checkpoint)o.checkpoint(s);}
}
CachedLibrary prepare_cached_library(const std::filesystem::path& music,const std::filesystem::path& root,const CacheOptions& options){
    const auto wav_settings_hash=hash_bytes(normalization_identity(options.normalization));
    CachedLibrary out;auto& lib=out.library;lib.cache_directory=root/"v2";
    auto found=discover(music);lib.diagnostics=std::move(found.diagnostics);
    try {
        for(auto p=std::filesystem::absolute(lib.cache_directory);!p.empty();){auto a=GetFileAttributesW(p.c_str());need(a==INVALID_FILE_ATTRIBUTES||!(a&FILE_ATTRIBUTE_REPARSE_POINT),"Cache ancestry contains a reparse point");auto parent=p.parent_path();if(parent==p)break;p=parent;}
        std::filesystem::create_directories(lib.cache_directory);
        Handle lock(CreateFileW((lib.cache_directory/"manager.lock").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr));
        // The quota covers the ENTIRE reserved Cache root, including previous
        // private run caches. Legacy/unknown files may be referenced by an older
        // candidate, so count them but never infer they are safe to delete.
        uint64_t legacy=0;size_t legacy_count=0;
        for(std::filesystem::recursive_directory_iterator it(root),end;it!=end;++it){
            need(++legacy_count<=40001&&it.depth()<=8,"Legacy cache scan limit exceeded");auto p=it->path();auto a=GetFileAttributesW(p.c_str());
            need(a!=INVALID_FILE_ATTRIBUTES&&!(a&FILE_ATTRIBUTE_REPARSE_POINT),"Legacy cache contains a reparse point");
            if(it.depth()==0&&!_wcsicmp(p.filename().c_str(),L"v2")){it.disable_recursion_pending();continue;}
            if(it->is_regular_file()){auto n=it->file_size();need(n<=UINT64_MAX-legacy,"Legacy cache size overflow");legacy+=n;}
        }
        std::vector<Entry> entries;uint64_t total=legacy;size_t count=0;
        // A namespace this manager alone owns; do not follow reparse points or
        // remove unknown user files. Limit directory work and fail closed.
        auto attr=GetFileAttributesW(lib.cache_directory.c_str());need(attr!=INVALID_FILE_ATTRIBUTES&&!(attr&FILE_ATTRIBUTE_REPARSE_POINT),"Cache root is a reparse point");
        for(auto& file:std::filesystem::directory_iterator(lib.cache_directory)){
            need(++count<=40001,"Cache directory entry limit exceeded");auto p=file.path();
            auto a=GetFileAttributesW(p.c_str());need(a!=INVALID_FILE_ATTRIBUTES&&!(a&FILE_ATTRIBUTE_REPARSE_POINT)&&file.is_regular_file(),"Invalid managed cache entry");
            auto n=file.file_size();need(n<=UINT64_MAX-total,"Cache size overflow");total+=n;
            // Unknown regular files consume quota but are never removed. Unknown
            // directories/reparse points refuse growth rather than hide usage.
            if(!owned(p))continue;
            auto suffix=p.filename().u8string().substr(64);
            if(suffix!=".pcm"&&suffix!=".meta"){if(remove_file(p,out))total-=n;continue;}
            if(suffix==".meta"&&!std::filesystem::exists(p.parent_path()/(p.stem().wstring()+L".pcm"))){if(remove_file(p,out))total-=n;}
        }
        // Account remaining complete/orphan PCM pairs for quota and oldest first.
        for(auto& file:std::filesystem::directory_iterator(lib.cache_directory)){auto p=file.path();if(!owned(p)||p.extension()!=L".pcm")continue;auto meta=p;meta.replace_extension(L".meta");uint64_t n=file.file_size();if(std::filesystem::exists(meta))n+=std::filesystem::file_size(meta);entries.push_back({p,meta,n,std::filesystem::exists(meta)?std::filesystem::last_write_time(meta):file.last_write_time()});}
        std::sort(entries.begin(),entries.end(),[](auto& a,auto& b){return a.age<b.age;});
        auto evict_entry=[&](Entry& e){
            if(!e.bytes||!remove_file(e.pcm,out))return;
            uint64_t removed=e.bytes;if(!e.meta.empty()&&std::filesystem::exists(e.meta)&&!remove_file(e.meta,out))removed-=std::filesystem::file_size(e.meta);
            need(removed<=total,"Cache accounting invariant");total-=removed;e.bytes=0;
        };
        auto evict=[&](uint64_t reserve,const std::filesystem::path& keep){
            if(reserve>options.bytes)return false;
            for(auto& e:entries){if(total<=options.bytes-reserve)break;if(!e.bytes||e.pcm==keep)continue;
                // Delete PCM first. Active/referenced Windows pins veto deletion;
                // metadata then remains intact. No waiting, force or sharing bypass.
                evict_entry(e);
            }
            return total<=options.bytes-reserve;
        };
        // Pin the current library's valid cache hits BEFORE evicting anything so
        // a lower quota cannot destroy later songs discovered in this same scan.
        struct Source {std::filesystem::path path,pcm,meta;std::shared_ptr<Handle> hold;Identity id;std::string key,hash;bool hit{};Prepared prepared;std::shared_ptr<Handle> lease;std::string settings;};
        std::vector<Source> sources;sources.reserve(found.paths.size());
        for(auto& path:found.paths){try{
            auto settings_hash=is_mp3(path)?hash_bytes(normalization_identity(options.normalization)+"/"+mp3_recipe):wav_settings_hash;
            auto canonical=std::filesystem::weakly_canonical(path).wstring();Hash key_hash;key_hash.add(canonical.data(),canonical.size()*sizeof(wchar_t));auto key=key_hash.finish();
            auto hold=pin(path);auto id=identity(hold->h);
            // Reject malformed/unsupported/oversized containers before reading
            // their entire contents for SHA. The source pin spans both checks.
            {Wav header(path);}
            auto source_hash=hash_handle(hold->h,out.hashed_bytes);
            // Content addressed by path+recipe+full content. Timestamp/file ID
            // changes also invalidate metadata, even if content is unchanged.
            auto entry_key=hash_bytes(key+source_hash+std::to_string(cache_recipe)+settings_hash);auto pcm=lib.cache_directory/(entry_key+".pcm"),meta=lib.cache_directory/(entry_key+".meta");
            Source s{path,pcm,meta,std::move(hold),id,key,source_hash,false,{},nullptr,settings_hash};
            try {auto m=parse(meta);need(m.settings_hash==settings_hash&&m.normalization.enabled==options.normalization.enabled&&m.path_hash==key&&m.source_hash==source_hash&&same(m.source,id),"Source identity changed");auto lease=pin(pcm);need(identity(lease->h).size==m.frames*4&&hash_handle(lease->h,out.hashed_bytes)==m.pcm_hash,"Cache integrity mismatch");
                s.hit=true;s.prepared={pcm,m.frames,m.peak,m.gain,m.source_hash};s.prepared.normalization=m.normalization;s.lease=std::move(lease);
            }catch(const std::exception&){}s.hold.reset();sources.push_back(std::move(s));
        }catch(const std::exception& error){lib.diagnostics.push_back(path.filename().u8string()+": "+error.what());}}
        // Only one source handle during scanning/preparation, plus PCM leases.
        // Reopen misses and verify the snapshot before DSP; prepare hashes again
        // under its own write/delete-denying pin before the pair is published.
        for(auto& s:sources)s.hold.reset();
        // Empty diagnostics are the current obsolete-entry eviction gate.
        // Accepted C1: interrupted enumeration can silently truncate discovery,
        // so this gate can evict otherwise valid decoded pairs and force rebuilds.
        // It is not a proven discovery-complete signal; original songs stay intact.
        if(lib.diagnostics.empty()){std::set<std::filesystem::path> current;for(auto& s:sources)current.insert(s.pcm);for(auto& e:entries)if(!current.count(e.pcm))evict_entry(e);}
        evict(0,{});
        for(auto& s:sources){try{
            if(s.hit){++out.reused;std::error_code ec;std::filesystem::last_write_time(s.meta,std::filesystem::file_time_type::clock::now(),ec);}
            else {
                s.hold=pin(s.path);need(same(identity(s.hold->h),s.id),"Source changed after discovery");
                // Remove invalid/orphan pairs only if no other library references
                // them. Their old sizes count against the total until removed.
                if(std::filesystem::exists(s.pcm)){auto n=std::filesystem::file_size(s.pcm);need(remove_file(s.pcm,out),"Invalid cache is referenced/in use");total-=n;}
                if(std::filesystem::exists(s.meta)){auto n=std::filesystem::file_size(s.meta);need(remove_file(s.meta,out),"Invalid metadata unavailable");total-=n;}
                for(auto& e:entries)if(e.pcm==s.pcm)e.bytes=0;
                uint64_t required;{Wav wav(s.path);auto f=wav.format();required=((f.frames*24000+f.rate-1)/f.rate)*4;}
                need(required<=UINT64_MAX/2-2048&&evict(required*2+2048,s.pcm),"Cache quota exhausted (referenced files retained)");
                ULARGE_INTEGER free{};need(GetDiskFreeSpaceExW(lib.cache_directory.c_str(),&free,nullptr,nullptr)&&free.QuadPart>=required*2+2048,"Insufficient free cache space");
                checkpoint(options,"before_prepare");s.prepared=prepare(s.path,s.pcm,options.normalization);checkpoint(options,"pcm_published");
                need(s.prepared.frames*4==required&&s.prepared.source_sha256==s.hash&&same(identity(s.hold->h),s.id),"Source changed during preparation");
                // Flush PCM before atomically publishing checked metadata.
                // Recovery validates a complete pair or treats partial output as
                // rebuildable; this is not a guarantee against every power/OS failure.
                {Handle write(CreateFileW(s.pcm.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr));need(FlushFileBuffers(write.h)!=0,"PCM durable flush failed");}
                s.lease=pin(s.pcm);auto pcm_hash=hash_handle(s.lease->h,out.hashed_bytes);checkpoint(options,"before_metadata");
                publish(s.meta,serialize({cache_recipe,s.id,s.prepared.frames,s.prepared.measured_reconstructed_peak,s.prepared.gain,s.key,s.hash,pcm_hash,s.settings,s.prepared.normalization}));checkpoint(options,"metadata_published");
                total+=required+std::filesystem::file_size(s.meta);++out.rebuilt;s.hold.reset();
            }
            s.prepared.lease=s.lease;
            lib.tracks.push_back(std::move(s.prepared));lib.sources.push_back(s.path);out.leases.push_back(std::move(s.lease));
        }catch(const std::exception& error){
            lib.diagnostics.push_back(s.path.filename().u8string()+": "+error.what());s.lease.reset();s.hold.reset();
            // Reconcile both bytes AND evictable files after an interrupted
            // publication. New orphans/pending outputs were absent from the
            // initial inventory. Windows leases still veto their deletion.
            total=legacy;entries.clear();count=0;
            for(auto& file:std::filesystem::directory_iterator(lib.cache_directory)){
                need(++count<=40001,"Cache directory entry limit exceeded");auto p=file.path();auto a=GetFileAttributesW(p.c_str());
                need(a!=INVALID_FILE_ATTRIBUTES&&!(a&FILE_ATTRIBUTE_REPARSE_POINT)&&file.is_regular_file(),"Invalid managed cache entry");
                auto n=file.file_size();need(n<=UINT64_MAX-total,"Cache size overflow");total+=n;
                if(!owned(p))continue;
                if(p.extension()==L".pcm"){
                    auto meta=p;meta.replace_extension(L".meta");auto pair=n;
                    if(std::filesystem::exists(meta)){auto m=std::filesystem::file_size(meta);need(m<=UINT64_MAX-pair,"Cache size overflow");pair+=m;}
                    entries.push_back({p,meta,pair,std::filesystem::exists(meta)?std::filesystem::last_write_time(meta):file.last_write_time()});
                }else if(p.extension()!=L".meta"||!std::filesystem::exists(p.parent_path()/(p.stem().wstring()+L".pcm"))){
                    entries.push_back({p,{},n,file.last_write_time()});
                }
            }
            std::sort(entries.begin(),entries.end(),[](auto& a,auto& b){return a.age<b.age;});
        }}
        evict(0,{});out.disk_bytes=total;
        if(total>options.bytes)lib.diagnostics.push_back("Cache exceeds configured quota: referenced or unmanaged entries retained; no growth allowed");
    }catch(const std::exception& error){lib.diagnostics.push_back(std::string("Cache unavailable: ")+error.what());}
    return out;
}
CacheConfiguration cache_configuration(const std::filesystem::path& mod){
    CacheConfiguration c{mod/"Music",{}};auto ini=mod/"playlist.ini";wchar_t value[32768]{};
    GetPrivateProfileStringW(L"Playlist",L"CacheMiB",L"3072",value,32768,ini.c_str());
    wchar_t* end{};auto amount=wcstoull(value,&end,10);need(end!=value&&!*end&&amount>=16&&amount<=65536,"CacheMiB must be 16..65536");c.cache.bytes=amount*1024*1024;
    auto n=GetPrivateProfileStringW(L"Playlist",L"Music",L"Music",value,32768,ini.c_str());need(n&&n<32767,"Invalid Music configuration");std::filesystem::path path(value);c.music=path.is_absolute()?path:mod/path;c.cache.normalization=normalization_configuration(mod);return c;
}
}
