#define OWH_SIMPLE_EDITION 1
#define wWinMain unusedApplicationEntry
#include "agent/main.cpp"
#undef wWinMain
#include <iostream>
void requireSimple(bool ok,const char *why) {if(!ok) throw std::runtime_error(why);}
RECT bounds(HWND h) {RECT r{};GetWindowRect(h,&r);MapWindowPoints(nullptr,mainWindow,reinterpret_cast<POINT*>(&r),2);return r;}
void sizeSimple(int width,int height) {
 RECT r{0,0,u(width),u(height)};AdjustWindowRectExForDpi(&r,DWORD(GetWindowLongPtrW(mainWindow,GWL_STYLE)),FALSE,0,dpi);
 SetWindowPos(mainWindow,nullptr,-30000,-30000,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE|SWP_NOZORDER);resizeSimple();
}
void renderSimple(const wchar_t *name) {
 RECT r{};GetClientRect(mainWindow,&r);auto dc=CreateCompatibleDC(nullptr);BITMAPINFO info{};
 info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;
 info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;void *pixels{};
 auto bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);auto old=SelectObject(dc,bitmap);
 requireSimple(PrintWindow(mainWindow,dc,PW_CLIENTONLY)!=FALSE,"Render simple UI");GdiFlush();
 Frame frame{int(r.right),int(r.bottom),std::vector<std::uint32_t>(std::size_t(r.right)*r.bottom)};
 std::memcpy(frame.pixels.data(),pixels,frame.pixels.size()*4);
 requireSimple(saveFrameBmp(frame,(std::filesystem::path(directory)/name).wstring()),"Save preview");
 SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
}
int main() {
 try {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);instance=GetModuleHandleW(nullptr);dpi=96;
  directory=std::filesystem::absolute(std::filesystem::path(L"simple-ui-test-data")/std::to_wstring(GetCurrentProcessId())).wstring();
  auto legacy=directory+L"\\developer";std::filesystem::create_directories(legacy);std::filesystem::create_directories(directory);
  writeIni(legacy+L"\\settings.ini",L"agent",L"neac_path",L"C:\\fixture\\OWNeacClient.exe");
  writeIni(legacy+L"\\settings.ini",L"agent",L"launcher_path",L"C:\\fixture\\Battle.net.exe");
  writeIni(legacy+L"\\settings.ini",L"agent",L"dry_run",L"1");
  config=loadSimpleConfig(directory,legacy);failures.open(directory);
  requireSimple(!config.dryRun && config.semanticRecognition && config.cnPreset && config.holdShift && config.autoRecover,
     "Simple edition defaults to functional built-in automation");
  requireSimple(config.steps.empty() && config.server.empty() && config.token.empty(),"No manual calibration or telemetry is required");
  requireSimple(embeddedAsset(201).size()>1000 && !embeddedAsset(202).empty(),"OCR and profile are embedded in this EXE");
  auto script=embeddedOcrScript(directory);requireSimple(std::filesystem::is_regular_file(script),"Embedded script materializes in data directory");
  auto again=embeddedOcrScript(directory);requireSimple(again==script,"Embedded script path is content-addressed and stable");
  updateFonts();WNDCLASSW wc{};wc.hInstance=instance;wc.lpfnWndProc=mainProc;wc.lpszClassName=AppClass;
  wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);RegisterClassW(&wc);
  mainWindow=CreateWindowExW(0,AppClass,L"Simple fixture",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,-30000,-30000,700,540,nullptr,nullptr,instance,nullptr);
  windowLayout.attach(mainWindow,nullptr,dpi);controls();refreshStatus();ShowWindow(mainWindow,SW_SHOWNOACTIVATE);UpdateWindow(mainWindow);
  for(int id:{Start,Stop,RestartNow,ForegroundUse,ModeBox,FailureList,OpenFailures}) requireSimple(IsWindow(control(id)),"Every basic operation exists");
  for(int id:{GameExe,Record,Semantic,Learning,ImportProfile,RoomTarget,Dry,Save}) requireSimple(!control(id),"Advanced controls are absent, not merely hidden");
  requireSimple(IsWindowEnabled(control(ForegroundUse)),"Foreground-use button is clickable without OCR evidence");
  requireSimple(SendMessageW(control(ModeBox),CB_GETCOUNT,0,0)==4,"Foreground/background custom and deathmatch modes are selectable");
  SendMessageW(control(ModeBox),CB_SETCURSEL,3,0);requireSimple(syncConfig(),"Mode selection saves without advanced controls");
  requireSimple(config.mode==WorkflowMode::Deathmatch && simpleBackground,"Selected backend is applied");
  requireSimple(readIni(legacy+L"\\settings.ini",L"agent",L"dry_run")==L"1","Developer configuration is untouched");
  requireSimple(simpleModeIndex(directory,config)==3,"Simple mode survives reload");
  auto savedConfig=config;auto originalForeground=GetForegroundWindow();
  WNDCLASSW fake{};fake.hInstance=instance;fake.lpfnWndProc=DefWindowProcW;fake.lpszClassName=L"OWH.SimpleFakeGame";RegisterClassW(&fake);
  auto fakeWindow=CreateWindowW(fake.lpszClassName,L"Isolated game fixture",WS_OVERLAPPEDWINDOW,-30000,-30000,200,100,nullptr,nullptr,instance,nullptr);
  ShowWindow(fakeWindow,SW_SHOWNOACTIVATE);config.gameExe=processName(GetCurrentProcessId());config.gameClass=fake.lpszClassName;
  generation=42;running=true;input.enable(true,42);
  const auto observerTicket=matchObserver.begin(42,config.mode,GetTickCount64());
  SendMessageW(mainWindow,WM_COMMAND,ForegroundUse,0);
  requireSimple(foregroundUse && backgroundInput.target().window==fakeWindow,"Explicit button starts on verified fixture without any OCR observations");
  requireSimple(GetForegroundWindow()==originalForeground,"Starting foreground-use does not activate fixture");
  const auto boundObservation=backgroundInput.target();
  ShowWindow(fakeWindow,SW_MINIMIZE);
  Process notYetScanned;
  {
    MatchObservationTick tick{matchObserver,observerTicket,notYetScanned,foregroundUse};
    tick.target(boundObservation);
  }
  const auto backgroundObservation=matchObserver.snapshot(GetTickCount64());
  requireSimple(backgroundObservation.game==matchProcess(boundObservation) && backgroundObservation.window.minimized &&
      backgroundObservation.window.backgroundInput && backgroundObservation.state==MatchState::Unverified,
      "Background observation pairs the verified bound PID with its minimized window even before worker scan");
  setRunning(false);requireSimple(!foregroundUse,"Stop exits background mode");
  const auto stoppedObservation=matchObserver.snapshot(GetTickCount64());
  requireSimple(!stoppedObservation.running && stoppedObservation.state==MatchState::Unverified &&
      stoppedObservation.automation==AutomationState::STOPPED,"Actual UI stop immediately revokes observer without claiming game exit");
  requireSimple(!matchObserver.automation(observerTicket,AutomationState::ACTIVE),"An actual UI stop rejects old worker status");
  DestroyWindow(fakeWindow);config=savedConfig;
  running=true;publish(AutomationState::INPUT_BLOCKED,L"fixture 失败：目标权限不足",1234);auto revision=failures.snapshot().first;
  publish(AutomationState::INPUT_BLOCKED,L"fixture 失败：目标权限不足",1234);
  requireSimple(failures.snapshot().first==revision,"Repeated identical status is not duplicated in failure log");
  refreshStatus();requireSimple(textOf(FailureList).find(L"1234")!=std::wstring::npos && textOf(FailureList).find(L"目标权限不足")!=std::wstring::npos,
     "Failure UI contains PID and concrete reason");
  running=false;FailureLog reloaded;reloaded.open(directory);requireSimple(reloaded.snapshot().second.find(L"fixture")!=std::wstring::npos,"Failure records survive restart");
  requireSimple(!failureState("RECOVERING",L"手动请求：重启") && !failureState("WAITING_FOCUS",L"等待") &&
     !failureState("UNKNOWN",L"未匹配页面，等待加载"),"Manual restart and normal waiting are not false failures");
  failures.open(directory+L"\\preview");
  publish(AutomationState::STOPPED,L"选择挂机模式后开始；已在局内时可直接点前台使用。");refreshStatus();
  sizeSimple(640,480);renderSimple(L"simple-default.bmp");
  for(auto size: {std::pair{640,400},std::pair{1000,720}}) {
   sizeSimple(size.first,size.second);RECT client{};GetClientRect(mainWindow,&client);
   for(int id:{Start,Stop,RestartNow,ForegroundUse,FailureList,OpenFailures}) {auto r=bounds(control(id));requireSimple(r.left>=0 && r.top>=0 && r.right<=client.right && r.bottom<=client.bottom,"Simple controls fit window at each size");}
   requireSimple(bounds(control(ForegroundUse)).left>=bounds(control(RestartNow)).right,"Action buttons do not overlap");
  }
  dpi=144;windowLayout.setDpi(dpi);updateFonts();sizeSimple(640,400);renderSimple(L"simple-dpi150.bmp");
  requireSimple(bounds(control(ModeHeading)).top>=bounds(control(StatusTitle)).bottom && bounds(control(ModeHeading)).right-bounds(control(ModeHeading)).left==u(76),"Mode label rescales and stays below title");
  requireSimple(bounds(control(ForegroundUse)).bottom<=bounds(control(StatusDetail)).top,"DPI-scaled buttons remain separate from detail");
  DestroyWindow(mainWindow);DeleteObject(font);DeleteObject(titleFont);
  std::cout<<"Simple UI, independent settings, embedded assets, persistent failure records and DPI passed; no workers, game input or game restart\n";return 0;
 } catch(const std::exception &e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
