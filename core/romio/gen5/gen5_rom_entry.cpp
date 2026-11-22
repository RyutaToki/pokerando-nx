#include "romio/gen5/gen5_rom_entry.hpp"

#include <cstdio>
#include <regex>
#include <stdexcept>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::gen5 {

namespace {
constexpr int Type_BW = 0, Type_BW2 = 1;

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

RoamingPokemon parseRoamingPokemon(const std::string& s) {
    RoamingPokemon rp;
    static const std::regex re(R"([A-z]+=\[(0x[0-9a-fA-F]+,?\s?)+\]|[A-z]+=\[([0-9]+:0x[0-9a-fA-F]+,?\s?)+\])");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        auto segments = jc::split(it->str(), "=");
        auto offsets = bracketList(segments[1]);
        if (segments[0] == "Species") {
            rp.speciesOverlayOffsets.clear();
            for (auto& o : offsets) rp.speciesOverlayOffsets.push_back(ini::parseInt(o));
        } else if (segments[0] == "Level") {
            rp.levelOverlayOffsets.clear();
            for (auto& o : offsets) rp.levelOverlayOffsets.push_back(ini::parseInt(o));
        } else if (segments[0] == "Script") {
            rp.speciesScriptOffsets.clear();
            for (auto& o : offsets) rp.speciesScriptOffsets.push_back(parseInFile(o));
        }
    }
    return rp;
}
}  // namespace

void Gen5RomEntry::setRomTypeFromString(const std::string& s) {
    if (jc::equalsIgnoreCase(s, "BW1")) setRomType(Type_BW);
    else if (jc::equalsIgnoreCase(s, "BW2")) setRomType(Type_BW2);
    else std::fprintf(stderr, "unrecognised rom type: %s\n", s.c_str());
}

const std::string& Gen5RomEntry::getFile(const std::string& key) const {
    auto it = files_.find(key);
    if (it == files_.end()) throw std::invalid_argument("File \"" + key + "\" does not exist.");
    return it->second.path;
}

int64_t Gen5RomEntry::getFileExpectedCRC32(const std::string& key) const {
    auto it = files_.find(key);
    return it == files_.end() ? 0 : it->second.crc;
}

std::vector<std::string> Gen5RomEntry::getFileKeys() const {
    JavaHashSet<std::string> keys;
    for (const auto& [k, v] : files_) keys.add(k);
    std::vector<std::string> out;
    keys.forEach([&](const std::string& k) { out.push_back(k); });
    return out;
}

const std::vector<InFileEntry>* Gen5RomEntry::getOffsetArrayEntry(const std::string& key) const {
    auto it = offsetArrayEntries_.find(key);
    return it == offsetArrayEntries_.end() ? nullptr : &it->second;
}

std::string Gen5RomEntry::getStoryText(const std::string& key) const {
    auto it = storyTexts_.find(key);
    return it == storyTexts_.end() ? "" : it->second;
}

void Gen5RomEntry::copyFrom(const IniEntry& other) {
    IniEntry::copyFrom(other);
    auto* o = dynamic_cast<const Gen5RomEntry*>(&other);
    if (!o) return;
    // AbstractDSRomEntry.copyFrom
    for (const auto& [k, v] : o->files_) files_[k] = v;
    if (copyStaticPokemon_) {
        staticPokemon_.insert(staticPokemon_.end(), o->staticPokemon_.begin(), o->staticPokemon_.end());
        staticPokemonSupport_ = true;
    } else {
        staticPokemonSupport_ = false;
    }
    // Gen5RomEntry.copyFrom
    for (const auto& [k, v] : o->offsetArrayEntries_) offsetArrayEntries_[k] = v;
    if (copyStaticPokemon_) {
        staticPokemonFakeBall_.insert(staticPokemonFakeBall_.end(), o->staticPokemonFakeBall_.begin(), o->staticPokemonFakeBall_.end());
        staticPokemonSupport_ = true;
    } else {
        staticPokemonSupport_ = false;
    }
    if (copyRoamingPokemon_) roamingPokemon_.insert(roamingPokemon_.end(), o->roamingPokemon_.begin(), o->roamingPokemon_.end());
    if (copyTradeScripts_) tradeScripts_.insert(tradeScripts_.end(), o->tradeScripts_.begin(), o->tradeScripts_.end());
    if (copyStoryText_)
        for (const auto& [k, v] : o->storyTexts_) storyTexts_[k] = v;
}

struct Gen5ReaderSetup {
    static std::vector<std::unique_ptr<Gen5RomEntry>> readAll() {
        using E = Gen5RomEntry;
        using R = IniEntryReader<E>;
        R reader(R::DefaultReadMode::STRING, R::CopyFromMode::ROMCODE, [](const std::string& n) { return std::make_unique<E>(n); });
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
            e.files_[key] = Gen5RomEntry::FileEntry{jc::trim(values.at(0)), ini::parseLong("0x" + jc::trim(values.at(1)))};
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
        // Gen5RomEntryReader
        reader.putSpecialKeyMethod("IsBlack", [](E& e, const std::string& v) { e.black_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("CopyTradeScripts", [](E& e, const std::string& v) { e.copyTradeScripts_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("CopyStoryText", [](E& e, const std::string& v) { e.copyStoryText_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("StaticPokemonFakeBall{}", [](E& e, const std::string& v) {
            e.staticPokemonFakeBall_.push_back(parseStaticPokemon(v));
        });
        reader.putSpecialKeyMethod("RoamingPokemon{}", [](E& e, const std::string& v) { e.roamingPokemon_.push_back(parseRoamingPokemon(v)); });
        reader.putSpecialKeyMethod("TradeScript[]", [](E& e, const std::string& v) {
            TradeScript ts;
            for (const auto& off : bracketList(v)) {
                auto parts = jc::split(off, ":");
                ts.fileNum = ini::parseInt(parts.at(0));
                ts.requestedOffsets.push_back(ini::parseInt(parts.at(1)));
                ts.givenOffsets.push_back(ini::parseInt(parts.at(2)));
            }
            e.tradeScripts_.push_back(std::move(ts));
        });
        reader.putKeyPrefixMethod("StoryText<", [](E& e, const std::string& k, const std::string& v) {
            e.storyTexts_[jc::split(jc::split(k, "<")[1], ">")[0]] = v;
        });
        reader.putKeyPrefixMethod("StarterOffsets", [](E& e, const std::string& k, const std::string& v) {
            std::vector<InFileEntry> offs;
            for (const auto& off : bracketList(v)) offs.push_back(parseInFile(off));
            e.offsetArrayEntries_[k] = std::move(offs);
        });

        auto text = embeddedFile("romentries/gen5_offsets.ini");
        if (!text) throw std::runtime_error("Falta romentries/gen5_offsets.ini");
        return reader.read(*text, "gen5_offsets.ini");
    }
};

const std::vector<std::unique_ptr<Gen5RomEntry>>& Gen5RomEntry::all() {
    static const auto entries = Gen5ReaderSetup::readAll();
    return entries;
}

}  // namespace pr::gen5
