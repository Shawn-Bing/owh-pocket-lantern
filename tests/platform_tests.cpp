#include "agent/startup.hpp"
#include <iostream>
#include <stdexcept>

void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
int main(int argc, char **argv) {
  using namespace owh;
  if (argc == 2 && std::string(argv[1]) == "--termination-fixture") {
    Sleep(30000);
    return 42;
  }
  try {
    constexpr LONG_PTR nvidiaHud = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    require(passiveGameOverlay(L"NVIDIA Overlay.exe", L"CEF-OSC-WIDGET", nvidiaHud),
            "Passive NVIDIA full-screen HUD must not block game capture");
    require(!passiveGameOverlay(L"another-app.exe", L"CEF-OSC-WIDGET", nvidiaHud),
            "An unrelated click-through application must still block capture");
    require(!passiveGameOverlay(L"NVIDIA Overlay.exe", L"Chrome_WidgetWin_0", nvidiaHud),
            "Other NVIDIA windows are not the passive HUD");
    require(!passiveGameOverlay(L"NVIDIA Overlay.exe", L"CEF-OSC-WIDGET", nvidiaHud & ~WS_EX_TRANSPARENT),
            "An interactive NVIDIA overlay must still block capture");
    require(!unobscured(nullptr) && captureAvailability(nullptr, true).code == "window_unavailable",
            "Missing target must have a specific capture error");
    Capture capture;
    Frame staleFrame{80, 60, std::vector<std::uint32_t>(80*60, 0xff123456)};
    require(!capture.grab(nullptr, staleFrame, true) && staleFrame.pixels.empty() && staleFrame.width == 0,
            "Capture failure must discard stale pixels");
    require(CaptureFailure{"occluded", 0, 123, L"blocking-app.exe"}.message().find(L"blocking-app.exe") != std::wstring::npos,
            "Occlusion message must identify the blocker");
    // Own off-screen fixture only: no activation, desktop input or other app changes.
    HWND layered = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
                                   L"STATIC", L"Capture test fixture", WS_POPUP,
                                   -30000, -30000, 100, 100, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    require(layered != nullptr, "Create isolated layered-window fixture");
    bool alphaSet = SetLayeredWindowAttributes(layered, 0, 0, LWA_ALPHA) != 0;
    bool invisibleIgnored = ignorableCaptureOverlay(layered);
    bool opaqueSet = SetLayeredWindowAttributes(layered, 0, 255, LWA_ALPHA) != 0;
    bool opaqueBlocked = !ignorableCaptureOverlay(layered);
    DestroyWindow(layered);
    require(alphaSet && invisibleIgnored, "A globally transparent layered window does not obscure pixels");
    require(opaqueSet && opaqueBlocked, "Click-through style does not make opaque pixels safe to capture");
    auto windowFixtureName = processName(GetCurrentProcessId());
    HWND large = CreateWindowExW(WS_EX_NOACTIVATE, L"STATIC", L"Main-window selection fixture",
                                WS_POPUP | WS_VISIBLE, -30000, -30000, 955, 633,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    HWND tiny = CreateWindowExW(WS_EX_NOACTIVATE, L"STATIC", L"Auxiliary-window selection fixture",
                               WS_POPUP | WS_VISIBLE, -30000, -30000, 160, 28,
                               nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    require(large && tiny, "Create isolated window-ranking fixtures");
    auto ranked = findProcess(windowFixtureName, L"Static");
    DestroyWindow(tiny);
    DestroyWindow(large);
    require(ranked.pid == GetCurrentProcessId() && ranked.window == large,
            "Choose main window instead of first small auxiliary window");
    auto options = parseOptions({L"helper.exe", L"--demo", L"--data-dir", L"中文 folder", L"--diagnose"});
    require(options.demo && options.diagnose && options.dataDir == L"中文 folder", "CLI options");
    require(!parseOptions({L"C:\\--demo\\helper.exe"}).demo, "Path must not enable demo mode");
    for (auto args : {std::vector<std::wstring>{L"helper", L"--data-dir"},
                      std::vector<std::wstring>{L"helper", L"--demo-extra"},
                      std::vector<std::wstring>{L"helper", L"--data-dir", L"--diagnose"}}) {
      bool rejected = false;
      try { parseOptions(args); } catch (const std::runtime_error &) { rejected = true; }
      require(rejected, "Invalid CLI must fail");
    }
    StartupOptions test;
    test.demo = true;
    test.dataDir = (std::filesystem::current_path() / L"platform-test-data" /
                   std::to_wstring(GetCurrentProcessId())).wstring();
    auto dir = resolveDataDirectory(test);
    Config c;
    c.demo = true;
    c.gameExe = L"owh-demo.exe";
    c.machine = L"private-machine";
    c.token = L"DO-NOT-REPORT-THIS-TOKEN";
    c.server = L"https://private-server.example";
    c.launcherPath = exeDirectory() + L"\\owh-demo.exe";
    c.neacPath = L"C:\\Games\\中文目录\\OWNeacClient.exe";
    c.pulseShift = false;
    c.standbySeconds = 25;
    Step s;
    s.label = L"目标房间 / 中文";
    s.text = L"private-search-text";
    s.action = Action::Ready;
    s.roi = {.1, .2, .3, .4};
    s.click = {.8, .7};
    s.image.resize(TemplateW * TemplateH);
    for (std::size_t i = 0; i < s.image.size(); ++i)
      s.image[i] = i % 2 ? 220 : 20;
    c.steps.push_back(s);
    saveConfig(dir, c);
    auto loaded = loadConfig(dir, true);
    require(loaded.steps.size() == 1, "Template persisted");
    require(loaded.steps[0].label == s.label && loaded.steps[0].image == s.image, "Unicode/template roundtrip");
    require(loaded.token == c.token && loaded.dryRun, "Settings roundtrip");
    require(!loaded.pulseShift && loaded.neacPath == c.neacPath,
            "Pulse and residual process options roundtrip");
    require(loaded.standbySeconds == 25, "Standby takeover delay roundtrip");
    require(Config{}.pulseShift && Config{}.standbyOnInput, "New configurations pulse and stand by");
    require(Config{}.standbySeconds == 10, "Standby takeover delay defaults to ten seconds");
    // A settings file is user input: an out-of-range delay must not be able to
    // disable the standby guard or make it fire on every keystroke.
    writeIni(dir + L"\\settings.ini", L"agent", L"standby_seconds", L"0");
    require(loadConfig(dir, true).standbySeconds == StandbySecondsMin, "Standby delay clamped up");
    writeIni(dir + L"\\settings.ini", L"agent", L"standby_seconds", L"99999");
    require(loadConfig(dir, true).standbySeconds == StandbySecondsMax, "Standby delay clamped down");
    saveConfig(dir, c);
    require(Config{}.cnPreset && !loaded.cnPreset, "CN preset defaults on and stays disabled in demo");
    c.cnPreset = false;
    saveConfig(dir, c);
    require(!loadConfig(dir, false).cnPreset, "CN preset opt-out persists");
    c.cnPreset = true;
    saveConfig(dir, c);
    require(loadConfig(dir, false).cnPreset, "CN preset opt-in persists");
    require(validNeacPath(c.neacPath), "Exact absolute client path accepted");
    require(!validNeacPath(L"OWNeacClient.exe") && !validNeacPath(L"C:\\Games\\NeacLoader.exe"),
            "Relative path and loader cannot be cleanup targets");
    require(exactExecutable(L"C:\\Games\\中文目录\\owneacclient.EXE", c.neacPath), "Path case insensitive");
    require(!exactExecutable(L"D:\\OtherGame\\OWNeacClient.exe", c.neacPath), "Different installation rejected");
    require(!exactExecutable(L"", c.neacPath), "Unreadable process path rejected");
    std::atomic<bool> stopped{false};
    std::atomic<unsigned> generation{3};
    require(cleanupNeac(c, stopped, generation, 3) == CleanupResult::Cancelled,
            "Stopped run cannot enumerate or clean processes");
    c.steps.clear();
    saveConfig(dir, c);
    require(loadConfig(dir, true).steps.empty(), "Removed templates must not return after reload");
    c.steps.push_back(s);
    saveConfig(dir, c);
    writeIni(dir + L"\\settings.ini", L"step0", L"geometry", L"-1 0 1 1 0 0 1.777");
    require(loadConfig(dir, true).steps.empty(), "Corrupt geometry must be rejected");
    auto report = diagnosticReport(c);
    for (auto secret : {c.token, c.server, c.machine, s.label, s.text})
      require(report.find(utf8(secret)) == std::string::npos, "Diagnostic leaked configuration");
    require(report.find("templates=1") != std::string::npos, "Diagnostic template count");
    require(std::filesystem::is_regular_file(writeDiagnostics(dir, c)), "Diagnostic file written");
    Handle self(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId()));
    Process process{GetCurrentProcessId(), processCreation(self.h), nullptr};
    auto name = processName(process.pid);
    require(sameProcess(process, name), "Current process identity");
    ++process.created;
    require(!sameProcess(process, name), "Stale identity must be rejected");
    Input input;
    input.enable(true, 2);
    auto foregroundWindow = GetForegroundWindow();
    require(!input.hold(foregroundWindow, 1), "Old generation cannot hold Shift");
    require(!input.pulse(foregroundWindow, 1), "Old generation cannot pulse");
    require(!input.click(foregroundWindow, {.5, .5}, 1), "Old generation cannot click");
    require(!input.press(foregroundWindow, VK_RETURN, 1), "Old generation cannot press keys");
    require(!input.text(foregroundWindow, L"must never be typed", 1), "Old generation cannot type");
    input.enable(false, 3);
    input.holdWindow(Millis(0)); // Clamped, and never sends while stopped.
    require(!input.hold(foregroundWindow, 3), "Stopped input must stay disabled");
    require(!input.pulse(foregroundWindow, 3), "Stopped pulses stay disabled");
    // Only this test's console child is terminated; no desktop input or game process is touched.
    wchar_t executable[32768]{};
    GetModuleFileNameW(nullptr, executable, 32768);
    std::wstring command = L"\"" + std::wstring(executable) + L"\" --termination-fixture";
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION child{};
    require(CreateProcessW(executable, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                           nullptr, nullptr, &startup, &child), "Create isolated termination fixture");
    Handle childProcess(child.hProcess), childThread(child.hThread);
    Process fixture{child.dwProcessId, processCreation(child.hProcess), nullptr};
    auto fixtureName = std::filesystem::path(executable).filename().wstring();
    std::atomic<bool> active{true};
    auto stale = fixture;
    ++stale.created;
    require(!terminateGame(stale, fixtureName, active, generation, 3), "Stale process cannot be terminated");
    require(!terminateGame(fixture, fixtureName, active, generation, 2), "Old run cannot terminate fixture");
    require(WaitForSingleObject(child.hProcess, 0) == WAIT_TIMEOUT, "Rejected termination leaves child alive");
    require(terminateGame(fixture, fixtureName, active, generation, 3), "Verified test child terminates");
    require(WaitForSingleObject(child.hProcess, 0) == WAIT_OBJECT_0, "Success means child has actually exited");
    DWORD exitCode{};
    GetExitCodeProcess(child.hProcess, &exitCode);
    require(exitCode == 0, "Fixture was terminated rather than reaching natural timeout");
    std::cout << "Windows platform scenarios passed (no input sent)\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
