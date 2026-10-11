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
    const auto ownRecords=recordedProcessIds(windowFixtureName);
    require(std::find(ownRecords.begin(),ownRecords.end(),GetCurrentProcessId())!=ownRecords.end(),
            "Stable process snapshot preserves the launch-time executable name");
    require(!recordedLauncherName(GetCurrentProcessId(),windowFixtureName),
            "Temporary-image fallback is restricted to the launcher name");
    HWND hiddenMain=CreateWindowExW(0,L"STATIC",L"Hidden launcher fixture",WS_POPUP,
                                  -30000,-30000,960,640,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    HWND hiddenUtility=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"Hidden utility fixture",WS_POPUP,
                                     -30000,-30000,1300,900,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    require(hiddenMain && hiddenUtility,"Create isolated hidden launcher fixtures");
    auto tray=findProcess(windowFixtureName,L"",true);
    require(tray.pid==GetCurrentProcessId() && tray.window==hiddenMain,"Hidden main window wins over utility windows without a fixed Chromium class");
    int activated{},started{},found{};
    auto discover=[&] {++found;return tray;};
    auto activate=[&](const Process &value) {require(value.window==hiddenMain,"Activate original main handle");++activated;};
    auto start=[&] {++started;return true;};
    require(dispatchLauncher(false,discover,activate,start)==LauncherStartResult::AlreadyRunning && activated==1 && started==0,
            "Existing launcher is restored without launching another process");
    tray.window=nullptr;
    require(dispatchLauncher(false,discover,activate,start)==LauncherStartResult::AlreadyRunning && activated==1 && started==0,
            "Background launcher without HWND waits without re-executing");
    tray={};
    require(dispatchLauncher(false,discover,activate,start)==LauncherStartResult::Started && started==1,
            "Only an absent launcher calls the executable start callback");
    require(dispatchLauncher(false,discover,activate,[] {return false;})==LauncherStartResult::Failed,
            "Actual launch failure is reported separately");
    DestroyWindow(hiddenUtility);DestroyWindow(hiddenMain);
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
    c.telemetryV2=false; c.hardwareEnabled=false; c.hardwareIntervalMs=12000;
    c.hardwareCache=dir+L"\\sensor-fixture.json"; c.hardwareLhmDirectory=L"C:\\fixture\\FanControl";
    c.hardwareGpuUuid=L"GPU-fixture";
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
    require(!loaded.telemetryV2 && !loaded.hardwareEnabled && loaded.hardwareIntervalMs==12000 &&
        loaded.hardwareCache==c.hardwareCache && loaded.hardwareLhmDirectory==c.hardwareLhmDirectory &&
        loaded.hardwareGpuUuid==c.hardwareGpuUuid,"Advanced telemetry settings survive save/reload");
    require(Config{}.pulseShift && Config{}.standbyOnInput, "New configurations pulse and stand by");
    require(Config{}.standbySeconds == 10, "Standby takeover delay defaults to ten seconds");
    // A settings file is user input: an out-of-range delay must not be able to
    // disable the standby guard or make it fire on every keystroke.
    writeIni(dir + L"\\settings.ini", L"agent", L"standby_seconds", L"0");
    require(loadConfig(dir, true).standbySeconds == StandbySecondsMin, "Standby delay clamped up");
    writeIni(dir + L"\\settings.ini", L"agent", L"standby_seconds", L"99999");
    require(loadConfig(dir, true).standbySeconds == StandbySecondsMax, "Standby delay clamped down");
    // A removed option must not survive in the settings file, or it reads as live
    // configuration that simply has no effect.
    writeIni(dir + L"\\settings.ini", L"agent", L"background_pulse", L"1");
    saveConfig(dir, c);
    require(readIni(dir + L"\\settings.ini", L"agent", L"background_pulse", L"gone") == L"gone",
            "Retired background pulse option is removed on save");
    saveConfig(dir, c);
    require(Config{}.cnPreset && !loaded.cnPreset, "CN preset defaults on and stays disabled in demo");
    c.cnPreset = false;
    saveConfig(dir, c);
    require(!loadConfig(dir, false).cnPreset, "CN preset opt-out persists");
    c.cnPreset = true;
    saveConfig(dir, c);
    require(loadConfig(dir, false).cnPreset, "CN preset opt-in persists");
    c.mode = WorkflowMode::Deathmatch;
    c.roomTarget = L"测试房间";
    c.semanticRecognition = false;
    saveConfig(dir,c);
    auto modeConfig = loadConfig(dir,false);
    require(modeConfig.mode == WorkflowMode::Deathmatch && modeConfig.roomTarget == c.roomTarget && !modeConfig.semanticRecognition,
            "Workflow mode, target and OCR preference persist");
    require(Config{}.semanticRecognition && Config{}.mode == WorkflowMode::CustomRoom,
            "New configurations use OCR and retain custom-room workflow");
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
    PlayerActivityTracker activity;
    activity.baseline(true,900);
    require(!activity.observe(1000,900,10000)&&activity.revision()==0,
            "Starting input tracking does not turn the current timestamp into a player event");
    require(activity.observe(1200,1150,10000)&&activity.revision()==1&&activity.changed(),
            "A genuine new external input starts a hold and advances its revision");
    activity.sent(1500);
    require(activity.observe(1550,1508,10000)&&activity.revision()==1&&!activity.changed()&&activity.lastPlayerInput()==1150,
            "Own injection during a player hold does not revoke a newly requested Join");
    require(activity.observe(2200,2150,10000)&&activity.revision()==2&&activity.changed(),
            "New external input after our send still advances activity");
    require(activity.observe(2300,2150,10000)&&activity.revision()==2&&!activity.changed(),
            "Repeated polling of one external input is not a second activity event");
    activity.clearHold();
    require(!activity.observe(2400,2150,10000)&&activity.revision()==2&&activity.lastPlayerInput()==2150,
            "Clearing standby cancels only the hold and cannot manufacture a new event");
    require(activity.observe(2700,2650,10000)&&activity.revision()==3,
            "Clearing standby does not suppress the next genuine external input");
    activity.baseline(true,2650);
    require(!activity.observe(2800,2650,10000)&&activity.revision()==3,
            "Re-enabling preserves the monotonic activity revision without adding an event");
    require(activity.observe(15000,14990,10000)&&activity.revision()==4,
            "A long interval with no helper sends still detects new user activity");
    require(!activity.observe(25000,14990,10000)&&activity.revision()==4,
            "An expired hold stays expired without creating a synthetic send timestamp");
    constexpr Millis wrap=Millis(1)<<32;
    PlayerActivityTracker rollover;
    rollover.baseline(true,DWORD(wrap-1000));
    require(rollover.observe(wrap-600,DWORD(wrap-650),10000)&&rollover.revision()==1,
            "External activity before the DWORD rollover is tracked");
    rollover.sent(wrap-20);
    require(rollover.observe(wrap+100,5,10000)&&rollover.revision()==1&&!rollover.changed(),
            "Own input across the DWORD rollover remains excluded while paused");
    require(rollover.observe(wrap+900,850,10000)&&rollover.revision()==2&&rollover.idle()==50,
            "External input after rollover has a small correct idle duration");
    require(!rollover.observe(wrap+10900,850,10000)&&rollover.revision()==2,
            "Idle hold expiration is correct after the DWORD rollover");
    rollover.clearHold();
    require(rollover.revision()==2&&!rollover.observe(wrap+11000,850,10000),
            "Clear after rollover does not increment or repeat an event");
    PlayerActivityTracker ancient;
    ancient.baseline(true,2000); ancient.sent(1000);
    require(ancient.observe(wrap+1100,1050,10000)&&ancient.revision()==1,
            "An own send from a prior DWORD cycle cannot hide current external activity");
    auto leftDown=gameInputEvent(VK_LBUTTON,false),leftUp=gameInputEvent(VK_LBUTTON,true);
    auto rightDown=gameInputEvent(VK_RBUTTON,false),rightUp=gameInputEvent(VK_RBUTTON,true);
    require(leftDown.type==INPUT_MOUSE && leftDown.mi.dwFlags==MOUSEEVENTF_LEFTDOWN &&
            leftUp.mi.dwFlags==MOUSEEVENTF_LEFTUP && rightDown.type==INPUT_MOUSE &&
            rightDown.mi.dwFlags==MOUSEEVENTF_RIGHTDOWN && rightUp.mi.dwFlags==MOUSEEVENTF_RIGHTUP,
            "Mouse input emits real paired button events without pointer movement");
    require(gameInputEvent('W',false).type==INPUT_KEYBOARD && gameInputEvent('W',true).ki.dwFlags==KEYEVENTF_KEYUP,
            "Keyboard event mapping remains unchanged");
    Input input;
    input.enable(true, 2);
    const auto enabledRevision=input.playerActivityRevision();
    input.clearPlayerHold();
    require(input.playerActivityRevision()==enabledRevision,
            "The public clearPlayerHold API never creates a player activity revision");
    auto foregroundWindow = GetForegroundWindow();
    require(!input.hold(foregroundWindow, 1), "Old generation cannot hold Shift");
    require(!input.pulse(foregroundWindow, 1), "Old generation cannot pulse");
    require(!input.click(foregroundWindow, {.5, .5}, 1), "Old generation cannot click");
    require(!input.doubleClick(foregroundWindow,{.5,.5},1),"Old generation cannot double click");
    require(!input.press(foregroundWindow, VK_RETURN, 1), "Old generation cannot press keys");
    require(!input.tap(foregroundWindow,'W',100,1), "Old generation cannot start random game input");
    require(!input.tapMouse(foregroundWindow,false,1) && !input.tapMouse(foregroundWindow,true,1),
            "Old generation cannot inject either mouse button");
    require(!input.text(foregroundWindow, L"must never be typed", 1), "Old generation cannot type");
    input.enable(false, 3);
    input.holdWindow(Millis(0)); // Clamped, and never sends while stopped.
    require(!input.hold(foregroundWindow, 3), "Stopped input must stay disabled");
    require(!input.pulse(foregroundWindow, 3), "Stopped pulses stay disabled");
    require(!input.tap(foregroundWindow,'W',100,3), "Stopped random game input stays disabled");
    require(!input.tapMouse(foregroundWindow,false,3) && !input.tapMouse(foregroundWindow,true,3),
            "Stopped input cannot inject either mouse button");
    require(!input.doubleClick(foregroundWindow,{.5,.5},3),"Stopped input cannot double click");
    input.enable(false,4);
    require(!input.click(foregroundWindow,{.5,.5},4,true),"Dry-run/stopped refresh cannot click");
    input.enable(true,5);
    require(!input.click(foregroundWindow,{.5,.5},4,true),"Stale refresh epoch cannot click");
    input.blockForeground(true);
    require(!input.click(foregroundWindow,{.5,.5},5,true),"Foreground-use pause cancels refresh");
    input.blockForeground(false);
    require(!input.click(nullptr,{.5,.5},5,true),"Lost target focus cannot receive refresh");
    require(!input.click(foregroundWindow,{.5,.5},5,true,[]{return false;}),"Revoked session authorization cannot click");
    require(!input.doubleClick(foregroundWindow,{.5,.5},5,[]{return false;}),"Revoked filter version cannot double click");
    input.enable(false,6);
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
    require(terminateGameDetailed(stale,fixtureName,active,generation,3).result==GameTermination::IdentityChanged,
            "Changed creation time is distinguished from a termination rejection");
    require(terminateGameDetailed(fixture,fixtureName,active,generation,2).result==GameTermination::Cancelled,
            "Old run is cancelled before requesting process access");
    require(!terminateGame(fixture, fixtureName, active, generation, 2), "Old run cannot terminate fixture");
    require(WaitForSingleObject(child.hProcess, 0) == WAIT_TIMEOUT, "Rejected termination leaves child alive");
    require(gameExitStillPending(fixture),"Owned live fixture cannot be treated as an exited game");
    const auto ended=terminateGameDetailed(fixture,fixtureName,active,generation,3);
    require(ended.result==GameTermination::Exited,"Verified test child terminates with a confirmed result");
    require(!gameExitStillPending(fixture),"Owned terminated fixture resumes recovery");
    require(terminateGameDetailed(fixture,fixtureName,active,generation,3).result==GameTermination::Exited,
            "An already-exited owned process is successful rather than a new failure");
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
