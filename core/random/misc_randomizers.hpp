// Port de EncounterHeldItemRandomizer, IntroPokemonRandomizer y MiscTweakRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class EncounterHeldItemRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeWildHeldItems();
};

class IntroPokemonRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeIntroPokemon();
    Species* introSpecies = nullptr;
};

class MiscTweakRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void applyMiscTweaks();

private:
    void randomizeCatchingTutorial();
    void randomizePCPotion();
};

}  // namespace pr
