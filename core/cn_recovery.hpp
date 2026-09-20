#pragma once
#include "core/core.hpp"
#include <string>

namespace owh {
// Menu covers pages the player opens deliberately -- settings, hero gallery --
// that must be recognised so an established room session ends, but never acted
// on. Appended last so it cannot shift an existing identifier.
enum class CNPage { None, LoginError, Home, Play, More, Browser, Join, Launch, Room, Menu };

// One anchor passes when any of its variants reaches `minimum`. Variants exist
// because the same on-screen text yields different 32x16 samples at different
// window pixel sizes: they express one requirement observed at several sizes,
// not several different pages. A rule passes only when every anchor passes.
struct CNAnchor {
  Rect roi;
  double minimum;
  std::vector<std::vector<std::uint8_t>> variants;
};
struct CNRule {
  CNPage page;
  std::wstring label;
  Action action;
  Point click;
  double aspect, minimum;
  std::vector<CNAnchor> anchors;
};

// Battle.net anchors are positioned in native client pixels scaled by DPI, not
// as a fraction of the window: the Play button is anchored to the bottom-left
// and the logo to the top-left, so both move with the window origin but not with
// its size. A negative y therefore measures up from the bottom edge.
struct CNLauncherAnchor {
  double x{}, y{}, w{}, h{};
  double minimum{.94};
  // Optional click in the same native pixel space; only the Play anchor sets it.
  bool clickable{};
  double clickX{}, clickY{};
  std::vector<std::vector<std::uint8_t>> variants;
};
inline Rect launcherRect(const CNLauncherAnchor &a, int width, int height, double dpiScale) {
  double y = a.y < 0 ? 1.0 + a.y * dpiScale / height : a.y * dpiScale / height;
  return {a.x * dpiScale / width, y, a.w * dpiScale / width, a.h * dpiScale / height};
}

// The "自动加入" state is not a template: it is the target card's cyan selection
// border plus the orange join button. Keeping the geometry here rather than in
// the matching code lets a different target mode ship its own numbers.
struct CNJoinSpec {
  Rect selectionRoi;
  double selectionMinimum{.08};
  Rect buttonRoi;
  double buttonMinimum{.45};
  Point click;
  std::wstring label;
};

struct CNObservation {
  CNPage page{CNPage::None};
  Step step;
  double score{};
  explicit operator bool() const { return page != CNPage::None; }
  int key() const { return 100 + int(page); }
};
class CNJoinGate {
  Millis selectedAt_{};
  bool selected_{};
public:
  void reset() {selected_=false;}
  // `card` is the target-card click taken from the profile's Browser rule. It
  // used to be duplicated as a literal here, so moving the card silently left
  // this step pointing at the old position.
  CNObservation prepare(CNObservation observation, Millis now, Point card) {
    if (selected_ && (now<selectedAt_ || now-selectedAt_>15000)) reset();
    if (observation && observation.page!=CNPage::Browser && observation.page!=CNPage::Join) reset();
    // A cyan border can mean hover, while Join still points to a different
    // selected card. Always select the verified target ourselves first.
    if (observation.page==CNPage::Join && !selected_) {
      observation.page=CNPage::Browser;
      observation.step.label=L"先选中最稳轮换版";
      observation.step.action=Action::Click;
      observation.step.click=card;
    }
    return observation;
  }
  void acted(CNPage page, bool success, Millis now) {
    if (page==CNPage::Browser) {selected_=success;selectedAt_=now;}
    if (page==CNPage::Join) reset();
  }
};
inline double colorFraction(const Frame &f, Rect roi, bool cyan) {
  if (!valid(roi) || f.width <= 0 || f.height <= 0 || f.pixels.size() != std::size_t(f.width)*f.height) return 0;
  int matched=0, total=0;
  for (int y=int(roi.y*f.height); y<std::min(f.height,int((roi.y+roi.h)*f.height)); ++y)
    for (int x=int(roi.x*f.width); x<std::min(f.width,int((roi.x+roi.w)*f.width)); ++x) {
      auto p=f.pixels[std::size_t(y)*f.width+x];
      int r=(p>>16)&255, g=(p>>8)&255, b=p&255;
      ++total;
      matched += cyan ? (g>130 && b>130 && g-r>50 && b-r>50) : (r>200 && g>40 && g<170 && b<90);
    }
  return total ? double(matched)/total : 0;
}
// Best score of one anchor across its variants. The bounded alignment search is
// only paid when the primary position misses, and it stays limited to pages
// whose text moves by a sample cell when a recorder JPEG is restored to the
// native client size.
inline double anchorScore(const Frame &frame, const CNAnchor &anchor, bool searchOffsets) {
  double value=0;
  for (const auto &variant:anchor.variants)
    value=std::max(value,similarity(sample(frame,anchor.roi),variant));
  if (!searchOffsets || value>=anchor.minimum) return value;
  for (int dy=-2;dy<=2;++dy) for (int dx=-2;dx<=2;++dx) {
    auto roi=anchor.roi;
    roi.x+=double(dx)/frame.width;
    roi.y+=double(dy)/frame.height;
    for (const auto &variant:anchor.variants)
      value=std::max(value,similarity(sample(frame,roi),variant));
  }
  return value;
}
// Reports the closest rule when nothing matched. A bare failure cannot
// distinguish "this page was never recorded" from "the threshold is too strict
// for this window size", which is the difference between adding a step and
// tuning one, so the per-anchor scores of the best candidate are kept.
struct CNGameScore {
  bool evaluated{};
  bool accepted{};
  double best{};
  int rule{-1};
  CNPage page{CNPage::None};
  std::vector<double> anchors;
};
inline CNObservation recognizeCNGame(const Frame &frame, const std::vector<CNRule> &rules,
                                     const CNJoinSpec &join, CNGameScore *detail = nullptr) {
  if (detail) *detail = CNGameScore{};
  CNObservation result;
  for (std::size_t index = 0; index < rules.size(); ++index) {
    const auto &rule = rules[index];
    if (frame.height<=0 || std::abs(double(frame.width)/frame.height/rule.aspect-1)>.035) continue;
    // Every anchor is scored even after one fails: the scores are what makes a
    // near miss diagnosable, and the cost is negligible next to a capture.
    double score=1;
    bool accepted=true;
    std::vector<double> anchorScores;
    anchorScores.reserve(rule.anchors.size());
    for (const auto &a:rule.anchors) {
      auto value=anchorScore(frame,a,rule.page==CNPage::Browser);
      anchorScores.push_back(value);
      score=std::min(score,value);
      if (value<a.minimum) accepted=false;
    }
    if (detail) {
      bool better=!detail->evaluated || (accepted && !detail->accepted) ||
                  (accepted==detail->accepted && score>detail->best);
      if (better) {
        detail->evaluated=true;
        detail->accepted=accepted;
        detail->best=score;
        detail->rule=int(index);
        detail->page=rule.page;
        detail->anchors=anchorScores;
      }
    }
    if (!accepted) continue;
    if (result && result.page!=rule.page) return {}; // Never choose between conflicting pages.
    if (result && result.score>=score) continue;
    result.page=rule.page;
    result.score=score;
    result.step.label=rule.label;
    result.step.action=rule.action;
    result.step.click=rule.click;
  }
  if (result.page==CNPage::Browser &&
      colorFraction(frame,join.selectionRoi,true)>join.selectionMinimum &&
      colorFraction(frame,join.buttonRoi,false)>=join.buttonMinimum) {
    result.page=CNPage::Join;
    result.step.label=join.label;
    result.step.click=join.click;
  }
  return result;
}
// Reports why the Battle.net Play button was or was not accepted. Both anchors
// must reach their minimum, so a near miss is worth surfacing instead of a bare
// failure.
struct CNLauncherScore {
  bool evaluated{};
  std::vector<double> scores; // one per launcher anchor, in profile order
};
inline CNObservation recognizeCNLauncher(const Frame &frame, double dpiScale,
                                         const std::vector<CNLauncherAnchor> &anchors,
                                         CNLauncherScore *detail = nullptr) {
  // Native Battle.net client coordinates: the Play button is anchored to the
  // bottom-left, not to a fraction of the resizable window. Check game branding
  // AND the actual Play text; blue Update/Playing buttons must not be clicked.
  if (!std::isfinite(dpiScale) || dpiScale<.5 || dpiScale>4 || frame.width<800*dpiScale || frame.height<500*dpiScale) return {};
  if (anchors.empty()) return {};
  CNObservation result;
  double worst=1;
  std::vector<double> scores;
  scores.reserve(anchors.size());
  for (const auto &anchor:anchors) {
    if (anchor.variants.empty()) return {};
    auto roi=launcherRect(anchor,frame.width,frame.height,dpiScale);
    double value=0;
    for (const auto &variant:anchor.variants)
      value=std::max(value,similarity(sample(frame,roi),variant));
    scores.push_back(value);
    if (value<anchor.minimum) {
      if (detail) {detail->evaluated=true;detail->scores=scores;}
      return {};
    }
    worst=std::min(worst,value);
  }
  if (detail) {detail->evaluated=true;detail->scores=scores;}
  for (const auto &anchor:anchors)
    if (anchor.clickable) {
      result.step.click={anchor.clickX*dpiScale/frame.width,
                         anchor.clickY<0 ? 1.0+anchor.clickY*dpiScale/frame.height
                                         : anchor.clickY*dpiScale/frame.height};
      break;
    }
  result.page=CNPage::Launch;
  result.score=worst;
  result.step.launcher=true;
  result.step.action=Action::Launch;
  result.step.label=L"战网守望先锋：进入游戏";
  return result;
}
} // namespace owh
