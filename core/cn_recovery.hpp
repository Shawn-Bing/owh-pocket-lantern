#pragma once
#include "core/core.hpp"

namespace owh {
enum class CNPage { None, LoginError, Home, Play, More, Browser, Join, Launch, Room };
struct CNAnchor {
  Rect roi;
  double minimum;
  std::vector<std::uint8_t> image;
};
struct CNRule {
  CNPage page;
  const wchar_t *label;
  Action action;
  Point click;
  double aspect, minimum;
  std::vector<CNAnchor> anchors;
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
  CNObservation prepare(CNObservation observation, Millis now) {
    if (selected_ && (now<selectedAt_ || now-selectedAt_>15000)) reset();
    if (observation && observation.page!=CNPage::Browser && observation.page!=CNPage::Join) reset();
    // A cyan border can mean hover, while Join still points to a different
    // selected card. Always select the verified target ourselves first.
    if (observation.page==CNPage::Join && !selected_) {
      observation.page=CNPage::Browser;
      observation.step.label=L"先选中最稳轮换版";
      observation.step.action=Action::Click;
      observation.step.click={176./960,368./540};
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
inline CNObservation recognizeCNGame(const Frame &frame) {
  static const std::vector<CNRule> rules{
#include "core/cn_templates.inc"
  };
  CNObservation result;
  for (const auto &rule: rules) {
    if (frame.height<=0 || std::abs(double(frame.width)/frame.height/rule.aspect-1)>.035) continue;
    double score=1;
    bool accepted=true;
    for (const auto &a:rule.anchors) {
      auto value=similarity(sample(frame,a.roi),a.image);
      // Text in recorder JPEGs moves by a sample cell when restored to the
      // native client size. A small bounded alignment search keeps the title
      // requirement, instead of lowering the threshold for similar mode names.
      if (rule.page==CNPage::Browser && value<a.minimum)
        for (int dy=-2;dy<=2;++dy) for (int dx=-2;dx<=2;++dx) {
          auto roi=a.roi;roi.x+=double(dx)/frame.width;roi.y+=double(dy)/frame.height;
          value=std::max(value,similarity(sample(frame,roi),a.image));
        }
      if (value<a.minimum) {accepted=false;break;}
      score=std::min(score,value);
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
  if (result.page==CNPage::Browser && colorFraction(frame,{34./960,316./540,8./960,125./540},true)>.08 &&
      colorFraction(frame,{483./960,472./540,70./960,21./540},false)>=.45) {
    result.page=CNPage::Join;
    result.step.label=L"最稳轮换版：自动加入";
    result.step.click={520./960,483./540};
  }
  return result;
}
inline CNObservation recognizeCNLauncher(const Frame &frame, double dpiScale) {
  // Native Battle.net client coordinates: the Play button is anchored to the
  // bottom-left, not to a fraction of the resizable window. Check game branding
  // AND the actual Play text; blue Update/Playing buttons must not be clicked.
  if (!std::isfinite(dpiScale) || dpiScale<.5 || dpiScale>4 || frame.width<800*dpiScale || frame.height<500*dpiScale) return {};
  static const std::vector<std::vector<std::uint8_t>> images{
#include "core/cn_launcher_templates.inc"
  };
  Rect logo{57*dpiScale/frame.width,277*dpiScale/frame.height,215*dpiScale/frame.width,35*dpiScale/frame.height};
  Rect play{99*dpiScale/frame.width,1-113*dpiScale/frame.height,92*dpiScale/frame.width,31*dpiScale/frame.height};
  auto branding=similarity(sample(frame,logo),images[0]);
  auto button=similarity(sample(frame,play),images[1]);
  if (branding<.94 || button<.94) return {};
  CNObservation result;
  result.page=CNPage::Launch;
  result.score=std::min(branding,button);
  result.step.launcher=true;
  result.step.action=Action::Launch;
  result.step.label=L"战网守望先锋：进入游戏";
  result.step.click={145*dpiScale/frame.width,1-100*dpiScale/frame.height};
  return result;
}
} // namespace owh
