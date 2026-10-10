#pragma once
#include "core/match_observer.hpp"
#include "core/semantic.hpp"

namespace owh {
// Called BEFORE automation's observe()/prepare() mutate sessions. A Ready
// template, room.active(), and deathmatch.trusted() deliberately do not appear:
// each can be established by the very frame whose provenance is being checked.
inline MatchEvidence matchEvidence(const Frame &frame, const SemanticObservation &semantic,
    const CNObservation &builtin, WorkflowMode mode, MatchProcess process,
    const CNJoinGate &join, const CNRoomStart &explicitStart,
    const DeathmatchSession &deathmatch, Millis now, bool ambiguous = false) {
  MatchEvidence result;
  if (ambiguous) { result.error = "ambiguous_page"; return result; }
  const auto &observed = semantic ? semantic.observation : builtin;
  if (!observed || observed.step.launcher) return result;
  result.page = observed.page;
  const bool explicitSource = explicitStart.trusted(process.pid, process.created, now);
  if (mode == WorkflowMode::CustomRoom && observed.page == CNPage::Room) {
    // A named lobby proves the target mode, but is not a live match HUD.
    if (semantic && !semantic.liveRoomHud && semantic.evidence.size() >= 2) {
      result.kind = MatchVisualKind::OutOfMatch; result.source = MatchSource::TargetTitle;
      result.error.clear(); return result;
    }
    if (semantic && semantic.liveRoomHud && combatHud(frame)) {
      result.kind = MatchVisualKind::InMatch;
      if (join.awaitingRoom(now)) result.source = MatchSource::TargetJoin;
      else if (explicitSource) result.source = MatchSource::ExplicitStart;
      result.error.clear(); return result;
    }
    result.error = "room_hud_unverified"; return result;
  }
  if (mode == WorkflowMode::Deathmatch && observed.page == CNPage::Combat) {
    if (semantic && combatHud(frame)) {
      result.kind = MatchVisualKind::InMatch;
      if (semantic.verifiedDeathmatch && deathmatchScoreboard(frame)) result.source = MatchSource::DeathmatchScoreboard;
      else if (explicitSource) result.source = MatchSource::ExplicitStart;
      result.error.clear(); return result;
    }
    result.error = "combat_hud_unverified"; return result;
  }
  if (mode == WorkflowMode::Deathmatch && semantic &&
      (observed.page == CNPage::Queue || observed.page == CNPage::Hero) &&
      (semantic.verifiedDeathmatch || deathmatch.matchRequested()))
    result.source = MatchSource::DeathmatchQueue;
  if (explicitSource && (observed.page == CNPage::Hero || observed.page == CNPage::Respawn))
    result.source = MatchSource::ExplicitStart;
  switch (observed.page) {
  case CNPage::None: return result;
  case CNPage::Hero: case CNPage::Joining: case CNPage::Respawn:
    result.kind = MatchVisualKind::Transition; result.error = "transition_page"; return result;
  case CNPage::Room: case CNPage::Combat:
    result.error = "wrong_mode_page"; return result;
  case CNPage::Menu: case CNPage::Queue:
    result.kind = MatchVisualKind::OutOfMatch; result.error.clear(); return result;
  default:
    result.kind = MatchVisualKind::OutOfMatch; result.revokeSource = true;
    result.error.clear(); return result;
  }
}
} // namespace owh
