// SPDX-License-Identifier: MIT
#include "playlist/persistent_cache.hpp"
#include "playlist/discovery_io.hpp"
#include "remediation_harness.hpp"
#include <fstream>
#include <map>
using namespace bytebeat::playlist;
using remediation_test::expect;using remediation_test::require;
namespace {
enum Failure {none,first_error,next_error,metadata_error,root_error,empty,vanished};
Failure failure{};unsigned seen{},closed{},root_reads{};
HANDLE WINAPI first(LPCWSTR p,LPWIN32_FIND_DATAW d){
    if(failure==first_error){SetLastError(ERROR_ACCESS_DENIED);return INVALID_HANDLE_VALUE;}
    if(failure==empty||failure==vanished){SetLastError(ERROR_FILE_NOT_FOUND);return INVALID_HANDLE_VALUE;}
    auto h=FindFirstFileW(p,d);if(h!=INVALID_HANDLE_VALUE&&std::filesystem::path(d->cFileName).extension()==L".wav")++seen;return h;
}
BOOL WINAPI next(HANDLE h,LPWIN32_FIND_DATAW d){
    if(failure==next_error&&seen){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    auto r=FindNextFileW(h,d);if(r&&std::filesystem::path(d->cFileName).extension()==L".wav")++seen;return r;
}
DWORD WINAPI attributes(LPCWSTR name){
    auto p=std::filesystem::path(name);
    if(p.filename()==L"Music"){
        ++root_reads;if(failure==root_error||(failure==vanished&&root_reads>1)){SetLastError(ERROR_ACCESS_DENIED);return INVALID_FILE_ATTRIBUTES;}
    }
    if(failure==metadata_error&&p.filename()==L"b.wav"){SetLastError(ERROR_ACCESS_DENIED);return INVALID_FILE_ATTRIBUTES;}
    return GetFileAttributesW(name);
}
BOOL WINAPI close(HANDLE h){++closed;return FindClose(h);}
DiscoveryIO io{first,next,attributes,close,GetLastError};
void wav(const std::filesystem::path& p){
    std::ofstream f(p,std::ios::binary);uint32_t size=36+400,fmt=16,rate=24000,bytes=96000,data=400;uint16_t tag=3,channels=1,align=4,bits=32;float sample=.1f;
    f.write("RIFF",4);f.write((char*)&size,4);f.write("WAVEfmt ",8);f.write((char*)&fmt,4);f.write((char*)&tag,2);f.write((char*)&channels,2);f.write((char*)&rate,4);f.write((char*)&bytes,4);f.write((char*)&align,2);f.write((char*)&bits,2);f.write("data",4);f.write((char*)&data,4);for(unsigned n=0;n<100;++n)f.write((char*)&sample,4);require(bool(f),"owned WAV write");
}
void setup(){std::filesystem::create_directories("Music");for(auto n:{"a.wav","b.wav","c.wav"})wav(std::filesystem::path("Music")/n);}
std::string bytes(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
std::map<std::filesystem::path,std::string> pairs(){std::map<std::filesystem::path,std::string> result;for(auto& p:std::filesystem::directory_iterator("Cache/v2"))if(p.path().extension()==L".pcm"||p.path().extension()==L".meta")result.emplace(p.path(),bytes(p.path()));return result;}
void interrupted(Failure kind){
    setup();{auto cold=prepare_cached_library("Music","Cache");require(cold.rebuilt==3&&cold.discovery_complete,"complete cold cache");}
    auto original=pairs();require(original.size()==6,"three reusable cache pairs");std::map<std::filesystem::path,std::string> music;for(auto n:{"a.wav","b.wav","c.wav"})music[n]=bytes(std::filesystem::path("Music")/n);
    failure=kind;CacheOptions o;o.bytes=1;o.discovery_io=&io;
    {auto partial=prepare_cached_library("Music","Cache",o);expect(!partial.discovery_complete&&!partial.library.diagnostics.empty(),"failure is explicitly incomplete with diagnostic");expect(!partial.removed,"incomplete scan cannot authorize obsolete/quota/orphan removal");expect(pairs()==original,"all omitted and discovered reusable cache bytes retained under quota pressure");}
    failure=none;{auto recovered=prepare_cached_library("Music","Cache");expect(recovered.discovery_complete&&recovered.reused==3&&!recovered.rebuilt,"later clean scan reuses all retained pairs");}
    for(auto& p:music)expect(bytes(std::filesystem::path("Music")/p.first)==p.second,"original song bytes unchanged");
}
void initial(){interrupted(first_error);}void next_fail(){interrupted(next_error);}void metadata(){interrupted(metadata_error);}void root_fail(){interrupted(root_error);}
void complete(){setup();auto d=discover("Music",io);expect(d.complete&&d.paths.size()==3&&d.diagnostics.empty()&&closed==1,"normal end closes handle and completes");std::filesystem::create_directory("Music/nested.wav");wav("Music/Shuffle On.wav");std::ofstream("Music/ignored.txt")<<"sentinel";auto second=discover("Music");expect(second.complete&&second.paths.size()==3,"intentional exclusions do not truncate discovery");}
void valid_empty(){std::filesystem::create_directory("Music");failure=empty;auto d=discover("Music",io);expect(d.complete&&d.paths.empty()&&d.diagnostics.empty(),"initial no-match on existing directory is a complete empty scan");}
void missing_empty(){std::filesystem::create_directory("Music");failure=vanished;auto d=discover("Music",io);expect(!d.complete&&!d.diagnostics.empty(),"missing directory on no-match recheck is incomplete");}
void empty_evicts(){setup();{auto c=prepare_cached_library("Music","Cache");require(c.rebuilt==3,"cold pairs");}for(auto n:{"a.wav","b.wav","c.wav"})std::filesystem::remove(std::filesystem::path("Music")/n);auto d=prepare_cached_library("Music","Cache");expect(d.discovery_complete&&d.removed==6&&pairs().empty(),"genuinely complete empty library still permits obsolete cleanup");}
unsigned synthetic_index{},synthetic_count{};
DWORD WINAPI synthetic_attributes(LPCWSTR p){return std::filesystem::path(p).filename()==L"Music"?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_NORMAL;}
HANDLE WINAPI synthetic_first(LPCWSTR,LPWIN32_FIND_DATAW data){synthetic_index=0;swprintf(data->cFileName,MAX_PATH,L"%u.wav",synthetic_index);return reinterpret_cast<HANDLE>(uintptr_t(1));}
BOOL WINAPI synthetic_next(HANDLE,LPWIN32_FIND_DATAW data){if(++synthetic_index==synthetic_count){SetLastError(ERROR_NO_MORE_FILES);return FALSE;}swprintf(data->cFileName,MAX_PATH,L"%u.wav",synthetic_index);return TRUE;}
BOOL WINAPI synthetic_close(HANDLE){return TRUE;}
void limits(){DiscoveryIO injected{synthetic_first,synthetic_next,synthetic_attributes,synthetic_close,GetLastError};synthetic_count=10000;auto full=discover("Music",injected);expect(full.complete&&full.paths.size()==10000,"exact file limit can be complete");synthetic_count=10001;auto truncated=discover("Music",injected);expect(!truncated.complete&&truncated.paths.size()==10000&&!truncated.diagnostics.empty(),"truncation at file limit is explicitly incomplete");}
}
int main(int argc,char** argv){const remediation_test::Case cases[]={
    {"initial_enumeration_error",initial},{"next_enumeration_error",next_fail},{"entry_metadata_error",metadata},{"root_metadata_error",root_fail},
    {"normal_end_and_exclusions",complete},{"initial_empty_directory",valid_empty},{"no_match_root_recheck",missing_empty},{"complete_empty_eviction",empty_evicts},{"complete_vs_truncated_file_limit",limits}};
    return remediation_test::main(argc,argv,cases);
}
