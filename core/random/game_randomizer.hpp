// Port de GameRandomizer: aplica los randomizadores en el mismo orden que FVX y guarda la ROM.
#pragma once
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "random/evolution_randomizer.hpp"
#include "random/misc_randomizers.hpp"
#include "random/move_randomizers.hpp"
#include "random/name_randomizers.hpp"
#include "random/palette_randomizer.hpp"
#include "random/type_effectiveness_randomizer.hpp"
#include "random/updaters.hpp"
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
    // Lanza excepción si algo falla. Si log no es nulo, escribe ahí el registro (spoiler log).
    void randomize(const std::string& outFile, int64_t seed, std::ostream* log = nullptr);

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
    void captureOriginals();
    void writeLog(std::ostream& log, long long elapsedMs, int64_t seed);

    // estado original que necesita el registro (lo guarda RandomizationLogger al construirse)
    std::vector<int> origMTMoves_;
    std::vector<std::string> origTrainerNames_;
    std::vector<StaticEncounter> origStatics_;
    std::vector<InGameTrade> origTrades_;
    std::vector<std::optional<std::string>> origMoveNames_;

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
    MoveNameRandomizer moveNames_;
    TrainerNameRandomizer trainerNames_;
    TypeEffectivenessRandomizer typeEff_;
    Gen3to5PaletteRandomizer palettes_;
    TypeEffectivenessUpdater typeEffUpdater_;
    MoveUpdater moveUpdater_;
    SpeciesBaseStatUpdater bsUpdater_;
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
