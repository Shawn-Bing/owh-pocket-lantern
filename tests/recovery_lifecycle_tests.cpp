#include "agent/platform.hpp"
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool value,const char *message) {if (!value) throw std::runtime_error(message);}
int main() {
  try {
    const std::string failed="00:49:02.968\ts s\r\n00:49:03.976\tport 34000\r\n00:51:06.315\tneac start process failed 1223\r\n";
    auto failure=parseNeacLaunchFailure(failed);
    require(failure && failure->code==ERROR_CANCELLED && failure->loaderPid==34000,"Actual loader cancellation identifies code and PID");
    require(!parseNeacLaunchFailure(failed+"01:01:01.000\ts s\n"),"A new launch removes the previous error");
    require(!parseNeacLaunchFailure(failed+"01:01:01.000\tport 5\n"),"A newer loader must not inherit failure from another PID");
    require(!parseNeacLaunchFailure("01:00:00.000\tneac start process failed 1223\n"),"Failure needs this log's loader identity");
    require(!parseNeacLaunchFailure("01:00:00.000\tport 5\n01:00:01.000\tneac start process failed 1223"),"Incomplete appended records wait for completion");
    require(!parseNeacLaunchFailure("01:00:00.000\tport 5\n01:00:01.000\tneac start process failed 1223 run command\n"),"Log text cannot be treated as executable instructions or a valid numeric result");
    require(!parseNeacLaunchFailure("01:00:00.000\tport 5\n01:00:01.000\tneac start process failed 0\n"),"Zero is not an error");
    require(neacLaunchRequiresUser(ERROR_CANCELLED) && neacLaunchRequiresUser(ERROR_ACCESS_DENIED) &&
            neacLaunchRequiresUser(ERROR_ELEVATION_REQUIRED),"Cancelled or denied authorization is not an automatic retry");
    require(!neacLaunchRequiresUser(10060),"A connection failure remains eligible for unattended retry");
    GameExitWait exit;
    require(exit.observe(true,1)==GameExitWait::State::Idle,"No exit transaction exists before an accepted request");
    exit.begin(1000);require(exit.observe(true,30999)==GameExitWait::State::Waiting,"Accepted slow exit keeps recovery alive");
    require(exit.observe(false,40000)==GameExitWait::State::Exited,"Late process exit resumes recovery");
    require(exit.observe(true,91000)==GameExitWait::State::Expired,"A genuinely stuck exit has a bounded wait");
    exit.pause(120000);require(exit.observe(true,91000)==GameExitWait::State::Waiting,"Standby and secure desktop do not consume exit wait");
    exit.reset();require(!exit.active(),"Stop or generation reset cancels the old wait");
    const auto folder=std::filesystem::current_path()/L"recovery-test-data"/std::to_wstring(GetCurrentProcessId());
    std::filesystem::create_directories(folder/L"log");
    const auto path=folder/L"log"/L"NeacLoader.log";
    auto write=[&](const std::string &text) {std::ofstream file(path,std::ios::binary|std::ios::trunc);file<<text;file.close();require(bool(file),"Write owned log fixture");};
    write(failed);
    Config c;c.demo=false;c.neacPath=(folder/L"OWNeacClient.exe").wstring();
    NeacLaunchMonitor monitor;monitor.begin(c);
    require(!monitor.failure(),"The real file's pre-existing cancellation is ignored after a new launch");
    write(failed);
    // Advance the owned log timestamp to model a fresh complete worker reply.
    FILETIME time{};GetSystemTimeAsFileTime(&time);
    ULONGLONG ticks=(ULONGLONG(time.dwHighDateTime)<<32)|time.dwLowDateTime;ticks+=100000;
    time={DWORD(ticks),DWORD(ticks>>32)};
    Handle log(CreateFileW(path.c_str(),FILE_WRITE_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr));
    require(log.h!=INVALID_HANDLE_VALUE && SetFileTime(log.h,nullptr,nullptr,&time),"Set fresh owned fixture timestamp");
    failure=monitor.failure();require(failure && failure->code==1223,"Fresh cancellation is observed immediately, before launch timeout or retry");
    require(neacLaunchFailureText(*failure).find(L"1223")!=std::wstring::npos,"Cancellation status preserves exact error code");
    monitor.reset();require(!monitor.failure(),"Successful game, stop or recovery clears prior launch evidence");
    c.demo=true;monitor.begin(c);require(!monitor.failure(),"Demo never reads a real launch log");
    c.demo=false;c.neacPath=L"OWNeacClient.exe";monitor.begin(c);require(!monitor.failure(),"A relative path cannot supply trusted launch evidence");
    std::cout<<"Current launch cancellation, stale logs, log rotation, numeric records, coordinate-free diagnostics and delayed exit policy passed (owned log only)\n";
    return 0;
  } catch(const std::exception &error) {std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
