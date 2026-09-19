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
enum class Action { Click, Enter, Escape, Text, Ready, Restart, Launch };
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
inline Match match(const Frame &f, const std::vector<Step> &steps, bool launcher,
                   double threshold = .90) {
  Match result;
  double second = 0;
  for (std::size_t i = 0; i < steps.size(); ++i) {
    auto &s = steps[i];
    if (s.launcher != launcher)
      continue;
    if (f.height <= 0 || std::abs(double(f.width) / f.height / s.aspect - 1) > .035) {
      result.aspectMismatch = true;
      continue;
    }
    auto score = similarity(sample(f, s.roi), s.image);
    if (score > result.score) {
      second = result.score;
      result.score = score;
      result.index = int(i);
    } else
      second = std::max(second, score);
  }
  result.ambiguous =
      result.score >= threshold && second >= threshold && result.score - second < .025;
  if (result.score < threshold || result.ambiguous)
    result.index = -1;
  return result;
}
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
  bool exhausted(int index) const { return lastAction_ == index && attempts_ >= 3; }
  bool cooldownReady(Millis now) const { return lastAt_ == 0 || now - lastAt_ >= 5000; }
  bool canAct(int index, Millis now) const { return cooldownReady(now) && !exhausted(index); }
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
