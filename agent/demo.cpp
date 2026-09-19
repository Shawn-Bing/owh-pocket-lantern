#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <windows.h>
#include <windowsx.h>
int page = 0;
HFONT largeFont, normalFont;
void draw(HDC dc, RECT r, const wchar_t *text, int size, COLORREF color) {
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, color);
  SelectObject(dc, size ? largeFont : normalFont);
  DrawTextW(dc, text, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}
LRESULT CALLBACK proc(HWND w, UINT m, WPARAM p, LPARAM l) {
  switch (m) {
  case WM_PAINT: {
    PAINTSTRUCT ps{};
    auto dc = BeginPaint(w, &ps);
    RECT client{};
    GetClientRect(w, &client);
    auto bg = CreateSolidBrush(RGB(20, 31, 48));
    FillRect(dc, &client, bg);
    DeleteObject(bg);
    double sx = client.right / 960.0, sy = client.bottom / 540.0;
    auto rect = [&](int x, int y, int wi, int he) {
      return RECT{LONG(x * sx), LONG(y * sy), LONG((x + wi) * sx), LONG((y + he) * sy)};
    };
    draw(dc, rect(40, 12, 850, 26), L"OVERWATCH HELPER · 本地测试窗口", 0, RGB(122, 156, 179));
    const wchar_t *titles[] = {L"大厅 / MAIN MENU", L"自定义房间列表 / ROOM LIST",
                               L"目标房间已加入 / ROOM READY", L"连接中断 / CONNECTION LOST"};
    auto panel = rect(40, 75, 880, 95);
    auto brush = CreateSolidBrush(page == 2 ? RGB(26, 78, 63)
                                            : (page == 3 ? RGB(89, 45, 45) : RGB(35, 54, 82)));
    FillRect(dc, &panel, brush);
    DeleteObject(brush);
    draw(dc, rect(60, 92, 820, 60), titles[page], 1, RGB(233, 242, 249));
    const wchar_t *buttons[] = {L"进入自定义游戏  →", L"加入测试房：Torbjorn  →", L"模拟断线",
                                L"返回大厅（手动）"};
    auto button = rect(60, 220, 720, 68);
    brush = CreateSolidBrush(RGB(48, 103, 160));
    FillRect(dc, &button, brush);
    DeleteObject(brush);
    draw(dc, rect(83, 220, 670, 68), buttons[page], 0, RGB(255, 255, 255));
    draw(dc, rect(60, 346, 810, 32), L"校准：圈选上方独特标题区域，再标记中间按钮。", 0,
         RGB(168, 187, 205));
    bool shift = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
    draw(dc, rect(60, 400, 810, 40), shift ? L"Shift：正在按下 ✓" : L"Shift：未按下", 1,
         shift ? RGB(103, 226, 167) : RGB(188, 201, 212));
    draw(dc, rect(60, 481, 810, 26), L"F8：停止助手    F9：关闭测试窗口，验证自动重启", 0,
         RGB(122, 156, 179));
    EndPaint(w, &ps);
    return 0;
  }
  case WM_LBUTTONUP: {
    RECT r{};
    GetClientRect(w, &r);
    double x = GET_X_LPARAM(l) * 960.0 / r.right, y = GET_Y_LPARAM(l) * 540.0 / r.bottom;
    if (x >= 60 && x <= 780 && y >= 220 && y <= 288) {
      page = (page + 1) % 4;
      InvalidateRect(w, nullptr, FALSE);
    }
    return 0;
  }
  case WM_TIMER:
    InvalidateRect(w, nullptr, FALSE);
    return 0;
  case WM_KEYDOWN:
    if (p == VK_F9) {
      DestroyWindow(w);
      return 0;
    }
    if (p == VK_ESCAPE) {
      page = 0;
      InvalidateRect(w, nullptr, FALSE);
      return 0;
    }
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(w, m, p, l);
}
int WINAPI wWinMain(HINSTANCE h, HINSTANCE, PWSTR, int) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  largeFont = CreateFontW(-28, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY,
                          0, L"Microsoft YaHei UI");
  normalFont = CreateFontW(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                           CLEARTYPE_QUALITY, 0, L"Microsoft YaHei UI");
  WNDCLASSW c{};
  c.hInstance = h;
  c.lpfnWndProc = proc;
  c.lpszClassName = L"OWH-DemoGame";
  c.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  RegisterClassW(&c);
  RECT r{0, 0, 960, 540};
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  auto w = CreateWindowW(c.lpszClassName, L"OverwatchHelper 演示游戏", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr,
                         nullptr, h, nullptr);
  SetTimer(w, 1, 200, nullptr);
  ShowWindow(w, SW_SHOW);
  MSG m{};
  while (GetMessageW(&m, nullptr, 0, 0) > 0) {
    TranslateMessage(&m);
    DispatchMessageW(&m);
  }
  DeleteObject(largeFont);
  DeleteObject(normalFont);
  return 0;
}
