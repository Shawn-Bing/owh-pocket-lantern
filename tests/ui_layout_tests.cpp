// Exercise the actual native controls without launching workers, hooks or a game.
#define wWinMain unusedApplicationEntry
#include "agent/main.cpp"
#undef wWinMain
#include <iostream>

void requireUi(bool value,const char *message) {
  if (!value) throw std::runtime_error(message);
}
RECT childBounds(HWND child,HWND parent) {
  RECT bounds{};GetWindowRect(child,&bounds);MapWindowPoints(nullptr,parent,reinterpret_cast<POINT*>(&bounds),2);return bounds;
}
void clientSize(int width,int height) {
  RECT outer{0,0,u(width),u(height)};
  AdjustWindowRectExForDpi(&outer,DWORD(GetWindowLongPtrW(mainWindow,GWL_STYLE)),FALSE,0,dpi);
  SetWindowPos(mainWindow,nullptr,-30000,-30000,outer.right-outer.left,outer.bottom-outer.top,SWP_NOACTIVATE|SWP_NOZORDER);
}
void preview(const wchar_t *name) {
  RECT client{};GetClientRect(mainWindow,&client);
  auto dc=CreateCompatibleDC(nullptr);BITMAPINFO info{};
  info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=client.right;
  info.bmiHeader.biHeight=-client.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
  void *pixels{};auto bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
  auto previous=SelectObject(dc,bitmap);
  requireUi(PrintWindow(mainWindow,dc,PW_CLIENTONLY)!=FALSE,"Render native UI preview");
  GdiFlush();Frame image{int(client.right),int(client.bottom),std::vector<std::uint32_t>(std::size_t(client.right)*client.bottom)};
  std::memcpy(image.pixels.data(),pixels,image.pixels.size()*4);
  std::filesystem::create_directories("ui-layout-test-data");
  requireUi(saveFrameBmp(image,(std::filesystem::path(L"ui-layout-test-data")/name).wstring()),"Save UI preview");
  SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
}
int main() {
  try {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    instance=GetModuleHandleW(nullptr);dpi=96;config.demo=true;
    config.machine=L"界面验证";config.gameExe=L"Overwatch.exe";config.roomTarget=L"最稳轮换版";
    updateFonts();
    WNDCLASSW root{};root.hInstance=instance;root.lpfnWndProc=mainProc;root.lpszClassName=AppClass;
    root.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);RegisterClassW(&root);
    root.lpfnWndProc=ui::paneProc;root.lpszClassName=ui::PaneClass;RegisterClassW(&root);
    mainWindow=CreateWindowExW(0,AppClass,L"UI fixture",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                              -30000,-30000,1000,900,nullptr,nullptr,instance,nullptr);
    auto pane=CreateWindowExW(WS_EX_CONTROLPARENT,ui::PaneClass,L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_CLIPCHILDREN,
                             0,0,0,0,mainWindow,nullptr,instance,&windowLayout);
    windowLayout.attach(mainWindow,pane,dpi);controls();refreshStatus();
    ShowWindow(mainWindow,SW_SHOWNOACTIVATE);UpdateWindow(mainWindow);
    requireUi(IsWindowEnabled(control(ForegroundUse)),"Explicit foreground-use request is available without an OCR match");
    requireUi(GetParent(control(GameExe))==pane && GetParent(control(Start))==mainWindow,
              "Content scrolls independently of persistent run controls");
    for (const auto &item:windowLayout.items()) requireUi(IsWindow(item.window),"All production controls are created");
    requireUi(GetNextDlgTabItem(mainWindow,nullptr,FALSE)==control(Start),"Tab navigation starts in the persistent run controls");
    requireUi(GetNextDlgTabItem(mainWindow,control(Diagnose),FALSE)==control(GameExe),
              "Tab enters settings after the header actions");
    clientSize(960,826);const auto normal=childBounds(control(Args),pane);
    preview(L"ui-default.bmp");
    SetWindowTextW(control(RoomTarget),L"保留的房间名称");
    SendMessageW(control(Dry),BM_SETCHECK,BST_CHECKED,0);EnableWindow(control(RoomTarget),FALSE);
    clientSize(1280,920);const auto wide=childBounds(control(Args),pane);
    requireUi(wide.right-wide.left>normal.right-normal.left+u(300),"Input fields expand with the window");
    const auto expandedList=childBounds(control(Steps),pane);
    requireUi(expandedList.bottom-expandedList.top>u(82),"Extra height expands the page list");
    requireUi(textOf(RoomTarget)==L"保留的房间名称" && checked(Dry) && !IsWindowEnabled(control(RoomTarget)),
              "Resize preserves text, checkboxes and running-state control locks");
    preview(L"ui-wide.bmp");
    clientSize(844,460);requireUi(windowLayout.maximum()>0,"Short windows expose a scrollable settings viewport");
    RECT client{};GetClientRect(mainWindow,&client);
    const auto start=childBounds(control(Start),mainWindow),footer=childBounds(control(OpenData),mainWindow);
    requireUi(start.top>=0 && start.bottom<u(ui::WindowLayout::ContentTop) && footer.bottom<=client.bottom,
              "Start and status actions remain visible at minimum height");
    windowLayout.scrollTo(windowLayout.maximum());
    RECT viewport{};GetClientRect(pane,&viewport);const auto last=childBounds(control(ImportProfile),pane);
    requireUi(last.top>=0 && last.bottom<=viewport.bottom,"Last settings action is reachable by scrolling");
    windowLayout.reveal(control(GameExe));const auto first=childBounds(control(GameExe),pane);
    requireUi(first.top>=0 && first.bottom<=viewport.bottom,"Keyboard focus can reveal an offscreen first field");
    windowLayout.reveal(control(ImportProfile));preview(L"ui-short.bmp");
    clientSize(960,826);requireUi(windowLayout.position()==0 && windowLayout.maximum()==0,
                                "Growing back clamps the old scroll position");
    dpi=144;windowLayout.setDpi(dpi);updateFonts();clientSize(844,460);
    const auto scaled=childBounds(control(Start),mainWindow);
    requireUi(scaled.right-scaled.left==u(115),"DPI changes rescale control geometry");
    LOGFONTW measured{};GetObjectW(font,sizeof(measured),&measured);
    requireUi(measured.lfHeight==-u(14),"DPI changes rescale the UI fonts");
    windowLayout.scrollTo(windowLayout.maximum());windowLayout.reveal(control(Launcher));
    const auto path=childBounds(control(Launcher),pane);GetClientRect(pane,&viewport);
    requireUi(path.top>=0 && path.bottom<=viewport.bottom,"Focus reveal also works at 150 percent DPI");
    preview(L"ui-dpi150.bmp");
    DestroyWindow(mainWindow);DeleteObject(font);DeleteObject(titleFont);
    std::cout<<"Actual Win32 controls: resizing, scroll reachability, stable header/footer, state preservation and DPI passed (no workers or input started)\n";
    return 0;
  } catch (const std::exception &error) { std::cerr<<"FAIL: "<<error.what()<<'\n';return 1; }
}
