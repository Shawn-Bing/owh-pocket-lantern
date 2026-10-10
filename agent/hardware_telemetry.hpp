#pragma once
#include "core/hardware_telemetry.hpp"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdio>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>
namespace owh {
inline std::string telemetryUtcNow() {
  SYSTEMTIME t{}; GetSystemTime(&t); char b[40]{};
  std::snprintf(b,sizeof(b),"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",
      unsigned(t.wYear),unsigned(t.wMonth),unsigned(t.wDay),unsigned(t.wHour),
      unsigned(t.wMinute),unsigned(t.wSecond),unsigned(t.wMilliseconds));
  return b;
}
inline HardwareSnapshot hardwareFailureAttempt(HardwareSnapshot value) {
  if (!value.sequence) {
    value.sampledTick=GetTickCount64(); value.sequence=value.sampledTick;
    value.sampledAt=telemetryUtcNow();
  }
  return value;
}
// The network/automation threads only copy a bounded snapshot. All filesystem
// operations run on this independent reader; all native sensor calls run in
// the separate bridge process. Stop never waits on disk or third-party code.
class HardwareCacheReader {
  struct Shared {
    std::mutex mutex;
    std::condition_variable wake;
    std::atomic<bool> stopping{};
    HardwareSnapshot snapshot = unavailableHardware("bridge_not_running");
  };
  std::shared_ptr<Shared> shared;
  static HardwareSnapshot read(const std::wstring &path) {
    if (path.size() < 3 || path[1] != L':' || path[2] != L'\\')
      return unavailableHardware("cache_path_not_local");
    wchar_t root[] = {path[0], L':', L'\\', 0};
    if (GetDriveTypeW(root) == DRIVE_REMOTE) return unavailableHardware("cache_path_not_local");
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                              nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (file == INVALID_HANDLE_VALUE) return unavailableHardware("bridge_cache_unavailable");
    struct Close { HANDLE value; ~Close() { CloseHandle(value); } } close{file};
    BY_HANDLE_FILE_INFORMATION info{};
    LARGE_INTEGER size{};
    if (GetFileType(file) != FILE_TYPE_DISK || !GetFileInformationByHandle(file,&info) ||
        (info.dwFileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY)) ||
        !GetFileSizeEx(file,&size) || size.QuadPart <= 0 || size.QuadPart > 16384)
      return unavailableHardware("bridge_cache_invalid");
    wchar_t resolved[32768]{};
    auto length = GetFinalPathNameByHandleW(file,resolved,32768,FILE_NAME_NORMALIZED);
    if (!length || length >= 32768 || std::wstring(resolved).rfind(L"\\\\?\\UNC\\",0) == 0)
      return unavailableHardware("cache_path_not_local");
    std::string content(std::size_t(size.QuadPart),'\0');
    DWORD count{};
    if (!ReadFile(file,content.data(),DWORD(content.size()),&count,nullptr) || count != content.size())
      return unavailableHardware("bridge_cache_read_failed");
    HardwareSnapshot result;
    return parseHardware(content,result) ? freshHardware(std::move(result),GetTickCount64())
                                         : unavailableHardware("bridge_cache_invalid");
  }
public:
  HardwareCacheReader() = default;
  HardwareCacheReader(const HardwareCacheReader &) = delete;
  HardwareCacheReader &operator=(const HardwareCacheReader &) = delete;
  ~HardwareCacheReader() { stop(); }
  void start(std::wstring path) {
    stop();
    auto state = shared = std::make_shared<Shared>();
    std::thread([state,path=std::move(path)] {
      while (!state->stopping) {
        HardwareSnapshot next;
        try { next = read(path); } catch (...) { next = unavailableHardware("bridge_cache_read_failed"); }
        {
          std::lock_guard lock(state->mutex);
          state->snapshot = hardwareFailureAttempt(std::move(next));
        }
        std::unique_lock lock(state->mutex);
        state->wake.wait_for(lock,std::chrono::seconds(1),[&] { return state->stopping.load(); });
      }
    }).detach();
  }
  void stop() {
    if (shared) { shared->stopping = true; shared->wake.notify_all(); shared.reset(); }
  }
  HardwareSnapshot snapshot(std::uint64_t now) const {
    if (!shared) return hardwareFailureAttempt(unavailableHardware("bridge_not_running"));
    std::lock_guard lock(shared->mutex);
    return hardwareFailureAttempt(freshHardware(shared->snapshot,now));
  }
};

inline std::wstring sensorCommandArg(const std::wstring &value) {
  std::wstring out=L"\"";
  unsigned slashes=0;
  for(wchar_t c:value) {
    if(c==L'\\') { ++slashes; continue; }
    if(c==L'"') { out.append(slashes*2+1,L'\\'); out+=c; slashes=0; }
    else { out.append(slashes,L'\\'); slashes=0; out+=c; }
  }
  out.append(slashes*2,L'\\'); return out+L'"';
}
// Own child only: closing this private job ends the bridge even if a native
// provider hangs. It never opens/terminates FanControl or any game process.
class HardwareBridgeProcess {
  HANDLE job{};
public:
  HardwareBridgeProcess() = default;
  HardwareBridgeProcess(const HardwareBridgeProcess &) = delete;
  HardwareBridgeProcess &operator=(const HardwareBridgeProcess &) = delete;
  ~HardwareBridgeProcess() { if(job) CloseHandle(job); }
  bool start(const std::wstring &exe,const std::wstring &cache,unsigned interval,
             const std::wstring &lhm,const std::wstring &gpu) {
    if (job || exe.empty() || exe.rfind(L"\\\\",0)==0) return false;
    DWORD attributes=GetFileAttributesW(exe.c_str());
    if(attributes==INVALID_FILE_ATTRIBUTES || attributes&FILE_ATTRIBUTE_DIRECTORY) return false;
    job=CreateJobObjectW(nullptr,nullptr);
    if(!job) return false;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))) {
      CloseHandle(job); job=nullptr; return false;
    }
    std::wstring command=sensorCommandArg(exe)+L" --output "+sensorCommandArg(cache)+
        L" --interval-ms "+std::to_wstring(interval);
    if(!lhm.empty()) command+=L" --lhm-dir "+sensorCommandArg(lhm);
    if(!gpu.empty()) command+=L" --gpu-uuid "+sensorCommandArg(gpu);
    STARTUPINFOW startup{}; startup.cb=sizeof(startup); PROCESS_INFORMATION process{};
    if(!CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,
          CREATE_NO_WINDOW|CREATE_SUSPENDED,nullptr,nullptr,&startup,&process)) {
      CloseHandle(job); job=nullptr; return false;
    }
    const bool assigned=AssignProcessToJobObject(job,process.hProcess)!=FALSE;
    const bool started=assigned && ResumeThread(process.hThread)!=DWORD(-1);
    if(!started) TerminateProcess(process.hProcess,1);
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    if(!started) { CloseHandle(job); job=nullptr; }
    return started;
  }
};

} // namespace owh
