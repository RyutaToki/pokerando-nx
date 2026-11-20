// Port de TrainerPokemonRandomizer.
#pragma once
#include <map>
#include <memory>
#include <set>

#include "random/randomizer.hpp"

namespace pr {

class TrainerPokemonRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void applyTrainerLevelModifier();
    void randomizeTrainerPokes();
    void makeRivalCarryStarter();
    void evolveTrainerPokemonAsFarAsLegal();
    void addTrainerPokemon();
    void modifyBattleStyle();
    void randomizeTrainerHeldItems();
    void randomUsableZCrystals();

private:
    using SetPtr = std::shared_ptr<SpeciesSet>;

    std::optional<Type> getTypeForTrainer(const Trainer& t, bool isTypeThemed, bool weightByFrequency, bool noLegendaries,
                                          bool includeFormes, bool keepTypeThemes, bool keepThemeOrPrimaryTypes);
    std::map<std::string, Type> pickGroupTypeThemes(bool useOriginal, const std::vector<std::string>& groupNames);
    Species* pickTrainerPokeReplacement(Species* current, int level, bool usePowerLevels, SpeciesSet& alreadyPlaced,
                                        bool noPremature, std::optional<Type> type, bool usePlacementHistory,
                                        bool swapMegaEvos, SetPtr useInsteadOfCached, bool evolveAsFarAsLegal,
                                        const std::set<Type>& bannedTypes, const SpeciesSet& bannedPokemon);
    Type pickType(bool weightByFrequency, bool noLegendaries, bool allowAltFormes);
    void initTypeWeightings(bool noLegendaries, bool allowAltFormes);
    int getRandomAbilitySlot(const Species& s);
    int getValidAbilitySlotFromOriginal(const Species& s, int original) const;
    int getLevelOfStarter(const std::string& tag) const;
    void changeStarterWithTag(const std::string& tag, const std::map<int, Species*>& byLevel, int abilitySlot);
    Species* fullyEvolve(Species* s);
    void randomizeCosmeticForme(TrainerPokemon& tp);
    void rivalCarriesStarterUpdate(const std::string& prefix, int offset);
    std::map<int, Species*> getEvolutionsByLevel(Species* base, int initialLevel, int maxLevel);
    Species* evolveAsFarAsLegal(Species* s, int level, double mod);
    void randomizeHeldItem(TrainerPokemon& tp, const std::array<int, 4>& moveset);
    double getPlacementAverage() const;

    std::map<Type, SetPtr> cachedByType_;
    SetPtr cachedAll_;
    SpeciesSet usedAsUnique_;
    std::map<Type, int> typeWeightings_;  // TreeMap
    int totalTypeWeighting_ = 0;
    JavaHashMap<Species*, int, JavaHashCode<Species*>, SpeciesPtrEq> placementHistory_;
    std::set<Type> usedUberTypes_;
    std::map<int, Type> trainerTypes_;  // HashMap<Trainer, Type> (clave: índice del entrenador)
};

}  // namespace pr
