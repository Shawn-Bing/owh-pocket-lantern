#include "core/visual_calibration.hpp"
#include "core/cn_default_profile.hpp"
#include <cstdlib>
#include <iostream>
using namespace owh;
void require(bool result, const char *message) {
  if (!result) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
Frame calibrationFrame() {
  Frame frame{1280, 720, std::vector<std::uint32_t>(1280 * 720)};
  for (int y = 0; y < frame.height; ++y) for (int x = 0; x < frame.width; ++x)
    frame.pixels[std::size_t(y) * frame.width + x] = ((x / 5 + y / 7) % 2) ? 0xfff0f0f0 : 0xff101010;
  return frame;
}
int main() {
  std::string error;
  LocalVisionEndpoint endpoint;
  require(parseLocalVisionEndpoint("http://localhost:8080/v1", endpoint, error) && endpoint.host == "127.0.0.1" && endpoint.port == 8080,
          "localhost is pinned to numeric loopback");
  require(parseLocalVisionEndpoint("http://[::1]:8081/v1/chat/completions", endpoint, error) && endpoint.host == "::1", "IPv6 loopback supported");
  for (const char *url : {"http://127.0.0.1.attacker.example:8080", "http://192.168.1.2:8080", "http://127.0.0.1@other:8080",
                         "http://127.0.0.1:0", "http://127.0.0.1:65536", "http://[::1]evil:80", "http://localhost:8080/v1?x=y",
                         "http://localhost:", "file://localhost/test", "http://localhost:8080/redirect"})
    require(!parseLocalVisionEndpoint(url, endpoint, error), "non-loopback or unsupported URL rejected");
  require(safeTeachingImageName("frames/frame-001.bmp"), "relative recording subdirectory allowed");
  for (const char *name : {"../secret.bmp", "frames/../secret.bmp", "C:\\secret.bmp", "/secret.bmp", "\\\\server\\secret.bmp", "frame.bmp:stream", "frames//x.bmp", "frames./x.bmp"})
    require(!safeTeachingImageName(name), "recording path traversal rejected");
  std::string manifest = R"json({"format":"owh-teaching","version":1,"room_target":"最稳轮换版","steps":[{"image":"frame-001.bmp","scope":"game","action":"click","click":[0.23,0.67],"width":1280,"height":720,"dpi":1.25,"tick_ms":1000}]})json";
  TeachingSession session;
  require(parseTeachingSession(manifest, session, error) && session.steps.size() == 1 && session.steps[0].hasClick, "teaching manifest accepted");
  auto noClick = manifest; auto begin = noClick.find("\"click\":["); auto end = noClick.find(']', begin); noClick.erase(begin, end - begin + 2);
  require(!parseTeachingSession(noClick, session, error), "a click needs its recorded coordinate");
  require(!parseTeachingSession(R"json({"format":"owh-teaching","version":1,"room_target":"","steps":[]})json", session, error), "empty sessions rejected");
  require(parseTeachingSession(manifest, session, error), "restore valid session");
  auto pairedManifest = manifest;
  pairedManifest.insert(pairedManifest.find("\"scope\""), "\"verify_image\":\"verify-001.bmp\",");
  require(parseTeachingSession(pairedManifest, session, error) && session.steps[0].verificationImage == "verify-001.bmp", "recorded verify_image accepted");
  auto escapeVerification = pairedManifest;
  escapeVerification.replace(escapeVerification.find("verify-001.bmp"), 14, "../private.bmp");
  require(!parseTeachingSession(escapeVerification, session, error), "verification image traversal rejected");
  require(parseTeachingSession(manifest, session, error), "restore unpaired manifest");
  auto step = session.steps.front();
  std::string model = R"json({"page":"home","action":"click","label":"开始与社交","confidence":0.98,"click":[0.9,0.1],"anchors":[{"label":"开始","roi":[0.1,0.1,0.2,0.06]},{"label":"社交","roi":[0.6,0.5,0.1,0.06]}]})json";
  VisualAnnotation annotation;
  require(parseVisualAnnotation(model, annotation, error), "strict visual annotation accepted");
  auto frame = calibrationFrame(); VisualCalibration result;
  require(calibrateVisualStep(step, annotation, frame, result, error), "valid demonstrated click calibrates");
  require(result.rule.click.x == step.click.x && result.rule.click.y == step.click.y, "model cannot replace a recorded click");
  require(verifyVisualCalibration(result, frame, error), "unchanged anchors pass independent frame verification");
  require(!stableVisualCandidates(frame, frame).empty(), "stable high-contrast measured coordinate candidates found");
  auto flatCandidate = frame; std::fill(flatCandidate.pixels.begin(), flatCandidate.pixels.end(), 0xff303030);
  require(stableVisualCandidates(frame, flatCandidate).empty(), "unstable or flat patches cannot be model coordinate candidates");
  auto changed = frame;
  const auto changedRoi = annotation.anchors.front().roi;
  for (int y = int(changedRoi.y * changed.height); y < int((changedRoi.y + changedRoi.h) * changed.height); ++y)
    for (int x = int(changedRoi.x * changed.width); x < int((changedRoi.x + changedRoi.w) * changed.width); ++x)
      changed.pixels[std::size_t(y) * changed.width + x] = 0xff303030;
  require(!verifyVisualCalibration(result, changed, error), "changing scenery cannot pass a second-frame anchor check");
  require(result.rule.anchors.size() == 2 && result.rule.anchors.front().variants.front() == sample(frame, annotation.anchors.front().roi), "templates sampled from actual native pixels");
  CNProfile profile;
  require(parseCNProfile(defaultCNProfileJson(), profile, error), "default profile parses");
  profile.rules = {result.rule};
  CNProfile restored;
  require(parseCNProfile(writeCNProfile(profile), restored, error), "calibration profile is importable");
  auto observed = recognizeCNGame(frame, restored.rules, restored.join);
  require(observed.page == CNPage::Home && observed.step.click.x == step.click.x, "generated template replay keeps real click");
  auto observeStep = step; observeStep.action = "observe";
  require(!calibrateVisualStep(observeStep, annotation, frame, result, error), "self-check proposals cannot become automatic rules");
  auto uncertain = annotation; uncertain.confidence = .899;
  require(!calibrateVisualStep(step, uncertain, frame, result, error), "low-confidence annotations rejected");
  auto overlap = annotation; overlap.anchors[1].roi = overlap.anchors[0].roi;
  require(!calibrateVisualStep(step, overlap, frame, result, error), "duplicate anchors cannot supply independent evidence");
  auto outside = annotation; outside.anchors[0].roi = {.95, .1, .2, .1};
  require(!calibrateVisualStep(step, outside, frame, result, error), "out-of-frame anchors rejected");
  auto flat = frame; std::fill(flat.pixels.begin(), flat.pixels.end(), 0xff303030);
  require(!calibrateVisualStep(step, annotation, flat, result, error), "flat anchors rejected");
  auto mismatch = step; mismatch.width = 1920;
  require(!calibrateVisualStep(mismatch, annotation, frame, result, error), "stale window geometry rejected");
  auto room = annotation; room.page = CNPage::Room; room.action = Action::Ready;
  require(!calibrateVisualStep(step, room, frame, result, error), "ordinary clicks cannot authorize room readiness");
  auto ready = step; ready.action = "ready"; ready.hasClick = false;
  require(calibrateVisualStep(ready, room, frame, result, error), "explicit room-ready marker may authorize a room template");
  auto combat = model; combat.replace(combat.find("home"), 4, "combat"); combat.replace(combat.find("\"click\""), 7, "\"combat\"");
  require(!parseVisualAnnotation(combat, annotation, error), "model cannot introduce unrestricted combat actions");
  auto injected = model; injected.insert(injected.size() - 1, ",\"command\":\"anything\"");
  require(!parseVisualAnnotation(injected, annotation, error), "extra model execution fields rejected");
  require(parseVisualAnnotation(model, annotation, error), "restore annotation");
  auto launcher = step; launcher.scope = "launcher"; launcher.dpi = 1; launcher.click = {.16, .89};
  annotation.page = CNPage::Launch; annotation.action = Action::Launch;
  annotation.anchors[1].roi = {.08, .82, .18, .06};
  require(calibrateVisualStep(launcher, annotation, frame, result, error) && result.isLauncher, "launcher real click and logo calibration");
  auto rect = launcherRect(result.launcher[1], frame.width, frame.height, launcher.dpi);
  require(std::abs(rect.y - .82) < 1e-12 && result.launcher[1].y < 0 && result.launcher.front().clickY < 0, "bottom anchored launcher geometry roundtrips");
  auto launchObserved = recognizeCNLauncher(frame, 1, result.launcher);
  require(launchObserved.page == CNPage::Launch && std::abs(launchObserved.step.click.y - launcher.click.y) < 1e-12, "launcher replay preserves real click");
  std::cout << "Visual calibration contracts, local-only endpoint, demonstrated coordinates, native anchors and profile replay passed\n";
}
