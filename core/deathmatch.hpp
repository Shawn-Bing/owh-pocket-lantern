#pragma once
#include "core/cn_recovery.hpp"
#include <array>
#include <random>

namespace owh {
enum class WorkflowMode { CustomRoom, Deathmatch };
class DeathmatchSession {
  bool finished_{}, leaving_{};
  bool requested_{}, queued_{}, inMatch_{};
  Millis lastCombat_{}, lastObserved_{}, nextInput_{};
  int combatFrames_{}, endorsementClicks_{};
  bool endorsementAttempted_{}, endorsementClosePending_{};
  Millis endorsementCloseAt_{}, endorsementCloseUntil_{};
  int endorsementCloseAttempts_{};
  std::mt19937 random_;
public:
  // Recognition includes a worker roundtrip. A valid second observation may
  // arrive after 3 seconds on a slower machine; input still requires a fresh
  // current observation, and any non-combat frame clears the confirmation.
  static constexpr Millis ConfirmationWindow = 10000;
  struct KeyStroke { std::uint16_t key{}; Millis duration{}; bool rightClick{}; };
  explicit DeathmatchSession(unsigned seed = 1) : random_(seed) {}
  void reset() {
    finished_ = leaving_ = false;
    requested_ = queued_ = inMatch_ = false;
    lastCombat_ = lastObserved_ = nextInput_ = 0;
    combatFrames_ = endorsementClicks_ = 0;endorsementAttempted_=false;
    cancelEndorsementClose();
  }
  // The second accepted card closes the bounded endorsement transaction. Its
  // title changes to "Thanks", so the final Escape must not require another OCR
  // match. A manual input or a positively observed different page revokes it.
  void cancelEndorsementClose() {
    endorsementClosePending_=false;endorsementCloseAt_=endorsementCloseUntil_=0;
    endorsementCloseAttempts_=0;
  }
  bool endorsementCloseReady(Millis now) const {
    return endorsementClosePending_ && endorsementCloseAttempts_<3 &&
           now>=endorsementCloseAt_ && now<=endorsementCloseUntil_;
  }
  void endorsementCloseAttempted(bool ok,Millis now) {
    if (!endorsementClosePending_) return;
    if (ok) cancelEndorsementClose();
    else { ++endorsementCloseAttempts_;endorsementCloseAt_=now+1000; }
  }
  bool endorsementAttempted() const { return endorsementAttempted_; }
  int endorsementClicks() const { return endorsementClicks_; }
  bool finished() const { return finished_; }
  bool leaving() const { return leaving_; }
  bool trusted() const { return queued_ || inMatch_; }
  void matched() { queued_ = true; finished_ = leaving_ = false; }
  void observe(CNPage page, Millis now) {
    if (page!=CNPage::None && page!=CNPage::Endorse) cancelEndorsementClose();
    if (page == CNPage::Combat) {
      if (!lastObserved_ || now < lastObserved_ || now - lastObserved_ > ConfirmationWindow) combatFrames_ = 0;
      ++combatFrames_;
      lastCombat_ = now;
      finished_ = leaving_ = false;
      inMatch_ = true;endorsementAttempted_=false;endorsementClicks_=0;
    } else {
      combatFrames_ = 0;
      if (page == CNPage::Results || page==CNPage::Endorse) {
        if (!finished_) requested_ = false;
        finished_ = true; queued_ = inMatch_ = false;
        if (page==CNPage::Endorse) endorsementAttempted_=true;
      }
      if (page == CNPage::Queue && requested_) queued_ = true;
      if (page == CNPage::Hero && (queued_ || requested_)) {
        // Backfill can jump from our mode/start click straight to hero select.
        // A missed Queue frame must not strand the subsequent untitled HUD.
        inMatch_ = true; queued_ = true;
      }
      if (page == CNPage::Home || page == CNPage::Play || page == CNPage::Queue ||
          page == CNPage::Hero || page == CNPage::LoginError || page == CNPage::Login) {
        finished_ = leaving_ = false;
        lastCombat_ = 0;
      }
      if (page == CNPage::Home || page == CNPage::Play || page == CNPage::LoginError || page == CNPage::Login)
        requested_ = queued_ = inMatch_ = false;
    }
    lastObserved_ = now;
  }
  void acted(CNPage page, bool ok, Action action = Action::Click, Millis now = 0) {
    if (ok && page==CNPage::Results && action==Action::Endorse) endorsementAttempted_=true;
    if (ok && page==CNPage::Endorse && action==Action::Click) {
      ++endorsementClicks_;
      if (endorsementClicks_==2 && now) {
        endorsementClosePending_=true;endorsementCloseAttempts_=0;
        endorsementCloseAt_=now+300;endorsementCloseUntil_=now+10000;
      }
    }
    if (ok && page==CNPage::Endorse && action==Action::Escape) cancelEndorsementClose();
    if (ok && action == Action::Click &&
        (page == CNPage::More || page == CNPage::DeathmatchStart || page == CNPage::Results)) requested_ = true;
    if (page == CNPage::Leave && ok && finished_) leaving_ = true;
    // Input submission is not proof that the dialog closed. Keep confirmation
    // authorized until a new lobby/queue/hero/combat observation clears it.
  }
  bool confirmedCombat(Millis now) const {
    return combatFrames_ >= 2 && !finished_ && now >= lastCombat_ && now - lastCombat_ <= 1500;
  }
  bool canInput(Millis now) const { return confirmedCombat(now) && now>=nextInput_; }
  KeyStroke next(Millis now) {
    // The old sample selects one of eight keys and holds it rand()%401+100 ms
    // (0x1400034B7–0x140003501). Keep the same timing range, with game keys only.
    constexpr std::array<std::uint16_t, 8> keys{'W', 'S', 'A', 'D', 'V', 'E', 'Q', 0xa0};
    std::uniform_int_distribution<int> key(0, int(keys.size()) - 1), duration(100, 500), gap(600, 1400);
    KeyStroke result{keys[std::size_t(key(random_))], Millis(duration(random_))};
    // Sample 0x140003506-0x140003512 follows every random key with a
    // 20 ms mouse click: even hold durations use left, odd use right.
    result.rightClick=(result.duration&1)!=0;
    nextInput_ = now + result.duration + 20 + Millis(gap(random_));
    return result;
  }
};
} // namespace owh
