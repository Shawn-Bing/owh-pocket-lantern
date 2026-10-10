#include "agent/startup.hpp"
#include "agent/vision.hpp"
#include "agent/learning.hpp"
#include "agent/background_input.hpp"
#include "agent/match_observer.hpp"
#include "agent/simple_config.hpp"
#include "agent/failure_log.hpp"
#include "agent/window_layout.hpp"
#include "core/cn_recovery.hpp"
#include "core/automation_state.hpp"
#include <commctrl.h>
#include <condition_variable>
#include <iomanip>
#include <thread>

using namespace owh;
namespace {
#ifdef OWH_SIMPLE_EDITION
constexpr bool SimpleEdition=true;
#else
constexpr bool SimpleEdition=false;
#endif
constexpr UINT StartSimpleBackground=WM_APP+3;
constexpr wchar_t AppClass[] = L"OverwatchHelper.Main";
constexpr wchar_t OverlayClass[] = L"OverwatchHelper.Calibrate";
constexpr const wchar_t *ActionNames[] = {L"点击按钮",
                                          L"按 Enter",
                                          L"按 Esc",
                                          L"输入文字并 Enter",
                                          L"到达目标房间 / 持续 Shift",
                                          L"登录/错误页重启游戏",
                                          L"战网启动游戏",
                                          L"仅观察（结束房间会话）",
                                          L"等待（不发送输入）",
                                          L"死斗局内 / 随机按键",
                                          L"按 Space（重生）",
                                          L"按 N（赞赏）",
                                          L"双击进入"};
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
  Standby,
  StandbySeconds,
  NeacPath,
  NeacBrowse,
  CnPreset,
  RestartNow,
  ModeBox,
  Semantic,
  RoomTarget,
  StartInRoom,
  Learning,
  ImportProfile,
  ForegroundUse,
  FailureList,
  OpenFailures,
  FailureHeading,
  ModeHeading
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
std::atomic<DWORD> roomStartPid{};
std::atomic<ULONGLONG> roomStartCreated{};
std::condition_variable wake, networkWake;
Input input;
FailureLog failures;
std::atomic<bool> simpleBackground{}, simpleBackgroundArmed{};
BackgroundInput backgroundInput;
std::atomic<bool> foregroundUse{};
std::mutex backgroundMutex;
Process backgroundCandidate{};
Millis backgroundCandidateAt{};
void offerBackground(const Process &game,Millis now) {
  std::lock_guard lock(backgroundMutex);
  backgroundCandidate=game;backgroundCandidateAt=now;
}
Process backgroundOffer() {
  std::lock_guard lock(backgroundMutex);
  const auto now=GetTickCount64();
  return running && backgroundCandidateAt && now>=backgroundCandidateAt && now-backgroundCandidateAt<=10000
      ? backgroundCandidate : Process{};
}
Millis bootTime = GetTickCount64();
MatchObserver matchObserver{std::to_string(GetCurrentProcessId()) + "-" +
    std::to_string(processCreation(GetCurrentProcess()))};
// UI publication uses the current ticket; worker publications stay attached to
// the ticket captured before any blocking recognition call.
thread_local MatchTicket observationTicket;
thread_local bool workerPublication{};
struct Status {
  std::string state = "STOPPED";
  std::wstring detail = L"本地 OCR 自动识别与校准；支持自定义房间和自由死斗。先演练确认，再开启实际动作。";
  DWORD pid{};
  double score{};
  unsigned restarts{};
  Millis next{};
  int http{};
};
Status status;
bool networkDirty = true;
std::vector<int> settingsIds;
ui::WindowLayout windowLayout;
HWND control(int id) { auto window=windowLayout.find(id);return window ? window : GetDlgItem(mainWindow,id); }
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
  if (score.accepted)
    return out + L"旧截图匹配，但当前模式未采用此导航；本地 OCR 尚未确认入口。";
  bool named = false;
  for (std::size_t i = 0; i < score.anchors.size() && i < rule.anchors.size(); ++i)
    if (score.anchors[i] < rule.anchors[i].minimum) {
      out += L"锚点" + std::to_wstring(i) + L" " + fixed3(score.anchors[i]) + L" < " +
             fixed3(rule.anchors[i].minimum);
      named = true;
      break;
    }
  if (!named)
    out += L"页面证据未通过完整匹配";
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
// Durations are shown the way a person reads them: whole minutes above a minute,
// seconds below. Getting this from the setting rather than hardcoding it matters,
// because the message is the only place the user sees which delay is in force.
std::wstring durationText(int seconds) {
  if (seconds >= 60 && seconds % 60 == 0)
    return std::to_wstring(seconds / 60) + L" 分钟";
  return std::to_wstring(seconds) + L" 秒";
}
// The recovery counter is the number the user reads off the status bar, so every
// increment is logged with its reason. Two of the three sites used to bump it
// silently, which made "恢复了几次" unanswerable from the log afterwards.
void countRecovery(const std::wstring &what) {
  unsigned n;
  {
    std::lock_guard lock(statusMutex);
    n = ++status.restarts;
  }
  log(L"恢复计数 " + std::to_wstring(n) + L"：" + what);
}
void publish(AutomationState value, const std::wstring &detail, DWORD pid = 0, double score = 0,
             Millis next = 0) {
  const auto *state = automationStateName(value);
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
  matchObserver.automation(workerPublication ? observationTicket : matchObserver.currentTicket(), value);
  if (changed) {
    log(wide(state) + L" | " + detail);
    failures.record(state,detail,pid);
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
  workerPublication = true;
  Capture capture;
  Frame frame;
  StepGate gate;
  RoomSession room;
  DeathmatchSession deathmatch{unsigned(GetTickCount64())};
  AdaptiveVision vision;
  // Login recovery needs a short, confirmed observation. Keep recent evidence
  // across player standby; returning still requires current page confirmation.
  ContinuousEvidence disconnect{5 * 60 * 1000};
  CNJoinGate cnJoin;
  CNRoomStart explicitRoom;
  std::uint64_t seenPlayerActivity{};
  RecoveryBudget budget;
  unsigned epoch = ~0u;
  Millis tickAt{}, missingSince{}, hungSince{}, unknownSince{}, desktopPaused{},
      launchDeadline{}, lastProcessScan{}, windowSince{}, activationDeadline{}, standbySince{},
      lastUnmatchedSave{};
  int typedPage = -1;
  Process game{}, launcher{};
  Config c;
  bool blocked{}, cycleBudgeted{}, waitingGame{}, activationPending{}, cleanupChecked{}, semanticBurst{};
  Millis neacRetrySince{};
  bool launchFailurePaused{};
  NeacLaunchMonitor launchMonitor;
  GameExitWait exitWait;
  Process pendingGameExit;
  bool launcherCoverRetried{};
  // Whether this run has ever seen the game process. It is what separates a
  // cold start -- 开始 pressed with nothing running -- from a client that
  // vanished mid-session and may be restarting itself. The two need very
  // different patience, and the old code gave both the long one.
  bool sawGame{};
  int unmatchedSaved{};
  Millis launcherCoverSince{}, backgroundNext{}, backgroundEnteredAt{};
  bool wasForegroundUse{}, backgroundOkay{true};
  std::uint64_t observationFrameSequence{};
  for (;;) {
    input.checkFocus();
    {
      std::unique_lock lock(networkMutex);
      wake.wait_for(lock, std::chrono::milliseconds(100), [] { return quitting.load(); });
    }
    if (quitting) {
      backgroundInput.stop();
      input.release();
      return;
    }
    if (!running) {
      backgroundInput.stop();
      input.release();
      vision.cancel();
      continue;
    }
    auto now = GetTickCount64();
    if (epoch != generation.load()) {
      epoch = generation.load();
      c = snapshotConfig();
      gate.reset();
      room.reset();
      deathmatch.reset();
      vision.reset(directory);
      disconnect.reset();
      cnJoin.reset();
      semanticBurst = false;
      explicitRoom.begin(roomStartPid.load(),roomStartCreated.load(),now);
      seenPlayerActivity = input.playerActivityRevision();
      missingSince = hungSince = unknownSince = desktopPaused = launchDeadline =
          lastProcessScan = windowSince = activationDeadline = standbySince = 0;
      typedPage = -1;
      blocked = false;
      cycleBudgeted = false;
      waitingGame = false;
      activationPending = true;
      cleanupChecked = false;
      neacRetrySince = 0;
      sawGame = false;
      launchMonitor.reset();exitWait.reset();pendingGameExit={};launchFailurePaused=false;
      tickAt = 0;
      launcherCoverRetried=false;
      launcherCoverSince=0;wasForegroundUse=false;backgroundNext=0;
      log(L"开始运行 OverwatchHelper "+wide(Version)+L"，助手完整性级别："+integrityName(selfIntegrity()));
    }
    if (now < tickAt)
      continue;
    // A join burst needs five clicks a second, and this loop is what paces them:
    // one iteration is one click. Outside a burst the cycle stays at one second,
    // because it is a full capture and recognition and running it faster only
    // re-reads a screen that has not changed. Inside one it drops to the burst
    // interval, which also means the page is still recognised on every click --
    // that is how the burst notices it has been left and stops immediately.
    tickAt = now + (cnJoin.bursting(now) ? CNJoinGate::BurstInterval : 1000);
    observationTicket = matchObserver.ticketFor(epoch);
    MatchObservationTick observationTick{matchObserver,observationTicket,game,foregroundUse};
    if (!interactiveDesktop()) {
      backgroundInput.release();
      input.release();
      gate.observe(-1);
      if (!desktopPaused) desktopPaused = now;
      publish(AutomationState::WAITING_DESKTOP, L"等待返回普通桌面；如有 NeacProtect 管理员授权，请手动处理。",
              game.pid);
      continue;
    }
    if (desktopPaused) {
      if (launchDeadline) launchDeadline += now - desktopPaused;
      exitWait.pause(now-desktopPaused);
      desktopPaused = hungSince = windowSince = 0;
      lastProcessScan = 0;
    }
    bool requestedRestart=restartRequested.load()==epoch;
    if (requestedRestart) {blocked=false;launchFailurePaused=false;}
    if (blocked) {
      input.release();
      // After a failed authorization the user may start the game themselves.
      // Observe that success without clicking Play or consuming another retry.
      if (launchFailurePaused && findProcess(c.gameExe,c.gameClass).created) {
        launchFailurePaused=blocked=false;lastProcessScan=0;activationPending=true;
        log(L"检测到游戏已由用户启动，结束启动失败暂停并继续监控。");
      } else continue;
    }
    // Explicit foreground-use mode bypasses capture, OCR, navigation and recovery.
    // Its only input backend posts to the already-confirmed process/window.
    if (foregroundUse.load()) {
      const auto bound=backgroundInput.target();
      if (!wasForegroundUse) {
        wasForegroundUse=true;backgroundNext=0;backgroundEnteredAt=now;backgroundOkay=true;vision.cancel();input.release();cnJoin.reset();semanticBurst=false;gate.reset();
        log(L"前台使用模式开启：暂停 OCR/导航/恢复，定向窗口消息；PID "+std::to_wstring(bound.pid)+
            L"，按键效果尚未验证。");
      }
      activationPending=false;activationDeadline=0;standbySince=0;
      if (!sameProcess(bound,c.gameExe) || !IsWindow(bound.window)) {
        backgroundInput.stop();foregroundUse=false;input.blockForeground(false);
        blocked=true;
        publish(AutomationState::NEEDS_USER,L"后台目标进程或窗口已变化，已停止后台按键；请重新确认游戏后开始。",bound.pid);
        continue;
      }
      matchObserver.selectProcess(observationTicket,matchProcess(bound),now);
      observationTick.target(bound);
      if (foreground(bound.window) && now-backgroundEnteredAt<1000) {
        publish(AutomationState::WAITING_FOCUS,L"正在开启前台使用，等待游戏最小化。",bound.pid);continue;
      }
      if (foreground(bound.window)) {
        backgroundInput.stop();foregroundUse=false;input.blockForeground(false);
      } else {
        if (!c.dryRun && c.holdShift && now>=backgroundNext) {
          if (c.mode==WorkflowMode::CustomRoom) {
            backgroundOkay=backgroundInput.tap(VK_LSHIFT,20,epoch);backgroundNext=now+1000;
          } else {
            auto stroke=deathmatch.next(now);
            backgroundOkay=backgroundInput.tap(stroke.key,stroke.duration,epoch);
            if (backgroundOkay) backgroundOkay=backgroundInput.tap(stroke.rightClick ? VK_RBUTTON : VK_LBUTTON,20,epoch,true);
            backgroundNext=now+stroke.duration+20+1000;
          }
        }
        publish(c.dryRun ? AutomationState::DRY_RUN : (!backgroundOkay ? AutomationState::BACKGROUND_INPUT_BLOCKED : AutomationState::BACKGROUND_UNVERIFIED),
          c.dryRun ? L"前台使用演练：已暂停 OCR，不发送后台按键。"
          : !backgroundOkay ? L"后台消息发送失败，Windows 错误 "+std::to_wstring(backgroundInput.error())+L"；切回游戏检查。"
          : !c.holdShift ? L"前台使用：已暂停 OCR 和按键，游戏可最小化；切回游戏后恢复检测。"
          : L"前台使用：定向后台按键消息，游戏可最小化；OCR/导航/恢复已暂停，游戏内是否生效待验证。切回游戏恢复检测。",bound.pid);
        continue;
      }
    }
    if (wasForegroundUse) {
      observationTick.useScannedWindow();
      wasForegroundUse=false;backgroundInput.stop();vision.cancel();
      room.reset();deathmatch.reset();gate.reset();cnJoin.reset();disconnect.reset();
      explicitRoom.begin(game.pid,game.created,now);
      lastProcessScan=0;unknownSince=0;activationPending=false;activationDeadline=0;
      input.clearPlayerHold();
      {std::lock_guard lock(backgroundMutex);backgroundCandidate={};backgroundCandidateAt=0;}
      log(L"前台使用模式结束：重新确认当前画面后恢复 OCR 和自动操作。");
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
    const bool playerBusy = input.playerBusy();
    const auto playerActivity = input.playerActivityRevision();
    if (playerActivity != seenPlayerActivity) {
      seenPlayerActivity = playerActivity;
      deathmatch.cancelEndorsementClose();
      if (playerActivity && foreground(game.window) && cnJoin.awaitingRoom(now)) {
        // Manual navigation may have replaced the pending automatic join.
        cnJoin.reset(); semanticBurst = false;
      }
    }
    if (c.standbyOnInput && playerBusy && !foreground(game.window) && !requestedRestart) {
      input.release();
      gate.observe(-1);
      if (!standbySince)
        standbySince = now;
      publish(AutomationState::STANDBY,
              L"你正在使用电脑，助手待机中；不发按键、不检测，停手约 " +
                  durationText(c.standbySeconds) + L"后自动恢复并切回游戏（" +
                  input.activityText() + L"）。",
              game.pid);
      continue;
    }
    if (standbySince) {
      // Deadlines must not expire while nobody was watching.
      if (launchDeadline)
        launchDeadline += now - standbySince;
      exitWait.pause(now-standbySince);
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
      publish(AutomationState::NEEDS_USER,
              L"游戏以管理员权限运行，助手是普通权限，Windows 会丢弃助手发送的全部按键和点击。"
              L"请关闭助手，以管理员身份重新运行后再开始。",
              game.pid);
      blocked = true;
      continue;
    }
    if (now - lastProcessScan >= 2000 || lastProcessScan == 0) {
      auto discovered = findProcess(c.gameExe, c.gameClass);
      auto discoveredLauncher =
          c.demo ? Process{} : findLauncherProcess();
      if (discovered.pid != game.pid || discovered.created != game.created) {
        if (!explicitRoom.trusted(discovered.pid,discovered.created,now)) explicitRoom.reset();
        if (game.pid || cycleBudgeted || launchDeadline)
          activationPending = true;
        input.release();
        gate.reset();
        room.reset();
        deathmatch.reset();
        vision.cancel();
        disconnect.reset();
        cnJoin.reset();
        hungSince = windowSince = 0;
        typedPage = -1;
        cleanupChecked = false;
        neacRetrySince = 0;
      }
      // Battle.net can appear on its own -- opened by the recovery path, or
      // restored from the tray once the game exits. When it does, nothing above
      // asks for the foreground, because that check only ever watched the game.
      // A click on its Play button is a foreground-only event, so without this
      // the helper sat publishing "请把战网窗口置于前台" and waited for a human to
      // alt-tab, which is the opposite of what it is for.
      if (discoveredLauncher.pid && discoveredLauncher.window &&
          (discoveredLauncher.pid != launcher.pid || discoveredLauncher.created != launcher.created ||
           discoveredLauncher.window != launcher.window))
        activationPending = true;
      game = discovered;
      matchObserver.selectProcess(observationTicket,matchProcess(game),now);
      launcher = discoveredLauncher;
      lastProcessScan = now;
    }
    auto finishGameExit = [&] {
      game={};gate.reset();disconnect.reset();typedPage=-1;
      matchObserver.selectProcess(observationTicket,{},GetTickCount64());
      missingSince=GetTickCount64();lastProcessScan=0;launchDeadline=0;
      activationPending=true;waitingGame=cleanupChecked=false;neacRetrySince=0;
      launcherCoverRetried=false;launcherCoverSince=0;
      exitWait.reset();pendingGameExit={};launchMonitor.reset();launchFailurePaused=false;
    };
    auto recover = [&](const std::wstring &reason, bool manual = false) {
      input.release();
      room.reset();
      deathmatch.reset();
      vision.cancel();
      cnJoin.reset();
      if (!running || foregroundUse || epoch != generation.load()) return;
      if (exitWait.active()) {
        publish(AutomationState::WAITING_GAME_EXIT,L"游戏终止请求已接受，仍在等待进程退出；未重复终止或启动。",pendingGameExit.pid);
        return;
      }
      std::error_code neacError,launcherError;
      if (!c.demo && ((!c.cnPreset && !c.semanticRecognition && std::none_of(c.steps.begin(),c.steps.end(),[](const Step &s){return s.launcher && s.action==Action::Launch;})) ||
          !validNeacPath(c.neacPath) || !std::filesystem::is_regular_file(c.neacPath,neacError) ||
          (!launcher.pid && !std::filesystem::is_regular_file(c.launcherPath,launcherError)))) {
        publish(AutomationState::NEEDS_CONFIG, SimpleEdition ? L"无法自动核验游戏或战网安装路径。请先从战网启动一次游戏后重试，或在开发版保存正确路径。游戏尚未关闭。" : L"重启前请填写 OWNeacClient.exe 完整路径和启动器，并启用本地 OCR、国服预设或录制战网启动按钮。游戏尚未关闭。",game.pid);
        blocked=true;
        return;
      }
      if (c.dryRun) {
        publish(AutomationState::DRY_RUN, L"演练：将恢复游戏。" + reason, game.pid);
        if (manual) blocked=true;
        return;
      }
      if (!c.autoRecover && !manual) {
        publish(AutomationState::NEEDS_USER, L"自动恢复已关闭。" + reason, game.pid);
        return;
      }
      if (!budget.available(now)) {
        publish(AutomationState::BACKOFF, L"恢复冷却中；次数不限，到期继续恢复。", game.pid, 0, budget.retryAt());
        return;
      }
      if (!budget.take(now))
        return;
      cycleBudgeted = true;
      countRecovery(L"游戏缺失或卡死，正在执行恢复。" + reason);
      publish(AutomationState::RECOVERING, reason, game.pid);
      if (game.pid) {
        const auto ended=terminateGameDetailed(game,c.gameExe,running,generation,epoch);
        if (ended.result==GameTermination::Cancelled) return;
        if (ended.result==GameTermination::Pending) {
          pendingGameExit=game;exitWait.begin(GetTickCount64());
          log(L"游戏退出请求已接受但尚未完成：PID "+std::to_wstring(game.pid)+L"，继续等待，不启动战网游戏。");
          publish(AutomationState::WAITING_GAME_EXIT,L"游戏终止请求已接受，等待进程实际退出；尚未清理反作弊或重新启动。",game.pid);
          return;
        }
        if (ended.result==GameTermination::IdentityChanged) {lastProcessScan=0;return;}
        if (ended.result==GameTermination::Denied) {
          publish(AutomationState::NEEDS_USER,L"游戏退出失败：PID "+std::to_wstring(game.pid)+L"，阶段「"+ended.phase+
                  L"」，Windows 错误 "+std::to_wstring(ended.error)+L"；游戏仍未确认退出，未清理或重新启动。",game.pid);
          blocked=true;return;
        }
        log(L"游戏进程退出已确认：PID "+std::to_wstring(game.pid));
      }
      finishGameExit();
    };
    auto continueRoom = [&](double score, bool background) {
      if (c.mode != WorkflowMode::CustomRoom || !room.active() || !running || foregroundUse || epoch != generation.load()) return;
      if (background && c.holdShift && !c.pulseShift) {
        input.release();
        publish(AutomationState::WAITING_FOCUS, L"持续按住模式只在前台工作；游戏回到前台后继续。", game.pid, score);
        return;
      }
      // The pulse is a global event addressed to whatever window has focus, so
      // while the player is working it reaches their application instead of the
      // game. Standby already covers that case before this point; this is the
      // fallback for the transition second, where standby has not yet engaged.
      if (c.standbyOnInput && c.pulseShift && input.playerBusy()) {
        input.release();
        publish(AutomationState::STANDBY,
                L"你正在使用电脑，助手待机中（" + input.activityText() + L"）。", game.pid, score);
        return;
      }
      bool ok = true;
      if (c.dryRun || !c.holdShift) input.release();
      else if (!sameProcess(game, c.gameExe)) {
        input.release();
        room.reset();
        lastProcessScan = 0;
        publish(AutomationState::INPUT_BLOCKED, L"无法核对游戏进程身份，已暂停按键。", game.pid);
        return;
      } else if (c.pulseShift) ok = input.pulse(game.window, epoch);
      else ok = input.hold(game.window, epoch);
      publish(c.dryRun ? AutomationState::DRY_RUN : (!ok ? AutomationState::INPUT_BLOCKED : (background ? AutomationState::BACKGROUND_INPUT : AutomationState::ACTIVE)),
              c.dryRun ? (c.holdShift ? L"演练：房间会话已确认，将持续发送 Shift。"
                                     : L"演练：房间会话已确认，仅监控。")
                       : (!ok ? L"Shift 本次未发送；检查手动按键、游戏身份或权限。"
                              : (!c.holdShift ? L"房间会话已确认，仅监控。"
                                 : (background ? L"游戏未在前台，Shift 未发送。"
                                    : (c.pulseShift ? L"房间会话已确认，每秒请求点按左 Shift（20 毫秒）。"
                                                    : L"房间会话已确认，保持左 Shift。")))),
              game.pid, score);
    };
    if (exitWait.active()) {
      if (requestedRestart) {unsigned requestEpoch=epoch;restartRequested.compare_exchange_strong(requestEpoch,~0u);}
      const auto state=exitWait.observe(gameExitStillPending(pendingGameExit),now);
      if (state==GameExitWait::State::Exited) {
        log(L"延迟的游戏退出已确认：PID "+std::to_wstring(pendingGameExit.pid)+L"，继续恢复。");
        finishGameExit();continue;
      }
      if (state==GameExitWait::State::Expired) {
        publish(AutomationState::NEEDS_USER,L"游戏终止请求已接受，但终止后等待 120 秒仍未确认退出（PID "+
                std::to_wstring(pendingGameExit.pid)+L"）；未重新启动，处理后点重启游戏。",pendingGameExit.pid);
        exitWait.reset();pendingGameExit={};blocked=true;continue;
      }
      publish(AutomationState::WAITING_GAME_EXIT,L"游戏退出尚未完成；继续核验进程，未重复发送终止请求或启动游戏。",pendingGameExit.pid);
      continue;
    }
    if (requestedRestart) {
      unsigned requestEpoch=epoch;
      if (restartRequested.compare_exchange_strong(requestEpoch,~0u)) {
          recover(L"手动请求：关闭当前游戏、核验并清理加载器和客户端残留，再通过战网启动。",true);
      }
      continue;
    }
    if (game.pid) {
      sawGame = true;
      missingSince = 0;
      launchDeadline = 0;
      waitingGame = false;
      launchMonitor.reset();
      cycleBudgeted = false;
      if (!game.window) {
        input.release();
        if (!windowSince)
          windowSince = now;
        publish(AutomationState::STARTING, L"进程已存在，等待游戏窗口。", game.pid);
        if (now - windowSince > 120000)
          recover(L"游戏进程存在，但 120 秒未出现主窗口。");
        continue;
      }
      windowSince = 0;
      if (!responsive(game.window)) {
        input.release();
        if (!hungSince)
          hungSince = now;
        publish(AutomationState::WINDOW_HUNG, L"窗口暂未响应，连续确认中。", game.pid);
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
        publish(AutomationState::GAME_NOT_RUNNING, L"未发现游戏；自动恢复已关闭。");
        continue;
      }
      if (waitingGame && !c.demo) {
        if (const auto failure=launchMonitor.failure()) {
          if (!neacLaunchRequiresUser(failure->code) &&
              c.autoRecover && !c.dryRun) {
            if (!budget.available(now)) {
              publish(AutomationState::BACKOFF,L"反作弊启动失败（错误 "+std::to_wstring(failure->code)+L"），冷却后继续恢复，次数不限。",
                      failure->loaderPid,0,budget.retryAt());continue;
            }
            recover(L"反作弊启动失败（错误 "+std::to_wstring(failure->code)+L"）；核验残留后继续恢复，次数不限。");
            continue;
          }
          // Re-clicking cannot grant denied or cancelled authorization.
          publish(AutomationState::NEEDS_USER,neacLaunchFailureText(*failure),failure->loaderPid);
          log(L"已从当前启动的 NeacLoader.log 确认失败；停止本轮等待和自动重试。");
          waitingGame=false;launchDeadline=0;launchMonitor.reset();blocked=launchFailurePaused=true;continue;
        }
      }
      if (launchDeadline && now>=launchDeadline) {
        if (c.autoRecover && !c.dryRun) {
          if (!budget.available(now)) {
            publish(AutomationState::BACKOFF,L"游戏启动超时，等待恢复冷却后清理反作弊残留并重试。",0,0,budget.retryAt());
            continue;
          }
          recover(L"启动后 120 秒未发现游戏：重新核验 NeacLoader / OWNeacClient 残留并继续重试（次数不限）。");
          continue;
        }
        publish(AutomationState::NEEDS_USER,L"游戏启动超时；自动恢复未开启或当前为演练，请检查战网登录、更新或授权。");
        blocked=true;continue;
      }
      if (!waitingGame && !cleanupChecked && !c.demo) {
        // The long grace protects a client that is restarting itself: killing
        // OWNeacClient.exe underneath a game that is mid-launch is what turns a
        // restart into a launch failure, so that case is worth waiting out.
        // A cold start has nothing to protect -- the user pressed 开始 with no
        // game running -- and the same wait only delays the launch they just
        // asked for, measured at 73 seconds from 开始 to Battle.net appearing.
        // The short grace is still kept there: it covers the other reading of
        // "no game yet", where they double-clicked the game a moment before
        // pressing 开始 and it has not reached the process list.
        const Millis grace = missingGameGrace(sawGame, cycleBudgeted);
        if (now - missingSince < grace) {
          // Say what is actually happening. The old text claimed the helper was
          // waiting for a process to exit or finish launching, which reads like
          // work in progress while it was in fact doing nothing at all.
          publish(AutomationState::STARTING,
                  sawGame ? L"未发现游戏，等待进程退出或启动完成，再核对残留进程。"
                          : L"未发现游戏，正在启动战网…",
                  0, 0, missingSince + grace);
          continue;
        }
        std::wstring neacWhy;
        auto result = cleanupNeac(c, running, generation, epoch, &neacWhy);
        if (result == CleanupResult::Cancelled) continue;
        if (result == CleanupResult::GamePresent) { lastProcessScan = 0; continue; }
        if (result == CleanupResult::NeedsPath) {
          publish(AutomationState::NEEDS_CONFIG, L"检测到 NeacLoader / OWNeacClient 残留，请停止后填写 OWNeacClient.exe 完整路径。");
          blocked = true;
          continue;
        }
        if (result!=CleanupResult::Clear) {
          if (!neacRetrySince) neacRetrySince=GetTickCount64();
          const auto checkedAt=GetTickCount64();
          if (checkedAt-neacRetrySince<15000) {
            publish(AutomationState::NEAC_RETRY,L"反作弊加载器/客户端残留尚未清理，暂不启动游戏："+neacWhy,
                    0,0,neacRetrySince+15000);
            continue;
          }
          publish(AutomationState::NEEDS_USER,L"无法清理已核验的反作弊残留，未启动游戏："+neacWhy+
                  L"。处理后可点击重启游戏再次尝试。");
          blocked=true;continue;
        }
        cleanupChecked = true;
        neacRetrySince = 0;
        log(c.dryRun ? L"演练：残留进程检查通过，未结束任何进程。"
                     : L"游戏已退出，NeacLoader.exe 与 OWNeacClient.exe 残留清理检查通过。");
      }
      // Reuse an existing Battle.net process even when its window is hidden or
      // not created yet. Only a genuinely absent launcher may be started.
      if (!launcher.pid) {
        if (!c.demo) {
          launcher=findLauncherProcess();
          if (launcher.pid) { activationPending=true; lastProcessScan=0; continue; }
        }
        if (launchDeadline) {
          publish(AutomationState::STARTING, L"等待启动器或游戏出现。", 0, 0, launchDeadline);
          continue;
        }
        if (c.launcherPath.empty() || !std::filesystem::exists(c.launcherPath)) {
          publish(AutomationState::NEEDS_CONFIG, L"未找到启动器，请选择战网 EXE 或你平时使用的快捷方式。");
          continue;
        }
        if (c.dryRun) {
          publish(AutomationState::DRY_RUN, L"演练：游戏未运行，将打开配置的启动器。");
          continue;
        }
        if (!cycleBudgeted) {
          if (!budget.available(now)) {
            publish(AutomationState::BACKOFF, L"启动冷却中；次数不限，到期继续恢复。", 0, 0, budget.retryAt());
            continue;
          }
          if (!budget.take(now))
            continue;
          cycleBudgeted = true;
          countRecovery(L"检测到游戏缺失，正在拉起启动器。");
        }
        if (!running || foregroundUse || epoch != generation.load())
          continue;
        const auto launched=launch(c);
        if (launched==LauncherStartResult::Failed) {
          publish(AutomationState::NEEDS_USER, L"启动器启动失败，请检查路径和权限。");
          blocked = true;
          continue;
        }
        if (launched==LauncherStartResult::AlreadyRunning) {
          activationPending=true; activationDeadline=0; lastProcessScan=0;
          publish(AutomationState::STARTING,L"检测到战网已运行，正在恢复原窗口，不重复启动。");
          continue;
        }
        waitingGame = c.demo;
        launchDeadline = now + 120000;
        publish(AutomationState::STARTING, L"已启动，等待窗口。", 0, 0, launchDeadline);
        continue;
      }
    }
    bool isLauncher = !game.pid;
    Process target = isLauncher ? launcher : game;
    if (isLauncher && launchDeadline && waitingGame) {
      publish(AutomationState::STARTING, L"等待游戏启动；可在战网中检查更新或登录。", launcher.pid, 0,
              launchDeadline);
      continue;
    }
    if (!target.window) {
      publish(AutomationState::WAITING_WINDOW, L"战网已运行，等待原进程创建主窗口；若仅显示托盘图标，请打开托盘中的战网。不会重复启动。", target.pid);
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
    // A global key event reaches whatever window has focus, so with the game in
    // the background a pulse could only ever land in the player's own application.
    // The helper therefore never acts -- and never spends a capture -- on a game
    // it cannot see. Standby covers the same ground earlier and more cheaply.
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
    if (background) {
      input.release();
      gate.observe(-1);
      // A window owned by a higher-integrity process can never be activated or
      // receive our input, so asking the user to click it would be misleading.
      bool uipi = outranks(target);
      publish(uipi ? AutomationState::NEEDS_USER : AutomationState::WAITING_FOCUS,
              uipi ? (isLauncher ? L"战网以管理员权限运行，助手是普通权限；Windows 会阻止助手切换窗口和发送点击。"
                                   L"请以管理员身份运行助手，或让战网以普通权限运行。"
                                 : L"游戏以管理员权限运行，助手是普通权限；Windows 会阻止助手发送按键。"
                                   L"请以管理员身份运行助手。")
                   : (isLauncher ? L"请把战网窗口置于前台；切换回来后自动继续。"
                                 : L"已释放 Shift。游戏回到前台后自动继续。"),
              target.pid);
      continue;
    }
    const auto observationCapturedAt = GetTickCount64();
    const auto observationSequence = ++observationFrameSequence;
    if (!capture.grab(target.window, frame, false)) {
      observationTick.error = capture.failure().code;
      input.release();
      gate.observe(-1);
      disconnect.reset();
      publish(AutomationState::CAPTURE_FAILED, capture.failure().message(), target.pid);
      continue;
    }
    // Finish the final endorsement click as a short transaction. After the
    // allowance reaches zero its title changes and OCR may return UNKNOWN.
    // This branch still requires the captured foreground game, current input
    // generation and no intervening manual input; every failure stays retryable.
    if (!isLauncher && c.mode==WorkflowMode::Deathmatch && !c.dryRun &&
        deathmatch.endorsementCloseReady(now)) {
      if (!running || foregroundUse || epoch!=generation.load()) continue;
      input.release();gate.observe(-1);
      const bool ok=sameProcess(target,c.gameExe) && input.press(target.window,VK_ESCAPE,epoch);
      deathmatch.endorsementCloseAttempted(ok,GetTickCount64());
      publish(ok ? AutomationState::NAVIGATING : AutomationState::INPUT_BLOCKED,
              ok ? L"赞赏次数已用完，已按 Esc 关闭赞赏界面。"
                 : L"关闭赞赏界面未成功，等待焦点或手动按键释放后重试。",target.pid);
      continue;
    }
    bool hasTemplates = std::any_of(c.steps.begin(), c.steps.end(),
                                    [&](auto &s) { return s.launcher == isLauncher; });
    if (!hasTemplates && (!c.cnPreset || c.demo) && !c.semanticRecognition) {
      input.release();
      publish(AutomationState::NEEDS_CALIBRATION,
              isLauncher ? L"请校准战网启动按钮（操作选择“战网启动游戏”）。"
                         : L"请先校准大厅入口、房间导航和“到达目标房间”。",
              target.pid);
      continue;
    }
    auto manualMatch = [&] {
      auto value = match(frame, c.steps, isLauncher, c.threshold);
      if (value.index >= 0 && ((c.mode == WorkflowMode::Deathmatch && c.steps[value.index].action == Action::Ready) ||
                              (c.mode == WorkflowMode::CustomRoom && c.steps[value.index].action == Action::Combat))) value.index = -1;
      return value;
    };
    auto m = manualMatch();
    CNObservation builtin;
    SemanticObservation semantic;
    CNLauncherScore launcherScore;
    CNGameScore gameScore;
    const bool guardSemanticBurst = semanticBurst && cnJoin.bursting(now);
    if (c.semanticRecognition && !c.demo) {
      SemanticContext context{isLauncher,c.mode,c.roomTarget,deathmatch.finished(),deathmatch.leaving(),
          deathmatch.trusted() || (c.mode==WorkflowMode::Deathmatch && explicitRoom.trusted(game.pid,game.created,now))};
      context.endorsementAttempted=deathmatch.endorsementAttempted();
      context.endorsementClicks=deathmatch.endorsementClicks();
      context.trustedCustomRoom = !isLauncher && c.mode == WorkflowMode::CustomRoom &&
          (room.active() || cnJoin.awaitingRoom(now) || explicitRoom.trusted(game.pid,game.created,now));
      const auto activityAtRecognition = input.playerActivityRevision();
      semantic = vision.recognize(frame,context,[&] { return running && !foregroundUse && !quitting && epoch == generation.load(); },
                                  !cnJoin.bursting(now));
      if (!running || foregroundUse || epoch != generation.load()) continue;
      if (vision.usedOcr) {
        // OCR can take a second. Bind the action to a new capture of this same
        // process, so a loading transition or a manual click cannot reuse it.
        Frame current;
        if (!sameProcess(target,isLauncher ? L"Battle.net.exe" : c.gameExe) ||
            !capture.grab(target.window,current,false) ||
            (semantic && !semanticStillVisible(frame,current,semantic))) {
          observationTick.error = "post_ocr_revalidation_failed";
          input.release(); gate.observe(-1); cnJoin.observe(CNPage::None,GetTickCount64());
          deathmatch.observe(CNPage::None,GetTickCount64());
          continue;
        }
        frame = std::move(current);
        m = manualMatch();
      }
      now = GetTickCount64();
      if (vision.usedOcr) {
        tickAt = now + 1000;
        input.playerBusy();
        const auto activityAfterRecognition = input.playerActivityRevision();
        if (activityAfterRecognition != activityAtRecognition) {
          seenPlayerActivity = activityAfterRecognition;
          input.release(); gate.observe(-1);
          if (!room.active()) { cnJoin.reset(); explicitRoom.reset(); semanticBurst = false; }
          continue;
        }
      }
      // OCR can outlive the pending join/start window. Fresh HUD pixels cannot
      // extend an expired request into a permanent room session.
      if (semantic.observation.page==CNPage::Combat && semantic.evidence.empty() && !semantic.verifiedDeathmatch &&
          !deathmatch.trusted() && !explicitRoom.trusted(game.pid,game.created,now)) { semantic={};gate.observe(-1); }
      if (semantic.liveRoomHud && !(room.active() || cnJoin.awaitingRoom(now) ||
          explicitRoom.trusted(game.pid,game.created,now))) {
        semantic = {}; gate.observe(-1);
      }
      if (semantic) builtin = semantic.observation;
    }
    // A changed live room list invalidates the burst. The old fixed card
    // preset must not resurrect that action while fast OCR is deliberately off.
    if (guardSemanticBurst && !semantic) m.index = -1;
    if (!builtin && c.cnPreset && !c.demo && !(guardSemanticBurst && !semantic))
      builtin=isLauncher ? recognizeCNLauncher(frame,GetDpiForWindow(target.window)/96.0,c.cnProfile.launcher,&launcherScore)
                         : recognizeCNGame(frame,c.cnProfile.rules,c.cnProfile.join,&gameScore);
    if (!semantic && !isLauncher && builtin &&
        (c.mode == WorkflowMode::Deathmatch &&
          (builtin.page == CNPage::Play || builtin.page == CNPage::More || builtin.page == CNPage::Browser || builtin.page == CNPage::Join || builtin.page == CNPage::Room) ||
         c.mode == WorkflowMode::CustomRoom && !cnProfileTargets(c.cnProfile,c.roomTarget) &&
          (builtin.page == CNPage::Browser || builtin.page == CNPage::Join || builtin.page == CNPage::Room))) builtin = {};
    builtin=normalizeGameLoginRecovery(std::move(builtin),isLauncher);
    // Read-only monitor consumes original page/source evidence before the
    // automation below mutates its join gate or self-establishes Combat trust.
    if (!isLauncher)
      observationTick.visual(target,c.gameExe,frame,semantic,builtin,c.mode,cnJoin,explicitRoom,
          deathmatch,observationCapturedAt,observationSequence,m.ambiguous && !semantic && !builtin);
    // Read the evidence before anything else touches the gate. `prepare` below
    // resets it the moment the page is neither browse nor join, so asking it
    // second would throw away the very observation that confirms the join --
    // and that is precisely the case that matters, arriving in the room.
    const CNPage observedPage = builtin ? builtin.page :
        (m.index >= 0 && c.steps[m.index].action == Action::Ready ? CNPage::Room :
         m.index >= 0 && c.steps[m.index].action == Action::Combat ? CNPage::Combat : CNPage::None);
    if (c.mode==WorkflowMode::Deathmatch) {
      if (observedPage!=CNPage::None && observedPage!=CNPage::Hero && observedPage!=CNPage::Combat && observedPage!=CNPage::Respawn) explicitRoom.reset();
    } else explicitRoom.observe(observedPage);
    if (c.mode == WorkflowMode::Deathmatch) deathmatch.observe(observedPage,now);
    const auto joinState=cnJoin.observe(observedPage,now);
    auto card = builtin.hasCard ? builtin.card : cnCardClick(c.cnProfile);
    if (c.mode == WorkflowMode::CustomRoom) builtin=cnJoin.prepare(std::move(builtin),now,card);
    // Only two positive room observations confirm a join. Loading, unknown
    // frames and menus stop the burst without claiming that its click worked.
    if (joinState.joined && !c.dryRun) {
      room.joined();
      log(L"已连续确认目标房间画面，进入房间。");
    }
    bool useBuiltin=builtin && (semantic || builtin.step.action==Action::Restart ||
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
      if (unknownSince && now - unknownSince >= 5000 && unmatchedSaved < 40 &&
          now - lastUnmatchedSave >= 30000) {
        lastUnmatchedSave = now;
        std::error_code ec;
        auto dir = std::filesystem::path(directory) / L"unmatched";
        std::filesystem::create_directories(dir, ec);
        SYSTEMTIME t{};
        GetLocalTime(&t);
        std::wostringstream name;
        name << (isLauncher ? L"launcher-" : L"miss-") << std::setfill(L'0') << std::setw(2) << t.wHour << std::setw(2)
             << t.wMinute << std::setw(2) << t.wSecond << L"-" << std::setw(3)
             << t.wMilliseconds << L"-" << ++unmatchedSaved << L".bmp";
        auto path = dir / name.str();
        if (saveFrameBmp(frame, path))
          log(L"未识别画面已保存：" + path.wstring());
      }
      publish(m.ambiguous ? AutomationState::AMBIGUOUS : AutomationState::UNKNOWN,
              m.ambiguous ? L"多个页面特征过于相似，请重新圈选更独特的区域。"
                          : (c.semanticRecognition && !vision.error().empty()
                                 ? L"本地文字识别不可用：" + wide(vision.error())
                             : (m.aspectMismatch ? L"页面未匹配；检查窗口宽高比是否与校准时一致。"
                             : (isLauncher && launcherScore.evaluated
                                    ? L"战网启动页未匹配：锚点 " + launcherScoresText(launcherScore) +
                                          L"（每个锚点都要达到自己的阈值）。"
                                    : (!isLauncher && gameScore.evaluated
                                           ? gameClosestText(gameScore, c.cnProfile.rules)
                                           : L"未匹配页面，等待加载；本地 OCR 将继续检查按钮文字。")))),
              target.pid, m.score);
      continue;
    }
    unknownSince = 0;
    const auto &s = useBuiltin ? builtin.step : c.steps[m.index];
    if (!running || foregroundUse || epoch != generation.load())
      continue;
    int key = observationKey(s, m.index);
    bool confirmed = gate.observe(key);
    if (c.mode == WorkflowMode::CustomRoom) room.observe(&s, confirmed);
    if (!isLauncher && ((c.mode==WorkflowMode::CustomRoom && confirmed && s.action==Action::Ready && room.active()) ||
        (c.mode==WorkflowMode::Deathmatch && deathmatch.confirmedCombat(now)))) {
      offerBackground(game,now);
      if (SimpleEdition && simpleBackground && simpleBackgroundArmed.exchange(false))
        PostMessageW(mainWindow,StartSimpleBackground,epoch,0);
    }
    else if (!isLauncher && observedPage!=CNPage::None) {
      std::lock_guard lock(backgroundMutex);backgroundCandidate={};backgroundCandidateAt=0;
    }
    if (room.active() || (c.mode==WorkflowMode::Deathmatch && deathmatch.canInput(now))) explicitRoom.reset();
    disconnect.observe(s.action == Action::Restart, now);
    if (!confirmed) {
      if (room.active()) { continueRoom(m.score, background); continue; }
      input.release();
      publish(AutomationState::CONFIRMING, L"确认页面：" + s.label, target.pid, m.score);
      continue;
    }
    gate.arrived(key);
    if (semantic && vision.confirm(observedPage))
      log(L"自动页面校准已保存：" + s.label + L"（共 " + std::to_wstring(vision.learnedPages()) + L" 项）。");
    if (typedPage >= 0 && typedPage != m.index)
      typedPage = -1;
    if (s.action == Action::Ready) {
      continueRoom(m.score, background);
      continue;
    }
    input.release();
    if (s.action == Action::Wait) {
      if (c.mode == WorkflowMode::Deathmatch && observedPage == CNPage::Queue && semantic.verifiedDeathmatch)
        deathmatch.matched();
      publish(c.dryRun ? AutomationState::DRY_RUN : (observedPage == CNPage::Queue ? AutomationState::MATCHMAKING : AutomationState::WAITING_PAGE),
              s.label,target.pid,m.score,cnJoin.retryAt());
      continue;
    }
    if (s.action == Action::Combat) {
      if (c.mode != WorkflowMode::Deathmatch) continue;
      if (c.dryRun || !c.holdShift) {
        publish(c.dryRun ? AutomationState::DRY_RUN : AutomationState::ACTIVE, c.dryRun ? L"演练：死斗局内将随机按键，结算后自动重新匹配。" : L"死斗已确认，仅监控。",target.pid,m.score);
      } else if (c.standbyOnInput && input.playerBusy()) {
        deathmatch.observe(CNPage::None,now);
        publish(AutomationState::STANDBY,L"你正在操作，已暂停死斗随机按键。",target.pid,m.score);
      } else if (sameProcess(game,c.gameExe) && deathmatch.canInput(now)) {
        auto stroke = deathmatch.next(now);
        bool ok = input.tap(game.window,stroke.key,stroke.duration,epoch);
        if (ok) ok=sameProcess(game,c.gameExe) && input.tapMouse(game.window,stroke.rightClick,epoch);
        publish(ok ? AutomationState::ACTIVE : AutomationState::INPUT_BLOCKED,ok ? L"死斗局内随机按键及鼠标左右键；持续检查结算并自动重新匹配。" : L"随机按键未发送，等待焦点或手动按键释放。",target.pid,m.score);
      } else publish(AutomationState::ACTIVE,L"死斗局内，等待下一次随机按键。",target.pid,m.score);
      continue;
    }
    if (s.action == Action::Restart) {
      const Millis delay=c.demo ? 5000 : GameLoginRecoveryDelay;
      publish(AutomationState::LOGIN_STUCK, L"已识别恢复页面：" + s.label + L"；短暂确认后关闭并重新启动游戏。", target.pid,
              m.score, disconnect.deadline(delay));
      if (disconnect.elapsed(now, delay)) {
        if (isLauncher) {
          publish(AutomationState::NEEDS_USER, L"战网需要登录或验证，请处理后继续。", target.pid);
          continue;
        }
        recover(L"网络断线后的游戏登录/错误页面已确认，关闭游戏并重新启动。");
      }
      continue;
    }
    if (background) {
      publish(AutomationState::WAITING_FOCUS, L"已离开房间画面，暂停按键；请把游戏切回前台继续导航。", target.pid);
      continue;
    }
    if (c.dryRun) {
      publish(AutomationState::DRY_RUN, L"演练：" + s.label + L" → " + ActionNames[int(s.action)], target.pid,
              m.score);
      continue;
    }
    // "自动加入" is worth retrying quickly and often: if the click did not take,
    // the lobby may have filled in that instant, and the normal 5s/3 attempts
    // budget would stop and demand a manual restart instead of just asking again.
    const bool joining = useBuiltin && builtin.page == CNPage::Join;
    const bool bursting = useBuiltin && cnJoin.bursting(now);
    // Inside a burst the pacing is the burst's, not the navigation default.
    // Outside one nothing changes: a navigation button that did nothing still
    // gets five seconds and three tries.
    const Millis cooldown = bursting ? CNJoinGate::BurstInterval : (joining ? 1500 : 5000);
    const int maximum = bursting ? CNJoinGate::BurstClicks : (joining ? 8 : 3);
    // A burst is one attempt, not twenty-five. The budget is cleared whenever a
    // window is not running, and also the instant a window has spent all its
    // clicks -- otherwise that twenty-fifth click would be counted as the whole
    // allowance and the next attempt would start already exhausted.
    if ((useBuiltin && builtin.page==CNPage::RoomList && s.action==Action::DoubleClick) ||
        (joining && (!bursting || gate.exhausted(m.index, CNJoinGate::BurstClicks))))
      gate.restartAttempts();
    if (!gate.cooldownReady(now, cooldown)) {
      publish(AutomationState::NAVIGATING, L"等待页面变化：" + s.label, target.pid, m.score);
      continue;
    }
    if (gate.exhausted(m.index, maximum) || (s.action == Action::Text && typedPage == m.index)) {
      if (semantic && !isLauncher && c.mode==WorkflowMode::Deathmatch &&
          (builtin.page==CNPage::Play || builtin.page==CNPage::More)) {
        // Known-but-stalled navigation also needs an original frame for diagnosis;
        // the OCR worker's last BMP may only contain a small cropped label.
        const auto snapshot=std::filesystem::path(directory)/L"navigation-stalled.bmp";
        if (saveFrameBmp(frame,snapshot)) log(L"导航停滞画面已保存："+snapshot.wstring());
      }
      publish(AutomationState::NEEDS_USER,
              L"操作后页面没有变化：" + s.label + L"。为避免重复输入已暂停；停止后重新开始。",
              target.pid, m.score);
      blocked = true;
      continue;
    }
    if (!gate.canAct(m.index, now, cooldown, maximum)) {
      publish(AutomationState::NAVIGATING, L"等待页面变化：" + s.label, target.pid, m.score);
      continue;
    }
    bool ok = false;
    if (s.action==Action::Launch) launchMonitor.begin(c);
    if (!sameProcess(target, isLauncher ? L"Battle.net.exe" : c.gameExe)) {
      publish(AutomationState::WAITING_WINDOW, L"进程已变化，重新检测。");
      lastProcessScan = 0;
      continue;
    }
    if (s.action == Action::Launch && !cycleBudgeted) {
      if (!budget.take(now)) {
        publish(AutomationState::BACKOFF, L"启动冷却中。", target.pid, 0, budget.retryAt());
        continue;
      }
      cycleBudgeted = true;
      // The counter is consumed here, before the click: the budget slot is spent
      // either way, and a failed click must not be retried without limit.
      countRecovery(L"准备点击启动按钮「" + s.label + L"」，尚未执行。");
    }
    // A recognised page that must never be acted on: the room session has
    // already been cleared above, so all that is left is to say so and wait.
    if (s.action == Action::Ignore) {
      publish(observedPage == CNPage::Login ? AutomationState::NEEDS_USER : AutomationState::IDLE, L"已知的非对局页面：" + s.label + L"；已释放按键。",
              target.pid, m.score);
      continue;
    }
    switch (s.action) {
    case Action::Click:
    case Action::Launch:
      ok = input.click(target.window, s.click, epoch);
      break;
    case Action::DoubleClick:
      ok = input.doubleClick(target.window, s.click, epoch);
      break;
    case Action::Enter:
      ok = input.press(target.window, VK_RETURN, epoch);
      break;
    case Action::Escape:
      ok = input.press(target.window, VK_ESCAPE, epoch);
      break;
    case Action::Space:
      ok = input.tap(target.window,VK_SPACE,20,epoch);
      break;
    case Action::Endorse:
      ok = input.tap(target.window,'N',20,epoch);
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
    if (useBuiltin && builtin.page == CNPage::Join && ok) semanticBurst = bool(semantic);
    if (!cnJoin.bursting(now)) semanticBurst = false;
    if (useBuiltin && c.mode == WorkflowMode::Deathmatch) deathmatch.acted(builtin.page,ok,s.action,GetTickCount64());
    // That click may have just opened a burst. The tick length was decided at the
    // top of the loop, before the burst existed, so re-pace it here or the second
    // click waits out the whole navigation second and the window loses a fifth of
    // its clicks.
    if (useBuiltin && cnJoin.bursting(now))
      tickAt = now + CNJoinGate::BurstInterval;
    if (!ok) {
      if (s.action==Action::Launch) launchMonitor.reset();
      publish(AutomationState::INPUT_BLOCKED,
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
      publish(AutomationState::STARTING, L"已点击启动，等待游戏窗口。", target.pid, m.score, launchDeadline);
    } else
      publish(AutomationState::NAVIGATING, L"已执行：" + s.label, target.pid, m.score);
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
  if (!c.cnPreset) {
    if (c.semanticRecognition) add(L"[自动] 本地 OCR 识别按钮文字；确认后保存 adaptive-profile.json");
    return;
  }
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
  add(L"—— 页面配置 · " + source + L" · 只读 ——");
  if (!c.cnProfile.launcher.empty())
    add(L"[战网] 战网启动锚点 ×" + std::to_wstring(c.cnProfile.launcher.size()) +
        L"  →  战网启动游戏   （预设）");
  for (const auto &rule : c.cnProfile.rules)
    add(L"[游戏] " + rule.label + L"  →  " + ActionNames[int(rule.action)] + L"   （预设）");
  if (c.semanticRecognition) add(L"[自动] 本地 OCR 识别按钮文字；确认后保存 adaptive-profile.json");
}
bool syncConfig() {
  auto c = snapshotConfig();
  if (SimpleEdition) {
    try {
      const int selection=std::clamp(int(SendMessageW(control(ModeBox),CB_GETCURSEL,0,0)),0,3);
      if (!saveSimpleConfig(directory,c,selection)) throw std::runtime_error("Cannot save simple mode");
      simpleBackground=(selection%2)!=0;
      std::lock_guard lock(configMutex);config=std::move(c);return true;
    } catch (...) {
      failures.record("NEEDS_CONFIG",L"简易版设置或内置识别初始化失败，请检查配置目录写入权限。",0);
      MessageBoxW(mainWindow,L"无法保存简易版设置，请检查本地目录权限。",L"设置失败",MB_ICONERROR);return false;
    }
  }
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
  c.standbyOnInput = checked(Standby);
  c.standbySeconds = std::clamp(int(number(textOf(StandbySeconds))), StandbySecondsMin,
                                StandbySecondsMax);
  c.cnPreset = !c.demo && checked(CnPreset);
  c.semanticRecognition = !c.demo && checked(Semantic);
  c.mode = WorkflowMode(std::clamp<int>(int(SendMessageW(control(ModeBox),CB_GETCURSEL,0,0)),0,1));
  c.roomTarget = textOf(RoomTarget);
  if (c.roomTarget.empty() || c.roomTarget.size() > 64 || c.roomTarget.find_first_of(L"\r\n\t") != std::wstring::npos) {
    MessageBoxW(mainWindow,L"房间名称需为 1–64 个字符。",L"配置未保存",MB_ICONWARNING);
    return false;
  }
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
void setRunning(bool on, bool manualRestart = false, bool alreadyInRoom = false) {
  if (on && learning::isOpen()) {
    MessageBoxW(mainWindow,L"请先完成或取消录制/标注，并关闭演示学习窗口，再开始运行。",L"演示学习正在打开",MB_ICONINFORMATION);
    return;
  }
  if (on && running && manualRestart) {
    restartRequested=generation.load();
    matchObserver.begin(generation.load(),snapshotConfig().mode,GetTickCount64());
    if (SimpleEdition) simpleBackgroundArmed=simpleBackground.load();
    foregroundUse=false;backgroundInput.stop();input.blockForeground(false);
    wake.notify_all();
    return;
  }
  if (on && (running || calibrating))
    return;
  if (on && !syncConfig())
    return;
  if (SimpleEdition) simpleBackgroundArmed=on && simpleBackground;
  Process declaredRoom;
  if (on && alreadyInRoom) {
    auto current = snapshotConfig();
    declaredRoom = findProcess(current.gameExe,current.gameClass);
    if (!declaredRoom.window) {
      MessageBoxW(mainWindow,L"请先进入与当前模式对应的自定义房间或死斗对局。",L"当前对局启动",MB_ICONINFORMATION);
      return;
    }
  }
  ++generation;
  if (on) matchObserver.begin(generation.load(),snapshotConfig().mode,GetTickCount64());
  else matchObserver.stop(generation.load(),GetTickCount64());
  foregroundUse=false;backgroundInput.stop();input.blockForeground(false);
  {std::lock_guard lock(backgroundMutex);backgroundCandidate={};backgroundCandidateAt=0;}
  roomStartPid = declaredRoom.pid; roomStartCreated = declaredRoom.created;
  restartRequested=on && manualRestart ? generation.load() : ~0u;
  if (on) {
    auto snapshot = snapshotConfig();
    input.holdWindow(Millis(snapshot.standbySeconds) * 1000);
    input.enable(!snapshot.dryRun, generation.load());
    running = true;
  } else {
    running = false;
    input.enable(false, generation.load());
    cancelCalibration();
    learning::cancel();
  }
  if (on) {
    publish(AutomationState::DETECTING, alreadyInRoom
      ? L"已标记当前目标对局，正在连续确认局内画面；停止手动操作后开始按键。"
      : L"正在检测游戏与页面。");
  } else {
    input.release();
    publish(AutomationState::STOPPED, L"已停止，已释放本程序按下的全部按键。");
  }
  for (int id : settingsIds)
    EnableWindow(control(id), !on);
  EnableWindow(control(Start), !on);
  wake.notify_all();
  if (on) {
    auto c = snapshotConfig();
    auto p = findProcess(c.gameExe, c.gameClass);
    if (!p.window && !c.demo)
      p = findLauncherProcess();
    if (p.window) {
      requestForeground(p.window);
    }
  }
}

void applyImportedProfile(const CNProfile &profile) {
  if (!IsWindow(mainWindow)) return;
  auto updated = snapshotConfig();
  updated.cnProfile = profile;
  updated.cnProfileSource = "local";
  updated.cnProfileError.clear();
  updated.cnPreset = true;
  if (!profile.roomTarget.empty()) updated.roomTarget = profile.roomTarget;
  {
    std::lock_guard lock(configMutex);
    config = updated;
  }
  SendMessageW(control(CnPreset),BM_SETCHECK,BST_CHECKED,0);
  SetWindowTextW(control(RoomTarget),updated.roomTarget.c_str());
  refreshSteps();
  try {
    saveConfig(directory,updated);
    publish(AutomationState::STOPPED,L"页面配置已导入，旧配置已备份。可先演练检查，再开始运行。");
  } catch (...) {
    MessageBoxW(mainWindow,L"页面文件已导入，但运行设置保存失败。请检查本地目录权限后重新保存配置。",L"设置保存失败",MB_ICONERROR);
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
    publish(AutomationState::STOPPED, L"校准已保存：" + s.label);
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
      if (calibration.step.action == Action::Click || calibration.step.action == Action::DoubleClick || calibration.step.action == Action::Launch ||
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
  if (running || calibrating || learning::isOpen() || !syncConfig())
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
  if ((calibration.step.launcher && (calibration.step.action == Action::Ready || calibration.step.action == Action::Combat)) ||
      (!calibration.step.launcher && calibration.step.action == Action::Launch)) {
    MessageBoxW(mainWindow, L"房间和死斗局内校准仅用于游戏；“战网启动游戏”仅用于战网。", L"校准对象",
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
  auto p = calibration.step.launcher ? findLauncherProcess()
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
void updateFonts() {
  auto oldFont=font,oldTitle=titleFont;
  font=CreateFontW(-u(14),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
  titleFont=CreateFontW(-u(22),0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
  for (const auto &item:windowLayout.items())
    SendMessageW(item.window,WM_SETFONT,reinterpret_cast<WPARAM>(item.id==StatusTitle ? titleFont : font),TRUE);
  if (oldFont) DeleteObject(oldFont);
  if (oldTitle) DeleteObject(oldTitle);
}
HWND add(const wchar_t *cls, const std::wstring &txt, int id, int x, int y, int width, int height,
         DWORD style = 0) {
  using Band=ui::WindowLayout::Band;
  using Rule=ui::WindowLayout::WidthRule;
  const auto band=y>=721 ? Band::Footer : y>=153 ? Band::Content : Band::Header;
  auto rule=width>=600 ? Rule::Stretch : Rule::Fixed;
  switch (id) {
  case Launcher:case NeacPath:case Machine:case TextArg:case Label:case RoomTarget:
  case CnPreset:case NetStatus:rule=Rule::Stretch;break;
  case Browse:case NeacBrowse:case Remove:case Record:case Semantic:case Diagnose:
  case RestartNow:case OpenData:case Standby:case StandbySeconds:rule=Rule::Right;break;
  case StartInRoom:rule=Rule::ThirdFirst;break;
  case Learning:rule=Rule::ThirdMiddle;break;
  case ImportProfile:rule=Rule::ThirdLast;break;
  }
  if (!id && y==333) rule=Rule::Right;
  const bool interactive=wcscmp(cls,L"STATIC")!=0;
  auto parent=band==Band::Content ? windowLayout.pane() : mainWindow;
  auto h=CreateWindowExW(wcscmp(cls,L"EDIT")==0 ? WS_EX_CLIENTEDGE : 0,cls,txt.c_str(),
                        WS_CHILD|WS_VISIBLE|style|(interactive ? WS_TABSTOP : 0),u(x),
                        u(y-(band==Band::Content ? ui::WindowLayout::ContentTop : 0)),u(width),u(height),
                        parent,reinterpret_cast<HMENU>(INT_PTR(id)),instance,nullptr);
  windowLayout.remember(h,id,x,y,width,height,band,rule,id==Steps);
  if (interactive && band==Band::Content)
    SetWindowSubclass(h,ui::focusSubclass,1,reinterpret_cast<DWORD_PTR>(&windowLayout));
  SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(id==StatusTitle ? titleFont : font),TRUE);
  return h;
}
void resizeSimple() {
  RECT client{};GetClientRect(mainWindow,&client);
  const int width=client.right,height=client.bottom,margin=u(20),gap=u(12);
  auto place=[&](int id,int x,int y,int w,int h){MoveWindow(control(id),x,y,w,h,TRUE);};
  place(StatusTitle,margin,u(16),width-margin*2,u(30));
  place(ModeHeading,margin,u(62),u(76),u(26));
  place(ModeBox,u(100),u(58),width-u(120),u(140));
  const int button=(width-margin*2-gap*3)/4;
  place(Start,margin,u(100),button,u(34));place(Stop,margin+button+gap,u(100),button,u(34));
  place(RestartNow,margin+(button+gap)*2,u(100),button,u(34));
  place(ForegroundUse,margin+(button+gap)*3,u(100),width-margin-(margin+(button+gap)*3),u(34));
  place(StatusDetail,margin,u(148),width-margin*2,u(50));
  place(FailureHeading,margin,u(204),width-margin*2,u(24));
  place(FailureList,margin,u(234),width-margin*2,std::max(u(80),height-u(302)));
  place(Stats,margin,height-u(47),width-u(220),u(24));
  place(OpenFailures,width-u(190),height-u(52),u(170),u(32));
}
void simpleControls() {
  auto create=[&](const wchar_t *cls,const wchar_t *label,int id,int x,int y,int w,int h,DWORD style=0) {
    auto window=CreateWindowExW(wcscmp(cls,L"EDIT")==0 ? WS_EX_CLIENTEDGE : 0,cls,label,
      WS_CHILD|WS_VISIBLE|style|(wcscmp(cls,L"STATIC") ? WS_TABSTOP : 0),u(x),u(y),u(w),u(h),
      mainWindow,reinterpret_cast<HMENU>(INT_PTR(id)),instance,nullptr);
    windowLayout.remember(window,id,x,y,w,h,ui::WindowLayout::Band::Header,ui::WindowLayout::WidthRule::Fixed);
    SendMessageW(window,WM_SETFONT,reinterpret_cast<WPARAM>(id==StatusTitle ? titleFont : font),TRUE);
  };
  create(L"STATIC",L"守望挂机 · 简易版",StatusTitle,20,16,600,30);
  create(L"STATIC",L"挂机模式",ModeHeading,20,62,76,26);
  create(L"COMBOBOX",L"",ModeBox,100,58,520,140,CBS_DROPDOWNLIST);
  for (auto label:{L"战令房间 · 前台",L"战令房间 · 后台（实验）",L"自由死斗 · 前台",L"自由死斗 · 后台（实验）"})
    SendMessageW(control(ModeBox),CB_ADDSTRING,0,reinterpret_cast<LPARAM>(label));
  SendMessageW(control(ModeBox),CB_SETCURSEL,simpleModeIndex(directory,snapshotConfig()),0);
  create(L"BUTTON",L"开始挂机",Start,20,100,190,34,BS_PUSHBUTTON);
  create(L"BUTTON",L"停止 · F8",Stop,222,100,190,34,BS_PUSHBUTTON);
  create(L"BUTTON",L"重启游戏",RestartNow,324,100,140,34,BS_PUSHBUTTON);
  create(L"BUTTON",L"前台使用",ForegroundUse,480,100,140,34,BS_PUSHBUTTON);
  create(L"STATIC",L"选择模式后开始。识别和失败记录会自动运行。",StatusDetail,20,148,600,50);
  create(L"STATIC",L"失败记录 · 自动保存，最新在上",FailureHeading,20,204,600,24);
  create(L"EDIT",L"尚无失败记录。",FailureList,20,234,600,178,
         ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL);
  create(L"STATIC",L"",Stats,20,433,420,24);
  create(L"BUTTON",L"打开失败日志",OpenFailures,450,428,170,32,BS_PUSHBUTTON);
  settingsIds={ModeBox};resizeSimple();
}
std::wstring simpleStateTitle(const std::string &state) {
  if(state=="STOPPED") return L"已停止";
  if(state=="ACTIVE") return L"挂机中";
  if(state=="BACKGROUND_UNVERIFIED") return L"后台挂机 · 效果待验证";
  if(state=="STANDBY") return L"等待停止手动操作";
  if(state=="RECOVERING" || state=="STARTING" || state=="WAITING_GAME_EXIT") return L"正在重启游戏";
  if(state=="BACKOFF") return L"等待恢复冷却";
  if(state=="WAITING_FOCUS") return L"等待游戏前台";
  if(state=="WAITING_DESKTOP") return L"等待返回桌面或授权";
  if(failureState(state,L"")) return L"需要检查 · 原因已记录";
  return L"正在识别与进入对局";
}
void controls() {
  if (SimpleEdition) {simpleControls();return;}
  auto c = snapshotConfig();
  add(L"STATIC", L"OverwatchHelper · 国服试用版", StatusTitle, 22, 17, 810, 30);
  add(L"STATIC", L"", StatusDetail, 22, 56, 810, 42);
  add(L"BUTTON", L"开始", Start, 22, 106, 115, 32, BS_PUSHBUTTON);
  add(L"BUTTON", L"停止 · F8", Stop, 148, 106, 115, 32, BS_PUSHBUTTON);
  add(L"BUTTON", L"保存配置", Save, 274, 106, 100, 32);
  add(L"BUTTON", L"前台使用（实验）", ForegroundUse, 385, 106, 173, 32, BS_PUSHBUTTON);
  EnableWindow(control(ForegroundUse),TRUE);
  add(L"BUTTON", L"演练（无动作）", Dry, 568, 107, 145, 28, BS_AUTOCHECKBOX);
  add(L"BUTTON", L"运行诊断", Diagnose, 726, 106, 92, 32, BS_PUSHBUTTON);
  SendMessageW(control(Dry), BM_SETCHECK, c.dryRun ? BST_CHECKED : BST_UNCHECKED, 0);
  auto edit = [&](const wchar_t *label, int id, const std::wstring &value, int y, int width = 648,
                  DWORD extra = 0) {
    add(L"STATIC", label, 0, 22, y + 4, 118, 24);
    add(L"EDIT", value, id, 144, y, width, 26, ES_AUTOHSCROLL | WS_TABSTOP | extra);
  };
  edit(L"游戏进程", GameExe, c.gameExe, 153, 235);
  add(L"STATIC", L"机器名", 0, 402, 157, 65, 24);
  add(L"EDIT", c.machine, Machine, 473, 153, 345, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  edit(L"启动器路径", Launcher, c.launcherPath, 188, 570);
  add(L"BUTTON", L"浏览", Browse, 726, 188, 92, 26);
  edit(L"启动参数（可空）", Args, c.launcherArgs, 222, 674);
  edit(L"看板服务地址", Server, c.server, 256, 674);
  edit(L"Agent 令牌", Token, c.token, 290, 674, ES_PASSWORD);
  add(L"BUTTON", L"发送挂机按键", Hold, 22, 329, 170, 26, BS_AUTOCHECKBOX);
  add(L"COMBOBOX", L"", ShiftMode, 195, 329, 190, 100, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"每秒点按（20ms）", L"持续按住（前台）"})
    SendMessageW(control(ShiftMode), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ShiftMode), CB_SETCURSEL, c.pulseShift ? 0 : 1, 0);
  add(L"BUTTON", L"自动恢复（次数不限，带冷却）", Auto, 22, 365, 350, 26, BS_AUTOCHECKBOX);
  add(L"BUTTON", L"页面配置 / 旧预设回退", CnPreset, 405, 365, 413, 26, BS_AUTOCHECKBOX);
  SendMessageW(control(CnPreset),BM_SETCHECK,c.cnPreset?BST_CHECKED:BST_UNCHECKED,0);
  // Standby sits on the same row as the Shift controls, which stop at x=385.
  add(L"BUTTON", L"你在用电脑时待机", Standby, 405, 329, 200, 26, BS_AUTOCHECKBOX);
  add(L"STATIC", L"停止操作", 0, 612, 333, 72, 22);
  add(L"EDIT", std::to_wstring(c.standbySeconds), StandbySeconds, 686, 329, 56, 26,
      ES_AUTOHSCROLL | ES_NUMBER | WS_TABSTOP);
  add(L"STATIC", L"秒后接管", 0, 748, 333, 72, 22);
  SendMessageW(control(Standby), BM_SETCHECK, c.standbyOnInput ? BST_CHECKED : BST_UNCHECKED, 0);
  edit(L"OWNeac 路径", NeacPath, c.neacPath, 399, 570);
  add(L"BUTTON", L"浏览", NeacBrowse, 726, 399, 92, 26);
  SendMessageW(control(Hold), BM_SETCHECK, c.holdShift ? BST_CHECKED : BST_UNCHECKED, 0);
  SendMessageW(control(Auto), BM_SETCHECK, c.autoRecover ? BST_CHECKED : BST_UNCHECKED, 0);
  add(L"STATIC",L"模式",0,22,437,50,24);
  add(L"COMBOBOX",L"",ModeBox,74,433,230,110,CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"自定义房间 / 战令", L"自由死斗 / 自动匹配"})
    SendMessageW(control(ModeBox),CB_ADDSTRING,0,reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ModeBox),CB_SETCURSEL,int(c.mode),0);
  add(L"STATIC",L"房间",0,315,437,46,24);
  add(L"EDIT",c.roomTarget,RoomTarget,361,433,220,26,ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON",L"本地 OCR / 自动校准",Semantic,602,433,216,26,BS_AUTOCHECKBOX);
  SendMessageW(control(Semantic),BM_SETCHECK,c.semanticRecognition ? BST_CHECKED : BST_UNCHECKED,0);
  add(L"STATIC", L"页面校准 · 手动打开一个页面，选择操作，录制特征与点击位置", 0, 22, 477, 800, 24);
  add(L"COMBOBOX", L"", Scope, 22, 510, 92, 140, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : {L"游戏", L"战网"})
    SendMessageW(control(Scope), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(Scope), CB_SETCURSEL, 0, 0);
  add(L"COMBOBOX", L"", ActionBox, 126, 510, 260, 280, CBS_DROPDOWNLIST | WS_TABSTOP);
  for (auto t : ActionNames)
    SendMessageW(control(ActionBox), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t));
  SendMessageW(control(ActionBox), CB_SETCURSEL, 0, 0);
  add(L"EDIT", L"页面名称", Label, 399, 510, 220, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"3 秒后录制", Record, 632, 510, 186, 28);
  add(L"STATIC", L"输入文字内容", 0, 22, 548, 115, 24);
  add(L"EDIT", L"", TextArg, 144, 544, 480, 26, ES_AUTOHSCROLL | WS_TABSTOP);
  add(L"BUTTON", L"删除选中页面", Remove, 637, 544, 181, 28);
  add(L"LISTBOX", L"", Steps, 22, 586, 796, 82, WS_BORDER | WS_VSCROLL | LBS_NOTIFY | WS_TABSTOP);
  add(L"BUTTON",L"已在局内，开始挂机",StartInRoom,22,680,226,30,BS_PUSHBUTTON);
  add(L"BUTTON",L"演示 / 本地模型校准",Learning,260,680,280,30,BS_PUSHBUTTON);
  add(L"BUTTON",L"导入页面配置",ImportProfile,552,680,266,30,BS_PUSHBUTTON);
  add(L"STATIC", L"", Stats, 22, 721, 790, 22);
  add(L"STATIC", L"", NetStatus, 22, 751, 480, 22);
  add(L"BUTTON", L"重启游戏", RestartNow, 512, 745, 160, 30);
  add(L"BUTTON", L"配置与日志", OpenData, 682, 745, 136, 30);
  add(L"STATIC",
      L"自定义房间发送 Shift；死斗发送随机按键。登录验证仍需手动处理；F8 立即停下。",
      0, 22, 785, 800, 26);
  settingsIds = {Save,    Browse, Record, Remove, GameExe, Launcher,  Args,  Server, Token,
                 Machine, Dry,    Hold,   Auto,   Scope,   ActionBox, Label, TextArg,
                 ShiftMode, NeacPath, NeacBrowse, CnPreset, Standby, StandbySeconds, ModeBox, Semantic, RoomTarget, Learning, ImportProfile};
  refreshSteps();
  windowLayout.arrangeTabOrder();
  windowLayout.resize();
}
void refreshStatus() {
  Status s;
  {
    std::lock_guard lock(statusMutex);
    s = status;
  }
  auto title = SimpleEdition ? L"守望挂机 · "+simpleStateTitle(s.state) : L"OverwatchHelper 开发版 · " + wide(s.state);
  if (SimpleEdition) {
    const auto history=failures.snapshot();static unsigned long long lastRevision=~0ull;
    if (history.first!=lastRevision) {SetWindowTextW(control(FailureList),history.second.c_str());lastRevision=history.first;}
    const auto summary=L"已运行 "+std::to_wstring((GetTickCount64()-bootTime)/1000)+L" 秒 · 恢复 "+std::to_wstring(s.restarts)+L" 次";
    SetWindowTextW(control(ForegroundUse),foregroundUse ? L"结束前台使用" : L"前台使用");
    EnableWindow(control(ForegroundUse),!calibrating);
    SetWindowTextW(control(Stats),summary.c_str());SetWindowTextW(control(StatusTitle),title.c_str());
    SetWindowTextW(control(StatusDetail),s.detail.c_str());return;
  }
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
  SetWindowTextW(control(ForegroundUse),foregroundUse ? L"结束前台使用" : L"前台使用（实验）");
  EnableWindow(control(ForegroundUse),!calibrating);
  auto net = s.http == 0 ? L"远程看板：未配置"
                         : (s.http >= 200 && s.http < 300
                                ? L"远程看板：已连接"
                                : L"远程看板：连接失败 / HTTP " + std::to_wstring(s.http));
  SetWindowTextW(control(NetStatus), net.c_str());
}
LRESULT CALLBACK mainProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_CTLCOLORSTATIC: {
    auto dc=reinterpret_cast<HDC>(wp);SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,reinterpret_cast<HWND>(lp)==control(StatusTitle) ? RGB(25,55,85) : GetSysColor(COLOR_WINDOWTEXT));
    return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
  }
  case WM_GETMINMAXINFO: {
    RECT minimum{0,0,u(SimpleEdition ? 640 : ui::WindowLayout::BaseWidth),u(SimpleEdition ? 400 : ui::WindowLayout::MinimumHeight)};
    AdjustWindowRectExForDpi(&minimum,DWORD(GetWindowLongPtrW(w,GWL_STYLE)),FALSE,0,dpi);
    auto *limits=reinterpret_cast<MINMAXINFO*>(lp);
    limits->ptMinTrackSize={minimum.right-minimum.left,minimum.bottom-minimum.top};
    return 0;
  }
  case WM_SIZE:
    if (wp!=SIZE_MINIMIZED) {if (SimpleEdition) resizeSimple();else windowLayout.resize();}
    return 0;
  case WM_DPICHANGED: {
    dpi=HIWORD(wp);windowLayout.setDpi(dpi);updateFonts();
    const auto *suggested=reinterpret_cast<RECT*>(lp);
    SetWindowPos(w,nullptr,suggested->left,suggested->top,suggested->right-suggested->left,
                 suggested->bottom-suggested->top,SWP_NOZORDER|SWP_NOACTIVATE);
    if (SimpleEdition) resizeSimple();else windowLayout.resize();return 0;
  }
  case WM_MOUSEWHEEL:
    windowLayout.mouseWheel(GET_WHEEL_DELTA_WPARAM(wp));return 0;
  case StartSimpleBackground:
    if (SimpleEdition && running && wp==generation.load() && simpleBackground && !foregroundUse) {
      if (backgroundOffer().pid) SendMessageW(w,WM_COMMAND,ForegroundUse,0);
      else simpleBackgroundArmed=true;
    }
    return 0;
  case WM_COMMAND:
    switch (LOWORD(wp)) {
    case ForegroundUse:
      if (foregroundUse.exchange(false)) {
        const auto target=backgroundInput.target();backgroundInput.stop();input.blockForeground(false);
        if (sameProcess(target,snapshotConfig().gameExe)) requestForeground(target.window);
        wake.notify_all();
      } else {
        auto current=snapshotConfig();
        auto target=findProcess(current.gameExe,current.gameClass);
        if (!target.window || !sameProcess(target,current.gameExe)) {
          failures.record("INPUT_BLOCKED",L"前台使用未开启：未找到可核验的游戏窗口，请先进入目标房间或死斗对局。",target.pid);
          MessageBoxW(w,L"未找到游戏窗口。请先打开游戏并进入目标房间或死斗对局，再点前台使用。",L"前台使用未开启",MB_ICONINFORMATION);
          break;
        }
        // This button is an explicit declaration of the current selected game
        // session, equivalent to StartInRoom. OCR is not a prerequisite.
        if (!running) setRunning(true,false,true);
        if (!running) break;
        current=snapshotConfig();
        simpleBackgroundArmed=false;
        if (target.pid && sameProcess(target,current.gameExe)) {
          input.blockForeground(true);
          if (backgroundInput.begin(target,current.gameExe,generation.load())) {
            foregroundUse=true;
            ShowWindowAsync(target.window,SW_MINIMIZE);
            const auto observationNow=GetTickCount64();
            const auto ticket=matchObserver.currentTicket();
            matchObserver.selectProcess(ticket,matchProcess(target),observationNow);
            matchObserver.unavailable(ticket,matchProcess(target),observationNow,
                matchWindow(target,true),"background_observation_paused");
            publish(AutomationState::BACKGROUND_UNVERIFIED,L"前台使用：按你指定的当前对局开启后台消息并暂停 OCR；实际效果待验证。",target.pid);
            wake.notify_all();
          } else { input.blockForeground(false);publish(AutomationState::INPUT_BLOCKED,L"无法核验后台目标或权限，未开启前台使用。Windows 错误 "+std::to_wstring(backgroundInput.error()),target.pid); }
        }
      }
      break;
    case ModeBox:
      if (SimpleEdition && HIWORD(wp)==CBN_SELCHANGE && !running) syncConfig();
      break;
    case OpenFailures: {
      auto path=std::filesystem::path(directory)/L"failures.log";
      if (std::filesystem::exists(path)) ShellExecuteW(w,L"open",path.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
      else MessageBoxW(w,L"尚无失败记录。出现异常后会自动保存。",L"失败记录",MB_OK);
      break;
    }
    case Start:
      setRunning(true,false,SimpleEdition && bool(findProcess(snapshotConfig().gameExe,snapshotConfig().gameClass).window));
      break;
    case StartInRoom:
      if (learning::isOpen()) {
        setRunning(true,false,true);
        break;
      }
      if (running) setRunning(false);
      SendMessageW(control(Semantic),BM_SETCHECK,BST_CHECKED,0);
      setRunning(true,false,true);
      break;
    case Learning:
      if (!running && !calibrating && syncConfig()) {
        auto current = snapshotConfig();
        learning::show(w,directory,current.gameExe,current.roomTarget,applyImportedProfile);
      }
      break;
    case ImportProfile: {
      if (running || calibrating || learning::isOpen() || !syncConfig()) break;
      wchar_t path[32768]{};
      OPENFILENAMEW dialog{};
      dialog.lStructSize = sizeof(dialog); dialog.hwndOwner = w;
      dialog.lpstrFile = path; dialog.nMaxFile = 32768;
      dialog.lpstrFilter = L"页面配置 JSON\0*.json\0所有文件\0*.*\0";
      dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
      if (GetOpenFileNameW(&dialog)) {
        CNProfile imported; std::wstring error;
        if (learning::importProfile(directory,path,imported,error)) applyImportedProfile(imported);
        else MessageBoxW(w,error.c_str(),L"导入失败",MB_ICONERROR);
      }
      break;
    }
    case RestartNow:
      setRunning(true,true);
      break;
    case Stop:
      setRunning(false);
      break;
    case Save:
      if (syncConfig())
        publish(AutomationState::STOPPED, L"配置已保存。");
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
    directory = SimpleEdition ? simpleDataDirectory(options) : resolveDataDirectory(options);
    failures.open(directory);
    config = SimpleEdition ? loadSimpleConfig(directory,dataDirectory(false)) : loadConfig(directory, options.demo);
    if (SimpleEdition) {simpleBackground=(simpleModeIndex(directory,config)%2)!=0;status.detail=L"选择挂机模式后开始；已在局内时可直接点前台使用。";}
    if (options.diagnose) {
      writeDiagnostics(directory, config);
      return 0;
    }
  } catch (...) {
    failures.record("NEEDS_CONFIG",L"启动参数、配置目录或内置识别资源读取失败。",0);
    // CLI diagnosis must not leave an unattended message box on failure.
    if (!options.diagnose)
      MessageBoxW(nullptr, L"无法读取启动参数或配置目录。\n支持 --demo、--data-dir 路径、--diagnose。",
                  L"OverwatchHelper", MB_ICONERROR);
    return 1;
  }
  bool demo = options.demo && !SimpleEdition;
  Handle singleton(CreateMutexW(
      nullptr, TRUE, demo ? L"Local\\OverwatchHelper.Demo" : L"Local\\OverwatchHelper.Main"));
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBoxW(nullptr, L"助手已经在运行。", L"OverwatchHelper", MB_OK);
    return 0;
  }
  dpi = GetDpiForSystem();
  updateFonts();
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
  wc.lpfnWndProc=ui::paneProc;wc.lpszClassName=ui::PaneClass;
  RegisterClassW(&wc);
  RECT r{0,0,u(SimpleEdition ? 640 : 960),u(SimpleEdition ? 480 : 826)},work{};
  DWORD style=WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN;
  AdjustWindowRectExForDpi(&r,style,FALSE,0,dpi);
  SystemParametersInfoW(SPI_GETWORKAREA,0,&work,0);
  const int initialWidth=std::min(int(r.right-r.left),int(work.right-work.left)-u(24));
  const int initialHeight=std::min(int(r.bottom-r.top),int(work.bottom-work.top)-u(24));
  mainWindow = CreateWindowExW(0, AppClass,
                               SimpleEdition ? L"守望挂机 0.5.13 · 简易版" : (demo ? L"OverwatchHelper 0.5.13 · 开发版演示" : L"OverwatchHelper 0.5.13 · 开发版"),
                               style,work.left+(work.right-work.left-initialWidth)/2,
                               work.top+(work.bottom-work.top-initialHeight)/2,initialWidth,
                               initialHeight,nullptr,nullptr,h,nullptr);
  if (!mainWindow) return 1;
  dpi=GetDpiForWindow(mainWindow);updateFonts();
  if (SimpleEdition) windowLayout.attach(mainWindow,nullptr,dpi);
  else {
  auto pane=CreateWindowExW(WS_EX_CONTROLPARENT,ui::PaneClass,L"配置与页面校准",
      WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_CLIPCHILDREN,0,0,0,0,mainWindow,nullptr,h,&windowLayout);
  if (!pane) { DestroyWindow(mainWindow);return 1; }
  windowLayout.attach(mainWindow,pane,dpi);
  }
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
