#pragma once
#include "agent/platform.hpp"
#include <functional>
#include <commctrl.h>
#include <memory>

namespace owh::learning {
inline constexpr std::size_t MaximumProfileBytes = 4 * 1024 * 1024;
inline constexpr wchar_t WindowClass[] = L"OverwatchHelper.Learning";
inline constexpr UINT CancelMessage = WM_APP + 210;
inline std::atomic<bool> activeChild{};
inline std::atomic<HWND> learningWindow{};
inline std::atomic<unsigned> uniqueCounter{};

// Windows argv quoting, including trailing backslashes and embedded quotes.
// These arguments go directly to CreateProcessW, never to a command shell.
inline std::wstring quoteArgument(const std::wstring &argument) {
  std::wstring result = L"\"";
  std::size_t slashes{};
  for (wchar_t c : argument) {
    if (c == L'\\') { ++slashes; continue; }
    result.append(c == L'\"' ? slashes * 2 + 1 : slashes, L'\\');
    slashes = 0; result += c;
  }
  result.append(slashes * 2, L'\\'); result += L'\"';
  return result;
}
inline std::wstring commandLine(const std::wstring &program, const std::vector<std::wstring> &arguments) {
  auto result = quoteArgument(program);
  for (const auto &argument : arguments) result += L" " + quoteArgument(argument);
  return result;
}
inline std::wstring uniqueName(const wchar_t *prefix) {
  SYSTEMTIME now{}; GetLocalTime(&now);
  wchar_t name[180]{};
  swprintf(name,180,L"%ls-%04u%02u%02u-%02u%02u%02u-%03u-%lu-%llu-%u",prefix,
           now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,now.wMilliseconds,
           GetCurrentProcessId(),GetTickCount64(),++uniqueCounter);
  return name;
}
inline std::wstring failure(const wchar_t *message) {
  return std::wstring(message) + L"（Windows 错误 " + std::to_wstring(GetLastError()) + L"）";
}
inline bool readBounded(const std::filesystem::path &path,std::string &out,std::wstring &error,
                        std::size_t maximum = MaximumProfileBytes) {
  Handle file(CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
  if (file.h == INVALID_HANDLE_VALUE) { error = failure(L"无法读取文件"); return false; }
  LARGE_INTEGER size{};
  if (!GetFileSizeEx(file.h,&size) || size.QuadPart <= 0 || size.QuadPart > LONGLONG(maximum)) {
    error = L"文件为空或超过允许大小（4 MiB）。"; return false;
  }
  std::string contents(std::size_t(size.QuadPart),'\0'); DWORD read{};
  if (!ReadFile(file.h,contents.data(),DWORD(contents.size()),&read,nullptr) || read != contents.size()) {
    error = failure(L"文件读取不完整"); return false;
  }
  out = std::move(contents); return true;
}
inline bool importProfile(const std::wstring &dir,const std::wstring &path,CNProfile &out,std::wstring &error) {
  error.clear(); std::string source, parseError;
  if (!readBounded(std::filesystem::path(path),source,error)) return false;
  CNProfile parsed;
  if (!parseCNProfile(source,parsed,parseError)) {
    error = L"配置校验失败：" + wide(parseError); return false;
  }
  const auto normalized = writeCNProfile(parsed);
  CNProfile checked;
  if (normalized.size() > MaximumProfileBytes || !parseCNProfile(normalized,checked,parseError)) {
    error = L"配置无法完整保存：" + wide(parseError); return false;
  }
  std::error_code ec;
  std::filesystem::create_directories(dir,ec);
  if (ec) { error = L"无法创建配置目录：" + wide(ec.message()); return false; }
  const auto destination = std::filesystem::path(dir)/L"cn-profile.json";
  const auto temporary = std::filesystem::path(dir)/(uniqueName(L"cn-profile-pending") + L".json");
  {
    Handle file(CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
    if (file.h == INVALID_HANDLE_VALUE) { error = failure(L"无法创建待导入配置"); return false; }
    DWORD written{};
    if (!WriteFile(file.h,normalized.data(),DWORD(normalized.size()),&written,nullptr) ||
        written != normalized.size() || !FlushFileBuffers(file.h)) {
      error = failure(L"无法完整写入待导入配置"); return false;
    }
  }
  DWORD attributes = GetFileAttributesW(destination.c_str());
  if (attributes != INVALID_FILE_ATTRIBUTES) {
    const auto backup = std::filesystem::path(dir)/(uniqueName(L"cn-profile-backup") + L".json");
    if (!CopyFileW(destination.c_str(),backup.c_str(),TRUE)) {
      error = failure(L"无法备份现有配置，未替换原文件"); return false;
    }
  } else if (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND) {
    error = failure(L"无法检查现有配置，未替换原文件"); return false;
  }
  if (!MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
    error = failure(L"无法替换配置，原文件仍保留"); return false;
  }
  out = std::move(checked); return true;
}
inline bool localEndpoint(const std::wstring &endpoint) {
  URL_COMPONENTS url{}; url.dwStructSize = sizeof(url);
  wchar_t host[512]{}; url.lpszHostName = host; url.dwHostNameLength = 512;
  url.dwUserNameLength = DWORD(-1); url.dwPasswordLength = DWORD(-1);
  if (!WinHttpCrackUrl(endpoint.c_str(),DWORD(endpoint.size()),0,&url) ||
      (url.nScheme != INTERNET_SCHEME_HTTP && url.nScheme != INTERNET_SCHEME_HTTPS) ||
      url.dwUserNameLength || url.dwPasswordLength) return false;
  auto hostname = std::wstring(host,url.dwHostNameLength);
  return _wcsicmp(hostname.c_str(),L"localhost") == 0 || hostname == L"127.0.0.1" ||
         hostname == L"::1" || hostname == L"[::1]";
}
inline bool busy() { return activeChild.load(); }
inline bool isOpen() { return learningWindow.load()!=nullptr; }
inline void cancel() {
  if (auto window = learningWindow.load()) PostMessageW(window,CancelMessage,0,0);
}

enum ControlId { Endpoint = 8101, Model, Session, Output, Record, BrowseSession, Annotate,
                 Stop, View, Import, BrowseProfile, Status, Log };
enum class ChildKind { None, Recording, Model };
struct Placement { HWND control{}; int x{},y{},w{},h{}; };
struct WindowState {
  HWND window{},owner{};
  HFONT font{};
  UINT dpi{96};
  std::wstring directory,gameExe,target;
  std::function<void(const CNProfile&)> imported;
  std::vector<Placement> controls;
  HANDLE process{},job{};
  ChildKind child{ChildKind::None};
  std::filesystem::path childDirectory,logPath;
  std::wstring lastLog;
  bool cancelling{},closing{};
  ~WindowState() {
    if (process) CloseHandle(process);
    if (job) CloseHandle(job);
    if (font) DeleteObject(font);
  }
  int u(int n) const { return MulDiv(n,dpi,96); }
  HWND control(int id) const { return GetDlgItem(window,id); }
  std::wstring text(int id) const {
    HWND item=control(id); int n=GetWindowTextLengthW(item);
    std::wstring result(std::size_t(n)+1,0); GetWindowTextW(item,result.data(),n+1); result.resize(n); return result;
  }
  void status(const std::wstring &value) { SetWindowTextW(control(Status),value.c_str()); }
  HWND add(const wchar_t *type,const std::wstring &label,int id,int x,int y,int w,int h,DWORD style=0) {
    HWND item=CreateWindowExW(type==std::wstring(L"EDIT") ? WS_EX_CLIENTEDGE : 0,type,label.c_str(),
       WS_CHILD|WS_VISIBLE|style,u(x),u(y),u(w),u(h),window,
       reinterpret_cast<HMENU>(INT_PTR(id)),GetModuleHandleW(nullptr),nullptr);
    controls.push_back({item,x,y,w,h}); SendMessageW(item,WM_SETFONT,WPARAM(font),TRUE); return item;
  }
  void layout() {
    auto previous=font;
    font=CreateFontW(-u(16),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
    for (const auto &item:controls) {
      MoveWindow(item.control,u(item.x),u(item.y),u(item.w),u(item.h),TRUE);
      SendMessageW(item.control,WM_SETFONT,WPARAM(font),TRUE);
    }
    if (previous) DeleteObject(previous);
  }
  void settings() {
    auto path=(std::filesystem::path(directory)/L"local-vision.ini").wstring();
    writeIni(path,L"local-vision",L"endpoint",text(Endpoint));
    writeIni(path,L"local-vision",L"model",text(Model));
    writeIni(path,L"local-vision",L"last_session",text(Session));
    writeIni(path,L"local-vision",L"last_profile",text(Output));
  }
  void enable() {
    const bool idle=child==ChildKind::None;
    for (int id:{Endpoint,Model,Session,Output,Record,BrowseSession,Annotate,Import,BrowseProfile})
      EnableWindow(control(id),idle);
    EnableWindow(control(Stop),!idle&&!cancelling);
    EnableWindow(control(View),!text(Output).empty()||!logPath.empty());
  }
  void createControls() {
    dpi=GetDpiForWindow(window); layout();
    add(L"STATIC",L"演示学习与本地视觉校准",0,18,14,735,28);
    add(L"STATIC",L"1. 录制实际操作。F9 采集页面，F10 标记房间后停留 1–2 秒，F8 完成。\r\n"
        L"2. 本地模型标注页面和点击位置；第二帧验证房间特征。\r\n"
        L"3. 查看图文结果并导入，关闭此窗口后回到主界面开始。",0,18,49,744,76);
    add(L"STATIC",L"模型地址",0,18,135,80,24);
    add(L"EDIT",L"",Endpoint,104,131,400,28,ES_AUTOHSCROLL|WS_TABSTOP);
    add(L"STATIC",L"模型名称",0,516,135,78,24);
    add(L"EDIT",L"",Model,598,131,164,28,ES_AUTOHSCROLL|WS_TABSTOP);
    SendMessageW(control(Model),EM_SETCUEBANNER,TRUE,LPARAM(L"留空自动发现"));
    SendMessageW(control(Endpoint),EM_SETLIMITTEXT,2048,0);
    SendMessageW(control(Model),EM_SETLIMITTEXT,512,0);
    add(L"STATIC",L"录制文件",0,18,175,80,24);
    add(L"EDIT",L"",Session,104,171,548,28,ES_AUTOHSCROLL|WS_TABSTOP);
    add(L"BUTTON",L"浏览录制",BrowseSession,662,171,100,28,WS_TABSTOP);
    add(L"STATIC",L"配置文件",0,18,215,80,24);
    add(L"EDIT",L"",Output,104,211,548,28,ES_AUTOHSCROLL|WS_TABSTOP);
    add(L"BUTTON",L"选择配置",BrowseProfile,662,211,100,28,WS_TABSTOP);
    add(L"BUTTON",L"录制操作/页面",Record,18,255,157,34,WS_TABSTOP);
    add(L"BUTTON",L"开始模型标注",Annotate,185,255,157,34,WS_TABSTOP);
    add(L"BUTTON",L"取消 / F8",Stop,352,255,114,34,WS_TABSTOP);
    add(L"BUTTON",L"查看结果",View,476,255,136,34,WS_TABSTOP);
    add(L"BUTTON",L"导入配置",Import,622,255,140,34,WS_TABSTOP);
    add(L"STATIC",L"选择已有录制，或先录制一次需要修复的页面。",Status,18,307,744,47);
    add(L"EDIT",L"",Log,18,363,744,213,ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP);
    add(L"STATIC",L"导入先校验并备份现有配置；保存配置不会自动开始按键。日志和模型审查保存在输出目录。",0,18,590,744,43);
    auto path=(std::filesystem::path(directory)/L"local-vision.ini").wstring();
    SetWindowTextW(control(Endpoint),readIni(path,L"local-vision",L"endpoint",L"http://127.0.0.1:8080").c_str());
    SetWindowTextW(control(Model),readIni(path,L"local-vision",L"model").c_str());
    SetWindowTextW(control(Session),readIni(path,L"local-vision",L"last_session").c_str());
    SetWindowTextW(control(Output),readIni(path,L"local-vision",L"last_profile").c_str());
    SetTimer(window,1,250,nullptr); enable();
  }
  bool startChild(const wchar_t *tool,const std::vector<std::wstring> &arguments,
                  ChildKind kind,const std::filesystem::path &log,std::wstring &error) {
    if (child!=ChildKind::None) { error=L"请先完成当前录制或标注。"; return false; }
    const auto program=std::filesystem::path(exeDirectory())/tool;
    std::error_code ec;
    if (!std::filesystem::is_regular_file(program,ec)) {
      error=L"缺少 " + std::wstring(tool) + L"。请完整解压软件包。"; return false;
    }
    auto command=commandLine(program.wstring(),arguments);
    if (command.size()>=32760) { error=L"命令参数过长。"; return false; }
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES),nullptr,TRUE};
    Handle output(CreateFileW(log.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,
                              CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
    Handle input(CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,
                             OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if (output.h==INVALID_HANDLE_VALUE || input.h==INVALID_HANDLE_VALUE) {
      error=failure(L"无法创建学习任务日志"); return false;
    }
    Handle ownedJob(CreateJobObjectW(nullptr,nullptr));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!ownedJob.h || !SetInformationJobObject(ownedJob.h,JobObjectExtendedLimitInformation,&limits,sizeof(limits))) {
      error=failure(L"无法创建子进程控制"); return false;
    }
    SIZE_T attributeSize{}; InitializeProcThreadAttributeList(nullptr,1,0,&attributeSize);
    std::vector<unsigned char> buffer(attributeSize);
    auto attributes=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
    if (!InitializeProcThreadAttributeList(attributes,1,0,&attributeSize)) {
      error=failure(L"无法准备子进程参数"); return false;
    }
    HANDLE inherited[]{input.h,output.h};
    bool configured=UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                             inherited,sizeof(inherited),nullptr,nullptr)!=FALSE;
    STARTUPINFOEXW startup{}; startup.StartupInfo.cb=sizeof(startup);
    startup.StartupInfo.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    startup.StartupInfo.wShowWindow=kind==ChildKind::Model ? SW_HIDE : SW_SHOWNORMAL;
    startup.StartupInfo.hStdInput=input.h; startup.StartupInfo.hStdOutput=output.h; startup.StartupInfo.hStdError=output.h;
    startup.lpAttributeList=attributes; PROCESS_INFORMATION processInfo{};
    DWORD flags=CREATE_SUSPENDED|EXTENDED_STARTUPINFO_PRESENT|(kind==ChildKind::Model ? CREATE_NO_WINDOW : 0);
    bool created=configured&&CreateProcessW(program.c_str(),command.data(),nullptr,nullptr,TRUE,flags,nullptr,
                                            program.parent_path().c_str(),&startup.StartupInfo,&processInfo);
    DWORD processError=GetLastError(); DeleteProcThreadAttributeList(attributes);
    if (!created) { SetLastError(processError); error=failure(L"无法启动学习工具"); return false; }
    Handle thread(processInfo.hThread); Handle pendingProcess(processInfo.hProcess);
    if (!AssignProcessToJobObject(ownedJob.h,pendingProcess.h) || ResumeThread(thread.h)==DWORD(-1)) {
      processError=GetLastError(); TerminateProcess(pendingProcess.h,1);
      SetLastError(processError); error=failure(L"无法管理学习工具"); return false;
    }
    process=pendingProcess.h; pendingProcess.h=nullptr;
    job=ownedJob.h; ownedJob.h=nullptr;
    child=kind; logPath=log; lastLog.clear(); cancelling=false;
    activeChild=true; SetWindowTextW(control(Log),L""); enable(); return true;
  }
  void choose(bool profile) {
    wchar_t path[32768]{};
    const auto previous=text(profile ? Output : Session);
    if (previous.size()<32768) std::copy(previous.begin(),previous.end(),path);
    OPENFILENAMEW dialog{}; dialog.lStructSize=sizeof(dialog); dialog.hwndOwner=window;
    dialog.lpstrFilter=L"JSON 文件 (*.json)\0*.json\0所有文件\0*.*\0";
    dialog.lpstrFile=path; dialog.nMaxFile=32768;
    dialog.lpstrTitle=profile ? L"选择已生成的页面配置" : L"选择录制 manifest.json";
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&dialog)) { SetWindowTextW(control(profile?Output:Session),path); enable(); }
  }
  void record() {
    std::error_code ec; auto root=std::filesystem::path(directory)/L"teaching";
    std::filesystem::create_directories(root,ec);
    if (ec) { status(L"无法创建录制目录："+wide(ec.message())); return; }
    childDirectory=root/uniqueName(L"teach");
    auto log=root/(childDirectory.filename().wstring()+L".log");
    std::wstring error; settings();
    if (startChild(L"owh-teach.exe",{L"--output",childDirectory.wstring(),L"--game-exe",gameExe,L"--target",target},
                   ChildKind::Recording,log,error))
      status(L"录制窗口已打开。请在该窗口开始，再回到游戏操作；F9 采集；F10 标记房间后停留 1–2 秒，再 F8 完成。");
    else status(error);
  }
  void annotate() {
    const auto session=text(Session), endpoint=text(Endpoint), model=text(Model);
    if (!localEndpoint(endpoint)) { status(L"请输入本机视觉模型地址，例如 http://127.0.0.1:8080。"); return; }
    std::string contents,parseError; std::wstring error; Json data;
    if (!readBounded(session,contents,error) || !parseJson(contents,data,parseError)) {
      status(error.empty()?L"录制 JSON 无法读取："+wide(parseError):error); return;
    }
    const auto format=data.find("format"), steps=data.find("steps");
    if (!format || !format->is(Json::Kind::String) || format->text!="owh-teaching" ||
        !steps || !steps->is(Json::Kind::Array) || steps->items.empty()) {
      status(L"请选择含有效页面步骤的录制 manifest.json。"); return;
    }
    std::error_code ec; childDirectory=std::filesystem::path(directory)/L"calibrations"/uniqueName(L"vision");
    std::filesystem::create_directories(childDirectory,ec);
    if (ec) { status(L"无法创建标注目录："+wide(ec.message())); return; }
    auto output=childDirectory/L"profile.json";
    SetWindowTextW(control(Output),output.c_str()); settings();
    std::vector<std::wstring> args{L"--session",session,L"--endpoint",endpoint,L"--out",output.wstring()};
    const auto currentProfile=std::filesystem::path(directory)/L"cn-profile.json";
    if (std::filesystem::is_regular_file(currentProfile,ec)) {
      args.push_back(L"--base-profile"); args.push_back(currentProfile.wstring());
    }
    if (!model.empty()) { args.push_back(L"--model"); args.push_back(model); }
    if (startChild(L"owh-vision-calibrate.exe",args,ChildKind::Model,childDirectory/L"run.log",error))
      status(L"本地模型正在标注录制页面；完成后请查看结果并导入。F8 可取消本次标注。");
    else status(error);
  }
  void stop() {
    if (child==ChildKind::None || cancelling) return;
    cancelling=true;
    if (child==ChildKind::Recording) {
      Handle event(OpenEventW(EVENT_MODIFY_STATE,FALSE,L"Local\\OverwatchHelper.Teaching.Stop"));
      if (event.h) SetEvent(event.h);
      // The recorder flushes the manifest before exiting. Do not kill it just
      // because the user pressed F8 while a screenshot was being saved.
      status(L"正在完成录制并保存页面，请稍候。");
    } else {
      if (job) TerminateJobObject(job,ERROR_CANCELLED);
      status(L"已取消本次模型标注，本地模型服务继续运行。");
    }
    enable();
  }
  void updateLog() {
    if (logPath.empty()) return;
    Handle file(CreateFileW(logPath.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    LARGE_INTEGER size{};
    if (file.h==INVALID_HANDLE_VALUE || !GetFileSizeEx(file.h,&size)) return;
    constexpr DWORD maximum=12000;
    if (size.QuadPart>maximum) { LARGE_INTEGER from{}; from.QuadPart=size.QuadPart-maximum; SetFilePointerEx(file.h,from,nullptr,FILE_BEGIN); }
    std::string bytes(std::size_t(std::min<LONGLONG>(size.QuadPart,maximum)),0); DWORD count{};
    if (!ReadFile(file.h,bytes.data(),DWORD(bytes.size()),&count,nullptr)) return;
    bytes.resize(count); auto current=wide(bytes);
    if (current!=lastLog) {
      lastLog=current; SetWindowTextW(control(Log),current.c_str());
      SendMessageW(control(Log),EM_SETSEL,WPARAM(current.size()),LPARAM(current.size()));
      SendMessageW(control(Log),EM_SCROLLCARET,0,0);
    }
  }
  void poll() {
    if (child==ChildKind::None) return;
    updateLog();
    if (cancelling&&child==ChildKind::Recording) {
      Handle event(OpenEventW(EVENT_MODIFY_STATE,FALSE,L"Local\\OverwatchHelper.Teaching.Stop"));
      if (event.h) SetEvent(event.h);
    }
    if (WaitForSingleObject(process,0)!=WAIT_OBJECT_0) return;
    DWORD exitCode{}; GetExitCodeProcess(process,&exitCode);
    const auto completed=child;
    CloseHandle(process); process=nullptr; CloseHandle(job); job=nullptr;
    child=ChildKind::None; activeChild=false;
    if (completed==ChildKind::Recording) {
      const auto manifest=childDirectory/L"manifest.json";
      std::error_code ec;
      if (std::filesystem::is_regular_file(manifest,ec)) {
        SetWindowTextW(control(Session),manifest.c_str());
        status(L"录制已保存。可继续开始模型标注；仅采集页面时，请在审查中确认模型建议。");
      } else status(L"录制结束，但没有生成 manifest.json。请查看下方日志后重试。");
    } else if (cancelling) status(L"本次模型标注已取消。输出目录保留已有日志和审查结果。");
    else if (exitCode==0) status(L"标注与源图回放验证完成。请查看结果，然后导入配置。");
    else if (exitCode==3) status(L"模型审查已生成，但没有通过的可执行步骤。请查看结果，补充演示或房间标记后重试。");
    else status(L"标注未完成（退出码 "+std::to_wstring(exitCode)+L"）。请查看日志及模型服务状态。");
    cancelling=false; settings(); enable();
    if (closing) DestroyWindow(window);
  }
  void view() {
    const auto path=text(Output); std::error_code ec;
    if (!path.empty()) {
      const auto review=std::filesystem::path(path+L".review.html");
      if (std::filesystem::is_regular_file(review,ec) &&
          INT_PTR(ShellExecuteW(window,L"open",review.c_str(),nullptr,nullptr,SW_SHOWNORMAL))>32) return;
      ec.clear();
    }
    std::filesystem::path folder=path.empty() ? logPath.parent_path() : std::filesystem::path(path).parent_path();
    if (folder.empty()||!std::filesystem::is_directory(folder,ec)) { status(L"尚未创建输出目录，请先录制或标注。"); return; }
    ShellExecuteW(window,L"open",folder.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
  }
  void importSelected() {
    CNProfile profile; std::wstring error;
    if (!importProfile(directory,text(Output),profile,error)) { status(error); return; }
    if (imported&&(!owner||IsWindow(owner))) imported(profile);
    status(L"配置已校验并导入，原配置已备份。请回主窗口确认设置，再点击开始。"); settings();
  }
};

inline LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM w,LPARAM l) {
  auto state=reinterpret_cast<WindowState*>(GetWindowLongPtrW(window,GWLP_USERDATA));
  if (message==WM_NCCREATE) {
    state=static_cast<WindowState*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);
    state->window=window; SetWindowLongPtrW(window,GWLP_USERDATA,LONG_PTR(state));
  }
  if (!state) return DefWindowProcW(window,message,w,l);
  try {
    switch (message) {
    case WM_CREATE: state->createControls(); return 0;
    case WM_COMMAND:
      if (HIWORD(w)!=BN_CLICKED) break;
      switch (LOWORD(w)) {
      case Record: state->record(); break;
      case BrowseSession: state->choose(false); break;
      case BrowseProfile: state->choose(true); break;
      case Annotate: state->annotate(); break;
      case Stop: state->stop(); break;
      case View: state->view(); break;
      case Import: state->importSelected(); break;
      }
      return 0;
    case WM_TIMER: state->poll(); return 0;
    case CancelMessage: state->stop(); return 0;
    case WM_DPICHANGED: {
      state->dpi=HIWORD(w); state->layout(); auto rect=reinterpret_cast<RECT*>(l);
      SetWindowPos(window,nullptr,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,SWP_NOZORDER|SWP_NOACTIVATE);
      return 0;
    }
    case WM_CLOSE:
      try { state->settings(); } catch (...) { /* Closing still cancels owned work if preferences cannot be saved. */ }
      if (state->child!=ChildKind::None) { state->closing=true; state->stop(); ShowWindow(window,SW_HIDE); }
      else DestroyWindow(window);
      return 0;
    case WM_NCDESTROY:
      KillTimer(window,1); learningWindow=nullptr; activeChild=false;
      SetWindowLongPtrW(window,GWLP_USERDATA,0); delete state;
      return DefWindowProcW(window,message,w,l);
    }
  } catch (const std::exception &error) {
    state->status(L"操作未完成："+wide(error.what()));
    return 0;
  }
  return DefWindowProcW(window,message,w,l);
}
inline void show(HWND owner,const std::wstring &dir,const std::wstring &gameExe,const std::wstring &target,
                 std::function<void(const CNProfile&)> onImported) {
  if (auto window=learningWindow.load()) {
    auto state=reinterpret_cast<WindowState*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    if (state) { state->closing=false; state->imported=std::move(onImported); state->gameExe=gameExe; state->target=target; }
    ShowWindow(window,SW_RESTORE); SetForegroundWindow(window); return;
  }
  WNDCLASSEXW type{sizeof(type)}; type.lpfnWndProc=procedure; type.hInstance=GetModuleHandleW(nullptr);
  type.hCursor=LoadCursorW(nullptr,IDC_ARROW); type.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
  type.lpszClassName=WindowClass;
  if (!RegisterClassExW(&type)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return;
  auto state=std::make_unique<WindowState>(); state->owner=owner; state->directory=dir; state->gameExe=gameExe; state->target=target;
  state->imported=std::move(onImported);
  UINT dpi=owner ? GetDpiForWindow(owner) : 96;
  DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
  RECT size{0,0,MulDiv(780,dpi,96),MulDiv(644,dpi,96)};
  AdjustWindowRectExForDpi(&size,style,FALSE,WS_EX_CONTROLPARENT,dpi);
  auto raw=state.release();
  HWND window=CreateWindowExW(WS_EX_CONTROLPARENT,WindowClass,L"演示学习 / 本地视觉模型",style,
      CW_USEDEFAULT,CW_USEDEFAULT,size.right-size.left,size.bottom-size.top,owner,nullptr,GetModuleHandleW(nullptr),raw);
  if (!window) return; // WM_NCDESTROY owns state after WM_NCCREATE.
  learningWindow=window; ShowWindow(window,SW_SHOWNORMAL); UpdateWindow(window);
}
} // namespace owh::learning
