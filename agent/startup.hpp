#pragma once
#include "agent/platform.hpp"
#include "core/cn_recovery.hpp"

namespace owh {
inline constexpr char Version[] = "0.3.0";
struct StartupOptions {
  bool demo{}, diagnose{};
  std::wstring dataDir;
};
inline StartupOptions parseOptions(const std::vector<std::wstring> &args) {
  StartupOptions result;
  for (std::size_t i = 1; i < args.size(); ++i) {
    if (args[i] == L"--demo")
      result.demo = true;
    else if (args[i] == L"--diagnose")
      result.diagnose = true;
    else if (args[i] == L"--data-dir" && i + 1 < args.size() && !args[i + 1].empty() &&
             args[i + 1].rfind(L"--", 0) != 0)
      result.dataDir = args[++i];
    else
      throw std::runtime_error("Unknown option or missing value");
  }
  return result;
}
inline StartupOptions commandLineOptions() {
  int count{};
  auto args = CommandLineToArgvW(GetCommandLineW(), &count);
  if (!args)
    throw std::runtime_error("Cannot read command line");
  std::vector<std::wstring> values(args, args + count);
  LocalFree(args);
  return parseOptions(values);
}
inline std::wstring resolveDataDirectory(const StartupOptions &options) {
  if (options.dataDir.empty())
    return dataDirectory(options.demo);
  auto path = std::filesystem::absolute(options.dataDir).lexically_normal();
  std::filesystem::create_directories(path);
  return path.wstring();
}
// Deliberately omit tokens, server URLs, machine names, search text and template pixels.
inline std::string diagnosticReport(const Config &c) {
  std::ostringstream out;
  out << "OverwatchHelper " << Version << " / Windows diagnostics\n"
      << "mode=" << (c.demo ? "demo" : "game") << "\n"
      << "dry_run=" << c.dryRun << "\nhold_shift=" << c.holdShift
      << "\nauto_recover=" << c.autoRecover << "\n"
      << "pulse_shift=" << c.pulseShift << "\nbackground_pulse=" << c.backgroundPulse
      << "\nstandby_on_input=" << c.standbyOnInput << "\n"
      << "cn_preset=" << c.cnPreset << "\n"
      << "cn_profile_source=" << (c.cnProfileSource.empty() ? "none" : c.cnProfileSource) << "\n"
      << "cn_profile_rules=" << c.cnProfile.rules.size() << "\n"
      << "neac_path_valid=" << validNeacPath(c.neacPath) << "\n"
      << "interactive_desktop=" << interactiveDesktop() << "\n"
      << "launcher_configured=" << !c.launcherPath.empty() << "\n";
  // A helper that is not at least as privileged as the game or Battle.net can
  // neither activate their windows nor send them input (UIPI). Report both
  // sides so a mismatch is visible instead of looking like a focus problem.
  auto helperLevel = selfIntegrity();
  out << "helper.integrity=" << utf8(integrityName(helperLevel)) << "\n"
      << "helper.elevated=" << selfElevated() << "\n";
  // This diagnostic is often started from an ordinary shell or by double-clicking
  // diagnose.cmd while the real helper runs elevated. Judging the game against
  // this process alone would then raise a false alarm, so list every running
  // helper and compare against the strongest one.
  auto effectiveHelper = helperLevel;
  {
    std::vector<DWORD> running;
    if (processIds(L"overwatch-helper.exe", running)) {
      out << "helpers.running=" << running.size() << "\n";
      for (auto pid : running) {
        auto level = processIntegrity(pid);
        if (int(level) > int(effectiveHelper)) effectiveHelper = level;
        out << "helper_process.pid=" << pid << " self=" << (pid == GetCurrentProcessId())
            << " integrity=" << utf8(integrityName(level)) << "\n";
      }
    }
  }
  Integrity gameLevel{}, launcherLevel{};
  std::error_code error;
  out << "launcher_exists=" << std::filesystem::is_regular_file(c.launcherPath, error) << "\n"
      << "dashboard_configured=" << (!c.server.empty() && !c.token.empty()) << "\n"
      << "system_dpi=" << GetDpiForSystem() << "\n";
  auto reportProcess = [&](const char *label, const Process &p, const std::wstring &name) {
    RECT r{};
    wchar_t cls[256]{};
    GetClientRect(p.window, &r);
    GetClassNameW(p.window, cls, 256);
    POINT origin{};
    ClientToScreen(p.window, &origin);
    out << label << ".pid=" << p.pid << "\n" << label << ".verified=" << sameProcess(p, name)
        << "\n" << label << ".window=" << (p.window != nullptr)
        << "\n" << label << ".class=" << utf8(cls)
        << "\n" << label << ".client=" << r.right << 'x' << r.bottom
        << "\n" << label << ".foreground=" << foreground(p.window)
        << "\n" << label << ".minimized=" << (p.window && IsIconic(p.window))
        << "\n" << label << ".origin=" << origin.x << ',' << origin.y
        << "\n" << label << ".unobscured=" << unobscured(p.window) << "\n";
    auto level = processIntegrity(p.pid);
    if (std::string(label) == "game") gameLevel = level;
    if (std::string(label) == "launcher") launcherLevel = level;
    out << label << ".integrity=" << utf8(integrityName(level)) << "\n";
    // Measure whether PrintWindow can stand in for a screen copy. If it returns a
    // structured image for a covered window, the helper could keep detecting a
    // disconnect while the player works alongside the game, which is the one case
    // where detection currently stops. An all-black DirectX surface is reported
    // by low ink and deviation rather than pretended to be a working capture.
    if (p.window) {
      Frame printed;
      if (printWindowFrame(p.window, printed)) {
        auto ink = frameInk(printed);
        out << label << ".printwindow=ok\n"
            << label << ".printwindow_ink=" << ink.ink
            << "\n" << label << ".printwindow_deviation=" << ink.deviation << "\n";
      } else {
        out << label << ".printwindow=failed";
        // PrintWindow works by sending WM_PRINT, which UIPI blocks towards a
        // higher-integrity window. A diagnostic started from an ordinary shell
        // therefore fails against an elevated game even when PrintWindow itself
        // would work from the elevated helper.
        if (level != Integrity::Unknown && int(level) > int(helperLevel))
          out << " (expected: this process is lower integrity, so UIPI blocked it;"
                 " run the diagnostic from the helper)";
        out << "\n";
      }
    }
    if (p.window && c.cnPreset && !c.demo) {
      Capture capture;Frame frame;
      if (capture.grab(p.window,frame,true)) {
        bool launcher=std::string(label)=="launcher";
        CNLauncherScore launcherScore;
        CNGameScore gameScore;
        auto page=launcher ? recognizeCNLauncher(frame,GetDpiForWindow(p.window)/96.0,c.cnProfile.launcher,&launcherScore)
                           : recognizeCNGame(frame,c.cnProfile.rules,c.cnProfile.join,&gameScore);
        out << label << ".capture=ok\n"
            << label << ".captured=" << frame.width << 'x' << frame.height
            << "\n" << label << ".dpi=" << GetDpiForWindow(p.window) << "\n"
            << label << ".cn_page=" << int(page.page)
            << "\n" << label << ".cn_score=" << page.score << "\n";
        // Every launcher anchor must reach its own minimum; a near miss is the
        // difference between "wrong page" and "threshold too strict for this
        // window size".
        if (launcher && launcherScore.evaluated)
          for (std::size_t i = 0; i < launcherScore.scores.size(); ++i)
            out << label << ".cn_anchor[" << i << "]=" << launcherScore.scores[i] << "\n";
        // Without this a failing game page is indistinguishable from a page that
        // was never recorded, which is the difference between tuning a threshold
        // and adding a step.
        if (!launcher && gameScore.evaluated && gameScore.rule >= 0 &&
            std::size_t(gameScore.rule) < c.cnProfile.rules.size()) {
          const auto &rule = c.cnProfile.rules[std::size_t(gameScore.rule)];
          out << label << ".cn_closest=" << cnPageName(gameScore.page)
              << "\n" << label << ".cn_closest_label=" << utf8(rule.label)
              << "\n" << label << ".cn_closest_score=" << gameScore.best
              << "\n" << label << ".cn_closest_accepted=" << gameScore.accepted << "\n";
          for (std::size_t i = 0; i < gameScore.anchors.size() && i < rule.anchors.size(); ++i)
            out << label << ".cn_closest_anchor[" << i << "]=" << gameScore.anchors[i]
                << " min=" << rule.anchors[i].minimum << "\n";
        }
      } else out << label << ".capture=" << capture.failure().code << "\n";
    }
    if (p.window) {
      RECT client{origin.x, origin.y, origin.x+r.right, origin.y+r.bottom};
      int count = 0;
      for (HWND other = GetWindow(p.window, GW_HWNDPREV); other && count < 12;
           other = GetWindow(other, GW_HWNDPREV)) {
        RECT bounds{}, intersection{};
        if (!IsWindowVisible(other) || IsIconic(other) || !GetWindowRect(other, &bounds) ||
            !IntersectRect(&intersection, &bounds, &client)) continue;
        DWORD pid{};
        GetWindowThreadProcessId(other, &pid);
        wchar_t otherClass[256]{};
        GetClassNameW(other, otherClass, 256);
        DWORD cloaked{}, attributes{};
        BYTE alpha{};
        COLORREF color{};
        auto cloakResult = DwmGetWindowAttribute(other, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
        auto layered = GetLayeredWindowAttributes(other, &color, &alpha, &attributes);
        out << label << ".overlap[" << count++ << "]=" << utf8(processName(pid))
            << " class=" << utf8(otherClass) << " exstyle=" << GetWindowLongPtrW(other, GWL_EXSTYLE)
            << " cloaked=" << cloaked << " cloak_result=" << cloakResult
            << " layered_attributes=" << layered << " alpha=" << int(alpha) << " flags=" << attributes << " color=" << color
            << " ignored_for_capture=" << ignorableCaptureOverlay(other)
            << " bounds=" << bounds.left << ',' << bounds.top << ',' << bounds.right << ',' << bounds.bottom << "\n";
      }
    }
  };
  out << "desktop.bounds=" << GetSystemMetrics(SM_XVIRTUALSCREEN) << ',' << GetSystemMetrics(SM_YVIRTUALSCREEN)
      << ',' << GetSystemMetrics(SM_CXVIRTUALSCREEN) << ',' << GetSystemMetrics(SM_CYVIRTUALSCREEN) << "\n";
  reportProcess("game", findProcess(c.gameExe, c.gameClass), c.gameExe);
  if (!c.demo)
    reportProcess("launcher", findProcess(L"Battle.net.exe", L"Chrome_WidgetWin_0"), L"Battle.net.exe");
  if (!c.demo) {
    std::vector<DWORD> ids;
    out << "neac.snapshot_ok=" << processIds(L"OWNeacClient.exe", ids) << "\n"
        << "neac.count=" << ids.size() << "\n";
    for (auto pid : ids) {
      Handle query(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
      Handle access(OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION,
                                FALSE, pid));
      out << "neac.pid=" << pid << "\n"
          << "neac.identity_readable=" << (query.h && processCreation(query.h) != 0 &&
                                           !fullProcessPath(query.h).empty()) << "\n"
          << "neac.matches_configured_path=" << (query.h && exactExecutable(fullProcessPath(query.h), c.neacPath)) << "\n"
          << "neac.terminate_handle_available=" << (access.h != nullptr) << "\n";
    }
  }
  int ready = 0, launchSteps = 0, errorSteps = 0;
  out << "templates=" << c.steps.size() << "\n";
  for (std::size_t i = 0; i < c.steps.size(); ++i) {
    const auto &s = c.steps[i];
    ready += !s.launcher && s.action == Action::Ready;
    launchSteps += s.launcher && s.action == Action::Launch;
    errorSteps += !s.launcher && s.action == Action::Restart;
    out << "template[" << i << "]: target=" << (s.launcher ? "launcher" : "game")
        << " action=" << int(s.action) << " aspect=" << s.aspect
        << " contrast=" << contrast(s.image) << "\n";
  }
  if (!ready && !c.cnPreset)
    out << "CHECK: Calibrate a unique target-room feature (Ready action).\n";
  if (!c.demo && c.autoRecover && !launchSteps && !c.cnPreset)
    out << "CHECK: Calibrate the Battle.net Play button (Launch action).\n";
  if (!c.demo && c.autoRecover && !errorSteps && !c.cnPreset)
    out << "CHECK: Calibrate disconnect error text (Restart action).\n";
  if (!c.demo && c.autoRecover && !validNeacPath(c.neacPath))
    out << "CHECK: Configure the exact OWNeacClient.exe path for stale-client cleanup.\n";
  if (!c.demo && gameLevel != Integrity::Unknown && effectiveHelper != Integrity::Unknown &&
      int(gameLevel) > int(effectiveHelper))
    out << "CHECK: The game outranks every running helper; Windows will block its keystrokes "
           "and foreground requests (UIPI). Run the helper as administrator too.\n";
  if (!c.demo && launcherLevel != Integrity::Unknown && effectiveHelper != Integrity::Unknown &&
      int(launcherLevel) > int(effectiveHelper))
    out << "CHECK: Battle.net outranks every running helper; the Play button click will be "
           "blocked (UIPI). Run the helper as administrator too.\n";
  if (!c.cnProfileError.empty())
    out << "cn_profile_error=" << c.cnProfileError << "\n";
  if (c.cnPreset && c.cnProfile.rules.empty())
    out << "CHECK: The recognition profile has no rules; CN preset navigation is disabled.\n";
  out << "Diagnostic collection does not send input, switch focus, launch/terminate games or make network requests.\n";
  return out.str();
}
inline std::wstring writeDiagnostics(const std::wstring &dir, const Config &c) {
  auto path = std::filesystem::path(dir) / L"diagnostics.txt";
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  file.exceptions(std::ios::failbit | std::ios::badbit);
  file << diagnosticReport(c);
  file.close();
  return path.wstring();
}
} // namespace owh
