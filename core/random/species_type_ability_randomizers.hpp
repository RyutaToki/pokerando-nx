// Port de SpeciesTypeRandomizer y SpeciesAbilityRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class SpeciesTypeRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeSpeciesTypes();

private:
    void assignRandomSecondaryType(Species* sp, double chance, bool dualTypeOnly);
};

class SpeciesAbilityRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeAbilities();

private:
    int pickRandomAbilityVariation(int selected, const std::vector<int>& alreadySet);
    int pickRandomAbility(int maxAbility, const std::vector<int>& banned, bool useVariations, const std::vector<int>& alreadySet);
};

}  // namespace pr
