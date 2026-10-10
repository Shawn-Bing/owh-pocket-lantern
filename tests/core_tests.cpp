#include "core/core.hpp"
#include "core/cn_recovery.hpp"
#include "core/cn_profile.hpp"
#include "core/cn_default_profile.hpp"
#include <cstdlib>
#include <iostream>
void require(bool x, const char *msg) {
  if (!x) {
    std::cerr << "FAIL: " << msg << '\n';
    std::exit(1);
  }
}
owh::Frame frame(int w, int h) {
  owh::Frame f{w, h, {}};
  f.pixels.resize(w * h);
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      int v = ((x * 8 / w + y * 4 / h) % 2) ? 200 : 40;
      f.pixels[y * w + x] = v * 0x010101;
    }
  return f;
}
int main() {
  using namespace owh;
  auto f = frame(640, 360), g = frame(1280, 720);
  Rect full{0, 0, 1, 1};
  auto a = sample(f, full), b = sample(g, full);
  require(similarity(a, b) > .99, "same UI at two resolutions");
  require(sample(f, {-1, 0, 1, 1}).empty(), "invalid ROI rejected");
  Frame empty;
  require(sample(empty, full).empty(), "empty capture rejected");
  auto different = a;
  for (auto &v : different)
    v = 255 - v;
  require(similarity(a, different) < .5, "different scene rejected");
  require(contrast(std::vector<std::uint8_t>(512, 80)) == 0, "flat templates identifiable");
  Step s;
  s.roi = full;
  s.image = a;
  std::vector<Step> steps{s};
  require(match(g, steps, false).index == 0, "scale-independent recognition");
  require(match(g, steps, true).index == -1, "game/launcher templates isolated");
  steps.push_back(s);
  require(match(g, steps, false).ambiguous, "ambiguous templates cannot act");
  for (auto &step : steps) step.action = Action::Ready;
  require(match(g, steps, false).index >= 0, "overlapping room variants are equivalent");
  steps[1].action = Action::Restart;
  require(match(g, steps, false).ambiguous, "room and error conflict must stop input");
  steps[0].action = Action::Restart;
  require(match(g, steps, false).index >= 0, "error variants are equivalent");
  auto shifted = frame(128, 72), original = shifted;
  for (int y = 2; y < shifted.height; ++y)
    for (int x = 2; x < shifted.width; ++x)
      shifted.pixels[y * shifted.width + x] = original.pixels[(y - 2) * shifted.width + x - 2];
  Step hud;
  hud.action = Action::Ready;
  hud.roi = {.25, .25, .5, .5};
  hud.image = sample(original, hud.roi);
  require(templateScore(shifted, hud) > .99, "room HUD tolerates two-pixel alignment change");
  hud.action = Action::Click;
  require(templateScore(shifted, hud) < .9, "navigation keeps strict spatial matching");
  require(match(frame(800, 600), {s}, false).index == -1, "different aspect ratio rejected");
  StepGate gate;
  require(!gate.observe(0) && gate.observe(0), "two consecutive confirmations");
  gate.observe(-1);
  require(!gate.observe(0), "unknown breaks confirmation streak");
  gate.acted(0, 10000);
  require(!gate.canAct(0, 14999) && gate.canAct(0, 15000), "action cooldown");
  gate.acted(0, 15000);
  gate.acted(0, 20000);
  require(gate.exhausted(0), "unchanged page limited to three actions");
  gate.arrived(1);
  require(!gate.exhausted(0), "verified navigation clears prior retries");
  Step ready, error, menu;
  ready.action = Action::Ready;
  error.action = Action::Restart;
  gate.reset();
  require(!gate.observe(observationKey(ready, 4)) && gate.observe(observationKey(ready, 5)),
          "alternating equivalent room variants confirm");
  RoomSession room;
  room.observe(&ready, false);
  require(!room.active(), "one frame cannot establish a room");
  room.observe(&ready, true);
  room.observe(nullptr, false);
  require(room.active(), "missing HUD in spectating/results preserves room session");
  room.observe(&error, false);
  require(!room.active(), "first error evidence immediately stops input");
  room.observe(&ready, true);
  room.observe(&menu, false);
  require(!room.active(), "recognized navigation exits the room session");
  room.observe(&ready, true);
  room.observe(nullptr, false, true);
  require(!room.active(), "ambiguity stops established room input");
  room.observe(&ready, true);
  room.reset();
  require(!room.active(), "process replacement/stop resets room session");
  ContinuousEvidence errorTime;
  for (Millis t = 1000; t <= 91000; t += 1000) errorTime.observe(true, t);
  require(errorTime.elapsed(91000, 90000), "continuous error reaches recovery deadline");
  errorTime.observe(false, 92000);
  errorTime.observe(true, 93000);
  require(!errorTime.elapsed(93000, 90000), "unknown/focus loss interrupts error timer");
  errorTime.observe(true, 200000);
  require(!errorTime.elapsed(200000, 90000), "capture gap cannot count as error duration");
  errorTime.reset();
  require(!errorTime.elapsed(300000, 90000), "an explicit reset clears the evidence");
  // The disconnect gate is the one caller that has to survive the player taking
  // the computer back. Standby, a locked desktop and the game sitting behind
  // another window all stop the loop before it captures, and none of them says
  // the page changed -- clearing the evidence there is what made the ninety
  // second timeout unfinishable, because standby is armed by any mouse movement.
  ContinuousEvidence stuck{5 * 60 * 1000};
  for (Millis t = 1000; t <= 91000; t += 1000) stuck.observe(true, t);
  require(stuck.elapsed(91000, 90000), "a page the player is watching still times out");
  // Forty seconds of standby in the middle: the countdown is interrupted, but it
  // is not restarted, so the deadline arrives on wall clock as the player expects.
  ContinuousEvidence paused{5 * 60 * 1000};
  for (Millis t = 1000; t <= 30000; t += 1000) paused.observe(true, t);
  paused.observe(true, 70000);
  require(!paused.elapsed(70000, 90000), "forty seconds of standby is not ninety seconds of page");
  require(paused.elapsed(95000, 90000), "standby interrupts the countdown without restarting it");
  require(paused.deadline(90000) == 91000, "the deadline stays anchored to the first sighting");
  // Past the gap the helper was not watching for long enough to claim the page
  // was there throughout, so the run starts over rather than firing at once.
  ContinuousEvidence stale{5 * 60 * 1000};
  for (Millis t = 1000; t <= 30000; t += 1000) stale.observe(true, t);
  const Millis back = 30000 + 5 * 60 * 1000 + 1000;
  stale.observe(true, back);
  require(!stale.elapsed(back, 90000), "an absence longer than the gap starts the run over");
  // The default gap still refuses to count a slow or missed tick, which is what
  // every other caller relies on.
  ContinuousEvidence strict;
  strict.observe(true, 1000);
  strict.observe(true, 5000);
  strict.observe(true, 90000);
  require(!strict.elapsed(90000, 90000), "the default gap restarts the run after a missed tick");
  RecoveryBudget budget;
  require(budget.take(10000), "first recovery");
  require(!budget.take(20000), "restart cooldown");
  require(budget.take(40000), "second recovery");
  require(budget.take(100000), "third recovery");
  require(!budget.take(219999),"Third failure retains its two-minute cooldown");
  require(budget.take(220000),"Fourth recovery continues within the same hour");
  Millis retry=budget.retryAt();
  for (int attempt=0;attempt<1000;++attempt) {
    require(!budget.take(retry-1),"Unlimited attempts still obey cooldown");
    require(budget.take(retry),"Long unattended runs never exhaust the recovery count");
    require(budget.retryAt()==retry+120000,"Repeated failures use a bounded two-minute delay");
    retry=budget.retryAt();
  }
  // The window exists so that pressing 开始 with Battle.net already open and no
  // game running does not cost thirty seconds of doing nothing; the long wait is
  // only owed to a client that might be restarting itself.
  require(missingGameGrace(false, false) == 3000, "A cold start does not wait out the restart window");
  require(missingGameGrace(false, true) == 3000, "A cold start stays prompt inside a recovery cycle");
  require(missingGameGrace(true, false) == 30000,
          "A client that vanished mid-session may be restarting itself");
  require(missingGameGrace(true, true) == 3000,
          "A budgeted recovery cycle does not wait out the restart window");
  Point card{.1833, .6815};
  CNJoinGate join;
  CNObservation selected;
  selected.page = CNPage::Join;
  selected.step.action = Action::Click;
  selected.step.click = {.54, .89};
  require(join.prepare(selected, 1000, card).page == CNPage::Browser,
          "Highlighted or hovered card requires our own selection before joining");
  join.acted(CNPage::Browser, false, 1000);
  require(join.prepare(selected, 2000, card).page == CNPage::Browser,
          "Failed selection cannot authorize joining");
  join.acted(CNPage::Browser, true, 3000);
  require(join.prepare(selected, 8000, card).page == CNPage::Join,
          "Fresh confirmed selection allows joining");
  // A landed join click opens the burst window and -- unlike before -- keeps the
  // selection. Clearing it here is what made every round start over at the card
  // and produced the seven-second loop that never left the lobby.
  join.acted(CNPage::Join, true, 8000);
  require(join.bursting(8000) && join.bursting(8000 + CNJoinGate::BurstWindow - 1),
          "A landed join click opens a five second burst window");
  require(join.prepare(selected, 8200, card).page == CNPage::Join,
          "Inside the burst the join step is kept rather than rewritten to the card");
  require(!join.observe(CNPage::Join, 8400).joined,
          "Still looking at the join page is not evidence of having joined");
  require(!join.observe(CNPage::Browser, 8600).joined,
          "Still looking at the browse page is not evidence of having joined");
  require(join.bursting(8600), "The window stays open while the page does not change");
  // A window that closes with the page unchanged is one attempt spent. Five of
  // those is where the helper stops and says so rather than clicking all night.
  Millis burst = 8000;
  for (int spent = 0; spent < CNJoinGate::BurstAttempts - 1; ++spent) {
    require(!join.observe(CNPage::Join, burst + CNJoinGate::BurstWindow).joined,
            "An expired window is not a join");
    require(!join.gaveUp(), "Fewer than five spent bursts is not giving up");
    require(!join.bursting(burst + CNJoinGate::BurstWindow),
            "The window is closed once it expires");
    burst += CNJoinGate::BurstWindow;
    join.acted(CNPage::Join, true, burst);
  }
  join.observe(CNPage::Join, burst + CNJoinGate::BurstWindow);
  require(join.gaveUp(), "Five bursts without the page ever changing gives up");
  require(!join.bursting(burst + CNJoinGate::BurstWindow), "Giving up leaves the burst closed");
  // A loading/unknown frame cancels clicks without claiming success. Only two
  // positive room observations establish a join.
  join.acted(CNPage::Join, true, burst + CNJoinGate::BurstWindow + 1);
  require(!join.observe(CNPage::None, burst + CNJoinGate::BurstWindow + 200).joined,
          "Unknown or loading frames cannot prove a join");
  require(!join.observe(CNPage::Room,burst + CNJoinGate::BurstWindow + 400).joined &&
           join.observe(CNPage::Room,burst + CNJoinGate::BurstWindow + 600).joined,
          "Two room observations prove a join");
  require(!join.gaveUp(), "A confirmed join clears the spent burst count");
  require(!join.bursting(burst + CNJoinGate::BurstWindow + 200),
          "A confirmed join ends the burst immediately");
  // A failed click never reached the game, so it must not consume the window or
  // be mistaken for a join; the selection stays valid and the next tick retries.
  join.acted(CNPage::Browser, true, 30000);
  join.acted(CNPage::Join, false, 30100);
  require(!join.bursting(30100), "A click that never landed opens no window");
  require(join.prepare(selected, 30200, card).page == CNPage::Join,
          "A click that never landed keeps the selection it did not use");
  require(!join.observe(CNPage::Join, 30300).joined, "A click that never landed is not a join");
  join.acted(CNPage::Browser, true, 31000);
  CNObservation home;
  home.page = CNPage::Home;
  join.prepare(home, 32000, card);
  require(join.prepare(selected, 33000, card).page == CNPage::Browser,
          "Leaving browser revokes selection");
  join.acted(CNPage::Browser, true, 34000);
  join.reset();
  require(join.prepare(selected, 35000, card).page == CNPage::Browser,
          "Stop or process replacement revokes selection");
  join.acted(CNPage::Browser, true, 36000);
  require(join.prepare(selected, 35000, card).page == CNPage::Browser,
          "Clock rollback revokes selection");
  require(jsonEscape("a\"\n\\") == "a\\\"\\u000a\\\\", "telemetry JSON escaping");
  // The preset is data now, so the parser sits on the recovery path: a profile
  // that parsed only partially would silently drop navigation steps.
  CNProfile profile, ignored;
  std::string profileError;
  require(parseCNProfile(defaultCNProfileJson(), profile, profileError),
          ("embedded CN profile must parse: " + profileError).c_str());
  require(!profile.rules.empty() && profile.launcher.size() >= 2,
          "embedded CN profile must be complete");
  bool hasVariants = false;
  for (const auto &rule : profile.rules)
    for (const auto &anchor : rule.anchors)
      hasVariants = hasVariants || anchor.variants.size() > 1;
  require(hasVariants, "one page sampled at two window sizes must share a rule as variants");
  CNProfile reparsed;
  require(parseCNProfile(writeCNProfile(profile), reparsed, profileError),
          ("written CN profile must parse back: " + profileError).c_str());
  require(reparsed.rules.size() == profile.rules.size() &&
              reparsed.launcher.size() == profile.launcher.size(),
          "round trip preserves the rule and launcher counts");
  bool identical = true;
  for (std::size_t i = 0; i < profile.rules.size() && identical; ++i) {
    const auto &before = profile.rules[i], &after = reparsed.rules[i];
    identical = before.page == after.page && before.action == after.action &&
                before.label == after.label &&
                std::abs(before.click.x - after.click.x) < 1e-12 &&
                std::abs(before.aspect - after.aspect) < 1e-12 &&
                before.anchors.size() == after.anchors.size();
    for (std::size_t j = 0; identical && j < before.anchors.size(); ++j)
      identical = before.anchors[j].variants == after.anchors[j].variants;
  }
  require(identical, "round trip preserves anchors and template bytes");
  require(cnProfileTargets(profile,L"最稳轮换版") && !cnProfileTargets(profile,L"其他目标"),
          "Legacy profiles cannot authorize another target room");
  auto trained = profile; trained.roomTarget = L"用户示范的目标";
  require(parseCNProfile(writeCNProfile(trained),reparsed,profileError) &&
          reparsed.roomTarget == trained.roomTarget && cnProfileTargets(reparsed,trained.roomTarget) &&
          !cnProfileTargets(reparsed,L"最稳轮换版"),"Imported calibration remains bound to its demonstrated room target");
  CNJoinGate longLoad;
  longLoad.acted(CNPage::Browser,true,1000);
  longLoad.acted(CNPage::Join,true,2000);
  CNObservation arrived; arrived.page=CNPage::Room; arrived.step.action=Action::Ready;
  require(!longLoad.observe(CNPage::Room,71000).joined,"One room observation cannot confirm a slow join");
  longLoad.prepare(arrived,71000,card);
  require(longLoad.awaitingRoom(72000) && longLoad.observe(CNPage::Room,72000).joined,
          "Selection expiry cannot erase a pending slow join when loading frames were unavailable");
  CNJoinGate lateJoin;
  for (Millis time=1000; time<26000; time+=5000) {
    lateJoin.acted(CNPage::Browser,true,time); lateJoin.acted(CNPage::Join,true,time);
    lateJoin.observe(CNPage::Join,time+5000);
  }
  require(lateJoin.gaveUp(),"Five attempts establish retry backoff");
  lateJoin.observe(CNPage::None,27000);
  lateJoin.prepare({},42000,card);
  require(!lateJoin.gaveUp() && lateJoin.awaitingRoom(70000),
          "Backoff expiry preserves a late join still loading");
  require(!lateJoin.observe(CNPage::Room,71000).joined && lateJoin.observe(CNPage::Room,73000).joined,
          "Late server join still requires two positive room observations");
  CNRoomStart knownRoom; knownRoom.begin(71,800,1000);
  require(knownRoom.trusted(71,800,2000) && !knownRoom.trusted(72,800,2000) &&
          !knownRoom.trusted(71,801,2000) && !knownRoom.trusted(71,800,200000),
          "Explicit current-room confirmation is bounded to a process and deadline");
  knownRoom.observe(CNPage::None);
  require(knownRoom.trusted(71,800,3000),"Loading keeps explicit room confirmation pending");
  knownRoom.observe(CNPage::Hero);
  require(knownRoom.trusted(71,800,3000),"Hero selection keeps explicit room confirmation pending");
  CNJoinGate choosing;choosing.acted(CNPage::Join,true,1000);
  require(!choosing.observe(CNPage::Hero,2000).joined && choosing.awaitingRoom(2000) && !choosing.bursting(2000),
          "Hero selection stops join clicks without claiming the room is ready");
  CNObservation heroStep;heroStep.page=CNPage::Hero;choosing.prepare(heroStep,3000,card);
  require(choosing.awaitingRoom(3000) && !choosing.observe(CNPage::Room,4000).joined &&
          choosing.observe(CNPage::Room,5000).joined,
          "After hero confirmation the custom room still needs two fresh observations");
  knownRoom.observe(CNPage::Home);
  require(!knownRoom.trusted(71,800,3000),"Navigation revokes explicit room confirmation");
  require(!parseCNProfile("{", ignored, profileError), "truncated profile rejected");
  require(!parseCNProfile("[]", ignored, profileError), "non-object profile rejected");
  require(!parseCNProfile("{\"format\":\"owh-profile\",\"version\":2,\"rules\":[]}", ignored,
                          profileError),
          "unsupported profile version rejected");
  std::string shortTemplate =
      "{\"format\":\"owh-profile\",\"version\":1,\"rules\":[{\"page\":\"home\",\"label\":\"x\","
      "\"action\":\"click\",\"click\":[0.5,0.5],\"aspect\":1.7777,\"minimum\":0.9,\"anchors\":"
      "[{\"roi\":[0,0,0.1,0.1],\"minimum\":0.9,\"variants\":[\"00ff\"]}]}],\"join\":"
      "{\"selection_roi\":[0,0,0.1,0.1],\"button_roi\":[0,0,0.1,0.1],\"click\":[0.5,0.5]}}";
  require(!parseCNProfile(shortTemplate, ignored, profileError), "short template rejected");
  // 512 identical samples have zero contrast and cannot tell two pages apart.
  std::string flat =
      "{\"format\":\"owh-profile\",\"version\":1,\"rules\":[{\"page\":\"home\",\"label\":\"x\","
      "\"action\":\"click\",\"click\":[0.5,0.5],\"aspect\":1.7777,\"minimum\":0.9,\"anchors\":"
      "[{\"roi\":[0,0,0.1,0.1],\"minimum\":0.9,\"variants\":[\"" +
      std::string(1024, '8') +
      "\"]}]}],\"join\":{\"selection_roi\":[0,0,0.1,0.1],\"button_roi\":[0,0,0.1,0.1],"
      "\"click\":[0.5,0.5]}}";
  require(!parseCNProfile(flat, ignored, profileError), "flat template rejected");
  require(cnCardClick(profile).x > .1 && cnCardClick(profile).x < .3,
          "card click comes from the profile's browser rule");
  std::cout << "All core scenarios passed\n";
}
