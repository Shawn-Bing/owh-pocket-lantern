#include "core/core.hpp"
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
  RecoveryBudget budget;
  require(budget.take(10000), "first recovery");
  require(!budget.take(20000), "restart cooldown");
  require(budget.take(40000), "second recovery");
  require(budget.take(100000), "third recovery");
  require(!budget.take(500000), "hourly restart cap");
  require(budget.take(3610000), "budget replenishes after one hour");
  require(jsonEscape("a\"\n\\") == "a\\\"\\u000a\\\\", "telemetry JSON escaping");
  std::cout << "All core scenarios passed\n";
}
