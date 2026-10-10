#pragma once
#include "core/automation_state.hpp"
#include "core/deathmatch.hpp"
#include <mutex>
#include <string>

namespace owh {
enum class MatchState { InMatch, OutOfMatch, Unverified, Offline };
enum class MatchConfidence { None, Process, Visual, RepeatedVisual };
enum class MatchSource { None, TargetTitle, TargetJoin, ExplicitStart, DeathmatchQueue, DeathmatchScoreboard };
inline const char *matchStateName(MatchState v) {
  switch (v) {
  case MatchState::InMatch: return "IN_MATCH";
  case MatchState::OutOfMatch: return "OUT_OF_MATCH";
  case MatchState::Offline: return "OFFLINE";
  default: return "UNVERIFIED";
  }
}
inline const char *matchConfidenceName(MatchConfidence v) {
  switch (v) {
  case MatchConfidence::Process: return "PROCESS";
  case MatchConfidence::Visual: return "VISUAL";
  case MatchConfidence::RepeatedVisual: return "REPEATED_VISUAL";
  default: return "NONE";
  }
}
inline const char *matchSourceName(MatchSource v) {
  switch (v) {
  case MatchSource::TargetTitle: return "TARGET_TITLE";
  case MatchSource::TargetJoin: return "TARGET_JOIN";
  case MatchSource::ExplicitStart: return "EXPLICIT_START";
  case MatchSource::DeathmatchQueue: return "DEATHMATCH_QUEUE";
  case MatchSource::DeathmatchScoreboard: return "DEATHMATCH_SCOREBOARD";
  default: return "NONE";
  }
}
inline const char *workflowModeName(WorkflowMode v) {
  return v == WorkflowMode::Deathmatch ? "DEATHMATCH" : "CUSTOM_ROOM";
}
inline const char *matchPageName(CNPage v) {
  switch (v) {
#define OWH_MATCH_PAGE(name) case CNPage::name: return #name;
  OWH_MATCH_PAGE(None) OWH_MATCH_PAGE(LoginError) OWH_MATCH_PAGE(Home)
  OWH_MATCH_PAGE(Play) OWH_MATCH_PAGE(More) OWH_MATCH_PAGE(Browser)
  OWH_MATCH_PAGE(Join) OWH_MATCH_PAGE(Launch) OWH_MATCH_PAGE(Room)
  OWH_MATCH_PAGE(Menu) OWH_MATCH_PAGE(Login) OWH_MATCH_PAGE(LauncherSelect)
  OWH_MATCH_PAGE(BrowserSearch) OWH_MATCH_PAGE(Queue) OWH_MATCH_PAGE(Hero)
  OWH_MATCH_PAGE(Combat) OWH_MATCH_PAGE(Results) OWH_MATCH_PAGE(Leave)
  OWH_MATCH_PAGE(ConfirmLeave) OWH_MATCH_PAGE(Respawn) OWH_MATCH_PAGE(DeathmatchStart)
  OWH_MATCH_PAGE(Endorse) OWH_MATCH_PAGE(RoomList) OWH_MATCH_PAGE(Joining)
#undef OWH_MATCH_PAGE
  }
  return "None";
}
struct MatchObserverConfig {
  Millis evidenceTtlMs{15000};
  Millis processTtlMs{15000};
  Millis confirmationWindowMs{DeathmatchSession::ConfirmationWindow};
  Millis sourceRetentionMs{CNJoinGate::PendingWindow};
};
struct MatchProcess {
  std::uint32_t pid{};
  std::uint64_t created{};
  bool verified() const { return pid && created; }
  bool operator==(const MatchProcess &) const = default;
};
struct MatchWindow {
  bool minimized{}, occluded{}, foreground{}, backgroundInput{};
  bool operator==(const MatchWindow &) const = default;
};
struct MatchTicket {
  std::uint64_t revision{};
  unsigned generation{};
  explicit operator bool() const { return revision != 0; }
  bool operator==(const MatchTicket &) const = default;
};
enum class MatchVisualKind { Unknown, InMatch, OutOfMatch, Transition };
struct MatchEvidence {
  CNPage page{CNPage::None};
  MatchVisualKind kind{MatchVisualKind::Unknown};
  MatchSource source{MatchSource::None};
  bool revokeSource{};
  std::string error{"unknown_page"};
};
struct MatchSnapshot {
  WorkflowMode mode{WorkflowMode::CustomRoom};
  AutomationState automation{AutomationState::STOPPED};
  MatchProcess game;
  MatchState state{MatchState::Unverified};
  MatchConfidence confidence{MatchConfidence::None};
  MatchSource source{MatchSource::None};
  Millis lastConfirmedAt{}, lastObservedAt{}, lastReliableVisualAt{}, processObservedAt{};
  Millis evidenceTtlMs{}, processTtlMs{};
  CNPage page{CNPage::None};
  MatchWindow window;
  std::string sessionId, lastError{"assistant_stopped"};
  std::uint64_t revision{};
  bool running{};
};

// Output-only consumer: it retains no input/recovery callback or mutable
// automation session. OCR/capture/network work always runs outside this mutex.
class MatchObserver {
  mutable std::mutex mutex_;
  MatchObserverConfig config_;
  std::string instanceId_;
  MatchTicket ticket_;
  MatchSnapshot value_;
  std::uint64_t processSequence_{}, lastFrameSequence_{};
  Millis lastFrameAt_{}, lastCandidateAt_{}, sourceAt_{};
  unsigned candidates_{};
  bool processChecked_{};
  bool accepts(MatchTicket t) const { return t && t == ticket_; }
  static bool fresh(Millis then, Millis now, Millis ttl) {
    return then && now >= then && now - then <= ttl;
  }
  void sessionId() {
    value_.sessionId = instanceId_ + ":" + std::to_string(ticket_.revision) + ":" +
        std::to_string(processSequence_) + ":" + std::to_string(value_.game.pid) + ":" +
        std::to_string(value_.game.created);
  }
  void unverified(const std::string &reason) {
    value_.state = MatchState::Unverified; value_.confidence = MatchConfidence::None;
    value_.lastError = reason; candidates_ = 0; lastCandidateAt_ = 0;
  }
  bool sourceMatchesMode(MatchSource source) const {
    return source == MatchSource::ExplicitStart ||
        (value_.mode == WorkflowMode::CustomRoom
          ? source == MatchSource::TargetTitle || source == MatchSource::TargetJoin
          : source == MatchSource::DeathmatchQueue || source == MatchSource::DeathmatchScoreboard);
  }
  void clearSource() { value_.source = MatchSource::None; sourceAt_ = 0; }
  void resetEvidence() {
    value_.lastConfirmedAt = value_.lastObservedAt = value_.lastReliableVisualAt = 0;
    value_.page = CNPage::None; value_.window = {};
    lastFrameAt_ = lastFrameSequence_ = 0;
    clearSource(); unverified("awaiting_visual_evidence");
  }
public:
  explicit MatchObserver(std::string instanceId, MatchObserverConfig config = {})
      : config_(config), instanceId_(std::move(instanceId)) {}
  MatchTicket begin(unsigned generation, WorkflowMode mode, Millis now) {
    std::lock_guard lock(mutex_);
    ticket_ = {ticket_.revision + 1, generation};
    value_ = {}; value_.mode = mode; value_.running = true;
    value_.revision = ticket_.revision; value_.automation = AutomationState::DETECTING;
    processChecked_ = false; resetEvidence(); value_.lastObservedAt = now;
    value_.lastError = "awaiting_process_scan"; sessionId(); return ticket_;
  }
  void stop(unsigned generation, Millis now) {
    std::lock_guard lock(mutex_);
    const auto mode = value_.mode;
    ticket_ = {ticket_.revision + 1, generation};
    value_ = {}; value_.mode = mode; value_.revision = ticket_.revision;
    processChecked_ = false; resetEvidence(); value_.lastObservedAt = now;
    value_.lastError = "assistant_stopped"; sessionId();
  }
  MatchTicket ticketFor(unsigned generation) const {
    std::lock_guard lock(mutex_);
    return value_.running && ticket_.generation == generation ? ticket_ : MatchTicket{};
  }
  MatchTicket currentTicket() const { std::lock_guard lock(mutex_); return ticket_; }
  bool automation(MatchTicket t, AutomationState value) {
    std::lock_guard lock(mutex_);
    if (!accepts(t)) return false;
    value_.automation = value; return true;
  }
  bool selectProcess(MatchTicket t, MatchProcess game, Millis now) {
    std::lock_guard lock(mutex_);
    if (!accepts(t) || !value_.running || now < value_.processObservedAt) return false;
    if (!processChecked_ || game != value_.game) {
      value_.game = game; ++processSequence_; resetEvidence(); sessionId();
    }
    processChecked_ = true; value_.processObservedAt = now;
    if (!game.pid) {
      value_.state = MatchState::Offline; value_.confidence = MatchConfidence::Process;
      value_.lastError = "game_process_absent"; clearSource();
    } else if (!game.verified()) unverified("process_identity_unavailable");
    return true;
  }
  bool unavailable(MatchTicket t, MatchProcess game, Millis now,
                   MatchWindow window, const std::string &reason) {
    std::lock_guard lock(mutex_);
    if (!accepts(t) || !value_.running || game != value_.game || now < value_.lastObservedAt) return false;
    value_.lastObservedAt = now; value_.window = window; value_.page = CNPage::None;
    if (value_.state != MatchState::Offline) unverified(reason);
    if (!fresh(sourceAt_, now, config_.sourceRetentionMs)) clearSource();
    return true;
  }
  bool observe(MatchTicket t, MatchProcess game, Millis capturedAt,
               std::uint64_t frameSequence, Millis now, MatchWindow window,
               const MatchEvidence &evidence) {
    std::lock_guard lock(mutex_);
    if (!accepts(t) || !value_.running || !processChecked_ || game != value_.game ||
        !game.verified() || !capturedAt || capturedAt <= lastFrameAt_ ||
        frameSequence <= lastFrameSequence_ || capturedAt < value_.lastObservedAt || now < capturedAt) return false;
    lastFrameAt_ = capturedAt; lastFrameSequence_ = frameSequence;
    value_.lastObservedAt = capturedAt; value_.page = evidence.page; value_.window = window;
    if (!fresh(sourceAt_, now, config_.sourceRetentionMs)) clearSource();
    if (window.minimized || window.occluded || !window.foreground || window.backgroundInput) {
      unverified("capture_unavailable"); return true;
    }
    // Delayed OCR retains the original frame time. A subsequent HUD-shape
    // check cannot turn old text recognition into a new visual observation.
    if (!fresh(capturedAt, now, config_.evidenceTtlMs)) {
      unverified("visual_evidence_expired"); return true;
    }
    if (evidence.revokeSource) clearSource();
    if (evidence.source != MatchSource::None && sourceMatchesMode(evidence.source)) {
      value_.source = evidence.source; sourceAt_ = capturedAt;
    }
    if (evidence.kind == MatchVisualKind::Unknown || evidence.kind == MatchVisualKind::Transition) {
      unverified(evidence.error); return true;
    }
    value_.lastReliableVisualAt = capturedAt;
    if (evidence.kind == MatchVisualKind::OutOfMatch) {
      unverified(""); value_.state = MatchState::OutOfMatch;
      value_.confidence = MatchConfidence::Visual; return true;
    }
    if (evidence.page != (value_.mode == WorkflowMode::CustomRoom ? CNPage::Room : CNPage::Combat)) {
      unverified("wrong_mode_page"); return true;
    }
    if (value_.source == MatchSource::None) {
      unverified("mode_source_unverified"); return true;
    }
    if (!fresh(lastCandidateAt_, capturedAt, config_.confirmationWindowMs)) candidates_ = 0;
    lastCandidateAt_ = capturedAt; candidates_ = std::min(2u, candidates_ + 1);
    sourceAt_ = capturedAt;
    if (candidates_ < 2) {
      value_.state = MatchState::Unverified; value_.confidence = MatchConfidence::Visual;
      value_.lastError = "awaiting_second_visual"; return true;
    }
    value_.state = MatchState::InMatch; value_.confidence = MatchConfidence::RepeatedVisual;
    value_.lastConfirmedAt = capturedAt; value_.lastError.clear(); return true;
  }
  MatchSnapshot snapshot(Millis now) const {
    std::lock_guard lock(mutex_);
    auto result = value_;
    result.evidenceTtlMs = config_.evidenceTtlMs; result.processTtlMs = config_.processTtlMs;
    if (!fresh(sourceAt_, now, config_.sourceRetentionMs)) result.source = MatchSource::None;
    if ((result.state == MatchState::InMatch || result.state == MatchState::OutOfMatch) &&
        !fresh(result.lastReliableVisualAt, now, config_.evidenceTtlMs)) {
      result.state = MatchState::Unverified; result.confidence = MatchConfidence::None;
      result.lastError = "visual_evidence_expired";
    }
    if (result.state == MatchState::Offline && !fresh(result.processObservedAt, now, config_.processTtlMs)) {
      result.state = MatchState::Unverified; result.confidence = MatchConfidence::None;
      result.lastError = "process_evidence_expired";
    }
    return result;
  }
};
} // namespace owh
