// Port de Gen3RomHandler (FVX). De momento: detección, configuración, texto,
// objetos, habilidades y datos base de los Pokémon. El resto se añade por fases.
#pragma once
#include <array>
#include <functional>
#include <map>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gamedata/encounter.hpp"
#include "gamedata/move.hpp"
#include "gamedata/species.hpp"
#include "gamedata/types.hpp"
#include "romio/gb/freed_space.hpp"
#include "romio/rom_handler.hpp"
#include "romio/gen3/gen3_rom_entry.hpp"

namespace pr::gen3 {

class Gen3RomHandler : public RomHandler {
public:
    // ¿Es una ROM de Gen 3 que conocemos? Solo mira la cabecera y el tamaño.
    static bool isLoadable(const std::vector<uint8_t>& headerBytes, size_t fileSize);

    // Carga completa; lanza std::runtime_error si no se reconoce.
    void loadRom(std::vector<uint8_t> romBytes, const std::string& fileName);
    // Igual que AbstractRomHandler.saveRom: escribe los datos cargados y guarda el archivo
    void saveRom(const std::string& fileName);
    void saveRomFileOnly(const std::string& fileName) const;

    const Gen3RomEntry& romEntry() const { return *romEntry_; }
    std::string romName() const override { return "Pokemon " + romEntry_->name(); }
    const std::string& romCode() const override { return romEntry_->romCode(); }
    int romType() const override { return romEntry_->romType(); }
    uint32_t actualCRC32() const { return actualCRC32_; }
    bool isRomValid() const override;
    bool isRomHack() const { return isRomHack_; }
    int generationOfPokemon() const override { return 3; }

    // lista "real" (posición 0 vacía), como getSpecies() en Java
    const std::vector<Species*>& getSpecies() const override { return speciesList_; }
    const std::vector<Species*>& getSpeciesInclFormes() const override { return speciesList_; }
    const std::vector<Species*>& species() const { return speciesList_; }
    // objetos indexados por id estándar (posiciones vacías = nullopt)
    const std::vector<std::optional<Item>>& items() const { return items_; }
    const std::string& abilityName(int n) const { return abilityNames_.at(n); }
    // movimientos por número (posición 0 vacía)
    const std::vector<std::optional<Move>>& moves() const { return moves_; }
    const std::vector<std::optional<Move>>& getMoves() const override { return moves_; }
    int perfectAccuracy() const { return perfectAccuracy_; }

    const std::vector<uint8_t>& rom() const { return rom_; }

    // Movimientos por nivel: número de Pokédex -> lista (TreeMap en Java)
    std::map<int, std::vector<MoveLearnt>> getMovesLearnt() const override;
    void setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) override;
    std::map<int, std::vector<int>> getEggMoves() const;
    void setEggMoves(const std::map<int, std::vector<int>>& eggMoves);
    int freeSpaceBytes() const { return freedSpace_.lengthSum(); }

    // Encuentros salvajes
    std::vector<EncounterArea> getEncounters(bool useTimeOfDay) const;
    std::vector<EncounterArea> getSortedEncounters(bool useTimeOfDay) const;
    void setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas);
    SpeciesSet getBannedForWildEncounters() const;
    bool hasEncounterLocations() const { return true; }

private:
    void initRomEntry();
    void initTextTables();
    void loadTextTable(const std::string& filename);
    void loadItems();
    void loadSpeciesStats();
    void loadPokemonNames();
    void loadPokedexOrder();
    void constructPokemonList();
    void loadBasicPokeStats(Species& pk, int offset);
    void loadAbilityNames();
    void loadMoves();
    void loadStatChangesFromEffect(Move& move, int secondaryEffectChance);
    void loadStatusFromEffect(Move& move, int secondaryEffectChance);
    void loadMiscMoveInfoFromEffect(Move& move, int secondaryEffectChance);
    int generationOf(const Species& pk) const;

    int readPointerOrInvalid(int offset) const;  // readPointer(offset, true)
    void determineMapBankSizes();
    void preprocessMaps();
    EncounterArea readEncounterArea(int offset, int numOfEntries, const std::string& name, EncounterType type) const;
    void writeEncounterArea(int offset, int numOfEntries, const EncounterArea& area);
    bool hasBattleTrappingAbility(const Species* s) const;
    void tagEncounterAreas(std::vector<EncounterArea>& areas) const;
    void attemptObedienceEvolutionPatches();
    void prepareSaveRom();
    void saveSpeciesStats();
    void saveBasicPokeStats(const Species& pk, int offset);
    void saveMoves();
    void writeFixedLengthString(const std::string& str, int offset, int length);
    void loadEvolutions();
    void writeEvolutions();
    std::vector<MoveLearnt> readMovesLearnt(int offset) const;
    std::vector<uint8_t> movesLearntToBytes(const std::vector<MoveLearnt>& ml) const;

    // escritura y espacio libre
    void writeByte(int offset, uint8_t v) { rom_.at(offset) = v; }
    void writeBytes(int offset, const std::vector<uint8_t>& values);
    void writeWord(int offset, int value);
    void writeLong(int offset, int value);
    void writePointer(int offset, int pointer);
    int searchForFirst(int beginOffset, const std::vector<uint8_t>& needle) const;
    void freeSpace(int offset, int length);
    bool isRomSpaceUsed(int offset, int length) const;
    int findAndUnfreeSpace(int length, bool longAligned = true);
    // DataRewriter.rewriteData: libera los datos viejos, busca hueco, escribe y reapunta
    int rewriteData(int pointerOffset, const std::vector<uint8_t>& newData, const std::function<int(int)>& lengthOfOld,
                    const std::vector<int>& secondaryPointerOffsets = {}, bool longAlign = true);
    void freeAllUnusedSpace();

    std::string readString(int offset, int maxLength) const;
    std::vector<uint8_t> translateString(const std::string& text) const;
    std::string readFixedLengthString(int offset, int length) const { return readString(offset, length); }
    std::string readVariableLengthString(int offset) const { return readString(offset, 0x7FFFFFFF); }

    int readWord(int offset) const { return rom_.at(offset) | (rom_.at(offset + 1) << 8); }
    int readLong(int offset) const;
    int readPointer(int offset) const;
    int find(std::string_view hexString) const;
    bool matches(int offset, const uint8_t* sig, size_t len) const;

    std::vector<uint8_t> rom_, originalRom_;
    std::string loadedFileName_;
    uint32_t actualCRC32_ = 0;
    std::unique_ptr<Gen3RomEntry> romEntry_;

    std::array<std::optional<std::u32string>, 256> tb_;
    std::unordered_map<std::u32string, uint8_t> d_;

    std::vector<std::unique_ptr<Species>> speciesStorage_;
    std::vector<Species*> pokes_, pokesInternal_, speciesList_;
    int numRealPokemon_ = 0;
    std::vector<std::optional<Item>> items_;
    std::vector<std::string> abilityNames_;
    std::vector<std::optional<Move>> moves_;
    std::vector<std::string> pokeNames_;
    std::vector<int> internalToPokedex_, pokedexToInternal_;
    int pokedexCount_ = 0;
    bool isRomHack_ = false;
    bool jamboMovesetHack_ = false;
    FreedSpace freedSpace_;
    bool havePatchedObedience_ = false;
    std::vector<int> itemOffs_;
    std::vector<std::vector<std::string>> mapNames_;
};

}  // namespace pr::gen3
