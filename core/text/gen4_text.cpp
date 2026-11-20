#include "text/gen4_text.hpp"

#include <cstdio>
#include <map>
#include <stdexcept>
#include <string_view>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::text4 {

namespace {
using Bytes = std::vector<uint8_t>;

// UnicodeParser: tb[código] = texto y d[texto] = código (si se repite, gana la última línea, como el HashMap)
struct Table {
    std::vector<std::u32string> tb;
    std::vector<bool> has;
    std::map<std::u32string, int> d;
    Table() : tb(65536), has(65536, false) {
        auto file = embeddedFile("texttables/Generation4.tbl");
        if (!file) throw std::runtime_error("Couldn't find /text/Generation4.tbl");
        std::string_view all = *file;
        size_t pos = 0;
        while (pos <= all.size()) {
            size_t nl = all.find('\n', pos);
            std::string_view line = all.substr(pos, nl == std::string_view::npos ? std::string_view::npos : nl - pos);
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
            pos = nl == std::string_view::npos ? all.size() + 1 : nl + 1;
            if (jc::trim(std::string(line)).empty()) continue;
            size_t eq = line.find('=');
            if (eq == std::string_view::npos) continue;
            int code = std::stoi(std::string(line.substr(0, eq)), nullptr, 16);
            std::u32string value = jc::utf8ToU32(std::string(line.substr(eq + 1)));
            tb[code] = value;
            has[code] = true;
            d[value] = code;
        }
    }
};

const Table& table() {
    static Table t;
    return t;
}

int read16(const Bytes& b, size_t o) { return b.at(o) | (b.at(o + 1) << 8); }
void write16(Bytes& b, size_t o, int v) {
    b.at(o) = static_cast<uint8_t>(v & 0xFF);
    b.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int read32(const Bytes& b, size_t o) {
    return static_cast<int>(static_cast<uint32_t>(b.at(o)) | (static_cast<uint32_t>(b.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(b.at(o + 2)) << 16) | (static_cast<uint32_t>(b.at(o + 3)) << 24));
}
void write32(Bytes& b, size_t o, int v) {
    for (int i = 0; i < 4; i++) b.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}

void decryptPtrs(Bytes& data, int count, int key, int sdidx) {
    key = (key * 0x2FD) & 0xFFFF;
    for (int i = 0; i < count; i++) {
        int key2 = (key * (i + 1)) & 0xFFFF;
        int realkey = key2 | (key2 << 16);
        write32(data, sdidx, read32(data, sdidx) ^ realkey);
        write32(data, sdidx + 4, read32(data, sdidx + 4) ^ realkey);
        sdidx += 8;
    }
}

void decryptTxt(Bytes& data, int count, int id, int idx) {
    int key = (0x91BD3 * id) & 0xFFFF;
    for (int i = 0; i < count; i++) {
        write16(data, idx, read16(data, idx) ^ key);
        key = (key + 0x493D) & 0xFFFF;
        idx += 2;
    }
}

std::string hex4(int v) {
    char buf[8];
    snprintf(buf, sizeof buf, "%04X", v & 0xFFFF);
    return buf;
}

std::u32string ascii(const std::string& s) { return std::u32string(s.begin(), s.end()); }

std::u32string makeString(const Bytes& data, int count, int idx, bool& compressFlag) {
    const Table& t = table();
    std::vector<int> chars;
    for (int i = 0; i < count; i++) {
        chars.push_back(read16(data, idx));
        idx += 2;
    }
    if (!chars.empty() && chars[0] == 0xF100) {
        compressFlag = true;
        std::vector<int> uncomp;
        size_t j = 1;
        int shift1 = 0, trans = 0;
        while (true) {
            if (j >= chars.size()) throw std::runtime_error("Texto comprimido incorrecto");
            if (shift1 >= 0xF) {
                shift1 -= 0xF;
                if (shift1 > 0) {
                    int tmp1 = (trans | ((chars[j] << (9 - shift1)) & 0x1FF));
                    if (tmp1 == 0x1FF) break;
                    uncomp.push_back(tmp1);
                }
            } else {
                int tmp1 = ((chars[j] >> shift1) & 0x1FF);
                if (tmp1 == 0x1FF) break;
                uncomp.push_back(tmp1);
                shift1 += 9;
                if (shift1 < 0xF) {
                    trans = ((chars[j] >> shift1) & 0x1FF);
                    shift1 += 9;
                }
                j += 1;
            }
        }
        chars = uncomp;
    }
    std::u32string out;
    size_t i = 0;
    for (size_t c = 0; c < chars.size(); c++) {
        if (i >= chars.size()) throw std::runtime_error("Texto incorrecto");
        int cur = chars[i];
        if (t.has[cur]) {
            out += t.tb[cur];
        } else if (cur == 0xFFFE) {
            i++;
            out += ascii("\\v" + hex4(chars.at(i)));
            i++;
            int total = chars.at(i);
            if (total == 0) out += ascii("\\x" + hex4(0));
            for (int z = 0; z < total; z++) {
                i++;
                out += ascii("\\z" + hex4(chars.at(i)));
            }
        } else if (cur == 0xFFFF) {
            break;
        } else {
            out += ascii("\\x" + hex4(cur));
        }
        i++;
    }
    return out;
}

int parseHex(const std::u32string& s, size_t from, size_t len) {
    std::string a;
    for (size_t k = from; k < from + len && k < s.size(); k++) a += static_cast<char>(s[k]);
    return std::stoi(a, nullptr, 16);
}

std::vector<int> toCode(std::u32string text, bool compressed) {
    const Table& t = table();
    std::vector<int> data;
    while (!text.empty()) {
        int i = std::max(0, 6 - static_cast<int>(text.size()));
        if (text[0] == U'\\') {
            char32_t c1 = text.size() > 1 ? text[1] : 0;
            if (c1 == U'x') {
                data.push_back(parseHex(text, 2, 4));
                text = text.substr(6);
            } else if (c1 == U'v') {
                data.push_back(0xFFFE);
                data.push_back(parseHex(text, 2, 4));
                text = text.substr(6);
            } else if (c1 == U'z') {
                std::vector<int> var;
                int w = 0;
                while (!text.empty()) {
                    if (text[0] == U'\\' && text.size() > 1 && text[1] == U'z') {
                        w++;
                        var.push_back(parseHex(text, 2, 4));
                        text = text.substr(6);
                    } else {
                        break;
                    }
                }
                data.push_back(w);
                data.insert(data.end(), var.begin(), var.end());
            } else if (c1 == U'n') {
                data.push_back(0xE000);
                text = text.substr(2);
            } else if (c1 == U'p') {
                data.push_back(0x25BC);
                text = text.substr(2);
            } else if (c1 == U'l') {
                data.push_back(0x25BD);
                text = text.substr(2);
            } else if (text.compare(1, 3, U"and") == 0) {
                data.push_back(0x1C2);
                text = text.substr(4);
            } else {
                text = text.substr(std::min<size_t>(2, text.size()));
            }
        } else {
            while (!(t.d.count(text.substr(0, 6 - i)) || i == 6)) i++;
            if (i == 6) {
                text = text.substr(1);  // carácter que no está en la tabla: se omite (como FVX)
            } else {
                data.push_back(t.d.at(text.substr(0, 6 - i)));
                text = text.substr(6 - i);
            }
        }
    }
    if (compressed) {
        if (data.size() % 5 != 0 || data.empty()) data.push_back(0x1FF);
        std::vector<uint8_t> bits;
        for (int v : data)
            for (int j = 0; j < 9; j++) bits.push_back(static_cast<uint8_t>((v >> j) & 1));
        int tmp = 0;
        data.clear();
        data.push_back(0xF100);
        for (size_t i = 0; i < bits.size(); i++) {
            if (i % 15 == 0 && i != 0) {
                data.push_back(tmp);
                tmp = 0;
            }
            tmp |= (bits[i] << (i % 15));
        }
        data.push_back(tmp);
    }
    data.push_back(0xFFFF);
    return data;
}
}  // namespace

Decoded readTexts(const Bytes& input) {
    Bytes data = input;
    Decoded out;
    int num = read16(data, 0);
    decryptPtrs(data, num, read16(data, 2), 4);
    std::vector<std::pair<int, int>> ptrs;
    for (int i = 0; i < num; i++) ptrs.push_back({read32(data, 4 + i * 8), read32(data, 8 + i * 8)});
    for (int i = 0; i < num; i++) {
        decryptTxt(data, ptrs[i].second, i + 1, ptrs[i].first);
        out.strings.push_back(jc::u32ToUtf8(makeString(data, ptrs[i].second, ptrs[i].first, out.compressed)));
    }
    return out;
}

Bytes makeEncryptedFile(const std::vector<std::string>& textarr, bool compressed) {
    // TextToPoke.MakeFile
    int base = static_cast<int>(textarr.size()) * 8 + 4;
    std::vector<std::pair<int, int>> ptrtable;
    std::vector<std::vector<int>> raw;
    for (const auto& s : textarr) {
        auto d = toCode(jc::utf8ToU32(s), compressed);
        int l = static_cast<int>(d.size());
        ptrtable.push_back({base, l});
        raw.push_back(std::move(d));
        base += l * 2;
    }
    Bytes data(4 + textarr.size() * 8, 0);
    write16(data, 0, static_cast<int>(textarr.size()));
    write16(data, 2, 0);
    for (size_t i = 0; i < ptrtable.size(); i++) {
        write32(data, 4 + i * 8, ptrtable[i].first);
        write32(data, 8 + i * 8, ptrtable[i].second);
    }
    for (const auto& r : raw)
        for (int w : r) {
            data.push_back(static_cast<uint8_t>(w & 0xFF));
            data.push_back(static_cast<uint8_t>((w >> 8) & 0xFF));
        }
    // PokeTextData: SetKey(0xD00E) + encrypt()
    write16(data, 2, 0xD00E);
    int num = read16(data, 0);
    for (int i = 0; i < num; i++) decryptTxt(data, read32(data, 8 + i * 8), i + 1, read32(data, 4 + i * 8));
    decryptPtrs(data, num, read16(data, 2), 4);
    return data;
}

}  // namespace pr::text4
