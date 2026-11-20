// Port de EvolutionRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class EvolutionRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeEvolutions();
    void adjustEvolutionLevels() {
        romHandler.adjustEvolutionLevels();
        changesMade = true;
    }
};

}  // namespace pr
