#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace owh {
using Millis = std::uint64_t;
struct Rect {
  double x{}, y{}, w{}, h{};
};
struct Point {
  double x{}, y{};
};
struct Frame {
  int width{}, height{};
  std::vector<std::uint32_t> pixels;
};
constexpr int TemplateW = 32, TemplateH = 16;
inline bool valid(Rect r) {
  return std::isfinite(r.x) && std::isfinite(r.y) && std::isfinite(r.w) && std::isfinite(r.h) &&
         r.x >= 0 && r.y >= 0 && r.w > 0 && r.h > 0 && r.x + r.w <= 1.00001 && r.y + r.h <= 1.00001;
}
inline std::vector<std::uint8_t> sample(const Frame &f, Rect r) {
  if (!valid(r) || f.width <= 0 || f.height <= 0 ||
      f.pixels.size() != std::size_t(f.width) * f.height)
    return {};
  std::vector<std::uint8_t> out(TemplateW * TemplateH);
  for (int y = 0; y < TemplateH; ++y)
    for (int x = 0; x < TemplateW; ++x) {
      // Four samples per cell reduce aliasing without processing the whole
      // frame.
      int sum = 0;
      for (int sy = 0; sy < 2; ++sy)
        for (int sx = 0; sx < 2; ++sx) {
          int px = std::clamp(int((r.x + r.w * (x + (sx + .5) / 2) / TemplateW) * f.width), 0,
                              f.width - 1);
          int py = std::clamp(int((r.y + r.h * (y + (sy + .5) / 2) / TemplateH) * f.height), 0,
                              f.height - 1);
          auto p = f.pixels[std::size_t(py) * f.width + px];
          sum += (int((p >> 16) & 255) * 30 + int((p >> 8) & 255) * 59 + int(p & 255) * 11) / 100;
        }
      out[y * TemplateW + x] = std::uint8_t(sum / 4);
    }
  return out;
}
inline double contrast(const std::vector<std::uint8_t> &a) {
  if (a.empty())
    return 0;
  double m = 0, v = 0;
  for (auto p : a)
    m += p;
  m /= a.size();
  for (auto p : a)
    v += (p - m) * (p - m);
  return std::sqrt(v / a.size());
}
inline double similarity(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) {
  if (a.size() != TemplateW * TemplateH || b.size() != a.size())
    return 0;
  double ma = 0, mb = 0;
  for (auto x : a)
    ma += x;
  for (auto x : b)
    mb += x;
  ma /= a.size();
  mb /= b.size();
  double diff = 0;
  for (std::size_t i = 0; i < a.size(); ++i)
    diff += std::abs((a[i] - ma) - (b[i] - mb));
  // Preserve some luminance sensitivity: an entirely different bright page must
  // not match.
  diff = diff / a.size() + .20 * std::abs(ma - mb);
  return std::clamp(1 - diff / 128.0, 0.0, 1.0);
}
// Ignore marks a page recognised only so an established room session ends: it
// is never clicked, and unlike Ready it stops the Shift pulses. Menus the player
// opens deliberately -- settings, hero gallery -- belong here. Appended last so
// the integer values already stored in settings.ini keep their meaning.
enum class Action { Click, Enter, Escape, Text, Ready, Restart, Launch, Ignore };
struct Step {
  std::wstring label, text;
  bool launcher{};
  Action action{Action::Click};
  Rect roi;
  Point click;
  double aspect{16.0 / 9.0};
  std::vector<std::uint8_t> image;
};
struct Match {
  int index{-1};
  double score{};
  bool ambiguous{};
  bool aspectMismatch{};
};
inline bool equivalentObservation(const Step &a, const Step &b) {
  return a.launcher == b.launcher && a.action == b.action &&
         (a.action == Action::Ready || a.action == Action::Restart);
}
inline int observationKey(const Step &s, int index) {
  // Configuration admits at most 32 templates. Non-action observations may have variants.
  return s.action == Action::Ready ? 32 : (s.action == Action::Restart ? 33 : index);
}
inline double templateScore(const Frame &f, const Step &s) {
  double best = similarity(sample(f, s.roi), s.image);
  if (s.action != Action::Ready || best >= .98 || f.width <= 0 || f.height <= 0) return best;
  // Small HUD alignment tolerance only; navigation and error templates remain exact.
  double dx = std::max(1, f.width / 1024) / double(f.width);
  double dy = std::max(1, f.height / 576) / double(f.height);
  for (int y = -2; y <= 2; ++y)
    for (int x = -2; x <= 2; ++x) {
      if (x == 0 && y == 0) continue;
      auto r = s.roi;
      r.x += x * dx;
      r.y += y * dy;
      best = std::max(best, similarity(sample(f, r), s.image));
    }
  return best;
}
inline Match match(const Frame &f, const std::vector<Step> &steps, bool launcher,
                   double threshold = .90) {
  Match result;
  std::vector<double> scores(steps.size());
  for (std::size_t i = 0; i < steps.size(); ++i) {
    auto &s = steps[i];
    if (s.launcher != launcher)
      continue;
    if (f.height <= 0 || std::abs(double(f.width) / f.height / s.aspect - 1) > .035) {
      result.aspectMismatch = true;
      continue;
    }
    auto score = templateScore(f, s);
    scores[i] = score;
    if (score > result.score) {
      result.score = score;
      result.index = int(i);
    }
  }
  if (result.index >= 0 && result.score >= threshold)
    for (std::size_t i = 0; i < steps.size(); ++i)
      if (int(i) != result.index && scores[i] >= threshold && result.score - scores[i] < .025 &&
          !equivalentObservation(steps[result.index], steps[i]))
        result.ambiguous = true;
  if (result.score < threshold || result.ambiguous)
    result.index = -1;
  return result;
}
class RoomSession {
  bool active_{};
public:
  void reset() { active_ = false; }
  bool active() const { return active_; }
  // A confirmed join is itself evidence of being in a room. The lobby, the map
  // intro, spectating and the results screen all follow it, and none of them
  // shows the HUD the Ready template was recorded from, so waiting for that HUD
  // could delay the first pulse by minutes. Recognised menus, error pages,
  // ambiguity and process changes still end the session.
  void joined() { active_ = true; }
  void observe(const Step *step, bool confirmed, bool ambiguous = false) {
    if (ambiguous || (step && (step->launcher || step->action != Action::Ready)))
      active_ = false;
    else if (step && confirmed)
      active_ = true;
    // Missing HUD during spectating/loading/results does not end an established session.
  }
};
class ContinuousEvidence {
  Millis start_{}, last_{};
  bool tracking_{};
public:
  void reset() { tracking_ = false; }
  void observe(bool present, Millis now) {
    if (!present) { reset(); return; }
    if (!tracking_ || now < last_ || now - last_ > 2500) start_ = now;
    tracking_ = true;
    last_ = now;
  }
  bool elapsed(Millis now, Millis duration) const {
    return tracking_ && now >= last_ && now - last_ <= 2500 && now - start_ >= duration;
  }
  Millis deadline(Millis duration) const { return tracking_ ? start_ + duration : 0; }
};
class StepGate {
  int candidate_{-1}, confirmations_{}, lastAction_{-1}, attempts_{};
  Millis lastAt_{};

public:
  void reset() { *this = StepGate{}; }
  bool observe(int index) {
    if (index < 0) {
      candidate_ = -1;
      confirmations_ = 0;
      return false;
    }
    if (index != candidate_) {
      candidate_ = index;
      confirmations_ = 1;
    } else
      confirmations_ = std::min(2, confirmations_ + 1);
    return confirmations_ >= 2;
  }
  // The retry budget is per action kind rather than global. A join that silently
  // did nothing is worth retrying quickly, because the lobby may simply have been
  // full at that instant and stopping there strands the whole session. A
  // navigation button that did nothing usually means the screen is not what we
  // think it is, so it keeps the conservative default.
  bool exhausted(int index, int maximum = 3) const {
    return lastAction_ == index && attempts_ >= maximum;
  }
  bool cooldownReady(Millis now, Millis cooldown = 5000) const {
    return lastAt_ == 0 || now - lastAt_ >= cooldown;
  }
  bool canAct(int index, Millis now, Millis cooldown = 5000, int maximum = 3) const {
    return cooldownReady(now, cooldown) && !exhausted(index, maximum);
  }
  void acted(int index, Millis now) {
    if (lastAction_ != index) {
      attempts_ = 0;
      lastAction_ = index;
    }
    ++attempts_;
    lastAt_ = now;
  }
  // Entering a different, verified page completes the previous action sequence.
  void arrived(int index) {
    if (lastAction_ >= 0 && lastAction_ != index) {
      lastAction_ = -1;
      attempts_ = 0;
    }
  }
};
class RecoveryBudget {
  std::deque<Millis> attempts_;
  Millis next_{};

public:
  bool available(Millis now) {
    while (!attempts_.empty() && now - attempts_.front() >= 3600000)
      attempts_.pop_front();
    return attempts_.size() < 3 && now >= next_;
  }
  bool take(Millis now) {
    if (!available(now))
      return false;
    attempts_.push_back(now);
    constexpr Millis delays[] = {30000, 60000, 120000};
    next_ = now + delays[attempts_.size() - 1];
    return true;
  }
  Millis retryAt() const {
    return attempts_.size() >= 3 ? std::max(next_, attempts_.front() + 3600000) : next_;
  }
};
inline std::string jsonEscape(const std::string &s) {
  std::string r;
  const char hex[] = "0123456789abcdef";
  for (unsigned char c : s) {
    if (c == '"' || c == '\\') {
      r += '\\';
      r += char(c);
    } else if (c < 32) {
      r += "\\u00";
      r += hex[c >> 4];
      r += hex[c & 15];
    } else
      r += char(c);
  }
  return r;
}
} // namespace owh
