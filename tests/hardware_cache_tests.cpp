#include "agent/hardware_telemetry.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool v,const char *why) { if(!v) throw std::runtime_error(why); }
int main() {
  wchar_t temp[MAX_PATH]{}; GetTempPathW(MAX_PATH,temp);
  auto path=std::filesystem::path(temp)/(L"owh-hardware-"+std::to_wstring(GetCurrentProcessId())+L".json");
  try {
    auto write=[&](const std::string &value) { std::ofstream f(path,std::ios::binary|std::ios::trunc); f<<value; };
    auto wait=[&](HardwareCacheReader &reader,const std::string &reason) {
      const auto start=GetTickCount64();
      while(GetTickCount64()-start<3000) {
        auto result=reader.snapshot(GetTickCount64());
        if(result.gpuPower.reason==reason) return result;
        Sleep(20);
      }
      throw std::runtime_error("cache observation timeout");
    };
    HardwareSnapshot sample; sample.sampledAt=telemetryUtcNow();
    sample.sampledTick=GetTickCount64()+60000; sample.sequence=1;
    sample.cpuTemperature.reason=sample.cpuPower.reason="cpu_access_denied";
    sample.gpuPower={40,"watts","nvml","GPU-test",""};
    write(hardwareJson(sample));
    HardwareCacheReader reader; reader.start(path.wstring());
    auto future=wait(reader,"sample_future");
    HardwareSnapshot parsed;
    require(future.sampledTick<=GetTickCount64() && future.sequence && parseHardware(hardwareJson(future),parsed),
        "future cache becomes a legal failure attempt");
    require(!future.gpuPower.value && future.cpuPower.reason=="sample_future","future cache has no trusted readings");
    reader.stop(); sample.sampledAt="not-a-date"; sample.sampledTick=GetTickCount64();
    write(hardwareJson(sample)); reader.start(path.wstring());
    auto malformed=wait(reader,"bridge_cache_invalid");
    require(parseHardware(hardwareJson(malformed),parsed),"malformed cache does not invalidate v2 envelope");
    reader.stop(); write(std::string(16385,'x')); reader.start(path.wstring());
    auto large=wait(reader,"bridge_cache_invalid");
    require(!large.gpuPower.value,"bounded cache read");
    auto stopped=GetTickCount64(); reader.stop();
    require(GetTickCount64()-stopped<200,"stop does not wait on provider or filesystem");
    require(sensorCommandArg(L"C:\\test folder\\")==L"\"C:\\test folder\\\\\"","command path trailing slash");
    DeleteFileW(path.c_str());
    std::cout<<"hardware cache isolation/future/malformed/size tests passed\n";
    return 0;
  } catch(const std::exception &e) { DeleteFileW(path.c_str()); std::cerr<<e.what()<<'\n'; return 1; }
}
