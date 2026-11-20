// Port de Gen4RomHandler (y la parte de AbstractDSRomHandler que usa) de FVX:
// Diamante, Perla, Platino, Oro HeartGold y Plata SoulSilver.
#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "nds/narc.hpp"
#include "nds/nds_rom.hpp"
#include "romio/gb/freed_space.hpp"
#include "romio/gen4/gen4_rom_entry.hpp"
#include "romio/rom_handler.hpp"

namespace pr::gen4 {

using Bytes = std::vector<uint8_t>;

class Gen4RomHandler : public RomHandler {
public:
    // ¿Es una ROM de Gen 4 que conocemos? (código del juego y versión de la cabecera)
    static bool isLoadable(const std::string& ndsCode, int version);

    // Carga completa; lanza std::runtime_error si no se reconoce.
    void loadRom(const std::string& fileName);
    void saveRomTo(const std::string& fileName) override;

    const Gen4RomEntry& romEntry() const { return *romEntry_; }
    std::string romName() const override { return "Pokemon " + romEntry_->name(); }
    const std::string& romCode() const override { return romEntry_->romCode(); }
    int romType() const override { return romEntry_->romType(); }
    int generationOfPokemon() const override { return 4; }
    bool isRomValid() const override;

    // --- Pokémon, movimientos, objetos ---
    const std::vector<Species*>& getSpecies() const override { return species_; }
    const std::vector<Species*>& getSpeciesInclFormes() const override { return speciesInclFormes_; }
    SpeciesSet getAltFormes() const override;
    SpeciesSet getIrregularFormes() const override;
    bool hasFunctionalFormes() const override { return romEntry_->romType() != 0; }  // no en DP
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

    // --- entrenadores ---
    std::vector<int> getEliteFourTrainers(bool isChallengeMode) const override;
    std::vector<std::pair<std::string, Type>> getGymAndEliteTypeThemes() const override;
    void saveTrainerClasses() override;
    void saveTrainers() override;

    // --- iniciales y estáticos ---
    std::vector<Species*> getStarters() const override;
    bool setStarters(const std::vector<Species*>& starters) override;
    int starterCount() const override { return 3; }
    bool supportsStarterHeldItems() const override;
    std::vector<std::optional<int>> getStarterHeldItems() const override;
    void setStarterHeldItems(const std::vector<std::optional<int>>& items) override;
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
    bool isTMsReusable() const override { return tmsReusable_; }
    bool canTMsBeHeld() const override { return true; }

    // --- objetos, tiendas, intercambios ---
    ItemSet getEvolutionItems() const override;
    ItemSet getOPShopItems() const override;
    ItemSet getAllConsumableHeldItems() const override;
    ItemSet getRequiredFieldTMs() const override;
    std::vector<int> getFieldItems() const override;
    void setFieldItems(const std::vector<int>& fieldItems) override;
    bool hasShopSupport() const override;
    bool canChangeShopSizes() const override;
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
    int maxTrainerNameLength() const override;
    int maxTrainerClassNameLength() const override;
    std::vector<int> getDoublesTrainerClasses() const override;
    std::vector<int> getMainGameLegendaries() const override;
    std::vector<int> getSpecialMusicStatics() const override;
    bool hasStaticMusicFix() const override;
    int abilitiesPerSpecies() const override { return 2; }
    int highestAbilityIndex() const override;
    std::vector<int> getUselessAbilities() const override;
    bool isTrainerPokemonAlwaysUseAbility1() const override;
    int internalStringLength(const std::string& s) const override;
    int getMaxMoveNameLength() const override;
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

    // --- textos (msg.narc) ---
    std::vector<std::string> getStrings(int index) const;
    void setStrings(int index, const std::vector<std::string>& strings, bool compressed = false);

    int perfectAccuracy() const { return perfectAccuracy_; }

private:
    // AbstractDSRomHandler
    Bytes readFile(const std::string& path) const;
    void writeFile(const std::string& path, const Bytes& data);
    nds::NARCArchive readNARC(const std::string& path) const { return nds::NARCArchive(readFile(path)); }
    void writeNARC(const std::string& path, const nds::NARCArchive& narc) { writeFile(path, narc.getBytes()); }
    Bytes readOverlay(int number) const;
    void writeOverlay(int number, const Bytes& data);
    int find(const Bytes& data, const std::string& hex) const;
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
    void loadStatChangesFromEffect(Move& move, int secondaryEffectChance);
    void loadStatusFromEffect(Move& move, int secondaryEffectChance);
    void loadMiscMoveInfoFromEffect(Move& move, int secondaryEffectChance);
    void prepareSaveRom();
    void saveMoves();
    void saveSpeciesStats();
    void saveBasicPokeStats(const Species& pk, Bytes& stats);
    void loadPokemonPalettes();
    void loadTrainerClasses();
    void loadTrainers();
    std::vector<std::string> loadTrainerNames() const;
    void fixAbilitySlotValuesForHGSS();
    void saveTrainerNames();
    // encuentros
    void loadWildMapNames() const;
    void tagEncounterAreas(std::vector<EncounterArea>& areas, bool useTimeOfDay) const;
    void readMainEncountersDPPt(std::vector<EncounterArea>& areas, bool useTimeOfDay) const;
    void readExtraEncountersDPPt(std::vector<EncounterArea>& areas) const;
    void readMainEncountersHGSS(std::vector<EncounterArea>& areas, bool useTimeOfDay) const;
    void readHeadbuttEncounters(std::vector<EncounterArea>& areas) const;
    void readBugCatchingContestEncounters(std::vector<EncounterArea>& areas) const;
    void savePokemonPalettes();

    const Gen4RomEntry* romEntry_ = nullptr;
    std::unique_ptr<nds::NDSRom> baseRom_;
    std::string loadedFN_;
    Bytes arm9_;
    bool arm9Extended_ = false;
    FreedSpace arm9FreedSpace_;
    int tcmCopyingPointersOffset_ = -1;

    std::vector<std::unique_ptr<Species>> speciesStorage_;
    std::vector<Species*> pokes_;  // incluye las formas (índice > pokemonCount)
    std::vector<Species*> species_, speciesInclFormes_;
    std::vector<std::optional<Move>> moves_;
    std::vector<std::optional<Item>> items_;
    std::vector<std::string> abilityNames_;
    nds::NARCArchive pokeNarc_, moveNarc_, msgNarc_, scriptNarc_, eventNarc_;
    bool tmsReusable_ = false;
    bool roamerRandomizationEnabled_ = false;
    mutable bool lastStringsCompressed_ = false;
    // nombres de mapas para los encuentros (HashMap en Java; getEncounters los amplía)
    mutable JavaHashMap<int, std::string> wildMapNames_;
    mutable std::map<int, std::string> headbuttMapNames_;
    mutable bool loadedWildMapNames_ = false;
    mutable std::optional<TypeTable> typeTable_;
    uint32_t actualArm9CRC32_ = 0;
    JavaHashMap<int, uint32_t> actualOverlayCRC32s_;
    std::map<std::string, uint32_t> actualFileCRC32s_;
};

}  // namespace pr::gen4
