// Classifies recorded frames against the recognition profile.
//
// The profile says which pages the helper can act on. This tool says which pages
// actually occurred in a real session, how close each one came, and which anchor
// missed. That distinction is invisible at runtime: a page that was never
// recorded and a page whose threshold is slightly too strict both surface as a
// bare "未识别", yet the first needs a new step and the second needs tuning.
//
// It reads images through GDI+ like the replay test does, so it needs no Python
// or imaging library, and it never captures the screen, sends input or touches a
// game process.
// GDI+ needs the base Windows and COM headers first for PROPID and friends.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>
#include <shellapi.h>
#include <gdiplus.h>
#include "core/cn_default_profile.hpp"
#include "core/cn_profile.hpp"
#include "core/json.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace owh;

namespace {
std::wstring widen(const std::string &s) {
  if (s.empty())
    return {};
  int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
  std::wstring out(std::size_t(n), 0);
  MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), out.data(), n);
  return out;
}

// GDI+ must be started before any Bitmap is constructed, and it is easy to
// forget in a second entry point, so both commands share one guard.
struct GdiPlus {
  ULONG_PTR token{};
  bool ok{};
  GdiPlus() {
    Gdiplus::GdiplusStartupInput input;
    ok = Gdiplus::GdiplusStartup(&token, &input, nullptr) == Gdiplus::Ok;
  }
  ~GdiPlus() {
    if (ok)
      Gdiplus::GdiplusShutdown(token);
  }
  GdiPlus(const GdiPlus &) = delete;
  GdiPlus &operator=(const GdiPlus &) = delete;
};

bool loadFrame(const std::wstring &path, int cropTop, Frame &frame, std::string &error) {
  Gdiplus::Bitmap image(path.c_str());
  if (image.GetLastStatus() != Gdiplus::Ok) {
    error = "cannot read image";
    return false;
  }
  auto height = int(image.GetHeight()) - cropTop;
  frame.width = int(image.GetWidth());
  frame.height = height;
  if (frame.width <= 0 || frame.height <= 0) {
    error = "image smaller than the crop";
    return false;
  }
  Gdiplus::Rect rect(0, cropTop, frame.width, frame.height);
  Gdiplus::BitmapData data{};
  if (image.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok) {
    error = "cannot lock pixels";
    return false;
  }
  frame.pixels.resize(std::size_t(frame.width) * std::size_t(frame.height));
  for (int y = 0; y < frame.height; ++y)
    std::memcpy(frame.pixels.data() + std::size_t(y) * frame.width,
                static_cast<const char *>(data.Scan0) + std::ptrdiff_t(y) * data.Stride,
                std::size_t(frame.width) * 4);
  image.UnlockBits(&data);
  return true;
}

std::string readFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void usage() {
  std::cerr << "Usage: owh-profile scan --index <image-index.json> [--profile <cn-profile.json>]\n"
               "                   [--crop-top <pixels>] [--worst <count>] [--rows]\n"
               "       owh-profile add --profile <in.json> --image <screenshot>\n"
               "                   --page <page> --label <label> --action <action>\n"
               "                   --roi <x,y,w,h[:min]> [--roi ...] [--minimum <m>]\n"
               "                   [--click <x,y>] [--crop-top <pixels>] [--out <out.json>]\n"
               "                   [--index <image-index.json>]\n"
               "\n"
               "scan classifies every frame listed in the index and reports which profile page\n"
               "it matched, or the closest page and the anchor that missed. --rows prints one\n"
               "line per frame, which is what a missing step is found in.\n"
               "\n"
               "add samples ROI fractions from a screenshot into a new rule. Anchor thresholds\n"
               "default to --minimum; every anchor must pass. Pass --index to re-scan\n"
               "immediately, which is how a new rule is checked for false matches.\n";
}

struct Args {
  std::string command, index, profile, out, image, page, label, action;
  std::vector<std::string> rois;
  std::string click;
  double minimum = .90;
  int cropTop = 0, worst = 25;
  bool rows = false, replace = false;
};

// Windows hands main() an argv encoded in the ANSI code page, which turns any
// non-ASCII page label into mojibake. Re-read the command line as UTF-16 and
// convert, the same way the agent's own option parser does.
std::vector<std::string> utf8Args() {
  int count = 0;
  auto wide = CommandLineToArgvW(GetCommandLineW(), &count);
  std::vector<std::string> out;
  if (!wide)
    return out;
  out.reserve(std::size_t(count));
  for (int i = 0; i < count; ++i) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
    std::string text(size > 1 ? std::size_t(size - 1) : 0, 0);
    if (size > 1)
      WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, text.data(), size, nullptr, nullptr);
    out.push_back(std::move(text));
  }
  LocalFree(wide);
  return out;
}

bool parseArgs(const std::vector<std::string> &argv, Args &args, bool &help) {
  help = false;
  for (std::size_t i = 1; i < argv.size(); ++i) {
    const std::string &arg = argv[i];
    auto next = [&]() -> std::string { return i + 1 < argv.size() ? argv[++i] : std::string(); };
    if (arg == "scan" || arg == "add" || arg == "variant") {
      if (args.command.empty())
        args.command = arg;
      else
        return false;
    } else if (arg == "--index")
      args.index = next();
    else if (arg == "--profile")
      args.profile = next();
    else if (arg == "--out")
      args.out = next();
    else if (arg == "--image")
      args.image = next();
    else if (arg == "--page")
      args.page = next();
    else if (arg == "--label")
      args.label = next();
    else if (arg == "--action")
      args.action = next();
    else if (arg == "--click")
      args.click = next();
    else if (arg == "--roi")
      args.rois.push_back(next());
    else if (arg == "--minimum")
      args.minimum = std::atof(next().c_str());
    else if (arg == "--crop-top")
      args.cropTop = std::atoi(next().c_str());
    else if (arg == "--worst")
      args.worst = std::atoi(next().c_str());
    else if (arg == "--rows")
      args.rows = true;
    else if (arg == "--replace")
      args.replace = true;
    else if (arg == "--help" || arg == "-h")
      help = true;
    else {
      std::cerr << "Unknown argument: " << arg << '\n';
      return false;
    }
  }
  return true;
}

// "x,y,w,h" or "x,y,w,h:min", all as fractions of the client area.
bool parseRoi(const std::string &text, Rect &roi, double &minimum, double fallback) {
  auto colon = text.find(':');
  std::string numbers = colon == std::string::npos ? text : text.substr(0, colon);
  minimum = colon == std::string::npos ? fallback : std::atof(text.substr(colon + 1).c_str());
  double values[4]{};
  std::istringstream stream(numbers);
  std::string field;
  int count = 0;
  while (count < 4 && std::getline(stream, field, ','))
    values[count++] = std::atof(field.c_str());
  if (count != 4)
    return false;
  roi = {values[0], values[1], values[2], values[3]};
  return valid(roi) && minimum >= .5 && minimum <= 1;
}
// "x,y" as fractions of the client area.
bool readPointText(const std::string &text, Point &out) {
  double values[2]{};
  std::istringstream stream(text);
  std::string field;
  int count = 0;
  while (count < 2 && std::getline(stream, field, ','))
    values[count++] = std::atof(field.c_str());
  if (count != 2)
    return false;
  out = {values[0], values[1]};
  return std::isfinite(out.x) && std::isfinite(out.y) && out.x >= 0 && out.x <= 1 && out.y >= 0 &&
         out.y <= 1;
}
} // namespace

int runAdd(const Args &args);
int runScan(const Args &args);
int runVariant(const Args &args);

int main() {
  auto argv = utf8Args();
  Args args;
  bool help = false;
  if (argv.empty() || !parseArgs(argv, args, help) || help) {
    usage();
    return help ? 0 : 2;
  }
  if (args.command == "add")
    return runAdd(args);
  if (args.command == "variant")
    return runVariant(args);
  if (args.command == "scan")
    return runScan(args);
  usage();
  return 2;
}

int runAdd(const Args &args) {
  if (args.profile.empty() || args.image.empty() || args.rois.empty()) {
    usage();
    return 2;
  }
  CNProfile profile;
  std::string error;
  if (!parseCNProfile(readFile(args.profile), profile, error)) {
    std::cerr << "Cannot read " << args.profile << ": " << error << '\n';
    return 1;
  }
  Frame frame;
  // GDI+ must be started before any Bitmap is constructed.
  GdiPlus gdi;
  if (!gdi.ok) {
    std::cerr << "GDI+ startup failed\n";
    return 1;
  }
  if (!loadFrame(widen(args.image), args.cropTop, frame, error)) {
    std::cerr << "Cannot read " << args.image << ": " << error << '\n';
    return 1;
  }
  CNRule rule;
  if (!cnPageFromName(args.page, rule.page)) {
    std::cerr << "Unknown page: " << args.page << '\n';
    return 2;
  }
  if (!cnActionFromName(args.action, rule.action)) {
    std::cerr << "Unknown action: " << args.action << '\n';
    return 2;
  }
  if (rule.action == Action::Launch) {
    std::cerr << "Launch belongs in the launcher array, not in rules\n";
    return 2;
  }
  rule.label = utf8ToWide(args.label);
  rule.minimum = args.minimum;
  rule.aspect = double(frame.width) / frame.height;
  rule.click = {0, 0};
  if (!args.click.empty() && !readPointText(args.click, rule.click)) {
    std::cerr << "Bad --click: " << args.click << '\n';
    return 2;
  }
  for (const auto &spec : args.rois) {
    CNAnchor anchor;
    if (!parseRoi(spec, anchor.roi, anchor.minimum, args.minimum)) {
      std::cerr << "Bad --roi: " << spec << '\n';
      return 2;
    }
    auto image = sample(frame, anchor.roi);
    if (image.size() != std::size_t(TemplateW) * TemplateH || contrast(image) < 8) {
      std::cerr << "ROI samples to a flat or empty template: " << spec << '\n';
      return 2;
    }
    anchor.variants.push_back(std::move(image));
    rule.anchors.push_back(std::move(anchor));
  }
  std::cout << "image=" << frame.width << 'x' << frame.height << " aspect=" << rule.aspect << '\n';
  for (const auto &anchor : rule.anchors)
    std::cout << "  anchor roi=" << anchor.roi.x << ',' << anchor.roi.y << ',' << anchor.roi.w << ','
              << anchor.roi.h << " min=" << anchor.minimum
              << " contrast=" << contrast(anchor.variants.front()) << '\n';
  // Two error screens can share a page and action, so replacing by default would
  // silently drop the other one. Only --replace removes earlier rules.
  if (args.replace) {
    auto before = profile.rules.size();
    profile.rules.erase(std::remove_if(profile.rules.begin(), profile.rules.end(),
                                       [&](const CNRule &existing) {
                                         return existing.page == rule.page &&
                                                existing.action == rule.action;
                                       }),
                        profile.rules.end());
    if (before != profile.rules.size())
      std::cout << "replaced " << (before - profile.rules.size()) << " existing rule(s)\n";
  }
  profile.rules.push_back(std::move(rule));
  auto target = args.out.empty() ? args.profile : args.out;
  std::ofstream(target, std::ios::binary | std::ios::trunc) << writeCNProfile(profile);
  std::cout << "wrote " << target << " rules=" << profile.rules.size() << '\n';
  if (!args.index.empty()) {
    Args scan = args;
    scan.command = "scan";
    scan.profile = target;
    scan.rows = false;
    return runScan(scan);
  }
  return 0;
}

// Appends the sampled templates of an existing rule as extra variants.
//
// A page whose on-screen appearance varies -- the room HUD across spectating,
// results and lighting, or a reconnect screen mid-animation -- yields several
// distinct 32x16 samples of one requirement. Recording them as variants of a
// single anchor is what keeps the rule from sitting exactly on its threshold,
// which is where the room rule was: real frames missed by 0.0004.
int runVariant(const Args &args) {
  if (args.profile.empty() || args.image.empty() || (args.label.empty() && args.page.empty())) {
    usage();
    return 2;
  }
  CNProfile profile;
  std::string error;
  if (!parseCNProfile(readFile(args.profile), profile, error)) {
    std::cerr << "Cannot read " << args.profile << ": " << error << '\n';
    return 1;
  }
  CNRule *found = nullptr;
  for (auto &rule : profile.rules)
    if ((!args.label.empty() && wideToUtf8(rule.label) == args.label) ||
        (args.label.empty() && cnPageName(rule.page) == args.page)) {
      found = &rule;
      break;
    }
  if (!found) {
    std::cerr << "No rule matches label='" << args.label << "' page='" << args.page << "'\n";
    return 2;
  }
  Frame frame;
  GdiPlus gdi;
  if (!gdi.ok) {
    std::cerr << "GDI+ startup failed\n";
    return 1;
  }
  if (!loadFrame(widen(args.image), args.cropTop, frame, error)) {
    std::cerr << "Cannot read " << args.image << ": " << error << '\n';
    return 1;
  }
  if (std::abs(double(frame.width) / frame.height / found->aspect - 1) > .035) {
    std::cerr << "Frame aspect does not match the rule; it would never be compared\n";
    return 2;
  }
  std::size_t added = 0;
  for (auto &anchor : found->anchors) {
    auto image = sample(frame, anchor.roi);
    if (image.size() != std::size_t(TemplateW) * TemplateH || contrast(image) < 8)
      continue;
    if (std::find(anchor.variants.begin(), anchor.variants.end(), image) != anchor.variants.end())
      continue;
    anchor.variants.push_back(std::move(image));
    ++added;
  }
  auto target = args.out.empty() ? args.profile : args.out;
  std::ofstream(target, std::ios::binary | std::ios::trunc) << writeCNProfile(profile);
  std::cout << "added " << added << " variant(s) to \"" << wideToUtf8(found->label)
            << "\"; wrote " << target << '\n';
  return added ? 0 : 1;
}

int runScan(const Args &args) {
  CNProfile profile;
  std::string error;
  std::string source;
  if (!args.profile.empty() && parseCNProfile(readFile(args.profile), profile, error))
    source = args.profile;
  else if (!parseCNProfile(defaultCNProfileJson(), profile, error)) {
    std::cerr << "Embedded profile does not parse: " << error << '\n';
    return 1;
  } else
    source = "<embedded>";
  std::cout << "profile=" << source << " rules=" << profile.rules.size() << '\n';

  Json index;
  if (!parseJson(readFile(args.index), index, error) || !index.is(Json::Kind::Array)) {
    std::cerr << "Cannot read index " << args.index << ": " << (error.empty() ? "not an array" : error)
              << '\n';
    return 1;
  }

  GdiPlus gdi;
  if (!gdi.ok) {
    std::cerr << "GDI+ startup failed\n";
    return 1;
  }

  if (args.rows)
    std::cout << "n\tlocal\tmatched\taccepted\tscore\tclosest\tclosest_score\tmissed_anchor"
                 "\tmissed_score\tmissed_min\n";
  std::map<int, int> histogram;
  std::map<std::string, int> labels;
  int unreadable = 0, nearMisses = 0;
  struct Sample {
    double score;
    std::string line;
  };
  std::vector<Sample> worst;
  int total = 0;
  for (const auto &entry : index.items) {
    auto path = entry.find("path");
    auto local = entry.find("local");
    if (!path || !path->is(Json::Kind::String))
      continue;
    Frame frame;
    std::string failure;
    if (!loadFrame(widen(path->text), args.cropTop, frame, failure)) {
      ++unreadable;
      continue;
    }
    ++total;
    CNGameScore detail;
    auto result = recognizeCNGame(frame, profile.rules, profile.join, &detail);
    ++histogram[int(result.page)];
    if (result)
      ++labels[cnPageName(result.page)];

    // Name the first anchor that missed its own threshold; that is the anchor a
    // new variant or a lower threshold has to address.
    int missed = -1;
    double missedScore = 0, missedMinimum = 0;
    std::string closestLabel;
    double closestScore = 0;
    if (detail.evaluated && detail.rule >= 0 && std::size_t(detail.rule) < profile.rules.size()) {
      const auto &rule = profile.rules[std::size_t(detail.rule)];
      closestLabel = wideToUtf8(rule.label);
      closestScore = detail.best;
      for (std::size_t i = 0; i < detail.anchors.size() && i < rule.anchors.size(); ++i)
        if (detail.anchors[i] < rule.anchors[i].minimum) {
          missed = int(i);
          missedScore = detail.anchors[i];
          missedMinimum = rule.anchors[i].minimum;
          break;
        }
    }
    // A near miss is a frame that reached most of the threshold without being
    // accepted: exactly the frames worth looking at when tuning.
    if (!result && detail.evaluated && detail.best >= .75 && missed >= 0)
      ++nearMisses;

    std::ostringstream line;
    line << total << '\t' << (local && local->is(Json::Kind::String) ? local->text : "-") << '\t'
         << (result ? cnPageName(result.page) : "-") << '\t' << (result ? 1 : 0) << '\t'
         << (result ? result.score : 0) << '\t' << closestLabel << '\t' << closestScore << '\t'
         << missed << '\t' << missedScore << '\t' << missedMinimum;
    if (args.rows)
      std::cout << line.str() << '\n';
    worst.push_back({result ? result.score : closestScore, line.str() + '\n'});
  }

  // CNPage::None is page 0, so the histogram already holds the unmatched count.
  int unmatched = 0;
  if (auto entry = histogram.find(0); entry != histogram.end())
    unmatched = entry->second;
  std::cout << "\nframes=" << total << " unreadable=" << unreadable << '\n';
  std::cout << "matched:";
  if (labels.empty())
    std::cout << " none";
  for (const auto &entry : labels)
    std::cout << ' ' << entry.first << '=' << entry.second;
  std::cout << "\nunmatched=" << unmatched << " near_misses(>=0.75)=" << nearMisses << '\n';
  std::sort(worst.begin(), worst.end(),
            [](const Sample &a, const Sample &b) { return a.score < b.score; });
  std::cout << "\nclosest " << std::min<std::size_t>(std::size_t(args.worst), worst.size())
            << " frames by score (lowest first):\n";
  for (std::size_t i = 0; i < worst.size() && i < std::size_t(args.worst); ++i)
    std::cout << worst[i].line;
  return 0;
}
