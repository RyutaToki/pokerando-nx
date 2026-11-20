// Port de GameRandomizer: aplica los randomizadores en el mismo orden que FVX y guarda la ROM.
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "random/evolution_randomizer.hpp"
#include "random/misc_randomizers.hpp"
#include "random/move_randomizers.hpp"
#include "random/species_base_stat_randomizer.hpp"
#include "random/species_moveset_randomizer.hpp"
#include "random/species_type_ability_randomizers.hpp"
#include "random/starter_randomizer.hpp"
#include "random/tmhm_compat_randomizer.hpp"
#include "random/trainer_moveset_randomizer.hpp"
#include "random/static_pokemon_randomizer.hpp"
#include "random/trade_item_randomizers.hpp"
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
    SpeciesMovesetRandomizer movesets_;
    EvolutionRandomizer evo_;
    EncounterHeldItemRandomizer encItems_;
    IntroPokemonRandomizer intro_;
    MiscTweakRandomizer misc_;
    MoveDataRandomizer moveData_;
    TMTutorMoveRandomizer tmt_;
    TMHMTutorCompatibilityRandomizer comp_;
    TrainerMovesetRandomizer trainerMoves_;
    SpeciesTypeRandomizer types_;
    SpeciesAbilityRandomizer abilities_;
    StarterRandomizer starter_;
    StaticPokemonRandomizer statics_;
    TradeRandomizer trades_;
    ItemRandomizer items_;
    WildEncounterRandomizer wild_;
    TrainerPokemonRandomizer trainerPoke_;
    int checkValue_ = 0;
};

}  // namespace pr
