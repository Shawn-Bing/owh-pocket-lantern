#pragma once
#include "core/core.hpp"
#include <optional>
namespace owh {
// Only the complete title glyphs from the user supplied 1024x576 client crop.
// No account, desktop pixels or OCR strings are stored in this template.
inline constexpr const char *RoomTitleMask[] = {
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
"00001000011001000100000111111000010011100000110000000011111100011011000011000100001001000001010011100",
"00001011110001000100000100001000011110100001111000000011111100111011110111111110001011111001011111000",
"00011101100001000101100111111000011111110011101100000011111100010111110011011011011111111001011100000",
"00001001100001111110000111111000010110001110110111000111111110111100010011111001101011111001111111100",
"00001111111001000100000111111101111010110111111110000111111110011111110011111011001110101001001110100",
"00111001100001000100001110111101001011100000000110000111111110111111110001101100011111111101111111100",
"00001001101001000101101000100101001011010011001000000111111110110111110111111001001111111101011011000",
"00001001101001111101101111111101111111010001110000000111111100010110011001101001001001011001011011000",
"00011001111001100111001100100001001101110000011000000100111110010111111001101111011110001111011110100",
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
};
inline constexpr const char *RoomTitleMaskSecond[] = {
"00000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000",
"00001001111001000100000111111000010011100000110000000011111100111111110011000100001001110001011111100",
"00011111100001000100100111111000011110100001111000000011111100110110110111111010011111010001011100000",
"00011101100001111111100111111000010111110111101110000011111100111111110011111011001011111001111111100",
"00001001111101110110000111111000010010110100010010000111111110111111110011111001001010101001001111100",
"00011111110001000100001111111101111011100111111110000111111110011000010011111111011110101001111110100",
"00011001100001000100001000100101001011100010001100000111111110111111110001101100001111111101011111100",
"00001001101001000101101000100101001011010011111000000111101100110111111111111001001001110001011011000",
"00001001101001111111101111111101111111010000110000000111111110010110011001101111001011011001011111100",
"00011000111001000000000000100001001000100000001000000000110010010011110001101111011010001101011100100",
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
};
inline constexpr const char *RoomRefreshMask[] = {
"00000000000000000000000000000000",
"00000000000000000000000000000000",
"00000000000000000000000000000000",
"00000000000000000100000000000000",
"00000000000000001111110000000000",
"00000000000111001111110000000000",
"00000000000111100100110000000000",
"00000000000100000000110000000000",
"00000000000100000000110000000000",
"00000000000100000000110000000000",
"00000000000100100111110000000000",
"00000000000111110011110000000000",
"00000000000111110000000000000000",
"00000000000000100000000000000000",
"00000000000000000000000000000000",
"00000000000000000000000000000000",
"00000000000000000000000000000000",
"00000000000000000000000000000000"};
inline bool roomTemplateScale(const Frame &f) {
  return f.width>=640 && f.width<=2048 && f.height>=360 &&
      std::abs(double(f.width)/f.height/(16./9)-1)<.02 && f.pixels.size()==std::size_t(f.width)*f.height;
}
// Foreground IoU compares both missing and extra glyphs; blank background
// cannot dominate the score. Sampling is always in normalized client space.
inline double roomMaskScore(const Frame &f,Rect box,const char *const *mask,int w,int h,int threshold) {
  if (!roomTemplateScale(f) || !valid(box)) return 0;
  int intersection{},combined{};
  for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
    auto p=f.pixels[std::size_t(std::clamp(int((box.y+(y+.5)*box.h/h)*f.height),0,f.height-1))*f.width+
        std::clamp(int((box.x+(x+.5)*box.w/w)*f.width),0,f.width-1)];
    bool actual=std::min({(p>>16)&255,(p>>8)&255,p&255})>unsigned(threshold), expected=mask[y][x]=='1';
    intersection+=actual&&expected;combined+=actual||expected;
  }
  return combined?double(intersection)/combined:0;
}
inline double roomTitleScore(const Frame &f,Rect box,const char *const *mask) {
  auto total=roomMaskScore(f,box,mask,101,12,100);
  if(total<.94)return 0;
  // A changed single character cannot hide inside the other nine characters.
  for(int group=0;group<10;++group){
    int start=group*10,end=group==9?101:start+10,hit{},combined{};
    for(int y=0;y<12;++y)for(int x=start;x<end;++x){
      auto p=f.pixels[std::size_t(int((box.y+(y+.5)*box.h/12)*f.height))*f.width+int((box.x+(x+.5)*box.w/101)*f.width)];
      bool actual=std::min({(p>>16)&255,(p>>8)&255,p&255})>100,expected=mask[y][x]=='1';
      hit+=actual&&expected;combined+=actual||expected;
    }
    if(combined && double(hit)/combined<.90)return 0;
  }
  return total;
}
inline std::optional<Rect> roomTitleGlyph(const Frame &f,double rowY) {
  if(!roomTemplateScale(f)) return {};
  Rect best{};double score=.94;
  for(int dy=-3;dy<=3;++dy) for(int x=185;x<=210;++x) {
    Rect box{double(x)/1024,double(int((rowY-6./576)*f.height)+dy)/f.height,101./1024,12./576};
    auto value=std::max(roomTitleScore(f,box,RoomTitleMask),roomTitleScore(f,box,RoomTitleMaskSecond));
    if(value>score) {score=value;best=box;}
  }
  if(!valid(best)) return {};
  // The entire title column must end here: reject prefix/suffix additions,
  // overlapping text, and truncation instead of granting a substring match.
  int extra{};
  for(int y=int(best.y*f.height);y<int((best.y+best.h)*f.height);++y)
    for(int x=int(.17*f.width);x<int(.47*f.width);++x) {
      if(x>=int(best.x*f.width)-1 && x<=int((best.x+best.w)*f.width)+1) continue;
      auto p=f.pixels[std::size_t(y)*f.width+x];
      extra+=std::min({(p>>16)&255,(p>>8)&255,p&255})>100;
    }
  if(extra>int(8.*f.width*f.height/(1024.*576))) return {};
  return best;
}
inline std::optional<Rect> roomRefreshGlyph(const Frame &f,Rect header) {
  if(!roomTemplateScale(f)) return {};
  // Verified screenshot: control above the table, aligned with occupancy.
  // Search near that structural relation, never a desktop absolute point.
  Rect best{};double score=.90;
  for(int dy=-8;dy<=8;++dy) for(int dx=-10;dx<=10;++dx) {
    Rect box{(800.+dx)/1024,header.y-30./576+double(dy)/f.height,32./1024,18./576};
    auto value=roomMaskScore(f,box,RoomRefreshMask,32,18,210);
    if(value<=score) continue;
    int orange{},total{};
    for(int y=0;y<18;++y) for(int x=0;x<32;++x) {
      auto p=f.pixels[std::size_t(int((box.y+(y+.5)*box.h/18)*f.height))*f.width+int((box.x+(x+.5)*box.w/32)*f.width)];
      int r=(p>>16)&255,g=(p>>8)&255,b=p&255;
      orange+=r>190&&g>40&&g<180&&b<120;++total;
    }
    if(orange>total*.55){score=value;best=box;}
  }
  return valid(best)?std::optional<Rect>(best):std::nullopt;
}
class RoomListRefresh {
  Millis since_{},last_{},next_{}; unsigned count_{},failures_{};
  bool unknownExhausted_{};
public:
  void reset(){*this={};}
  void observe(bool roomList,bool joinable,bool unknown,bool exhausted){
    if(!roomList||joinable)unknownExhausted_=false;
    else if(unknown&&exhausted)unknownExhausted_=true;
  }
  bool unknownExhausted()const{return unknownExhausted_;}
  bool due(bool roomList,bool candidate,bool joining,Millis now) {
    if(!roomList){since_=0;return false;}
    if(!since_) since_=now;
    return (!candidate||unknownExhausted_)&&!joining&&now>=since_&&now-since_>=15000&&now>=next_;
  }
  void acted(bool ok,Millis now){if(ok)unknownExhausted_=false;last_=now;++count_;failures_=ok?0:std::min(3u,failures_+1);next_=now+15000*(1u<<failures_);}
  Millis last()const{return last_;} unsigned count()const{return count_;}
};
} // namespace owh
