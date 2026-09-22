#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#include "core/core.hpp"
#include "core/cn_default_profile.hpp"
#include "core/cn_profile.hpp"
#include <atomic>
#include <commdlg.h>
#include <cstring>
#include <dwmapi.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <shellapi.h>
#include <sstream>
#include <tlhelp32.h>
#include <windowsx.h>
#include <winhttp.h>

namespace owh {
inline std::string utf8(const std::wstring &s) {
  if (s.empty())
    return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0, nullptr, nullptr);
  std::string r(n, 0);
  WideCharToMultiByte(CP_UTF8, 0, s.data(), int(s.size()), r.data(), n, nullptr, nullptr);
  return r;
}
inline std::wstring wide(const std::string &s) {
  if (s.empty())
    return {};
  int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
  std::wstring r(n, 0);
  MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), r.data(), n);
  return r;
}
struct Handle {
  HANDLE h{};
  explicit Handle(HANDLE v = nullptr) : h(v) {}
  ~Handle() {
    if (h && h != INVALID_HANDLE_VALUE)
      CloseHandle(h);
  }
  Handle(const Handle &) = delete;
  Handle &operator=(const Handle &) = delete;
};
inline std::wstring exeDirectory() {
  wchar_t b[32768]{};
  GetModuleFileNameW(nullptr, b, 32768);
  return std::filesystem::path(b).parent_path().wstring();
}
inline std::wstring dataDirectory(bool demo) {
  wchar_t b[32768]{};
  GetEnvironmentVariableW(L"LOCALAPPDATA", b, 32768);
  auto p = std::filesystem::path(b) / (demo ? L"OverwatchHelperDemo" : L"OverwatchHelper");
  std::filesystem::create_directories(p);
  return p.wstring();
}
inline std::wstring readIni(const std::wstring &p, const wchar_t *section, const wchar_t *key,
                            const wchar_t *def = L"") {
  std::vector<wchar_t> b(16384);
  GetPrivateProfileStringW(section, key, def, b.data(), DWORD(b.size()), p.c_str());
  return b.data();
}
inline void writeIni(const std::wstring &p, const wchar_t *section, const wchar_t *key,
                     const std::wstring &v) {
  if (!std::filesystem::exists(p)) {
    Handle h(CreateFileW(p.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                         nullptr));
    WORD bom = 0xfeff;
    DWORD n;
    if (h.h != INVALID_HANDLE_VALUE)
      WriteFile(h.h, &bom, 2, &n, nullptr);
  }
  if (!WritePrivateProfileStringW(section, key, v.c_str(), p.c_str()))
    throw std::runtime_error("Cannot save configuration");
}
// WritePrivateProfileString with a null value deletes the key. Used to retire
// options that no longer exist: leaving a stale line in the user's settings file
// makes a removed feature look like live configuration.
inline void removeIni(const std::wstring &p, const wchar_t *section, const wchar_t *key) {
  if (std::filesystem::exists(p))
    WritePrivateProfileStringW(section, key, nullptr, p.c_str());
}
inline int number(const std::wstring &s, int def = 0) {
  try {
    return std::stoi(s);
  } catch (...) {
    return def;
  }
}
inline std::wstring discoverLauncher() {
  for (auto root : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER})
    for (auto view : {KEY_WOW64_32KEY, KEY_WOW64_64KEY}) {
      HKEY key{};
      if (RegOpenKeyExW(root,
                        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninsta"
                        L"ll\\Battle.net",
                        0, KEY_READ | view, &key) != ERROR_SUCCESS)
        continue;
      wchar_t path[32768]{};
      DWORD n = sizeof(path), type{};
      auto ok = RegQueryValueExW(key, L"InstallLocation", nullptr, &type,
                                 reinterpret_cast<BYTE *>(path), &n);
      RegCloseKey(key);
      if (ok == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)) {
        auto p = std::filesystem::path(path) / L"Battle.net.exe";
        if (std::filesystem::exists(p))
          return p.wstring();
      }
    }
  for (const auto *env : {L"ProgramFiles(x86)", L"ProgramFiles"}) {
    wchar_t b[32768]{};
    GetEnvironmentVariableW(env, b, 32768);
    auto p = std::filesystem::path(b) / L"Battle.net" / L"Battle.net.exe";
    if (std::filesystem::exists(p))
      return p.wstring();
  }
  return {};
}
// The recognition profile ships as data so steps can be edited, versioned and
// shared without rebuilding the executable. A local override wins over the
// shipped file, and the embedded copy is the last resort: an unattended recovery
// must not lose its navigation because a data file went missing.
struct CNProfileLoad {
  CNProfile profile;
  std::string source;
  std::string error;
};
inline CNProfileLoad loadCNProfile(const std::wstring &dir) {
  CNProfileLoad result;
  auto attempt = [&](const std::filesystem::path &path, const char *label) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
      return false;
    std::ostringstream buffer;
    buffer << file.rdbuf();
    CNProfile parsed;
    std::string error;
    if (!parseCNProfile(buffer.str(), parsed, error)) {
      result.error = std::string(label) + ": " + error;
      return false;
    }
    result.profile = std::move(parsed);
    result.source = label;
    result.error.clear();
    return true;
  };
  if (attempt(std::filesystem::path(dir) / L"cn-profile.json", "local"))
    return result;
  if (attempt(std::filesystem::path(exeDirectory()) / L"profiles" / L"cn-16x9.json", "shipped"))
    return result;
  CNProfile parsed;
  std::string error;
  if (parseCNProfile(defaultCNProfileJson(), parsed, error)) {
    result.profile = std::move(parsed);
    result.source = "builtin";
  } else if (result.error.empty()) {
    // Only reachable if the generated fallback itself is corrupt.
    result.error = "builtin: " + error;
  }
  return result;
}
// Bounds for the standby takeover delay. Below a few seconds the helper would
// treat ordinary reading as "away" and start pulsing into whatever the player is
// using; above ten minutes it would never take over during a real absence.
inline constexpr int StandbySecondsMin = 3;
inline constexpr int StandbySecondsMax = 600;
struct Config {
  std::wstring gameExe{L"Overwatch.exe"}, gameClass{L"TankWindowClass"}, launcherPath, launcherArgs;
  std::wstring server, token, machine, neacPath;
  bool dryRun{true}, holdShift{true}, autoRecover{true}, demo{};
  bool pulseShift{true};
  // While the player is at the computer the helper stands down entirely: they can
  // see a disconnect themselves, the pulses are global events that would land in
  // whatever application they are using, and captures would be spent on a screen
  // nobody is watching.
  bool standbyOnInput{true};
  // How long the player must stop producing input before the helper takes over.
  // Ten seconds distinguishes "reading or thinking mid-session" from "walked
  // away"; it is a setting because that judgement is the user's, not ours.
  int standbySeconds{10};
  bool cnPreset{true};
  double threshold{.90};
  CNProfile cnProfile;
  std::string cnProfileSource, cnProfileError;
  std::vector<Step> steps;
};
inline Config loadConfig(const std::wstring &dir, bool demo) {
  Config c;
  c.demo = demo;
  auto p = dir + L"\\settings.ini";
  c.gameExe = readIni(p, L"agent", L"game_exe", demo ? L"owh-demo.exe" : L"Overwatch.exe");
  c.gameClass = demo ? L"OWH-DemoGame" : L"TankWindowClass";
  c.launcherPath = readIni(p, L"agent", L"launcher_path");
  if (c.launcherPath.empty())
    c.launcherPath = demo ? exeDirectory() + L"\\owh-demo.exe" : discoverLauncher();
  c.launcherArgs = readIni(p, L"agent", L"launcher_args");
  c.server = readIni(p, L"agent", L"server");
  c.token = readIni(p, L"agent", L"token");
  wchar_t host[256]{};
  DWORD size = 256;
  GetComputerNameW(host, &size);
  c.machine = readIni(p, L"agent", L"machine", host);
  c.dryRun = number(readIni(p, L"agent", L"dry_run", L"1")) != 0;
  c.holdShift = number(readIni(p, L"agent", L"hold_shift", L"1")) != 0;
  c.autoRecover = number(readIni(p, L"agent", L"auto_recover", L"1")) != 0;
  c.pulseShift = number(readIni(p, L"agent", L"pulse_shift", L"1")) != 0;
  // A global pulse is delivered to whatever window has focus, so with the game in
  // the background it reached the player's application instead of the game and
  // could never help. The option is gone rather than defaulted off: keeping it
  // only invited that mistake.
  c.standbyOnInput = number(readIni(p, L"agent", L"standby_on_input", L"1")) != 0;
  c.standbySeconds = std::clamp(int(number(readIni(p, L"agent", L"standby_seconds", L"10"))),
                                StandbySecondsMin, StandbySecondsMax);
  c.cnPreset = !demo && number(readIni(p, L"agent", L"cn_preset", L"1")) != 0;
  c.neacPath = readIni(p, L"agent", L"neac_path");
  auto profile = loadCNProfile(dir);
  c.cnProfile = std::move(profile.profile);
  c.cnProfileSource = std::move(profile.source);
  c.cnProfileError = std::move(profile.error);
  int count = std::clamp(number(readIni(p, L"workflow", L"count")), 0, 32);
  for (int i = 0; i < count; ++i) {
    Step s;
    auto sec = L"step" + std::to_wstring(i);
    s.label = readIni(p, sec.c_str(), L"name");
    s.text = readIni(p, sec.c_str(), L"text");
    s.launcher = number(readIni(p, sec.c_str(), L"launcher")) != 0;
    int action = number(readIni(p, sec.c_str(), L"action"), -1);
    if (action < 0 || action > int(Action::Ignore))
      continue;
    s.action = Action(action);
    std::wistringstream v(readIni(p, sec.c_str(), L"geometry"));
    v >> s.roi.x >> s.roi.y >> s.roi.w >> s.roi.h >> s.click.x >> s.click.y >> s.aspect;
    auto hex = readIni(p, sec.c_str(), L"template");
    if (!v || !valid(s.roi) || !std::isfinite(s.aspect) || !std::isfinite(s.click.x) ||
        !std::isfinite(s.click.y) || s.aspect < .5 || s.aspect > 4 || s.click.x < 0 ||
        s.click.x > 1 || s.click.y < 0 || s.click.y > 1 || hex.size() != TemplateW * TemplateH * 2)
      continue;
    auto digit = [](wchar_t x) {
      if (x >= '0' && x <= '9')
        return x - '0';
      if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
      return -1;
    };
    bool ok = true;
    for (std::size_t j = 0; j < hex.size(); j += 2) {
      int hi = digit(hex[j]), lo = digit(hex[j + 1]);
      if (hi < 0 || lo < 0) {
        ok = false;
        break;
      }
      s.image.push_back(std::uint8_t(hi * 16 + lo));
    }
    if (ok && contrast(s.image) >= 8 && !(s.launcher && s.action == Action::Ready) &&
        !(!s.launcher && s.action == Action::Launch) && s.label.size() <= 128 &&
        s.text.size() <= 128)
      c.steps.push_back(std::move(s));
  }
  return c;
}
inline void saveConfig(const std::wstring &dir, const Config &c) {
  auto p = dir + L"\\settings.ini";
  for (auto kv : std::vector<std::pair<std::wstring, std::wstring>>{
           {L"game_exe", c.gameExe},
           {L"launcher_path", c.launcherPath},
           {L"launcher_args", c.launcherArgs},
           {L"server", c.server},
           {L"token", c.token},
           {L"machine", c.machine},
           {L"dry_run", c.dryRun ? L"1" : L"0"},
           {L"hold_shift", c.holdShift ? L"1" : L"0"},
           {L"pulse_shift", c.pulseShift ? L"1" : L"0"},
           {L"standby_on_input", c.standbyOnInput ? L"1" : L"0"},
           {L"standby_seconds", std::to_wstring(c.standbySeconds)},
           {L"cn_preset", c.cnPreset ? L"1" : L"0"},
           {L"neac_path", c.neacPath},
           {L"auto_recover", c.autoRecover ? L"1" : L"0"}})
    writeIni(p, L"agent", kv.first.c_str(), kv.second);
  writeIni(p, L"workflow", L"count", std::to_wstring(c.steps.size()));
  // Retired: the global background pulse no longer exists. Writing it with a
  // value would keep re-creating a setting nothing reads.
  removeIni(p, L"agent", L"background_pulse");
  for (std::size_t i = 0; i < c.steps.size(); ++i) {
    auto &s = c.steps[i];
    auto sec = L"step" + std::to_wstring(i);
    std::wostringstream geo;
    geo.precision(12);
    geo << s.roi.x << ' ' << s.roi.y << ' ' << s.roi.w << ' ' << s.roi.h << ' ' << s.click.x << ' '
        << s.click.y << ' ' << s.aspect;
    std::wstring hex;
    for (auto b : s.image) {
      hex += L"0123456789abcdef"[b >> 4];
      hex += L"0123456789abcdef"[b & 15];
    }
    for (auto kv : std::vector<std::pair<std::wstring, std::wstring>>{
             {L"name", s.label},
             {L"text", s.text},
             {L"launcher", s.launcher ? L"1" : L"0"},
             {L"action", std::to_wstring(int(s.action))},
             {L"geometry", geo.str()},
             {L"template", hex}})
      writeIni(p, sec.c_str(), kv.first.c_str(), kv.second);
  }
}
struct Process {
  DWORD pid{};
  std::uint64_t created{};
  HWND window{};
};
inline std::uint64_t processCreation(HANDLE p) {
  FILETIME c{}, e{}, k{}, u{};
  if (!GetProcessTimes(p, &c, &e, &k, &u))
    return 0;
  return (std::uint64_t(c.dwHighDateTime) << 32) | c.dwLowDateTime;
}
inline std::wstring processName(DWORD pid) {
  Handle p(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
  if (!p.h)
    return {};
  wchar_t b[32768]{};
  DWORD n = 32768;
  if (!QueryFullProcessImageNameW(p.h, 0, b, &n))
    return {};
  return std::filesystem::path(b).filename().wstring();
}
// Windows blocks input and foreground changes from a lower-integrity process
// towards a window owned by a higher-integrity one (UIPI). The mismatch is
// otherwise invisible: SendInput and SetForegroundWindow simply fail, or return
// success and do nothing. Compare the levels explicitly so the panel can name
// the real reason instead of reporting a generic focus or input failure.
enum class Integrity { Unknown, Untrusted, Low, Medium, High, System };
inline const wchar_t *integrityName(Integrity v) {
  switch (v) {
  case Integrity::Untrusted: return L"untrusted";
  case Integrity::Low: return L"low";
  case Integrity::Medium: return L"medium";
  case Integrity::High: return L"high";
  case Integrity::System: return L"system";
  default: return L"unknown";
  }
}
inline Integrity tokenIntegrity(HANDLE token) {
  DWORD size{};
  if (!token)
    return Integrity::Unknown;
  // The zero-length probe is expected to fail with ERROR_INSUFFICIENT_BUFFER;
  // any other failure means the level really is unavailable.
  if (GetTokenInformation(token, TokenIntegrityLevel, nullptr, 0, &size) ||
      GetLastError() != ERROR_INSUFFICIENT_BUFFER || size < sizeof(TOKEN_MANDATORY_LABEL))
    return Integrity::Unknown;
  std::vector<char> buffer(size);
  if (!GetTokenInformation(token, TokenIntegrityLevel, buffer.data(), size, &size))
    return Integrity::Unknown;
  auto label = reinterpret_cast<const TOKEN_MANDATORY_LABEL *>(buffer.data());
  if (!label->Label.Sid)
    return Integrity::Unknown;
  auto count = *GetSidSubAuthorityCount(label->Label.Sid);
  if (!count)
    return Integrity::Unknown;
  DWORD level = *GetSidSubAuthority(label->Label.Sid, DWORD(count - 1));
  if (level < SECURITY_MANDATORY_LOW_RID) return Integrity::Untrusted;
  if (level < SECURITY_MANDATORY_MEDIUM_RID) return Integrity::Low;
  if (level < SECURITY_MANDATORY_HIGH_RID) return Integrity::Medium;
  if (level < SECURITY_MANDATORY_SYSTEM_RID) return Integrity::High;
  return Integrity::System;
}
inline Integrity processIntegrity(DWORD pid) {
  if (!pid)
    return Integrity::Unknown;
  Handle p(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
  if (!p.h)
    return Integrity::Unknown;
  HANDLE raw{};
  if (!OpenProcessToken(p.h, TOKEN_QUERY, &raw))
    return Integrity::Unknown;
  Handle token(raw);
  return tokenIntegrity(token.h);
}
inline Integrity selfIntegrity() { return processIntegrity(GetCurrentProcessId()); }
inline bool selfElevated() {
  HANDLE raw{};
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &raw))
    return false;
  Handle token(raw);
  TOKEN_ELEVATION elevation{};
  DWORD size = sizeof(elevation);
  return GetTokenInformation(token.h, TokenElevation, &elevation, size, &size) &&
         elevation.TokenIsElevated != 0;
}
// True when the target outranks us, so Windows will refuse both our input and
// our foreground requests regardless of what the window is doing.
inline bool outranks(const Process &target) {
  auto mine = selfIntegrity(), theirs = processIntegrity(target.pid);
  return mine != Integrity::Unknown && theirs != Integrity::Unknown &&
         int(theirs) > int(mine);
}
inline Process findProcess(const std::wstring &name, const std::wstring &cls = L"") {
  Process p;
  // Prefer a verified main window, not an arbitrary same-name background
  // helper.
  struct Find {
    const std::wstring *name;
    const std::wstring *cls;
    Process *out;
    std::int64_t best{-1};
  } ctx{&name, &cls, &p};
  EnumWindows(
      [](HWND w, LPARAM a) -> BOOL {
        auto &x = *reinterpret_cast<Find *>(a);
        if (!IsWindowVisible(w))
          return TRUE;
        wchar_t cls[256]{};
        GetClassNameW(w, cls, 256);
        if (!x.cls->empty() && *x.cls != cls)
          return TRUE;
        DWORD pid{};
        GetWindowThreadProcessId(w, &pid);
        if (_wcsicmp(processName(pid).c_str(), x.name->c_str()) != 0)
          return TRUE;
        RECT client{};
        GetClientRect(w, &client);
        std::int64_t area = std::int64_t(std::max(0L,client.right))*std::max(0L,client.bottom);
        if (IsIconic(w)) {
          WINDOWPLACEMENT place{};place.length=sizeof(place);
          if (GetWindowPlacement(w,&place))
            area=std::int64_t(std::max(0L,place.rcNormalPosition.right-place.rcNormalPosition.left))*
                 std::max(0L,place.rcNormalPosition.bottom-place.rcNormalPosition.top);
        }
        // Battle.net also exposes tiny auxiliary Chromium windows.
        auto rank=area+(GetWindow(w,GW_OWNER)==nullptr ? (std::int64_t(1)<<40) : 0);
        if (rank<=x.best) return TRUE;
        x.best=rank;
        x.out->pid = pid;
        x.out->window = w;
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&ctx));
  if (!p.pid) {
    Handle snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    PROCESSENTRY32W e{};
    e.dwSize = sizeof(e);
    if (snap.h != INVALID_HANDLE_VALUE && Process32FirstW(snap.h, &e))
      do {
        if (_wcsicmp(e.szExeFile, name.c_str()) == 0) {
          p.pid = e.th32ProcessID;
          break;
        }
      } while (Process32NextW(snap.h, &e));
  }
  if (p.pid) {
    Handle h(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, p.pid));
    if (h.h)
      p.created = processCreation(h.h);
  }
  return p;
}
inline bool sameProcess(const Process &p, const std::wstring &name) {
  if (!p.pid || !p.created)
    return false;
  Handle h(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, p.pid));
  return h.h && processCreation(h.h) == p.created &&
         _wcsicmp(processName(p.pid).c_str(), name.c_str()) == 0;
}
inline bool responsive(HWND w) {
  DWORD_PTR result{};
  return SendMessageTimeoutW(w, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 200, &result) != 0;
}
inline bool foreground(HWND w) { return w && GetForegroundWindow() == w && !IsIconic(w); }
// Windows only grants a foreground change to a process that owns the foreground
// lock. A refused SetForegroundWindow reports nothing useful and merely flashes
// the taskbar button, so one request is never enough: the caller has to verify
// the result and retry. Sharing the input queue with the current foreground
// thread satisfies the same check without changing any system setting.
inline bool requestForeground(HWND w) {
  if (!w || !IsWindow(w) || !IsWindowVisible(w))
    return false;
  if (IsIconic(w))
    ShowWindow(w, SW_RESTORE);
  if (foreground(w))
    return true;
  SetWindowPos(w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  SetForegroundWindow(w);
  if (foreground(w))
    return true;
  auto current = GetForegroundWindow();
  DWORD other = current ? GetWindowThreadProcessId(current, nullptr) : 0;
  DWORD mine = GetCurrentThreadId();
  if (other && other != mine && AttachThreadInput(mine, other, TRUE)) {
    ShowWindow(w, SW_RESTORE);
    SetForegroundWindow(w);
    BringWindowToTop(w);
    AttachThreadInput(mine, other, FALSE);
  }
  return foreground(w);
}
inline bool interactiveDesktop() {
  HDESK input = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
  if (!input) return false;
  wchar_t a[256]{}, b[256]{};
  DWORD needed{};
  bool ok = GetUserObjectInformationW(input, UOI_NAME, a, sizeof(a), &needed) &&
            GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()), UOI_NAME,
                                      b, sizeof(b), &needed) && _wcsicmp(a, b) == 0;
  CloseDesktop(input);
  return ok;
}
inline bool cloaked(HWND w) {
  DWORD value{};
  return SUCCEEDED(DwmGetWindowAttribute(w, DWMWA_CLOAKED, &value, sizeof(value))) && value;
}
inline bool screenClientRect(HWND w, RECT &r) {
  POINT origin{};
  if (!GetClientRect(w, &r) || !ClientToScreen(w, &origin)) return false;
  OffsetRect(&r, origin.x, origin.y);
  return true;
}
inline bool passiveGameOverlay(const std::wstring &process, const std::wstring &cls, LONG_PTR style) {
  // WS_EX_TRANSPARENT alone describes input/painting order, not visual transparency.
  // Only allow the NVIDIA passive HUD identified on the affected machine. Its
  // per-pixel overlay can span the desktop even when no HUD pixels are drawn.
  constexpr LONG_PTR flags = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
  return (style & flags) == flags && _wcsicmp(process.c_str(), L"NVIDIA Overlay.exe") == 0 &&
         cls == L"CEF-OSC-WIDGET";
}
inline bool ignorableCaptureOverlay(HWND w) {
  if (cloaked(w)) return true;
  auto style = GetWindowLongPtrW(w, GWL_EXSTYLE);
  if (!(style & WS_EX_LAYERED)) return false;
  COLORREF color{};
  BYTE alpha{};
  DWORD flags{};
  if (GetLayeredWindowAttributes(w, &color, &alpha, &flags) && (flags & LWA_ALPHA) && alpha == 0)
    return true;
  wchar_t cls[256]{};
  GetClassNameW(w, cls, 256);
  if (wcscmp(cls, L"CEF-OSC-WIDGET") != 0) return false;
  DWORD pid{};
  GetWindowThreadProcessId(w, &pid);
  return passiveGameOverlay(processName(pid), cls, style);
}
inline HWND captureBlocker(HWND w, const RECT &client) {
  for (HWND other = GetWindow(w, GW_HWNDPREV); other; other = GetWindow(other, GW_HWNDPREV)) {
    RECT r{}, intersection{};
    if (IsWindowVisible(other) && !IsIconic(other) && GetWindowRect(other, &r) &&
        IntersectRect(&intersection, &r, &client) && !ignorableCaptureOverlay(other)) return other;
  }
  return nullptr;
}
inline bool unobscured(HWND w) {
  RECT client{};
  return w && IsWindowVisible(w) && !IsIconic(w) && !cloaked(w) && screenClientRect(w, client) &&
         !captureBlocker(w, client);
}
struct CaptureFailure {
  std::string code{"ok"};
  DWORD windowsError{}, blockerPid{};
  std::wstring blockerProcess;
  std::wstring message() const {
    if (code == "occluded") return L"目标画面被 " +
        (blockerProcess.empty() ? L"其他窗口" : blockerProcess) + L" 遮挡，请移开该窗口。";
    if (code == "not_foreground") return L"目标窗口没有获得前台焦点，请切回目标窗口后重试。";
    if (code == "minimized") return L"目标窗口已最小化，请还原窗口。";
    if (code == "window_unavailable") return L"目标窗口不存在或未显示，请确认目标程序已打开。";
    if (code == "desktop_unavailable") return L"当前桌面不可捕获，请解锁或完成系统授权后重试。";
    if (code == "offscreen") return L"目标画面超出屏幕边缘，请把整个窗口移回屏幕内。";
    if (code == "invalid_size") return L"窗口客户区大小不支持，请使用正常大小的窗口模式。";
    if (code == "window_changed") return L"截图时窗口位置或大小发生变化，请停稳后重试。";
    return L"画面读取失败（" + wide(code) + L"，Windows 错误 " + std::to_wstring(windowsError) + L"）。";
  }
};
inline CaptureFailure captureAvailability(HWND w, bool allowBackground) {
  if (!w || !IsWindow(w)) return {"window_unavailable"};
  if (!interactiveDesktop()) return {"desktop_unavailable"};
  if (IsIconic(w)) return {"minimized"};
  if (!IsWindowVisible(w) || cloaked(w)) return {"window_unavailable"};
  if (!allowBackground && !foreground(w)) return {"not_foreground"};
  RECT client{};
  if (!screenClientRect(w, client)) return {"geometry_failed", GetLastError()};
  auto blocker = captureBlocker(w, client);
  if (!blocker) return {};
  DWORD pid{};
  GetWindowThreadProcessId(blocker, &pid);
  return {"occluded", 0, pid, processName(pid)};
}
class Capture {
  HDC dc_{};
  HBITMAP bitmap_{}, old_{};
  void *bits_{};
  int w_{}, h_{};
  CaptureFailure failure_;
  void release() {
    if (dc_ && old_)
      SelectObject(dc_, old_);
    if (bitmap_)
      DeleteObject(bitmap_);
    if (dc_)
      DeleteDC(dc_);
    dc_ = nullptr;
    bitmap_ = old_ = nullptr;
    bits_ = nullptr;
    w_ = h_ = 0;
  }

public:
  ~Capture() { release(); }
  const CaptureFailure &failure() const { return failure_; }
  bool grab(HWND window, Frame &f, bool allowBackground = false) {
    // A failed attempt must never expose a previous successful frame.
    f.width = f.height = 0;
    f.pixels.clear();
    failure_ = captureAvailability(window, allowBackground);
    if (failure_.code != "ok") return false;
    auto fail = [&](const char *code, DWORD error = 0) {
      failure_ = {code, error};
      return false;
    };
    RECT r{};
    if (!screenClientRect(window, r)) return fail("geometry_failed", GetLastError());
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w < 80 || h < 60 || w > 7680 || h > 4320)
      return fail("invalid_size");
    POINT p{r.left, r.top};
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN), vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    if (p.x < vx || p.y < vy || p.x + w > vx + GetSystemMetrics(SM_CXVIRTUALSCREEN) ||
        p.y + h > vy + GetSystemMetrics(SM_CYVIRTUALSCREEN))
      return fail("offscreen");
    HDC desktop = GetDC(nullptr);
    if (!desktop)
      return fail("desktop_dc_failed", GetLastError());
    if (w != w_ || h != h_ || !dc_ || !bitmap_ || !bits_) {
      release();
      dc_ = CreateCompatibleDC(desktop);
      BITMAPINFO bi{};
      bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bi.bmiHeader.biWidth = w;
      bi.bmiHeader.biHeight = -h;
      bi.bmiHeader.biPlanes = 1;
      bi.bmiHeader.biBitCount = 32;
      bi.bmiHeader.biCompression = BI_RGB;
      bitmap_ = CreateDIBSection(desktop, &bi, DIB_RGB_COLORS, &bits_, nullptr, 0);
      if (dc_ && bitmap_)
        old_ = static_cast<HBITMAP>(SelectObject(dc_, bitmap_));
      w_ = w;
      h_ = h;
    }
    BOOL ok =
        dc_ && bitmap_ && bits_ && BitBlt(dc_, 0, 0, w, h, desktop, p.x, p.y, SRCCOPY | CAPTUREBLT);
    DWORD copyError = ok ? ERROR_SUCCESS : GetLastError();
    ReleaseDC(nullptr, desktop);
    if (!ok) return fail("bitblt_failed", copyError);
    if (!GdiFlush()) return fail("gdi_flush_failed", GetLastError());
    failure_ = captureAvailability(window, allowBackground);
    if (failure_.code != "ok") return false;
    RECT after{};
    if (!screenClientRect(window, after) || !EqualRect(&r, &after)) return fail("window_changed");
    f.width = w;
    f.height = h;
    f.pixels.resize(std::size_t(w) * h);
    std::memcpy(f.pixels.data(), bits_, f.pixels.size() * 4);
    return true;
  }
};
// PrintWindow asks the window to render itself into a DC, so unlike a screen copy
// it can return content for an occluded window. It is measured here rather than
// trusted: a DirectX surface may come back black or stale, and acting on a stale
// frame during recovery would be worse than refusing to capture at all.
#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT 0x00000002
#endif
inline bool printWindowFrame(HWND window, Frame &f) {
  RECT r{};
  if (!window || !screenClientRect(window, r))
    return false;
  int w = r.right - r.left, h = r.bottom - r.top;
  if (w < 80 || h < 60 || w > 7680 || h > 4320)
    return false;
  HDC desktop = GetDC(nullptr);
  if (!desktop)
    return false;
  HDC dc = CreateCompatibleDC(desktop);
  BITMAPINFO bi{};
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = w;
  bi.bmiHeader.biHeight = -h;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  bi.bmiHeader.biCompression = BI_RGB;
  void *bits = nullptr;
  HBITMAP bitmap = CreateDIBSection(desktop, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
  ReleaseDC(nullptr, desktop);
  if (!dc || !bitmap || !bits) {
    if (bitmap) DeleteObject(bitmap);
    if (dc) DeleteDC(dc);
    return false;
  }
  auto old = SelectObject(dc, bitmap);
  BOOL ok = PrintWindow(window, dc, PW_RENDERFULLCONTENT);
  GdiFlush();
  bool usable = ok != FALSE;
  if (usable) {
    f.width = w;
    f.height = h;
    f.pixels.resize(std::size_t(w) * std::size_t(h));
    std::memcpy(f.pixels.data(), bits, f.pixels.size() * 4);
  }
  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
  return usable;
}
inline int lumaOf(std::uint32_t p) {
  return (int((p >> 16) & 255) * 30 + int((p >> 8) & 255) * 59 + int(p & 255) * 11) / 100;
}
// How much structure a capture carries. An all-black DirectX surface and a real
// screenshot both "succeed"; only the numbers tell them apart.
struct FrameInk {
  double ink{};       // fraction of pixels that are not near black
  double deviation{}; // luminance standard deviation
};
inline FrameInk frameInk(const Frame &f) {
  FrameInk result;
  if (f.pixels.empty())
    return result;
  double sum = 0;
  std::size_t lit = 0;
  for (auto p : f.pixels) {
    int luma = lumaOf(p);
    if (luma > 12)
      ++lit;
    sum += luma;
  }
  double mean = sum / f.pixels.size();
  double variance = 0;
  for (auto p : f.pixels)
    variance += (lumaOf(p) - mean) * (lumaOf(p) - mean);
  result.ink = double(lit) / f.pixels.size();
  result.deviation = std::sqrt(variance / f.pixels.size());
  return result;
}
// Mean absolute luminance difference, used to tell a live render from a stale one.
inline double frameDifference(const Frame &a, const Frame &b) {
  if (a.width != b.width || a.height != b.height || a.pixels.empty())
    return -1;
  double sum = 0;
  for (std::size_t i = 0; i < a.pixels.size(); ++i)
    sum += std::abs(lumaOf(a.pixels[i]) - lumaOf(b.pixels[i]));
  return sum / a.pixels.size();
}
// Writes a 32-bit BMP. The helper links no image encoder, and the point of saving
// an unmatched frame is diagnosis: a page that missed its threshold by a
// thousandth needs a template sampled from the client's real pixels, not from a
// recording that was captured at a different size.
inline bool saveFrameBmp(const Frame &f, const std::filesystem::path &path) {
  if (f.width <= 0 || f.height <= 0 || f.pixels.size() != std::size_t(f.width) * f.height)
    return false;
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  auto u16 = [&](std::uint16_t v) { out.write(reinterpret_cast<const char *>(&v), 2); };
  auto u32 = [&](std::uint32_t v) { out.write(reinterpret_cast<const char *>(&v), 4); };
  auto i32 = [&](std::int32_t v) { out.write(reinterpret_cast<const char *>(&v), 4); };
  std::uint32_t pixels = std::uint32_t(f.pixels.size()) * 4;
  out.put('B');
  out.put('M');
  u32(14 + 40 + pixels);
  u32(0);
  u32(14 + 40);
  u32(40);
  u32(std::uint32_t(f.width));
  // Negative height marks top-down rows, which is the order Frame stores.
  i32(-f.height);
  u16(1);
  u16(32);
  u32(0);
  u32(pixels);
  u32(2835);
  u32(2835);
  u32(0);
  u32(0);
  out.write(reinterpret_cast<const char *>(f.pixels.data()), std::ptrdiff_t(f.pixels.size()) * 4);
  return out.good();
}
class Input {
  std::mutex mutex_;
  bool held_{}, enabled_{};
  unsigned epoch_{};
  HWND target_{};
  // When the helper last injected anything. Windows counts injected input as
  // input, so without this the helper's own pulses would look like the player
  // typing and the pause below would never end.
  Millis lastSend_{GetTickCount64()};
  // The pulse is a global event delivered to whatever window has focus, so while
  // the player is working it reaches their application instead of the game.
  // Skipping it costs the game nothing and stops fighting them for the keyboard.
  //
  // GetLastInputInfo counts injected input, so while pulsing the player's input
  // is whatever arrived clearly later than our own last send; our key-up lands
  // within a few milliseconds of it and a margin absorbs scheduling jitter.
  //
  // The pause must not be able to sustain itself. Once paused we inject nothing,
  // so that timestamp goes stale and "later than our last send" would stay true
  // forever, which is exactly the lock-up this replaced. While paused, any change
  // at all is therefore the player, and the pause simply ends once their input
  // stops advancing. A short hold keeps one key press from flickering the state.
  static constexpr Millis PlayerInputMargin = 400;
  // How long the player must stay silent before the helper takes over. Three
  // seconds was far too short: reading a paragraph, thinking, or watching
  // something all exceed it, so standby released during ordinary computer use and
  // the pulses landed in whatever application had focus. The default spans those
  // pauses while still resuming promptly once the player walks away, and the exact
  // value is a user setting because only the user knows their own rhythm.
  static constexpr Millis DefaultHoldWindow = 10000;
  Millis holdWindow_{DefaultHoldWindow};
  DWORD lastSeenInput_{}, lastPlayerInput_{};
  bool paused_{};
  Millis lastPlayerIdle_{};
  bool lastChanged_{};
  bool playerBusyLocked() {
    LASTINPUTINFO info{};
    info.cbSize = sizeof(info);
    if (!GetLastInputInfo(&info))
      return false;
    DWORD now = DWORD(GetTickCount64());
    DWORD last = info.dwTime;
    bool changed = last != lastSeenInput_;
    lastSeenInput_ = last;
    lastChanged_ = changed;
    // Both comparisons use wrapping 32-bit subtraction, which is correct across
    // the tick rollover that GetLastInputInfo's DWORD domain shares with
    // GetTickCount.
    if (paused_ ? changed
                : std::int32_t(last - DWORD(lastSend_)) > std::int32_t(PlayerInputMargin))
      lastPlayerInput_ = last;
    lastPlayerIdle_ = Millis(std::int32_t(now - lastPlayerInput_));
    bool busy = lastPlayerIdle_ < holdWindow_;
    if (paused_ && !busy)
      // Leaving the pause: the send timestamp is stale from the silent period and
      // would otherwise read as the player having been busy the whole time.
      lastSend_ = GetTickCount64();
    paused_ = busy;
    return busy;
  }

  bool key(WORD vk, bool up) {
    lastSend_ = GetTickCount64();
    INPUT i{};
    i.type = INPUT_KEYBOARD;
    i.ki.wVk = vk;
    i.ki.dwFlags = up ? KEYEVENTF_KEYUP : 0;
    return SendInput(1, &i, sizeof(i)) == 1;
  }

public:
  ~Input() { release(); }
  // Takes over after this much player silence. Clamped so a bad settings value
  // cannot disable the standby guard entirely or make it fire on every keystroke.
  void holdWindow(Millis ms) {
    std::lock_guard lock(mutex_);
    holdWindow_ = std::clamp<Millis>(ms, Millis(StandbySecondsMin) * 1000,
                                     Millis(StandbySecondsMax) * 1000);
  }
  void enable(bool on, unsigned epoch) {
    std::lock_guard lock(mutex_);
    epoch_ = epoch;
    enabled_ = on;
    // Start the idle window now, so the first check does not read the untouched
    // initial timestamp as "the player has been busy all along".
    if (on) {
      lastSend_ = GetTickCount64();
      lastSeenInput_ = 0;
      lastPlayerInput_ = 0;
      paused_ = false;
    }
    if (!on && held_) {
      key(VK_LSHIFT, true);
      held_ = false;
      target_ = nullptr;
    }
  }
  // Forgets the current activity burst. Called once the helper has taken control
  // back: the very keystroke that ended standby is still inside the hold window,
  // so leaving it counted put the helper straight back to sleep the instant it
  // resumed -- it detected the room and then began counting down again.
  void clearPlayerHold() {
    std::lock_guard lock(mutex_);
    lastPlayerInput_ = DWORD(GetTickCount64()) - DWORD(holdWindow_) - 1;
    // lastSend_ must move too. Nothing is injected while standing by, so it is
    // stale by the whole standby period, and the player's last input -- which is
    // what ended standby -- would then read as clearly newer than it and put the
    // helper straight back to sleep. The normal exit path refreshes it as well;
    // this entry point has to do the same.
    lastSend_ = GetTickCount64();
    lastSeenInput_ = 0;
    paused_ = false;
  }
  // True while the player has produced input that the helper did not send.
  bool playerBusy() {
    std::lock_guard lock(mutex_);
    return playerBusyLocked();
  }
  // Reported with the paused status so a stuck pause can be told apart from the
  // player genuinely still working: a small idle value means input keeps
  // arriving, a large one means the exit condition never fired.
  std::wstring activityText() {
    std::lock_guard lock(mutex_);
    return L"距上次你的操作 " + std::to_wstring(lastPlayerIdle_) + L" ms" +
           (lastChanged_ ? L"，本轮有新输入" : L"，本轮无新输入");
  }
  void release() {
    std::lock_guard lock(mutex_);
    if (held_) {
      key(VK_LSHIFT, true);
      held_ = false;
      target_ = nullptr;
    }
  }
  void checkFocus() {
    std::lock_guard lock(mutex_);
    if (held_ && !foreground(target_)) {
      key(VK_LSHIFT, true);
      held_ = false;
      target_ = nullptr;
    }
  }
  bool hold(HWND w, unsigned epoch) {
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch != epoch_ || !foreground(w))
      return false;
    if (held_)
      return target_ == w;
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
      return false;
    held_ = key(VK_LSHIFT, false);
    if (held_)
      target_ = w;
    return held_;
  }
  bool pulse(HWND w, unsigned epoch) {
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch != epoch_ || !w || !IsWindow(w) || !foreground(w) ||
        !interactiveDesktop() ||
        (GetAsyncKeyState(VK_SHIFT) & 0x8000))
      return false;
    // Compatibility with the user's known-working script. This API has no success result.
    keybd_event(VK_LSHIFT, 0, 0, 0);
    Sleep(20);
    keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
    lastSend_ = GetTickCount64();
    return true; // Events requested, not proof that the game accepted them.
  }
  bool click(HWND w, Point p, unsigned epoch) {
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch != epoch_ || !foreground(w))
      return false;
    RECT r{};
    GetClientRect(w, &r);
    POINT xy{LONG(p.x * r.right), LONG(p.y * r.bottom)};
    ClientToScreen(w, &xy);
    INPUT seq[3]{};
    for (auto &i : seq)
      i.type = INPUT_MOUSE;
    seq[0].mi.dx = LONG((xy.x - GetSystemMetrics(SM_XVIRTUALSCREEN)) * 65535.0 /
                        std::max(1, GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1));
    seq[0].mi.dy = LONG((xy.y - GetSystemMetrics(SM_YVIRTUALSCREEN)) * 65535.0 /
                        std::max(1, GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
    seq[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    seq[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    seq[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    auto n = SendInput(3, seq, sizeof(INPUT));
    if (n < 3) {
      INPUT up{};
      up.type = INPUT_MOUSE;
      up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
      SendInput(1, &up, sizeof(up));
    }
    lastSend_ = GetTickCount64();
    return n == 3;
  }
  bool press(HWND w, WORD vk, unsigned epoch) {
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch != epoch_ || !foreground(w))
      return false;
    bool ok = key(vk, false);
    return key(vk, true) && ok;
  }
  bool text(HWND w, const std::wstring &s, unsigned epoch) {
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch != epoch_ || !foreground(w) || s.size() > 256)
      return false;
    // This action is calibrated together with its input-field click.
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
      return false;
    INPUT select[4]{};
    for (auto &i : select)
      i.type = INPUT_KEYBOARD;
    select[0].ki.wVk = VK_CONTROL;
    select[1].ki.wVk = 'A';
    select[2].ki.wVk = 'A';
    select[2].ki.dwFlags = KEYEVENTF_KEYUP;
    select[3].ki.wVk = VK_CONTROL;
    select[3].ki.dwFlags = KEYEVENTF_KEYUP;
    if (SendInput(4, select, sizeof(INPUT)) != 4) {
      key('A', true);
      key(VK_CONTROL, true);
      return false;
    }
    lastSend_ = GetTickCount64();
    for (wchar_t ch : s) {
      if (!foreground(w))
        return false;
      INPUT a[2]{};
      for (auto &i : a) {
        i.type = INPUT_KEYBOARD;
        i.ki.wScan = ch;
        i.ki.dwFlags = KEYEVENTF_UNICODE;
      }
      a[1].ki.dwFlags |= KEYEVENTF_KEYUP;
      if (SendInput(2, a, sizeof(INPUT)) != 2) {
        a[1].ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(1, a + 1, sizeof(INPUT));
        return false;
      }
    }
    return true;
  }
};
inline bool launch(const Config &c) {
  if (c.launcherPath.empty() || !std::filesystem::exists(c.launcherPath))
    return false;
  SHELLEXECUTEINFOW s{};
  s.cbSize = sizeof(s);
  s.fMask = SEE_MASK_NOASYNC;
  s.lpFile = c.launcherPath.c_str();
  s.lpParameters = c.launcherArgs.c_str();
  s.nShow = SW_SHOWNORMAL;
  return ShellExecuteExW(&s) != FALSE;
}
inline bool terminateGame(const Process &p, const std::wstring &name,
                          const std::atomic<bool> &running, const std::atomic<unsigned> &generation,
                          unsigned epoch) {
  auto active = [&] { return running && generation.load() == epoch; };
  if (!sameProcess(p, name))
    return false;
  Handle h(OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                       p.pid));
  if (!h.h || processCreation(h.h) != p.created)
    return false;
  if (!active())
    return false;
  // Killing a client that has stopped talking to Windows is not one call. A
  // disconnected Overwatch leaves its window hung -- the log says WINDOW_HUNG
  // in as many words -- so a posted WM_CLOSE lands in a queue nobody will ever
  // read and the graceful phase can only burn its budget. TerminateProcess does
  // work, but a process parked inside a driver call, which is where an
  // anti-cheat client leaves a disconnected game, is not destroyed the instant
  // it is asked to be: it survives as a process object until the driver
  // returns. Five seconds of waiting after the kill was measured to be far too
  // short. The helper reported "无法关闭已验证的游戏进程" at 00:56:30, went idle,
  // and the process then disappeared on its own -- the recovery had already
  // been abandoned by the time the game actually let go. The two phases are
  // therefore sized by what can still succeed: asking a window that answers to
  // close itself is worth five seconds, asking one that does not is worth two,
  // and the kill gets the long wait it needs.
  const int gracefulTicks = (p.window && responsive(p.window)) ? 25 : 10;
  if (p.window) {
    DWORD owner{};
    GetWindowThreadProcessId(p.window, &owner);
    if (owner == p.pid)
      PostMessageW(p.window, WM_CLOSE, 0, 0);
  }
  for (int i = 0; i < gracefulTicks; ++i) {
    if (WaitForSingleObject(h.h, 200) == WAIT_OBJECT_0)
      return true;
    if (!active())
      return false;
  }
  if (!active() || !TerminateProcess(h.h, 0)) return false;
  for (int i = 0; i < 150; ++i) {
    if (WaitForSingleObject(h.h, 200) == WAIT_OBJECT_0) return true;
    if (!active()) return false;
  }
  return false;
}
inline bool validNeacPath(const std::wstring &path) {
  auto p = std::filesystem::path(path);
  return p.is_absolute() && _wcsicmp(p.filename().c_str(), L"OWNeacClient.exe") == 0;
}
inline std::wstring fullProcessPath(HANDLE h) {
  wchar_t path[32768]{};
  DWORD size = 32768;
  return QueryFullProcessImageNameW(h, 0, path, &size) ? path : L"";
}
inline bool exactExecutable(const std::wstring &actual, const std::wstring &expected) {
  if (!validNeacPath(actual) || !validNeacPath(expected)) return false;
  return _wcsicmp(std::filesystem::path(actual).lexically_normal().c_str(),
                  std::filesystem::path(expected).lexically_normal().c_str()) == 0;
}
inline bool processIds(const wchar_t *name, std::vector<DWORD> &ids) {
  ids.clear();
  Handle snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
  PROCESSENTRY32W entry{};
  entry.dwSize = sizeof(entry);
  if (snap.h == INVALID_HANDLE_VALUE || !Process32FirstW(snap.h, &entry)) return false;
  do {
    if (_wcsicmp(entry.szExeFile, name) == 0) ids.push_back(entry.th32ProcessID);
  } while (Process32NextW(snap.h, &entry));
  return GetLastError() == ERROR_NO_MORE_FILES;
}
enum class CleanupResult { Clear, NeedsPath, Denied, Cancelled, GamePresent };
// Every refusal used to look identical: one message covering path, permission
// and timing alike. The log could therefore say that the stale process could
// not be verified, but never which check refused it -- the one question that
// actually decides what to do about it. `why` carries the specific check, and
// the error code is read before anything else can overwrite it.
inline CleanupResult cleanupNeac(const Config &c, const std::atomic<bool> &running,
                                  const std::atomic<unsigned> &generation, unsigned epoch,
                                  std::wstring *why = nullptr) {
  auto deny = [&](std::wstring text) {
    if (why) *why = std::move(text);
    return CleanupResult::Denied;
  };
  auto active = [&] { return running && generation.load() == epoch && interactiveDesktop(); };
  if (!active()) return CleanupResult::Cancelled;
  if (c.demo) return CleanupResult::Clear;
  std::vector<DWORD> games, clients;
  if (!processIds(L"Overwatch.exe", games)) return deny(L"无法枚举 Overwatch.exe 进程");
  if (!games.empty()) return CleanupResult::GamePresent;
  if (!processIds(L"OWNeacClient.exe", clients)) return deny(L"无法枚举 OWNeacClient.exe 进程");
  if (clients.empty()) return CleanupResult::Clear;
  if (_wcsicmp(c.gameExe.c_str(), L"Overwatch.exe") || !validNeacPath(c.neacPath))
    return CleanupResult::NeedsPath;
  for (auto pid : clients) {
    const auto id = std::to_wstring(pid);
    Handle query(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
    if (!query.h) {
      const DWORD error = GetLastError();
      return deny(L"查询 PID " + id + L" 被拒绝（错误 " + std::to_wstring(error) + L"）");
    }
    auto created = processCreation(query.h);
    auto path = fullProcessPath(query.h);
    // Unknown or different installation: stop instead of terminating by basename alone.
    if (!created || !exactExecutable(path, c.neacPath))
      return deny(L"PID " + id + L" 路径不符：实际「" + path + L"」，配置「" + c.neacPath + L"」");
    if (c.dryRun) continue;
    Handle target(OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION,
                               FALSE, pid));
    if (!target.h) {
      // The usual cause by far: the helper is not elevated, so Windows refuses
      // PROCESS_TERMINATE on a process that is.
      const DWORD error = GetLastError();
      return deny(L"打开 PID " + id + L" 的终止句柄被拒绝（错误 " + std::to_wstring(error) +
                  L"）——助手多半没有以管理员身份运行");
    }
    if (processCreation(target.h) != created ||
        !exactExecutable(fullProcessPath(target.h), c.neacPath))
      return deny(L"PID " + id + L" 在核验期间已被替换，未终止");
    if (!processIds(L"Overwatch.exe", games)) return deny(L"无法再次枚举 Overwatch.exe 进程");
    if (!games.empty()) return CleanupResult::GamePresent;
    if (!active()) return CleanupResult::Cancelled;
    if (!TerminateProcess(target.h, 0)) {
      const DWORD error = GetLastError();
      return deny(L"结束 PID " + id + L" 被拒绝（错误 " + std::to_wstring(error) + L"）");
    }
    bool exited = false;
    for (int i = 0; i < 25; ++i) {
      if (WaitForSingleObject(target.h, 200) == WAIT_OBJECT_0) { exited = true; break; }
      if (!active()) return CleanupResult::Cancelled;
    }
    if (!exited) return deny(L"PID " + id + L" 在 5 秒内没有退出");
  }
  if (!c.dryRun) {
    if (!processIds(L"OWNeacClient.exe", clients)) return deny(L"结束后无法重新枚举 OWNeacClient.exe");
    if (!clients.empty())
      return deny(L"结束后重新枚举又出现了 OWNeacClient.exe（PID " +
                  std::to_wstring(clients.front()) + L"），可能被守护进程重新拉起");
  }
  return CleanupResult::Clear;
}
inline int heartbeat(const Config &c, const std::string &json) {
  if (c.server.empty() || c.token.empty())
    return 0;
  std::wstring url = c.server;
  while (!url.empty() && url.back() == '/')
    url.pop_back();
  url += L"/api/heartbeat";
  wchar_t host[1024]{}, path[4096]{};
  URL_COMPONENTS u{};
  u.dwStructSize = sizeof(u);
  u.lpszHostName = host;
  u.dwHostNameLength = 1024;
  u.lpszUrlPath = path;
  u.dwUrlPathLength = 4096;
  if (!WinHttpCrackUrl(url.c_str(), 0, 0, &u) ||
      (u.nScheme != INTERNET_SCHEME_HTTP && u.nScheme != INTERNET_SCHEME_HTTPS))
    return -1;
  auto session = WinHttpOpen(L"OverwatchHelper/0.1", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                             WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!session)
    return -2;
  WinHttpSetTimeouts(session, 1500, 1500, 1500, 1500);
  auto connect = WinHttpConnect(session, host, u.nPort, 0);
  auto request =
      connect ? WinHttpOpenRequest(connect, L"POST", path, nullptr, WINHTTP_NO_REFERER,
                                   WINHTTP_DEFAULT_ACCEPT_TYPES,
                                   u.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)
              : nullptr;
  int status = -3;
  if (request) {
    DWORD redirects = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    WinHttpSetOption(request, WINHTTP_OPTION_REDIRECT_POLICY, &redirects, sizeof(redirects));
    std::wstring headers =
        L"Content-Type: application/json\r\nAuthorization: Bearer " + c.token + L"\r\n";
    if (WinHttpSendRequest(request, headers.c_str(), DWORD(-1), const_cast<char *>(json.data()),
                           DWORD(json.size()), DWORD(json.size()), 0) &&
        WinHttpReceiveResponse(request, nullptr)) {
      DWORD code{}, n = sizeof(code);
      if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                              WINHTTP_HEADER_NAME_BY_INDEX, &code, &n, WINHTTP_NO_HEADER_INDEX))
        status = int(code);
    }
    WinHttpCloseHandle(request);
  }
  if (connect)
    WinHttpCloseHandle(connect);
  WinHttpCloseHandle(session);
  return status;
}
} // namespace owh
