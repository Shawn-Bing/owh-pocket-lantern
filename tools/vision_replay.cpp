#include "agent/vision.hpp"
#include <gdiplus.h>
#include <iostream>
using namespace owh;
Frame loadImage(const std::wstring &path,int top) {
  Gdiplus::Bitmap image(path.c_str());
  Frame frame{int(image.GetWidth()),int(image.GetHeight())-top,{}};
  if (image.GetLastStatus() != Gdiplus::Ok || frame.width <= 0 || frame.height <= 0) throw std::runtime_error("Cannot read image");
  frame.pixels.resize(std::size_t(frame.width)*frame.height);
  Gdiplus::Rect rect(0,top,frame.width,frame.height); Gdiplus::BitmapData data{};
  if (image.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data) != Gdiplus::Ok) throw std::runtime_error("Cannot read pixels");
  for (int y = 0; y < frame.height; ++y)
    std::memcpy(frame.pixels.data()+std::size_t(y)*frame.width,static_cast<const char *>(data.Scan0)+std::ptrdiff_t(y)*data.Stride,std::size_t(frame.width)*4);
  image.UnlockBits(&data); return frame;
}
int selfTest() {
  constexpr int width = 960, height = 540;
  HDC dc = CreateCompatibleDC(nullptr);
  BITMAPINFO info{}; info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = width; info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32;
  void *pixels{};
  HBITMAP bitmap = CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
  if (!dc || !bitmap || !pixels) throw std::runtime_error("Cannot create OCR fixture");
  auto previous = SelectObject(dc,bitmap);
  std::fill_n(static_cast<std::uint32_t *>(pixels),width*height,0xff202838);
  auto font = CreateFontW(-32,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei");
  auto previousFont = SelectObject(dc,font);
  SetBkMode(dc,TRANSPARENT); SetTextColor(dc,RGB(255,255,255));
  TextOutW(dc,430,30,L"开始",2);
  TextOutW(dc,680,100,L"社交",2); TextOutW(dc,680,170,L"英雄",2);
  TextOutW(dc,680,240,L"商城",2); TextOutW(dc,680,310,L"战令",2);
  TextOutW(dc,510,390,L"自由死斗",4);
  Frame frame{width,height,std::vector<std::uint32_t>(width*height)};
  GdiFlush(); std::memcpy(frame.pixels.data(),pixels,frame.pixels.size()*4);
  SelectObject(dc,previousFont); DeleteObject(font); SelectObject(dc,previous); DeleteObject(bitmap); DeleteDC(dc);
  LocalOcr ocr; OcrFrame words;
  auto directory = (std::filesystem::path(exeDirectory())/(L"vision-integration-data-"+std::to_wstring(GetCurrentProcessId()))).wstring();
  if (!ocr.recognize(frame,directory,words,[] { return true; })) {
    if (ocr.error().find("Chinese OCR is unavailable") != std::string::npos) {
      std::cout << "Chinese OCR language feature unavailable; integration test skipped\n"; return 77;
    }
    throw std::runtime_error(ocr.error());
  }
  SemanticContext context;
  auto observed = recognizeSemantic(frame,words,context);
  if (observed.observation.page != CNPage::Home || observed.observation.step.click.x < .4 ||
      observed.observation.step.click.x > .6 || observed.evidence.size() != 2)
    throw std::runtime_error("Native OCR text/coordinate contract failed");
  auto learned = learnSemantic(frame,observed,context);
  if (!learned) throw std::runtime_error("Native OCR anchors could not be calibrated");
  SemanticCache cache; cache.remember(*learned);
  SemanticCache restored; std::string error;
  if (!parseSemanticCache(writeSemanticCache(cache),restored,error) || !restored.match(frame,context))
    throw std::runtime_error("Native OCR calibration did not survive serialization");
  OcrFrame repeated;
  if (!ocr.recognize(frame,directory,repeated,[] { return true; }) ||
      recognizeSemantic(frame,repeated,context).observation.page != CNPage::Home)
    throw std::runtime_error("Persistent OCR worker could not read a replaced frame: "+ocr.error());
  OcrFrame modeWords;
  if (!readSmallModeText(ocr,frame,directory,modeWords,[] { return true; }))
    throw std::runtime_error("Mode card crop OCR failed: "+ocr.error());
  auto modeTitle=UiText(modeWords).one({L"自由死斗"},{.50,.65,.35,.20},true);
  if (!modeTitle || center(modeTitle->roi).x<.52 || center(modeTitle->roi).x>.72 ||
      center(modeTitle->roi).y<.70 || center(modeTitle->roi).y>.80)
    throw std::runtime_error("Native mode card crop must recover its full title and original-frame coordinates");
  OcrFrame cancelled;
  if (ocr.recognize(frame,directory,cancelled,[] { return false; }))
    throw std::runtime_error("Cancelled OCR request returned an observation");
  ocr.close();
  context.target = L"integration-"+std::to_wstring(GetCurrentProcessId());
  AdaptiveVision adaptive; adaptive.reset(directory);
  auto first = adaptive.recognize(frame,context,[] { return true; });
  auto second = adaptive.recognize(frame,context,[] { return true; });
  if (first.observation.page != CNPage::Home || second.observation.page != CNPage::Home ||
      !semanticStillVisible(frame,frame,second)) {
    std::ostringstream detail;
    detail << "Adaptive page confirmation failed: " << cnPageName(first.observation.page) << '/'
           << cnPageName(second.observation.page) << " visible=" << semanticStillVisible(frame,frame,second)
           << " error=" << adaptive.error();
    for (auto roi : second.evidence) detail << " glyph=" << glyphSimilarity(sample(frame,roi),sample(frame,roi));
    throw std::runtime_error(detail.str());
  }
  if (adaptive.usedOcr && !adaptive.confirm(CNPage::Home)) throw std::runtime_error("Adaptive cache could not be saved");
  adaptive.cancel();
  AdaptiveVision restarted; restarted.reset(directory);
  if (restarted.recognize(frame,context,[] { return true; }).observation.page != CNPage::Home || restarted.usedOcr)
    throw std::runtime_error("Adaptive calibration was not reused after restart");
  restarted.cancel();
  Frame hud{width,height,std::vector<std::uint32_t>(width*height,0xff202838)};
  auto paint = [&](Rect area) {
    for (int y = int(area.y*height); y < int((area.y+area.h)*height); ++y)
      for (int x = int(area.x*width); x < int((area.x+area.w)*width); ++x)
        hud.pixels[std::size_t(y)*width+x] = 0xfff4f4f4;
  };
  paint({.08,.85,.1,.01}); paint({.78,.85,.03,.055});
  auto roomContext = context; roomContext.trustedCustomRoom = true;
  auto liveRoom = recognizeSemantic(hud,OcrFrame{width,height,{}},roomContext);
  if (!liveRoom.liveRoomHud || !semanticStillVisible(hud,hud,liveRoom) ||
      semanticStillVisible(hud,frame,liveRoom))
    throw std::runtime_error("A live target-room HUD did not require current HUD evidence");
  auto roster=hud;std::fill(roster.pixels.begin(),roster.pixels.end(),0xff202838);
  auto fill=[&](Frame &target,Rect area,std::uint32_t color) {
    for (int y=int(area.y*height);y<int((area.y+area.h)*height);++y)
      for (int x=int(area.x*width);x<int((area.x+area.w)*width);++x)
        target.pixels[std::size_t(y)*width+x]=color;
  };
  for (int row=0;row<2;++row) for (int tile=0;tile<20;++tile) {
    const double x=.08+tile*.04,y=.74+row*.058;
    fill(roster,{x,y,.027,.002},0xffeeeeee);fill(roster,{x,y+.050,.027,.002},0xffeeeeee);
  }
  fill(roster,{.47,.89,.06,.055},0xfff87820);
  for (int glyph=0;glyph<6;++glyph) fill(roster,{.488+glyph*.004,.90,.002,.023},0xffeeeeee);
  OcrFrame rosterWords{width,height,{{{{L"继续",{.488,.90,.024,.023}}}}}};
  auto selected=recognizeSemantic(roster,rosterWords,context);
  if (!selected.heroRoster || !semanticStillVisible(roster,roster,selected))
    throw std::runtime_error("Hero roster and actual enabled Continue are not confirmed");
  auto changedRoster=roster;fill(changedRoster,{.05,.72,.90,.15},0xff202838);
  auto disabledRoster=roster;fill(disabledRoster,{.47,.89,.06,.055},0xff555555);
  if (semanticStillVisible(roster,changedRoster,selected) || semanticStillVisible(roster,disabledRoster,selected))
    throw std::runtime_error("Removed hero portraits or disabled Continue must block stale input");
  std::cout << "Native Chinese OCR, coordinates, disk calibration, restart reuse, cancellation, fresh room HUD, fresh hero roster and mode card crop coordinates passed (synthetic image only)\n";
  return 0;
}
int main(int argc,char **argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--self-test") return selfTest();
    if (argc < 2) { std::cerr << "vision-replay image [launcher|custom|deathmatch] [crop-top] [finished|leaving|trusted|words]\n"; return 1; }
    Gdiplus::GdiplusStartupInput input; ULONG_PTR token{};
    if (Gdiplus::GdiplusStartup(&token,&input,nullptr) != Gdiplus::Ok) return 1;
    const auto began = GetTickCount64();
    SemanticContext context;
    std::string scope = argc > 2 ? argv[2] : "custom";
    context.launcher = scope == "launcher";
    context.mode = scope == "deathmatch" ? WorkflowMode::Deathmatch : WorkflowMode::CustomRoom;
    context.finished = argc > 4 && (std::string(argv[4]) == "finished" || std::string(argv[4]) == "leaving");
    context.leaving = argc > 4 && std::string(argv[4]) == "leaving";
    context.endorsementAttempted=argc>4 && std::string(argv[4])=="endorsed";
    const bool trusted = argc > 4 && std::string(argv[4]) == "trusted";
    context.trustedDeathmatch = trusted && context.mode == WorkflowMode::Deathmatch;
    context.trustedCustomRoom = trusted && !context.launcher && context.mode == WorkflowMode::CustomRoom;
    auto frame = loadImage(wide(argv[1]),argc > 3 ? std::stoi(argv[3]) : 0);
    LocalOcr ocr; OcrFrame words;
    auto directory = (std::filesystem::path(exeDirectory())/L"vision-replay-data").wstring();
    if (!ocr.recognize(frame,directory,words,[] { return true; },!context.launcher)) throw std::runtime_error(ocr.error());
    if (argc > 4 && std::string(argv[4]) == "words") {
      for (const auto &line : words.lines) {
        Rect bounds{}; std::wstring label;
        for (const auto &word : line.words) { bounds = bounds.w ? unite(bounds,word.roi) : word.roi; label += word.text; }
        std::cout << utf8(label) << " roi=" << bounds.x << ',' << bounds.y << ',' << bounds.w << ',' << bounds.h << '\n';
      }
    }
    auto result = recognizeSemantic(frame,words,context);
    if (semanticNeedsDetail(context,result)) {
      OcrFrame detail;
      if (ocr.recognize(frame,directory,detail,[] { return true; },!context.launcher,true)) {
        result = recognizeSemanticSupplement(frame,words,detail,context,result);
        if (argc>4 && std::string(argv[4])=="words")
          for (const auto &line:detail.lines) { std::cout << "detail: "; for (const auto &word:line.words) std::cout << utf8(word.text); std::cout << std::endl; }
      }
    }
    if (deathmatchNavigationNeedsDetail(context,result)) {
      OcrFrame cards;
      if (readSmallModeText(ocr,frame,directory,cards,[] { return true; })) {
        if (argc>4 && std::string(argv[4])=="words")
          for (const auto &line:cards.lines) { std::cout << "cards: "; for (const auto &word:line.words) std::cout << utf8(word.text); std::cout << std::endl; }
        result=recognizeSemanticSupplement(frame,words,cards,context,result);
      }
    }
    if (deathmatchNavigationNeedsDetail(context,result)) {
      OcrFrame navigation;
      if (readSmallNavigationText(ocr,frame,directory,navigation,[] { return true; })) {
        if (argc > 4 && std::string(argv[4]) == "words")
          for (const auto &line:navigation.lines) { std::cout << "navigation: "; for (const auto &word:line.words) std::cout << utf8(word.text); std::cout << std::endl; }
        result=recognizeSemanticSupplement(frame,words,navigation,context,result);
      }
    }
    if (customRoomNeedsDetail(context,result)) {
      OcrFrame rooms;
      if (readCustomRoomText(ocr,frame,directory,rooms,[] {return true;})) {
        if (argc>4 && std::string(argv[4])=="words")
          for (const auto &line:rooms.lines) {std::cout<<"rooms: ";for(const auto &word:line.words)std::cout<<utf8(word.text);std::cout<<std::endl;}
        result=recognizeSemanticSupplement(frame,words,rooms,context,result);
      }
    }
    if (heroSelectionNeedsDetail(context,result)) {
      OcrFrame selection;
      if (readSmallHeroText(ocr,frame,directory,selection,[] { return true; })) {
        if (argc>4 && std::string(argv[4])=="words")
          for (const auto &line:selection.lines) { std::cout << "hero: "; for (const auto &word:line.words) std::cout << utf8(word.text); std::cout << std::endl; }
        auto combined=words;combined.lines.insert(combined.lines.end(),selection.lines.begin(),selection.lines.end());
        const auto recovered=recognizeSemantic(frame,combined,context);
        if (recovered && recovered.observation.page==CNPage::Hero) result=recovered;
      } else { std::cerr << "hero_ocr_error=" << ocr.error() << std::endl; }
    }
    std::cout << "page=" << cnPageName(result.observation.page) << " action=" << cnActionName(result.observation.step.action)
              << " click=" << result.observation.step.click.x << ',' << result.observation.step.click.y
              << " anchors=" << result.evidence.size() << " ms=" << GetTickCount64()-began << '\n';
    if (result && !semanticStillVisible(frame,frame,result))
      throw std::runtime_error("Recognized page fails the fresh evidence check even on an unchanged frame");
    if (result) std::cout << "fresh_evidence=passed room_instances=" << result.observation.cards.size() << '\n';
    auto learned = learnSemantic(frame,result,context);
    if (learned) {
      SemanticCache cache; cache.remember(*learned);
      SemanticCache restored; std::string error;
      if (!parseSemanticCache(writeSemanticCache(cache),restored,error) || !restored.match(frame,context)) throw std::runtime_error("Cache roundtrip failed");
      std::cout << "cache_roundtrip=passed\n";
    }
    Gdiplus::GdiplusShutdown(token);
    return result ? 0 : 2;
  } catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
