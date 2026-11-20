// Port de StaticPokemonRandomizer (sin tótems: solo existen en Gen 7).
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class StaticPokemonRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void onlyChangeStaticLevels();
    void randomizeStaticPokemon();

private:
    Species* getRestricted(const SpeciesSet& full, SpeciesSet& left, const StaticEncounter& old);
    Species* pickPowerLvlReplacement(SpeciesSet& pool, Species* current, bool banSame, bool limitBST);
    void randomizeCosmeticForme(StaticEncounter& se);
};

}  // namespace pr
