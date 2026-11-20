#include "romio/gen4/gen4_rom_entry.hpp"

#include <cstdio>
#include <regex>
#include <stdexcept>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::gen4 {

namespace {
constexpr int Type_DP = 0, Type_Plat = 1, Type_HGSS = 2;

InFileEntry parseInFile(const std::string& s) {
    auto parts = jc::split(s, ":");
    return {ini::parseInt(parts.at(0)), ini::parseInt(parts.at(1))};
}

std::vector<std::string> bracketList(const std::string& v) { return jc::split(v.substr(1, v.size() - 2), ","); }

// AbstractDSRomEntry.DSRomEntryReader.parseStaticPokemon
DSStaticPokemon parseStaticPokemon(const std::string& s) {
    DSStaticPokemon sp;
    static const std::regex re(R"([A-z]+=\[([0-9]+:0x[0-9a-fA-F]+,?\s?)+\])");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        auto segments = jc::split(it->str(), "=");
        std::vector<InFileEntry> entries;
        for (auto& o : bracketList(segments[1])) entries.push_back(parseInFile(o));
        if (segments[0] == "Species") sp.speciesEntries = entries;
        else if (segments[0] == "Level") sp.levelEntries = entries;
        else if (segments[0] == "Forme") sp.formeEntries = entries;
    }
    return sp;
}

DSStaticPokemon parseStaticPokemonGameCorner(const std::string& s) {
    DSStaticPokemon sp;
    sp.gameCorner = true;
    static const std::regex re(R"([A-z]+=\[([0-9]+:0x[0-9a-fA-F]+,?\s?)+\])");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        auto segments = jc::split(it->str(), "=");
        auto offsets = bracketList(segments[1]);
        if (segments[0] == "Species") {
            sp.speciesEntries.clear();
            for (auto& o : offsets) sp.speciesEntries.push_back(parseInFile(o));
        } else if (segments[0] == "Level") {
            sp.levelEntries.clear();
            for (auto& o : offsets) sp.levelEntries.push_back(parseInFile(o));
        } else if (segments[0] == "Text") {
            sp.textEntries.clear();
            for (auto& o : offsets) {
                InFileEntry e = parseInFile(o);
                sp.textEntries.push_back({e.file, e.offset});
            }
        }
    }
    return sp;
}

RoamingPokemon parseRoamingPokemon(const std::string& s) {
    RoamingPokemon rp;
    static const std::regex re(R"([A-z]+=\[(0x[0-9a-fA-F]+,?\s?)+\]|[A-z]+=\[([0-9]+:0x[0-9a-fA-F]+,?\s?)+\])");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        auto segments = jc::split(it->str(), "=");
        auto offsets = bracketList(segments[1]);
        if (segments[0] == "Species") {
            rp.speciesCodeOffsets.clear();
            for (auto& o : offsets) rp.speciesCodeOffsets.push_back(ini::parseInt(o));
        } else if (segments[0] == "Level") {
            rp.levelCodeOffsets.clear();
            for (auto& o : offsets) rp.levelCodeOffsets.push_back(ini::parseInt(o));
        } else if (segments[0] == "Script") {
            rp.speciesScriptOffsets.clear();
            for (auto& o : offsets) rp.speciesScriptOffsets.push_back(parseInFile(o));
        } else if (segments[0] == "Gender") {
            rp.genderOffsets.clear();
            for (auto& o : offsets) rp.genderOffsets.push_back(parseInFile(o));
        }
    }
    return rp;
}
}  // namespace

void Gen4RomEntry::setRomTypeFromString(const std::string& s) {
    if (jc::equalsIgnoreCase(s, "DP")) setRomType(Type_DP);
    else if (jc::equalsIgnoreCase(s, "Plat")) setRomType(Type_Plat);
    else if (jc::equalsIgnoreCase(s, "HGSS")) setRomType(Type_HGSS);
    else std::fprintf(stderr, "unrecognised rom type: %s\n", s.c_str());
}

const std::string& Gen4RomEntry::getFile(const std::string& key) const {
    auto it = files_.find(key);
    if (it == files_.end()) throw std::invalid_argument("File \"" + key + "\" does not exist.");
    return it->second.path;
}

int64_t Gen4RomEntry::getFileExpectedCRC32(const std::string& key) const {
    auto it = files_.find(key);
    return it == files_.end() ? 0 : it->second.crc;
}

std::vector<std::string> Gen4RomEntry::getFileKeys() const {
    JavaHashSet<std::string> keys;
    for (const auto& [k, v] : files_) keys.add(k);
    std::vector<std::string> out;
    keys.forEach([&](const std::string& k) { out.push_back(k); });
    return out;
}

void Gen4RomEntry::copyFrom(const IniEntry& other) {
    // RomEntry/IniEntry.copyFrom
    IniEntry::copyFrom(other);
    auto* o = dynamic_cast<const Gen4RomEntry*>(&other);
    if (!o) return;
    // AbstractDSRomEntry.copyFrom
    for (const auto& [k, v] : o->files_) files_[k] = v;
    if (copyStaticPokemon_) {
        staticPokemon_.insert(staticPokemon_.end(), o->staticPokemon_.begin(), o->staticPokemon_.end());
        staticPokemonSupport_ = true;
    } else {
        staticPokemonSupport_ = false;
    }
    // Gen4RomEntry.copyFrom
    if (copyStaticPokemon_ && ignoreGameCornerStatics_) {
        std::erase_if(staticPokemon_, [](const DSStaticPokemon& sp) { return sp.gameCorner; });
    }
    if (copyRoamingPokemon_) roamingPokemon_.insert(roamingPokemon_.end(), o->roamingPokemon_.begin(), o->roamingPokemon_.end());
    if (copyText_) {
        o->tmTexts_.forEach([&](const int& k, const std::vector<TextEntry>& v) { tmTexts_.put(k, v); });
        o->tmTextsGameCorner_.forEach([&](const int& k, const TextEntry& v) { tmTextsGameCorner_.put(k, v); });
        o->tmScriptOffsetsFrontier_.forEach([&](const int& k, const int& v) { tmScriptOffsetsFrontier_.put(k, v); });
        o->tmTextsFrontier_.forEach([&](const int& k, const int& v) { tmTextsFrontier_.put(k, v); });
    }
    marillCryScriptEntries_.insert(marillCryScriptEntries_.end(), o->marillCryScriptEntries_.begin(), o->marillCryScriptEntries_.end());
}

struct Gen4ReaderSetup {
    static std::vector<std::unique_ptr<Gen4RomEntry>> readAll() {
        using E = Gen4RomEntry;
        using R = IniEntryReader<E>;
        R reader(R::DefaultReadMode::STRING, R::CopyFromMode::NAME, [](const std::string& n) { return std::make_unique<E>(n); });
        // RomEntryReader
        reader.putSpecialKeyMethod("Game", [](E& e, const std::string& v) { e.setRomCode(v); });
        reader.putSpecialKeyMethod("Version", [](E& e, const std::string& v) { e.setVersion(ini::parseInt(v)); });
        reader.putSpecialKeyMethod("Type", [](E& e, const std::string& v) { e.setRomTypeFromString(v); });
        reader.putKeySuffixMethod("Tweak", [](E& e, const std::string& k, const std::string& v) { e.putTweakFile(k, v); });
        // DSRomEntryReader
        reader.putSpecialKeyMethod("Arm9CRC32", [](E& e, const std::string& v) { e.arm9ExpectedCRC32_ = ini::parseLong("0x" + v); });
        reader.putSpecialKeyMethod("StaticPokemonSupport", [](E& e, const std::string& v) { e.staticPokemonSupport_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("CopyStaticPokemon", [](E& e, const std::string& v) { e.copyStaticPokemon_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("CopyRoamingPokemon", [](E& e, const std::string& v) { e.copyRoamingPokemon_ = ini::parseBoolean(v); });
        reader.putKeyPrefixMethod("File<", [](E& e, const std::string& k, const std::string& v) {
            std::string key = jc::split(jc::split(k, "<")[1], ">")[0];
            auto values = jc::split(v.substr(1, v.size() - 2), ",");
            e.files_[key] = FileEntryAccess::make(jc::trim(values.at(0)), ini::parseLong("0x" + jc::trim(values.at(1))));
        });
        reader.putKeyPrefixMethod("OverlayCRC32<", [](E& e, const std::string& k, const std::string& v) {
            int key = ini::parseInt(jc::split(jc::split(k, "<")[1], ">")[0]);
            e.overlayCRC32s_.put(key, ini::parseLong("0x" + v));
        });
        reader.putSpecialKeyMethod("StaticPokemon{}", [](E& e, const std::string& v) { e.staticPokemon_.push_back(parseStaticPokemon(v)); });
        auto addInt = [](E& e, const std::string& k, const std::string& v) { e.putInt(k, ini::parseInt(v)); };
        reader.putKeySuffixMethod("Offset", addInt);
        reader.putKeySuffixMethod("Count", addInt);
        reader.putKeySuffixMethod("Number", addInt);
        reader.putKeySuffixMethod("Size", addInt);
        reader.putKeySuffixMethod("Index", addInt);
        // Gen4RomEntryReader ("Type" se registra otra vez con el mismo efecto)
        reader.putSpecialKeyMethod("CopyText", [](E& e, const std::string& v) { e.copyText_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("IgnoreGameCornerStatics", [](E& e, const std::string& v) { e.ignoreGameCornerStatics_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("RoamingPokemon{}", [](E& e, const std::string& v) { e.roamingPokemon_.push_back(parseRoamingPokemon(v)); });
        reader.putSpecialKeyMethod("StaticPokemonGameCorner{}", [](E& e, const std::string& v) { e.staticPokemon_.push_back(parseStaticPokemonGameCorner(v)); });
        reader.putSpecialKeyMethod("TMText{}", [](E& e, const std::string& v) {
            static const std::regex re(R"([0-9]+=\[([0-9]+:[0-9]+,?\s?)+\])");
            for (auto it = std::sregex_iterator(v.begin(), v.end(), re); it != std::sregex_iterator(); ++it) {
                auto segments = jc::split(it->str(), "=");
                int tmNum = ini::parseInt(segments[0]);
                std::vector<TextEntry> entries;
                for (auto& en : bracketList(segments[1])) {
                    auto ts = jc::split(en, ":");
                    entries.push_back({ini::parseInt(ts.at(0)), ini::parseInt(ts.at(1))});
                }
                e.tmTexts_.put(tmNum, entries);
            }
        });
        reader.putSpecialKeyMethod("TMTextGameCorner{}", [](E& e, const std::string& v) {
            for (auto& entry : bracketList(v)) {
                auto segments = jc::split(jc::trim(entry), "=");
                int tmNum = ini::parseInt(segments.at(0));
                std::string te = segments.at(1).substr(1, segments.at(1).size() - 2);
                auto ts = jc::split(te, ":");
                e.tmTextsGameCorner_.put(tmNum, TextEntry{ini::parseInt(ts.at(0)), ini::parseInt(ts.at(1))});
            }
        });
        reader.putSpecialKeyMethod("FrontierScriptTMOffsets{}", [](E& e, const std::string& v) {
            for (auto& off : bracketList(v)) {
                auto parts = jc::split(off, "=");
                e.tmScriptOffsetsFrontier_.put(ini::parseInt(parts.at(0)), ini::parseInt(parts.at(1)));
            }
        });
        reader.putSpecialKeyMethod("FrontierTMText{}", [](E& e, const std::string& v) {
            for (auto& off : bracketList(v)) {
                auto parts = jc::split(off, "=");
                e.tmTextsFrontier_.put(ini::parseInt(parts.at(0)), ini::parseInt(parts.at(1)));
            }
        });
        reader.putKeySuffixMethod("MarillCryScripts", [](E& e, const std::string&, const std::string& v) {
            e.marillCryScriptEntries_.clear();
            for (auto& off : bracketList(v)) e.marillCryScriptEntries_.push_back(parseInFile(off));
        });

        auto text = embeddedFile("romentries/gen4_offsets.ini");
        if (!text) throw std::runtime_error("Falta romentries/gen4_offsets.ini");
        return reader.read(*text, "gen4_offsets.ini");
    }

    struct FileEntryAccess {
        static Gen4RomEntry::FileEntry make(std::string p, int64_t c) { return {std::move(p), c}; }
    };
};

const std::vector<std::unique_ptr<Gen4RomEntry>>& Gen4RomEntry::all() {
    static const auto entries = Gen4ReaderSetup::readAll();
    return entries;
}

}  // namespace pr::gen4
