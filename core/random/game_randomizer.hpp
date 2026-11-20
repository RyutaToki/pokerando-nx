// Port de GameRandomizer: aplica los randomizadores en el mismo orden que FVX y guarda la ROM.
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "random/species_base_stat_randomizer.hpp"
#include "random/starter_randomizer.hpp"
#include "random/trainer_pokemon_randomizer.hpp"
#include "random/wild_encounter_randomizer.hpp"
#include "romio/rom_handler.hpp"
#include "settings/settings.hpp"
#include "util/java_random.hpp"

namespace pr {

class GameRandomizer {
public:
    GameRandomizer(Settings& settings, RomHandler& romHandler);
    // Lanza excepción si algo falla. saveRom recibe la ruta de salida.
    void randomize(const std::string& outFile, int64_t seed);

    int checkValue() const { return checkValue_; }
    JavaRandom& nonCosmetic() { return nonCosmetic_; }
    JavaRandom& cosmetic() { return cosmetic_; }

private:
    void setupSpeciesRestrictions();
    void applyUpdaters();
    void applyRandomizers();
    void maybeApplyEvolutionImprovements();
    int calculateCheckValue();
    [[noreturn]] void notPorted(const char* what);

    Settings& settings_;
    RomHandler& rh_;
    JavaRandom nonCosmetic_, cosmetic_;
    SpeciesBaseStatRandomizer bs_;
    StarterRandomizer starter_;
    WildEncounterRandomizer wild_;
    TrainerPokemonRandomizer trainerPoke_;
    int checkValue_ = 0;
};

}  // namespace pr
