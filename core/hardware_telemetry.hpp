#pragma once
#include "core/json.hpp"
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <sstream>
#include <iomanip>
#include <locale>
namespace owh {
struct HardwareReading {
  std::optional<double> value;
  std::string unit, source, sensor, reason = "not_sampled";
};
struct HardwareSnapshot {
  std::string sampledAt;
  std::uint64_t sampledTick{}, sequence{};
  unsigned intervalMs = 5000, staleAfterMs = 15000;
  HardwareReading cpuTemperature{{}, "celsius", "lhm-pawnio", "", "not_sampled"};
  HardwareReading cpuPower{{}, "watts", "lhm-pawnio", "", "not_sampled"};
  HardwareReading gpuPower{{}, "watts", "nvml", "", "not_sampled"};
};
inline HardwareSnapshot unavailableHardware(const std::string &reason) {
  HardwareSnapshot result;
  result.cpuTemperature.reason = result.cpuPower.reason = result.gpuPower.reason = reason;
  return result;
}
inline HardwareSnapshot freshHardware(HardwareSnapshot result, std::uint64_t now) {
  if (!result.sequence) return result;
  if (now < result.sampledTick || now - result.sampledTick >= result.staleAfterMs) {
    const auto reason = now < result.sampledTick ? "sample_future" : "sample_stale";
    for (auto *r : {&result.cpuTemperature, &result.cpuPower, &result.gpuPower}) {
      r->value.reset(); r->reason = reason;
    }
    if (now < result.sampledTick) {
      result.sampledTick=0; result.sequence=0; result.sampledAt.clear();
    }
  }
  return result;
}
namespace hardware_detail {
inline bool utcDate(const std::string &s) {
  if (s.size()!=24 || s[4]!='-' || s[7]!='-' || s[10]!='T' || s[13]!=':' ||
      s[16]!=':' || s[19]!='.' || s[23]!='Z') return false;
  for (std::size_t i=0;i<s.size();++i) {
    if (i==4||i==7||i==10||i==13||i==16||i==19||i==23) continue;
    if (s[i]<'0'||s[i]>'9') return false;
  }
  auto n=[&](std::size_t p,std::size_t count) { return std::stoi(s.substr(p,count)); };
  auto year=n(0,4), month=n(5,2), day=n(8,2);
  if (year<1601 || month<1 || month>12 || n(11,2)>23 || n(14,2)>59 || n(17,2)>59) return false;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if (year%4==0 && (year%100!=0 || year%400==0)) days[1]=29;
  return day>=1 && day<=days[month-1];
}
inline bool decimal(const Json *j, std::uint64_t &result) {
  if (!j || !j->is(Json::Kind::String) || j->text.empty() || j->text.size() > 20) return false;
  result = 0;
  for (char c : j->text) {
    if (c < '0' || c > '9' || result > (std::numeric_limits<std::uint64_t>::max() - (c-'0')) / 10) return false;
    result = result * 10 + (c-'0');
  }
  return true;
}
inline bool text(const Json *j, std::string &out, std::size_t max) {
  if (!j || !j->is(Json::Kind::String) || j->text.size() > max) return false;
  for (unsigned char c : j->text) if (c < 32) return false;
  out = j->text; return true;
}
inline bool reading(const Json *j, HardwareReading &out, const char *unit, double maximum) {
  if (!j || !j->is(Json::Kind::Object) || !text(j->find("unit"), out.unit, 16) || out.unit != unit ||
      !text(j->find("source"), out.source, 64) || !text(j->find("sensor"), out.sensor, 200) ||
      !text(j->find("reason"), out.reason, 120)) return false;
  auto v = j->find("value");
  if (!v) return false;
  if (v->is(Json::Kind::Null)) { out.value.reset(); return !out.reason.empty(); }
  if (!v->is(Json::Kind::Number) || !std::isfinite(v->number) || v->number < 0 || v->number > maximum ||
      out.source.empty() || out.sensor.empty() || !out.reason.empty()) return false;
  out.value = v->number; return true;
}
inline std::string quote(const std::string &value) {
  std::string out = "\"";
  for (unsigned char c : value) {
    if (c == '"' || c == '\\') out += '\\';
    if (c >= 32) out += char(c);
  }
  return out + '"';
}
inline void writeReading(std::ostream &out, const HardwareReading &r) {
  out << "{\"value\":";
  if (r.value && std::isfinite(*r.value)) out << std::fixed << std::setprecision(3) << *r.value;
  else out << "null";
  out << ",\"unit\":" << quote(r.unit) << ",\"source\":" << quote(r.source)
      << ",\"sensor\":" << quote(r.sensor) << ",\"reason\":" << quote(r.reason) << '}';
}
}
inline bool parseHardware(const std::string &source, HardwareSnapshot &out) {
  using namespace hardware_detail;
  if (source.empty() || source.size() > 16384) return false;
  Json j; std::string error; HardwareSnapshot result;
  if (!parseJson(source, j, error) || !j.is(Json::Kind::Object) ||
      !text(j.find("sampled_at"), result.sampledAt, 40) || !utcDate(result.sampledAt) ||
      !decimal(j.find("sampled_tick_ms"), result.sampledTick) ||
      !decimal(j.find("sequence"), result.sequence) || !result.sequence) return false;
  auto interval = j.find("interval_ms"), stale = j.find("stale_after_ms");
  if (!interval || !stale || !interval->is(Json::Kind::Number) || !stale->is(Json::Kind::Number) ||
      !std::isfinite(interval->number) || !std::isfinite(stale->number) ||
      interval->number < 3000 || interval->number > 60000 || std::floor(interval->number) != interval->number ||
      stale->number != 3 * interval->number) return false;
  result.intervalMs = unsigned(interval->number); result.staleAfterMs = unsigned(stale->number);
  if (!reading(j.find("cpu_package_temperature"), result.cpuTemperature, "celsius", 200) ||
      !reading(j.find("cpu_package_power"), result.cpuPower, "watts", 3000) ||
      !reading(j.find("gpu_power"), result.gpuPower, "watts", 3000)) return false;
  out = std::move(result); return true;
}
inline std::string hardwareJson(const HardwareSnapshot &value) {
  using namespace hardware_detail;
  std::ostringstream out; out.imbue(std::locale::classic());
  out << "{\"sampled_at\":" << quote(value.sampledAt)
      << ",\"sampled_tick_ms\":" << quote(std::to_string(value.sampledTick))
      << ",\"sequence\":" << quote(std::to_string(value.sequence))
      << ",\"interval_ms\":" << value.intervalMs << ",\"stale_after_ms\":" << value.staleAfterMs
      << ",\"cpu_package_temperature\":";
  writeReading(out, value.cpuTemperature); out << ",\"cpu_package_power\":";
  writeReading(out, value.cpuPower); out << ",\"gpu_power\":";
  writeReading(out, value.gpuPower); out << '}'; return out.str();
}
} // namespace owh
