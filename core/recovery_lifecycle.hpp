#pragma once
#include "core/core.hpp"
#include <optional>
#include <sstream>

namespace owh {
struct NeacLaunchFailure { unsigned code{},loaderPid{}; };
// Parse only complete numeric loader records. Log contents are diagnostic data,
// never commands. A newer launch record invalidates an older failure.
inline std::optional<NeacLaunchFailure> parseNeacLaunchFailure(const std::string &source) {
  std::optional<NeacLaunchFailure> failure;unsigned pid{};
  std::size_t offset{};
  while (offset<source.size()) {
    const auto end=source.find('\n',offset);if (end==std::string::npos) break;
    auto line=source.substr(offset,end-offset);offset=end+1;
    if (!line.empty() && line.back()=='\r') line.pop_back();
    auto record=line.find('\t');if (record==std::string::npos) continue;
    auto message=line.substr(record+1);
    if (message=="s s") {failure.reset();pid=0;continue;}
    if (message.rfind("port ",0)==0) {
      failure.reset();std::istringstream fields(message.substr(5));unsigned value{};std::string extra;
      pid=(fields>>value && value && !(fields>>extra)) ? value : 0;continue;
    }
    constexpr const char prefix[]="neac start process failed ";
    if (message.rfind(prefix,0)!=0 || !pid) continue;
    std::istringstream fields(message.substr(sizeof(prefix)-1));unsigned code{};std::string extra;
    if (fields>>code && code && code<=65535 && !(fields>>extra)) failure=NeacLaunchFailure{code,pid};
  }
  return failure;
}
// An accepted asynchronous exit request is still in progress until confirmed.
class GameExitWait {
  Millis deadline_{};
public:
  enum class State { Idle,Waiting,Exited,Expired };
  void begin(Millis now) {deadline_=now+90000;}
  void reset() {deadline_=0;}
  bool active() const {return deadline_!=0;}
  void pause(Millis duration) {if (deadline_) deadline_+=duration;}
  State observe(bool sameAlive,Millis now) const {
    if (!deadline_) return State::Idle;
    if (!sameAlive) return State::Exited;
    return now>=deadline_ ? State::Expired : State::Waiting;
  }
};
} // namespace owh
