// Port de Gen5RomEntry (y AbstractDSRomEntry) de FVX: lee gen5_offsets.ini.
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "romio/gen4/gen4_rom_entry.hpp"
#include "romio/ini_entry.hpp"
#include "util/java_hash.hpp"

namespace pr::gen5 {

using gen4::DSStaticPokemon;
using gen4::InFileEntry;

struct RoamingPokemon {
    std::vector<int> speciesOverlayOffsets, levelOverlayOffsets;
    std::vector<InFileEntry> speciesScriptOffsets;
};

struct TradeScript {
    int fileNum = 0;
    std::vector<int> requestedOffsets, givenOffsets;
};

class Gen5RomEntry : public RomEntry {
public:
    using RomEntry::RomEntry;
    static const std::vector<std::unique_ptr<Gen5RomEntry>>& all();

    void setRomTypeFromString(const std::string& s) override;
    void copyFrom(const IniEntry& other) override;

    // AbstractDSRomEntry
    int64_t arm9ExpectedCRC32() const { return arm9ExpectedCRC32_; }
    bool hasStaticPokemonSupport() const { return staticPokemonSupport_; }
    const std::string& getFile(const std::string& key) const;
    bool hasFile(const std::string& key) const { return files_.count(key) > 0; }
    int64_t getFileExpectedCRC32(const std::string& key) const;
    std::vector<std::string> getFileKeys() const;  // orden del HashMap<String, ...> de Java
    const JavaHashMap<int, int64_t>& overlayExpectedCRC32s() const { return overlayCRC32s_; }
    const std::vector<DSStaticPokemon>& getStaticPokemon() const { return staticPokemon_; }

    // Gen5RomEntry
    bool isBlack() const { return black_; }
    const std::vector<DSStaticPokemon>& getStaticPokemonFakeBall() const { return staticPokemonFakeBall_; }
    const std::vector<RoamingPokemon>& getRoamingPokemon() const { return roamingPokemon_; }
    const std::vector<TradeScript>& getTradeScripts() const { return tradeScripts_; }
    const std::vector<InFileEntry>* getOffsetArrayEntry(const std::string& key) const;
    std::string getStoryText(const std::string& key) const;

private:
    friend struct Gen5ReaderSetup;
    struct FileEntry {
        std::string path;
        int64_t crc = 0;
    };
    int64_t arm9ExpectedCRC32_ = 0;
    bool staticPokemonSupport_ = false, copyStaticPokemon_ = false, copyRoamingPokemon_ = false;
    std::map<std::string, FileEntry> files_;
    JavaHashMap<int, int64_t> overlayCRC32s_;
    std::vector<DSStaticPokemon> staticPokemon_;
    bool copyTradeScripts_ = false, copyStoryText_ = false, black_ = false;
    std::vector<DSStaticPokemon> staticPokemonFakeBall_;
    std::vector<RoamingPokemon> roamingPokemon_;
    std::vector<TradeScript> tradeScripts_;
    std::map<std::string, std::vector<InFileEntry>> offsetArrayEntries_;
    std::map<std::string, std::string> storyTexts_;
};

}  // namespace pr::gen5
