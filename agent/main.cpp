#include "agent/startup.hpp"
#include "core/cn_recovery.hpp"
#include <commctrl.h>
#include <condition_variable>
#include <iomanip>
#include <thread>

using namespace owh;
namespace {
constexpr wchar_t AppClass[] = L"OverwatchHelper.Main";
constexpr wchar_t OverlayClass[] = L"OverwatchHelper.Calibrate";
constexpr const wchar_t *ActionNames[] = {L"点击按钮",
                                          L"按 Enter",
                                          L"按 Esc",
                                          L"输入文字并 Enter",
                                          L"到达目标房间 / 持续 Shift",
                                          L"登录/错误页超时恢复",
                                          L"战网启动游戏",
                                          L"仅观察（结束房间会话）"};
enum {
  Start = 101,
  Stop,
  Save,
  Browse,
  Record,
  Remove,
  OpenData,
  GameExe,
  Launcher,
  Args,
  Server,
  Token,
  Machine,
  Dry,
  Hold,
  Auto,
  Scope,
  ActionBox,
  Label,
  TextArg,
  Steps,
  StatusTitle,
  StatusDetail,
  Stats,
  NetStatus,
  Diagnose,
  ShiftMode,
  BackgroundPulse,
  NeacPath,
  NeacBrowse,
  CnPreset,
  RestartNow
};
HINSTANCE instance{};
HWND mainWindow{}, calibrationWindow{};
bool calibrating{};
void cancelCalibration();
HFONT font{}, titleFont{};
int dpi = 96;
Config config;
std::wstring directory;
std::mutex configMutex, statusMutex, networkMutex, logMutex;
std::atomic<bool> running{}, quitting{};
std::atomic<unsigned> generation{};
std::atomic<unsigned> restartRequested{~0u};
std::condition_variable wake, networkWake;
Input input;
Millis bootTime = GetTickCount64();
struct Status {
  std::string state = "STOPPED";
  std::wstring detail = L"国服预设支持登录错误重启及最稳轮换版导航。先演练确认，再开启实际动作。";
  DWORD pid{};
  double score{};
  unsigned restarts{};
  Millis next{};
  int http{};
};
Status status;
bool networkDirty = true;
std::vector<int> settingsIds;
HWND control(int id) { return GetDlgItem(mainWindow, id); }
int u(int n) { return MulDiv(n, dpi, 96); }
std::wstring textOf(int id) {
  HWND h = control(id);
  int n = GetWindowTextLengthW(h);
  std::wstring s(n + 1, 0);
  GetWindowTextW(h, s.data(), n + 1);
  s.resize(n);
  return s;
}
bool checked(int id) { return SendMessageW(control(id), BM_GETCHECK, 0, 0) == BST_CHECKED; }
// Three decimals, the precision the recognition thresholds are expressed in.
std::wstring fixed3(double v) {
  std::wostringstream out;
  out << std::fixed << std::setprecision(3) << v;
  return out.str();
}
std::wstring launcherScoresText(const CNLauncherScore &score) {
  std::wstring out;
  for (std::size_t i = 0; i < score.scores.size(); ++i) {
    if (i)
      out += L" / ";
    out += fixed3(score.scores[i]);
  }
  return out;
}
// Names the closest recorded page and the first anchor that missed its own
// threshold, so a failing page says whether to retune or to record a new step.
std::wstring gameClosestText(const CNGameScore &score, const std::vector<CNRule> &rules) {
  if (!score.evaluated || score.rule < 0 || std::size_t(score.rule) >= rules.size())
    return L"未匹配任何已录制页面。";
  const auto &rule = rules[std::size_t(score.rule)];
  std::wstring out = L"最接近「" + rule.label + L"」" + fixed3(score.best) + L"：";
  bool named = false;
  for (std::size_t i = 0; i < score.anchors.size() && i < rule.anchors.size(); ++i)
    if (score.anchors[i] < rule.anchors[i].minimum) {
      out += L"锚点" + std::to_wstring(i) + L" " + fixed3(score.anchors[i]) + L" < " +
             fixed3(rule.anchors[i].minimum);
      named = true;
      break;
    }
  if (!named)
    out += L"宽高比或阈值不符";
  return out + L"。";
}
void log(const std::wstring &msg) {
  std::lock_guard lock(logMutex);
  try {
    auto p = std::filesystem::path(directory) / L"agent.log";
    if (std::filesystem::exists(p) && std::filesystem::file_size(p) > 512 * 1024) {
      auto old = p;
      old += L".1";
      std::error_code e;
      std::filesystem::remove(old, e);
      std::filesystem::rename(p, old, e);
    }
    SYSTEMTIME t{};
    GetLocalTime(&t);
    std::ofstream f(p, std::ios::app);
    f << t.wYear << '-' << t.wMonth << '-' << t.wDay << ' ' << t.wHour << ':' << t.wMinute << ':'
      << t.wSecond << ' ' << utf8(msg) << '\n';
  } catch (...) {
  }
}
void publish(const char *state, const std::wstring &detail, DWORD pid = 0, double score = 0,
             Millis next = 0) {
  bool changed;
  {
    std::lock_guard lock(statusMutex);
    if (!running && std::strcmp(state, "STOPPED") != 0)
      return;
    changed = status.state != state || status.detail != detail;
    status.state = state;
    status.detail = detail;
    status.pid = pid;
    status.score = score;
    status.next = next;
  }
  if (changed) {
    log(wide(state) + L" | " + detail);
    {
      std::lock_guard lock(networkMutex);
      networkDirty = true;
    }
    networkWake.notify_one();
  }
}
Config snapshotConfig() {
  std::lock_guard lock(configMutex);
  return config;
}
void telemetry() {
  Millis last{};
  for (;;) {
    {
      std::unique_lock lock(networkMutex);
      networkWake.wait_for(lock, std::chrono::seconds(1), [] { return quitting.load(); });
      if (quitting)
        return;
      if (GetTickCount64() - last < 2000)
        continue;
      if (!networkDirty && GetTickCount64() - last < 30000)
        continue;
      networkDirty = false;
    }
    auto c = snapshotConfig();
    Status s;
    {
      std::lock_guard lock(statusMutex);
      s = status;
    }
    last = GetTickCount64();
    std::ostringstream j;
    j << "{\"machine\":\"" << jsonEscape(utf8(c.machine)) << "\",\"state\":\"" << s.state
      << "\",\"detail\":\"" << jsonEscape(utf8(s.detail)) << "\",\"pid\":" << s.pid
      << ",\"uptime\":" << (last - bootTime) / 1000 << ",\"restarts\":" << s.restarts
      << ",\"score\":" << std::fixed << std::setprecision(3) << s.score
      << ",\"dry_run\":" << (c.dryRun ? "true" : "false") << ",\"version\":\"" << Version << "\"}";
    int code = heartbeat(c, j.str());
    {
      std::lock_guard lock(statusMutex);
      status.http = code;
    }
  }
}
void worker() {
  Capture capture;
  Frame frame;
  StepGate gate;
  RoomSession room;
  ContinuousEvidence disconnect;
  CNJoinGate cnJoin;
  RecoveryBudget budget;
  unsigned epoch = ~0u;
  Millis tickAt{}, missingSince{}, hungSince{}, unknownSince{}, desktopPaused{},
      launchDeadline{}, lastProcessScan{}, windowSince{}, activationDeadline{}, standbySince{},
      lastUnmatchedSave{};
  int typedPage = -1;
  Process game{}, launcher{};
  Config c;
  bool blocked{}, cycleBudgeted{}, waitingGame{}, activationPending{}, cleanupChecked{};
  bool launcherCoverRetried{};
  int unmatchedSaved{};
  Millis launcherCoverSince{};
  for (;;) {
    input.checkFocus();
    {
      std::unique_lock lock(networkMutex);
      wake.wait_for(lock, std::chrono::milliseconds(100), [] { return quitting.load(); });
    }
    if (quitting) {
      input.release();
      return;
    }
    if (!running) {
      input.release();
      continue;
    }
    auto now = GetTickCount64();
    if (epoch != generation.load()) {
      epoch = generation.load();
      c = snapshotConfig();
      gate.reset();
      room.reset();
      disconnect.reset();
      cnJoin.reset();
      missingSince = hungSince = unknownSince = desktopPaused = launchDeadline =
          lastProcessScan = windowSince = activationDeadline = standbySince = 0;
      typedPage = -1;
      blocked = false;
      cycleBudgeted = false;
      waitingGame = false;
      activationPending = true;
      cleanupChecked = false;
      tickAt = 0;
      launcherCoverRetried=false;
      launcherCoverSince=0;
    }
    if (now < tickAt)
      continue;
    tickAt = now + 1000;
    if (!interactiveDesktop()) {
      input.release();
      gate.observe(-1);
      disconnect.reset();
      if (!desktopPaused) desktopPaused = now;
      publish("WAITING_DESKTOP", L"等待返回普通桌面；如有 NeacProtect 管理员授权，请手动处理。",
              game.pid);
      continue;
    }
    if (desktopPaused) {
      if (launchDeadline) launchDeadline += now - desktopPaused;
      desktopPaused = hungSince = windowSince = 0;
      lastProcessScan = 0;
    }
    bool requestedRestart=restartRequested.load()==epoch;
    if (requestedRestart) blocked=false;
    if (blocked) {
      input.release();
      continue;
    }
    // While the player is at the computer the helper is redundant: they can see a
    // disconnect themselves, the pulses are global events that would land in
    // whatever application they are using rather than in the game, and captures
    // would be spent on a screen nobody is watching. Standing down is therefore
    // strictly cheaper than continuing, and it resumes by itself once their input
    // stops. This is deliberately placed after `blocked` so a configuration or
    // permission problem is still reported instead of being masked by standby.
    // The game being in front means the player is back at it, so the countdown
    // ends at once rather than running out: clicking the game is itself the signal
    // that they want it driven again. Without this the helper sits in standby while
    // the game is focused, never recognises the settings screen it was left on and
    // never presses Escape to close it.
    if (c.standbyOnInput && input.playerBusy() && !foreground(game.window)) {
      input.release();
      gate.observe(-1);
      disconnect.reset();
      if (!standbySince)
        standbySince = now;
      publish("STANDBY",
              L"你正在使用电脑，助手待机中；不发按键、不检测，停手约 1 分钟后自动恢复并切回游戏（" +
                  input.activityText() + L"）。",
              game.pid);
      continue;
    }
    if (standbySince) {
      // Deadlines must not expire while nobody was watching.
      if (launchDeadline)
        launchDeadline += now - standbySince;
      standbySince = 0;
      lastProcessScan = 0;
      // The input that ended standby must not immediately re-arm it, or the
      // helper resumes for one tick and sleeps again.
      input.clearPlayerHold();
      // Resuming is useless while the game is behind the window the player left
      // open, and the pulses would keep landing in that window. Ask for the
      // foreground back; the settings screen they usually leave open is closed by
      // the escape rule once it is recognised.
      activationPending = true;
      log(L"你已停手，助手恢复运行，并请求切回游戏窗口。");
    }
    // A higher-integrity game silently swallows every input we send: SendInput
    // reports success and the log says "已执行", while nothing reaches the game.
    // Saying so up front is far cheaper than debugging a run that clicks into the
    // void, which is exactly what this cost twice.
    if (game.pid && outranks(game)) {
      publish("NEEDS_USER",
              L"游戏以管理员权限运行，助手是普通权限，Windows 会丢弃助手发送的全部按键和点击。"
              L"请关闭助手，以管理员身份重新运行后再开始。",
              game.pid);
      blocked = true;
      continue;
    }
    if (now - lastProcessScan >= 2000 || lastProcessScan == 0) {
      auto discovered = findProcess(c.gameExe, c.gameClass);
      if (discovered.pid != game.pid || discovered.created != game.created) {
        if (game.pid || cycleBudgeted || launchDeadline)
          activationPending = true;
        input.release();
        gate.reset();
        room.reset();
        disconnect.reset();
        cnJoin.reset();
        hungSince = windowSince = 0;
        typedPage = -1;
        cleanupChecked = false;
      }
      game = discovered;
      launcher = c.demo ? Process{} : findProcess(L"Battle.net.exe", L"Chrome_WidgetWin_0");
      lastProcessScan = now;
    }
    auto recover = [&](const std::wstring &reason, bool manual = false) {
      input.release();
      room.reset();
      cnJoin.reset();
      if (!running || epoch != generation.load())
        return;
      std::error_code neacError,launcherError;
      if (!c.demo && ((!c.cnPreset && std::none_of(c.steps.begin(),c.steps.end(),[](const Step &s){return s.launcher && s.action==Action::Launch;})) ||
          !validNeacPath(c.neacPath) || !std::filesystem::is_regular_file(c.neacPath,neacError) ||
          ((!launcher.pid || !launcher.window) && !std::filesystem::is_regular_file(c.launcherPath,launcherError)))) {
        publish("NEEDS_CONFIG", L"重启前请填写 OWNeacClient.exe 完整路径和启动器，并启用国服预设或录制战网启动按钮。游戏尚未关闭。",game.pid);
        blocked=true;
        return;
      }
      if (c.dryRun) {
        publish("DRY_RUN", L"演练：将恢复游戏。" + reason, game.pid);
        if (manual) blocked=true;
        return;
      }
      if (!c.autoRecover && !manual) {
        publish("NEEDS_USER", L"自动恢复已关闭。" + reason, game.pid);
        return;
      }
      if (!budget.available(now)) {
        publish("BACKOFF", L"恢复冷却或达到每小时 3 次上限。", game.pid, 0, budget.retryAt());
        return;
      }
      if (!budget.take(now))
        return;
      cycleBudgeted = true;
      {
        std::lock_guard lock(statusMutex);
        ++status.restarts;
      }
      publish("RECOVERING", reason, game.pid);
      if (game.pid && !terminateGame(game, c.gameExe, running, generation, epoch)) {
        if (!running || epoch != generation.load())
          return;
        publish("NEEDS_USER", L"无法关闭已验证的游戏进程；请手动处理。", game.pid);
        blocked = true;
        return;
      }
      game = {};
      gate.reset();
      disconnect.reset();
      typedPage = -1;
      missingSince = GetTickCount64();
      lastProcessScan = 0;
      launchDeadline = 0;
      activationPending = true;
      waitingGame = cleanupChecked = false;
      launcherCoverRetried=false;
      launcherCoverSince=0;
    };
    auto continueRoom = [&](double score, bool background) {
      if (!room.active() || !running || epoch != generation.load()) return;
      if (background && c.holdShift && !c.pulseShift) {
        input.release();
        publish("WAITING_FOCUS", L"持续按住模式只在前台工作；游戏回到前台后继续。", game.pid, score);
        return;
      }
      // The pulse is a global event addressed to whatever window has focus, so
      // while the player is working it reaches their application instead of the
      // game. Standby already covers that case before this point; this is the
      // fallback for the transition second, where standby has not yet engaged.
      if (c.standbyOnInput && c.pulseShift && input.playerBusy()) {
        input.release();
        publish("STANDBY",
                L"你正在使用电脑，助手待机中（" + input.activityText() + L"）。", game.pid, score);
        return;
      }
      bool ok = true;
      if (c.dryRun || !c.holdShift) input.release();
      else if (!sameProcess(game, c.gameExe)) {
        input.release();
        room.reset();
        lastProcessScan = 0;
        publish("INPUT_BLOCKED", L"无法核对游戏进程身份，已暂停按键。", game.pid);
        return;
      } else if (c.pulseShift) ok = input.pulse(game.window, epoch, c.backgroundPulse);
      else ok = input.hold(game.window, epoch);
      publish(c.dryRun ? "DRY_RUN" : (!ok ? "INPUT_BLOCKED" : (background ? "BACKGROUND_INPUT" : "ACTIVE")),
              c.dryRun ? (c.holdShift ? L"演练：房间会话已确认，将持续发送 Shift。"
                                     : L"演练：房间会话已确认，仅监控。")
                       : (!ok ? L"Shift 本次未发送；检查手动按键、游戏身份或权限。"
                              : (!c.holdShift ? L"房间会话已确认，仅监控。"
                                 : (background ? L"继续全局点按 Shift；游戏被遮挡/最小化时暂停画面检测。"
                                    : (c.pulseShift ? L"房间会话已确认，每秒请求点按左 Shift（20 毫秒）。"
                                                    : L"房间会话已确认，保持左 Shift。")))),
              game.pid, score);
    };
    if (requestedRestart) {
      unsigned requestEpoch=epoch;
      if (restartRequested.compare_exchange_strong(requestEpoch,~0u))
        recover(L"手动请求：关闭当前游戏、核验并清理残留，再通过战网启动。",true);
      continue;
    }
    if (game.pid) {
      missingSince = 0;
      launchDeadline = 0;
      waitingGame = false;
      cycleBudgeted = false;
      if (!game.window) {
        input.release();
        if (!windowSince)
          windowSince = now;
        publish("STARTING", L"进程已存在，等待游戏窗口。", game.pid);
        if (now - windowSince > 120000)
          recover(L"游戏进程存在，但 120 秒未出现主窗口。");
        continue;
      }
      windowSince = 0;
      if (!responsive(game.window)) {
        input.release();
        if (!hungSince)
          hungSince = now;
        publish("WINDOW_HUNG", L"窗口暂未响应，连续确认中。", game.pid);
        if (now - hungSince > 30000)
          recover(L"游戏窗口连续 30 秒没有响应。");
        continue;
      }
      hungSince = 0;
    } else {
      input.release();
      room.reset();
      disconnect.reset();
      unknownSince = 0;
      if (!missingSince)
        missingSince = now;
      if (!c.autoRecover && !cycleBudgeted) {
        publish("GAME_NOT_RUNNING", L"未发现游戏；自动恢复已关闭。");
        continue;
      }
      if (launchDeadline && now >= launchDeadline) {
        publish("NEEDS_USER", L"启动后 120 "
                              L"秒未发现游戏；检查更新、登录或启动按钮。请停止后重新开始。");
        blocked = true;
        continue;
      }
      if (!waitingGame && !cleanupChecked && !c.demo) {
        // Allow a manually started client to finish launching before considering it stale.
        Millis grace = cycleBudgeted ? 3000 : 30000;
        if (now - missingSince < grace) {
          publish("STARTING", L"未发现游戏，等待进程退出或启动完成，再核对残留进程。", 0, 0,
                  missingSince + grace);
          continue;
        }
        auto result = cleanupNeac(c, running, generation, epoch);
        if (result == CleanupResult::Cancelled) continue;
        if (result == CleanupResult::GamePresent) { lastProcessScan = 0; continue; }
        if (result != CleanupResult::Clear) {
          publish(result == CleanupResult::NeedsPath ? "NEEDS_CONFIG" : "NEEDS_USER",
                  result == CleanupResult::NeedsPath
                      ? L"检测到残留 OWNeacClient.exe，请停止后填写其完整路径。"
                      : L"无法核验或结束残留 OWNeacClient.exe（路径/权限）。请手动处理后停止、重新开始。");
          blocked = true;
          continue;
        }
        cleanupChecked = true;
        log(c.dryRun ? L"演练：残留进程检查通过，未结束任何进程。"
                     : L"游戏已退出，残留进程清理检查通过。");
      }
      // An existing Battle.net process may be tray-only. Opening its configured
      // entry point requests the existing main window; the deadline prevents
      // repeatedly launching while waiting for it to appear.
      if (!launcher.pid || (!launcher.window && activationPending)) {
        if (launchDeadline) {
          publish("STARTING", L"等待启动器或游戏出现。", 0, 0, launchDeadline);
          continue;
        }
        if (c.launcherPath.empty() || !std::filesystem::exists(c.launcherPath)) {
          publish("NEEDS_CONFIG", L"未找到启动器，请选择战网 EXE 或你平时使用的快捷方式。");
          continue;
        }
        if (c.dryRun) {
          publish("DRY_RUN", L"演练：游戏未运行，将打开配置的启动器。");
          continue;
        }
        if (!cycleBudgeted) {
          if (!budget.available(now)) {
            publish("BACKOFF", L"启动冷却中；每小时最多 3 次恢复。", 0, 0, budget.retryAt());
            continue;
          }
          if (!budget.take(now))
            continue;
          cycleBudgeted = true;
          {
            std::lock_guard lock(statusMutex);
            ++status.restarts;
          }
        }
        if (!running || epoch != generation.load())
          continue;
        if (!launch(c)) {
          publish("NEEDS_USER", L"启动器启动失败，请检查路径和权限。");
          blocked = true;
          continue;
        }
        waitingGame = c.demo;
        launchDeadline = now + 120000;
        publish("STARTING", L"已启动，等待窗口。", 0, 0, launchDeadline);
        continue;
      }
    }
    bool isLauncher = !game.pid;
    Process target = isLauncher ? launcher : game;
    if (isLauncher && launchDeadline && waitingGame) {
      publish("STARTING", L"等待游戏启动；可在战网中检查更新或登录。", launcher.pid, 0,
              launchDeadline);
      continue;
    }
    if (!target.window) {
      publish("WAITING_WINDOW", L"启动器进程存在但窗口不可见，请打开战网窗口。", target.pid);
      continue;
    }
    // Request focus once per start/recovery transition. Manual focus changes while
    // running do not cause repeated focus stealing. A refused SetForegroundWindow
    // only flashes the taskbar button, so one request is not proof of success:
    // keep asking for a bounded window, then stop and record why.
    if (activationPending) {
      if (foreground(target.window)) {
        activationPending = false;
        activationDeadline = 0;
      } else if (!activationDeadline) {
        activationDeadline = now + 10000;
        requestForeground(target.window);
      } else if (now < activationDeadline) {
        requestForeground(target.window);
      } else {
        activationPending = false;
        activationDeadline = 0;
        log(outranks(target) ? L"前台切换超时：目标以更高权限运行，Windows 拒绝切换（UIPI）。"
                             : L"前台切换超时：Windows 拒绝了前台切换。");
      }
    }
    bool background = !foreground(target.window);
    bool backgroundRoom = !isLauncher && room.active() && c.pulseShift && c.backgroundPulse;
    bool backgroundCapture = !isLauncher && c.backgroundPulse;
    auto availability=captureAvailability(target.window,true);
    bool launcherCover=!isLauncher && background && c.cnPreset && c.autoRecover && !c.dryRun &&
        availability.code=="occluded" && launcher.pid && availability.blockerPid==launcher.pid;
    if (launcherCover) {
      if (!launcherCoverSince) launcherCoverSince=now;
      if (!launcherCoverRetried && now-launcherCoverSince>=5000 && running && generation.load()==epoch) {
        launcherCoverRetried=true;
        ShowWindow(target.window,SW_RESTORE);
        SetForegroundWindow(target.window);
        log(L"战网遮挡游戏超过 5 秒，已请求一次切回游戏以继续检查和导航。");
        continue;
      }
    } else {launcherCoverSince=0;launcherCoverRetried=false;}
    if (background && !backgroundRoom && !(backgroundCapture && unobscured(target.window))) {
      input.release();
      gate.observe(-1);
      disconnect.reset();
      // A window owned by a higher-integrity process can never be activated or
      // receive our input, so asking the user to click it would be misleading.
      bool uipi = outranks(target);
      publish(uipi ? "NEEDS_USER" : "WAITING_FOCUS",
              uipi ? (isLauncher ? L"战网以管理员权限运行，助手是普通权限；Windows 会阻止助手切换窗口和发送点击。"
                                   L"请以管理员身份运行助手，或让战网以普通权限运行。"
                                 : L"游戏以管理员权限运行，助手是普通权限；Windows 会阻止助手发送按键。"
                                   L"请以管理员身份运行助手。")
                   : (isLauncher ? L"请把战网窗口置于前台；切换回来后自动继续。"
                                 : L"已释放 Shift。游戏回到前台后自动继续。"),
              target.pid);
      continue;
    }
    if (!capture.grab(target.window, frame, backgroundCapture)) {
      input.release();
      gate.observe(-1);
      disconnect.reset();
      if (backgroundRoom) { continueRoom(0, background); continue; }
      publish("CAPTURE_FAILED", capture.failure().message(), target.pid);
      continue;
    }
    bool hasTemplates = std::any_of(c.steps.begin(), c.steps.end(),
                                    [&](auto &s) { return s.launcher == isLauncher; });
    if (!hasTemplates && (!c.cnPreset || c.demo)) {
      input.release();
      publish("NEEDS_CALIBRATION",
              isLauncher ? L"请校准战网启动按钮（操作选择“战网启动游戏”）。"
                         : L"请先校准大厅入口、房间导航和“到达目标房间”。",
              target.pid);
      continue;
    }
    auto m = match(frame, c.steps, isLauncher, c.threshold);
    CNObservation builtin;
    CNLauncherScore launcherScore;
    CNGameScore gameScore;
    if (c.cnPreset && !c.demo)
      builtin=isLauncher ? recognizeCNLauncher(frame,GetDpiForWindow(target.window)/96.0,c.cnProfile.launcher,&launcherScore)
                         : recognizeCNGame(frame,c.cnProfile.rules,c.cnProfile.join,&gameScore);
    builtin=cnJoin.prepare(std::move(builtin),now,cnCardClick(c.cnProfile));
    bool useBuiltin=builtin && (builtin.step.action==Action::Restart ||
        builtin.step.action==Action::Ignore || (m.index<0 && !m.ambiguous) ||
        (m.index>=0 && c.steps[m.index].action==Action::Ready && builtin.step.action!=Action::Ready));
    if (useBuiltin) {m.index=builtin.key();m.score=builtin.score;m.ambiguous=false;m.aspectMismatch=false;}
    if (m.index < 0) {
      room.observe(nullptr, false, m.ambiguous);
      gate.observe(-1);
      disconnect.reset();
      if (room.active()) { continueRoom(m.score, background); continue; }
      input.release();
      if (!unknownSince)
        unknownSince = now;
      // Stuck on an unrecognised page for a while: that frame is the evidence, and
      // it is saved whatever the score. An earlier "near miss >= 0.80" condition
      // skipped exactly the interesting case, where a page matches no rule at all
      // or matches only intermittently and so never passes the two-frame gate.
      if (!isLauncher && unknownSince && now - unknownSince >= 5000 && unmatchedSaved < 40 &&
          now - lastUnmatchedSave >= 30000) {
        lastUnmatchedSave = now;
        std::error_code ec;
        auto dir = std::filesystem::path(directory) / L"unmatched";
        std::filesystem::create_directories(dir, ec);
        SYSTEMTIME t{};
        GetLocalTime(&t);
        std::wostringstream name;
        name << L"miss-" << std::setfill(L'0') << std::setw(2) << t.wHour << std::setw(2)
             << t.wMinute << std::setw(2) << t.wSecond << L"-" << std::setw(3)
             << t.wMilliseconds << L"-" << ++unmatchedSaved << L".bmp";
        auto path = dir / name.str();
        if (saveFrameBmp(frame, path))
          log(L"未识别画面已保存：" + path.wstring());
      }
      publish(m.ambiguous ? "AMBIGUOUS" : "UNKNOWN",
              m.ambiguous ? L"多个页面特征过于相似，请重新圈选更独特的区域。"
                          : (m.aspectMismatch ? L"页面未匹配；检查窗口宽高比是否与校准时一致。"
                             : (isLauncher && launcherScore.evaluated
                                    ? L"战网启动页未匹配：锚点 " + launcherScoresText(launcherScore) +
                                          L"（每个锚点都要达到自己的阈值）。"
                                    : (!isLauncher && gameScore.evaluated
                                           ? gameClosestText(gameScore, c.cnProfile.rules)
                                           : L"未匹配已校准页面，等待加载或新的校准。"))),
              target.pid, m.score);
      continue;
    }
    unknownSince = 0;
    const auto &s = useBuiltin ? builtin.step : c.steps[m.index];
    if (!running || epoch != generation.load())
      continue;
    int key = observationKey(s, m.index);
    bool confirmed = gate.observe(key);
    room.observe(&s, confirmed);
    disconnect.observe(s.action == Action::Restart, now);
    if (!confirmed) {
      if (room.active()) { continueRoom(m.score, background); continue; }
      input.release();
      publish("CONFIRMING", L"确认页面：" + s.label, target.pid, m.score);
      continue;
    }
    gate.arrived(key);
    if (typedPage >= 0 && typedPage != m.index)
      typedPage = -1;
    if (s.action == Action::Ready) {
      continueRoom(m.score, background);
      continue;
    }
    input.release();
    if (s.action == Action::Restart) {
      Millis delay = c.demo ? 5000 : 90000;
      publish("LOGIN_STUCK", L"已识别恢复页面：" + s.label + L"；等待持续确认。", target.pid,
              m.score, disconnect.deadline(delay));
      if (disconnect.elapsed(now, delay)) {
        if (isLauncher) {
          publish("NEEDS_USER", L"战网需要登录或验证，请处理后继续。", target.pid);
          continue;
        }
        recover(L"登录/错误页面持续超时。");
      }
      continue;
    }
    if (background) {
      publish("WAITING_FOCUS", L"已离开房间画面，暂停按键；请把游戏切回前台继续导航。", target.pid);
      continue;
    }
    if (c.dryRun) {
      publish("DRY_RUN", L"演练：" + s.label + L" → " + ActionNames[int(s.action)], target.pid,
              m.score);
      continue;
    }
    // "自动加入" is worth retrying quickly and often: if the click did not take,
    // the lobby may have filled in that instant, and the normal 5s/3 attempts
    // budget would stop and demand a manual restart instead of just asking again.
    const bool joining = useBuiltin && builtin.page == CNPage::Join;
    const Millis cooldown = joining ? 1500 : 5000;
    const int maximum = joining ? 8 : 3;
    if (!gate.cooldownReady(now, cooldown)) {
      publish("NAVIGATING", L"等待页面变化：" + s.label, target.pid, m.score);
      continue;
    }
    if (gate.exhausted(m.index, maximum) || (s.action == Action::Text && typedPage == m.index)) {
      publish("NEEDS_USER",
              L"操作后页面没有变化：" + s.label + L"。为避免重复输入已暂停；停止后重新开始。",
              target.pid, m.score);
      blocked = true;
      continue;
    }
    if (!gate.canAct(m.index, now, cooldown, maximum)) {
      publish("NAVIGATING", L"等待页面变化：" + s.label, target.pid, m.score);
      continue;
    }
    bool ok = false;
    if (!sameProcess(target, isLauncher ? L"Battle.net.exe" : c.gameExe)) {
      publish("WAITING_WINDOW", L"进程已变化，重新检测。");
      lastProcessScan = 0;
      continue;
    }
    if (s.action == Action::Launch && !cycleBudgeted) {
      if (!budget.take(now)) {
        publish("BACKOFF", L"启动冷却中。", target.pid, 0, budget.retryAt());
        continue;
      }
      cycleBudgeted = true;
      {
        std::lock_guard lock(statusMutex);
        ++status.restarts;
      }
    }
    // A recognised page that must never be acted on: the room session has
    // already been cleared above, so all that is left is to say so and wait.
    if (s.action == Action::Ignore) {
      publish("IDLE", L"已知的非对局页面：" + s.label + L"；已释放 Shift 并结束房间会话。",
              target.pid, m.score);
      continue;
    }
    switch (s.action) {
    case Action::Click:
    case Action::Launch:
      ok = input.click(target.window, s.click, epoch);
      break;
    case Action::Enter:
      ok = input.press(target.window, VK_RETURN, epoch);
      break;
    case Action::Escape:
      ok = input.press(target.window, VK_ESCAPE, epoch);
      break;
    case Action::Text:
      ok = input.click(target.window, s.click, epoch) && input.text(target.window, s.text, epoch) &&
           input.press(target.window, VK_RETURN, epoch);
      typedPage = m.index;
      break;
    default:
      break;
    }
    // Partial SendInput failures are attempts too: never retry them every second forever.
    gate.acted(m.index, now);
    if (useBuiltin) cnJoin.acted(builtin.page,ok,now);
    // The join click is itself the evidence of being in a room: the lobby, the
    // map intro, spectating and the results screen all follow it and none shows
    // the HUD the Ready template was recorded from. Without this the pulses only
    // start once the first match HUD appears, which can be minutes later.
    if (useBuiltin && builtin.page == CNPage::Join && !c.dryRun) {
      room.joined();
      log(L"已确认加入房间，立即开始房间会话。");
    }
    if (!ok) {
      publish("INPUT_BLOCKED",
              outranks(target) ? L"输入被 Windows 拒绝：目标以更高权限运行（UIPI）。"
                                 L"请以管理员身份运行助手。"
                               : L"输入未成功或焦点改变，请检查窗口权限。",
              target.pid);
      continue;
    }
    if (s.action == Action::Launch) {
      waitingGame = true;
      activationPending = true;
      launchDeadline = now + 120000;
      publish("STARTING", L"已点击启动，等待游戏窗口。", target.pid, m.score, launchDeadline);
    } else
      publish("NAVIGATING", L"已执行：" + s.label, target.pid, m.score);
  }
}
void refreshSteps() {
  SendMessageW(control(Steps), LB_RESETCONTENT, 0, 0);
  auto c = snapshotConfig();
  auto add = [&](const std::wstring &line) {
    SendMessageW(control(Steps), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
  };
  for (auto &s : c.steps) {
    auto line =
        (s.launcher ? L"[战网] " : L"[游戏] ") + s.label + L"  →  " + ActionNames[int(s.action)];
    add(line);
  }
  // The preset lives in profiles/cn-16x9.json and is not edited from this list,
  // but leaving it out made the panel claim two steps while the helper was acting
  // on eight more. Preset entries come after the manual ones on purpose: the
  // remove handler already ignores any index past c.steps, so they are read-only.
  if (!c.cnPreset)
    return;
  if (!c.cnProfileError.empty()) {
    add(L"—— 国服预设加载失败：" + wide(c.cnProfileError));
    return;
  }
  if (c.cnProfile.rules.empty() && c.cnProfile.launcher.empty())
    return;
  std::wstring source = c.cnProfileSource == "local"     ? L"本机覆盖"
                        : c.cnProfileSource == "shipped" ? L"随包文件"
                        : c.cnProfileSource == "builtin" ? L"内置回退"
                                                         : L"未加载";
  add(L"—— 国服预设 · " + source + L" · 只读 ——");
  if (!c.cnProfile.launcher.empty())
    add(L"[战网] 战网启动锚点 ×" + std::to_wstring(c.cnProfile.launcher.size()) +
        L"  →  战网启动游戏   （预设）");
  for (const auto &rule : c.cnProfile.rules)
    add(L"[游戏] " + rule.label + L"  →  " + ActionNames[int(rule.action)] + L"   （预设）");
}
bool syncConfig() {
  auto c = snapshotConfig();
  c.gameExe = textOf(GameExe);
  c.launcherPath = textOf(Launcher);
  c.launcherArgs = textOf(Args);
  c.server = textOf(Server);
  c.token = textOf(Token);
  c.machine = textOf(Machine);
  c.dryRun = checked(Dry);
  c.holdShift = checked(Hold);
  c.autoRecover = checked(Auto);
  c.pulseShift = SendMessageW(control(ShiftMode), CB_GETCURSEL, 0, 0) == 0;
  c.backgroundPulse = false;  // 全局后台点按已移除：全局按键只会送到焦点窗口
  c.cnPreset = !c.demo && checked(CnPreset);
  c.neacPath = textOf(NeacPath);
  if (!c.neacPath.empty() && !validNeacPath(c.neacPath)) {
    MessageBoxW(mainWindow, L"残留进程路径必须是 OWNeacClient.exe 的完整绝对路径。", L"配置未保存",
                MB_ICONWARNING);
    return false;
  }
  if (c.gameExe.empty() || c.gameExe.find_first_of(L"/\\") != std::wstring::npos ||
      c.machine.empty() || utf8(c.machine).size() > 64 ||
      c.machine.find_first_of(L"/\\\r\n\t") != std::wstring::npos ||
      c.token.find_first_of(L"\r\n") != std::wstring::npos || c.token.size() > 512 ||
      c.launcherArgs.size() > 2048) {
    MessageBoxW(mainWindow, L"检查进程文件名、机器名和令牌格式。", L"配置未保存", MB_ICONWARNING);
    return false;
  }
  if (!c.server.empty() && c.server.rfind(L"https://", 0) != 0 &&
      c.server.rfind(L"http://", 0) != 0) {
    MessageBoxW(mainWindow, L"看板地址需以 http:// 或 https:// 开头。", L"地址格式",
                MB_ICONWARNING);
    return false;
  }
  try {
    saveConfig(directory, c);
    std::lock_guard lock(configMutex);
    config = std::move(c);
    return true;
  } catch (...) {
    MessageBoxW(mainWindow, L"配置保存失败，检查本地目录权限。", L"保存失败", MB_ICONERROR);
    return false;
  }
}
void setRunning(bool on, bool manualRestart = false) {
  if (on && running && manualRestart) {
    restartRequested=generation.load();
    wake.notify_all();
    return;
  }
  if (on && (running || calibrating))
    return;
  if (on && !syncConfig())
    return;
  ++generation;
  restartRequested=on && manualRestart ? generation.load() : ~0u;
  if (on) {
    input.enable(!snapshotConfig().dryRun, generation.load());
    running = true;
  } else {
    running = false;
    input.enable(false, generation.load());
    cancelCalibration();
  }
  if (on) {
    publish("DETECTING", L"正在检测游戏与页面。");
  } else {
    input.release();
    publish("STOPPED", L"已停止，已释放本程序按下的 Shift。");
  }
  for (int id : settingsIds)
    EnableWindow(control(id), !on);
  EnableWindow(control(Start), !on);
  wake.notify_all();
  if (on) {
    auto c = snapshotConfig();
    auto p = findProcess(c.gameExe, c.gameClass);
    if (!p.window && !c.demo)
      p = findProcess(L"Battle.net.exe", L"Chrome_WidgetWin_0");
    if (p.window) {
      ShowWindow(p.window, SW_RESTORE);
      SetForegroundWindow(p.window);
    }
  }
}

struct Calibration {
  Frame frame;
  Step step;
  HWND target{};
  POINT start{}, end{};
  bool dragging{}, chooseClick{};
};
Calibration calibration;
void cancelCalibration() {
  KillTimer(mainWindow, 2);
  if (calibrationWindow)
    DestroyWindow(calibrationWindow);
  if (calibrating) {
    calibrating = false;
    calibration = Calibration{};
    ShowWindow(mainWindow, SW_RESTORE);
  }
  EnableWindow(control(Start), !running);
  EnableWindow(control(Record), !running);
}
void finishCalibration(HWND overlay) {
  auto &s = calibration.step;
  s.image = sample(calibration.frame, s.roi);
  if (contrast(s.image) < 8) {
    MessageBoxW(overlay, L"区域过于单一，请圈选包含文字、图标或边缘的独特页面特征。", L"重新选区域",
                MB_ICONINFORMATION);
    calibration.chooseClick = false;
    InvalidateRect(overlay, nullptr, FALSE);
    return;
  }
  auto c = snapshotConfig();
  if (c.steps.size() >= 32) {
    MessageBoxW(overlay, L"最多保存 32 个页面，请先删除不用的页面。", L"页面已满", MB_ICONWARNING);
    DestroyWindow(overlay);
    return;
  }
  c.steps.push_back(s);
  try {
    saveConfig(directory, c);
    {
      std::lock_guard lock(configMutex);
      config = std::move(c);
    }
    publish("STOPPED", L"校准已保存：" + s.label);
  } catch (...) {
    MessageBoxW(overlay, L"无法保存校准。", L"保存失败", MB_ICONERROR);
  }
  DestroyWindow(overlay);
}
LRESULT CALLBACK overlayProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
  auto point = [&]() { return POINT{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}; };
  switch (msg) {
  case WM_KEYDOWN:
    if (wp == VK_ESCAPE) {
      DestroyWindow(w);
      return 0;
    }
    break;
  case WM_LBUTTONDOWN:
    if (calibration.chooseClick) {
      auto p = point();
      calibration.step.click = {double(p.x) / calibration.frame.width,
                                double(p.y) / calibration.frame.height};
      finishCalibration(w);
      return 0;
    }
    calibration.start = calibration.end = point();
    calibration.dragging = true;
    SetCapture(w);
    return 0;
  case WM_MOUSEMOVE:
    if (calibration.dragging) {
      calibration.end = point();
      InvalidateRect(w, nullptr, FALSE);
    }
    return 0;
  case WM_LBUTTONUP:
    if (calibration.dragging) {
      ReleaseCapture();
      calibration.dragging = false;
      calibration.end = point();
      int x = std::clamp(std::min(calibration.start.x, calibration.end.x), 0L,
                         LONG(calibration.frame.width));
      int y = std::clamp(std::min(calibration.start.y, calibration.end.y), 0L,
                         LONG(calibration.frame.height));
      int right = std::clamp(std::max(calibration.start.x, calibration.end.x), 0L,
                             LONG(calibration.frame.width));
      int bottom = std::clamp(std::max(calibration.start.y, calibration.end.y), 0L,
                              LONG(calibration.frame.height));
      if (right - x < 24 || bottom - y < 16)
        return 0;
      calibration.step.roi = {double(x) / calibration.frame.width,
                              double(y) / calibration.frame.height,
                              double(right - x) / calibration.frame.width,
                              double(bottom - y) / calibration.frame.height};
      if (calibration.step.action == Action::Click || calibration.step.action == Action::Launch ||
          calibration.step.action == Action::Text) {
        calibration.chooseClick = true;
        InvalidateRect(w, nullptr, FALSE);
      } else
        finishCalibration(w);
    }
    return 0;
  case WM_PAINT: {
    PAINTSTRUCT ps{};
    auto dc = BeginPaint(w, &ps);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = calibration.frame.width;
    bi.bmiHeader.biHeight = -calibration.frame.height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    StretchDIBits(dc, 0, 0, calibration.frame.width, calibration.frame.height, 0, 0,
                  calibration.frame.width, calibration.frame.height,
                  calibration.frame.pixels.data(), &bi, DIB_RGB_COLORS, SRCCOPY);
    auto pen = CreatePen(PS_SOLID, 3, RGB(40, 235, 180));
    auto oldpen = SelectObject(dc, pen);
    auto oldbrush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    if (calibration.dragging || calibration.chooseClick)
      Rectangle(dc, calibration.start.x, calibration.start.y, calibration.end.x, calibration.end.y);
    SelectObject(dc, oldpen);
    SelectObject(dc, oldbrush);
    DeleteObject(pen);
    RECT header{0, 0, calibration.frame.width, 52};
    auto brush = CreateSolidBrush(RGB(18, 28, 43));
    FillRect(dc, &header, brush);
    DeleteObject(brush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    SelectObject(dc, font);
    header.left = 12;
    DrawTextW(dc,
              calibration.chooseClick ? L"现在点击要操作的按钮 / 输入框位置 · Esc 取消"
                                      : L"拖动圈选独特、静止的页面特征 · Esc 取消",
              -1, &header, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    EndPaint(w, &ps);
    return 0;
  }
  case WM_DESTROY:
    calibrationWindow = nullptr;
    calibrating = false;
    calibration.frame.pixels.clear();
    ShowWindow(mainWindow, SW_RESTORE);
    SetForegroundWindow(mainWindow);
    refreshSteps();
    EnableWindow(control(Start), TRUE);
    EnableWindow(control(Record), TRUE);
    return 0;
  }
  return DefWindowProcW(w, msg, wp, lp);
}
void beginCalibration() {
  if (running || calibrating || !syncConfig())
    return;
  calibration = Calibration{};
  calibration.step.label = textOf(Label);
  calibration.step.text = textOf(TextArg);
  calibration.step.launcher = SendMessageW(control(Scope), CB_GETCURSEL, 0, 0) == 1;
  calibration.step.action = Action(SendMessageW(control(ActionBox), CB_GETCURSEL, 0, 0));
  if (calibration.step.label.empty() || calibration.step.label.size() > 128) {
    MessageBoxW(mainWindow, L"页面名称需为 1–128 个字符，例如：大厅入口、房间列表、目标房间。",
                L"页面名称", MB_ICONINFORMATION);
    return;
  }
  if ((calibration.step.launcher && calibration.step.action == Action::Ready) ||
      (!calibration.step.launcher && calibration.step.action == Action::Launch)) {
    MessageBoxW(mainWindow, L"“到达目标房间”仅用于游戏；“战网启动游戏”仅用于战网。", L"校准对象",
                MB_ICONINFORMATION);
    return;
  }
  if (calibration.step.action == Action::Text &&
      (calibration.step.text.empty() || calibration.step.text.size() > 128)) {
    MessageBoxW(mainWindow, L"输入文字操作需要 1–128 个字符。录制时请标记目标输入框。", L"文字内容",
                MB_ICONINFORMATION);
    return;
  }
  auto c = snapshotConfig();
  auto p = calibration.step.launcher ? findProcess(L"Battle.net.exe", L"Chrome_WidgetWin_0")
                                     : findProcess(c.gameExe, c.gameClass);
  if (!p.window) {
    MessageBoxW(mainWindow, L"没有找到目标窗口，请先打开游戏或战网。", L"无法校准", MB_ICONWARNING);
    return;
  }
  calibration.target = p.window;
  calibrating = true;
  EnableWindow(control(Start), FALSE);
  EnableWindow(control(Record), FALSE);
  ShowWindow(mainWindow, SW_MINIMIZE);
  ShowWindow(p.window, SW_RESTORE);
  SetForegroundWindow(p.window);
  SetTimer(mainWindow, 2, 3000, nullptr);
}
void captureCalibration() {
  KillTimer(mainWindow, 2);
  if (!calibrating)
    return;
  Capture cap;
  if (!cap.grab(calibration.target, calibration.frame)) {
    auto message = cap.failure().message();
    log(L"校准截图失败 [" + wide(cap.failure().code) + L"] " + message);
    cancelCalibration();
    MessageBoxW(mainWindow, message.c_str(), L"截图失败",
                MB_ICONWARNING);
    return;
  }
  calibration.step.aspect = double(calibration.frame.width) / calibration.frame.height;
  POINT p{};
  ClientToScreen(calibration.target, &p);
  HWND w = CreateWindowExW(WS_EX_TOPMOST, OverlayClass, L"校准", WS_POPUP, p.x, p.y,
                           calibration.frame.width, calibration.frame.height, mainWindow, nullptr,
                           instance, nullptr);
  calibrationWindow = w;
  if (!w) {
    cancelCalibration();
    return;
  }
  ShowWindow(w, SW_SHOW);
  SetForegroundWindow(w);
  SetFocus(w);
}
HWND add(const wchar_t *cls, const std::wstring &txt, int id, int x, int y, int width, int height,
         DWORD style = 0) {
  auto h = CreateWindowExW(wcscmp(cls, L"EDIT") == 0 ? WS_EX_CLIENTEDGE : 0, cls, txt.c_str(),
                           WS_CHILD | WS_VISIBLE | style, u(x), u(y), u(width), u(height),
                           mainWindow, reinterpret_cast<HMENU>(INT_PTR(id)), instance, nullptr);
  SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(id == StatusTitle ? titleFont : font), TRUE);
  return h;
}
void controls() {
  auto c = snapshotConfig();
  add(L"STATIC", L"OverwatchHelper · 国服试用版", StatusTitle, 22, 17, 810, 30);
  add(L"STATIC", L"", StatusDetail, 22, 56, 810, 42);
  add(L"BUTTON", L"开始", Start, 22, 106, 115, 32, BS_PUSHBUTTON);
  add(L"BUTTON", L"停止 · F8", Stop, 148, 106, 115, 32, BS_PUSHBUTTON);
  add(L"BUTTON", L"保存配置", Save, 274, 106, 110, 32);
  add(L"BUTTON", L"演练（不执行动作）", Dry, 409, 107, 220, 28, BS_AUTOCHECKBOX);
  add(L"BUTTON", L"运行诊断", Diagnose, 650, 106, 168, 32, BS_PUSHBUTTON);
  SendMessageW(control(Dry), BM_SETCHECK, c.dryRun ? BST_CHECKED : BST_UNCHECKED, 0);
  auto edit = [&](const wchar_t *label, int id, const std::wstring &value, int y, int width = 648,
                  DWORD extra = 0) {
    add(L"STATIC", label, 0, 22, y + 4, 118, 24);
    add(L"EDIT", value, id, 144, y, width, 26, ES_AUTOHSCROLL | WS_TABSTOP | extra);
  };
  edit(L"游戏进程", GameExe, c.gameExe, 153, 235);
  add(L"STATIC", L"机器名", 0, 402, 157, 65, 24);
  add(L"EDIT", c.machine, Machine, 473, 153, 345, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  edit(L"启动器 / 快捷方式", Launcher, c.launcherPath, 188, 570);
  add(L"BUTTON", L"浏览", Browse, 726, 188, 92, 26);
  edit(L"启动参数（可空）", Args, c.launcherArgs, 222, 674);
  edit(L"看板服务地址", Server, c.server, 256, 674);
  edit(L"Agent 令牌", Token, c.token, 290, 674, ES_PASSWORD);
  add(L"BUTTON", L"房间内发送 Shift", Hold, 22, 329, 170, 26, BS_AUTOCHECKBOX);
  add(L"COMBOBOX", L"", ShiftMode, 195, 329, 190, 100, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"每秒点按（20ms）", L"持续按住（前台）"})
    SendMessageW(control(ShiftMode), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ShiftMode), CB_SETCURSEL, c.pulseShift ? 0 : 1, 0);
  add(L"BUTTON", L"自动恢复（每小时最多 3 次）", Auto, 22, 365, 350, 26, BS_AUTOCHECKBOX);
  add(L"BUTTON", L"国服预设：最稳轮换版导航与重启", CnPreset, 405, 365, 413, 26, BS_AUTOCHECKBOX);
  SendMessageW(control(CnPreset),BM_SETCHECK,c.cnPreset?BST_CHECKED:BST_UNCHECKED,0);
  edit(L"OWNeac 路径", NeacPath, c.neacPath, 399, 570);
  add(L"BUTTON", L"浏览", NeacBrowse, 726, 399, 92, 26);
  SendMessageW(control(Hold), BM_SETCHECK, c.holdShift ? BST_CHECKED : BST_UNCHECKED, 0);
  SendMessageW(control(Auto), BM_SETCHECK, c.autoRecover ? BST_CHECKED : BST_UNCHECKED, 0);
  add(L"STATIC", L"页面校准 · 手动打开一个页面，选择操作，录制特征与点击位置", 0, 22, 441, 800, 24);
  add(L"COMBOBOX", L"", Scope, 22, 474, 92, 140, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"游戏", L"战网"})
    SendMessageW(control(Scope), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(Scope), CB_SETCURSEL, 0, 0);
  add(L"COMBOBOX", L"", ActionBox, 126, 474, 260, 230, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : ActionNames)
    SendMessageW(control(ActionBox), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ActionBox), CB_SETCURSEL, 0, 0);
  add(L"EDIT", L"页面名称", Label, 399, 474, 220, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"3 秒后录制", Record, 632, 474, 186, 28);
  add(L"STATIC", L"输入文字内容", 0, 22, 512, 115, 24);
  add(L"EDIT", L"", TextArg, 144, 508, 480, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"删除选中页面", Remove, 637, 508, 181, 28);
  add(L"LISTBOX", L"", Steps, 22, 550, 796, 122, WS_BORDER | WS_VSCROLL | LBS_NOTIFY | WS_TABSTOP);
  add(L"STATIC", L"", Stats, 22, 685, 790, 22);
  add(L"STATIC", L"", NetStatus, 22, 715, 480, 22);
  add(L"BUTTON", L"重启游戏", RestartNow, 512, 709, 160, 30);
  add(L"BUTTON", L"配置与日志", OpenData, 682, 709, 136, 30);
  add(L"STATIC",
      L"提示：小窗口保持原宽高比；后台检测要求游戏无遮挡。关闭窗口会退出助手。",
      0, 22, 749, 800, 26);
  settingsIds = {Save,    Browse, Record, Remove, GameExe, Launcher,  Args,  Server, Token,
                 Machine, Dry,    Hold,   Auto,   Scope,   ActionBox, Label, TextArg,
                 ShiftMode, NeacPath, NeacBrowse, CnPreset};
  refreshSteps();
}
void refreshStatus() {
  Status s;
  {
    std::lock_guard lock(statusMutex);
    s = status;
  }
  auto title = L"OverwatchHelper · " + wide(s.state);
  SetWindowTextW(control(StatusTitle), title.c_str());
  SetWindowTextW(control(StatusDetail), s.detail.c_str());
  const auto now = GetTickCount64();
  std::wostringstream stats;
  stats << L"PID " << s.pid << L"    匹配 " << std::fixed << std::setprecision(1) << s.score * 100
        << L"%    恢复次数 " << s.restarts << L"    运行 " << (now - bootTime) / 1000
        << L" 秒";
  if (s.next > now)
    stats << L"    等待 " << (s.next - now) / 1000 << L" 秒";
  SetWindowTextW(control(Stats), stats.str().c_str());
  auto net = s.http == 0 ? L"远程看板：未配置"
                         : (s.http >= 200 && s.http < 300
                                ? L"远程看板：已连接"
                                : L"远程看板：连接失败 / HTTP " + std::to_wstring(s.http));
  SetWindowTextW(control(NetStatus), net.c_str());
}
LRESULT CALLBACK mainProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_COMMAND:
    switch (LOWORD(wp)) {
    case Start:
      setRunning(true);
      break;
    case RestartNow:
      setRunning(true,true);
      break;
    case Stop:
      setRunning(false);
      break;
    case Save:
      if (syncConfig())
        publish("STOPPED", L"配置已保存。");
      break;
    case Record:
      beginCalibration();
      break;
    case Steps:
      // Double-clicking the preset header opens the profile, so thresholds and
      // variants can be edited without hunting for the path. Preset rows sit after
      // the manual ones and the remove handler already ignores them.
      if (HIWORD(wp) == LBN_DBLCLK) {
        auto c = snapshotConfig();
        auto i = SendMessageW(control(Steps), LB_GETCURSEL, 0, 0);
        if (i == int(c.steps.size())) {
          auto local = std::filesystem::path(directory) / L"cn-profile.json";
          auto shipped = std::filesystem::path(exeDirectory()) / L"profiles" / L"cn-16x9.json";
          auto target = std::filesystem::exists(local) ? local : shipped;
          if (std::filesystem::exists(target))
            ShellExecuteW(w, L"open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
          else
            MessageBoxW(w, L"没有找到预设文件。", L"国服预设", MB_ICONINFORMATION);
        }
      }
      break;
    case Remove: {
      auto i = SendMessageW(control(Steps), LB_GETCURSEL, 0, 0);
      auto c = snapshotConfig();
      if (i >= 0 && std::size_t(i) < c.steps.size()) {
        c.steps.erase(c.steps.begin() + i);
        try {
          saveConfig(directory, c);
          {
            std::lock_guard lock(configMutex);
            config = std::move(c);
          }
          refreshSteps();
        } catch (...) {
          MessageBoxW(w, L"保存失败。", L"错误", MB_ICONERROR);
        }
      }
      break;
    }
    case OpenData:
      ShellExecuteW(w, L"open", directory.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      break;
    case Diagnose:
      if (running || syncConfig()) {
        try {
          auto path = writeDiagnostics(directory, snapshotConfig());
          MessageBoxW(w, (L"诊断报告已保存：\n" + path +
                         L"\n\n包含进程、窗口、启动器和校准检查；不含令牌或搜索文字。").c_str(),
                      L"诊断完成", MB_OK);
        } catch (...) {
          MessageBoxW(w, L"无法写入诊断报告，请检查配置目录权限。", L"诊断失败", MB_ICONERROR);
        }
      }
      break;
    case Browse:
    case NeacBrowse: {
      wchar_t b[32768]{};
      OPENFILENAMEW o{};
      o.lStructSize = sizeof(o);
      o.hwndOwner = w;
      o.lpstrFile = b;
      o.nMaxFile = 32768;
      o.lpstrFilter = L"程序或快捷方式\0*.exe;*.lnk\0所有文件\0*.*\0";
      o.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
      if (GetOpenFileNameW(&o))
        SetWindowTextW(control(LOWORD(wp) == Browse ? Launcher : NeacPath), b);
      break;
    }
    }
    return 0;
  case WM_HOTKEY:
    if (wp == 1)
      setRunning(false);
    return 0;
  case WM_TIMER:
    if (wp == 1)
      refreshStatus();
    else if (wp == 2)
      captureCalibration();
    return 0;
  case WM_CLOSE:
    setRunning(false);
    quitting = true;
    wake.notify_all();
    networkWake.notify_all();
    DestroyWindow(w);
    return 0;
  case WM_DESTROY:
    UnregisterHotKey(w, 1);
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(w, msg, wp, lp);
}
} // namespace
int WINAPI wWinMain(HINSTANCE h, HINSTANCE, PWSTR, int) {
  instance = h;
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  StartupOptions options;
  try {
    options = commandLineOptions();
    directory = resolveDataDirectory(options);
    config = loadConfig(directory, options.demo);
    if (options.diagnose) {
      writeDiagnostics(directory, config);
      return 0;
    }
  } catch (...) {
    // CLI diagnosis must not leave an unattended message box on failure.
    if (!options.diagnose)
      MessageBoxW(nullptr, L"无法读取启动参数或配置目录。\n支持 --demo、--data-dir 路径、--diagnose。",
                  L"OverwatchHelper", MB_ICONERROR);
    return 1;
  }
  bool demo = options.demo;
  Handle singleton(CreateMutexW(
      nullptr, TRUE, demo ? L"Local\\OverwatchHelper.Demo" : L"Local\\OverwatchHelper.Main"));
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBoxW(nullptr, L"助手已经在运行。", L"OverwatchHelper", MB_OK);
    return 0;
  }
  dpi = GetDpiForSystem();
  font = CreateFontW(-u(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH,
                     L"Microsoft YaHei UI");
  titleFont = CreateFontW(-u(22), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH,
                          L"Microsoft YaHei UI");
  WNDCLASSW wc{};
  wc.lpfnWndProc = mainProc;
  wc.hInstance = h;
  wc.lpszClassName = AppClass;
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  RegisterClassW(&wc);
  wc.lpfnWndProc = overlayProc;
  wc.lpszClassName = OverlayClass;
  RegisterClassW(&wc);
  RECT r{0, 0, u(844), u(790)};
  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRectExForDpi(&r, style, FALSE, 0, dpi);
  mainWindow = CreateWindowExW(0, AppClass,
                               demo ? L"OverwatchHelper 0.3.0 · 演示配置" : L"OverwatchHelper 0.3.0 · 国服战网",
                               style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left,
                               r.bottom - r.top, nullptr, nullptr, h, nullptr);
  if (!mainWindow)
    return 1;
  controls();
  if (!RegisterHotKey(mainWindow, 1, MOD_NOREPEAT, VK_F8))
    MessageBoxW(mainWindow, L"F8 被其他程序占用，本次请用“停止”按钮。", L"全局停止热键不可用",
                MB_ICONWARNING);
  SetTimer(mainWindow, 1, 500, nullptr);
  ShowWindow(mainWindow, SW_SHOW);
  UpdateWindow(mainWindow);
  std::thread workerThread(worker), networkThread(telemetry);
  MSG msg{};
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    if (!IsDialogMessageW(mainWindow, &msg)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
  quitting = true;
  running = false;
  input.release();
  wake.notify_all();
  networkWake.notify_all();
  workerThread.join();
  networkThread.join();
  DeleteObject(font);
  DeleteObject(titleFont);
  return 0;
}
