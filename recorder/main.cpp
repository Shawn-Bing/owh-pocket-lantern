#include "recorder/collectors.hpp"
#include <condition_variable>

using namespace owh;
using namespace owh::rec;
namespace {
constexpr wchar_t StopEvent[] = L"Local\\OverwatchHelper.Recorder.Stop";
Options options;
HWND window{}, statusText{}, folderText{}, startButton{}, stopButton{}, shotCheckbox{};
HFONT font{};
std::unique_ptr<Store> output;
std::thread worker;
HANDLE stopSignal{};
std::atomic<bool> active{};
std::atomic<unsigned> shots{}, samples{}, marks{};
std::mutex statusMutex;
std::wstring currentStatus=L"准备就绪。记录工具不会发送 Shift 或自动恢复游戏。";
void status(const std::wstring &s) {std::lock_guard lock(statusMutex);currentStatus=s;}
bool stopped() {return WaitForSingleObject(stopSignal,0)==WAIT_OBJECT_0;}
void run() {
  auto &store=*output;
  const auto started=GetTickCount64();
  auto startedUtc=utc();
  std::atomic<bool> finished{};
  std::atomic<std::uint64_t> keyEdges{},lastKey{},keyPolls{},keyGaps{};
  std::atomic<bool> burst{};
  std::thread keys([&] {
    bool down=false;auto last=GetTickCount64();
    while(!finished && !stopped()) {
      auto now=GetTickCount64(); if(now-last>100)++keyGaps;last=now;
      bool current=(GetAsyncKeyState(VK_LSHIFT)&0x8000)!=0;
      if(current){lastKey=now;if(!down)++keyEdges;}
      down=current;++keyPolls;
      if(WaitForSingleObject(stopSignal,10)==WAIT_OBJECT_0)break;
    }
  });
  std::thread network([&] {
    struct History {unsigned failed{};bool everGood{};};
    std::map<std::string,History> history;
    HANDLE icmp=IcmpCreateFile();
    if(icmp==INVALID_HANDLE_VALUE){store.event("icmp_unavailable","\"error\":"+std::to_string(GetLastError()));return;}
    try {
      Millis nextRoutes{};Routes route;std::string previousRoutes;
      while(!finished && !stopped()) {
        auto start=GetTickCount64();
        if(start>=nextRoutes) {
          route=routes();nextRoutes=start+30000;
          if(route.json!=previousRoutes){store.event("route_snapshot","\"adapters\":"+route.json+",\"error\":"+std::to_string(route.error));previousRoutes=route.json;}
        }
        std::vector<std::pair<std::string,const char *>> targets;
        for(auto &ip:route.gateways)targets.emplace_back(ip,"gateway");
        targets.emplace_back("223.5.5.5","public_reference");
        targets.emplace_back("119.29.29.29","public_reference");
        if(options.probes) for(auto &[ip,role]:targets) {
          if(finished||stopped())break;
          auto result=ping(store,icmp,ip,role);auto &h=history[ip];
          if(result==IP_SUCCESS){
            if(h.failed>=3 && h.everGood)store.event("probe_recovered","\"target\":"+jsonString(ip));
            h.failed=0;h.everGood=true;
          } else if(++h.failed==3 && h.everGood) {
            store.event("probe_loss_candidate","\"target\":"+jsonString(ip)+",\"role\":"+jsonString(role));burst=true;
          }
        }
        auto spent=GetTickCount64()-start;
        if(WaitForSingleObject(stopSignal,DWORD(spent<5000?5000-spent:1))==WAIT_OBJECT_0)break;
      }
    }catch(...){store.event("network_collector_failed");}
    IcmpCloseHandle(icmp);
  });
  unsigned missingScreens{};std::string reason="user_stop";
  try {
    TIME_ZONE_INFORMATION zone{};auto zoneState=GetTimeZoneInformation(&zone);
    auto offset=-(zone.Bias+(zoneState==TIME_ZONE_ID_DAYLIGHT?zone.DaylightBias:zone.StandardBias));
    store.text("session.json","{\"version\":"+jsonString(RecorderVersion)+",\"started_utc\":"+jsonString(startedUtc)+
      ",\"local_utc_offset_minutes\":"+std::to_string(offset)+
      ",\"started_tick_ms\":"+std::to_string(started)+",\"max_seconds\":"+std::to_string(options.seconds)+
      ",\"screenshots\":"+(options.screenshots?"true":"false")+",\"source_logs\":"+(options.sources?"true":"false")+
      ",\"active_icmp_probes\":"+(options.probes?"true":"false")+",\"size_cap_bytes\":536870912,\"auto_recovery\":false,\"sends_keys\":false}\n",false);
    store.event("recording_started");
    Capture capture;Frame frame;Tailer tailer;SystemEvents system;
    auto roots=sourceRoots(options.demo);
    std::string previousGame;bool gameObserved=false;
    Millis nextSample{},nextImage{},nextSlow{},burstUntil{},lastSample{};
    unsigned recordedMarks{};
    while(!stopped()) {
      auto now=GetTickCount64();
      if(now-started>=Millis(options.seconds)*1000){reason="duration_limit";break;}
      if(store.capped){reason="size_limit";break;}
      if(store.failed){reason="write_error";break;}
      if(marks.load()!=recordedMarks){recordedMarks=marks.load();store.event("user_marker");burst=true;}
      if(burst.exchange(false)){burstUntil=now+60000;nextImage=0;}
      if(now>=nextSample) {
        nextSample=now+5000;
        if(lastSample && now-lastSample>10000)store.event("sampling_gap","\"gap_ms\":"+std::to_string(now-lastSample));
        lastSample=now;
        auto all=processes(options.demo);
        if(all.ok){
          if(gameObserved && all.gameIdentity!=previousGame){store.event("game_process_changed","\"before\":"+jsonString(previousGame)+",\"after\":"+jsonString(all.gameIdentity));burstUntil=now+60000;nextImage=0;}
          previousGame=all.gameIdentity;gameObserved=true;
        }
        auto game=findProcess(options.demo?L"owh-demo.exe":L"Overwatch.exe",options.demo?L"OWH-DemoGame":L"TankWindowClass");
        bool desktop=interactiveDesktop();
        LASTINPUTINFO lastInput{};lastInput.cbSize=sizeof(lastInput);
        bool inputTimeOk=desktop && GetLastInputInfo(&lastInput);
        DWORD foregroundPid{};GetWindowThreadProcessId(GetForegroundWindow(),&foregroundPid);
        MEMORYSTATUSEX mem{};mem.dwLength=sizeof(mem);bool memoryOk=GlobalMemoryStatusEx(&mem);
        FILETIME idle{},kernel{},user{};bool timeOk=GetSystemTimes(&idle,&kernel,&user);
        RECT client{};GetClientRect(game.window,&client);
        store.text("samples.jsonl","{"+stamp()+",\"process_snapshot_ok\":"+(all.ok?"true":"false")+
          ",\"processes\":"+all.json+",\"game_window\":"+(game.window?"true":"false")+
          ",\"game_minimized\":"+(game.window&&IsIconic(game.window)?"true":"false")+
          ",\"game_foreground\":"+(foreground(game.window)?"true":"false")+
          ",\"game_responsive\":"+(game.window?(responsive(game.window)?"true":"false"):"null")+
          ",\"client_width\":"+std::to_string(client.right)+",\"client_height\":"+std::to_string(client.bottom)+
          ",\"interactive_desktop\":"+(desktop?"true":"false")+
          ",\"system_input_idle_ms\":"+(inputTimeOk?std::to_string(DWORD(GetTickCount()-lastInput.dwTime)):"null")+
          ",\"foreground_process\":"+jsonString(processName(foregroundPid))+
          ",\"left_shift_observed_edges\":"+std::to_string(keyEdges.load())+
          ",\"left_shift_last_seen_tick_ms\":"+std::to_string(lastKey.load())+
          ",\"key_poll_count\":"+std::to_string(keyPolls.load())+
          ",\"key_poll_gaps_over_100ms\":"+std::to_string(keyGaps.load())+
          ",\"caps_lock_getkeystate\":"+((GetKeyState(VK_CAPITAL)&1)?"true":"false")+
          ",\"memory_load_pct\":"+(memoryOk?std::to_string(mem.dwMemoryLoad):"null")+
          ",\"available_memory\":"+(memoryOk?std::to_string(mem.ullAvailPhys):"null")+
          ",\"system_idle_ms_total\":"+(timeOk?std::to_string(ticks(idle)/10000):"null")+
          ",\"system_kernel_ms_total\":"+(timeOk?std::to_string(ticks(kernel)/10000):"null")+
          ",\"system_user_ms_total\":"+(timeOk?std::to_string(ticks(user)/10000):"null")+
          ",\"interfaces\":"+interfaces()+",\"endpoints\":"+endpoints(all.ids)+"}\n");
        ++samples;
      }
      if(options.screenshots && now>=nextImage) {
        nextImage=now+(now<burstUntil?5000:30000);
        auto p=findProcess(options.demo?L"owh-demo.exe":L"Overwatch.exe",options.demo?L"OWH-DemoGame":L"TankWindowClass");
        std::string cause;
        CaptureFailure detail;
        if(!p.window)cause="no_game_window";
        else if(!capture.grab(p.window,frame,true)){detail=capture.failure();cause=detail.code;}
        else if(!sameProcess(p,options.demo?L"owh-demo.exe":L"Overwatch.exe"))cause="process_changed";
        else {
          auto file="screens/"+std::to_string(now)+".jpg";
          if(saveJpeg(store,frame,file)){
            store.text("screens.jsonl","{"+stamp()+",\"file\":"+jsonString(file)+",\"pid\":"+std::to_string(p.pid)+
              ",\"width\":"+std::to_string(frame.width)+",\"height\":"+std::to_string(frame.height)+"}\n");++shots;
          } else cause="jpeg_or_write_failed";
        }
        if(!cause.empty()){
          ++missingScreens;
          store.text("screens.jsonl","{"+stamp()+",\"skipped\":"+jsonString(cause)+
            ",\"windows_error\":"+std::to_string(detail.windowsError)+
            ",\"blocker_pid\":"+std::to_string(detail.blockerPid)+
            ",\"blocker_process\":"+jsonString(utf8(detail.blockerProcess))+"}\n");
          std::wstring message = detail.code!="ok" ? detail.message() :
            cause=="no_game_window" ? L"未找到游戏窗口。" :
            cause=="process_changed" ? L"游戏进程已变化，等待下次截图。" : L"截图编码或保存失败，请检查磁盘空间。";
          status(L"文字与网络仍在记录；本次截图跳过："+message);
        }
        else status(L"正在记录：游戏截图、进程、网络与相关日志。");
      } else if(!options.screenshots)status(L"正在记录文字与网络数据；截图已关闭。");
      if(now>=nextSlow) {
        nextSlow=now+60000;
        ULARGE_INTEGER free{};
        if(GetDiskFreeSpaceExW(store.directory.c_str(),&free,nullptr,nullptr) && free.QuadPart<256ULL*1024*1024){reason="low_disk_space";break;}
        system.collect(store);
        if(options.sources)collectSources(store,tailer,roots);
      }
      if(WaitForSingleObject(stopSignal,100)==WAIT_OBJECT_0)break;
    }
    if(reason=="user_stop" && !store.capped && !store.failed) {
      system.collect(store);
      if(options.sources)collectSources(store,tailer,roots);
    }
  }catch(const std::exception &e){reason="collector_exception";store.event("collector_exception","\"message\":"+jsonString(e.what()));}
  finished=true;SetEvent(stopSignal);keys.join();network.join();
  store.event("recording_stopped","\"reason\":"+jsonString(reason));
  // Small completion manifest remains writable when the bounded data budget is exhausted.
  try {
    std::ofstream f(store.directory/L"summary.json",std::ios::binary);
    f.exceptions(std::ios::badbit|std::ios::failbit);
    f<<"{\"started_utc\":"<<jsonString(startedUtc)<<",\"stopped_utc\":"<<jsonString(utc())
     <<",\"reason\":"<<jsonString(reason)<<",\"samples\":"<<samples.load()<<",\"screenshots\":"<<shots.load()
     <<",\"screenshots_skipped\":"<<missingScreens<<",\"data_bytes\":"<<store.bytes()
     <<",\"capped\":"<<(store.capped?"true":"false")<<",\"write_failed\":"<<(store.failed?"true":"false")<<"}\n";
    f.close();
    std::ofstream readme(store.directory/L"README.txt",std::ios::binary);
    readme.exceptions(std::ios::badbit|std::ios::failbit);
    readme<<"Overwatch overnight recording\nStart: "<<startedUtc<<"\nStop: "<<utc()<<"\nReason: "<<reason
      <<"\nOpen summary.json first. Inspect timeline.jsonl and screens.jsonl around the disconnect.\n"
      <<"samples.jsonl: process CPU/memory, interfaces, TCP peers and UDP local bindings, sampled Shift/CapsLock.\n"
      <<"probes.jsonl: gateway/public ICMP references, not the game server path or game packet loss.\n"
      <<"windows-events.jsonl: selected Windows event XML, original event time is inside the XML.\n"
      <<"sources.jsonl maps preserved source log segments to sources/*.log. Initial tails and skipped bytes are recorded.\n"
      <<"All JSON timestamps are UTC; tick_ms is monotonic uptime. Screenshots contain only a verified unobscured game client.\n"
      <<"Missing screenshots and unavailable data are explicit gaps, not evidence of a healthy game.\n"
      <<"No packet payloads, copied configuration files, memory dumps, general keyboard text, or desktop screenshots collected.\n"
      <<"Original logs and screens may contain account/player names, authentication-related metadata and network addresses. Keep the folder private.\n";
    readme.close();
  }catch(...){store.failed=true;reason="finalize_write_error";}
  status(reason=="user_stop"||reason=="duration_limit"?L"记录已停止，日志已保留。点击“打开记录目录”。"
    :L"记录已停止（容量、磁盘或采集错误），原因见 summary.json。已有日志保留。");
  active=false;
}
void begin() {
  if(active)return;if(worker.joinable())worker.join();
  options.screenshots=SendMessageW(shotCheckbox,BM_GETCHECK,0,0)==BST_CHECKED;
  try {output=std::make_unique<Store>(sessionPath(options.output));}
  catch(...){MessageBoxW(window,L"无法创建记录目录，请解压到可写的位置。",L"记录失败",MB_ICONERROR);return;}
  shots=0;samples=0;marks=0;ResetEvent(stopSignal);active=true;
  SetWindowTextW(folderText,output->directory.c_str());status(L"正在开始记录。请切回游戏，保持游戏窗口可见。");
  worker=std::thread(run);
}
LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM wp,LPARAM lp) {
  if(m==WM_COMMAND) {
    if(LOWORD(wp)==101)begin();
    if(LOWORD(wp)==102 && active){status(L"正在保存最后一批日志，请稍候。");SetEvent(stopSignal);}
    if(LOWORD(wp)==103 && output)ShellExecuteW(w,L"open",output->directory.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
    if(LOWORD(wp)==104 && active)++marks;
    return 0;
  }
  if(m==WM_TIMER) {
    std::wstring text;{std::lock_guard lock(statusMutex);text=currentStatus;}
    text+=L"\r\n样本 "+std::to_wstring(samples.load())+L"   截图 "+std::to_wstring(shots.load());
    if(output)text+=L"   已写入 "+std::to_wstring(output->bytes()/1024/1024)+L" MiB";
    SetWindowTextW(statusText,text.c_str());EnableWindow(startButton,!active);EnableWindow(stopButton,active);
    EnableWindow(shotCheckbox,!active);return 0;
  }
  if(m==WM_HOTKEY && active){++marks;return 0;}
  if(m==WM_CLOSE){SetEvent(stopSignal);ShowWindow(w,SW_HIDE);DestroyWindow(w);return 0;}
  if(m==WM_DESTROY){UnregisterHotKey(w,1);PostQuitMessage(0);return 0;}
  return DefWindowProcW(w,m,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  int argc{};auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);
  try {if(!argv)return 2;std::vector<std::wstring> args(argv,argv+argc);LocalFree(argv);options=parse(args);}
  catch(...){return 2;}
  if(options.stop){Handle signal(OpenEventW(EVENT_MODIFY_STATE,FALSE,StopEvent));return signal.h&&SetEvent(signal.h)?0:1;}
  Handle singleton(options.headless?nullptr:CreateMutexW(nullptr,FALSE,L"Local\\OverwatchHelper.Recorder"));
  if(!options.headless && GetLastError()==ERROR_ALREADY_EXISTS){MessageBoxW(nullptr,L"记录工具已经打开，请使用现有窗口。",L"OverwatchHelper",MB_OK);return 0;}
  Handle signal(CreateEventW(nullptr,TRUE,FALSE,options.headless?nullptr:StopEvent));stopSignal=signal.h;
  if(!stopSignal)return 2;
  WSADATA socketData{};int sockets=WSAStartup(MAKEWORD(2,2),&socketData);
  Gdiplus::GdiplusStartupInput graphicsInput;ULONG_PTR graphics{};
  auto graphicsStatus=Gdiplus::GdiplusStartup(&graphics,&graphicsInput,nullptr);
  if(sockets || graphicsStatus!=Gdiplus::Ok)return 2;
  if(options.headless) {
    output=std::make_unique<Store>(sessionPath(options.output));active=true;run();
    Gdiplus::GdiplusShutdown(graphics);WSACleanup();return output->failed?1:0;
  }
  auto dpi=GetDpiForSystem();auto u=[&](int n){return MulDiv(n,dpi,96);};
  WNDCLASSW cls{};cls.hInstance=instance;cls.lpszClassName=L"OverwatchHelper.Recorder";
  cls.lpfnWndProc=procedure;cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
  RegisterClassW(&cls);RECT r{0,0,u(650),u(370)};
  DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
  AdjustWindowRectExForDpi(&r,style,FALSE,0,dpi);
  auto title=L"OverwatchHelper · 整夜诊断记录 "+wide(RecorderVersion);
  window=CreateWindowW(cls.lpszClassName,title.c_str(),style,CW_USEDEFAULT,CW_USEDEFAULT,
    r.right-r.left,r.bottom-r.top,nullptr,nullptr,instance,nullptr);
  font=CreateFontW(-u(15),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Microsoft YaHei UI");
  auto add=[&](const wchar_t *kind,const wchar_t *text,int id,int x,int y,int width,int height,DWORD extra=0){
    auto h=CreateWindowW(kind,text,WS_CHILD|WS_VISIBLE|extra,u(x),u(y),u(width),u(height),window,reinterpret_cast<HMENU>(INT_PTR(id)),instance,nullptr);
    SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);return h;};
  add(L"STATIC",L"整夜记录 · 明早停止后分析",0,22,20,600,30);
  statusText=add(L"STATIC",currentStatus.c_str(),0,22,59,606,62);
  shotCheckbox=add(L"BUTTON",L"每 30 秒保存游戏截图（只存本机）",0,22,129,600,28,BS_AUTOCHECKBOX|WS_TABSTOP);
  SendMessageW(shotCheckbox,BM_SETCHECK,options.screenshots?BST_CHECKED:BST_UNCHECKED,0);
  startButton=add(L"BUTTON",L"开始记录",101,22,174,135,35,WS_TABSTOP);
  stopButton=add(L"BUTTON",L"停止并保存",102,173,174,135,35,WS_TABSTOP);
  add(L"BUTTON",L"标记此刻",104,324,174,130,35,WS_TABSTOP);
  add(L"BUTTON",L"打开记录目录",103,470,174,155,35,WS_TABSTOP);
  folderText=add(L"EDIT",options.output.c_str(),0,22,227,603,45,ES_MULTILINE|ES_READONLY);
  bool hotkey=RegisterHotKey(window,1,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_F10)!=FALSE;
  auto hint=std::wstring(L"照常使用原来的 Shift 脚本；本工具只取证，不操作游戏。\r\n请保持游戏可见且无遮挡。最多 18 小时 / 512 MiB，达到上限停止。\r\n")+
    (hotkey?L"明早先停止记录再处理掉线。Ctrl+Alt+F10 可标记当前时刻。":L"标记热键已被占用，请使用“标记此刻”按钮。");
  add(L"STATIC",hint.c_str(),0,22,284,608,74);
  SetTimer(window,1,500,nullptr);ShowWindow(window,SW_SHOW);UpdateWindow(window);
  MSG message{};while(GetMessageW(&message,nullptr,0,0)>0){if(!IsDialogMessageW(window,&message)){TranslateMessage(&message);DispatchMessageW(&message);}}
  SetEvent(stopSignal);if(worker.joinable())worker.join();DeleteObject(font);
  Gdiplus::GdiplusShutdown(graphics);WSACleanup();return 0;
}
