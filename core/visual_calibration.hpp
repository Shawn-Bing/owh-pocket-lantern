#pragma once
#include "core/cn_profile.hpp"
#include <cctype>
#include <optional>
#include <sstream>

namespace owh {
struct TeachingStep {
  std::string image, scope, action;
  Point click{};
  bool hasClick{};
  int width{}, height{};
  double dpi{1};
  Millis tick{};
  std::string verificationImage;
};
struct TeachingSession {
  std::string target;
  std::vector<TeachingStep> steps;
};
struct VisualAnchor {
  Rect roi;
  std::string label;
};
struct VisualAnnotation {
  CNPage page{CNPage::None};
  Action action{Action::Wait};
  std::string label;
  Point click{};
  bool hasClick{};
  double confidence{};
  std::vector<VisualAnchor> anchors;
};
struct LocalVisionEndpoint {
  std::string host;
  unsigned short port{8080};
  bool secure{};
};

inline std::string visualJsonQuote(const std::string &value) {
  std::string out = "\"";
  constexpr char digits[] = "0123456789abcdef";
  for (unsigned char ch : value) {
    if (ch == '"' || ch == '\\') { out += '\\'; out += char(ch); }
    else if (ch < 0x20) { out += "\\u00"; out += digits[ch >> 4]; out += digits[ch & 15]; }
    else out += char(ch);
  }
  return out + '"';
}
inline bool visualText(const Json *value, std::string &out, std::size_t limit, bool empty = false) {
  if (!value || !value->is(Json::Kind::String) || value->text.size() > limit ||
      (!empty && value->text.empty())) return false;
  for (unsigned char ch : value->text) if (ch < 0x20) return false;
  out = value->text; return true;
}
inline bool safeTeachingImageName(const std::string &name) {
  if (name.empty() || name.size() > 1024 || name.front() == '/' || name.front() == '\\') return false;
  std::string part;
  for (std::size_t i = 0; i <= name.size(); ++i) {
    unsigned char ch = i == name.size() ? '/' : static_cast<unsigned char>(name[i]);
    if (ch < 0x20 || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') return false;
    if (ch == '/' || ch == '\\') {
      if (part.empty() || part == "." || part == ".." || part.back() == '.' || part.back() == ' ') return false;
      part.clear();
    } else part += char(ch);
  }
  return true;
}
inline bool parseTeachingSession(const std::string &source, TeachingSession &out, std::string &error) {
  if (source.size() > 1024 * 1024) return error = "teaching manifest exceeds 1 MiB", false;
  Json root;
  if (!parseJson(source, root, error)) return false;
  auto format = root.find("format"), version = root.find("version"), steps = root.find("steps");
  if (!root.is(Json::Kind::Object) || !format || !format->is(Json::Kind::String) || format->text != "owh-teaching" ||
      !version || !version->is(Json::Kind::Number) || version->number != 1)
    return error = "expected owh-teaching version 1", false;
  if (!steps || !steps->is(Json::Kind::Array) || steps->items.empty() || steps->items.size() > 128)
    return error = "teaching session needs 1 to 128 steps", false;
  TeachingSession session;
  if (!visualText(root.find("room_target"), session.target, 384, true))
    return error = "invalid room_target", false;
  for (const auto &item : steps->items) {
    TeachingStep step;
    if (!item.is(Json::Kind::Object) || !visualText(item.find("image"), step.image, 1024) || !safeTeachingImageName(step.image))
      return error = "step image must stay inside the session directory", false;
    auto verification = item.find("verify_image");
    auto legacyVerification = item.find("verification_image");
    if (!verification) verification = legacyVerification;
    if (verification) {
      if (!visualText(verification, step.verificationImage, 1024) || !safeTeachingImageName(step.verificationImage))
        return error = "verification image must stay inside the session directory", false;
      if (legacyVerification && (!legacyVerification->is(Json::Kind::String) || legacyVerification->text != step.verificationImage))
        return error = "conflicting verification image aliases", false;
    }
    if (!visualText(item.find("scope"), step.scope, 16) || (step.scope != "game" && step.scope != "launcher"))
      return error = "step scope must be game or launcher", false;
    if (!visualText(item.find("action"), step.action, 16) ||
        (step.action != "click" && step.action != "enter" && step.action != "escape" && step.action != "ready" && step.action != "observe"))
      return error = "unsupported demonstrated action", false;
    if (auto click = item.find("click")) {
      if (!detail::readPoint(*click, step.click)) return error = "invalid recorded click", false;
      step.hasClick = true;
    }
    if (step.action == "click" && !step.hasClick) return error = "click demonstration has no recorded coordinate", false;
    for (const char *key : {"width", "height"}) {
      auto value = item.find(key);
      if (!value || !value->is(Json::Kind::Number) || !std::isfinite(value->number) ||
          std::floor(value->number) != value->number || value->number < 64 || value->number > 16384)
        return error = "invalid recorded client dimensions", false;
      (std::string(key) == "width" ? step.width : step.height) = int(value->number);
    }
    if (std::uint64_t(step.width) * step.height > 64000000) return error = "recorded frame is too large", false;
    auto dpi = item.find("dpi"), tick = item.find("tick_ms");
    if (!dpi || !dpi->is(Json::Kind::Number) || !std::isfinite(dpi->number) || dpi->number < .5 || dpi->number > 4)
      return error = "invalid recorded dpi", false;
    if (!tick || !tick->is(Json::Kind::Number) || !std::isfinite(tick->number) || tick->number < 0 ||
        tick->number > 9007199254740991. || std::floor(tick->number) != tick->number)
      return error = "invalid recorded tick_ms", false;
    step.dpi = dpi->number; step.tick = Millis(tick->number);
    session.steps.push_back(std::move(step));
  }
  out = std::move(session); error.clear(); return true;
}

// A local-only client deliberately accepts no URL credentials, arbitrary paths,
// DNS names, proxy routing, or redirection to a non-loopback inference service.
inline bool parseLocalVisionEndpoint(const std::string &source, LocalVisionEndpoint &out, std::string &error) {
  LocalVisionEndpoint endpoint;
  std::size_t start{};
  if (source.rfind("http://", 0) == 0) { start = 7; endpoint.port = 80; }
  else if (source.rfind("https://", 0) == 0) { start = 8; endpoint.port = 443; endpoint.secure = true; }
  else return error = "endpoint must start with http:// or https://", false;
  auto pathAt = source.find('/', start);
  std::string host = source.substr(start, pathAt == std::string::npos ? std::string::npos : pathAt - start);
  std::string path = pathAt == std::string::npos ? "" : source.substr(pathAt);
  if (path != "" && path != "/" && path != "/v1" && path != "/v1/" && path != "/v1/chat/completions")
    return error = "endpoint path must be empty, /v1, or /v1/chat/completions", false;
  std::string port;
  if (host.rfind("[::1]", 0) == 0) {
    if (host.size() > 5) {
      if (host[5] != ':') return error = "invalid loopback endpoint", false;
      port = host.substr(6);
      if (port.empty()) return error = "empty endpoint port", false;
    }
    endpoint.host = "::1";
  } else {
    auto colon = host.find(':');
    if (colon != std::string::npos) { port = host.substr(colon + 1); host.resize(colon); if (port.empty()) return error = "empty endpoint port", false; }
    if (host != "localhost" && host != "127.0.0.1") return error = "only localhost, 127.0.0.1, and [::1] are allowed", false;
    endpoint.host = "127.0.0.1";
  }
  if (!port.empty()) {
    unsigned value{};
    if (port.size() > 5) return error = "invalid endpoint port", false;
    for (unsigned char ch : port) { if (ch < '0' || ch > '9') return error = "invalid endpoint port", false; value = value * 10 + ch - '0'; }
    if (!value || value > 65535) return error = "invalid endpoint port", false;
    endpoint.port = static_cast<unsigned short>(value);
  }
  out = endpoint; error.clear(); return true;
}

inline bool visualActionAllowed(CNPage page, Action action) {
  switch (page) {
  case CNPage::Home: case CNPage::Play: case CNPage::More: case CNPage::Browser:
  case CNPage::Join: case CNPage::DeathmatchStart:
    return action == Action::Click;
  case CNPage::Hero: return action == Action::Click || action == Action::Enter;
  case CNPage::Room: return action == Action::Ready;
  case CNPage::Menu: return action == Action::Escape || action == Action::Ignore;
  case CNPage::Login: return action == Action::Enter || action == Action::Wait;
  case CNPage::Results: case CNPage::Leave: case CNPage::ConfirmLeave:
    return action == Action::Click || action == Action::Escape || action == Action::Enter;
  case CNPage::Queue: return action == Action::Wait;
  case CNPage::Launch: return action == Action::Launch;
  default: return false;
  }
}
inline bool parseVisualAnnotation(const std::string &source, VisualAnnotation &out, std::string &error) {
  if (source.size() > 65536) return error = "model annotation is too large", false;
  Json root;
  if (!parseJson(source, root, error) || !root.is(Json::Kind::Object)) return error = "model must return one JSON object: " + error, false;
  for (const auto &[key, value] : root.members) {
    (void)value;
    if (key != "page" && key != "action" && key != "label" && key != "click" && key != "confidence" && key != "anchors")
      return error = "unsupported model annotation field: " + key, false;
  }
  VisualAnnotation annotation;
  std::string page, action;
  if (!visualText(root.find("page"), page, 32) || !visualText(root.find("action"), action, 32) ||
      !visualText(root.find("label"), annotation.label, 384)) return error = "model page/action/label is missing", false;
  if (page == "none" && action == "observe") { annotation.page = CNPage::None; annotation.action = Action::Wait; }
  else if (!cnPageFromName(page, annotation.page) || !cnActionFromName(action, annotation.action) ||
           !visualActionAllowed(annotation.page, annotation.action)) return error = "unsupported model page/action combination", false;
  auto confidence = root.find("confidence"), click = root.find("click"), anchors = root.find("anchors");
  if (!confidence || !confidence->is(Json::Kind::Number) || !std::isfinite(confidence->number) ||
      confidence->number < 0 || confidence->number > 1) return error = "model confidence must be between 0 and 1", false;
  annotation.confidence = confidence->number;
  if (!click) return error = "model click must be a point or null", false;
  if (!click->is(Json::Kind::Null)) {
    if (!detail::readPoint(*click, annotation.click)) return error = "invalid model click", false;
    annotation.hasClick = true;
  }
  if (!anchors || !anchors->is(Json::Kind::Array) || anchors->items.size() > 6)
    return error = "model anchors must be an array of at most 6 regions", false;
  for (const auto &item : anchors->items) {
    VisualAnchor anchor;
    if (!item.is(Json::Kind::Object) || !item.find("roi") || !detail::readRect(*item.find("roi"), anchor.roi) ||
        !visualText(item.find("label"), anchor.label, 192)) return error = "invalid model anchor", false;
    for (const auto &[key, value] : item.members) { (void)value; if (key != "roi" && key != "label") return error = "unsupported anchor field", false; }
    annotation.anchors.push_back(std::move(anchor));
  }
  out = std::move(annotation); error.clear(); return true;
}
inline std::string writeVisualAnnotation(const VisualAnnotation &a) {
  std::string out = "{\"page\":" + visualJsonQuote(cnPageName(a.page)) + ",\"action\":" +
    visualJsonQuote(a.page == CNPage::None ? "observe" : cnActionName(a.action)) + ",\"label\":" + visualJsonQuote(a.label) +
    ",\"confidence\":" + formatNumber(a.confidence) + ",\"click\":";
  out += a.hasClick ? "[" + formatNumber(a.click.x) + "," + formatNumber(a.click.y) + "]" : "null";
  out += ",\"anchors\":[";
  for (std::size_t i = 0; i < a.anchors.size(); ++i) {
    auto r = a.anchors[i].roi;
    if (i) out += ',';
    out += "{\"label\":" + visualJsonQuote(a.anchors[i].label) + ",\"roi\":[" + formatNumber(r.x) + ',' +
      formatNumber(r.y) + ',' + formatNumber(r.w) + ',' + formatNumber(r.h) + "]}";
  }
  return out + "]}";
}
struct VisualCalibration {
  CNRule rule{};
  std::vector<CNLauncherAnchor> launcher;
  bool isLauncher{};
};
inline bool calibrateVisualStep(const TeachingStep &step, const VisualAnnotation &annotation, const Frame &frame,
                               VisualCalibration &out, std::string &error) {
  if (frame.width != step.width || frame.height != step.height || frame.pixels.size() != std::size_t(frame.width) * frame.height)
    return error = "image dimensions differ from recorded client geometry", false;
  if (!std::isfinite(step.dpi) || step.dpi < .5 || step.dpi > 4) return error = "invalid dpi", false;
  if (annotation.page == CNPage::None || !visualActionAllowed(annotation.page, annotation.action))
    return error = "model could not identify an actionable page", false;
  if (!std::isfinite(annotation.confidence) || annotation.confidence < .90 || annotation.confidence > 1)
    return error = "model confidence is below 0.90", false;
  if (step.action == "observe") return error = "observation is a proposal only; demonstrate the action before importing", false;
  bool launcher = step.scope == "launcher";
  if (launcher && (annotation.page != CNPage::Launch || annotation.action != Action::Launch || step.action != "click"))
    return error = "launcher calibration requires a demonstrated Play click", false;
  if (!launcher && (step.scope != "game" || annotation.page == CNPage::Launch)) return error = "model page disagrees with captured scope", false;
  if (annotation.page == CNPage::Room && step.action != "ready") return error = "room readiness requires the user's explicit ready marker", false;
  if (!launcher && step.action != cnActionName(annotation.action)) return error = "model action disagrees with the demonstrated action", false;
  if ((annotation.action == Action::Click || launcher) && !step.hasClick) return error = "missing recorded click", false;
  if (step.hasClick && (!std::isfinite(step.click.x) || !std::isfinite(step.click.y) || step.click.x < 0 || step.click.x > 1 || step.click.y < 0 || step.click.y > 1))
    return error = "recorded click is outside the client area", false;
  if (annotation.anchors.size() < 2 || annotation.anchors.size() > 6) return error = "need 2 to 6 independent stable anchors", false;
  VisualCalibration calibrated;
  calibrated.isLauncher = launcher;
  calibrated.rule = {annotation.page, utf8ToWide(annotation.label), annotation.action,
                     step.hasClick ? step.click : Point{}, double(frame.width) / frame.height, .965, {}};
  if (calibrated.rule.aspect < .5 || calibrated.rule.aspect > 4) return error = "unsupported frame aspect", false;
  for (std::size_t i = 0; i < annotation.anchors.size(); ++i) {
    const auto &anchor = annotation.anchors[i]; auto roi = anchor.roi;
    if (!valid(roi) || roi.w * frame.width < 6 || roi.h * frame.height < 6 || roi.w * roi.h > .15)
      return error = "anchor is too small, too large, or outside the screenshot", false;
    for (std::size_t j = 0; j < i; ++j) {
      auto other = annotation.anchors[j].roi;
      double area = std::max(0., std::min(roi.x + roi.w, other.x + other.w) - std::max(roi.x, other.x)) *
                    std::max(0., std::min(roi.y + roi.h, other.y + other.h) - std::max(roi.y, other.y));
      if (area > .75 * std::min(roi.w * roi.h, other.w * other.h)) return error = "anchors are duplicated or heavily overlapping", false;
    }
    auto pixels = sample(frame, roi);
    if (contrast(pixels) < 8) return error = "anchor has insufficient image contrast", false;
    calibrated.rule.anchors.push_back({roi, .965, {pixels}});
    if (launcher) {
      CNLauncherAnchor item;
      item.x = roi.x * frame.width / step.dpi;
      item.y = (roi.y >= .5 ? roi.y - 1 : roi.y) * frame.height / step.dpi;
      item.w = roi.w * frame.width / step.dpi; item.h = roi.h * frame.height / step.dpi;
      item.minimum = .965; item.variants.push_back(std::move(pixels));
      if (i == 0) {
        item.clickable = true; item.clickX = step.click.x * frame.width / step.dpi;
        item.clickY = (step.click.y >= .5 ? step.click.y - 1 : step.click.y) * frame.height / step.dpi;
      }
      calibrated.launcher.push_back(std::move(item));
    }
  }
  out = std::move(calibrated); error.clear(); return true;
}
// Candidate positions are measured from pixels before the model sees them.
// This removes the need for a small local model to invent precise coordinates.
// Candidates are suggestions only: the model still selects and labels them,
// and all selected anchors pass the ordinary two-frame verification below.
inline std::vector<Rect> stableVisualCandidates(const Frame &first, const Frame &second) {
  if (first.width != second.width || first.height != second.height || first.width < 64 || first.height < 64) return {};
  struct Candidate { Rect roi; double quality; };
  std::vector<Candidate> candidates;
  for (double y : {.015, .035, .055, .075, .10, .125, .79, .815, .84, .865, .89, .905, .92, .935}) {
    for (double width : {.022, .04, .065, .095}) for (double height : {.025, .04}) {
      for (int column = 1; column < 49; ++column) {
        Rect roi{column * .02, y, width, height};
        if (!valid(roi) || roi.w * first.width < 12 || roi.h * first.height < 8) continue;
        auto pixels = sample(first, roi), comparison = sample(second, roi);
        double spread = contrast(pixels), score = similarity(pixels, comparison);
        if (spread < 18 || contrast(comparison) < 18 || score < .97) continue;
        candidates.push_back({roi, score * 100 + std::min(70., spread) * .2 - roi.w * roi.h * 300 + (y >= .89 && roi.x >= .45 ? 8 : 0)});
      }
    }
  }
  std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b) { return a.quality > b.quality; });
  std::vector<Rect> selected;
  for (const auto &candidate : candidates) {
    auto roi = candidate.roi; bool overlaps = false;
    for (auto previous : selected) {
      double area = std::max(0., std::min(roi.x + roi.w, previous.x + previous.w) - std::max(roi.x, previous.x)) *
                    std::max(0., std::min(roi.y + roi.h, previous.y + previous.h) - std::max(roi.y, previous.y));
      if (area > .2 * std::min(roi.w * roi.h, previous.w * previous.h)) { overlaps = true; break; }
    }
    if (!overlaps) selected.push_back(roi);
    if (selected.size() >= 24) break;
  }
  return selected;
}
inline bool verifyVisualCalibration(const VisualCalibration &calibrated, const Frame &frame, std::string &error) {
  if (frame.width <= 0 || frame.height <= 0 || frame.pixels.size() != std::size_t(frame.width) * frame.height ||
      std::abs(double(frame.width) / frame.height / calibrated.rule.aspect - 1) > .035)
    return error = "verification frame geometry is incompatible", false;
  for (std::size_t i = 0; i < calibrated.rule.anchors.size(); ++i) {
    const auto &anchor = calibrated.rule.anchors[i];
    auto score = anchorScore(frame, anchor, false);
    if (score < anchor.minimum)
      return error = "anchor " + std::to_string(i + 1) + " changes between frames (score " + formatNumber(score) +
                     ", requires " + formatNumber(anchor.minimum) + ")", false;
  }
  error.clear(); return true;
}
} // namespace owh
