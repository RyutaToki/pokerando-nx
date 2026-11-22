#include <cctype>
#include <algorithm>
#include "romio/rom_functions.hpp"

#include <stdexcept>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::romfn {

std::string replaceAll(std::string s, const std::string& from, const std::string& to) {
    if (from.empty()) return s;
    std::string out;
    size_t pos = 0;
    while (true) {
        size_t f = s.find(from, pos);
        if (f == std::string::npos) break;
        out.append(s, pos, f - pos);
        out += to;
        pos = f + from.size();
    }
    out.append(s, pos, std::string::npos);
    return out;
}

namespace {
char32_t lastChar(const std::string& w) {
    auto u = jc::utf8ToU32(w);
    return u.empty() ? 0 : u.back();
}
}  // namespace

std::string rewriteDescriptionForNewLineSize(std::string d, const std::string& newline, int lineSize,
                                             const StringSizeDeterminer& ssd) {
    d = replaceAll(replaceAll(d, "-" + newline, ""), newline, " ");
    d = replaceAll(d, "Sp. Atk", "Sp__Atk");
    d = replaceAll(d, "Sp. Def", "Sp__Def");
    d = replaceAll(d, "SP. ATK", "SP__ATK");
    d = replaceAll(d, "SP. DEF", "SP__DEF");
    auto words = jc::split(d, " ");
    std::string full, line;
    int wc = 0, cc = 0, written = 0;
    for (auto& w : words) {
        w = replaceAll(replaceAll(w, "SP__", "SP. "), "Sp__", "Sp. ");
        int req = ssd(w);
        if (wc > 0) req++;
        if (cc + req <= lineSize) {
            if (wc > 0) line += ' ';
            line += w;
            wc++;
            cc += req;
        } else {
            if (wc > 0) {
                if (written > 0) full += newline;
                full += line;
                written++;
                line.clear();
            }
            line = w;
            wc = 1;
            cc = ssd(w);
        }
    }
    if (wc > 0) {
        if (written > 0) full += newline;
        full += line;
    }
    return full;
}

std::string formatTextWithReplacements(std::string text, const std::vector<std::pair<std::string, std::string>>* repl,
                                       const std::string& newline, const std::string& extraline,
                                       const std::string& newpara, int maxLineLength, const StringSizeDeterminer& ssd) {
    bool endsWithPara = false;
    if (jc::endsWith(text, newpara)) {
        endsWithPara = true;
        text = text.substr(0, text.size() - newpara.size());
    }
    text = replaceAll(replaceAll(text, newline, " "), extraline, " ");
    if (repl) {
        int index = 0;
        for (auto& [k, v] : *repl) text = replaceAll(text, k, "<tmpreplace" + std::to_string(++index) + ">");
        index = 0;
        for (auto& [k, v] : *repl) text = replaceAll(text, "<tmpreplace" + std::to_string(++index) + ">", v);
    }
    auto paragraphs = jc::split(text, newpara);
    std::string result;
    int sentenceNewLineSize = std::max(10, maxLineLength / 2);
    for (size_t para = 0; para < paragraphs.size(); para++) {
        auto words = jc::split(paragraphs[para], " ");
        std::string full, line;
        int wc = 0, cc = 0, written = 0;
        char32_t last = 0;
        for (const auto& w : words) {
            int req = ssd(w);
            if (wc > 0) req++;
            if (cc + req > maxLineLength ||
                (cc >= sentenceNewLineSize && (last == '.' || last == '?' || last == '!' || last == ','))) {
                if (wc > 0) {
                    if (written > 1) full += extraline;
                    else if (written == 1) full += newline;
                    full += line;
                    written++;
                    line.clear();
                }
                line = w;
                wc = 1;
                cc = ssd(w);
            } else {
                if (wc > 0) line += ' ';
                line += w;
                wc++;
                cc += req;
            }
            last = lastChar(w);
        }
        if (wc > 0) {
            if (written > 1) full += extraline;
            else if (written == 1) full += newline;
            full += line;
        }
        if (para > 0) result += newpara;
        result += full;
    }
    if (endsWithPara) result += newpara;
    return result;
}

std::string camelCase(const std::string& original) {
    auto s = jc::utf8ToU32(original);
    for (auto& c : s) {
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        else if (c >= 0xC0 && c <= 0xDE && c != 0xD7) c += 0x20;  // Latin-1 (Java toLowerCase)
    }
    auto isLetter = [](char32_t c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0x24F && c != 0xD7 && c != 0xF7);
    };
    bool docap = true;
    for (auto& c : s) {
        if (docap && isLetter(c)) {
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            else if (c >= 0xE0 && c <= 0xFE && c != 0xF7) c -= 0x20;
            docap = false;
        } else if (!docap && !isLetter(c) && c != '\'' && c != U'’') {
            docap = true;
        }
    }
    return jc::u32ToUtf8(s);
}

void applyPatch(std::vector<uint8_t>& rom, const std::string& patchName) {
    auto data = embeddedFile("patches/" + patchName + ".ips");
    if (!data) throw std::runtime_error("No se encuentra el parche " + patchName);
    const auto* p = reinterpret_cast<const uint8_t*>(data->data());
    size_t len = data->size();
    if (len < 8 || std::string_view(data->data(), 5) != "PATCH") throw std::runtime_error("not a valid IPS file");
    size_t off = 5;
    while (off + 2 < len) {
        size_t writeOffset = (p[off] << 16) | (p[off + 1] << 8) | p[off + 2];
        if (writeOffset == 0x454f46) return;  // EOF
        off += 3;
        if (off + 1 >= len) throw std::runtime_error("abrupt ending to IPS file, entry cut off before size");
        size_t size = (p[off] << 8) | p[off + 1];
        off += 2;
        if (size == 0) {
            if (off + 1 >= len) throw std::runtime_error("abrupt ending to IPS file, entry cut off before RLE size");
            size_t rle = (p[off] << 8) | p[off + 1];
            if (writeOffset + rle > rom.size()) throw std::runtime_error("trying to patch data past the end of the ROM file");
            off += 2;
            if (off >= len) throw std::runtime_error("abrupt ending to IPS file, entry cut off before RLE byte");
            uint8_t b = p[off++];
            for (size_t i = writeOffset; i < writeOffset + rle; i++) rom[i] = b;
        } else {
            if (off + size > len) throw std::runtime_error("abrupt ending to IPS file, entry cut off before end of data block");
            if (writeOffset + size > rom.size()) throw std::runtime_error("trying to patch data past the end of the ROM file");
            std::copy(p + off, p + off + size, rom.begin() + static_cast<long>(writeOffset));
            off += size;
        }
    }
    throw std::runtime_error("improperly terminated IPS file");
}

std::vector<uint8_t> hexToBytes(const std::string& hexIn) {
    // como en Java: se quitan los espacios en blanco y la longitud tiene que ser par
    std::string hex;
    for (char c : hexIn)
        if (!std::isspace(static_cast<unsigned char>(c))) hex.push_back(c);
    if (hex.size() % 2 != 0) throw std::invalid_argument("hex string is not of even length");
    std::vector<uint8_t> out;
    for (size_t i = 0; i < hex.size(); i += 2) out.push_back(static_cast<uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    return out;
}

std::vector<int> search(const std::vector<uint8_t>& hay, const std::vector<uint8_t>& needle, int begin, int end) {
    std::vector<int> out;
    if (end < 0) end = static_cast<int>(hay.size());
    if (needle.empty()) return out;
    int i = begin;
    while (i + static_cast<int>(needle.size()) <= end) {
        if (std::equal(needle.begin(), needle.end(), hay.begin() + i)) {
            out.push_back(i);
            i += static_cast<int>(needle.size());
        } else {
            i++;
        }
    }
    return out;
}

int searchForFirst(const std::vector<uint8_t>& hay, int begin, const std::vector<uint8_t>& needle) {
    if (needle.empty()) return -1;
    for (int i = begin; i + static_cast<int>(needle.size()) <= static_cast<int>(hay.size()); i++)
        if (std::equal(needle.begin(), needle.end(), hay.begin() + i)) return i;
    return -1;
}

}  // namespace pr::romfn
