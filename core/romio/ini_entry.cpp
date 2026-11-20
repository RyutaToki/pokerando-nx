#include "romio/ini_entry.hpp"

#include <cstdio>

#include "util/java_compat.hpp"

namespace pr {

int IniEntry::getInt(const std::string& key) { return ints_[key]; }
int IniEntry::getInt(const std::string& key) const {
    auto it = ints_.find(key);
    return it == ints_.end() ? 0 : it->second;
}
const std::string& IniEntry::getString(const std::string& key) { return strings_[key]; }
const std::vector<int>& IniEntry::getArray(const std::string& key) { return arrays_[key]; }
const std::string& IniEntry::getString(const std::string& key) const {
    static const std::string empty;
    auto it = strings_.find(key);
    return it == strings_.end() ? empty : it->second;
}
const std::vector<int>& IniEntry::getArray(const std::string& key) const {
    static const std::vector<int> empty;
    auto it = arrays_.find(key);
    return it == arrays_.end() ? empty : it->second;
}

void IniEntry::copyFrom(const IniEntry& o) {
    // putAll: lo del otro sobrescribe lo que ya hubiera
    for (auto& [k, v] : o.ints_) ints_[k] = v;
    for (auto& [k, v] : o.strings_) strings_[k] = v;
    for (auto& [k, v] : o.arrays_) arrays_[k] = v;
}

std::string RomEntry::getTweakFile(const std::string& key) const {
    auto it = tweakFiles_.find(key);
    return it == tweakFiles_.end() ? std::string() : it->second;
}

void RomEntry::copyRomEntryFields(const RomEntry& o) {
    romCode_ = o.romCode_;
    version_ = o.version_;
    romType_ = o.romType_;
    ints_ = o.ints_;
    strings_ = o.strings_;
    arrays_ = o.arrays_;
    tweakFiles_ = o.tweakFiles_;
}

namespace ini {

int parseInt(std::string_view sv) {
    int radix = 10;
    std::string s = jc::toLower(jc::trim(sv));
    if (jc::startsWith(s, "0x") || jc::startsWith(s, "&h")) {
        radix = 16;
        s = s.substr(2);
    }
    int32_t v;
    if (!jc::parseIntRadix(s, radix, v)) {
        std::fprintf(stderr, "invalid base %d number %s\n", radix, s.c_str());
        return 0;
    }
    return v;
}

int64_t parseLong(std::string_view sv) {
    int radix = 10;
    std::string s = jc::toLower(jc::trim(sv));
    if (jc::startsWith(s, "0x") || jc::startsWith(s, "&h")) {
        radix = 16;
        s = s.substr(2);
    }
    int64_t v;
    if (!jc::parseLongRadix(s, radix, v)) {
        std::fprintf(stderr, "invalid base %d number %s\n", radix, s.c_str());
        return 0;
    }
    return v;
}

bool parseBoolean(std::string_view s) { return parseInt(s) > 0; }

std::vector<int> parseArray(std::string_view s) {
    auto parts = jc::split(s.substr(1, s.size() - 2), ",");
    if (parts.size() == 1 && jc::trim(parts[0]).empty()) return {};
    std::vector<int> v;
    v.reserve(parts.size());
    for (auto& p : parts) v.push_back(parseInt(p));
    return v;
}

}  // namespace ini
}  // namespace pr
