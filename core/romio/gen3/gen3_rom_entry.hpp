// Port de Gen3RomEntry (y AbstractGBRomEntry) de FVX.
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "romio/ini_entry.hpp"

namespace pr::gen3 {

struct StaticPokemonOffsets {
    std::vector<int> speciesOffsets;
    std::vector<int> levelOffsets;
};

struct EventTextEntry {
    int id = 0;
    int mapBank = 0, mapNumber = 0;
    int personNum = 0;
    std::vector<int> relativePointerOffsets;
    std::string templateText;
    int actualPointerOffset = -1;
};

struct UnusedChunk {
    int offset = 0;
    int length = 0;
};

class Gen3RomEntry : public RomEntry {
public:
    explicit Gen3RomEntry(std::string name) : RomEntry(std::move(name)) {}
    Gen3RomEntry(const Gen3RomEntry& o) = default;

    void setRomTypeFromString(const std::string& s) override;
    void copyFrom(const IniEntry& other) override;

    bool hasStaticPokemonSupport() const { return getInt("StaticPokemonSupport") > 0; }
    int64_t expectedCRC32() const { return expectedCRC32_; }
    const std::string& tableFile() const { return tableFile_; }
    const std::vector<StaticPokemonOffsets>& staticPokemon() const { return staticPokemon_; }
    const std::vector<StaticPokemonOffsets>& roamingPokemon() const { return roamingPokemon_; }
    std::vector<EventTextEntry>& starterTexts() { return starterTexts_; }
    std::vector<EventTextEntry>& tmTexts() { return tmTexts_; }
    std::vector<EventTextEntry>& moveTutorTexts() { return moveTutorTexts_; }
    const std::map<std::string, UnusedChunk>& unusedChunks() const { return unusedChunks_; }

    // lectura de todas las entradas de gen3_offsets.ini (incrustado)
    static const std::vector<std::unique_ptr<Gen3RomEntry>>& all();

private:
    friend struct Gen3ReaderSetup;

    int64_t expectedCRC32_ = -1;
    std::map<std::string, UnusedChunk> unusedChunks_;
    std::string tableFile_;
    bool copyStaticPokemon_ = false;
    std::vector<StaticPokemonOffsets> staticPokemon_;
    std::vector<StaticPokemonOffsets> roamingPokemon_;
    std::vector<EventTextEntry> starterTexts_;
    std::vector<EventTextEntry> tmTexts_;
    std::vector<EventTextEntry> moveTutorTexts_;
};

}  // namespace pr::gen3
