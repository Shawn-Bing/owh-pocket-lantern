#pragma once
#include "agent/platform.hpp"
#include <objidl.h>
#include <gdiplus.h>
#include <array>
#include <iomanip>
#include <memory>

namespace owh::teach {
inline constexpr std::size_t MaxSteps = 128;
inline constexpr std::uint64_t MaxBytes = 256ULL * 1024 * 1024;
inline constexpr Millis MaxDuration = 10 * 60 * 1000;
inline constexpr Millis MaxFrameAge = 300;
// Bound the retained pre-click frames as well as files. This accepts 4K and
// ordinary ultrawide clients; at most three queued snapshots are retained.
inline constexpr std::size_t MaxPixels = 3840ULL * 2160;
inline constexpr wchar_t StopEvent[] = L"Local\\OverwatchHelper.Teaching.Stop";

inline std::filesystem::path sessionPath(const std::filesystem::path &root);

struct Options {
  std::filesystem::path output;
  std::wstring gameExe{L"Overwatch.exe"}, target{L"最稳轮换版"};
  bool selfTest{};
};
inline Options parseOptions(const std::vector<std::wstring> &args) {
  Options out;
  for (std::size_t i = 1; i < args.size(); ++i) {
    const auto &arg = args[i];
    if ((arg == L"--output" || arg == L"--game-exe" || arg == L"--target") && i + 1 < args.size()) {
      const auto &value = args[++i];
      if (arg == L"--output") out.output = value;
      else if (arg == L"--game-exe") out.gameExe = value;
      else out.target = value;
    } else if (arg == L"--self-test") out.selfTest = true;
    else throw std::runtime_error("Unknown or incomplete teaching recorder option");
  }
  if (out.gameExe.size() < 5 || out.gameExe.size() > 120 ||
      out.gameExe.find_first_of(L"\\/:\r\n\t\"") != std::wstring::npos ||
      _wcsicmp(out.gameExe.substr(out.gameExe.size() - 4).c_str(), L".exe"))
    throw std::runtime_error("--game-exe must be an executable filename");
  if (out.target.empty() || out.target.size() > 64 || out.target.find_first_of(L"\r\n\t") != std::wstring::npos)
    throw std::runtime_error("--target must contain 1..64 characters on one line");
  if (out.output.empty()) {
    if (out.selfTest) out.output = std::filesystem::temp_directory_path() / L"owh-teaching-tests";
    else out.output = sessionPath(std::filesystem::path(dataDirectory(false)) / L"teaching");
  }
  out.output = std::filesystem::absolute(out.output).lexically_normal();
  return out;
}

enum class RecordedAction { Click, Enter, Escape, Observe, Ready };
inline const char *actionName(RecordedAction action) {
  switch (action) {
  case RecordedAction::Click: return "click";
  case RecordedAction::Enter: return "enter";
  case RecordedAction::Escape: return "escape";
  case RecordedAction::Observe: return "observe";
  case RecordedAction::Ready: return "ready";
  }
  return "observe";
}
struct Snapshot {
  Process process;
  std::wstring executable;
  bool launcher{};
  RECT client{};
  double dpi{1};
  Millis tick{};
  unsigned inputEpoch{};
  Frame frame;
};
struct Event {
  std::shared_ptr<const Snapshot> before;
  RecordedAction action{RecordedAction::Observe};
  HWND window{};
  DWORD pid{};
  RECT client{};
  POINT screenPoint{};
  Millis tick{};
};
inline bool frameFits(const RECT &r) {
  const auto w = r.right - r.left, h = r.bottom - r.top;
  return w >= 80 && h >= 60 && w <= 7680 && h <= 4320 &&
         std::uint64_t(w) * std::uint64_t(h) <= MaxPixels;
}
inline bool eventMatches(const Event &event, Point &click) {
  if (!event.before) return false;
  const auto &shot = *event.before;
  if (!shot.process.window || !shot.process.pid || shot.process.window != event.window ||
      shot.process.pid != event.pid || !EqualRect(&shot.client, &event.client) ||
      !frameFits(shot.client) || shot.frame.width != shot.client.right - shot.client.left ||
      shot.frame.height != shot.client.bottom - shot.client.top ||
      shot.frame.pixels.size() != std::size_t(shot.frame.width) * shot.frame.height ||
      event.tick < shot.tick || event.tick - shot.tick > MaxFrameAge ||
      !std::isfinite(shot.dpi) || shot.dpi < .5 || shot.dpi > 8 ||
      (event.action == RecordedAction::Ready && shot.launcher)) return false;
  click = {};
  if (event.action == RecordedAction::Click) {
    if (!PtInRect(&shot.client, event.screenPoint)) return false;
    click.x = double(event.screenPoint.x - shot.client.left) / shot.frame.width;
    click.y = double(event.screenPoint.y - shot.client.top) / shot.frame.height;
  }
  return true;
}

struct PendingVerification {
  std::size_t step{};
  Process process;
  RECT client{};
  double dpi{1};
  unsigned inputEpoch{};
  Millis earliest{}, deadline{};
};
inline PendingVerification verificationFor(std::size_t step, const Event &event) {
  return {step, event.before->process, event.before->client, event.before->dpi,
          event.before->inputEpoch, event.tick + 1000, event.tick + 3000};
}
inline bool verificationMatches(const PendingVerification &pending, const Snapshot &shot,
                                 unsigned currentEpoch, Millis now) {
  return !shot.launcher && pending.process.window && pending.process.created &&
         pending.process.window == shot.process.window && pending.process.pid == shot.process.pid &&
         pending.process.created == shot.process.created && EqualRect(&pending.client, &shot.client) &&
         std::abs(pending.dpi - shot.dpi) < .001 && pending.inputEpoch == currentEpoch &&
         shot.inputEpoch == currentEpoch && shot.tick >= pending.earliest && shot.tick <= pending.deadline &&
         now >= shot.tick && now - shot.tick <= MaxFrameAge && frameFits(shot.client) &&
         shot.frame.width == shot.client.right - shot.client.left &&
         shot.frame.height == shot.client.bottom - shot.client.top &&
         shot.frame.pixels.size() == std::size_t(shot.frame.width) * shot.frame.height;
}

// The low-level hooks run on the UI thread and have one consumer. They never
// capture, encode, write files, enumerate processes, or wait for a disk lock.
class EventQueue {
  std::array<Event, 4> events_;
  std::atomic<std::size_t> read_{}, write_{};
public:
  bool push(Event event) {
    const auto index = write_.load(std::memory_order_relaxed);
    const auto next = (index + 1) % events_.size();
    if (next == read_.load(std::memory_order_acquire)) return false;
    events_[index] = std::move(event);
    write_.store(next, std::memory_order_release);
    return true;
  }
  bool pop(Event &event) {
    const auto index = read_.load(std::memory_order_relaxed);
    if (index == write_.load(std::memory_order_acquire)) return false;
    event = std::move(events_[index]);
    events_[index] = {};
    read_.store((index + 1) % events_.size(), std::memory_order_release);
    return true;
  }
};

inline std::string quote(const std::string &s) { return "\"" + jsonEscape(s) + "\""; }
struct Step {
  std::string image;
  bool launcher{};
  RecordedAction action{RecordedAction::Observe};
  Point click;
  int width{}, height{};
  double dpi{1};
  Millis tick{}, frameTick{};
  std::string verifyImage;
};
inline std::string manifest(const std::wstring &target, const std::vector<Step> &steps,
                            const std::string &reason) {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(12) << "{\"format\":\"owh-teaching\",\"version\":1,\"room_target\":"
      << quote(utf8(target)) << ",\"stopped_reason\":" << quote(reason) << ",\"steps\":[";
  bool comma = false;
  for (const auto &s : steps) {
    if (comma) out << ',';
    comma = true;
    out << "\n{\"image\":" << quote(s.image) << ",\"scope\":" << quote(s.launcher ? "launcher" : "game")
        << ",\"action\":" << quote(actionName(s.action)) << ",\"click\":[" << s.click.x << ',' << s.click.y
        << "],\"width\":" << s.width << ",\"height\":" << s.height << ",\"dpi\":" << s.dpi
        << ",\"tick_ms\":" << s.tick << ",\"frame_tick_ms\":" << s.frameTick;
    if (!s.verifyImage.empty()) out << ",\"verify_image\":" << quote(s.verifyImage);
    out << '}';
  }
  out << "\n]}\n";
  return out.str();
}
inline std::filesystem::path sessionPath(const std::filesystem::path &root) {
  SYSTEMTIME now{};
  GetLocalTime(&now);
  wchar_t name[100]{};
  swprintf(name, 100, L"teach-%04u%02u%02u-%02u%02u%02u-%lu", now.wYear, now.wMonth, now.wDay,
           now.wHour, now.wMinute, now.wSecond, GetCurrentProcessId());
  auto result = root / name;
  for (unsigned suffix = 1; std::filesystem::exists(result); ++suffix)
    result = root / (std::wstring(name) + L"-" + std::to_wstring(suffix));
  return result;
}
inline bool writeBytes(const std::filesystem::path &path, const void *data, std::size_t length) {
  // CREATE_NEW keeps an existing session file from being silently overwritten.
  Handle file(CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!file.h || file.h == INVALID_HANDLE_VALUE || length > MAXDWORD) return false;
  DWORD written{};
  return WriteFile(file.h, data, DWORD(length), &written, nullptr) && written == length && FlushFileBuffers(file.h);
}
inline bool pngBytes(const Frame &frame, std::vector<std::uint8_t> &bytes) {
  if (frame.width <= 0 || frame.height <= 0 || frame.pixels.size() > MaxPixels ||
      frame.pixels.size() != std::size_t(frame.width) * frame.height) return false;
  IStream *stream{};
  if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) return false;
  struct Release { IStream *stream; ~Release() { stream->Release(); } } owner{stream};
  const CLSID png = {0x557cf406, 0x1a04, 0x11d3, {0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
  Gdiplus::Bitmap bitmap(frame.width, frame.height, frame.width * 4, PixelFormat32bppRGB,
                        reinterpret_cast<BYTE *>(const_cast<std::uint32_t *>(frame.pixels.data())));
  if (bitmap.GetLastStatus() != Gdiplus::Ok || bitmap.Save(stream, &png, nullptr) != Gdiplus::Ok) return false;
  STATSTG stat{};
  if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)) || stat.cbSize.QuadPart > MaxPixels * 4 + 1024 * 1024) return false;
  bytes.resize(std::size_t(stat.cbSize.QuadPart));
  LARGE_INTEGER zero{};
  if (FAILED(stream->Seek(zero, STREAM_SEEK_SET, nullptr))) return false;
  ULONG read{};
  return SUCCEEDED(stream->Read(bytes.data(), ULONG(bytes.size()), &read)) && read == bytes.size();
}

class Store {
  const std::wstring target_;
  const std::uint64_t cap_;
  const std::size_t stepCap_;
  std::uint64_t imageBytes_{};
  std::vector<Step> steps_;
  unsigned revision_{};
  bool updateManifest(const std::string &reason) {
    const auto contents = manifest(target_, steps_, reason);
    if (imageBytes_ + contents.size() > cap_) { error = "size_limit"; return false; }
    // Each attempt gets its own name, preserving recovery evidence if a disk
    // error prevents the final rename. A successful rename is atomic on NTFS.
    auto temporary = directory / (L"manifest-pending-" + std::to_wstring(++revision_) + L".json");
    if (!writeBytes(temporary, contents.data(), contents.size()) ||
        !MoveFileExW(temporary.c_str(), (directory / L"manifest.json").c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
      error = "manifest_write_failed";
      return false;
    }
    return true;
  }
public:
  const std::filesystem::path directory;
  std::string error;
  explicit Store(std::filesystem::path path, std::wstring target,
                 std::uint64_t cap = MaxBytes, std::size_t stepCap = MaxSteps)
      : target_(std::move(target)), cap_(cap), stepCap_(stepCap), directory(std::move(path)) {
    std::filesystem::create_directories(directory.parent_path());
    if (!std::filesystem::create_directory(directory) && !std::filesystem::is_empty(directory))
      throw std::runtime_error("Teaching session directory must be empty");
    if (!updateManifest("recording")) throw std::runtime_error("Cannot create teaching manifest");
  }
  std::size_t size() const { return steps_.size(); }
  std::uint64_t bytes() const { return imageBytes_; }
  bool append(const Event &event) {
    Point click;
    if (!eventMatches(event, click)) { error = "invalid_pre_action_frame"; return false; }
    if (steps_.size() >= stepCap_) { error = "step_limit"; return false; }
    std::vector<std::uint8_t> image;
    if (!pngBytes(event.before->frame, image)) { error = "image_encode_failed"; return false; }
    // Leave room for the maximum bounded manifest even at the last step.
    if (image.size() + imageBytes_ + 128 * 1024 > cap_) { error = "size_limit"; return false; }
    char filename[32]{};
    std::snprintf(filename, sizeof(filename), "frame-%04u.png", unsigned(steps_.size() + 1));
    if (!writeBytes(directory / filename, image.data(), image.size())) { error = "image_write_failed"; return false; }
    imageBytes_ += image.size();
    const auto &shot = *event.before;
    steps_.push_back({filename, shot.launcher, event.action, click, shot.frame.width, shot.frame.height,
                      shot.dpi, event.tick, shot.tick});
    return updateManifest("recording");
  }
  bool attachVerification(std::size_t index, const Snapshot &shot) {
    if (index >= steps_.size() || steps_[index].action != RecordedAction::Ready || steps_[index].launcher ||
        shot.launcher || !steps_[index].verifyImage.empty() || shot.frame.width != steps_[index].width ||
        shot.frame.height != steps_[index].height || std::abs(shot.dpi - steps_[index].dpi) >= .001) {
      error = "invalid_verification_frame";
      return false;
    }
    std::vector<std::uint8_t> image;
    if (!pngBytes(shot.frame, image)) { error = "image_encode_failed"; return false; }
    if (image.size() + imageBytes_ + 128 * 1024 > cap_) { error = "size_limit"; return false; }
    char filename[32]{};
    std::snprintf(filename, sizeof(filename), "verify-%04u.png", unsigned(index + 1));
    if (!writeBytes(directory / filename, image.data(), image.size())) { error = "image_write_failed"; return false; }
    imageBytes_ += image.size();
    steps_[index].verifyImage = filename;
    return updateManifest("recording");
  }
  bool finish(const std::string &reason) { return updateManifest(reason); }
};
} // namespace owh::teach
