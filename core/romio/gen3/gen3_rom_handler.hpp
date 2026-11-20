// Port de Gen3RomHandler (FVX). De momento: detección, configuración, texto,
// objetos, habilidades y datos base de los Pokémon. El resto se añade por fases.
#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gamedata/move.hpp"
#include "gamedata/species.hpp"
#include "gamedata/types.hpp"
#include "romio/gen3/gen3_rom_entry.hpp"

namespace pr::gen3 {

class Gen3RomHandler {
public:
    // ¿Es una ROM de Gen 3 que conocemos? Solo mira la cabecera y el tamaño.
    static bool isLoadable(const std::vector<uint8_t>& headerBytes, size_t fileSize);

    // Carga completa; lanza std::runtime_error si no se reconoce.
    void loadRom(std::vector<uint8_t> romBytes, const std::string& fileName);
    void saveRom(const std::string& fileName) const;

    const Gen3RomEntry& romEntry() const { return *romEntry_; }
    std::string romName() const { return "Pokemon " + romEntry_->name(); }
    const std::string& romCode() const { return romEntry_->romCode(); }
    uint32_t actualCRC32() const { return actualCRC32_; }
    bool isRomValid() const;
    bool isRomHack() const { return isRomHack_; }
    int generationOfPokemon() const { return 3; }

    // lista "real" (posición 0 vacía), como getSpecies() en Java
    const std::vector<Species*>& species() const { return speciesList_; }
    // objetos indexados por id estándar (posiciones vacías = nullopt)
    const std::vector<std::optional<Item>>& items() const { return items_; }
    const std::string& abilityName(int n) const { return abilityNames_.at(n); }
    // movimientos por número (posición 0 vacía)
    const std::vector<std::optional<Move>>& moves() const { return moves_; }
    int perfectAccuracy() const { return perfectAccuracy_; }

    const std::vector<uint8_t>& rom() const { return rom_; }

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
    int perfectAccuracy_ = 100;  // valor por defecto de AbstractRomHandler
    std::vector<std::string> pokeNames_;
    std::vector<int> internalToPokedex_, pokedexToInternal_;
    int pokedexCount_ = 0;
    bool isRomHack_ = false;
};

}  // namespace pr::gen3
