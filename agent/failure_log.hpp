#pragma once
#include "agent/platform.hpp"
#include <deque>
namespace owh {
inline bool failureState(const std::string &state,const std::wstring &detail) {
  if(state=="UNKNOWN") return detail.find(L"不可用")!=std::wstring::npos || detail.find(L"失败")!=std::wstring::npos;
  if(state=="RECOVERING") return detail.rfind(L"手动请求",0)!=0;
  for(auto name:{"INPUT_BLOCKED","BACKGROUND_INPUT_BLOCKED","CAPTURE_FAILED","WINDOW_HUNG",
      "GAME_NOT_RUNNING","NEEDS_CONFIG","NEEDS_USER","NEEDS_CALIBRATION","AMBIGUOUS","LOGIN_STUCK"})
    if(state==name) return true;
  return false;
}
class FailureLog {
  std::mutex mutex_;
  std::wstring directory_,storageError_;
  std::deque<std::wstring> lines_;
  unsigned long long revision_{};
  static constexpr std::size_t MaxLines=200;
public:
  void open(const std::wstring &directory) {
    std::lock_guard lock(mutex_);directory_=directory;lines_.clear();storageError_.clear();
    std::ifstream input(std::filesystem::path(directory)/L"failures.log",std::ios::binary);
    if(input) {
      input.seekg(0,std::ios::end);const auto size=input.tellg();
      if(size>std::streamoff(128*1024)) {input.seekg(size-std::streamoff(128*1024));std::string skip;std::getline(input,skip);}
      else input.seekg(0);
      std::string line;while(std::getline(input,line)) {lines_.push_back(wide(line));if(lines_.size()>MaxLines) lines_.pop_front();}
    }
    ++revision_;
  }
  void record(const std::string &state,const std::wstring &detail,DWORD pid) {
    if(!failureState(state,detail)) return;
    SYSTEMTIME t{};GetLocalTime(&t);wchar_t stamp[64]{};
    swprintf_s(stamp,L"%04u-%02u-%02u %02u:%02u:%02u",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);
    auto clean=detail;for(auto &ch:clean) if(ch==L'\r'||ch==L'\n'||ch==L'\t') ch=L' ';
    auto line=std::wstring(stamp)+L" ["+wide(state)+L"] PID "+std::to_wstring(pid)+L" | "+clean;
    std::lock_guard lock(mutex_);lines_.push_back(line);if(lines_.size()>MaxLines) lines_.pop_front();++revision_;
    if(directory_.empty()) return;
    try {
      auto path=std::filesystem::path(directory_)/L"failures.log";
      if(std::filesystem::exists(path) && std::filesystem::file_size(path)>1024*1024) {
        auto archive=std::filesystem::path(directory_)/(L"failures-"+std::to_wstring(t.wYear)+L"-"+
          std::to_wstring(GetTickCount64())+L"-"+std::to_wstring(GetCurrentProcessId())+L".log");
        std::filesystem::rename(path,archive);
      }
      std::ofstream output(path,std::ios::binary|std::ios::app);output<<utf8(line)<<'\n';output.close();
      if(!output) throw std::runtime_error("write failed");
      storageError_.clear();
    } catch(...) {storageError_=L"失败日志无法写入磁盘；以下记录暂存内存。\r\n";}
  }
  std::pair<unsigned long long,std::wstring> snapshot() {
    std::lock_guard lock(mutex_);auto text=storageError_;
    for(auto it=lines_.rbegin();it!=lines_.rend();++it) text+=*it+L"\r\n";
    if(text.empty()) text=L"尚无失败记录。发生异常时会自动记录时间和具体原因。";
    return {revision_,text};
  }
};
} // namespace owh
