#pragma once
#include "agent/platform.hpp"
#include "core/match_evidence.hpp"

namespace owh {
inline MatchProcess matchProcess(const Process &process) { return {process.pid,process.created}; }
inline MatchWindow matchWindow(const Process &process, bool backgroundInput) {
  MatchWindow result;
  result.backgroundInput=backgroundInput;
  if (!process.window || !IsWindow(process.window)) return result;
  result.minimized=IsIconic(process.window)!=FALSE;
  result.foreground=foreground(process.window);
  // Focus and occlusion are different facts; a remote overlay can cover a
  // focused game. Reuse the existing cheap visibility guard, never a new OCR.
  result.occluded=captureAvailability(process.window,true).code=="occluded";
  return result;
}
// Every completed worker tick either commits its already-captured visual result
// or records why observation was unavailable. Early continue paths (standby,
// desktop lock, recovery and background input) therefore cannot preserve green.
class MatchObservationTick {
  MatchObserver &observer_;
  MatchTicket ticket_;
  const Process &game_;
  const std::atomic<bool> &background_;
  bool submitted_{};
  Process target_{};
  bool targetOverridden_{};
public:
  std::string error;
  MatchObservationTick(MatchObserver &observer, MatchTicket ticket, const Process &game,
                       const std::atomic<bool> &background)
      : observer_(observer),ticket_(ticket),game_(game),background_(background) {}
  ~MatchObservationTick() {
    if (submitted_ || !ticket_) return;
    const auto now=GetTickCount64();
    const auto snapshot=observer_.snapshot(now);
    const auto &observedGame=targetOverridden_ ? target_ : game_;
    const auto window=matchWindow(observedGame,background_.load());
    auto reason=error;
    if (reason.empty()) {
      if (window.minimized) reason="minimized";
      else if (window.occluded) reason="occluded";
      else if (window.backgroundInput) reason="background_observation_paused";
      else if (observedGame.window && !window.foreground) reason="not_foreground";
      else reason=std::string("observation_paused:")+automationStateName(snapshot.automation);
    }
    observer_.unavailable(ticket_,snapshot.game,now,window,reason);
  }
  void target(const Process &process) { target_=process; targetOverridden_=true; }
  void useScannedWindow() { targetOverridden_=false; }
  void visual(const Process &target, const std::wstring &gameName, const Frame &frame,
      const SemanticObservation &semantic, const CNObservation &builtin,
      WorkflowMode mode, const CNJoinGate &join, const CNRoomStart &explicitStart,
      const DeathmatchSession &deathmatch, Millis capturedAt, std::uint64_t sequence,
      bool ambiguous) {
    const auto now=GetTickCount64();
    const auto available=captureAvailability(target.window,false);
    if (!sameProcess(target,gameName)) { error="process_identity_changed"; return; }
    if (available.code!="ok") { error=available.code; return; }
    const auto evidence=matchEvidence(frame,semantic,builtin,mode,matchProcess(target),
        join,explicitStart,deathmatch,now,ambiguous);
    submitted_=observer_.observe(ticket_,matchProcess(target),capturedAt,sequence,now,
        matchWindow(target,background_.load()),evidence);
    if (!submitted_) error="visual_evidence_rejected";
  }
};
} // namespace owh
