#pragma once
#include "core/core.hpp"
#include "core/cn_recovery.hpp"
#include "core/json.hpp"
#include <cstdio>
#include <string>

namespace owh {
// Text conversions live here so core stays independent of the Windows agent
// layer, which has its own Win32-based helpers.
inline std::wstring utf8ToWide(const std::string &s) {
  std::wstring out;
  out.reserve(s.size());
  for (std::size_t i = 0; i < s.size();) {
    unsigned char c = (unsigned char)s[i];
    unsigned code{};
    int extra{};
    if (c < 0x80) { code = c; extra = 0; }
    else if ((c & 0xe0) == 0xc0) { code = c & 0x1f; extra = 1; }
    else if ((c & 0xf0) == 0xe0) { code = c & 0x0f; extra = 2; }
    else if ((c & 0xf8) == 0xf0) { code = c & 0x07; extra = 3; }
    else { out += L'\ufffd'; ++i; continue; }
    if (i + std::size_t(extra) >= s.size()) { out += L'\ufffd'; break; }
    bool ok = true;
    for (int k = 0; k < extra; ++k) {
      unsigned char cc = (unsigned char)s[i + 1 + std::size_t(k)];
      if ((cc & 0xc0) != 0x80) { ok = false; break; }
      code = (code << 6) | (cc & 0x3f);
    }
    if (!ok) { out += L'\ufffd'; ++i; continue; }
    i += std::size_t(extra) + 1;
    if (code <= 0xffff)
      out += wchar_t(code);
    else {
      code -= 0x10000;
      out += wchar_t(0xd800 + (code >> 10));
      out += wchar_t(0xdc00 + (code & 0x3ff));
    }
  }
  return out;
}
inline std::string wideToUtf8(const std::wstring &s) {
  std::string out;
  for (std::size_t i = 0; i < s.size(); ++i) {
    unsigned code = (unsigned)s[i];
    if (code >= 0xd800 && code <= 0xdbff && i + 1 < s.size()) {
      unsigned low = (unsigned)s[i + 1];
      if (low >= 0xdc00 && low <= 0xdfff) {
        code = 0x10000 + ((code - 0xd800) << 10) + (low - 0xdc00);
        ++i;
      }
    }
    if (code <= 0x7f)
      out += char(code);
    else if (code <= 0x7ff) {
      out += char(0xc0 | (code >> 6));
      out += char(0x80 | (code & 0x3f));
    } else if (code <= 0xffff) {
      out += char(0xe0 | (code >> 12));
      out += char(0x80 | ((code >> 6) & 0x3f));
      out += char(0x80 | (code & 0x3f));
    } else {
      out += char(0xf0 | (code >> 18));
      out += char(0x80 | ((code >> 12) & 0x3f));
      out += char(0x80 | ((code >> 6) & 0x3f));
      out += char(0x80 | (code & 0x3f));
    }
  }
  return out;
}

struct CNProfile {
  std::string name;
  std::vector<CNRule> rules;
  std::vector<CNLauncherAnchor> launcher;
  CNJoinSpec join;
  std::wstring roomTarget; // Empty means the original shipped target.
};
inline bool cnProfileTargets(const CNProfile &profile, const std::wstring &target) {
  return profile.roomTarget.empty() ? target == L"最稳轮换版" : target == profile.roomTarget;
}
// The target card click, taken from the profile rather than duplicated at each
// call site: the join gate has to select the card itself before joining, and the
// two positions must not be able to drift apart.
inline Point cnCardClick(const CNProfile &profile) {
  for (const auto &rule : profile.rules)
    if (rule.page == CNPage::Browser)
      return rule.click;
  return {176. / 960, 368. / 540};
}
// Limits keep a hand-edited or downloaded profile from making matching
// unbounded. They are far above what the shipped preset needs.
inline constexpr std::size_t CNMaxRules = 64;
inline constexpr std::size_t CNMaxAnchors = 8;
inline constexpr std::size_t CNMaxVariants = 8;

inline const char *cnPageName(CNPage page) {
  switch (page) {
  case CNPage::LoginError: return "login_error";
  case CNPage::Home: return "home";
  case CNPage::Play: return "play";
  case CNPage::More: return "more";
  case CNPage::Browser: return "browser";
  case CNPage::Join: return "join";
  case CNPage::Launch: return "launch";
  case CNPage::Room: return "room";
  case CNPage::Menu: return "menu";
  case CNPage::Login: return "login";
  case CNPage::LauncherSelect: return "launcher_select";
  case CNPage::BrowserSearch: return "browser_search";
  case CNPage::Queue: return "queue";
  case CNPage::Hero: return "hero";
  case CNPage::Combat: return "combat";
  case CNPage::Results: return "results";
  case CNPage::Leave: return "leave";
  case CNPage::ConfirmLeave: return "confirm_leave";
  case CNPage::Respawn: return "respawn";
  case CNPage::DeathmatchStart: return "deathmatch_start";
  case CNPage::Endorse: return "endorse";
  case CNPage::RoomList: return "room_list";
  case CNPage::Joining: return "joining";
  default: return "none";
  }
}
inline bool cnPageFromName(const std::string &name, CNPage &out) {
  for (int i = 0; i <= int(CNPage::Joining); ++i) {
    auto page = CNPage(i);
    if (page != CNPage::None && name == cnPageName(page)) {
      out = page;
      return true;
    }
  }
  return false;
}
inline const char *cnActionName(Action action) {
  switch (action) {
  case Action::Click: return "click";
  case Action::Enter: return "enter";
  case Action::Escape: return "escape";
  case Action::Text: return "text";
  case Action::Ready: return "ready";
  case Action::Restart: return "restart";
  case Action::Launch: return "launch";
  case Action::Ignore: return "ignore";
  case Action::Wait: return "wait";
  case Action::Combat: return "combat";
  case Action::Space: return "space";
  case Action::Endorse: return "endorse";
  case Action::DoubleClick: return "double_click";
  }
  return "click";
}
inline bool cnActionFromName(const std::string &name, Action &out) {
  for (int i = 0; i <= int(Action::DoubleClick); ++i)
    if (name == cnActionName(Action(i))) {
      out = Action(i);
      return true;
    }
  return false;
}
namespace detail {
inline bool decodeTemplate(const std::string &hex, std::vector<std::uint8_t> &out) {
  if (hex.size() != std::size_t(TemplateW) * TemplateH * 2)
    return false;
  auto digit = [](char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };
  out.clear();
  for (std::size_t i = 0; i < hex.size(); i += 2) {
    int hi = digit(hex[i]), lo = digit(hex[i + 1]);
    if (hi < 0 || lo < 0) {
      out.clear();
      return false;
    }
    out.push_back(std::uint8_t(hi * 16 + lo));
  }
  return true;
}
inline bool readRect(const Json &value, Rect &out) {
  if (!value.is(Json::Kind::Array) || value.items.size() != 4)
    return false;
  for (const auto &item : value.items)
    if (!item.is(Json::Kind::Number))
      return false;
  out = {value.items[0].number, value.items[1].number, value.items[2].number,
         value.items[3].number};
  return valid(out);
}
inline bool readPoint(const Json &value, Point &out) {
  if (!value.is(Json::Kind::Array) || value.items.size() != 2)
    return false;
  for (const auto &item : value.items)
    if (!item.is(Json::Kind::Number))
      return false;
  out = {value.items[0].number, value.items[1].number};
  return std::isfinite(out.x) && std::isfinite(out.y) && out.x >= 0 && out.x <= 1 &&
         out.y >= 0 && out.y <= 1;
}
// A template that is flat or nearly flat cannot discriminate between pages, so
// it is rejected here exactly as a calibrated one is.
inline bool readVariants(const Json &value, std::vector<std::vector<std::uint8_t>> &out) {
  if (!value.is(Json::Kind::Array) || value.items.empty() || value.items.size() > CNMaxVariants)
    return false;
  out.clear();
  for (const auto &item : value.items) {
    if (!item.is(Json::Kind::String))
      return false;
    std::vector<std::uint8_t> image;
    if (!decodeTemplate(item.text, image) || contrast(image) < 8)
      return false;
    out.push_back(std::move(image));
  }
  return true;
}
// Optional key: a null value means the key was absent and the fallback applies.
inline bool readMinimum(const Json *value, double fallback, double &out) {
  if (!value)
    out = fallback;
  else if (value->is(Json::Kind::Number))
    out = value->number;
  else
    return false;
  return std::isfinite(out) && out >= .5 && out <= 1;
}
// Coverage fractions (share of matching pixels) are not template scores, so they
// legitimately fall below the .5 floor a similarity threshold must respect.
inline bool readFraction(const Json *value, double fallback, double &out) {
  if (!value)
    out = fallback;
  else if (value->is(Json::Kind::Number))
    out = value->number;
  else
    return false;
  return std::isfinite(out) && out >= 0 && out <= 1;
}
} // namespace detail

// Returns false and fills error on any malformed rule. A profile that parses
// partially would silently drop recognition steps, so the whole file is
// rejected instead and the caller falls back to the built-in preset.
inline bool parseCNProfile(const std::string &json, CNProfile &out, std::string &error) {
  Json root;
  if (!parseJson(json, root, error))
    return false;
  if (!root.is(Json::Kind::Object))
    return error = "profile root must be an object", false;
  auto format = root.find("format");
  if (!format || !format->is(Json::Kind::String) || format->text != "owh-profile")
    return error = "not an owh-profile document", false;
  auto version = root.find("version");
  if (!version || !version->is(Json::Kind::Number) || int(version->number) != 1)
    return error = "unsupported profile version", false;

  CNProfile profile;
  if (auto name = root.find("name"); name && name->is(Json::Kind::String))
    profile.name = name->text;
  if (auto target = root.find("room_target")) {
    if (!target->is(Json::Kind::String) || target->text.size() > 256)
      return error = "invalid profile room target", false;
    profile.roomTarget = utf8ToWide(target->text);
    if (profile.roomTarget.size() > 64 || profile.roomTarget.find_first_of(L"\r\n\t") != std::wstring::npos)
      return error = "invalid profile room target", false;
  }

  auto rules = root.find("rules");
  if (!rules || !rules->is(Json::Kind::Array))
    return error = "profile has no rules array", false;
  if (rules->items.size() > CNMaxRules)
    return error = "too many rules", false;
  for (const auto &entry : rules->items) {
    if (!entry.is(Json::Kind::Object))
      return error = "rule must be an object", false;
    CNRule rule;
    auto page = entry.find("page");
    if (!page || !page->is(Json::Kind::String) || !cnPageFromName(page->text, rule.page))
      return error = "rule has an unknown page", false;
    auto label = entry.find("label");
    if (!label || !label->is(Json::Kind::String) || label->text.size() > 384)
      return error = "rule has no usable label", false;
    rule.label = utf8ToWide(label->text);
    auto action = entry.find("action");
    if (!action || !action->is(Json::Kind::String) || !cnActionFromName(action->text, rule.action))
      return error = "rule has an unknown action", false;
    auto click = entry.find("click");
    if (!click || !detail::readPoint(*click, rule.click))
      return error = "rule has an invalid click point", false;
    auto aspect = entry.find("aspect");
    if (!aspect || !aspect->is(Json::Kind::Number) || !std::isfinite(aspect->number) ||
        aspect->number < .5 || aspect->number > 4)
      return error = "rule has an invalid aspect", false;
    rule.aspect = aspect->number;
    if (!detail::readMinimum(entry.find("minimum"), .90, rule.minimum))
      return error = "rule has an invalid minimum", false;
    auto anchors = entry.find("anchors");
    if (!anchors || !anchors->is(Json::Kind::Array) || anchors->items.empty() ||
        anchors->items.size() > CNMaxAnchors)
      return error = "rule needs between 1 and 8 anchors", false;
    for (const auto &item : anchors->items) {
      if (!item.is(Json::Kind::Object))
        return error = "anchor must be an object", false;
      CNAnchor anchor;
      auto roi = item.find("roi");
      if (!roi || !detail::readRect(*roi, anchor.roi))
        return error = "anchor has an invalid roi", false;
      if (!detail::readMinimum(item.find("minimum"), rule.minimum, anchor.minimum))
        return error = "anchor has an invalid minimum", false;
      auto variants = item.find("variants");
      if (!variants || !detail::readVariants(*variants, anchor.variants))
        return error = "anchor has no usable template", false;
      rule.anchors.push_back(std::move(anchor));
    }
    // Launcher matching has its own DPI-anchored geometry, so a Launch action
    // never belongs in the game rules array.
    if (rule.action == Action::Launch)
      return error = "Launch belongs in the launcher array, not in rules", false;
    profile.rules.push_back(std::move(rule));
  }

  if (auto launcher = root.find("launcher")) {
    if (!launcher->is(Json::Kind::Array) || launcher->items.size() > CNMaxAnchors)
      return error = "launcher must be an array of at most 8 anchors", false;
    for (const auto &item : launcher->items) {
      if (!item.is(Json::Kind::Object))
        return error = "launcher anchor must be an object", false;
      CNLauncherAnchor anchor;
      auto rect = item.find("rect");
      if (!rect || !rect->is(Json::Kind::Array) || rect->items.size() != 4)
        return error = "launcher anchor needs a 4-number rect", false;
      for (const auto &n : rect->items)
        if (!n.is(Json::Kind::Number))
          return error = "launcher rect must be numeric", false;
      anchor.x = rect->items[0].number;
      anchor.y = rect->items[1].number;
      anchor.w = rect->items[2].number;
      anchor.h = rect->items[3].number;
      if (!std::isfinite(anchor.x) || !std::isfinite(anchor.y) || !std::isfinite(anchor.w) ||
          !std::isfinite(anchor.h) || anchor.x < 0 || anchor.w <= 0 || anchor.h <= 0)
        return error = "launcher rect out of range", false;
      if (!detail::readMinimum(item.find("minimum"), .94, anchor.minimum))
        return error = "launcher anchor has an invalid minimum", false;
      if (auto click = item.find("click")) {
        if (!click->is(Json::Kind::Array) || click->items.size() != 2 ||
            !click->items[0].is(Json::Kind::Number) || !click->items[1].is(Json::Kind::Number))
          return error = "launcher click must be two numbers", false;
        anchor.clickable = true;
        anchor.clickX = click->items[0].number;
        anchor.clickY = click->items[1].number;
        if (!std::isfinite(anchor.clickX) || !std::isfinite(anchor.clickY))
          return error = "launcher click out of range", false;
      }
      auto variants = item.find("variants");
      if (!variants || !detail::readVariants(*variants, anchor.variants))
        return error = "launcher anchor has no usable template", false;
      profile.launcher.push_back(std::move(anchor));
    }
  }
  // Required rather than defaulted: without it the join state can never be
  // reached, which would silently strand the navigation one step short of a
  // room instead of reporting a broken profile.
  auto join = root.find("join");
  if (!join || !join->is(Json::Kind::Object))
    return error = "profile has no join section", false;
  auto selection = join->find("selection_roi");
  if (!selection || !detail::readRect(*selection, profile.join.selectionRoi))
    return error = "join has an invalid selection_roi", false;
  auto button = join->find("button_roi");
  if (!button || !detail::readRect(*button, profile.join.buttonRoi))
    return error = "join has an invalid button_roi", false;
  auto joinClick = join->find("click");
  if (!joinClick || !detail::readPoint(*joinClick, profile.join.click))
    return error = "join has an invalid click point", false;
  if (!detail::readFraction(join->find("selection_minimum"), .08, profile.join.selectionMinimum))
    return error = "join has an invalid selection_minimum", false;
  if (!detail::readFraction(join->find("button_minimum"), .45, profile.join.buttonMinimum))
    return error = "join has an invalid button_minimum", false;
  if (auto label = join->find("label"); label && label->is(Json::Kind::String))
    profile.join.label = utf8ToWide(label->text);
  if (profile.join.label.empty())
    profile.join.label = L"最稳轮换版：自动加入";

  out = std::move(profile);
  error.clear();
  return true;
}

inline std::string formatNumber(double v) {
  char buffer[40]{};
  std::snprintf(buffer, sizeof(buffer), "%.17g", v);
  return buffer;
}
inline std::string encodeTemplate(const std::vector<std::uint8_t> &image) {
  static const char digits[] = "0123456789abcdef";
  std::string hex;
  hex.reserve(image.size() * 2);
  for (auto byte : image) {
    hex += digits[byte >> 4];
    hex += digits[byte & 15];
  }
  return hex;
}
// Emitted by the profile generator and by the panel's export action, so the
// written file is the same shape the reader accepts.
inline std::string writeCNProfile(const CNProfile &profile) {
  auto quote = [](const std::string &s) {
    std::string out = "\"";
    for (char c : s) {
      if (c == '"' || c == '\\')
        out += '\\', out += c;
      else if (c == '\n')
        out += "\\n";
      else if ((unsigned char)c < 0x20)
        out += ' ';
      else
        out += c;
    }
    out += '"';
    return out;
  };
  std::string out = "{\n  \"format\": \"owh-profile\",\n  \"version\": 1,\n  \"name\": ";
  out += quote(profile.name) + ",\n  \"room_target\": " + quote(wideToUtf8(profile.roomTarget)) + ",\n  \"rules\": [\n";
  for (std::size_t r = 0; r < profile.rules.size(); ++r) {
    const auto &rule = profile.rules[r];
    out += "    {\n      \"page\": " + quote(cnPageName(rule.page)) + ",\n";
    out += "      \"label\": " + quote(wideToUtf8(rule.label)) + ",\n";
    out += "      \"action\": " + quote(cnActionName(rule.action)) + ",\n";
    out += "      \"click\": [" + formatNumber(rule.click.x) + ", " + formatNumber(rule.click.y) +
           "],\n";
    out += "      \"aspect\": " + formatNumber(rule.aspect) + ",\n";
    out += "      \"minimum\": " + formatNumber(rule.minimum) + ",\n";
    out += "      \"anchors\": [\n";
    for (std::size_t a = 0; a < rule.anchors.size(); ++a) {
      const auto &anchor = rule.anchors[a];
      out += "        { \"roi\": [" + formatNumber(anchor.roi.x) + ", " +
             formatNumber(anchor.roi.y) + ", " + formatNumber(anchor.roi.w) + ", " +
             formatNumber(anchor.roi.h) + "], \"minimum\": " + formatNumber(anchor.minimum) +
             ",\n          \"variants\": [\n";
      for (std::size_t v = 0; v < anchor.variants.size(); ++v) {
        out += "            " + quote(encodeTemplate(anchor.variants[v]));
        out += v + 1 < anchor.variants.size() ? ",\n" : "\n";
      }
      out += "          ] }";
      out += a + 1 < rule.anchors.size() ? ",\n" : "\n";
    }
    out += "      ]\n    }";
    out += r + 1 < profile.rules.size() ? ",\n" : "\n";
  }
  out += "  ],\n  \"join\": {\n";
  out += "    \"selection_roi\": [" + formatNumber(profile.join.selectionRoi.x) + ", " +
         formatNumber(profile.join.selectionRoi.y) + ", " +
         formatNumber(profile.join.selectionRoi.w) + ", " +
         formatNumber(profile.join.selectionRoi.h) + "],\n";
  out += "    \"selection_minimum\": " + formatNumber(profile.join.selectionMinimum) + ",\n";
  out += "    \"button_roi\": [" + formatNumber(profile.join.buttonRoi.x) + ", " +
         formatNumber(profile.join.buttonRoi.y) + ", " + formatNumber(profile.join.buttonRoi.w) +
         ", " + formatNumber(profile.join.buttonRoi.h) + "],\n";
  out += "    \"button_minimum\": " + formatNumber(profile.join.buttonMinimum) + ",\n";
  out += "    \"click\": [" + formatNumber(profile.join.click.x) + ", " +
         formatNumber(profile.join.click.y) + "],\n";
  out += "    \"label\": " + quote(wideToUtf8(profile.join.label)) + "\n";
  out += "  },\n  \"launcher\": [\n";
  for (std::size_t i = 0; i < profile.launcher.size(); ++i) {
    const auto &anchor = profile.launcher[i];
    out += "    { \"rect\": [" + formatNumber(anchor.x) + ", " + formatNumber(anchor.y) + ", " +
           formatNumber(anchor.w) + ", " + formatNumber(anchor.h) +
           "], \"minimum\": " + formatNumber(anchor.minimum);
    if (anchor.clickable)
      out += ", \"click\": [" + formatNumber(anchor.clickX) + ", " + formatNumber(anchor.clickY) +
             "]";
    out += ",\n      \"variants\": [\n";
    for (std::size_t v = 0; v < anchor.variants.size(); ++v) {
      out += "        " + quote(encodeTemplate(anchor.variants[v]));
      out += v + 1 < anchor.variants.size() ? ",\n" : "\n";
    }
    out += "      ] }";
    out += i + 1 < profile.launcher.size() ? ",\n" : "\n";
  }
  out += "  ]\n}\n";
  return out;
}
} // namespace owh
