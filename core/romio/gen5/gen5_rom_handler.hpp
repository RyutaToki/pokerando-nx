// Port de Gen5RomHandler (y la parte de AbstractDSRomHandler que usa) de FVX:
// Negro, Blanco, Negro 2 y Blanco 2.
#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "nds/narc.hpp"
#include "nds/nds_rom.hpp"
#include "romio/gb/freed_space.hpp"
#include "romio/gen5/gen5_rom_entry.hpp"
#include "romio/rom_handler.hpp"

namespace pr::gen5 {

using Bytes = std::vector<uint8_t>;

class Gen5RomHandler : public RomHandler {
public:
    // ¿Es una ROM de Gen 5 que conocemos? (código del juego y versión de la cabecera)
    static bool isLoadable(const std::string& ndsCode, int version);

    // Carga completa; lanza std::runtime_error si no se reconoce.
    void loadRom(const std::string& fileName);
    void saveRomTo(const std::string& fileName) override;

    const Gen5RomEntry& romEntry() const { return *romEntry_; }
    std::string romName() const override { return "Pokemon " + romEntry_->name(); }
    const std::string& romCode() const override { return romEntry_->romCode(); }
    int romType() const override { return romEntry_->romType(); }
    int generationOfPokemon() const override { return 5; }
    bool isRomValid() const override;

    // --- Pokémon, movimientos, objetos ---
    const std::vector<Species*>& getSpecies() const override { return species_; }
    const std::vector<Species*>& getSpeciesInclFormes() const override { return speciesInclFormes_; }
    SpeciesSet getAltFormes() const override;
    SpeciesSet getIrregularFormes() const override;
    bool hasFunctionalFormes() const override { return true; }
    const std::vector<std::optional<Move>>& getMoves() const override { return moves_; }
    const std::vector<std::optional<Item>>& getItems() const override { return items_; }
    std::vector<std::optional<Item>>& getItemsMutable() override { return items_; }
    const std::string& abilityName(int n) const override { return abilityNames_.at(n); }
    std::vector<std::pair<int, std::vector<int>>> getAbilityVariations() const override;

    std::map<int, std::vector<MoveLearnt>> getMovesLearnt() const override;
    void setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) override;
    std::map<int, std::vector<int>> getEggMoves() const override;
    void setEggMoves(const std::map<int, std::vector<int>>& eggMoves) override;

    // --- encuentros ---
    std::vector<EncounterArea> getEncounters(bool useTimeOfDay) const override;
    std::vector<EncounterArea> getSortedEncounters(bool useTimeOfDay) const override;
    void setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) override;
    SpeciesSet getBannedForWildEncounters() const override;
    SpeciesSet getBannedFormesForTrainerPokemon() const override;
    bool hasEncounterLocations() const override { return true; }
    bool hasTimeBasedEncounters() const override { return true; }  // estaciones
    bool hasWildAltFormes() const override { return true; }
    bool hasDarkGrassHeldItems() const override { return true; }
    bool hasPhysicalSpecialSplit() const override { return true; }
    bool hasGuaranteedWildHeldItems() const override { return true; }  // AbstractRomHandler
    bool isTrainerPokemonUseBaseFormeAbilities() const override { return true; }
    bool hasStarterAltFormes() const override { return false; }

    // --- entrenadores ---
    std::vector<int> getMainPlaythroughTrainers() const override;
    std::vector<int> getEliteFourTrainers(bool isChallengeMode) const override;
    std::vector<std::pair<std::string, Type>> getGymAndEliteTypeThemes() const override;
    void saveTrainerClasses() override;
    void saveTrainers() override;
    void makeDoubleBattleModePossible() override;

    // --- iniciales y estáticos ---
    std::vector<Species*> getStarters() const override;
    bool setStarters(const std::vector<Species*>& starters) override;
    int starterCount() const override { return 3; }
    bool supportsStarterHeldItems() const override { return false; }
    std::vector<std::optional<int>> getStarterHeldItems() const override { return {}; }
    void setStarterHeldItems(const std::vector<std::optional<int>>&) override {}
    std::vector<StaticEncounter> getStaticPokemon() override;
    bool setStaticPokemon(const std::vector<StaticEncounter>& statics) override;
    bool canChangeStaticPokemon() const override { return romEntry_->hasStaticPokemonSupport(); }

    // --- MT / tutores ---
    std::vector<int> getTMMoves() const override;
    std::vector<int> getHMMoves() const override;
    void setTMMoves(const std::vector<int>& moveIndexes) override;
    int getTMCount() const override;
    int getHMCount() const override;
    std::map<int, std::vector<bool>> getTMHMCompatibility() const override;
    void setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) override;
    bool hasMoveTutors() const override;
    std::vector<int> getMoveTutorMoves() const override;
    void setMoveTutorMoves(const std::vector<int>& moveIndexes) override;
    std::map<int, std::vector<bool>> getMoveTutorCompatibility() const override;
    void setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) override;
    bool isTMsReusable() const override { return true; }
    bool canTMsBeHeld() const override { return true; }

    // --- objetos, tiendas, intercambios ---
    ItemSet getEvolutionItems() const override;
    ItemSet getOPShopItems() const override;
    ItemSet getAllConsumableHeldItems() const override;
    ItemSet getRequiredFieldTMs() const override;
    std::vector<int> getFieldItems() const override;
    void setFieldItems(const std::vector<int>& fieldItems) override;
    bool hasShopSupport() const override { return true; }
    bool canChangeShopSizes() const override { return true; }
    std::vector<Shop> getShops() const override;
    void setShops(const std::vector<Shop>& shops) override;
    std::vector<int> getShopPrices() const override;
    void setShopPrices(const std::vector<int>& prices) override;
    void setBalancedShopPrices() override;
    std::vector<PickupItem> getPickupItems() override;
    void setPickupItems(const std::vector<PickupItem>& items) override;
    std::vector<InGameTrade> getInGameTrades() const override;
    void setInGameTrades(const std::vector<InGameTrade>& trades) override;
    std::vector<std::optional<int>> getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                            const std::array<int, 4>& pokeMoves) const override;
    ItemSet getAllHeldItems() const override;

    // --- varios ---
    int maxTrainerNameLength() const override { return 10; }
    int maxTrainerClassNameLength() const override { return 12; }
    std::vector<int> getDoublesTrainerClasses() const override;
    std::vector<int> getMainGameLegendaries() const override;
    std::vector<int> getSpecialMusicStatics() const override;
    bool hasStaticMusicFix() const override;
    int abilitiesPerSpecies() const override { return 3; }
    int highestAbilityIndex() const override;
    std::vector<int> getUselessAbilities() const override;
    bool isTrainerPokemonAlwaysUseAbility1() const override { return false; }
    int internalStringLength(const std::string& s) const override;
    int getMaxMoveNameLength() const override { return 15; }
    bool isEnglish() const override;
    TypeTable getTypeTable() const override;
    void setTypeTable(const TypeTable& t) override;
    bool hasTypeEffectivenessSupport() const override { return true; }
    void removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) override;
    void makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) override;
    void removeEvosForPokemonPool(const SpeciesSet& included) override;
    int miscTweaksAvailable() const override;
    void applyMiscTweak(const MiscTweak& t) override;
    bool setIntroPokemon(Species* pk) override;
    bool setCatchingTutorial(Species* opponent, Species* player) override;
    void setPCPotionItem(int item) override;
    void enableGuaranteedPokemonCatching() override;
    void applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) override;
    std::vector<int> getFieldMoves() const override;
    std::vector<int> getEarlyRequiredHMMoves() const override;
    bool supportsFourStartingMoves() const override { return true; }
    bool hasDVs() const override { return false; }
    std::vector<ExpCurve> getExpCurves() const override;
    std::string getPaletteFilesID() const override;

    // --- textos (a/0/0/2 y a/0/0/3) ---
    std::vector<std::string> getStrings(bool isStoryText, int index) const;
    void setStrings(bool isStoryText, int index, const std::vector<std::string>& strings);

    int perfectAccuracy() const { return perfectAccuracy_; }

private:
    struct FormeInfo {
        int baseForme = 0, formeNumber = 0;
    };

    // AbstractDSRomHandler
    Bytes readFile(const std::string& path) const;
    void writeFile(const std::string& path, const Bytes& data);
    nds::NARCArchive readNARC(const std::string& path) const { return nds::NARCArchive(readFile(path)); }
    void writeNARC(const std::string& path, const nds::NARCArchive& narc) { writeFile(path, narc.getBytes()); }
    Bytes readOverlay(int number) const;
    void writeOverlay(int number, const Bytes& data);
    int find(const Bytes& data, const std::string& hex) const;
    int find(const Bytes& data, std::string_view hex) const { return find(data, std::string(hex)); }
    void extendARM9(int extendBy, int repointExtendBy, const Bytes& prefix);
    bool genericIPSPatch(Bytes& data, const std::string& ctName);

    void loadedROM();
    void computeCRC32sForRom();
    void loadItems();
    void loadSpeciesStats();
    void loadBasicPokeStats(Species& pk, const Bytes& stats);
    std::vector<std::string> readPokemonNames() const;
    int generationOf(const Species& pk) const;
    void populateEvolutions();
    void writeEvolutions();
    void writeShedinjaEvolution();
    void loadMoves();
    void prepareSaveRom();
    void saveMoves();
    void saveSpeciesStats();
    void saveBasicPokeStats(const Species& pk, Bytes& stats);
    void loadPokemonPalettes();
    void savePokemonPalettes();
    int graphicsIndex(const Species& pk) const;
    void loadTrainerClasses();
    void loadTrainers();
    std::vector<std::string> loadTrainerNames() const;
    void saveTrainerNames();
    void removeFreeLuckyEgg();
    // objetos
    int itcmSrcOffset() const;
    bool isARM9Pointer(const Bytes& data, int offset) const;
    int readARM9Pointer(const Bytes& data, int offset) const;
    void writeARM9Pointer(Bytes& data, int offset, int pointer) const;
    std::vector<int> getFieldItemIds() const;
    void setFieldItemIds(const std::vector<int>& fieldItems);
    void replaceAllStringsInEntry(int entry, const std::map<std::string, std::string>& replacements);
    void replaceStarterFiles(nds::NARCArchive& starterNARC, const nds::NARCArchive& pokespritesNARC, int starterIndex, int pokeNumber);
    // encuentros
    using AreaIter = std::vector<EncounterArea>::const_iterator;
    void loadWildMapNames() const;
    void processEncounterEntry(std::vector<EncounterArea>& areas, const Bytes& entry, int startOffset, int idx) const;
    void writeEncounterEntry(AreaIter& it, AreaIter end, Bytes& entry, int startOffset);
    void updatePokedexAreaData(const nds::NARCArchive& encounterNARC);
    void updateAreaDataFromEncounterEntry(const Bytes& entry, int startOffset, std::vector<Bytes>& areaData, int season,
                                          int fileNumber) const;

    const Gen5RomEntry* romEntry_ = nullptr;
    std::unique_ptr<nds::NDSRom> baseRom_;
    std::string loadedFN_;
    Bytes arm9_;
    bool arm9Extended_ = false;
    FreedSpace arm9FreedSpace_;
    int tcmCopyingPointersOffset_ = -1;

    std::vector<std::unique_ptr<Species>> speciesStorage_;
    std::vector<Species*> pokes_;  // incluye las formas (índice > pokemonCount)
    std::vector<Species*> species_, speciesInclFormes_;
    std::map<int, FormeInfo> formeMappings_;               // TreeMap: archivo de stats -> forma
    std::map<int, int> formeGraphicsIndices_;               // por número de especie
    std::map<int, int> graphicalFormeCounts_;               // TreeMap<Species, Integer> (orden por número)
    std::vector<std::optional<Move>> moves_;
    std::vector<std::optional<Item>> items_;
    std::vector<std::string> abilityNames_;
    std::vector<std::string_view> shopNames_;
    nds::NARCArchive pokeNarc_, moveNarc_, stringsNarc_, storyTextNarc_, scriptNarc_, shopNarc_;
    mutable bool loadedWildMapNames_ = false;
    mutable JavaHashMap<int, std::string> wildMapNames_;
    mutable int hiddenHollowCount_ = 0;
    mutable bool hiddenHollowCounted_ = false;
    std::vector<int> originalDoubleTrainers_;
    int pickupItemsTableOffset_ = 0;
    mutable std::optional<TypeTable> typeTable_;
    uint32_t actualArm9CRC32_ = 0;
    JavaHashMap<int, uint32_t> actualOverlayCRC32s_;
    std::map<std::string, uint32_t> actualFileCRC32s_;
};

}  // namespace pr::gen5
