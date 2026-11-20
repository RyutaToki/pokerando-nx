// Port de Gen4RomEntry y AbstractDSRomEntry de FVX: lee gen4_offsets.ini.
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "romio/ini_entry.hpp"
#include "util/java_hash.hpp"

namespace pr::gen4 {

struct InFileEntry {
    int file = 0, offset = 0;
};

struct TextEntry {
    int textIndex = 0, stringNumber = 0;
};

// DSStaticPokemon y su subclase StaticPokemonGameCorner (con textos que nombran al Pokémon)
struct DSStaticPokemon {
    std::vector<InFileEntry> speciesEntries, formeEntries, levelEntries;
    bool gameCorner = false;
    std::vector<TextEntry> textEntries;  // solo en los del Casino
};

struct RoamingPokemon {
    std::vector<int> speciesCodeOffsets, levelCodeOffsets;
    std::vector<InFileEntry> speciesScriptOffsets, genderOffsets;
};

class Gen4RomEntry : public RomEntry {
public:
    using RomEntry::RomEntry;
    static const std::vector<std::unique_ptr<Gen4RomEntry>>& all();

    void setRomTypeFromString(const std::string& s) override;
    void copyFrom(const IniEntry& other) override;

    // AbstractDSRomEntry
    int64_t arm9ExpectedCRC32() const { return arm9ExpectedCRC32_; }
    bool hasStaticPokemonSupport() const { return staticPokemonSupport_; }
    bool isCopyStaticPokemon() const { return copyStaticPokemon_; }
    bool isCopyRoamingPokemon() const { return copyRoamingPokemon_; }
    const std::string& getFile(const std::string& key) const;
    bool hasFile(const std::string& key) const { return files_.count(key) > 0; }
    int64_t getFileExpectedCRC32(const std::string& key) const;
    // claves de archivos en el orden de iteración del HashMap<String, ...> de Java
    std::vector<std::string> getFileKeys() const;
    const JavaHashMap<int, int64_t>& overlayExpectedCRC32s() const { return overlayCRC32s_; }
    const std::vector<DSStaticPokemon>& getStaticPokemon() const { return staticPokemon_; }

    // Gen4RomEntry
    const std::vector<RoamingPokemon>& getRoamingPokemon() const { return roamingPokemon_; }
    const JavaHashMap<int, std::vector<TextEntry>>& getTMTexts() const { return tmTexts_; }
    const JavaHashMap<int, TextEntry>& getTmTextsGameCorner() const { return tmTextsGameCorner_; }
    const JavaHashMap<int, int>& getTMScriptOffsetsFrontier() const { return tmScriptOffsetsFrontier_; }
    const JavaHashMap<int, int>& getTMTextsFrontier() const { return tmTextsFrontier_; }
    const std::vector<InFileEntry>& getMarillCryScriptEntries() const { return marillCryScriptEntries_; }

private:
    friend struct Gen4ReaderSetup;
    struct FileEntry {
        std::string path;
        int64_t crc = 0;
    };
    int64_t arm9ExpectedCRC32_ = 0;
    bool staticPokemonSupport_ = false, copyStaticPokemon_ = false, copyRoamingPokemon_ = false;
    std::map<std::string, FileEntry> files_;
    JavaHashMap<int, int64_t> overlayCRC32s_;
    std::vector<DSStaticPokemon> staticPokemon_;
    bool ignoreGameCornerStatics_ = false, copyText_ = false;
    std::vector<RoamingPokemon> roamingPokemon_;
    JavaHashMap<int, std::vector<TextEntry>> tmTexts_;
    JavaHashMap<int, TextEntry> tmTextsGameCorner_;
    JavaHashMap<int, int> tmScriptOffsetsFrontier_, tmTextsFrontier_;
    std::vector<InFileEntry> marillCryScriptEntries_;
};

}  // namespace pr::gen4
