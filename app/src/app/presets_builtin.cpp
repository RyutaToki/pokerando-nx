// Presets de serie (§6.5).
#include "app/state.hpp"
#include "romio/misc_tweak.hpp"
#include "settings/settings.hpp"

namespace app {

using S = pr::Settings;

namespace {
Preset make(const char* name, const S& s) {
    Preset p;
    p.name = name;
    p.settings = s.toString();
    p.builtin = true;
    p.date = "de serie";
    return p;
}
}  // namespace

std::vector<Preset> builtinPresets() {
    std::vector<Preset> out;
    {
        S s;  // Caos total: todo aleatorio
        s.baseStatisticsMod = S::BaseStatisticsMod::RANDOM;
        s.baseStatsFollowEvolutions = true;
        s.speciesTypesMod = S::SpeciesTypesMod::RANDOM_FOLLOW_EVOLUTIONS;
        s.abilitiesMod = S::AbilitiesMod::RANDOMIZE;
        s.abilitiesFollowEvolutions = true;
        s.evolutionsMod = S::EvolutionsMod::RANDOM;
        s.startersMod = S::StartersMod::COMPLETELY_RANDOM;
        s.staticPokemonMod = S::StaticPokemonMod::RANDOM_MATCHING;
        s.inGameTradesMod = S::InGameTradesMod::RANDOMIZE_GIVEN_AND_REQUESTED;
        s.movesetsMod = S::MovesetsMod::COMPLETELY_RANDOM;
        s.randomizeMovePowers = s.randomizeMoveAccuracies = s.randomizeMovePPs = s.randomizeMoveTypes = true;
        s.trainersMod = S::TrainersMod::RANDOM;
        s.randomizeWildPokemon = true;
        s.tmsMod = S::TMsMod::RANDOM;
        s.tmsHmsCompatibilityMod = S::TMsHMsCompatibilityMod::COMPLETELY_RANDOM;
        s.moveTutorMovesMod = S::MoveTutorMovesMod::RANDOM;
        s.moveTutorsCompatibilityMod = S::MoveTutorsCompatibilityMod::COMPLETELY_RANDOM;
        s.fieldItemsMod = S::FieldItemsMod::RANDOM;
        s.shopItemsMod = S::ShopItemsMod::RANDOM;
        s.pickupItemsMod = S::PickupItemsMod::RANDOM;
        s.typeEffectivenessMod = S::TypeEffectivenessMod::RANDOM_BALANCED;
        s.pokemonPalettesMod = S::PokemonPalettesMod::RANDOM;
        s.changeImpossibleEvolutions = true;
        out.push_back(make("Caos total", s));
    }
    {
        S s;  // Solo salvajes
        s.randomizeWildPokemon = true;
        s.similarStrengthEncounters = true;
        out.push_back(make("Solo salvajes", s));
    }
    {
        S s;  // Nuzlocke justo: zonas 1 a 1, fuerza similar y evoluciones posibles
        s.startersMod = S::StartersMod::RANDOM_WITH_TWO_EVOLUTIONS;
        s.startersTypeMod = S::StartersTypeMod::UNIQUE;
        s.randomizeWildPokemon = true;
        s.wildPokemonZoneMod = S::WildPokemonZoneMod::ENCOUNTER_SET;
        s.similarStrengthEncounters = true;
        s.catchEmAllEncounters = true;
        s.blockWildLegendaries = true;
        s.trainersMod = S::TrainersMod::RANDOM;
        s.trainersUsePokemonOfSimilarStrength = true;
        s.rivalCarriesStarterThroughout = true;
        s.tmsHmsCompatibilityMod = S::TMsHMsCompatibilityMod::RANDOM_PREFER_TYPE;
        s.changeImpossibleEvolutions = true;
        s.makeEvolutionsEasier = true;
        s.fieldItemsMod = S::FieldItemsMod::SHUFFLE;
        s.currentMiscTweaks = pr::misctweak::FASTEST_TEXT.value | pr::misctweak::RUNNING_SHOES_INDOORS.value;
        out.push_back(make("Nuzlocke justo", s));
    }
    {
        S s;  // Kaizo: rivales mucho más duros
        s.startersMod = S::StartersMod::RANDOM_BASIC;
        s.randomizeWildPokemon = true;
        s.trainersMod = S::TrainersMod::RANDOM;
        s.trainersUsePokemonOfSimilarStrength = true;
        s.trainersEvolveTheirPokemon = true;
        s.trainersLevelModified = true;
        s.trainersLevelModifier = 30;
        s.additionalBossTrainerPokemon = 2;
        s.additionalImportantTrainerPokemon = 1;
        s.randomizeHeldItemsForBossTrainerPokemon = s.randomizeHeldItemsForImportantTrainerPokemon = true;
        s.sensibleItemsOnlyForTrainerPokemon = true;
        s.betterBossTrainerMovesets = s.betterImportantTrainerMovesets = s.betterRegularTrainerMovesets = true;
        s.eliteFourUniquePokemonNumber = 2;
        s.changeImpossibleEvolutions = true;
        out.push_back(make("Kaizo", s));
    }
    return out;
}

}  // namespace app
