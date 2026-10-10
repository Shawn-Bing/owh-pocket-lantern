#pragma once
#include "core/match_observer.hpp"
#include "core/hardware_telemetry.hpp"
#include <algorithm>
namespace owh {
inline std::string observationJson(const MatchSnapshot &s) {
  using hardware_detail::quote;
  std::ostringstream out; out.imbue(std::locale::classic());
  auto flag=[](bool v) { return v ? "true" : "false"; };
  out << "{\"session_id\":" << quote(s.sessionId) << ",\"revision\":" << quote(std::to_string(s.revision))
      << ",\"mode\":" << quote(workflowModeName(s.mode))
      << ",\"automation_state\":" << quote(automationStateName(s.automation))
      << ",\"running\":" << flag(s.running) << ",\"game_pid\":" << s.game.pid
      << ",\"game_created\":" << quote(std::to_string(s.game.created))
      << ",\"state\":" << quote(matchStateName(s.state))
      << ",\"confidence\":" << quote(matchConfidenceName(s.confidence))
      << ",\"source\":" << quote(matchSourceName(s.source))
      << ",\"last_confirmed_at_ms\":" << quote(std::to_string(s.lastConfirmedAt))
      << ",\"last_observed_at_ms\":" << quote(std::to_string(s.lastObservedAt))
      << ",\"last_reliable_visual_at_ms\":" << quote(std::to_string(s.lastReliableVisualAt))
      << ",\"process_observed_at_ms\":" << quote(std::to_string(s.processObservedAt))
      << ",\"page\":" << quote(matchPageName(s.page))
      << ",\"window\":{\"minimized\":" << flag(s.window.minimized)
      << ",\"occluded\":" << flag(s.window.occluded) << ",\"foreground\":" << flag(s.window.foreground)
      << ",\"background_input\":" << flag(s.window.backgroundInput)
      << "},\"last_error\":" << quote(s.lastError) << ",\"evidence_ttl_ms\":" << s.evidenceTtlMs
      << ",\"process_ttl_ms\":" << s.processTtlMs << '}';
  return out.str();
}
inline std::string telemetryV2Json(std::uint32_t pid, std::uint64_t created, std::uint64_t sequence,
    std::uint64_t sentTick, const std::string &sentAt, const MatchSnapshot &match, const HardwareSnapshot &hardware) {
  using hardware_detail::quote;
  std::ostringstream out; out.imbue(std::locale::classic());
  out << "{\"agent_pid\":" << pid << ",\"agent_created\":" << quote(std::to_string(created))
      << ",\"sequence\":" << quote(std::to_string(sequence)) << ",\"sent_at\":" << quote(sentAt)
      << ",\"sent_at_ms\":" << quote(std::to_string(sentTick))
      << ",\"observation\":" << observationJson(match) << ",\"hardware\":" << hardwareJson(hardware) << '}';
  return out.str();
}
// Status changes coalesce at 2s. Renewal sends require genuinely newer visual
// evidence or a new sample; repeating a heartbeat never creates freshness.
class TelemetrySchedule {
  std::uint64_t lastSent{}, lastConfirmed{}, lastVisual{}, lastProcess{}, lastHardwareSequence{};
  std::string previousObservation;
  static std::string changeKey(MatchSnapshot s) {
    s.lastConfirmedAt=s.lastObservedAt=s.lastReliableVisualAt=s.processObservedAt=0;
    return observationJson(s);
  }
public:
  bool due(std::uint64_t now, bool dirty, const MatchSnapshot &match, const HardwareSnapshot &hardware) const {
    if (!lastSent) return true;
    if (now < lastSent || now-lastSent < 2000) return false;
    const auto elapsed=now-lastSent;
    if (dirty || previousObservation!=changeKey(match) || elapsed>=30000) return true;
    const auto visualRenew=std::clamp<std::uint64_t>(match.evidenceTtlMs*2/3,2000,30000);
    if (match.state==MatchState::InMatch && match.lastConfirmedAt>lastConfirmed && elapsed>=visualRenew) return true;
    if (match.state==MatchState::OutOfMatch && match.lastReliableVisualAt>lastVisual && elapsed>=visualRenew) return true;
    const auto processRenew=std::clamp<std::uint64_t>(match.processTtlMs*2/3,2000,30000);
    if (match.state==MatchState::Offline && match.processObservedAt>lastProcess && elapsed>=processRenew) return true;
    const bool available=hardware.cpuTemperature.value || hardware.cpuPower.value || hardware.gpuPower.value;
    const auto hardwareRenew=std::clamp<std::uint64_t>(hardware.staleAfterMs*2/3,2000,30000);
    return available && hardware.sequence!=lastHardwareSequence && elapsed>=hardwareRenew;
  }
  void sent(std::uint64_t now, const MatchSnapshot &match, const HardwareSnapshot &hardware) {
    lastSent=now; lastConfirmed=match.lastConfirmedAt; lastHardwareSequence=hardware.sequence;
    lastVisual=match.lastReliableVisualAt; lastProcess=match.processObservedAt;
    previousObservation=changeKey(match);
  }
};
} // namespace owh
