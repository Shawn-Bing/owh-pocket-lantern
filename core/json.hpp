#pragma once
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace owh {
// The recognition profile is the only structured file this project reads from
// disk. The build deliberately has no third-party dependencies so the Windows
// binaries stay free of extra runtimes, which makes a small strict reader
// cheaper than a JSON library. It accepts the subset the profile uses and
// rejects everything else instead of guessing at malformed input.
struct Json {
  enum class Kind { Null, Bool, Number, String, Array, Object };
  Kind kind{Kind::Null};
  bool boolean{};
  double number{};
  std::string text;
  std::vector<Json> items;
  std::vector<std::pair<std::string, Json>> members;

  bool is(Kind k) const { return kind == k; }
  const Json *find(const std::string &key) const {
    for (const auto &member : members)
      if (member.first == key)
        return &member.second;
    return nullptr;
  }
};

namespace detail {
inline void appendUtf8(std::string &out, unsigned code) {
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

struct Reader {
  const std::string &source;
  std::size_t at{};
  std::string error;
  int depth{};
  // Bounded so a corrupt file cannot exhaust the stack.
  static constexpr int MaxDepth = 32;

  explicit Reader(const std::string &text) : source(text) {}

  void skip() {
    while (at < source.size() &&
           (source[at] == ' ' || source[at] == '\t' || source[at] == '\n' || source[at] == '\r'))
      ++at;
  }
  bool fail(const char *why) {
    if (error.empty())
      error = std::string(why) + " at offset " + std::to_string(at);
    return false;
  }
  bool literal(const char *word) {
    auto length = std::char_traits<char>::length(word);
    if (source.compare(at, length, word) != 0)
      return fail("unexpected token");
    at += length;
    return true;
  }
  bool hex4(unsigned &value) {
    if (at + 4 > source.size())
      return fail("truncated \\u escape");
    value = 0;
    for (int i = 0; i < 4; ++i) {
      char c = source[at++];
      unsigned digit = c >= '0' && c <= '9'   ? unsigned(c - '0')
                       : c >= 'a' && c <= 'f' ? unsigned(c - 'a' + 10)
                       : c >= 'A' && c <= 'F' ? unsigned(c - 'A' + 10)
                                              : 0xffffffffu;
      if (digit == 0xffffffffu)
        return fail("bad hex digit in \\u escape");
      value = value * 16 + digit;
    }
    return true;
  }
  bool string(std::string &out) {
    if (at >= source.size() || source[at] != '"')
      return fail("expected string");
    ++at;
    out.clear();
    while (at < source.size()) {
      unsigned char c = (unsigned char)source[at++];
      if (c == '"')
        return true;
      if (c != '\\') {
        if (c < 0x20)
          return fail("raw control character in string");
        out += char(c);
        continue;
      }
      if (at >= source.size())
        return fail("truncated escape");
      switch (char e = source[at++]) {
      case '"': out += '"'; break;
      case '\\': out += '\\'; break;
      case '/': out += '/'; break;
      case 'b': out += '\b'; break;
      case 'f': out += '\f'; break;
      case 'n': out += '\n'; break;
      case 'r': out += '\r'; break;
      case 't': out += '\t'; break;
      case 'u': {
        unsigned code{};
        if (!hex4(code))
          return false;
        // A lone surrogate is not a character. Reject it rather than silently
        // emitting a replacement byte the template never held.
        if (code >= 0xd800 && code <= 0xdbff) {
          if (at + 1 >= source.size() || source[at] != '\\' || source[at + 1] != 'u')
            return fail("lone high surrogate");
          at += 2;
          unsigned low{};
          if (!hex4(low))
            return false;
          if (low < 0xdc00 || low > 0xdfff)
            return fail("high surrogate not followed by a low surrogate");
          code = 0x10000 + ((code - 0xd800) << 10) + (low - 0xdc00);
        } else if (code >= 0xdc00 && code <= 0xdfff)
          return fail("lone low surrogate");
        appendUtf8(out, code);
        break;
      }
      default: return fail("unknown escape");
      }
    }
    return fail("unterminated string");
  }
  bool number(double &out) {
    auto start = at;
    if (at < source.size() && (source[at] == '-' || source[at] == '+'))
      ++at;
    bool digits = false;
    while (at < source.size() && source[at] >= '0' && source[at] <= '9') {
      ++at;
      digits = true;
    }
    if (at < source.size() && source[at] == '.') {
      ++at;
      while (at < source.size() && source[at] >= '0' && source[at] <= '9') {
        ++at;
        digits = true;
      }
    }
    if (!digits)
      return fail("expected number");
    if (at < source.size() && (source[at] == 'e' || source[at] == 'E')) {
      ++at;
      if (at < source.size() && (source[at] == '-' || source[at] == '+'))
        ++at;
      bool exponent = false;
      while (at < source.size() && source[at] >= '0' && source[at] <= '9') {
        ++at;
        exponent = true;
      }
      if (!exponent)
        return fail("empty exponent");
    }
    try {
      out = std::stod(source.substr(start, at - start));
    } catch (...) {
      return fail("number out of range");
    }
    return true;
  }
  bool value(Json &out);
};

inline bool Reader::value(Json &out) {
  if (depth >= MaxDepth)
    return fail("nesting too deep");
  skip();
  if (at >= source.size())
    return fail("unexpected end of input");
  char c = source[at];
  if (c == '{') {
    ++at;
    out = Json{};
    out.kind = Json::Kind::Object;
    ++depth;
    skip();
    if (at < source.size() && source[at] == '}') {
      ++at;
      --depth;
      return true;
    }
    for (;;) {
      skip();
      std::string key;
      if (!string(key))
        return false;
      skip();
      if (at >= source.size() || source[at] != ':')
        return fail("expected ':'");
      ++at;
      Json member;
      if (!value(member))
        return false;
      // Duplicate keys would make lookups order-dependent, so refuse them.
      if (out.find(key))
        return fail("duplicate object key");
      out.members.emplace_back(std::move(key), std::move(member));
      skip();
      if (at < source.size() && source[at] == ',') {
        ++at;
        continue;
      }
      if (at < source.size() && source[at] == '}') {
        ++at;
        --depth;
        return true;
      }
      return fail("expected ',' or '}'");
    }
  }
  if (c == '[') {
    ++at;
    out = Json{};
    out.kind = Json::Kind::Array;
    ++depth;
    skip();
    if (at < source.size() && source[at] == ']') {
      ++at;
      --depth;
      return true;
    }
    for (;;) {
      Json item;
      if (!value(item))
        return false;
      out.items.push_back(std::move(item));
      skip();
      if (at < source.size() && source[at] == ',') {
        ++at;
        continue;
      }
      if (at < source.size() && source[at] == ']') {
        ++at;
        --depth;
        return true;
      }
      return fail("expected ',' or ']'");
    }
  }
  if (c == '"') {
    out = Json{};
    out.kind = Json::Kind::String;
    return string(out.text);
  }
  if (c == 't' || c == 'f') {
    out = Json{};
    out.kind = Json::Kind::Bool;
    if (c == 't') {
      out.boolean = true;
      return literal("true");
    }
    return literal("false");
  }
  if (c == 'n') {
    out = Json{};
    out.kind = Json::Kind::Null;
    return literal("null");
  }
  out = Json{};
  out.kind = Json::Kind::Number;
  return number(out.number);
}
} // namespace detail

// Returns false and fills error on malformed input or trailing content, so a
// truncated download is never mistaken for a valid profile.
inline bool parseJson(const std::string &text, Json &out, std::string &error) {
  detail::Reader reader(text);
  if (!reader.value(out)) {
    error = reader.error.empty() ? "invalid JSON" : reader.error;
    return false;
  }
  reader.skip();
  if (reader.at != text.size()) {
    error = "trailing content at offset " + std::to_string(reader.at);
    return false;
  }
  error.clear();
  return true;
}
} // namespace owh
