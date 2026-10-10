#pragma once
#include "agent/platform.hpp"
#include "core/semantic.hpp"
#include <functional>

namespace owh {
// An owned, hidden OCR process isolates WinRT/decoder stalls from the agent.
// Only app-created BMPs go to it; no network, window activation or input.
class LocalOcr {
  HANDLE process_{}, request_{}, response_{}, job_{};
  std::string buffered_;
  std::uint64_t serial_{};
  std::wstring imagePath_;
  Millis retryAt_{};
  std::string error_;
  bool readLine(std::string &line, Millis timeout, const std::function<bool()> &alive) {
    const auto deadline = GetTickCount64() + timeout;
    while (alive() && GetTickCount64() < deadline) {
      auto end = buffered_.find('\n');
      if (end != std::string::npos) {
        line = buffered_.substr(0,end); buffered_.erase(0,end+1); return true;
      }
      DWORD available{};
      if (!PeekNamedPipe(response_,nullptr,0,nullptr,&available,nullptr)) return false;
      if (available) {
        char chunk[8192]; DWORD read{};
        if (!ReadFile(response_,chunk,std::min<DWORD>(available,sizeof(chunk)),&read,nullptr) || !read) return false;
        buffered_.append(chunk,read);
        if (buffered_.size() > 512*1024) return false;
      } else {
        if (WaitForSingleObject(process_,0) == WAIT_OBJECT_0) return false;
        Sleep(20);
      }
    }
    return false;
  }
  bool start(const std::function<bool()> &alive,const std::wstring &directory) {
    if (process_) return true;
    if (GetTickCount64() < retryAt_) return false;
    SECURITY_ATTRIBUTES security{sizeof(security),nullptr,TRUE};
    HANDLE childRead{}, childWrite{}, parentRead{}, parentWrite{}, nullHandle{};
    auto closeHandle = [](HANDLE h) { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); };
    auto cleanup = [&] { closeHandle(childRead); closeHandle(childWrite); closeHandle(parentRead); closeHandle(parentWrite); closeHandle(nullHandle); };
    if (!CreatePipe(&childRead,&parentWrite,&security,0) || !CreatePipe(&parentRead,&childWrite,&security,0)) {
      cleanup(); error_ = "Cannot create OCR pipes"; return false;
    }
    SetHandleInformation(parentRead,HANDLE_FLAG_INHERIT,0);
    SetHandleInformation(parentWrite,HANDLE_FLAG_INHERIT,0);
    nullHandle = CreateFileW(L"NUL",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,OPEN_EXISTING,0,nullptr);
    wchar_t windows[32768]{}; GetWindowsDirectoryW(windows,32768);
    auto executable = std::filesystem::path(windows)/L"System32"/L"WindowsPowerShell"/L"v1.0"/L"powershell.exe";
    std::filesystem::path script;
    try { script=embeddedOcrScript(directory); }
    catch (...) {cleanup();error_="Cannot prepare embedded Windows OCR";return false;}
    if (script.empty()) script=std::filesystem::path(exeDirectory())/L"ocr-worker.ps1";
    if (!std::filesystem::is_regular_file(script)) {
      cleanup(); error_ = "Missing ocr-worker.ps1 beside the executable"; retryAt_ = GetTickCount64()+60000; return false;
    }
    std::wstring command = L"\""+executable.wstring()+L"\" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \""+script.wstring()+L"\"";
    SIZE_T bytes{};
    InitializeProcThreadAttributeList(nullptr,1,0,&bytes);
    std::vector<std::uint8_t> attributes(bytes);
    auto list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
    if (!InitializeProcThreadAttributeList(list,1,0,&bytes)) { cleanup(); return false; }
    HANDLE inherited[]{childRead,childWrite,nullHandle};
    bool attributeOk = UpdateProcThreadAttribute(list,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,inherited,sizeof(inherited),nullptr,nullptr) != FALSE;
    STARTUPINFOEXW startup{}; startup.StartupInfo.cb = sizeof(startup);
    startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    startup.StartupInfo.wShowWindow = SW_HIDE;
    startup.StartupInfo.hStdInput = childRead; startup.StartupInfo.hStdOutput = childWrite;
    startup.StartupInfo.hStdError = nullHandle; startup.lpAttributeList = list;
    PROCESS_INFORMATION child{};
    bool ok = attributeOk && CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,TRUE,
        CREATE_NO_WINDOW|EXTENDED_STARTUPINFO_PRESENT,nullptr,nullptr,&startup.StartupInfo,&child);
    DeleteProcThreadAttributeList(list);
    if (!ok) { cleanup(); error_ = "Cannot start local Windows OCR"; retryAt_ = GetTickCount64()+60000; return false; }
    CloseHandle(child.hThread);
    process_ = child.hProcess; request_ = parentWrite; response_ = parentRead;
    parentRead = parentWrite = nullptr; cleanup();
    job_ = CreateJobObjectW(nullptr,nullptr);
    if (job_) {
      JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit{};
      limit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
      if (!SetInformationJobObject(job_,JobObjectExtendedLimitInformation,&limit,sizeof(limit)) ||
          !AssignProcessToJobObject(job_,process_)) { CloseHandle(job_); job_ = nullptr; }
    }
    std::string ready; Json result; std::string parseError;
    if (!readLine(ready,5000,alive) || !parseJson(ready,result,parseError) ||
        !result.find("ready") || !result.find("ready")->is(Json::Kind::Bool) || !result.find("ready")->boolean) {
      auto reason = result.find("error");
      error_ = reason && reason->is(Json::Kind::String) ? reason->text : "Windows OCR startup timed out";
      close(); retryAt_ = GetTickCount64()+60000; return false;
    }
    return true;
  }
public:
  LocalOcr() = default;
  LocalOcr(const LocalOcr &) = delete;
  ~LocalOcr() { close(); }
  void close() {
    if (request_) CloseHandle(request_);
    if (response_) CloseHandle(response_);
    if (process_) { TerminateProcess(process_,0); CloseHandle(process_); }
    if (job_) CloseHandle(job_);
    process_ = request_ = response_ = job_ = nullptr;
    buffered_.clear();
  }
  const std::string &error() const { return error_; }
  bool recognize(const Frame &frame, const std::wstring &dir, OcrFrame &result,
                 const std::function<bool()> &alive, bool deskew = true, bool highDetail = false) {
    if (!start(alive,dir) || !alive()) return false;
    auto folder = std::filesystem::path(dir)/L"adaptive";
    std::error_code ec; std::filesystem::create_directories(folder,ec);
    imagePath_ = (folder/(L"ocr-frame-"+std::to_wstring(GetCurrentProcessId())+L".bmp")).wstring();
    if (!saveFrameBmp(frame,imagePath_)) { error_ = "Cannot write local OCR frame"; return false; }
    auto id = ++serial_;
    auto request = "{\"id\":"+std::to_string(id)+",\"image\":\""+jsonEscape(utf8(imagePath_))+"\",\"deskew\":"+(deskew ? "true" : "false")+",\"high_detail\":"+(highDetail ? "true" : "false")+"}\n";
    DWORD written{};
    std::string reply, parseError; Json root;
    bool ok = WriteFile(request_,request.data(),DWORD(request.size()),&written,nullptr) && written == request.size() &&
              readLine(reply,8000,alive) && parseJson(reply,root,parseError);
    const Json *replyId = ok ? root.find("id") : nullptr;
    if (!ok || !replyId || !replyId->is(Json::Kind::Number) || replyId->number != double(id) ||
        !parseOcrFrame(reply,result,error_)) {
      if (error_.empty()) error_ = "Windows OCR timed out or returned an invalid response";
      close(); retryAt_ = GetTickCount64()+10000; return false;
    }
    error_.clear(); return true;
  }
};

inline bool deathmatchNavigationNeedsDetail(const SemanticContext &context,const SemanticObservation &result) {
  return !context.launcher && context.mode == WorkflowMode::Deathmatch &&
         (!result || result.observation.page == CNPage::Play ||
          (result.observation.page == CNPage::More && result.observation.step.action!=Action::Click));
}
inline bool readSmallNavigationText(LocalOcr &ocr,const Frame &frame,const std::wstring &directory,
    OcrFrame &out,const std::function<bool()> &alive) {
  if (frame.width < 640 || frame.height < 360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return false;
  // Isolate the compact Arcade navigation strip instead of increasing the
  // whole image beyond Windows OCR's dimension cap. Coordinates are remapped
  // into the original client area before any recognition or click is allowed.
  const int x=int(frame.width*.59), y=int(frame.height*.098);
  const int width=int(frame.width*.144), height=int(frame.height*.075), scale=3;
  Frame crop{width*scale,height*scale,std::vector<std::uint32_t>(std::size_t(width*height*scale*scale))};
  for (int cy=0;cy<crop.height;++cy) for (int cx=0;cx<crop.width;++cx) {
    auto pixel=frame.pixels[std::size_t(y+cy/scale)*frame.width+x+cx/scale];
    bool glyph=std::min({(pixel>>16)&255,(pixel>>8)&255,pixel&255}) >= 100;
    crop.pixels[std::size_t(cy)*crop.width+cx]=glyph ? 0xff000000 : 0xffffffff;
  }
  OcrFrame local;
  if (!ocr.recognize(crop,directory,local,alive,false,true)) return false;
  out={frame.width,frame.height,{}};
  for (auto line:local.lines) {
    for (auto &word:line.words) {
      word.roi={double(x)/frame.width+word.roi.x*width/frame.width,
                double(y)/frame.height+word.roi.y*height/frame.height,
                word.roi.w*width/frame.width,word.roi.h*height/frame.height};
    }
    out.lines.push_back(std::move(line));
  }
  return true;
}
inline bool readSmallModeText(LocalOcr &ocr,const Frame &frame,const std::wstring &directory,
    OcrFrame &out,const std::function<bool()> &alive) {
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return false;
  out={frame.width,frame.height,{}};
  // Card captions are regular UI text. Read the two halves without the
  // whole-frame italic correction; each stays below the OCR dimension cap.
  // The crops only locate text, never supply a guessed click position.
  for (Rect area : {Rect{.02,.20,.48,.66},Rect{.50,.20,.48,.66}}) {
    if (!alive()) return false;
    const int x=int(frame.width*area.x),y=int(frame.height*area.y);
    const int width=int(frame.width*area.w),height=int(frame.height*area.h);
    Frame crop{width,height,std::vector<std::uint32_t>(std::size_t(width)*height)};
    for (int row=0;row<height;++row)
      std::copy_n(frame.pixels.begin()+std::size_t(y+row)*frame.width+x,width,crop.pixels.begin()+std::size_t(row)*width);
    OcrFrame local;
    if (!ocr.recognize(crop,directory,local,alive,false,true)) return false;
    for (auto line:local.lines) {
      for (auto &word:line.words)
        word.roi={double(x)/frame.width+word.roi.x*width/frame.width,
                  double(y)/frame.height+word.roi.y*height/frame.height,
                  word.roi.w*width/frame.width,word.roi.h*height/frame.height};
      out.lines.push_back(std::move(line));
    }
  }
  return true;
}
inline bool heroSelectionNeedsDetail(const SemanticContext &context,const SemanticObservation &result) {
  return !context.launcher && (!result || (result.observation.page==CNPage::Hero && result.observation.step.action!=Action::Click));
}
inline bool semanticNeedsDetail(const SemanticContext &context,const SemanticObservation &result) {
  if (result && result.observation.page==CNPage::Hero && result.observation.step.action==Action::Click) return false;
  return !result || result.observation.page==CNPage::Browser || transientModeNavigation(context,result.observation.page);
}
inline bool readSmallHeroText(LocalOcr &ocr,const Frame &frame,const std::wstring &directory,
    OcrFrame &out,const std::function<bool()> &alive) {
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return false;
  out={frame.width,frame.height,{}};
  // Read small regular UI text without shearing it with the italic hero art.
  // Separate the footer from Chat and the F1 keycap. Native color at 3x reads
  // these captures better than binarization or indiscriminate 6x/8x scaling.
  // Returned boxes are mapped back to the original client area.
  for (Rect area : {Rect{.30,.86,.40,.10},Rect{.46,.88,.08,.07},Rect{.08,.935,.085,.052}}) {
    if (!alive()) return false;
    const int x=int(frame.width*area.x),y=int(frame.height*area.y);
    const int width=int(frame.width*area.w),height=int(frame.height*area.h),scale=1;
    Frame crop{width*scale,height*scale,std::vector<std::uint32_t>(std::size_t(width*height*scale*scale))};
    for (int cy=0;cy<crop.height;++cy) for (int cx=0;cx<crop.width;++cx) {
      const auto pixel=frame.pixels[std::size_t(y+cy/scale)*frame.width+x+cx/scale];
      crop.pixels[std::size_t(cy)*crop.width+cx]=pixel;
    }
    OcrFrame local;
    if (!ocr.recognize(crop,directory,local,alive,false,true)) return false;
    for (auto line:local.lines) {
      for (auto &word:line.words)
        word.roi={double(x)/frame.width+word.roi.x*width/frame.width,
                  double(y)/frame.height+word.roi.y*height/frame.height,
                  word.roi.w*width/frame.width,word.roi.h*height/frame.height};
      out.lines.push_back(std::move(line));
    }
  }
  return true;
}
inline bool customRoomNeedsDetail(const SemanticContext &context,const SemanticObservation &result) {
  return !context.launcher && context.mode==WorkflowMode::CustomRoom &&
      (!result || result.observation.page==CNPage::RoomList);
}
inline bool readCustomRoomText(LocalOcr &ocr,const Frame &frame,const std::wstring &directory,
    OcrFrame &out,const std::function<bool()> &alive) {
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return false;
  out={frame.width,frame.height,{}};
  for (Rect area:{Rect{300./1024,52./576,440./1024,48./576},Rect{.17,.22,.42,.59},Rect{777./1024,160./576,64./1024,300./576}}) {
    if (!alive()) return false;
    const int x=int(frame.width*area.x),y=int(frame.height*area.y);
    const int width=int(frame.width*area.w),height=int(frame.height*area.h);
    // The narrow occupancy column needs nearest-neighbor enlargement before
    // Windows OCR; wide crops lose the slash in small 11/12 labels.
    const int scale=area.x>.7 ? 4 : 1;
    Frame crop{width*scale,height*scale,std::vector<std::uint32_t>(std::size_t(width)*height*scale*scale)};
    for (int row=0;row<crop.height;++row) for (int col=0;col<crop.width;++col)
      crop.pixels[std::size_t(row)*crop.width+col]=frame.pixels[std::size_t(y+row/scale)*frame.width+x+col/scale];
    OcrFrame local;
    if (!ocr.recognize(crop,directory,local,alive,area.y<.2,true)) return false;
    for (auto line:local.lines) {
      for (auto &word:line.words) word.roi={double(x)/frame.width+word.roi.x*width/frame.width,
          double(y)/frame.height+word.roi.y*height/frame.height,word.roi.w*width/frame.width,word.roi.h*height/frame.height};
      out.lines.push_back(std::move(line));
    }
  }
  return true;
}
class AdaptiveVision {
  LocalOcr ocr_;
  SemanticCache cache_;
  std::optional<LearnedPage> pending_;
  std::wstring dir_;
  std::string error_;
public:
  bool usedOcr{};
  void reset(const std::wstring &dir) {
    dir_ = dir; pending_.reset(); cache_.pages.clear(); ocr_.close();
    error_.clear();
    std::ifstream file(std::filesystem::path(dir)/L"adaptive-profile.json",std::ios::binary);
    if (file) {
      std::string source;
      char chunk[4096];
      while (file && source.size() <= 1024*1024) { file.read(chunk,sizeof(chunk)); source.append(chunk,std::size_t(file.gcount())); }
      if (!parseSemanticCache(source,cache_,error_)) cache_.pages.clear();
    }
  }
  void cancel() { ocr_.close(); pending_.reset(); }
  const std::string &error() const { return error_.empty() ? ocr_.error() : error_; }
  std::size_t learnedPages() const { return cache_.pages.size(); }
  SemanticObservation recognize(const Frame &frame, const SemanticContext &context,
                                const std::function<bool()> &alive, bool allowOcr = true) {
    usedOcr = false;
    auto result = cache_.match(frame,context);
    if (result) return result;
    pending_.reset();
    if (!allowOcr) return {};
    OcrFrame words;
    // The caller must recapture even when an attempted OCR request fails or
    // finds no semantic page: its template fallback predates this wait.
    usedOcr = true;
    if (!ocr_.recognize(frame,dir_,words,alive,!context.launcher)) return {};
    result = recognizeSemantic(frame,words,context);
    // Preserve the ordinary pass. A denser rendering helps small Chinese
    // glyphs, but can miss labels that the original pass already recognized.
    if (semanticNeedsDetail(context,result) && alive()) {
      OcrFrame detail;
      if (ocr_.recognize(frame,dir_,detail,alive,!context.launcher,true)) {
        result = recognizeSemanticSupplement(frame,words,detail,context,result);
      }
    }
    if (deathmatchNavigationNeedsDetail(context,result) && alive()) {
      OcrFrame cards;
      if (readSmallModeText(ocr_,frame,dir_,cards,alive))
        result=recognizeSemanticSupplement(frame,words,cards,context,result);
    }
    if (deathmatchNavigationNeedsDetail(context,result) && alive()) {
      OcrFrame navigation;
      if (readSmallNavigationText(ocr_,frame,dir_,navigation,alive)) {
        result=recognizeSemanticSupplement(frame,words,navigation,context,result);
      }
    }
    if (customRoomNeedsDetail(context,result) && alive()) {
      OcrFrame rooms;
      if (readCustomRoomText(ocr_,frame,dir_,rooms,alive))
        result=recognizeSemanticSupplement(frame,words,rooms,context,result);
    }
    if (heroSelectionNeedsDetail(context,result) && alive()) {
      OcrFrame selection;
      if (readSmallHeroText(ocr_,frame,dir_,selection,alive)) {
        auto combined=words;combined.lines.insert(combined.lines.end(),selection.lines.begin(),selection.lines.end());
        const auto recovered=recognizeSemantic(frame,combined,context);
        if (recovered && recovered.observation.page==CNPage::Hero) result=recovered;
      }
    }
    if (result) pending_ = learnSemantic(frame,result,context);
    return result;
  }
  bool confirm(CNPage page) {
    if (!pending_ || pending_->semantic.observation.page != page) return false;
    if (!cache_.remember(std::move(*pending_))) { pending_.reset(); return false; }
    pending_.reset();
    auto path = std::filesystem::path(dir_)/L"adaptive-profile.json";
    auto temporary = std::filesystem::path(dir_)/(L"adaptive-profile-"+std::to_wstring(GetCurrentProcessId())+L".tmp");
    std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
    auto source = writeSemanticCache(cache_);
    file << source; file.close();
    if (!file || !MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
      error_ = "Cannot persist adaptive page calibration"; return false;
    }
    error_.clear(); return true;
  }
};
inline bool semanticStillVisible(const Frame &before, const Frame &after, const SemanticObservation &semantic) {
  if (!semantic || before.width != after.width || before.height != after.height) return false;
  if ((semantic.observation.page == CNPage::Combat || semantic.liveRoomHud) && !combatHud(after)) return false;
  if (semantic.observation.page==CNPage::Combat && semantic.verifiedDeathmatch && !deathmatchScoreboard(after)) return false;
  if (semantic.heroRoster) {
    const auto grid=heroPortraitGrid(after);
    if (!grid || semantic.evidence.empty()) return false;
    const auto original=semantic.evidence.front();
    if (std::abs(grid->x-original.x)>.02 || std::abs(grid->y-original.y)>.02 ||
        std::abs(grid->w-original.w)>.04 || std::abs(grid->h-original.h)>.02) return false;
  }
  for (std::size_t i = 0; i < semantic.evidence.size(); ++i) {
    if (semantic.heroRoster && i==0) continue;
    // The numeric health region changes during combat. The score/title is the
    // stable text; the HUD shape is checked above on the fresh capture.
    if (semantic.observation.page == CNPage::Combat && i > 0) continue;
    auto roi = semantic.evidence[i];
    if (glyphSimilarity(sample(before,roi),sample(after,roi)) < .95) return false;
  }
  return !semantic.coloredButton || semantic.observation.step.action == Action::Wait ||
         enabledUiButton(after,semantic.button);
}
} // namespace owh
