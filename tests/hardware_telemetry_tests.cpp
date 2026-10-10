#include "core/hardware_telemetry.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
void require(bool test,const char *message) { if(!test) throw std::runtime_error(message); }
int main(int argc,char **argv) {
  using namespace owh;
  if(argc==3 && std::string(argv[1])=="--cache") {
    std::ifstream file(argv[2],std::ios::binary);
    std::string text((std::istreambuf_iterator<char>(file)),{});
    HardwareSnapshot parsed;
    if(!parseHardware(text,parsed)) return 2;
    std::cout<<"real bridge cache accepted by C++ parser\n"; return 0;
  }
  try {
    HardwareSnapshot sample;
    sample.sampledAt="2026-10-10T12:00:00.000Z"; sample.sampledTick=10000; sample.sequence=1;
    sample.cpuTemperature.reason="cpu_access_denied";
    sample.cpuPower.reason="cpu_access_denied";
    sample.gpuPower={39.9,"watts","nvml","GPU-example",""};
    HardwareSnapshot parsed;
    require(parseHardware(hardwareJson(sample),parsed),"valid partial real sample");
    require(!parsed.cpuPower.value && parsed.cpuPower.reason=="cpu_access_denied","missing is not zero");
    require(parsed.gpuPower.value && *parsed.gpuPower.value==39.9,"valid power round trip");
    require(freshHardware(parsed,24999).gpuPower.value.has_value(),"TTL before boundary");
    require(!freshHardware(parsed,25000).gpuPower.value,"TTL boundary clears value");
    require(freshHardware(parsed,9999).gpuPower.reason=="sample_future","future rejected");
    sample.intervalMs=60000; sample.staleAfterMs=180000;
    require(parseHardware(hardwareJson(sample),parsed),"lower frequency permitted");
    sample.staleAfterMs=15000;
    require(!parseHardware(hardwareJson(sample),parsed),"TTL must track interval");
    sample.intervalMs=5000; sample.staleAfterMs=15000;
    sample.cpuPower.reason="";
    require(!parseHardware(hardwareJson(sample),parsed),"null requires diagnostic");
    sample.cpuPower.reason="cpu_access_denied";
    sample.gpuPower.value=std::numeric_limits<double>::infinity();
    require(!parseHardware(hardwareJson(sample),parsed),"nonfinite cannot masquerade as missing without reason");
    sample.gpuPower.value=-1;
    require(!parseHardware(hardwareJson(sample),parsed),"negative power rejected");
    require(!parseHardware(std::string(16385,' '),parsed),"oversized cache rejected");
    require(!parseHardware("{",parsed),"truncated cache rejected");
    sample.gpuPower.value=40; sample.sampledAt="2026-02-30T12:00:00.000Z";
    require(!parseHardware(hardwareJson(sample),parsed),"invalid calendar date rejected");
    sample.sampledAt="2026-10-10T25:00:00.000Z";
    require(!parseHardware(hardwareJson(sample),parsed),"invalid UTC time rejected");
    std::uint64_t number;
    Json value; value.kind=Json::Kind::String; value.text="18446744073709551615";
    require(hardware_detail::decimal(&value,number) && number==std::numeric_limits<std::uint64_t>::max(),"exact uint64 strings");
    value.text="18446744073709551616";
    require(!hardware_detail::decimal(&value,number),"overflow rejected");
    std::cout<<"hardware telemetry tests passed\n"; return 0;
  } catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
}
