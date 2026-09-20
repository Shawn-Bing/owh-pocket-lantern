#include "core/core.hpp"
#include "core/cn_recovery.hpp"
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
  require(!errorTime.elapsed(300000, 90000), "desktop pause resets disconnect evidence");
  RecoveryBudget budget;
  require(budget.take(10000), "first recovery");
  require(!budget.take(20000), "restart cooldown");
  require(budget.take(40000), "second recovery");
  require(budget.take(100000), "third recovery");
  require(!budget.take(500000), "hourly restart cap");
  require(budget.take(3610000), "budget replenishes after one hour");
  CNJoinGate join;
  CNObservation selected;
  selected.page = CNPage::Join;
  selected.step.action = Action::Click;
  selected.step.click = {.54, .89};
  require(join.prepare(selected, 1000).page == CNPage::Browser,
          "Highlighted or hovered card requires our own selection before joining");
  join.acted(CNPage::Browser, false, 1000);
  require(join.prepare(selected, 2000).page == CNPage::Browser,
          "Failed selection cannot authorize joining");
  join.acted(CNPage::Browser, true, 3000);
  require(join.prepare(selected, 8000).page == CNPage::Join,
          "Fresh confirmed selection allows joining");
  join.acted(CNPage::Join, false, 8000);
  require(join.prepare(selected, 9000).page == CNPage::Browser,
          "Even failed join consumes selection evidence");
  join.acted(CNPage::Browser, true, 10000);
  require(join.prepare(selected, 25001).page == CNPage::Browser,
          "Stale selection cannot authorize joining");
  join.acted(CNPage::Browser, true, 30000);
  CNObservation home;
  home.page = CNPage::Home;
  join.prepare(home, 31000);
  require(join.prepare(selected, 32000).page == CNPage::Browser,
          "Leaving browser revokes selection");
  join.acted(CNPage::Browser, true, 33000);
  join.reset();
  require(join.prepare(selected, 34000).page == CNPage::Browser,
          "Stop or process replacement revokes selection");
  join.acted(CNPage::Browser, true, 35000);
  require(join.prepare(selected, 34000).page == CNPage::Browser,
          "Clock rollback revokes selection");
  require(jsonEscape("a\"\n\\") == "a\\\"\\u000a\\\\", "telemetry JSON escaping");
  std::cout << "All core scenarios passed\n";
}
