#include "util/java_compat.hpp"

#include <cctype>
#include <limits>

namespace pr::jc {

std::string trim(std::string_view s) {
    size_t start = 0, end = s.size();
    while (start < end && static_cast<unsigned char>(s[start]) <= ' ') start++;
    while (end > start && static_cast<unsigned char>(s[end - 1]) <= ' ') end--;
    return std::string(s.substr(start, end - start));
}

std::vector<std::string> split(std::string_view s, std::string_view sep, int limit) {
    std::vector<std::string> parts;
    size_t pos = 0;
    while (true) {
        if (limit > 0 && static_cast<int>(parts.size()) == limit - 1) break;
        size_t found = s.find(sep, pos);
        if (found == std::string_view::npos) break;
        parts.emplace_back(s.substr(pos, found - pos));
        pos = found + sep.size();
    }
    parts.emplace_back(s.substr(pos));
    if (limit == 0) {
        while (!parts.empty() && parts.back().empty()) parts.pop_back();
        // Java devuelve [""] para una cadena vacía de entrada
        if (parts.empty() && s.empty()) parts.emplace_back();
    }
    return parts;
}

std::string toLower(std::string_view s) {
    std::string r(s);
    for (char& c : r) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return r;
}

bool startsWith(std::string_view s, std::string_view p) { return s.substr(0, p.size()) == p; }
bool endsWith(std::string_view s, std::string_view p) {
    return s.size() >= p.size() && s.substr(s.size() - p.size()) == p;
}
bool contains(std::string_view s, std::string_view n) { return s.find(n) != std::string_view::npos; }
bool equalsIgnoreCase(std::string_view a, std::string_view b) { return toLower(a) == toLower(b); }

bool parseLongRadix(std::string_view s, int radix, int64_t& out) {
    if (s.empty()) return false;
    size_t i = 0;
    bool neg = false;
    if (s[0] == '-' || s[0] == '+') {
        neg = s[0] == '-';
        i = 1;
        if (s.size() == 1) return false;
    }
    // acumulamos en negativo, como Long.parseLong, para admitir el mínimo
    const int64_t limit = neg ? std::numeric_limits<int64_t>::min() : -std::numeric_limits<int64_t>::max();
    int64_t result = 0;
    for (; i < s.size(); i++) {
        int c = static_cast<unsigned char>(s[i]);
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
        else return false;
        if (digit >= radix) return false;
        if (result < limit / radix) return false;
        result *= radix;
        if (result < limit + digit) return false;
        result -= digit;
    }
    out = neg ? result : -result;
    return true;
}

bool parseIntRadix(std::string_view s, int radix, int32_t& out) {
    int64_t v;
    if (!parseLongRadix(s, radix, v)) return false;
    if (v < std::numeric_limits<int32_t>::min() || v > std::numeric_limits<int32_t>::max()) return false;
    out = static_cast<int32_t>(v);
    return true;
}

std::u32string utf8ToU32(std::string_view s) {
    std::u32string out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        char32_t cp;
        int len;
        if (c < 0x80) { cp = c; len = 1; }
        else if ((c >> 5) == 0x6) { cp = c & 0x1F; len = 2; }
        else if ((c >> 4) == 0xE) { cp = c & 0x0F; len = 3; }
        else { cp = c & 0x07; len = 4; }
        for (int k = 1; k < len && i + k < s.size(); k++) {
            cp = (cp << 6) | (static_cast<unsigned char>(s[i + k]) & 0x3F);
        }
        out.push_back(cp);
        i += len;
    }
    return out;
}

std::string u32ToUtf8(std::u32string_view s) {
    std::string out;
    for (char32_t cp : s) {
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

std::string toUpperCase(std::string_view s) {
    std::u32string out;
    for (char32_t c : utf8ToU32(s)) {
        if (c >= 'a' && c <= 'z') out += c - 0x20;
        else if (c >= 0xE0 && c <= 0xFE && c != 0xF7) out += c - 0x20;
        else if (c == 0xDF) out += U"SS";
        else if (c == 0xFF) out += char32_t(0x178);
        else if (c == 0xB5) out += char32_t(0x39C);
        else out += c;
    }
    return u32ToUtf8(out);
}

}  // namespace pr::jc
