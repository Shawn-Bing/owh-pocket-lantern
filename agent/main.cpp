#include "agent/platform.hpp"
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
                                          L"到达目标房间 / 保持 Shift",
                                          L"登录/错误页超时恢复",
                                          L"战网启动游戏"};
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
  NetStatus
};
HINSTANCE instance{};
HWND mainWindow{};
HFONT font{}, titleFont{};
int dpi = 96;
Config config;
std::wstring directory;
std::mutex configMutex, statusMutex, networkMutex, logMutex;
std::atomic<bool> running{}, quitting{};
std::atomic<unsigned> generation{};
std::condition_variable wake, networkWake;
Input input;
Millis bootTime = GetTickCount64();
struct Status {
  std::string state = "STOPPED";
  std::wstring detail = L"先校准页面，演练确认后再开启实际动作。";
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
      << ",\"dry_run\":" << (c.dryRun ? "true" : "false") << ",\"version\":\"0.1.0\"}";
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
  RecoveryBudget budget;
  unsigned epoch = ~0u;
  Millis tickAt{}, missingSince{}, hungSince{}, unknownSince{}, restartPageSince{},
      launchDeadline{}, lastProcessScan{}, windowSince{};
  int restartPage = -1, typedPage = -1;
  Process game{}, launcher{};
  Config c;
  bool blocked{}, cycleBudgeted{}, waitingGame{}, activationPending{};
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
      missingSince = hungSince = unknownSince = restartPageSince = launchDeadline =
          lastProcessScan = windowSince = 0;
      restartPage = typedPage = -1;
      blocked = false;
      cycleBudgeted = false;
      waitingGame = false;
      activationPending = true;
      tickAt = 0;
    }
    if (now < tickAt)
      continue;
    tickAt = now + 1000;
    if (blocked) {
      input.release();
      continue;
    }
    if (now - lastProcessScan >= 2000 || lastProcessScan == 0) {
      auto discovered = findProcess(c.gameExe, c.gameClass);
      if (discovered.pid != game.pid || discovered.created != game.created) {
        if (game.pid || cycleBudgeted || launchDeadline)
          activationPending = true;
        input.release();
        gate.reset();
        hungSince = windowSince = restartPageSince = 0;
        restartPage = typedPage = -1;
      }
      game = discovered;
      launcher = c.demo ? Process{} : findProcess(L"Battle.net.exe", L"Chrome_WidgetWin_0");
      lastProcessScan = now;
    }
    auto recover = [&](const std::wstring &reason) {
      input.release();
      if (!running || epoch != generation.load())
        return;
      if (c.dryRun) {
        publish("DRY_RUN", L"演练：将恢复游戏。" + reason, game.pid);
        return;
      }
      if (!c.autoRecover) {
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
      restartPageSince = 0;
      restartPage = typedPage = -1;
      missingSince = now;
      lastProcessScan = 0;
      launchDeadline = 0;
      activationPending = true;
    };
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
      unknownSince = 0;
      if (!missingSince)
        missingSince = now;
      if (!c.autoRecover) {
        publish("GAME_NOT_RUNNING", L"未发现游戏；自动恢复已关闭。");
        continue;
      }
      if (launchDeadline && now >= launchDeadline) {
        publish("NEEDS_USER", L"启动后 120 "
                              L"秒未发现游戏；检查更新、登录或启动按钮。请停止后重新开始。");
        blocked = true;
        continue;
      }
      if (!launcher.pid) {
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
    // running do not cause repeated focus stealing; Windows may deny this request.
    if (activationPending) {
      activationPending = false;
      if (running && !foreground(target.window)) {
        ShowWindow(target.window, SW_RESTORE);
        SetForegroundWindow(target.window);
      }
    }
    if (!foreground(target.window)) {
      input.release();
      gate.observe(-1);
      restartPage = -1;
      restartPageSince = 0;
      publish("WAITING_FOCUS",
              isLauncher ? L"请把战网窗口置于前台；切换回来后自动继续。"
                         : L"已释放 Shift。游戏回到前台后自动继续。",
              target.pid);
      continue;
    }
    if (!capture.grab(target.window, frame)) {
      input.release();
      gate.observe(-1);
      restartPage = -1;
      restartPageSince = 0;
      publish("CAPTURE_FAILED", L"无法取得可见客户区；请使用窗口化并让窗口完整显示。", target.pid);
      continue;
    }
    bool hasTemplates = std::any_of(c.steps.begin(), c.steps.end(),
                                    [&](auto &s) { return s.launcher == isLauncher; });
    if (!hasTemplates) {
      input.release();
      publish("NEEDS_CALIBRATION",
              isLauncher ? L"请校准战网启动按钮（操作选择“战网启动游戏”）。"
                         : L"请先校准大厅入口、房间导航和“到达目标房间”。",
              target.pid);
      continue;
    }
    auto m = match(frame, c.steps, isLauncher, c.threshold);
    if (m.index < 0) {
      input.release();
      gate.observe(-1);
      restartPage = -1;
      restartPageSince = 0;
      if (!unknownSince)
        unknownSince = now;
      publish(m.ambiguous ? "AMBIGUOUS" : "UNKNOWN",
              m.ambiguous ? L"多个页面特征过于相似，请重新圈选更独特的区域。"
                          : (m.aspectMismatch ? L"页面未匹配；检查窗口宽高比是否与校准时一致。"
                                              : L"未匹配已校准页面，等待加载或新的校准。"),
              target.pid, m.score);
      continue;
    }
    unknownSince = 0;
    auto &s = c.steps[m.index];
    if (!running || epoch != generation.load())
      continue;
    if (!gate.observe(m.index)) {
      input.release();
      publish("CONFIRMING", L"确认页面：" + s.label, target.pid, m.score);
      continue;
    }
    gate.arrived(m.index);
    if (typedPage >= 0 && typedPage != m.index)
      typedPage = -1;
    if (s.action == Action::Ready) {
      restartPage = -1;
      restartPageSince = 0;
      bool ok = true;
      if (c.dryRun || !c.holdShift)
        input.release();
      else
        ok = input.hold(target.window);
      publish(c.dryRun ? "DRY_RUN" : (ok ? "ACTIVE" : "INPUT_BLOCKED"),
              c.dryRun ? L"演练匹配目标房间：" + s.label
                       : (ok ? (c.holdShift ? L"目标房间已确认，保持 Shift。"
                                            : L"目标房间已确认，仅监控。 ")
                             : L"Shift 输入未成功；检查手动按键或权限。"),
              target.pid, m.score);
      continue;
    }
    input.release();
    if (s.action == Action::Restart) {
      if (restartPage != m.index) {
        restartPage = m.index;
        restartPageSince = now;
      }
      Millis delay = c.demo ? 5000 : 90000;
      publish("LOGIN_STUCK", L"已识别恢复页面：" + s.label + L"；等待持续确认。", target.pid,
              m.score, restartPageSince + delay);
      if (now - restartPageSince >= delay) {
        if (isLauncher) {
          publish("NEEDS_USER", L"战网需要登录或验证，请处理后继续。", target.pid);
          continue;
        }
        recover(L"登录/错误页面持续超时。");
      }
      continue;
    }
    restartPage = -1;
    restartPageSince = 0;
    if (c.dryRun) {
      publish("DRY_RUN", L"演练：" + s.label + L" → " + ActionNames[int(s.action)], target.pid,
              m.score);
      continue;
    }
    if (!gate.cooldownReady(now)) {
      publish("NAVIGATING", L"等待页面变化：" + s.label, target.pid, m.score);
      continue;
    }
    if (gate.exhausted(m.index) || (s.action == Action::Text && typedPage == m.index)) {
      publish("NEEDS_USER",
              L"操作后页面没有变化：" + s.label + L"。为避免重复输入已暂停；停止后重新开始。",
              target.pid, m.score);
      blocked = true;
      continue;
    }
    if (!gate.canAct(m.index, now)) {
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
    switch (s.action) {
    case Action::Click:
    case Action::Launch:
      ok = input.click(target.window, s.click);
      break;
    case Action::Enter:
      ok = input.press(target.window, VK_RETURN);
      break;
    case Action::Escape:
      ok = input.press(target.window, VK_ESCAPE);
      break;
    case Action::Text:
      ok = input.click(target.window, s.click) && input.text(target.window, s.text) &&
           input.press(target.window, VK_RETURN);
      typedPage = m.index;
      break;
    default:
      break;
    }
    if (!ok) {
      publish("INPUT_BLOCKED", L"输入未成功或焦点改变，请检查窗口权限。", target.pid);
      continue;
    }
    gate.acted(m.index, now);
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
  for (auto &s : c.steps) {
    auto line =
        (s.launcher ? L"[战网] " : L"[游戏] ") + s.label + L"  →  " + ActionNames[int(s.action)];
    SendMessageW(control(Steps), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
  }
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
void setRunning(bool on) {
  if (on && !syncConfig())
    return;
  if (on) {
    ++generation;
    input.enable(!snapshotConfig().dryRun);
    running = true;
  } else {
    input.enable(false);
    running = false;
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
    calibration.frame.pixels.clear();
    ShowWindow(mainWindow, SW_RESTORE);
    SetForegroundWindow(mainWindow);
    refreshSteps();
    return 0;
  }
  return DefWindowProcW(w, msg, wp, lp);
}
void beginCalibration() {
  if (running || !syncConfig())
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
  ShowWindow(mainWindow, SW_MINIMIZE);
  ShowWindow(p.window, SW_RESTORE);
  SetForegroundWindow(p.window);
  SetTimer(mainWindow, 2, 3000, nullptr);
}
void captureCalibration() {
  KillTimer(mainWindow, 2);
  Capture cap;
  if (!cap.grab(calibration.target, calibration.frame)) {
    ShowWindow(mainWindow, SW_RESTORE);
    MessageBoxW(mainWindow, L"校准时目标必须位于前台并完整可见。请重新尝试。", L"截图失败",
                MB_ICONWARNING);
    return;
  }
  calibration.step.aspect = double(calibration.frame.width) / calibration.frame.height;
  POINT p{};
  ClientToScreen(calibration.target, &p);
  HWND w = CreateWindowExW(WS_EX_TOPMOST, OverlayClass, L"校准", WS_POPUP, p.x, p.y,
                           calibration.frame.width, calibration.frame.height, mainWindow, nullptr,
                           instance, nullptr);
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
  add(L"BUTTON", L"目标房间保持 Shift", Hold, 22, 329, 224, 26, BS_AUTOCHECKBOX);
  add(L"BUTTON", L"自动恢复（每小时最多 3 次）", Auto, 264, 329, 350, 26, BS_AUTOCHECKBOX);
  SendMessageW(control(Hold), BM_SETCHECK, c.holdShift ? BST_CHECKED : BST_UNCHECKED, 0);
  SendMessageW(control(Auto), BM_SETCHECK, c.autoRecover ? BST_CHECKED : BST_UNCHECKED, 0);
  add(L"STATIC", L"页面校准 · 手动打开一个页面，选择操作，录制特征与点击位置", 0, 22, 369, 800, 24);
  add(L"COMBOBOX", L"", Scope, 22, 402, 92, 140, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"游戏", L"战网"})
    SendMessageW(control(Scope), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(Scope), CB_SETCURSEL, 0, 0);
  add(L"COMBOBOX", L"", ActionBox, 126, 402, 260, 230, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : ActionNames)
    SendMessageW(control(ActionBox), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ActionBox), CB_SETCURSEL, 0, 0);
  add(L"EDIT", L"页面名称", Label, 399, 402, 220, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"3 秒后录制", Record, 632, 402, 186, 28);
  add(L"STATIC", L"输入文字内容", 0, 22, 440, 115, 24);
  add(L"EDIT", L"", TextArg, 144, 436, 480, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"删除选中页面", Remove, 637, 436, 181, 28);
  add(L"LISTBOX", L"", Steps, 22, 478, 796, 122, WS_BORDER | WS_VSCROLL | LBS_NOTIFY | WS_TABSTOP);
  add(L"STATIC", L"", Stats, 22, 613, 790, 22);
  add(L"STATIC", L"", NetStatus, 22, 643, 640, 22);
  add(L"BUTTON", L"配置与日志", OpenData, 682, 637, 136, 30);
  add(L"STATIC",
      L"提示：小窗口需保持原宽高比；切走窗口自动释放按键。关闭本窗口会退出助手"
      L"。",
      0, 22, 677, 800, 26);
  settingsIds = {Save,    Browse, Record, Remove, GameExe, Launcher,  Args,  Server, Token,
                 Machine, Dry,    Hold,   Auto,   Scope,   ActionBox, Label, TextArg};
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
  std::wostringstream stats;
  stats << L"PID " << s.pid << L"    匹配 " << std::fixed << std::setprecision(1) << s.score * 100
        << L"%    恢复次数 " << s.restarts << L"    运行 " << (GetTickCount64() - bootTime) / 1000
        << L" 秒";
  if (s.next > GetTickCount64())
    stats << L"    等待 " << (s.next - GetTickCount64()) / 1000 << L" 秒";
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
    case Browse: {
      wchar_t b[32768]{};
      OPENFILENAMEW o{};
      o.lStructSize = sizeof(o);
      o.hwndOwner = w;
      o.lpstrFile = b;
      o.nMaxFile = 32768;
      o.lpstrFilter = L"程序或快捷方式\0*.exe;*.lnk\0所有文件\0*.*\0";
      o.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
      if (GetOpenFileNameW(&o))
        SetWindowTextW(control(Launcher), b);
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
  bool demo = std::wstring(GetCommandLineW()).find(L"--demo") != std::wstring::npos;
  Handle singleton(CreateMutexW(
      nullptr, TRUE, demo ? L"Local\\OverwatchHelper.Demo" : L"Local\\OverwatchHelper.Main"));
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBoxW(nullptr, L"助手已经在运行。", L"OverwatchHelper", MB_OK);
    return 0;
  }
  try {
    directory = dataDirectory(demo);
    config = loadConfig(directory, demo);
  } catch (...) {
    MessageBoxW(nullptr, L"无法读取配置目录。", L"OverwatchHelper", MB_ICONERROR);
    return 1;
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
  RECT r{0, 0, u(844), u(718)};
  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRectExForDpi(&r, style, FALSE, 0, dpi);
  mainWindow = CreateWindowExW(0, AppClass,
                               demo ? L"OverwatchHelper · 演示配置" : L"OverwatchHelper · 国服战网",
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
