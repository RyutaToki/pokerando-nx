#include "text/gen5_text.hpp"

#include <algorithm>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string_view>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::text5 {

// ---------------- UTF-8 <-> UTF-16 ----------------

std::u16string toU16(const std::string& s) {
    std::u16string out;
    size_t i = 0;
    while (i < s.size()) {
        uint8_t c = static_cast<uint8_t>(s[i]);
        uint32_t cp;
        int n;
        if (c < 0x80) {
            cp = c;
            n = 1;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            n = 2;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            n = 3;
        } else {
            cp = c & 0x07;
            n = 4;
        }
        for (int k = 1; k < n && i + k < s.size(); k++) cp = (cp << 6) | (static_cast<uint8_t>(s[i + k]) & 0x3F);
        i += n;
        if (cp >= 0x10000) {
            cp -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 | (cp >> 10)));
            out.push_back(static_cast<char16_t>(0xDC00 | (cp & 0x3FF)));
        } else {
            out.push_back(static_cast<char16_t>(cp));
        }
    }
    return out;
}

std::string fromU16(const std::u16string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        uint32_t cp = s[i];
        if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < s.size() && s[i + 1] >= 0xDC00 && s[i + 1] < 0xE000) {
            cp = 0x10000 + ((cp - 0xD800) << 10) + (s[i + 1] - 0xDC00);
            i++;
        }
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

int javaLength(const std::string& s) { return static_cast<int>(toU16(s).size()); }

namespace {
using Bytes = std::vector<uint8_t>;

// pokeToText: carácter del juego -> texto; textToPoke: texto -> "\xNNNN" (gana la última línea repetida)
struct Table {
    std::map<char16_t, std::u16string> pokeToText;
    std::map<std::u16string, std::u16string> textToPoke;
    Table() {
        auto file = embeddedFile("texttables/Generation5.tbl");
        if (!file) throw std::runtime_error("Couldn't find /text/Generation5.tbl");
        std::string_view all = *file;
        size_t pos = 0;
        while (pos < all.size()) {
            size_t nl = all.find('\n', pos);
            std::string line(all.substr(pos, nl == std::string_view::npos ? std::string_view::npos : nl - pos));
            pos = nl == std::string_view::npos ? all.size() : nl + 1;
            if (!line.empty() && line.back() == '\r') line.pop_back();  // Scanner.nextLine quita \r\n
            if (jc::trim(line).empty()) continue;
            auto r = jc::split(line, "=", 2);
            std::u16string value = toU16(r.at(1));
            pokeToText[static_cast<char16_t>(std::stoi(r[0], nullptr, 16))] = value;
            textToPoke[value] = u"\\x" + toU16(r[0]);
        }
    }
};

const Table& table() {
    static Table t;
    return t;
}

int readWord(const Bytes& d, size_t o) { return d.at(o) | (d.at(o + 1) << 8); }
int readLong(const Bytes& d, size_t o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeWord(Bytes& d, size_t o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
void writeLong(Bytes& d, size_t o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}

// Character.UnicodeBlock.of(c) != null en Java 25: en el plano básico solo falta 2FE0-2FEF
bool hasUnicodeBlock(int c) { return !(c >= 0x2FE0 && c <= 0x2FEF); }

std::vector<int> decompress(const std::vector<int>& chars) {
    std::vector<int> uncomp;
    size_t j = 1;
    int shift1 = 0, trans = 0;
    while (true) {
        int tmp1;
        if (shift1 >= 0x10) {
            shift1 -= 0x10;
            if (shift1 > 0) {
                tmp1 = (trans | ((chars.at(j) << (9 - shift1)) & 0x1FF));
                if ((tmp1 & 0xFF) == 0xFF) break;
                if (tmp1 != 0x0 && tmp1 != 0x1) uncomp.push_back(tmp1);
            }
        } else {
            tmp1 = ((chars.at(j) >> shift1) & 0x1FF);
            if ((tmp1 & 0xFF) == 0xFF) break;
            if (tmp1 != 0x0 && tmp1 != 0x1) uncomp.push_back(tmp1);
            shift1 += 9;
            if (shift1 < 0x10) {
                trans = ((chars.at(j) >> shift1) & 0x1FF);
                shift1 += 9;
            }
            j += 1;
        }
    }
    return uncomp;
}

struct ReadState {
    std::vector<std::u16string> strings;
    std::vector<int> lastKeys, lastUnknowns;
};

std::string hex4(int v) {
    char buf[8];
    std::snprintf(buf, sizeof buf, "%04X", v);
    return buf;
}

ReadState readRaw(const Bytes& ds) {
    ReadState st;
    int numSections = readWord(ds, 0);
    int numEntries = readWord(ds, 2);
    size_t pos = 12;
    if (numSections > 0) {
        int sectionOffset[3] = {0, 0, 0};
        for (int z = 0; z < numSections; z++) {
            sectionOffset[z] = readLong(ds, pos);  // (Java: sectionOffset tiene 3 huecos; más secciones lanzaría)
            pos += 4;
        }
        pos = sectionOffset[0];
        pos += 4;  // tamaño de la sección
        std::vector<int> tableOffsets, characterCount;
        for (int j = 0; j < numEntries; j++) {
            tableOffsets.push_back(readLong(ds, pos));
            pos += 4;
            characterCount.push_back(readWord(ds, pos));
            pos += 2;
            int unk = readWord(ds, pos);
            pos += 2;
            st.lastUnknowns.push_back(unk);
        }
        for (int j = 0; j < numEntries; j++) {
            std::vector<int> enc;
            pos = sectionOffset[0] + tableOffsets[j];
            for (int k = 0; k < characterCount[j]; k++) {
                enc.push_back(readWord(ds, pos));
                pos += 2;
            }
            int key = enc.at(characterCount[j] - 1) ^ 0xFFFF;
            for (int k = characterCount[j] - 1; k >= 0; k--) {
                enc[k] = enc[k] ^ key;
                if (k == 0) st.lastKeys.push_back(key);
                key = ((static_cast<unsigned>(key) >> 3) | (key << 13)) & 0xffff;
            }
            if (enc.at(0) == 0xF100) enc = decompress(enc);
            std::u16string sb;
            for (int c : enc) {
                if (c == 0xFFFF) continue;
                if (c > 20 && c <= 0xFFF0 && hasUnicodeBlock(c)) {
                    auto it = table().pokeToText.find(static_cast<char16_t>(c));
                    if (it != table().pokeToText.end()) sb += it->second;
                    else sb.push_back(static_cast<char16_t>(c));
                } else {
                    std::string num = "\\x" + hex4(c);
                    sb += std::u16string(num.begin(), num.end());
                }
            }
            st.strings.push_back(std::move(sb));
        }
    }
    return st;
}

// bulkReplace con textToPokePattern: ningún texto de la tabla es prefijo de otro que empiece en el mismo
// sitio, así que el orden de la alternativa de la regex no cambia el resultado
std::u16string toPoke(const std::u16string& s) {
    const auto& m = table().textToPoke;
    std::u16string out;
    size_t i = 0;
    while (i < s.size()) {
        bool done = false;
        for (const auto& [k, v] : m) {
            if (s.compare(i, k.size(), k) == 0) {
                out += v;
                i += k.size();
                done = true;
                break;
            }
        }
        if (!done) out.push_back(s[i++]);
    }
    return out;
}

std::vector<int> parseString(const std::u16string& string, int entryId, const ReadState& st) {
    std::vector<int> chars;
    for (size_t i = 0; i < string.size(); i++) {
        if (string[i] != u'\\') {
            chars.push_back(string[i]);
        } else if (i + 2 < string.size() && string[i + 2] == u'{') {
            chars.push_back(string[i]);
        } else {
            if (i + 6 > string.size()) throw std::out_of_range("begin " + std::to_string(i + 2) + ", end " + std::to_string(i + 6));
            std::string hex;
            for (size_t k = i + 2; k < i + 6; k++) hex.push_back(static_cast<char>(string[k] < 0x80 ? string[k] : '?'));
            int32_t v;
            if (!jc::parseIntRadix(hex, 16, v)) throw std::invalid_argument("For input string: \"" + hex + "\"");
            chars.push_back(v);
            i += 5;
        }
    }
    chars.push_back(0xFFFF);
    int key = st.lastKeys.at(entryId);
    for (int& c : chars) {
        c = (c ^ key) & 0xFFFF;
        key = ((key << 3) | (static_cast<unsigned>(key) >> 13)) & 0xFFFF;
    }
    return chars;
}

Bytes makeSection(const std::vector<std::u16string>& strings, int numEntries, const ReadState& st) {
    std::vector<std::vector<int>> data;
    int size = 0;
    int offset = 4 + 8 * numEntries;
    for (int i = 0; i < numEntries; i++) {
        data.push_back(parseString(strings[i], i, st));
        size += static_cast<int>(data[i].size()) * 2;
    }
    if (size % 4 == 2) {
        size += 2;
        int tmpKey = st.lastKeys.at(numEntries - 1);
        for (size_t i = 0; i < data[numEntries - 1].size(); i++) tmpKey = ((tmpKey << 3) | (tmpKey >> 13)) & 0xFFFF;
        data[numEntries - 1].push_back(0xFFFF ^ tmpKey);
    }
    size += offset;
    Bytes section(size, 0);
    size_t pos = 0;
    writeLong(section, pos, size);
    pos += 4;
    for (int i = 0; i < numEntries; i++) {
        int charCount = static_cast<int>(data[i].size());
        writeLong(section, pos, offset);
        pos += 4;
        writeWord(section, pos, charCount);
        pos += 2;
        writeWord(section, pos, st.lastUnknowns.at(i));
        pos += 2;
        offset += charCount * 2;
    }
    for (int i = 0; i < numEntries; i++) {
        for (int w : data[i]) {
            writeWord(section, pos, w);
            pos += 2;
        }
    }
    return section;
}
}  // namespace

std::vector<std::string> readTexts(const Bytes& data) {
    auto st = readRaw(data);
    std::vector<std::string> out;
    out.reserve(st.strings.size());
    for (const auto& s : st.strings) out.push_back(fromU16(s));
    return out;
}

Bytes saveEntry(const Bytes& originalData, const std::vector<std::string>& text) {
    std::vector<std::u16string> t;
    t.reserve(text.size());
    for (const auto& s : text) t.push_back(toPoke(toU16(s)));
    ReadState st = readRaw(originalData);
    int numSections = readWord(originalData, 0);
    int numEntries = readWord(originalData, 2);
    size_t pos = 12;
    if (static_cast<int>(t.size()) < numEntries) {
        std::fprintf(stderr, "Can't do anything due to too few lines\n");
        return originalData;
    }
    Bytes newEntry = makeSection(t, numEntries, st);
    int sectionOffset[3] = {0, 0, 0}, sizeSections[3] = {0, 0, 0};
    for (int z = 0; z < numSections; z++) {
        sectionOffset[z] = readLong(originalData, pos);
        pos += 4;
    }
    for (int z = 0; z < numSections; z++) sizeSections[z] = readLong(originalData, sectionOffset[z]);
    int newSize0 = static_cast<int>(newEntry.size());
    Bytes newData(originalData.size() - sizeSections[0] + newSize0, 0);
    std::copy_n(originalData.begin(), std::min(originalData.size(), newData.size()), newData.begin());
    writeLong(newData, 4, newSize0);
    int newOffset1 = 0;
    if (numSections == 2) {
        newOffset1 = newSize0 + sectionOffset[0];
        writeLong(newData, 0x10, newOffset1);
    }
    std::copy(newEntry.begin(), newEntry.end(), newData.begin() + sectionOffset[0]);
    if (numSections == 2)
        std::copy_n(originalData.begin() + sectionOffset[1], sizeSections[1], newData.begin() + newOffset1);
    return newData;
}

}  // namespace pr::text5
