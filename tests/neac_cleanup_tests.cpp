#include "agent/platform.hpp"
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool value,const char *reason) {if (!value) throw std::runtime_error(reason);}
struct FakeNeac {
  std::vector<NeacProcessIdentity> processes;
  std::vector<DWORD> ended;
  int gameChecks{};bool game{},appears{},respawn{},denyQuery{},denyEnd{},failEnumeration{},active=true;
  bool enumerate(const wchar_t *name,std::vector<DWORD> &ids) {
    ids.clear();if (failEnumeration) return false;
    if (_wcsicmp(name,L"Overwatch.exe")==0) {
      ++gameChecks;if (game || (appears && gameChecks>=2)) ids.push_back(999);return true;
    }
    for (const auto &process:processes)
      if (_wcsicmp(std::filesystem::path(process.path).filename().c_str(),name)==0) ids.push_back(process.pid);
    return true;
  }
  NeacQueryResult query(DWORD pid,NeacProcessIdentity &identity,std::wstring &why) {
    if (denyQuery) {why=L"query refused";return NeacQueryResult::Denied;}
    for (const auto &process:processes) if (process.pid==pid) {identity=process;return NeacQueryResult::Found;}
    return NeacQueryResult::Gone;
  }
  template<class Active>
  CleanupResult terminate(const NeacProcessIdentity &identity,Active allowed,std::wstring &why) {
    if (!allowed()) return CleanupResult::Cancelled;
    if (denyEnd) {why=L"termination refused";return CleanupResult::Denied;}
    ended.push_back(identity.pid);
    processes.erase(std::remove_if(processes.begin(),processes.end(),[&](const auto &p){return p.pid==identity.pid;}),processes.end());
    if (respawn && std::filesystem::path(identity.path).filename()==L"NeacLoader.exe")
      processes.push_back({77,770,identity.path});
    return CleanupResult::Clear;
  }
};
int main(int argc,char **argv) {
  if (argc==2 && std::string(argv[1])=="--fixture") {Sleep(60000);return 42;}
  try {
    Config c;c.demo=false;c.dryRun=false;c.gameExe=L"Overwatch.exe";c.neacPath=L"D:\\Overwatch\\_retail_\\OWNeacClient.exe";
    auto make=[&] {FakeNeac f;f.processes={{2,20,c.neacPath},{1,10,neacLoaderPath(c.neacPath)}};return f;};
    auto run=[&](FakeNeac &f,std::wstring *why=nullptr){return cleanupNeacWith(c,[&]{return f.active;},f,why);};
    auto f=make();require(run(f)==CleanupResult::Clear && f.ended==std::vector<DWORD>{1,2},"Loader exits before the client and both are confirmed absent");
    f=make();f.processes.erase(f.processes.begin());require(run(f)==CleanupResult::Clear && f.ended==std::vector<DWORD>{1},"Loader-only residue cannot be reported clear without termination");
    f=make();f.processes.pop_back();require(run(f)==CleanupResult::Clear && f.ended==std::vector<DWORD>{2},"Client-only residue remains supported");
    f=make();f.game=true;require(run(f)==CleanupResult::GamePresent && f.ended.empty(),"A live game prevents all cleanup");
    f=make();f.appears=true;require(run(f)==CleanupResult::GamePresent && f.ended.empty(),"Game appearing after discovery prevents termination");
    f=make();f.active=false;require(run(f)==CleanupResult::Cancelled && !f.gameChecks,"Stopped or stale generation cannot enumerate or terminate");
    f=make();f.processes.back().path=L"E:\\OtherGame\\NeacLoader.exe";require(run(f)==CleanupResult::Denied && f.ended.empty(),"Loader from another installation is never terminated");
    f=make();f.denyQuery=true;require(run(f)==CleanupResult::Denied && f.ended.empty(),"Unreadable identity blocks launching over residue");
    f=make();f.denyEnd=true;std::wstring why;require(run(f,&why)==CleanupResult::Denied && why==L"termination refused","Termination rejection stays a failure with a concrete reason");
    f=make();f.respawn=true;require(run(f)==CleanupResult::Denied,"Recreated loader is caught by final enumeration");
    f=make();f.failEnumeration=true;require(run(f)==CleanupResult::Denied && f.ended.empty(),"Enumeration failure is not a clear result");
    f=make();c.dryRun=true;require(run(f)==CleanupResult::Clear && f.ended.empty() && f.processes.size()==2,"Dry run validates paths without terminating");c.dryRun=false;
    const auto path=c.neacPath;c.neacPath=L"OWNeacClient.exe";f=make();require(run(f)==CleanupResult::NeedsPath && f.ended.empty(),"Residual cleanup requires a full configured client path");c.neacPath=path;
    require(sameNeacPath(L"d:\\OVERWATCH\\_retail_\\neacloader.EXE",neacLoaderPath(path)),"Sibling loader path compares case-insensitively");
    require(!sameNeacPath(L"D:\\Overwatch\\_retail_\\NeacProtect.exe",neacLoaderPath(path)),"Other security components are not cleanup targets");
    WindowsNeacCleanup windows;NeacProcessIdentity self;std::wstring detail;
    require(windows.query(GetCurrentProcessId(),self,detail)==NeacQueryResult::Found && self.created &&
            self.pid==GetCurrentProcessId(),"Windows backend obtains a live handle-bound process identity");
    // Only our two copied console fixtures can be enumerated by this adapter.
    // Production query/terminate still recheck handles, creation time and game absence.
    const auto folder=std::filesystem::current_path()/L"neac-test-data"/std::to_wstring(GetCurrentProcessId());
    std::filesystem::create_directories(folder);
    struct Owned {HANDLE process;DWORD pid;std::wstring name;};
    std::vector<Owned> owned;
    for (const auto name:{L"NeacLoader.exe",L"OWNeacClient.exe"}) {
      const auto executable=folder/name;std::filesystem::copy_file(self.path,executable,std::filesystem::copy_options::overwrite_existing);
      auto command=L"\""+executable.wstring()+L"\" --fixture";STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION child{};
      require(CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&child),"Start owned console lifecycle fixture");
      CloseHandle(child.hThread);owned.push_back({child.hProcess,child.dwProcessId,name});
    }
    struct ScopedWindows : WindowsNeacCleanup {
      std::vector<Owned> *owned;
      bool enumerate(const wchar_t *name,std::vector<DWORD> &ids) {
        ids.clear();if (_wcsicmp(name,L"Overwatch.exe")==0) return processIds(name,ids);
        for (const auto &child:*owned) if (_wcsicmp(name,child.name.c_str())==0 && WaitForSingleObject(child.process,0)==WAIT_TIMEOUT) ids.push_back(child.pid);
        return true;
      }
    } scoped;scoped.owned=&owned;
    c.neacPath=(folder/L"OWNeacClient.exe").wstring();
    const auto actual=cleanupNeacWith(c,[]{return true;},scoped,&detail);
    for (const auto &child:owned) {
      if (WaitForSingleObject(child.process,0)==WAIT_TIMEOUT) TerminateProcess(child.process,0);
      require(WaitForSingleObject(child.process,5000)==WAIT_OBJECT_0,"Owned test process exits");CloseHandle(child.process);
    }
    require(actual==CleanupResult::Clear || actual==CleanupResult::GamePresent,"Windows cleanup succeeds or safely refuses a live game");
    std::cout<<"Loader/client ordering, exact installation, cancellation, game races, refusal and respawn policy passed; Windows fixtures="
             <<(actual==CleanupResult::Clear ? "both terminated and confirmed" : "withheld while real game runs")<<" (no user processes ended)\n";
    return 0;
  } catch (const std::exception &error) {std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
