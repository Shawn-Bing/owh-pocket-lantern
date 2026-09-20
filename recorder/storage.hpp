#pragma once
#include "agent/platform.hpp"
#include <map>
#include <set>
#include <iomanip>

namespace owh::rec {
inline constexpr char RecorderVersion[] = "0.1.1";
inline std::string jsonString(const std::string &s) { return "\"" + jsonEscape(s) + "\""; }
inline std::string jsonString(const std::wstring &s) { return jsonString(utf8(s)); }
inline std::string utc() {
  SYSTEMTIME t{}; GetSystemTime(&t);
  char s[40]{};
  std::snprintf(s, sizeof(s), "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ", t.wYear,t.wMonth,t.wDay,
                t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
  return s;
}
inline std::string stamp() {
  return "\"utc\":" + jsonString(utc()) + ",\"tick_ms\":" + std::to_string(GetTickCount64());
}
struct Options {
  std::filesystem::path output = std::filesystem::path(exeDirectory()) / L"recordings";
  bool screenshots{true}, sources{true}, headless{}, stop{}, demo{}, probes{true};
  unsigned seconds{18 * 3600};
};
inline Options parse(const std::vector<std::wstring> &args) {
  Options o;
  for (std::size_t i=1; i<args.size(); ++i) {
    const auto &a=args[i];
    if (a==L"--output" && i+1<args.size()) o.output=args[++i];
    else if (a==L"--seconds" && i+1<args.size()) {
      auto s=args[++i];
      if (s.empty() || s.find_first_not_of(L"0123456789") != std::wstring::npos)
        throw std::runtime_error("Invalid duration");
      auto n=std::stoul(s);
      if (n<1 || n>18*3600) throw std::runtime_error("Duration must be 1..64800 seconds");
      o.seconds=unsigned(n);
    } else if (a==L"--no-screenshots") o.screenshots=false;
    else if (a==L"--no-source-logs") o.sources=false;
    else if (a==L"--no-probes") o.probes=false;
    else if (a==L"--headless") o.headless=true;
    else if (a==L"--stop") o.stop=true;
    else if (a==L"--demo") o.demo=true;
    else throw std::runtime_error("Unknown option");
  }
  o.output=std::filesystem::absolute(o.output).lexically_normal();
  return o;
}
class Store {
  std::mutex mutex_;
  std::uint64_t used_{}, cap_;
public:
  const std::filesystem::path directory;
  std::atomic<bool> capped{}, failed{};
  explicit Store(std::filesystem::path p, std::uint64_t cap=512ULL*1024*1024)
      : cap_(cap), directory(std::move(p)) { std::filesystem::create_directories(directory); }
  bool write(const std::filesystem::path &relative, const void *data, std::size_t count,
             bool append=true) {
    if (relative.empty() || relative.is_absolute()) throw std::runtime_error("Relative file required");
    for (auto &part: relative) if (part==L"..") throw std::runtime_error("Outside session");
    std::lock_guard lock(mutex_);
    if (count>cap_-used_) { capped=true; return false; }
    try {
      auto path=directory/relative;
      std::filesystem::create_directories(path.parent_path());
      std::ofstream f(path,std::ios::binary | (append?std::ios::app:std::ios::trunc));
      f.exceptions(std::ios::badbit|std::ios::failbit);
      f.write(static_cast<const char *>(data),std::streamsize(count));
      f.flush();
      used_+=count;
      return true;
    } catch (...) { failed=true; return false; }
  }
  bool text(const std::filesystem::path &p,const std::string &s,bool append=true) {
    return write(p,s.data(),s.size(),append);
  }
  bool event(const std::string &kind,const std::string &fields="") {
    return text("timeline.jsonl","{"+stamp()+",\"kind\":"+jsonString(kind)+(fields.empty()?"":","+fields)+"}\n");
  }
  std::uint64_t bytes() { std::lock_guard lock(mutex_); return used_; }
};
inline std::filesystem::path sessionPath(const std::filesystem::path &root) {
  SYSTEMTIME t{}; GetLocalTime(&t); wchar_t name[80]{};
  swprintf(name,80,L"%04u%02u%02u-%02u%02u%02u-%lu",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,
           t.wSecond,GetCurrentProcessId());
  auto p=root/name;
  for (int suffix=0;std::filesystem::exists(p);++suffix) p=root/(std::wstring(name)+L"-"+std::to_wstring(suffix));
  return p;
}
struct TailPlan { std::uint64_t offset{}, length{}, skipped{}; bool reset{}; };
inline TailPlan planTail(std::uint64_t previous,std::uint64_t size,bool sameIdentity,
                         std::uint64_t cap=64*1024) {
  bool reset=!sameIdentity || size<previous;
  auto begin=reset?0:previous;
  auto available=size-begin;
  auto count=std::min(available,cap);
  return {size-count,count,available-count,reset};
}
class Tailer {
  struct Cursor { std::uint64_t identity{}, size{}, modified{}; DWORD volume{}; std::string file; };
  std::map<std::wstring,Cursor> cursors_;
  unsigned files_{};
public:
  void read(Store &store,const std::filesystem::path &path) {
    Handle h(CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                         nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if (h.h==INVALID_HANDLE_VALUE) {
      auto error=GetLastError();
      store.event("source_unreadable","\"path\":"+jsonString(path.wstring())+",\"error\":"+std::to_string(error));
      return;
    }
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandle(h.h,&info)) return;
    auto identity=(std::uint64_t(info.nFileIndexHigh)<<32)|info.nFileIndexLow;
    auto size=(std::uint64_t(info.nFileSizeHigh)<<32)|info.nFileSizeLow;
    auto modified=(std::uint64_t(info.ftLastWriteTime.dwHighDateTime)<<32)|info.ftLastWriteTime.dwLowDateTime;
    auto &cursor=cursors_[path.wstring()];
    bool same=cursor.identity==identity && cursor.volume==info.dwVolumeSerialNumber && !cursor.file.empty();
    if (same && size==cursor.size && modified==cursor.modified) return;
    // Same-size rewrites and truncation are new segments too, not "no new data".
    auto plan=planTail(cursor.size,size,same && !(size==cursor.size && modified!=cursor.modified));
    if (plan.reset || cursor.file.empty()) cursor.file="sources/"+std::to_string(++files_)+".log";
    LARGE_INTEGER pos{}; pos.QuadPart=plan.offset;
    if (!SetFilePointerEx(h.h,pos,nullptr,FILE_BEGIN)) return;
    std::vector<char> data(std::size_t(plan.length)); DWORD count{};
    if (plan.length && !ReadFile(h.h,data.data(),DWORD(data.size()),&count,nullptr)) return;
    if (!store.write(cursor.file,count?data.data():"",count)) return;
    store.text("sources.jsonl","{"+stamp()+",\"source\":"+jsonString(path.wstring())+
      ",\"saved\":"+jsonString(cursor.file)+",\"identity\":"+jsonString(std::to_string(identity))+
      ",\"source_offset\":"+std::to_string(plan.offset)+",\"bytes\":"+std::to_string(count)+
      ",\"skipped_bytes\":"+std::to_string(plan.skipped)+",\"reset\":"+(plan.reset?"true":"false")+"}\n");
    cursor.identity=identity; cursor.volume=info.dwVolumeSerialNumber;
    cursor.size=plan.offset+count; cursor.modified=modified;
  }
};
} // namespace owh::rec
