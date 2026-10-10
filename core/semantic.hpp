#pragma once
#include "core/cn_profile.hpp"
#include "core/deathmatch.hpp"
#include <optional>

namespace owh {
struct OcrWord { std::wstring text; Rect roi; };
struct OcrLine { std::vector<OcrWord> words; };
struct OcrFrame { int width{}, height{}; std::vector<OcrLine> lines; };
inline bool parseOcrFrame(const std::string &source, OcrFrame &out, std::string &error) {
  Json root;
  if (source.size() > 512 * 1024 || !parseJson(source, root, error)) return false;
  const auto *failure = root.find("error"), *width = root.find("width"),
             *height = root.find("height"), *lines = root.find("lines");
  if (failure && failure->is(Json::Kind::String) && !failure->text.empty())
    return error = failure->text, false;
  if (!width || !height || !width->is(Json::Kind::Number) || !height->is(Json::Kind::Number) ||
      width->number < 1 || height->number < 1 || width->number > 16384 || height->number > 16384 ||
      !lines || !lines->is(Json::Kind::Array) || lines->items.size() > 512)
    return error = "invalid OCR frame", false;
  OcrFrame parsed{int(width->number), int(height->number), {}};
  std::size_t count{};
  for (const auto &line : lines->items) {
    const auto *words = line.find("words");
    if (!words || !words->is(Json::Kind::Array) || words->items.size() > 256)
      return error = "invalid OCR line", false;
    OcrLine result;
    for (const auto &word : words->items) {
      const auto *text = word.find("text"), *roi = word.find("roi");
      OcrWord value;
      if (++count > 4096 || !text || !text->is(Json::Kind::String) || text->text.size() > 512 ||
          !roi || !detail::readRect(*roi, value.roi))
        return error = "invalid OCR word", false;
      value.text = utf8ToWide(text->text);
      result.words.push_back(std::move(value));
    }
    parsed.lines.push_back(std::move(result));
  }
  out = std::move(parsed);
  return true;
}
inline std::wstring normalizeUiText(const std::wstring &text) {
  std::wstring result;
  for (auto c : text) {
    if ((c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'z') || (c >= 0x4e00 && c <= 0x9fff))
      result += c;
    else if (c >= L'A' && c <= L'Z') result += wchar_t(c - L'A' + L'a');
  }
  return result;
}
inline Point center(Rect r) { return {r.x + r.w / 2, r.y + r.h / 2}; }
inline Rect unite(Rect a, Rect b) {
  double right = std::max(a.x + a.w, b.x + b.w), bottom = std::max(a.y + a.h, b.y + b.h);
  a.x = std::min(a.x, b.x); a.y = std::min(a.y, b.y);
  a.w = right - a.x; a.h = bottom - a.y;
  return a;
}
inline bool contains(Rect zone, Point p) {
  return p.x >= zone.x && p.x <= zone.x + zone.w && p.y >= zone.y && p.y <= zone.y + zone.h;
}
struct TextHit { Rect roi; std::wstring lineText; Rect lineRoi; };
class UiText {
  const OcrFrame &frame_;
public:
  explicit UiText(const OcrFrame &frame) : frame_(frame) {}
  std::vector<TextHit> find(const std::wstring &label, Rect zone = {0, 0, 1, .88}, bool exact = false) const {
    const auto needle = normalizeUiText(label);
    std::vector<TextHit> result;
    if (needle.empty()) return result;
    for (const auto &line : frame_.lines) {
      std::wstring text;
      std::vector<std::pair<std::size_t, std::size_t>> offsets;
      for (const auto &word : line.words) {
        auto start = text.size(); text += normalizeUiText(word.text);
        offsets.emplace_back(start, text.size());
      }
      if (exact && text != needle) continue;
      auto start = text.find(needle);
      if (start == std::wstring::npos) continue;
      Rect bounds{};
      for (std::size_t i = 0; i < offsets.size(); ++i)
        if (offsets[i].first < start + needle.size() && offsets[i].second > start)
          bounds = bounds.w ? unite(bounds, line.words[i].roi) : line.words[i].roi;
      if (valid(bounds) && contains(zone, center(bounds))) {
        auto p = center(bounds);
        bool duplicate = std::any_of(result.begin(),result.end(),[&](TextHit hit) {
          auto q = center(hit.roi); return std::abs(p.x-q.x) < .02 && std::abs(p.y-q.y) < .02;
        });
        Rect lineBounds{};
        for (const auto &word : line.words) lineBounds = lineBounds.w ? unite(lineBounds,word.roi) : word.roi;
        if (!duplicate) result.push_back({bounds,text,lineBounds});
      }
    }
    return result;
  }
  std::optional<TextHit> one(std::initializer_list<const wchar_t *> aliases,
                            Rect zone = {0, 0, 1, .88}, bool exact = false) const {
    std::vector<TextHit> hits;
    auto encloses = [](Rect outer, Rect inner) {
      return outer.x <= inner.x + .00001 && outer.y <= inner.y + .00001 &&
             outer.x + outer.w >= inner.x + inner.w - .00001 &&
             outer.y + outer.h >= inner.y + inner.h - .00001;
    };
    for (auto label : aliases)
      for (const auto &hit : find(label, zone, exact)) {
        auto p = center(hit.roi);
        auto duplicate = std::find_if(hits.begin(), hits.end(), [&](const TextHit &h) {
          bool sameLine = h.lineText == hit.lineText &&
                          std::abs(h.lineRoi.x-hit.lineRoi.x) < .00001 &&
                          std::abs(h.lineRoi.y-hit.lineRoi.y) < .00001 &&
                          std::abs(h.lineRoi.w-hit.lineRoi.w) < .00001 &&
                          std::abs(h.lineRoi.h-hit.lineRoi.h) < .00001;
          if (sameLine && (encloses(h.roi,hit.roi) || encloses(hit.roi,h.roi))) return true;
          auto q = center(h.roi);
          double w = std::max(0.,std::min(hit.roi.x+hit.roi.w,h.roi.x+h.roi.w)-std::max(hit.roi.x,h.roi.x));
          double height = std::max(0.,std::min(hit.roi.y+hit.roi.h,h.roi.y+h.roi.h)-std::max(hit.roi.y,h.roi.y));
          return std::abs(p.x-q.x) < .025 && std::abs(p.y-q.y) < .025 &&
                 w > std::min(hit.roi.w,h.roi.w)*.5 && height > std::min(hit.roi.h,h.roi.h)*.5;
        });
        if (duplicate == hits.end()) hits.push_back(hit);
        else if (hit.roi.w*hit.roi.h > duplicate->roi.w*duplicate->roi.h) *duplicate = hit;
      }
    return hits.size() == 1 ? std::optional<TextHit>(hits.front()) : std::nullopt;
  }
  std::optional<TextHit> first(std::initializer_list<const wchar_t *> aliases,
                              Rect zone = {0,0,1,.88}) const {
    for (auto label : aliases) {
      auto hits = find(label,zone);
      if (!hits.empty()) return hits.front();
    }
    return {};
  }
};
// A mode can appear as several rooms. Distinguish its complete title from
// different modes that merely contain the configured search phrase.
inline std::vector<TextHit> roomCandidates(const UiText &text, const std::wstring &target) {
  auto hits = text.find(target,{0,.27,.96,.57});
  std::vector<TextHit> titles;
  for (auto hit : hits) {
    if (!valid(hit.lineRoi) || hit.lineRoi.h < .021 || hit.lineText.size() > 64) continue;
    hit.roi = hit.lineRoi;
    bool duplicate = std::any_of(titles.begin(),titles.end(),[&](const TextHit &other) {
      double w = std::max(0.,std::min(hit.roi.x+hit.roi.w,other.roi.x+other.roi.w)-std::max(hit.roi.x,other.roi.x));
      double h = std::max(0.,std::min(hit.roi.y+hit.roi.h,other.roi.y+other.roi.h)-std::max(hit.roi.y,other.roi.y));
      return w > std::min(hit.roi.w,other.roi.w)*.5 && h > std::min(hit.roi.h,other.roi.h)*.5;
    });
    if (!duplicate) titles.push_back(std::move(hit));
  }
  const auto wanted = normalizeUiText(target);
  bool exact = std::any_of(titles.begin(),titles.end(),[&](const TextHit &hit) { return hit.lineText == wanted; });
  if (exact) std::erase_if(titles,[&](const TextHit &hit) { return hit.lineText != wanted; });
  std::sort(titles.begin(),titles.end(),[](const TextHit &a,const TextHit &b) {
    auto pa = center(a.roi), pb = center(b.roi);
    int rowA = int(std::round(pa.y/.05)), rowB = int(std::round(pb.y/.05));
    if (rowA != rowB) return rowA < rowB;
    return pa.x != pb.x ? pa.x < pb.x : pa.y < pb.y;
  });
  return titles;
}
struct RoomCapacity { int players{}, maximum{}; };
inline std::optional<RoomCapacity> parseRoomCapacity(const std::wstring &raw) {
  std::wstring value;
  for (auto c:raw) {
    if (c==L' ' || c==L'\t' || c==L'\n') continue;
    if (c>=L'０' && c<=L'９') c=L'0'+(c-L'０');
    if (c==L'／') c=L'/';
    value+=c;
  }
  const auto slash=value.find(L'/');
  if (slash==std::wstring::npos || slash==0 || slash>3 || value.size()-slash-1==0 || value.size()-slash-1>3) return {};
  RoomCapacity result;
  for (std::size_t i=0;i<value.size();++i) {
    if (i==slash) continue;
    if (value[i]<L'0' || value[i]>L'9') return {};
    auto &number=i<slash ? result.players : result.maximum;
    number=number*10+int(value[i]-L'0');
  }
  if (result.maximum<1 || result.maximum>128 || result.players>result.maximum) return {};
  return result;
}
struct VacantRoomRow { Rect count, title; RoomCapacity capacity; bool conflicting{}; };
inline std::vector<VacantRoomRow> vacantRoomRows(const OcrFrame &ocr) {
  std::vector<VacantRoomRow> rows;
  for (const auto &line:ocr.lines) {
    std::wstring raw; Rect bounds{};
    for (const auto &word:line.words) {
      if (!contains({.65,.26,.25,.56},center(word.roi))) continue;
      raw+=word.text; bounds=bounds.w ? unite(bounds,word.roi) : word.roi;
    }
    auto capacity=parseRoomCapacity(raw);
    if (!capacity || !valid(bounds)) continue;
    auto found=std::find_if(rows.begin(),rows.end(),[&](const auto &row) {
      return std::abs(center(row.count).y-center(bounds).y)<std::max(.005,std::min(row.count.h,bounds.h)*.5);
    });
    if (found!=rows.end()) {
      if (found->capacity.players!=capacity->players || found->capacity.maximum!=capacity->maximum) found->conflicting=true;
      continue;
    }
    Rect title{};
    for (const auto &candidate:ocr.lines) for (const auto &word:candidate.words) {
      if (word.text.empty() || !contains({.12,.26,.36,.56},center(word.roi)) ||
          std::abs(center(word.roi).y-center(bounds).y)>std::max(.008,bounds.h*.7)) continue;
      title=title.w ? unite(title,word.roi) : word.roi;
    }
    rows.push_back({bounds,title,*capacity,false});
  }
  std::erase_if(rows,[](const auto &row) { return row.conflicting || !valid(row.title) || row.capacity.players>=row.capacity.maximum; });
  std::stable_sort(rows.begin(),rows.end(),[](const auto &a,const auto &b) {
    const int spareA=a.capacity.maximum-a.capacity.players,spareB=b.capacity.maximum-b.capacity.players;
    return spareA!=spareB ? spareA>spareB : center(a.count).y<center(b.count).y;
  });
  return rows;
}
struct SemanticContext {
  bool launcher{};
  WorkflowMode mode{WorkflowMode::CustomRoom};
  std::wstring target{L"最稳轮换版"};
  bool finished{}, leaving{};
  bool trustedDeathmatch{};
  // Runtime evidence from a verified target join or an established room.
  // Never loaded from the adaptive cache or inferred from a generic HUD.
  bool trustedCustomRoom{};
  bool endorsementAttempted{};
  int endorsementClicks{};
};
struct SemanticObservation {
  CNObservation observation;
  std::vector<Rect> evidence;
  Rect button;
  bool coloredButton{};
  bool verifiedDeathmatch{};
  bool liveRoomHud{};
  bool heroRoster{};
  explicit operator bool() const { return bool(observation); }
};
// Check the actual button background. Recognized grey/disabled text must not
// trigger a launch or a repeated join click. This works for both orange game
// buttons and Battle.net's blue button, independent of its position.
inline bool enabledUiButton(const Frame &frame, Rect text) {
  if (!valid(text) || frame.width <= 0 || frame.height <= 0 ||
      frame.pixels.size() != std::size_t(frame.width) * frame.height) return false;
  Rect area{std::max(0., text.x - .012), std::max(0., text.y - .008), 0, 0};
  area.w = std::min(1., text.x + text.w + .012) - area.x;
  area.h = std::min(1., text.y + text.h + .008) - area.y;
  int colored{}, total{};
  for (int y = int(area.y * frame.height); y < int((area.y + area.h) * frame.height); y += 2)
    for (int x = int(area.x * frame.width); x < int((area.x + area.w) * frame.width); x += 2) {
      auto p = frame.pixels[std::size_t(y) * frame.width + x];
      int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
      colored += (r > 190 && g > 40 && g < 185 && b < 115) || (b > 130 && g > 65 && b - r > 55);
      ++total;
    }
  return total && double(colored) / total >= .18;
}
inline bool enabledResultButton(const Frame &frame,Rect roi) {
  if (enabledUiButton(frame,roi)) return true;
  roi.x=std::max(0.,roi.x-.012);roi.y=std::max(0.,roi.y-.008);
  roi.w=std::min(1.-roi.x,roi.w+.024);roi.h=std::min(1.-roi.y,roi.h+.016);
  int light=0,total=0;
  for (int y=int(roi.y*frame.height);y<int((roi.y+roi.h)*frame.height);++y)
    for (int x=int(roi.x*frame.width);x<int((roi.x+roi.w)*frame.width);++x) {
      const auto p=frame.pixels[std::size_t(y)*frame.width+x];
      light+=std::min({(p>>16)&255,(p>>8)&255,p&255})>210;++total;
    }
  return total && double(light)/total>.35;
}
inline std::vector<Rect> endorsementCards(const Frame &frame) {
  std::vector<Rect> cards;
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return cards;
  const int x0=int(frame.width*.10),y0=int(frame.height*.78),w=int(frame.width*.80),h=int(frame.height*.16);
  std::vector<unsigned char> seen(std::size_t(w)*h);
  auto light=[&](int x,int y) {
    const auto p=frame.pixels[std::size_t(y+y0)*frame.width+x+x0];
    return std::min({(p>>16)&255,(p>>8)&255,p&255})>210;
  };
  for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
    const int start=y*w+x;
    if (seen[start] || !light(x,y)) continue;
    std::vector<int> queue{start};seen[start]=1;
    int left=x,right=x,top=y,bottom=y;
    for (std::size_t i=0;i<queue.size();++i) {
      const int px=queue[i]%w,py=queue[i]/w;
      left=std::min(left,px);right=std::max(right,px);top=std::min(top,py);bottom=std::max(bottom,py);
      for (Point delta:{Point{-1,0},Point{1,0},Point{0,-1},Point{0,1}}) {
        const int nx=px+int(delta.x),ny=py+int(delta.y);
        if (nx<0 || nx>=w || ny<0 || ny>=h) continue;
        const int at=ny*w+nx;
        if (!seen[at] && light(nx,ny)) { seen[at]=1;queue.push_back(at); }
      }
    }
    Rect box{double(x0+left)/frame.width,double(y0+top)/frame.height,
             double(right-left+1)/frame.width,double(bottom-top+1)/frame.height};
    if (box.w<.08 || box.w>.30 || box.h<.045 || box.h>.13 ||
        queue.size()<std::size_t(frame.width*frame.height*.002)) continue;
    // The orange Already Endorsed badge excludes that card. Available cards
    // have a dark button and white nameplate; their bounds supply the click.
    if (colorFraction(frame,box,false)>.035) continue;
    cards.push_back(box);
  }
  std::sort(cards.begin(),cards.end(),[](Rect a,Rect b){return a.x<b.x;});
  return cards;
}
inline std::optional<TextHit> overwatchBrand(const Frame &frame) {
  if (frame.width < 400 || frame.height < 300 || frame.pixels.size() != std::size_t(frame.width)*frame.height) return {};
  // The large product emblem is stable even when its decorative title cannot
  // be OCR'd. Check the orange upper arc, white lower ring and dark interior.
  // Tiny library-tab icons are excluded: an unselected tab cannot prove which
  // product the large Play button belongs to.
  constexpr Point orange[]{{.5,.08},{.32,.12},{.68,.12},{.24,.18},{.76,.18}};
  constexpr Point white[]{{.12,.50},{.88,.50},{.16,.70},{.84,.70},{.28,.84},{.72,.84},{.50,.91}};
  constexpr Point dark[]{{.5,.26},{.29,.48},{.71,.48}};
  auto color = [&](int x,int y,int diameter,Point p,int type) {
    auto pixel = frame.pixels[std::size_t(y+int(p.y*diameter))*frame.width+x+int(p.x*diameter)];
    int r = (pixel>>16)&255, g = (pixel>>8)&255, b = pixel&255;
    if (type == 0) return r > 200 && g > 50 && g < 180 && b < 105;
    if (type == 1) return std::min({r,g,b}) > 180 && std::max({r,g,b})-std::min({r,g,b}) < 55;
    return std::max({r,g,b}) < 150;
  };
  const int maximum = std::min({512,frame.width/2,int(frame.height*.65)});
  for (int diameter = 72; diameter <= maximum; diameter += 8)
    for (int y = int(frame.height*.08); y+diameter < int(frame.height*.8); y += std::max(3,diameter/24))
      for (int x = 0; x+diameter < int(frame.width*.7); x += std::max(3,diameter/24)) {
        if (!color(x,y,diameter,orange[0],0)) continue;
        int oranges{}, whites{}, darks{};
        for (auto p : orange) oranges += color(x,y,diameter,p,0);
        if (oranges < 4) continue;
        for (auto p : white) whites += color(x,y,diameter,p,1);
        if (whites < 6) continue;
        for (auto p : dark) darks += color(x,y,diameter,p,2);
        if (darks >= 2) return TextHit{{double(x)/frame.width,double(y)/frame.height,
                                      double(diameter)/frame.width,double(diameter)/frame.height}};
      }
  return {};
}
// Two rows of compact portraits have four aligned horizontal border bands.
// Require many distinct tiles across most of the client, rather than a single
// white line or remembered hero artwork. All distances scale with the frame.
inline std::optional<Rect> heroPortraitGrid(const Frame &frame) {
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return {};
  struct Band { int y,left,right; };
  std::vector<Band> bands;
  for (int y=int(frame.height*.72);y<int(frame.height*.87);++y) {
    int start=-1,count=0,total=0,left=frame.width,right=0;
    for (int x=int(frame.width*.05);x<=int(frame.width*.95);++x) {
      const auto p=frame.pixels[std::size_t(y)*frame.width+x];
      const int r=(p>>16)&255,g=(p>>8)&255,b=p&255;
      const bool light=x<int(frame.width*.95) && std::min({r,g,b})>180 && std::max({r,g,b})-std::min({r,g,b})<55;
      if (light && start<0) start=x;
      if (!light && start>=0) {
        const int length=x-start;
        if (length>=frame.width*.018 && length<=frame.width*.045) {
          ++count;total+=length;left=std::min(left,start);right=x;
        }
        start=-1;
      }
    }
    if (count<12 || total<frame.width*.32 || right-left<frame.width*.60) continue;
    if (!bands.empty() && y-bands.back().y<=std::max(2,int(frame.height*.005))) continue;
    bands.push_back({y,left,right});
  }
  for (std::size_t i=0;i+3<bands.size();++i) {
    const auto &a=bands[i],&b=bands[i+1],&c=bands[i+2],&d=bands[i+3];
    const auto height=[&](int pixels) { return pixels>=frame.height*.035 && pixels<=frame.height*.075; };
    if (!height(b.y-a.y) || !height(d.y-c.y) || c.y-b.y>frame.height*.025 ||
        std::abs(a.left-c.left)>frame.width*.08 || std::abs(a.right-c.right)>frame.width*.08) continue;
    return Rect{double(std::min(a.left,c.left))/frame.width,double(a.y)/frame.height,
                double(std::max(a.right,c.right)-std::min(a.left,c.left))/frame.width,double(d.y-a.y+1)/frame.height};
  }
  return {};
}
inline bool combatHud(const Frame &frame) {
  if (frame.width < 640 || frame.height < 360 ||
      frame.pixels.size() != std::size_t(frame.width) * frame.height) return false;
  // A health bar has a long, nearly white horizontal run in the lower left.
  // Ability icons must also be present. OCR supplies the deathmatch score/title
  // separately; this shape alone never authorizes random input.
  int longest{}, run{}, gaps{}, bright{};
  for (int y = int(frame.height * .79); y < int(frame.height * .94); y += 2) {
    run = gaps = bright = 0;
    for (int x = int(frame.width * .035); x < int(frame.width * .25); ++x) {
      auto p = frame.pixels[std::size_t(y) * frame.width + x];
      int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
      if (r > 195 && g > 195 && b > 195 && std::max({r,g,b}) - std::min({r,g,b}) < 30) {
        ++run; ++bright; gaps = 0;
      } else if (run && ++gaps <= std::max(2,frame.width/400)) ++run;
      else run = gaps = bright = 0;
      if (bright > run*.55) longest = std::max(longest,run);
    }
  }
  int white{}, total{};
  for (int y = int(frame.height * .78); y < int(frame.height * .96); y += 3)
    for (int x = int(frame.width * .72); x < int(frame.width * .98); x += 3) {
      auto p = frame.pixels[std::size_t(y) * frame.width + x];
      white += ((p >> 16) & 255) > 190 && ((p >> 8) & 255) > 190 && (p & 255) > 190;
      ++total;
    }
  return longest >= frame.width * .025 && total && double(white) / total > .015;
}
inline bool deathmatchScoreboard(const Frame &frame) {
  if (frame.width<640 || frame.height<360 || frame.pixels.size()!=std::size_t(frame.width)*frame.height) return false;
  struct Band { int rows{},top{},bottom{},left{},right{}; } blue,red;
  for (int y=int(frame.height*.045);y<int(frame.height*.24);++y) {
    for (int team=0;team<2;++team) {
      int count{},left=frame.width,right{};
      for (int x=int(frame.width*.24);x<int(frame.width*.77);++x) {
        auto p=frame.pixels[std::size_t(y)*frame.width+x];int r=(p>>16)&255,g=(p>>8)&255,b=p&255;
        bool color=team==0 ? (b>110&&g>65&&b-r>65&&g-r>35) : (r>105&&r-g>60&&r-b>30);
        if (color) {++count;left=std::min(left,x);right=x;}
      }
      if (count<frame.width*.14 || right-left>frame.width*.45 || right-left<frame.width*.16) continue;
      auto &band=team==0 ? blue : red;
      if (!band.rows) {band.top=y;band.left=left;band.right=right;}
      ++band.rows;band.bottom=y;band.left=std::min(band.left,left);band.right=std::max(band.right,right);
    }
  }
  auto valid=[&](const Band &b) {return b.rows>=frame.height*.018 && b.bottom-b.top<=frame.height*.055;};
  const bool separated=(red.top>blue.bottom && red.top-blue.bottom<frame.height*.025) ||
                       (blue.top>red.bottom && blue.top-red.bottom<frame.height*.025);
  return valid(blue)&&valid(red)&&separated&&
         std::abs(blue.left-red.left)<frame.width*.025&&std::abs(blue.right-red.right)<frame.width*.025;
}
inline SemanticObservation recognizeSemantic(const Frame &frame, const OcrFrame &ocr,
                                             const SemanticContext &context) {
  if (frame.width != ocr.width || frame.height != ocr.height) return {};
  UiText text(ocr);
  const bool deathmatch = context.mode == WorkflowMode::Deathmatch;
  auto make = [&](CNPage page, Action action, const wchar_t *label, TextHit anchor,
                  std::optional<TextHit> button = {}, bool colored = false) {
    SemanticObservation result;
    auto &o = result.observation;
    o.page = page; o.score = .99; o.step.launcher = context.launcher;
    o.step.action = action; o.step.label = label;
    result.evidence.push_back(anchor.roi);
    if (button) {
      o.step.click = center(button->roi); result.button = button->roi;
      if (anchor.roi.x != button->roi.x || anchor.roi.y != button->roi.y) result.evidence.push_back(button->roi);
    }
    result.coloredButton = colored;
    if (colored && button && !enabledUiButton(frame, button->roi)) {
      o.step.action = Action::Wait;
      o.step.label = L"按钮暂不可用，等待更新、加载或可加入的房间";
    }
    return result;
  };
  // Account forms are observed, never filled. Saved Battle.net sessions and
  // reconnect buttons can be resumed without storing account secrets.
  auto password = text.one({L"密码", L"password"}, {.12,.15,.76,.65});
  auto login = text.one({L"登录", L"登入", L"log in", L"sign in"}, {.12,.15,.76,.65}, true);
  if (password) {
    if (context.launcher) return make(CNPage::Login,Action::Ignore,L"需要手动登录或验证",*password,login);
    if (login) return make(CNPage::LoginError,Action::Restart,L"游戏登录页面：关闭并重新启动游戏",*password,login);
  }
  if (!context.launcher && login) {
    if (auto account=text.one({L"电子邮箱",L"账号",L"账户",L"email"},{.12,.15,.76,.65}))
      return make(CNPage::LoginError,Action::Restart,L"游戏登录页面：关闭并重新启动游戏",*account,login);
  }
  if (auto error = text.one({L"连接战网服务超时", L"连接超时", L"与服务器的连接中断", L"连接已断开",
                             L"正在搜索游戏服务器", L"无法连接至游戏服务器", L"无法让您登录", L"无法让", L"无法登录"}, {.15,.18,.7,.65})) {
    if (context.launcher)
      if (auto retry = text.one({L"重新连接",L"重新连线",L"reconnect"}, {.1,.2,.8,.7}, true))
        return make(CNPage::Login,Action::Click,L"战网重新连接",*error,retry);
    return make(CNPage::LoginError, context.launcher ? Action::Ignore : Action::Restart,
                L"登录或连接错误", *error, text.one({L"重试", L"登录", L"重新连接"}, {.15,.35,.7,.5}, true));
  }
  if (context.launcher) {
    std::optional<TextHit> brand;
    // The unselected library rail also names installed games. Only a large
    // product heading or emblem can bind the main Play button to Overwatch.
    for (auto alias : {L"守望先锋", L"overwatch"})
      for (auto hit : text.find(alias,{.25,.20,.6,.56}))
        if (hit.roi.h >= .04 && hit.roi.w >= .15) { brand = hit; break; }
    if (!brand) brand = overwatchBrand(frame);
    auto reconnect = text.one({L"重新连接", L"重新连线", L"reconnect"}, {.1,.2,.8,.7}, true);
    if (reconnect && text.first({L"连接", L"离线", L"断开"}, {.1,.1,.8,.7}))
      return make(CNPage::Login, Action::Click, L"战网重新连接", *reconnect);
    if (brand) {
      auto play = text.one({L"进入游戏", L"开始游戏", L"开始", L"游玩", L"play"}, {0,.4,.7,.59}, true);
      if (play) return make(CNPage::Launch, Action::Launch, L"战网守望先锋：进入游戏", *brand, play, true);
      if (text.first({L"更新", L"安装", L"正在运行", L"游戏中", L"正在初始化", L"扫描和修复"}, {0,.4,.7,.59}))
        return make(CNPage::Launch, Action::Wait, L"等待战网更新或游戏启动", *brand);
    }
    auto rail = text.one({L"守望先锋",L"overwatch"},{0,.08,.25,.52});
    auto library = text.first({L"我的游戏",L"所有游戏",L"最近游玩",L"my games"},{0,0,.5,.4});
    if (rail && library)
      return make(CNPage::LauncherSelect, Action::Click, L"战网选择守望先锋", *library, rail);
    if (login && !text.first({L"账号", L"邮箱", L"账户", L"email"}, {.12,.15,.76,.65}))
      return make(CNPage::Login, Action::Click, L"继续已保存的登录会话", *login, login);
    return {};
  }
  auto defaults = text.first({L"默认设置",L"restore defaults"},{.45,.82,.55,.18});
  auto settingsTab = text.first({L"音频",L"辅助",L"控制",L"法律"},{0,0,.65,.16});
  auto settingsSection = text.first({L"图像质量",L"HDR",L"视频",L"显示模式"},{0,0,.7,.5});
  if (defaults && settingsTab && settingsSection)
    return make(CNPage::Menu,Action::Escape,L"返回游戏：关闭设置页",*settingsTab,defaults);
  auto menu = text.first({L"选项菜单", L"选项设置", L"选顶设置", L"options"}, {.2,.08,.7,.65});
  auto social = text.first({L"社交互动", L"生涯概况", L"social"}, {.2,.15,.7,.65});
  auto leave = text.one({L"离开比赛", L"离开游戏", L"leave game"}, {.2,.2,.65,.6}, true);
  auto cancel = text.one({L"取消", L"cancel"}, {.2,.3,.65,.55}, true);
  if (deathmatch && context.finished && context.leaving && cancel) {
    if (auto question = text.one({L"确定要离开", L"确定离开", L"离开比赛", L"离开游戏"}, {.15,.2,.7,.6}))
      if (auto yes = text.one({L"确定", L"是", L"离开", L"yes", L"leave game"}, {.2,.3,.65,.55}, true))
        return make(CNPage::ConfirmLeave, Action::Click, L"结算后确认离开，重新匹配", *question, yes);
  }
  if (menu && social) {
    if (deathmatch && context.finished && leave)
      return make(CNPage::Leave, Action::Click, L"比赛已结束，返回匹配菜单", *menu, leave);
    return make(CNPage::Menu, Action::Escape, L"关闭选项菜单", *menu, social);
  }
  auto endorsementDone=text.one({L"感谢赞赏",L"谢谢赞赏",L"thanks for endorsing"},{.15,.65,.70,.20});
  if (deathmatch && endorsementDone)
    return make(CNPage::Endorse,Action::Escape,L"赞赏完成，按 Esc 返回结算",*endorsementDone);
  auto endorsement=text.one({L"赞赏一名对手",L"赞赏队友",L"赞赏玩家",L"赞赏一名队友",L"endorse players"},{.15,.65,.70,.20});
  if (deathmatch && endorsement) {
    auto cards=endorsementCards(frame);
    if (!cards.empty() && context.endorsementClicks<2)
      return make(CNPage::Endorse,Action::Click,L"赞赏未赞赏的玩家名片",*endorsement,TextHit{cards.front()});
    return make(CNPage::Endorse,Action::Escape,L"赞赏完成或没有可赞赏名片，关闭赞赏界面",*endorsement);
  }
  auto searching = text.one({L"正在搜索", L"正在匹配", L"搜索比赛", L"正在排队", L"排队中", L"寻找比赛", L"searching"}, {0,0,1,.5});
  if (searching && (cancel || text.first({L"预计", L"已用", L"用时", L"自由死斗", L"死斗"}, {0,0,1,.55}))) {
    auto result = make(CNPage::Queue, Action::Wait, L"等待自动匹配对局", *searching, cancel);
    result.verifiedDeathmatch = bool(text.first({L"自由死斗",L"死斗",L"deathmatch"},{0,0,1,.55}));
    return result;
  }
  auto hero = text.one({L"选择英雄",L"请选择一名英雄",L"选择一名英雄",L"英雄选择",L"select a hero"},{0,.02,1,.88});
  auto select = text.one({L"确认选择",L"加入战斗",L"进入战斗",L"出击",L"选择",L"准备",L"继续",L"确认",L"select",L"ready"},{.15,.62,.8,.37});
  bool rosterEvidence=false;
  if (!hero && select) {
    auto tank=text.first({L"重装",L"坦克",L"tank"},{0,.65,1,.13});
    auto support=text.first({L"支援",L"辅助",L"support"},{0,.65,1,.13});
    auto damage=text.first({L"输出",L"damage"},{0,.65,1,.13});
    if (tank && support && damage) hero=tank;
    // Small windows may omit the role labels entirely. The game-specific
    // footer and a separately located, enabled selection button identify
    // this hero screen without relying on a remembered matchmaking frame.
    auto info=text.one({L"英雄信息",L"英信息",L"hero information"},{0,.91,.30,.09});
    auto choose=text.one({L"选择",L"确认选择",L"加入战斗",L"继续",L"continue",L"select"},{.30,.86,.40,.13},true);
    if (!hero && choose && enabledUiButton(frame,choose->roi)) {
      if (info) { hero=info;select=choose; }
      else if (const auto grid=heroPortraitGrid(frame)) {
        hero=TextHit{*grid};select=choose;rosterEvidence=true;
      }
    }
  }
  if (hero) {
    if (select && enabledUiButton(frame,select->roi)) {
      auto result=make(CNPage::Hero,Action::Click,L"确认英雄选择，进入对局",*hero,select,true);
      result.heroRoster=rosterEvidence;return result;
    }
    return make(CNPage::Hero,Action::Enter,L"选择当前高亮英雄，等待确认选择",*hero);
  }
  if (deathmatch) {
    auto ended = text.first({L"比赛结束", L"对战结束", L"最终得分", L"最终排名"}, {.15,.04,.75,.7});
    if (!ended) {
      auto outcome = text.one({L"胜利",L"失败",L"victory",L"defeat"}, {.15,.08,.7,.6}, true);
      if (outcome && outcome->roi.h >= .04 && text.first({L"经验",L"得分",L"继续",L"排名"}, {.1,.1,.8,.85})) ended = outcome;
    }
    auto again = text.one({L"再次排队",L"再次匹配",L"重新匹配",L"再次游玩",L"再玩一次",L"play again"},{.1,.45,.8,.53},true);
    auto overview=text.first({L"总览",L"overview"},{0,0,.3,.12});
    auto rewards=text.first({L"奖励",L"rewards"},{0,0,.3,.12});
    auto proceed=text.one({L"继续",L"continue"},{.25,.8,.50,.18},true);
    if (!again && overview && rewards && proceed) {
      auto partial=text.find(L"再次",{.25,.85,.5,.12});
      partial.erase(std::remove_if(partial.begin(),partial.end(),[](const TextHit &hit) {
        return hit.lineText.size()>7 || hit.lineText.find(L"队")==std::wstring::npos;
      }),partial.end());
      if (partial.size()==1) again=partial.front();
    }
    if (!ended && overview && rewards && (again || proceed)) ended=overview;
    if (ended || (context.finished && again)) {
      if (!context.endorsementAttempted)
        return make(CNPage::Results,Action::Endorse,L"比赛已结束，按 N 打开赞赏",ended ? *ended : *again,again ? again : proceed);
      if (again) {
        auto result=make(CNPage::Results,Action::Click,L"赞赏后再次排队死斗",ended ? *ended : *again,again);
        if (!enabledResultButton(frame,again->roi)) result.observation.step.action=Action::Wait;
        return result;
      }
      if (proceed) return make(CNPage::Results,Action::Click,L"继续结算，返回死斗匹配",*ended,proceed,true);
      return make(CNPage::Results,Action::Escape,L"比赛结束，返回死斗匹配",*ended);
    }
    if (auto respawn = text.one({L"重新部署", L"重生", L"respawn"}, {.25,.3,.65,.65})) {
      auto mode = text.first({L"自由死斗", L"死斗", L"更换英雄"}, {0,0,1,.9});
      if (mode || context.trustedDeathmatch)
        return make(CNPage::Respawn, Action::Space, L"死斗重生", *respawn, mode);
    }
    auto modeTitle = text.first({L"自由死斗",L"死斗",L"deathmatch"}, {0,.04,1,.75});
    auto queueButton = text.one({L"开始",L"开始游戏",L"开始匹配",L"开始搜索",L"play"}, {.15,.65,.75,.33}, true);
    if (modeTitle && queueButton)
      return make(CNPage::DeathmatchStart,Action::Click,L"开始自由死斗匹配",*modeTitle,queueButton,true);
  }
  if (!deathmatch) {
    auto joining=text.first({L"正在尝试自动加入",L"正在尝试加入",L"正在加入游戏",L"正在尝过自动加入"},{.2,.25,.65,.4});
    auto back=text.one({L"返回",L"取消"},{.25,.4,.5,.4},true);
    if (joining && back) return make(CNPage::Joining,Action::Wait,L"正在加入目标大厅，等待服务器响应",*joining,back);
    auto targetTitle=text.one({context.target.c_str()},{.1,.04,.8,.18});
    auto mapHeader=text.one({L"当前地图"},{.3,.18,.4,.16});
    auto createLobby=text.one({L"创建新的大厅",L"创建新大厅"},{.2,.8,.55,.18});
    if (targetTitle && mapHeader && createLobby) {
      auto rows=vacantRoomRows(ocr);
      auto result=make(CNPage::RoomList,rows.empty() ? Action::Wait : Action::DoubleClick,
          rows.empty() ? L"未识别到可加入的空位大厅，等待列表刷新" : L"双击加入有空位的目标大厅",*targetTitle,mapHeader);
      result.evidence.push_back(createLobby->roi);
      for (const auto &row:rows) {
        result.observation.cards.push_back(center(row.title));
        result.evidence.push_back(row.count); result.evidence.push_back(row.title);
      }
      if (!rows.empty()) {
        result.observation.hasCard=true; result.observation.card=result.observation.cards.front();
        result.observation.step.click=result.observation.card;
      }
      return result;
    }
    // This footer belongs to the room browser, including incomplete OCR frames.
    // Its target title must never fall through to the joined-lobby detector.
    if (createLobby)
      return make(CNPage::RoomList,Action::Wait,L"等待核实目标大厅标题与列表表头",*createLobby,mapHeader);
  }
  auto browse = text.one({L"浏览游戏", L"浏览自定游戏", L"游戏浏览器", L"game browser"}, {0,0,1,.28});
  std::optional<TextHit> browserProof;
  if (!browse) {
    auto create = text.one({L"创建游戏",L"create game"}, {0,0,1,.3});
    auto filter = text.one({L"筛选",L"搜索",L"filter",L"search"}, {0,.1,.65,.23});
    auto tabs = text.first({L"人气",L"收藏",L"近期",L"所有"}, {0,0,.65,.1});
    if (create && (filter || tabs)) { browse = create; browserProof = filter ? filter : tabs; }
    else if (filter && tabs) { browse = filter; browserProof = tabs; }
  }
  if (browse) {
    auto browserPage = [&](CNPage page, Action action, const wchar_t *label,
                           std::optional<TextHit> button = {}, bool colored = false) {
      auto result = make(page,action,label,*browse,button,colored);
      if (browserProof) result.evidence.push_back(browserProof->roi);
      return result;
    };
    if (deathmatch) {
      auto result = browserPage(CNPage::Browser, Action::Escape, L"返回死斗模式选择");
      return result;
    }
    auto cards = roomCandidates(text,context.target);
    bool sameMode = !cards.empty() && std::all_of(cards.begin(),cards.end(),[&](const TextHit &hit) {
      return hit.lineText == cards.front().lineText;
    });
    if (sameMode) {
      // Always browse the target by double click. This must also work when
      // the optional footer is disabled or its small label is missed by OCR.
      auto result=browserPage(CNPage::Browser,Action::DoubleClick,
                              L"双击目标模式，打开大厅列表",cards.front());
      result.observation.hasCard = true;
      result.observation.card = center(cards.front().roi);
      // Validate complete mode titles and their positions. Room metadata and
      // animated background are allowed to change between OCR and the click.
      for (const auto &card : cards) {
        result.observation.cards.push_back(center(card.roi));
        result.evidence.push_back(card.roi);
      }
      return result;
    }
    if (cards.empty()) {
      if (auto search = text.one({L"搜索", L"search"}, {.15,.03,.75,.26})) {
        auto result = browserPage(CNPage::BrowserSearch, Action::Text, L"搜索目标自定义房间", search);
        result.observation.step.text = context.target;
        return result;
      }
    }
    return browserPage(CNPage::Browser, Action::Wait, cards.empty() ? L"等待目标房间出现" : L"名称匹配多个不同模式，请填写完整模式名");
  }
  // A target-name lobby is direct evidence of having joined. A running
  // Workshop match omits that name; its weaker HUD fallback is checked only
  // after all definite navigation pages below have had a chance to match.
  if (!deathmatch) {
    auto target = text.find(context.target, {0,0,1,.65});
    auto players = text.first({L"观战", L"玩家", L"队伍", L"大厅"}, {0,.05,1,.85});
    if (target.size() == 1 && players)
      return make(CNPage::Room, Action::Ready, L"目标自定义房间", target.front(), players);
  }
  auto more = text.one({L"更多模式", L"其他模式", L"more modes"}, {0,0,1,.9});
  auto custom = text.one({L"自定游戏", L"自定义游戏", L"custom games"}, {0,.15,1,.7});
  auto dm = text.one({L"自由死斗", L"死斗", L"deathmatch"}, {0,.18,1,.68}, true);
  auto arcade = text.one({L"街机先锋", L"街机", L"arcade"}, {0,.08,1,.77});
  auto modes = text.first({L"开始",L"游戏模式",L"竞技比赛",L"休闲比赛",L"休闲比",L"快速游戏"},{0,0,1,.8});
  auto arcadeTab=text.one({L"街机先锋",L"街机",L"arcade"},{0,.08,1,.12});
  if (!arcadeTab && deathmatch && (more || custom || modes)) {
    // Small Chinese glyphs can lose the final letter or confuse 机/枳.
    // Require a short Arcade-like line on the navigation strip and a separate
    // mode-menu marker; its live text bounds supply the click position.
    auto partial=text.find(L"街",{0,.08,1,.12});
    // A white Arcade icon can merge with the first tiny glyph. Accept the
    // surviving two-character stem only on this strip, with independent
    // mode-menu evidence above; click coordinates still come from live text.
    if (partial.empty()) partial=text.find(L"机先",{0,.08,1,.12});
    partial.erase(std::remove_if(partial.begin(),partial.end(),[](const TextHit &hit) {
      return hit.lineText.size()>8 || (hit.lineText.find(L"机")==std::wstring::npos && hit.lineText.find(L"先")==std::wstring::npos);
    }),partial.end());
    if (partial.size()==1) { arcadeTab=partial.front();arcadeTab->roi=arcadeTab->lineRoi; }
  }
  // The page title sits to the left of the persistent top navigation. Both
  // may read "Arcade", so a whole-page unique lookup is deliberately not used
  // to decide whether Arcade is already open or to anchor its mode card.
  auto arcadeTitle=text.one({L"街机先锋",L"街机",L"arcade"},{0,.08,.55,.4});
  if (deathmatch && dm && (more || arcadeTitle || arcadeTab || arcade || modes))
    return make(CNPage::More, Action::Click, L"进入自由死斗匹配",
                arcadeTitle ? *arcadeTitle : (more ? *more : (arcadeTab ? *arcadeTab : (arcade ? *arcade : *modes))), dm);
  if (!deathmatch && custom && (more || arcade))
    return make(CNPage::More, Action::Click, L"进入自定游戏", more ? *more : *arcade, custom);
  if (!deathmatch && more && (custom || arcade || modes))
    return make(CNPage::Play, Action::Click, L"打开更多游戏模式", custom ? *custom : (arcade ? *arcade : *modes), more);
  if (deathmatch && !dm) {
    // The user's current client exposes Deathmatch under Arcade, not Unranked.
    if (arcadeTab && !arcadeTitle && (more || custom || modes))
      return make(CNPage::Play,Action::Click,L"进入街机先锋，查找死斗",more ? *more : (custom ? *custom : *modes),arcadeTab);
    if (arcade && !arcadeTitle && !more && !custom && center(arcade->roi).y>.25)
      return make(CNPage::Play,Action::Click,L"打开街机死斗入口",*arcade,arcade);
    if (custom || more || arcadeTitle || arcadeTab)
      return make(CNPage::More,Action::Wait,L"等待街机先锋中的死斗卡片；不进入休闲比赛或训练模式",
                  arcadeTitle ? *arcadeTitle : (custom ? *custom : (more ? *more : *arcadeTab)));
  }
  auto start = text.one({L"开始", L"开始游戏", L"play"}, {0,0,.85,.6});
  auto home = text.first({L"战令", L"英雄", L"商城", L"社交", L"battle pass"}, {0,0,1,.8});
  if (start && home) return make(CNPage::Home, Action::Click, L"主界面：开始", *home, start);
  if (deathmatch) {
    auto score = text.first({L"自由死斗", L"死斗", L"当前排名", L"消灭数", L"排名", L"deathmatch"}, {0,0,1,.4});
    const bool individualScoreboard=deathmatchScoreboard(frame);
    if ((score || context.trustedDeathmatch || individualScoreboard) && combatHud(frame)) {
      SemanticObservation result;
      if (score) result = make(CNPage::Combat, Action::Combat, L"死斗局内随机按键", *score);
      else {
        result.observation.page = CNPage::Combat; result.observation.step.action = Action::Combat;
        result.observation.step.label = individualScoreboard ? L"死斗个人排名与局内 HUD 已确认" : L"已匹配死斗：局内随机按键";
        result.observation.score = .99;
      }
      result.verifiedDeathmatch=individualScoreboard;
      return result;
    }
  }
  // Menu art and button borders can resemble the health/ability HUD. Runtime
  // target trust must never override text that positively identifies Home,
  // mode selection, settings, or the browser. Unlabelled gameplay is the only
  // remaining use of this fallback, and the caller rechecks its runtime trust.
  if (!deathmatch && context.trustedCustomRoom && combatHud(frame)) {
    SemanticObservation result;
    result.observation.page = CNPage::Room;
    result.observation.step.action = Action::Ready;
    result.observation.step.label = L"已加入目标房间：局内画面";
    result.observation.score = .99;
    result.liveRoomHud = true;
    return result;
  }
  return {};
}

inline bool transientModeNavigation(const SemanticContext &context,CNPage page) {
  return !context.launcher && context.mode == WorkflowMode::Deathmatch &&
         (page == CNPage::Home || page == CNPage::Play || page == CNPage::More || page == CNPage::DeathmatchStart) ||
         (!context.launcher && (page==CNPage::Hero || page==CNPage::Endorse || page==CNPage::Results ||
          page==CNPage::RoomList || page==CNPage::Joining));
}

// Extra resolution can recover small labels. Keep a valid ordinary browser
// observation if merging duplicate OCR lines makes the context ambiguous.
inline SemanticObservation recognizeSemanticDetail(const Frame &frame, const OcrFrame &ordinaryWords,
    const OcrFrame &detail, const SemanticContext &context, const SemanticObservation &ordinary) {
  OcrFrame combined = ordinaryWords;
  combined.lines.insert(combined.lines.end(),detail.lines.begin(),detail.lines.end());
  auto enhanced = recognizeSemantic(frame,combined,context);
  if (!ordinary) return enhanced ? enhanced : recognizeSemantic(frame,detail,context);
  if (transientModeNavigation(context,ordinary.observation.page))
    return enhanced ? enhanced : ordinary;
  if ((ordinary.observation.page == CNPage::Browser || ordinary.observation.page==CNPage::RoomList) && enhanced &&
      (enhanced.observation.page == CNPage::Browser || enhanced.observation.page == CNPage::Join || enhanced.observation.page==CNPage::RoomList))
    return enhanced;
  return ordinary;
}

// Keep all successful OCR passes for later local crops. A navigation crop
// must not erase a card/title recovered only by the full detailed pass.
inline SemanticObservation recognizeSemanticSupplement(const Frame &frame,OcrFrame &accumulated,
    const OcrFrame &supplement,const SemanticContext &context,const SemanticObservation &current) {
  auto result=recognizeSemanticDetail(frame,accumulated,supplement,context,current);
  accumulated.lines.insert(accumulated.lines.end(),supplement.lines.begin(),supplement.lines.end());
  return result;
}

struct LearnedAnchor { Rect roi; std::vector<std::uint8_t> image; };
struct LearnedPage {
  SemanticContext context;
  int width{}, height{};
  SemanticObservation semantic;
  std::vector<LearnedAnchor> anchors;
};
inline double glyphSimilarity(const std::vector<std::uint8_t> &a, const std::vector<std::uint8_t> &b) {
  if (a.size() != TemplateW * TemplateH || b.size() != a.size()) return 0;
  double ma{}, mb{}, aa{}, bb{}, ab{};
  for (std::size_t i = 0; i < a.size(); ++i) { ma += a[i]; mb += b[i]; }
  ma /= a.size(); mb /= b.size();
  for (std::size_t i = 0; i < a.size(); ++i) {
    auto x = a[i] - ma, y = b[i] - mb; aa += x*x; bb += y*y; ab += x*y;
  }
  if (aa < a.size() * 64 || bb < b.size() * 64) return 0;
  return std::min(similarity(a,b), std::clamp((1 + ab / std::sqrt(aa*bb)) / 2, 0., 1.));
}
inline bool sameSemanticContext(const SemanticContext &a, const SemanticContext &b) {
  return a.launcher == b.launcher && a.mode == b.mode && a.target == b.target &&
         (a.launcher || (a.finished == b.finished && a.leaving == b.leaving));
}
inline std::optional<LearnedPage> learnSemantic(const Frame &frame, const SemanticObservation &semantic,
                                              const SemanticContext &context) {
  if (!semantic || transientModeNavigation(context,semantic.observation.page) || semantic.liveRoomHud || semantic.observation.page == CNPage::Combat ||
      semantic.observation.page == CNPage::Browser || semantic.observation.page == CNPage::BrowserSearch || semantic.observation.step.action == Action::Wait)
    return {};
  LearnedPage result{context, frame.width, frame.height, semantic, {}};
  for (auto roi : semantic.evidence) {
    Rect padded{std::max(0.,roi.x-.002),std::max(0.,roi.y-.002),0,0};
    padded.w = std::min(1.,roi.x+roi.w+.002)-padded.x;
    padded.h = std::min(1.,roi.y+roi.h+.002)-padded.y;
    auto image = sample(frame,padded);
    if (contrast(image) < 8) return {};
    result.anchors.push_back({padded,std::move(image)});
  }
  if (result.anchors.size() < 2 || result.anchors.size() > 8) return {};
  return result;
}
inline double learnedScore(const Frame &frame, const LearnedPage &page, const SemanticContext &context) {
  if (!sameSemanticContext(context,page.context) || frame.height <= 0 || page.height <= 0 ||
      std::abs((double(frame.width)/frame.height)/(double(page.width)/page.height)-1) > .015 ||
      (context.launcher && (frame.width != page.width || frame.height != page.height))) return 0;
  double score = 1;
  for (const auto &a : page.anchors) score = std::min(score,glyphSimilarity(sample(frame,a.roi),a.image));
  return page.anchors.size() >= 2 ? score : 0;
}
class SemanticCache {
public:
  static constexpr std::size_t Maximum = 64;
  std::vector<LearnedPage> pages;
  SemanticObservation match(const Frame &frame, const SemanticContext &context) const {
    SemanticObservation result;
    for (const auto &page : pages) {
      // Room titles also appear above the browser list. Re-read legacy room
      // and Auto Join decisions so an old calibration cannot hide that footer.
      if ((!context.launcher && context.mode==WorkflowMode::CustomRoom &&
           (page.semantic.observation.page==CNPage::Join || page.semantic.observation.page==CNPage::Room)) ||
          transientModeNavigation(context,page.semantic.observation.page) || page.semantic.liveRoomHud || page.semantic.observation.page == CNPage::Browser ||
          !sameSemanticContext(context,page.context)) continue;
      auto score = learnedScore(frame,page,context);
      if (score < .965) continue;
      if (result && (result.observation.page != page.semantic.observation.page ||
                     result.observation.step.action != page.semantic.observation.step.action ||
                     std::abs(result.observation.step.click.x-page.semantic.observation.step.click.x) > .015 ||
                     std::abs(result.observation.step.click.y-page.semantic.observation.step.click.y) > .015)) return {};
      if (result && result.observation.score >= score) continue;
      result = page.semantic;
      result.observation.score = score;
      if (result.coloredButton && !enabledUiButton(frame,result.button)) {
        result.observation.step.action = Action::Wait;
        result.observation.step.label = L"等待按钮可用";
      }
    }
    return result;
  }
  bool remember(LearnedPage page) {
    if (transientModeNavigation(page.context,page.semantic.observation.page) || page.semantic.liveRoomHud || page.semantic.observation.page == CNPage::Browser) return false;
    auto pointSame = [](Point a,Point b) { return std::abs(a.x-b.x)<.00001 && std::abs(a.y-b.y)<.00001; };
    auto rectSame = [](Rect a,Rect b) { return std::abs(a.x-b.x)<.00001 && std::abs(a.y-b.y)<.00001 &&
                                           std::abs(a.w-b.w)<.00001 && std::abs(a.h-b.h)<.00001; };
    for (const auto &existing : pages)
      if (existing.semantic.observation.page == page.semantic.observation.page &&
          existing.semantic.observation.step.action == page.semantic.observation.step.action &&
          existing.semantic.observation.step.text == page.semantic.observation.step.text &&
          pointSame(existing.semantic.observation.step.click,page.semantic.observation.step.click) &&
          existing.semantic.observation.hasCard == page.semantic.observation.hasCard &&
          pointSame(existing.semantic.observation.card,page.semantic.observation.card) &&
          existing.semantic.coloredButton == page.semantic.coloredButton &&
          rectSame(existing.semantic.button,page.semantic.button) &&
          existing.semantic.observation.cards.size() == page.semantic.observation.cards.size() &&
          std::equal(existing.semantic.observation.cards.begin(),existing.semantic.observation.cards.end(),
                     page.semantic.observation.cards.begin(),pointSame) &&
          existing.width == page.width && existing.height == page.height &&
          sameSemanticContext(existing.context,page.context) && existing.anchors.size() == page.anchors.size()) {
        bool same = true;
        for (std::size_t i = 0; i < page.anchors.size(); ++i)
          same = same && rectSame(existing.anchors[i].roi,page.anchors[i].roi) &&
                         glyphSimilarity(existing.anchors[i].image,page.anchors[i].image) >= .985;
        if (same) return false;
      }
    if (pages.size() == Maximum) pages.erase(pages.begin());
    pages.push_back(std::move(page));
    return true;
  }
};
inline std::string writeSemanticCache(const SemanticCache &cache) {
  auto rect = [](Rect r) { return "["+formatNumber(r.x)+","+formatNumber(r.y)+","+formatNumber(r.w)+","+formatNumber(r.h)+"]"; };
  auto point = [](Point p) { return "["+formatNumber(p.x)+","+formatNumber(p.y)+"]"; };
  auto quote = [](const std::wstring &s) { return "\""+jsonEscape(wideToUtf8(s))+"\""; };
  std::string out = "{\"format\":\"owh-semantic-cache\",\"version\":3,\"pages\":[";
  for (std::size_t i = 0; i < cache.pages.size(); ++i) {
    const auto &p = cache.pages[i]; const auto &s = p.semantic.observation;
    if (i) out += ",";
    out += "{\"launcher\":"+std::to_string(p.context.launcher)+",\"mode\":"+std::to_string(int(p.context.mode))+
        ",\"target\":"+quote(p.context.target)+",\"finished\":"+std::to_string(p.context.finished)+
        ",\"leaving\":"+std::to_string(p.context.leaving)+",\"width\":"+std::to_string(p.width)+",\"height\":"+std::to_string(p.height)+
        ",\"page\":\""+cnPageName(s.page)+"\",\"action\":\""+cnActionName(s.step.action)+"\",\"label\":"+quote(s.step.label)+
        ",\"text\":"+quote(s.step.text)+",\"click\":"+point(s.step.click)+",\"has_card\":"+std::to_string(s.hasCard)+
        ",\"card\":"+point(s.card)+",\"colored\":"+std::to_string(p.semantic.coloredButton)+",\"button\":"+rect(p.semantic.button)+
        ",\"cards\":[";
    for (std::size_t j = 0; j < s.cards.size(); ++j) {
      if (j) out += ",";
      out += point(s.cards[j]);
    }
    out += "],\"anchors\":[";
    for (std::size_t j = 0; j < p.anchors.size(); ++j) {
      if (j) out += ",";
      out += "{\"roi\":"+rect(p.anchors[j].roi)+",\"image\":\""+encodeTemplate(p.anchors[j].image)+"\"}";
    }
    out += "]}";
  }
  return out+"]}";
}
inline bool parseSemanticCache(const std::string &source, SemanticCache &cache, std::string &error) {
  Json root;
  if (source.size() > 1024*1024 || !parseJson(source,root,error)) return false;
  auto format = root.find("format"), version = root.find("version"), pages = root.find("pages");
  if (!format || !format->is(Json::Kind::String) || format->text != "owh-semantic-cache" ||
      !version || !version->is(Json::Kind::Number) || (version->number != 2 && version->number != 3) ||
      !pages || !pages->is(Json::Kind::Array) || pages->items.size() > SemanticCache::Maximum)
    return error = "invalid semantic cache", false;
  SemanticCache parsed;
  for (const auto &entry : pages->items) {
    LearnedPage p;
    auto integer = [&](const char *key, int low, int high, int &out) {
      auto v = entry.find(key);
      if (!v || !v->is(Json::Kind::Number) || !std::isfinite(v->number) || v->number < low || v->number > high ||
          std::floor(v->number) != v->number) return false;
      out = int(v->number); return true;
    };
    auto string = [&](const char *key, std::wstring &out) {
      auto v = entry.find(key);
      if (!v || !v->is(Json::Kind::String) || v->text.size() > 384) return false;
      out = utf8ToWide(v->text); return true;
    };
    int launcher{}, mode{}, finished{}, leaving{}, colored{}, card{};
    auto page = entry.find("page"), action = entry.find("action"), click = entry.find("click"),
         cardPoint = entry.find("card"), button = entry.find("button"), anchors = entry.find("anchors"),
         cards = entry.find("cards");
    auto &o = p.semantic.observation;
    if (!integer("launcher",0,1,launcher) || !integer("mode",0,1,mode) || !integer("finished",0,1,finished) ||
        !integer("leaving",0,1,leaving) || !integer("colored",0,1,colored) || !integer("has_card",0,1,card) ||
        !integer("width",1,16384,p.width) || !integer("height",1,16384,p.height) ||
        !string("target",p.context.target) || !string("label",o.step.label) || !string("text",o.step.text) ||
        !page || !page->is(Json::Kind::String) || !cnPageFromName(page->text,o.page) ||
        !action || !action->is(Json::Kind::String) || !cnActionFromName(action->text,o.step.action) ||
        !click || !detail::readPoint(*click,o.step.click) || !cardPoint || !detail::readPoint(*cardPoint,o.card) ||
        !anchors || !anchors->is(Json::Kind::Array) || anchors->items.size() < 2 || anchors->items.size() > 8)
      return error = "invalid learned page", false;
    // Old room pages used a whole-scene fingerprint and incomplete title
    // evidence. Relearn them while preserving other saved navigation pages.
    if (o.page == CNPage::Browser || (version->number == 2 && o.page == CNPage::Join)) continue;
    if (version->number == 3) {
      if (!cards || !cards->is(Json::Kind::Array) || cards->items.size() > 64)
        return error = "invalid room candidates", false;
      for (const auto &candidate : cards->items) {
        Point position;
        if (!detail::readPoint(candidate,position)) return error = "invalid room candidate", false;
        o.cards.push_back(position);
      }
    }
    p.context.launcher = launcher != 0; p.context.mode = WorkflowMode(mode);
    if (transientModeNavigation(p.context,o.page)) continue;
    p.context.finished = finished != 0; p.context.leaving = leaving != 0;
    p.semantic.coloredButton = colored != 0; o.hasCard = card != 0; o.step.launcher = launcher != 0; o.score = .99;
    if (colored && (!button || !detail::readRect(*button,p.semantic.button))) return error = "invalid learned button", false;
    if (!colored && button && button->is(Json::Kind::Array) && button->items.size() == 4) {
      // Zero rectangle is legitimate for observations without a clickable button.
      detail::readRect(*button,p.semantic.button);
    }
    if ((launcher && o.step.action == Action::Combat) || (!launcher && o.step.action == Action::Launch) ||
        o.step.action == Action::Wait || o.step.action == Action::Combat || o.page == CNPage::BrowserSearch ||
        (!launcher && (o.page == CNPage::Browser || o.page == CNPage::Join) && (!o.hasCard || o.cards.empty())))
      return error = "invalid cached action", false;
    for (const auto &a : anchors->items) {
      LearnedAnchor anchor; auto roi = a.find("roi"), data = a.find("image");
      if (!roi || !detail::readRect(*roi,anchor.roi) || !data || !data->is(Json::Kind::String) ||
          !detail::decodeTemplate(data->text,anchor.image) || contrast(anchor.image) < 8)
        return error = "invalid learned anchor", false;
      p.semantic.evidence.push_back(anchor.roi); p.anchors.push_back(std::move(anchor));
    }
    parsed.pages.push_back(std::move(p));
  }
  cache = std::move(parsed); return true;
}
} // namespace owh
