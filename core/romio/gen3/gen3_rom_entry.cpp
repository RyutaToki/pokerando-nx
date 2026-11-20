#include "romio/gen3/gen3_rom_entry.hpp"

#include <regex>
#include <stdexcept>

#include "romio/gen3/gen3_constants.hpp"
#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::gen3 {

void Gen3RomEntry::setRomTypeFromString(const std::string& s) {
    if (jc::equalsIgnoreCase(s, "Ruby")) setRomType(RomType_Ruby);
    else if (jc::equalsIgnoreCase(s, "Sapp")) setRomType(RomType_Sapp);
    else if (jc::equalsIgnoreCase(s, "Em")) setRomType(RomType_Em);
    else if (jc::equalsIgnoreCase(s, "FRLG")) setRomType(RomType_FRLG);
    else std::fprintf(stderr, "unrecognised rom type: %s\n", s.c_str());
}

void Gen3RomEntry::copyFrom(const IniEntry& other) {
    IniEntry::copyFrom(other);
    auto* o = dynamic_cast<const Gen3RomEntry*>(&other);
    if (!o) return;
    // AbstractGBRomEntry.copyFrom
    for (auto& [k, v] : o->unusedChunks_) unusedChunks_[k] = v;
    // Gen3RomEntry.copyFrom
    if (copyStaticPokemon_) {
        staticPokemon_.insert(staticPokemon_.end(), o->staticPokemon_.begin(), o->staticPokemon_.end());
        roamingPokemon_.insert(roamingPokemon_.end(), o->roamingPokemon_.begin(), o->roamingPokemon_.end());
        putInt("StaticPokemonSupport", 1);
    } else {
        putInt("StaticPokemonSupport", 0);
    }
    if (getInt("CopyStarterText") == 1) {
        starterTexts_.insert(starterTexts_.end(), o->starterTexts_.begin(), o->starterTexts_.end());
    }
    if (getInt("CopyTMText") == 1) {
        tmTexts_.insert(tmTexts_.end(), o->tmTexts_.begin(), o->tmTexts_.end());
        moveTutorTexts_.insert(moveTutorTexts_.end(), o->moveTutorTexts_.begin(), o->moveTutorTexts_.end());
    }
    tableFile_ = o->tableFile_;
}

namespace {

StaticPokemonOffsets parseStaticPokemon(const std::string& s) {
    StaticPokemonOffsets sp;
    static const std::regex re(R"([A-z]+=\[(0x[0-9a-fA-F]+,?\s?)+\])");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        auto segments = jc::split(it->str(), "=");
        auto romOffsets = jc::split(segments[1].substr(1, segments[1].size() - 2), ",");
        std::vector<int> offsets;
        for (auto& o : romOffsets) offsets.push_back(ini::parseInt(o));
        if (segments[0] == "Species") sp.speciesOffsets = offsets;
        else if (segments[0] == "Level") sp.levelOffsets = offsets;
    }
    return sp;
}

std::vector<int> parseOffsetInScript(const std::string& s) {
    if (jc::startsWith(s, "[") && jc::endsWith(s, "]")) {
        std::vector<int> r;
        for (auto& p : jc::split(s.substr(1, s.size() - 2), ";")) r.push_back(ini::parseInt(p));
        return r;
    }
    return {ini::parseInt(s)};
}

bool parseEventTextEntry(const std::string& s, EventTextEntry& out) {
    if (!(jc::startsWith(s, "[") && jc::endsWith(s, "]"))) return false;
    auto parts = jc::split(s.substr(1, s.size() - 2), ",", 6);
    out.id = ini::parseInt(parts.at(0));
    out.mapBank = ini::parseInt(parts.at(1));
    out.mapNumber = ini::parseInt(parts.at(2));
    out.personNum = ini::parseInt(parts.at(3));
    out.relativePointerOffsets = parseOffsetInScript(parts.at(4));
    out.templateText = parts.at(5);
    return true;
}

}  // namespace

struct Gen3ReaderSetup {
    static std::vector<std::unique_ptr<Gen3RomEntry>> readAll() {
        using R = IniEntryReader<Gen3RomEntry>;
        R reader(R::DefaultReadMode::INT, R::CopyFromMode::NAME,
                 [](const std::string& n) { return std::make_unique<Gen3RomEntry>(n); });
        // RomEntryReader
        reader.putSpecialKeyMethod("Game", [](Gen3RomEntry& e, const std::string& v) { e.setRomCode(v); });
        reader.putSpecialKeyMethod("Version", [](Gen3RomEntry& e, const std::string& v) { e.setVersion(ini::parseInt(v)); });
        reader.putSpecialKeyMethod("Type", [](Gen3RomEntry& e, const std::string& v) { e.setRomTypeFromString(v); });
        reader.putKeySuffixMethod("Tweak", [](Gen3RomEntry& e, const std::string& k, const std::string& v) { e.putTweakFile(k, v); });
        // GBRomEntryReader
        reader.putSpecialKeyMethod("CRC32", [](Gen3RomEntry& e, const std::string& v) { e.expectedCRC32_ = ini::parseLong("0x" + v); });
        reader.putKeyPrefixMethod("UnusedChunk<", [](Gen3RomEntry& e, const std::string& k, const std::string& v) {
            std::string key = jc::split(jc::split(k, "<")[1], ">")[0];
            if (v == "null") {
                e.unusedChunks_.erase(key);
                return;
            }
            if (!jc::startsWith(v, "[") || !jc::endsWith(v, "]")) throw std::invalid_argument("UnusedChunk sin corchetes: " + k);
            auto parts = jc::split(v.substr(1, v.size() - 2), ",");
            if (parts.size() != 2) throw std::invalid_argument("UnusedChunk necesita 2 valores: " + k);
            e.unusedChunks_[key] = UnusedChunk{ini::parseInt(parts[0]), ini::parseInt(parts[1])};
        });
        auto addString = [](Gen3RomEntry& e, const std::string& k, const std::string& v) { e.putString(k, v); };
        reader.putKeySuffixMethod("Locator", addString);
        reader.putKeySuffixMethod("Prefix", addString);
        reader.putKeySuffixMethod("String", addString);
        // Gen3RomEntryReader
        reader.putSpecialKeyMethod("TableFile", [](Gen3RomEntry& e, const std::string& v) { e.tableFile_ = v; });
        reader.putSpecialKeyMethod("CopyStaticPokemon", [](Gen3RomEntry& e, const std::string& v) { e.copyStaticPokemon_ = ini::parseBoolean(v); });
        reader.putSpecialKeyMethod("StaticPokemon{}", [](Gen3RomEntry& e, const std::string& v) { e.staticPokemon_.push_back(parseStaticPokemon(v)); });
        reader.putSpecialKeyMethod("RoamingPokemon{}", [](Gen3RomEntry& e, const std::string& v) { e.roamingPokemon_.push_back(parseStaticPokemon(v)); });
        auto eventText = [](std::vector<EventTextEntry> Gen3RomEntry::*list) {
            return [list](Gen3RomEntry& e, const std::string& v) {
                EventTextEntry ete;
                if (parseEventTextEntry(v, ete)) (e.*list).push_back(ete);
            };
        };
        reader.putSpecialKeyMethod("StarterText[]", eventText(&Gen3RomEntry::starterTexts_));
        reader.putSpecialKeyMethod("TMText[]", eventText(&Gen3RomEntry::tmTexts_));
        reader.putSpecialKeyMethod("MoveTutorText[]", eventText(&Gen3RomEntry::moveTutorTexts_));
        reader.putSpecialKeyMethod("TMMovesReusableFunctionOffset", [](Gen3RomEntry& e, const std::string& v) {
            e.putArray("TMMovesReusableFunctionOffsets", {ini::parseInt(v)});
        });

        auto text = embeddedFile("romentries/gen3_offsets.ini");
        if (!text) throw std::runtime_error("Falta romentries/gen3_offsets.ini");
        return reader.read(*text, "gen3_offsets.ini");
    }
};

const std::vector<std::unique_ptr<Gen3RomEntry>>& Gen3RomEntry::all() {
    static const auto entries = Gen3ReaderSetup::readAll();
    return entries;
}

}  // namespace pr::gen3
