#include "agent/teaching.hpp"
#include <thread>
#include <iostream>

using namespace owh;
using namespace owh::teach;
namespace {
Options options;
HWND mainWindow{}, statusText{}, folderText{}, startButton{}, stopButton{};
HFONT font{};
HHOOK mouseHook{}, keyboardHook{};
HANDLE stopEvent{};
std::thread worker;
std::unique_ptr<Store> output;
EventQueue events;
std::atomic<std::shared_ptr<const Snapshot>> latest;
std::atomic<bool> active{}, stopRequested{}, queueOverflow{};
std::atomic<int> workerExitCode{};
std::atomic<bool> showStopMessage{};
std::atomic<unsigned> steps{}, skipped{}, inputEpoch{}, verifiedRooms{}, missingVerification{};
std::atomic<std::uint64_t> written{};
std::array<bool, 256> keyDown{};
Millis lastNavigationTick{}; // only touched by the hook/UI thread
std::mutex statusMutex;
std::wstring currentStatus{L"点击“开始录制”，再切回游戏或战网进行操作。"};
std::string requestedReason{"user_stop"}; // protected by statusMutex
bool startedOnce{};

void setStatus(const std::wstring &text) {
  std::lock_guard lock(statusMutex);
  currentStatus = text;
}
void requestStop(const char *reason) {
  { std::lock_guard lock(statusMutex); requestedReason = reason; }
  stopRequested.store(true);
  if (stopEvent) SetEvent(stopEvent);
}
void hookStop() {
  // A low-level hook must return promptly. The worker supplies the reason.
  stopRequested.store(true);
  if (stopEvent) SetEvent(stopEvent);
}
void queueAction(RecordedAction action, const POINT *point = nullptr) {
  if (!active.load() || stopRequested.load()) return;
  const auto now = GetTickCount64();
  const auto previousNavigation = lastNavigationTick;
  const auto previousEpoch = inputEpoch.load();
  if (action == RecordedAction::Click || action == RecordedAction::Enter || action == RecordedAction::Escape) {
    ++inputEpoch;
    lastNavigationTick = now;
  }
  auto before = latest.load();
  if (!before) { ++skipped; return; }
  Event event;
  event.before = std::move(before);
  event.action = action;
  event.tick = now;
  event.window = GetForegroundWindow();
  GetWindowThreadProcessId(event.window, &event.pid);
  if (!screenClientRect(event.window, event.client)) { ++skipped; return; }
  if (point) {
    event.screenPoint = *point;
    // Ignore title bars and a different window covering the click position.
    HWND under = WindowFromPoint(*point);
    if (!under || GetAncestor(under, GA_ROOT) != event.window) { ++skipped; return; }
  }
  Point normalized;
  if (event.before->inputEpoch != previousEpoch || event.before->tick < previousNavigation ||
      !eventMatches(event, normalized)) { ++skipped; return; }
  if (!events.push(std::move(event))) {
    ++skipped;
    queueOverflow = true;
    hookStop();
  }
}
LRESULT CALLBACK mouseProcedure(int code, WPARAM wp, LPARAM lp) {
  if (code == HC_ACTION && wp == WM_LBUTTONDOWN) {
    auto &event = *reinterpret_cast<const MSLLHOOKSTRUCT *>(lp);
    if (!(event.flags & (LLMHF_INJECTED | LLMHF_LOWER_IL_INJECTED)))
      queueAction(RecordedAction::Click, &event.pt);
  }
  return CallNextHookEx(mouseHook, code, wp, lp);
}
LRESULT CALLBACK keyboardProcedure(int code, WPARAM wp, LPARAM lp) {
  if (code == HC_ACTION) {
    const auto &event = *reinterpret_cast<const KBDLLHOOKSTRUCT *>(lp);
    if (!(event.flags & (LLKHF_INJECTED | LLKHF_LOWER_IL_INJECTED))) {
      // Ordinary text keys are neither examined nor retained.
      const auto key = event.vkCode;
      if (key == VK_RETURN || key == VK_ESCAPE || key == VK_F8 || key == VK_F9 || key == VK_F10) {
        if (wp == WM_KEYUP || wp == WM_SYSKEYUP) keyDown[key] = false;
        else if ((wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) && !keyDown[key]) {
          keyDown[key] = true;
          if (active.load()) {
            if (key == VK_F8) hookStop();
            else if (key == VK_F9) queueAction(RecordedAction::Observe);
            else if (key == VK_F10) queueAction(RecordedAction::Ready);
            else if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000) && !(GetAsyncKeyState(VK_MENU) & 0x8000))
              queueAction(key == VK_RETURN ? RecordedAction::Enter : RecordedAction::Escape);
          }
        }
      }
    }
  }
  return CallNextHookEx(keyboardHook, code, wp, lp);
}

Process foregroundTarget(std::wstring &executable, bool &launcher) {
  const auto window = GetForegroundWindow();
  if (!window) return {};
  DWORD pid{};
  GetWindowThreadProcessId(window, &pid);
  const auto name = processName(pid);
  launcher = _wcsicmp(name.c_str(), L"Battle.net.exe") == 0;
  if (!launcher && _wcsicmp(name.c_str(), options.gameExe.c_str())) return {};
  executable = launcher ? L"Battle.net.exe" : options.gameExe;
  // Use the same primary-window identity and creation-time check as the helper.
  auto process = findProcess(executable);
  if (process.pid != pid || process.window != window || !sameProcess(process, executable)) return {};
  return process;
}
std::wstring failureText(const std::string &reason) {
  if (reason == "size_limit") return L"已达到 256 MiB 上限，已有步骤已保留。";
  if (reason == "step_limit") return L"已达到 128 步上限，已有步骤已保留。";
  if (reason == "duration_limit") return L"已达到 10 分钟上限，录制已保存。";
  if (reason == "image_encode_failed") return L"截图编码失败，已有步骤已保留。";
  if (reason == "image_write_failed" || reason == "manifest_write_failed")
    return L"文件保存失败，已保存的步骤保留在下方目录。请检查磁盘空间和目录权限。";
  if (reason == "collector_exception") return L"录制异常，已保存的步骤保留在下方目录。";
  if (reason == "event_queue_full") return L"操作过快，录制已停止。已有步骤已保存，请每次页面显示后再操作。";
  return L"录制已保存。可在助手中选择此目录的 manifest.json 生成本地模型校准。";
}
bool recordPending(std::string &reason, std::vector<PendingVerification> &pending) {
  Event event;
  while (events.pop(event)) {
    Point click;
    if (!eventMatches(event, click)) { ++skipped; continue; }
    // The frame and event were validated before the input was delivered. Do
    // not replace that frame with a new screenshot of the destination page.
    if (!output->append(event)) { reason = output->error; return false; }
    steps.store(unsigned(output->size()));
    written.store(output->bytes());
    if (event.action == RecordedAction::Ready)
      pending.push_back(verificationFor(output->size() - 1, event));
    if (output->size() >= MaxSteps) { reason = "step_limit"; return false; }
  }
  return true;
}
void abandonVerification(std::vector<PendingVerification> &pending) {
  missingVerification.fetch_add(unsigned(pending.size()));
  pending.clear();
}
bool verifyPending(const Snapshot &shot, std::vector<PendingVerification> &pending, std::string &reason) {
  for (auto it = pending.begin(); it != pending.end();) {
    const auto now = GetTickCount64();
    if (stopRequested.load() || WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0) return true;
    if (now > it->deadline || it->inputEpoch != inputEpoch.load() ||
        it->process.pid != shot.process.pid || it->process.created != shot.process.created ||
        it->process.window != shot.process.window || !EqualRect(&it->client, &shot.client)) {
      ++missingVerification;
      it = pending.erase(it);
    } else if (verificationMatches(*it, shot, inputEpoch.load(), now)) {
      if (!output->attachVerification(it->step, shot)) { reason = output->error; return false; }
      ++verifiedRooms;
      written.store(output->bytes());
      it = pending.erase(it);
    } else ++it;
  }
  return true;
}
void record() {
  std::string reason = "user_stop";
  std::vector<PendingVerification> pending;
  const auto started = GetTickCount64();
  Millis nextCapture{};
  Capture capture;
  bool healthy = true;
  try {
    while (!stopRequested.load()) {
      const auto now = GetTickCount64();
      if (now - started >= MaxDuration) { reason = "duration_limit"; break; }
      // Poll as a fallback if another application owns a global F8 hotkey.
      if ((GetAsyncKeyState(VK_F8) & 0x8000) ||
          (stopEvent && WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0)) break;
      if (!recordPending(reason, pending)) { healthy = false; break; }
      if (now >= nextCapture) {
        nextCapture = now + 100;
        std::wstring executable;
        bool launcher{};
        const auto process = foregroundTarget(executable, launcher);
        if (!process.window) {
          latest.store({});
          abandonVerification(pending);
          setStatus(L"等待游戏或战网位于前台；切换到其他程序时暂停采集。" );
        } else {
          RECT client{};
          if (!screenClientRect(process.window, client) || !frameFits(client)) {
            latest.store({});
            abandonVerification(pending);
            setStatus(L"窗口大小不支持。请使用完整可见、像素数不超过 3840×2160 的客户区。" );
          } else {
            const auto epoch = inputEpoch.load();
            auto shot = std::make_shared<Snapshot>();
            shot->process = process;
            shot->executable = executable;
            shot->launcher = launcher;
            shot->client = client;
            shot->dpi = GetDpiForWindow(process.window) / 96.0;
            shot->inputEpoch = epoch;
            shot->tick = GetTickCount64();
            if (!capture.grab(process.window, shot->frame, false)) {
              latest.store({});
              abandonVerification(pending);
              setStatus(capture.failure().message());
            } else {
              RECT after{};
              if (epoch != inputEpoch.load() || !sameProcess(process, executable) ||
                  !screenClientRect(process.window, after) || !EqualRect(&client, &after)) {
                latest.store({});
                abandonVerification(pending);
              } else {
                if (!verifyPending(*shot, pending, reason)) { healthy = false; break; }
                latest.store(std::move(shot));
                if (!pending.empty())
                  setStatus(L"已标记目标房间，请保持当前页面约 1 秒，正在采集独立验证帧。" );
                else if (verifiedRooms.load())
                  setStatus(L"目标房间的两张截图已保存。可继续演示，或按 F8 完成录制。" );
                else if (missingVerification.load())
                  setStatus(L"房间标记缺少第二张有效截图。请回到目标房间按 F10，并保持页面约 1 秒。" );
                else setStatus(L"正在录制。每次页面显示稳定后再操作；到达目标房间后按 F10 标记。" );
              }
            }
          }
        }
      }
      if (WaitForSingleObject(stopEvent, 15) == WAIT_OBJECT_0) break;
    }
    stopRequested.store(true); // prevent hooks from appending during finalization
    latest.store({});
    if (healthy && reason != "step_limit") healthy = recordPending(reason, pending);
    if (healthy && reason == "user_stop") {
      std::lock_guard lock(statusMutex);
      reason = queueOverflow.load() ? "event_queue_full" : requestedReason;
    }
  } catch (...) {
    reason = "collector_exception";
    healthy = false;
  }
  stopRequested.store(true);
  latest.store({});
  abandonVerification(pending);
  // A failing manifest update must not discard the prior atomic manifest.
  if (healthy || reason == "size_limit" || reason == "step_limit") {
    try { if (!output->finish(reason)) reason = output->error; }
    catch (...) { reason = "manifest_write_failed"; }
  }
  workerExitCode = (reason == "image_encode_failed" || reason == "image_write_failed" ||
                    reason == "manifest_write_failed" || reason == "collector_exception" ||
                    reason == "event_queue_full") ? 1 : 0;
  showStopMessage = reason != "user_stop" && reason != "window_closed";
  setStatus(failureText(reason));
  active.store(false);
  PostMessageW(mainWindow, WM_APP + 1, 0, 0);
}
void begin() {
  if (active.load() || startedOnce) return;
  if (worker.joinable()) worker.join();
  Event discard;
  while (events.pop(discard)) {}
  try {
    output = std::make_unique<Store>(options.output, options.target);
  } catch (const std::exception &error) {
    MessageBoxW(mainWindow, (L"无法创建录制目录：" + wide(error.what())).c_str(), L"录制失败", MB_ICONERROR);
    return;
  }
  if (!mouseHook) mouseHook = SetWindowsHookExW(WH_MOUSE_LL, mouseProcedure, GetModuleHandleW(nullptr), 0);
  if (!keyboardHook) keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboardProcedure, GetModuleHandleW(nullptr), 0);
  if (!mouseHook || !keyboardHook) {
    output->finish("hook_unavailable");
    MessageBoxW(mainWindow, L"无法监听操作，录制未开始。请关闭后重新打开本工具。", L"录制失败", MB_ICONERROR);
    return;
  }
  startedOnce = true;
  queueOverflow = false;
  steps = 0;
  skipped = 0;
  verifiedRooms = 0;
  missingVerification = 0;
  written = 0;
  inputEpoch = 0;
  lastNavigationTick = 0;
  keyDown.fill(false);
  stopRequested = false;
  ResetEvent(stopEvent);
  { std::lock_guard lock(statusMutex); requestedReason = "user_stop"; }
  active = true;
  SetWindowTextW(folderText, (output->directory / L"manifest.json").c_str());
  setStatus(L"录制已开始。请切回游戏或战网，停稳后操作。" );
  worker = std::thread(record);
}
void refresh() {
  std::wstring text;
  { std::lock_guard lock(statusMutex); text = currentStatus; }
  text += L"\r\n已保存 " + std::to_wstring(steps.load()) + L" 步  /  " +
          std::to_wstring(written.load() / 1024 / 1024) + L" MiB  /  双帧房间 " + std::to_wstring(verifiedRooms.load()) + L" 组";
  if (skipped.load()) text += L"；未记录 " + std::to_wstring(skipped.load()) + L" 次（非目标窗口或截图未就绪）";
  SetWindowTextW(statusText, text.c_str());
  const bool busy = active.load() || (stopRequested.load() && worker.joinable());
  EnableWindow(startButton, !busy && !startedOnce);
  EnableWindow(stopButton, active.load() && !stopRequested.load());
}
LRESULT CALLBACK procedure(HWND window, UINT message, WPARAM wp, LPARAM lp) {
  if (message == WM_COMMAND) {
    if (LOWORD(wp) == 101) begin();
    else if (LOWORD(wp) == 102 && active.load()) requestStop("user_stop");
    else if (LOWORD(wp) == 103 && output)
      ShellExecuteW(window, L"open", output->directory.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return 0;
  }
  if (message == WM_TIMER) {
    // The parent can cancel while the user is still reading the start screen.
    if (!startedOnce && stopEvent && WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0) {
      DestroyWindow(window);
      return 0;
    }
    refresh();
    return 0;
  }
  if (message == WM_APP + 1) {
    if (worker.joinable()) worker.join();
    stopRequested = false;
    refresh();
    if (showStopMessage.load()) {
      std::wstring text;
      { std::lock_guard lock(statusMutex); text = currentStatus; }
      if (output) text += L"\r\n\r\n" + (output->directory / L"manifest.json").wstring();
      MessageBoxW(window, text.c_str(), L"演示录制已停止",
                  workerExitCode.load() ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    DestroyWindow(window);
    return 0;
  }
  if (message == WM_CLOSE) {
    if (active.load() || worker.joinable()) {
      requestStop("window_closed");
      setStatus(L"正在保存录制，完成后关闭。" );
    } else DestroyWindow(window);
    return 0;
  }
  if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(window, message, wp, lp);
}

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}
int selfTest() {
  try {
    auto snapshot = std::make_shared<Snapshot>();
    snapshot->process = {123, 456, reinterpret_cast<HWND>(INT_PTR(1))};
    snapshot->executable = L"Overwatch.exe";
    snapshot->client = {100, 200, 260, 290};
    snapshot->dpi = 1.25;
    snapshot->tick = 1000;
    snapshot->frame.width = 160;
    snapshot->frame.height = 90;
    snapshot->frame.pixels.resize(160 * 90);
    for (std::size_t i = 0; i < snapshot->frame.pixels.size(); ++i)
      snapshot->frame.pixels[i] = i % 2 ? 0x00a04020 : 0x002060a0;
    Event event{snapshot, RecordedAction::Click, snapshot->process.window, snapshot->process.pid,
                snapshot->client, {140, 245}, 1100};
    Point point;
    require(eventMatches(event, point) && point.x == .25 && point.y == .5, "Client coordinates must be normalized");
    auto invalid = event;
    invalid.tick = 999;
    require(!eventMatches(invalid, point), "A post-action screenshot must be rejected");
    invalid.tick = 1301;
    require(!eventMatches(invalid, point), "A stale pre-action screenshot must be rejected");
    invalid = event;
    ++invalid.client.left;
    require(!eventMatches(invalid, point), "Moved windows must be rejected");
    invalid = event;
    invalid.pid = 999;
    require(!eventMatches(invalid, point), "A different process must be rejected");
    invalid = event;
    invalid.screenPoint = {260, 245};
    require(!eventMatches(invalid, point), "Non-client clicks must be rejected");
    auto launcher = std::make_shared<Snapshot>(*snapshot);
    launcher->launcher = true;
    invalid = event;
    invalid.before = launcher;
    invalid.action = RecordedAction::Ready;
    require(!eventMatches(invalid, point), "A launcher must not be marked as the target room");
    EventQueue queue;
    require(queue.push(event) && queue.push(event) && queue.push(event) && !queue.push(event), "Event queue must be bounded");
    Event result;
    require(queue.pop(result) && result.before == snapshot, "The pre-action frame must survive queuing");

    const auto directory = sessionPath(options.output);
    Store store(directory, L"测试房间 \"最稳\"", MaxBytes, 4);
    require(store.append(event), "Click event must save");
    event.action = RecordedAction::Observe;
    require(store.append(event), "Observation must save");
    event.action = RecordedAction::Ready;
    require(store.append(event), "Explicit target-room marker must save");
    const auto pending = verificationFor(2, event);
    Snapshot second = *snapshot;
    second.tick = event.tick + 1000;
    second.frame.pixels[0] = 0x004080c0;
    require(verificationMatches(pending, second, 0, second.tick + 10), "A fresh delayed same-window verification frame must be accepted");
    auto wrong = second;
    --wrong.tick;
    require(!verificationMatches(pending, wrong, 0, second.tick + 10), "The verification image must wait a full second");
    wrong = second;
    ++wrong.process.created;
    require(!verificationMatches(pending, wrong, 0, second.tick + 10), "A restarted game must not verify the old room marker");
    wrong = second;
    ++wrong.client.left;
    require(!verificationMatches(pending, wrong, 0, second.tick + 10), "Moved or resized clients must not supply verification");
    require(!verificationMatches(pending, second, 1, second.tick + 10), "Manual navigation must invalidate delayed verification");
    require(!verificationMatches(pending, second, 0, pending.deadline + 1), "An expired verification must be rejected");
    require(store.attachVerification(2, second), "Independent Ready verification image must save");
    require(!store.attachVerification(0, second), "A click step must not accept room verification");
    require(store.append(event), "A room marker without a second frame must remain recoverable");
    require(!store.append(event) && store.error == "step_limit", "Step limit must stop before writing");
    require(store.finish("self_test"), "Manifest must finalize");
    std::ifstream file(directory / L"manifest.json", std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    Json json;
    std::string error;
    require(parseJson(buffer.str(), json, error), "Saved manifest must parse");
    const auto list = json.find("steps");
    require(list && list->items.size() == 4 && json.find("format")->text == "owh-teaching" &&
            json.find("version")->number == 1, "Manifest must meet the versioned import contract");
    require(list->items[0].find("click")->items[0].number == .25 &&
            list->items[0].find("frame_tick_ms")->number < list->items[0].find("tick_ms")->number &&
            list->items[2].find("action")->text == "ready", "Manifest must preserve pre-action geometry and ready confirmation");
    require(list->items[2].find("verify_image") && list->items[2].find("verify_image")->text == "verify-0003.png" &&
            !list->items[3].find("verify_image"), "Missing verification must remain visibly absent from the import contract");
    Gdiplus::Bitmap verified((directory / L"verify-0003.png").c_str());
    Gdiplus::Color verificationPixel;
    require(verified.GetLastStatus() == Gdiplus::Ok && verified.GetPixel(0, 0, &verificationPixel) == Gdiplus::Ok &&
            verificationPixel.GetR() == 0x40 && verificationPixel.GetG() == 0x80 && verificationPixel.GetB() == 0xc0,
            "Verification PNG must contain the later frame rather than duplicating the original");
    Gdiplus::Bitmap saved((directory / L"frame-0001.png").c_str());
    Gdiplus::Color pixel;
    require(saved.GetLastStatus() == Gdiplus::Ok && saved.GetWidth() == 160 && saved.GetHeight() == 90 &&
            saved.GetPixel(0, 0, &pixel) == Gdiplus::Ok && pixel.GetR() == 0x20 && pixel.GetG() == 0x60 && pixel.GetB() == 0xa0,
            "Saved lossless image must preserve the pre-action frame");
    Store capped(sessionPath(options.output), L"容量测试", 128 * 1024, 3);
    require(!capped.append(event) && capped.error == "size_limit" && capped.size() == 0, "Byte limit must preserve the previous manifest");
    std::cout << "Teaching recorder self-test passed: pre-action frame, identity, geometry, bounds, PNG, manifest and independent delayed verification.\n"
              << "Manifest: " << utf8(directory.wstring()) << "\\manifest.json\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Teaching recorder self-test failed: " << error.what() << '\n';
    return 1;
  }
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  int count{};
  auto argv = CommandLineToArgvW(GetCommandLineW(), &count);
  try {
    if (!argv) return 2;
    std::vector<std::wstring> args(argv, argv + count);
    LocalFree(argv);
    options = parseOptions(args);
  } catch (const std::exception &error) {
    MessageBoxW(nullptr, wide(error.what()).c_str(), L"录制参数无效", MB_ICONERROR);
    return 2;
  }
  Gdiplus::GdiplusStartupInput graphicsInput;
  ULONG_PTR graphics{};
  if (Gdiplus::GdiplusStartup(&graphics, &graphicsInput, nullptr) != Gdiplus::Ok) return 2;
  if (options.selfTest) {
    const auto result = selfTest();
    Gdiplus::GdiplusShutdown(graphics);
    return result;
  }
  Handle singleton(CreateMutexW(nullptr, FALSE, L"Local\\OverwatchHelper.Teaching"));
  if (!singleton.h || GetLastError() == ERROR_ALREADY_EXISTS) {
    if (auto existing = FindWindowW(L"OverwatchHelper.Teaching", nullptr)) {
      ShowWindow(existing, SW_RESTORE);
      SetForegroundWindow(existing);
    }
    MessageBoxW(nullptr, L"演示录制工具已经打开，请使用现有窗口。", L"OverwatchHelper", MB_OK);
    Gdiplus::GdiplusShutdown(graphics);
    return 0;
  }
  Handle stop(CreateEventW(nullptr, TRUE, FALSE, StopEvent));
  stopEvent = stop.h;
  if (!stopEvent) { Gdiplus::GdiplusShutdown(graphics); return 2; }
  ResetEvent(stopEvent);
  const auto dpi = GetDpiForSystem();
  auto scale = [dpi](int n) { return MulDiv(n, dpi, 96); };
  WNDCLASSW cls{};
  cls.hInstance = instance;
  cls.lpszClassName = L"OverwatchHelper.Teaching";
  cls.lpfnWndProc = procedure;
  cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&cls)) { Gdiplus::GdiplusShutdown(graphics); return 2; }
  RECT rect{0, 0, scale(720), scale(390)};
  const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRectExForDpi(&rect, style, FALSE, 0, dpi);
  mainWindow = CreateWindowW(cls.lpszClassName, L"OverwatchHelper · 操作演示录制", style,
      CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
      nullptr, nullptr, instance, nullptr);
  if (!mainWindow) { Gdiplus::GdiplusShutdown(graphics); return 2; }
  font = CreateFontW(-scale(15), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    0, 0, CLEARTYPE_QUALITY, 0, L"Microsoft YaHei UI");
  auto add = [&](const wchar_t *kind, const wchar_t *text, int id, int x, int y, int width, int height, DWORD extra = 0) {
    const auto control = CreateWindowW(kind, text, WS_CHILD | WS_VISIBLE | extra,
        scale(x), scale(y), scale(width), scale(height), mainWindow,
        reinterpret_cast<HMENU>(INT_PTR(id)), instance, nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return control;
  };
  add(L"STATIC", L"实操一遍：启动 / 进入大厅 / 浏览房间 / 加入目标房间", 0, 22, 20, 675, 28);
  statusText = add(L"STATIC", currentStatus.c_str(), 0, 22, 61, 675, 65);
  startButton = add(L"BUTTON", L"开始录制", 101, 22, 140, 145, 36, WS_TABSTOP);
  stopButton = add(L"BUTTON", L"完成并关闭（F8）", 102, 181, 140, 180, 36, WS_TABSTOP);
  add(L"BUTTON", L"打开录制目录", 103, 375, 140, 160, 36, WS_TABSTOP);
  add(L"STATIC", L"清单路径（结束后自动关闭）：", 0, 22, 193, 675, 22);
  folderText = add(L"EDIT", options.output.c_str(), 0, 22, 219, 675, 43,
                   ES_MULTILINE | ES_READONLY | WS_BORDER);
  add(L"STATIC", L"F9：采集页面；F10：标记目标房间后保持页面约 1 秒；F8：停止保存。\r\n"
                  L"只记录游戏 / 战网客户区、左键点击及 Enter / Esc；不记录普通文本键。\r\n"
                  L"保持窗口完整可见。最多 10 分钟、128 步或 256 MiB；关闭窗口也会保存。\r\n"
                  L"本工具只录制，不发送操作；生成校准后由助手导入使用。", 0, 22, 280, 675, 93);
  SetTimer(mainWindow, 1, 250, nullptr);
  EnableWindow(stopButton, FALSE);
  ShowWindow(mainWindow, SW_SHOW);
  UpdateWindow(mainWindow);
  MSG message{};
  while (GetMessageW(&message, nullptr, 0, 0) > 0) {
    if (!IsDialogMessageW(mainWindow, &message)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
  }
  requestStop("window_closed");
  if (worker.joinable()) worker.join();
  if (mouseHook) UnhookWindowsHookEx(mouseHook);
  if (keyboardHook) UnhookWindowsHookEx(keyboardHook);
  DeleteObject(font);
  Gdiplus::GdiplusShutdown(graphics);
  return workerExitCode.load();
}
