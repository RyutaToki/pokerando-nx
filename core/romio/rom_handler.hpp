// Base común de los manejadores de ROM: port de la interfaz RomHandler y de
// AbstractRomHandler de FVX. Cada generación implementa las partes virtuales.
#pragma once
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/encounter.hpp"
#include "gamedata/evolution.hpp"
#include "gamedata/move.hpp"
#include "gamedata/species.hpp"
#include "gamedata/trainer.hpp"
#include "gamedata/shop_trade.hpp"
#include "gamedata/static_encounter.hpp"
#include "gamedata/type_table.hpp"
#include "gamedata/types.hpp"
#include "romio/misc_tweak.hpp"
#include "util/java_hash.hpp"

namespace pr {

// Set<Item> de Java (HashSet; Item.hashCode() == id): ids en orden de iteración de Java
using ItemSet = JavaHashSet<int>;

class RomHandler {
public:
    virtual ~RomHandler() = default;

    // --- información de la ROM ---
    virtual std::string romName() const = 0;
    virtual const std::string& romCode() const = 0;
    virtual int romType() const = 0;
    virtual int generationOfPokemon() const = 0;
    virtual bool isRomValid() const = 0;

    // --- Pokémon ---
    // lista con posición 0 vacía (nullptr), como getSpecies() en Java
    virtual const std::vector<Species*>& getSpecies() const = 0;
    virtual const std::vector<Species*>& getSpeciesInclFormes() const = 0;
    // SpeciesSet.unmodifiable(getSpecies()): una copia nueva en cada llamada, como en Java
    SpeciesSet getSpeciesSet() const { return SpeciesSet::unmodifiable(getSpecies()); }
    SpeciesSet getSpeciesSetInclFormes() const { return SpeciesSet::unmodifiable(getSpeciesInclFormes()); }

    // --- movimientos ---
    virtual const std::vector<std::optional<Move>>& getMoves() const = 0;
    int getPerfectAccuracy() const { return perfectAccuracy_; }
    virtual std::map<int, std::vector<MoveLearnt>> getMovesLearnt() const = 0;
    virtual void setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) = 0;
    std::array<int, 4> getMovesAtLevel(const Species& pk, const std::map<int, std::vector<MoveLearnt>>& movesets,
                                       int level) const;

    // --- interfaz específica de cada generación (RomHandler.java) ---
    virtual std::map<int, std::vector<int>> getEggMoves() const = 0;
    virtual void setEggMoves(const std::map<int, std::vector<int>>& eggMoves) = 0;
    virtual std::vector<EncounterArea> getEncounters(bool useTimeOfDay) const = 0;
    virtual std::vector<EncounterArea> getSortedEncounters(bool useTimeOfDay) const = 0;
    virtual void setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) = 0;
    virtual SpeciesSet getBannedForWildEncounters() const = 0;
    virtual bool hasEncounterLocations() const = 0;
    virtual std::vector<int> getEliteFourTrainers(bool isChallengeMode) const = 0;
    virtual std::vector<std::pair<std::string, Type>> getGymAndEliteTypeThemes() const = 0;
    virtual void saveTrainerClasses() = 0;
    virtual void saveTrainers() = 0;
    virtual std::vector<Species*> getStarters() const = 0;
    virtual bool setStarters(const std::vector<Species*>& starters) = 0;
    virtual int starterCount() const = 0;
    virtual bool supportsStarterHeldItems() const = 0;
    virtual std::vector<std::optional<int>> getStarterHeldItems() const = 0;
    virtual void setStarterHeldItems(const std::vector<std::optional<int>>& items) = 0;
    virtual std::vector<StaticEncounter> getStaticPokemon() = 0;
    virtual bool setStaticPokemon(const std::vector<StaticEncounter>& statics) = 0;
    virtual bool canChangeStaticPokemon() const = 0;
    virtual std::vector<int> getTMMoves() const = 0;
    virtual std::vector<int> getHMMoves() const = 0;
    virtual void setTMMoves(const std::vector<int>& moveIndexes) = 0;
    virtual int getTMCount() const = 0;
    virtual int getHMCount() const = 0;
    virtual std::map<int, std::vector<bool>> getTMHMCompatibility() const = 0;
    virtual void setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) = 0;
    virtual bool hasMoveTutors() const = 0;
    virtual std::vector<int> getMoveTutorMoves() const = 0;
    virtual void setMoveTutorMoves(const std::vector<int>& moveIndexes) = 0;
    virtual std::map<int, std::vector<bool>> getMoveTutorCompatibility() const = 0;
    virtual void setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) = 0;
    virtual ItemSet getEvolutionItems() const = 0;
    virtual ItemSet getOPShopItems() const = 0;
    virtual ItemSet getAllConsumableHeldItems() const = 0;
    virtual ItemSet getRequiredFieldTMs() const = 0;
    virtual std::vector<int> getFieldItems() const = 0;
    virtual void setFieldItems(const std::vector<int>& fieldItems) = 0;
    virtual bool hasShopSupport() const = 0;
    virtual bool canChangeShopSizes() const = 0;
    virtual std::vector<Shop> getShops() const = 0;
    virtual void setShops(const std::vector<Shop>& shops) = 0;
    virtual std::vector<int> getShopPrices() const = 0;
    virtual void setShopPrices(const std::vector<int>& prices) = 0;
    virtual void setBalancedShopPrices() = 0;
    virtual std::vector<PickupItem> getPickupItems() = 0;
    virtual void setPickupItems(const std::vector<PickupItem>& items) = 0;
    virtual std::vector<InGameTrade> getInGameTrades() const = 0;
    virtual void setInGameTrades(const std::vector<InGameTrade>& trades) = 0;
    virtual std::vector<std::optional<int>> getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly, const std::array<int, 4>& pokeMoves) const = 0;
    virtual int maxTrainerNameLength() const = 0;
    virtual int maxTrainerClassNameLength() const = 0;
    virtual std::vector<int> getDoublesTrainerClasses() const = 0;
    virtual std::vector<int> getMainGameLegendaries() const = 0;
    virtual std::vector<int> getSpecialMusicStatics() const = 0;
    virtual bool hasStaticMusicFix() const = 0;
    virtual int abilitiesPerSpecies() const = 0;
    virtual int highestAbilityIndex() const = 0;
    virtual std::vector<int> getUselessAbilities() const = 0;
    virtual bool isTrainerPokemonAlwaysUseAbility1() const = 0;
    virtual int internalStringLength(const std::string& s) const = 0;
    virtual int getMaxMoveNameLength() const = 0;
    virtual bool isEnglish() const = 0;
    virtual TypeTable getTypeTable() const = 0;
    virtual void setTypeTable(const TypeTable& t) = 0;
    virtual bool hasTypeEffectivenessSupport() const = 0;
    virtual void removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) = 0;
    virtual void makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) = 0;
    virtual void removeEvosForPokemonPool(const SpeciesSet& included) = 0;
    virtual int miscTweaksAvailable() const = 0;
    virtual void applyMiscTweak(const MiscTweak& t) = 0;
    virtual bool setIntroPokemon(Species* pk) = 0;
    virtual bool setCatchingTutorial(Species* opponent, Species* player) = 0;
    virtual void setPCPotionItem(int item) = 0;
    virtual void enableGuaranteedPokemonCatching() = 0;
    virtual void applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) = 0;
    virtual ItemSet getAllHeldItems() const = 0;
    virtual std::vector<int> getFieldMoves() const = 0;
    virtual std::vector<int> getEarlyRequiredHMMoves() const = 0;
    virtual bool isTMsReusable() const = 0;
    virtual bool canTMsBeHeld() const = 0;
    virtual bool supportsFourStartingMoves() const = 0;
    virtual bool hasDVs() const = 0;

    // objetos por id estándar (posiciones vacías = nullopt) y habilidades
    virtual const std::vector<std::optional<Item>>& getItems() const = 0;
    virtual std::vector<std::optional<Item>>& getItemsMutable() = 0;
    virtual const std::string& abilityName(int n) const = 0;
    // Map<Integer, List<Integer>> en orden de iteración de Java
    virtual std::vector<std::pair<int, std::vector<int>>> getAbilityVariations() const = 0;

    // --- valores por defecto de AbstractRomHandler / AbstractGBRomHandler ---
    virtual bool canSetIntroPokemon() const { return true; }
    virtual bool hasStarterAltFormes() const { return false; }
    virtual bool canGiveCustomMovesetsToBossTrainers() const { return true; }
    virtual bool canGiveCustomMovesetsToImportantTrainers() const { return true; }
    virtual bool canGiveCustomMovesetsToRegularTrainers() const { return true; }
    virtual bool canAddPokemonToBossTrainers() const { return true; }
    virtual bool canAddPokemonToImportantTrainers() const { return true; }
    virtual bool canAddPokemonToRegularTrainers() const { return true; }
    virtual bool canAddHeldItemsToBossTrainers() const { return true; }
    virtual bool canAddHeldItemsToImportantTrainers() const { return true; }
    virtual bool canAddHeldItemsToRegularTrainers() const { return true; }
    virtual bool hasTimeBasedEncounters() const { return false; }
    virtual bool hasPhysicalSpecialSplit() const { return false; }  // Gen 1-3
    virtual bool hasWildAltFormes() const { return false; }
    virtual bool hasTimeBasedEvolutions() const { return true; }
    virtual bool canGiveEverySpeciesOneEvolutionEach() const { return true; }
    virtual bool altFormesCanHaveDifferentEvolutions() const { return false; }
    virtual bool isTrainerPokemonUseBaseFormeAbilities() const { return false; }
    virtual bool hasGuaranteedWildHeldItems() const { return false; }  // AbstractGBRomHandler
    virtual bool hasDarkGrassHeldItems() const { return false; }
    virtual bool hasMegaEvolutions() const { return false; }
    virtual bool hasPokemonPaletteSupport() const { return true; }
    virtual bool isYellow() const { return false; }
    virtual bool isORAS() const { return false; }
    virtual bool isUSUM() const { return false; }
    virtual bool forceSwapStaticMegaEvos() const { return false; }
    virtual bool hasStaticAltFormes() const { return false; }
    virtual int maxTradeNicknameLength() const { return 10; }
    virtual int maxTradeOTNameLength() const { return 7; }
    virtual int maxSumOfTrainerNameLengths() const { return 0x7FFFFFFF; }
    virtual std::vector<ExpCurve> getExpCurves() const {
        return {ExpCurve::MEDIUM_FAST, ExpCurve::MEDIUM_SLOW, ExpCurve::FAST, ExpCurve::SLOW};  // GB
    }
    virtual std::vector<int> getGameBreakingMoves() const { return {49, 82}; }  // Sonicboom, Dragon Rage
    virtual std::vector<int> getIllegalMoves() const { return {}; }
    virtual std::vector<int> getMovesBannedFromLevelup() const { return {}; }
    virtual SpeciesSet getBannedForStaticPokemon() const { return SpeciesSet(); }
    virtual bool hasStarterTypeTriangleSupport() const { return starterCount() % 3 == 0; }

    // --- entrenadores ---
    const std::vector<std::unique_ptr<TrainerClass>>& getTrainerClasses() const { return trainerClasses_; }
    const std::vector<TrainerPtr>& getTrainers() const { return trainers_; }
    std::vector<TrainerPtr>& getTrainers() { return trainers_; }

    // --- evoluciones (AbstractRomHandler) ---
    int getHighestEvoLvl() const { return highestEvoLvl_; }
    void condenseLevelEvolutions(int maxLevel);
    void adjustEvolutionLevels();
    void removeTimeBasedEvolutions();
    // TreeMap<Species, List<Evolution>> ordenado por número
    const std::map<int, std::vector<Evolution>>& getPreImprovedEvolutions() const { return preImprovedEvolutions_; }

protected:
    void estimateEvolutionLevels();
    void markImprovedEvolutions(Species* pk);
    bool hadEvolutionOfType(Species* pk, std::optional<EvolutionType> et) const;

    int perfectAccuracy_ = 100;
    std::vector<std::unique_ptr<TrainerClass>> trainerClasses_;
    std::vector<TrainerPtr> trainers_;

private:
    void calculateEstimatedLevels(const std::vector<EvolutionPtr>& targetEvos);

    int highestEvoLvl_ = 0;
    std::optional<std::vector<std::array<int, 3>>> levelUpEstimationTriplets_;
    std::map<int, std::vector<Evolution>> preImprovedEvolutions_;
};

}  // namespace pr
