// Port de Gen3to5PaletteRandomizer.
#pragma once
#include <memory>

#include "graphics/palette_populate.hpp"
#include "random/randomizer.hpp"

namespace pr {

class Gen3to5PaletteRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizePokemonPalettes();
};

}  // namespace pr
