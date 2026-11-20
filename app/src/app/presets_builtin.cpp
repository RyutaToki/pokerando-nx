// Presets de serie (§6.5).
#include "app/state.hpp"
#include "romio/misc_tweak.hpp"
#include "settings/settings.hpp"

namespace app {

using S = pr::Settings;

namespace {
Preset make(const char* name, S s) {
    // como en la interfaz de FVX, la curva de EXP siempre tiene valor (la primera del juego: Medium Fast)
    if (!s.selectedEXPCurve) s.selectedEXPCurve = pr::ExpCurve::MEDIUM_FAST;
    Preset p;
    p.name = name;
    p.settings = s.toString();
    p.builtin = true;
    p.date = "de serie";
    return p;
}
// Base común de los randomlocke: lo que casi todo el mundo activa para jugar un nuzlocke aleatorio.
S randomlockeBase() {
    S s;
    // iniciales aleatorias con dos evoluciones y tipos distintos
    s.startersMod = S::StartersMod::RANDOM_WITH_TWO_EVOLUTIONS;
    s.startersTypeMod = S::StartersTypeMod::UNIQUE;
    // salvajes: cada zona 1 a 1, fuerza parecida, sin legendarios y todas las especies capturables
    s.randomizeWildPokemon = true;
    s.wildPokemonZoneMod = S::WildPokemonZoneMod::ENCOUNTER_SET;
    s.similarStrengthEncounters = true;
    s.catchEmAllEncounters = true;
    s.blockWildLegendaries = true;
    // entrenadores aleatorios con fuerza parecida; el rival lleva tu inicial
    s.trainersMod = S::TrainersMod::RANDOM;
    s.trainersUsePokemonOfSimilarStrength = true;
    s.rivalCarriesStarterThroughout = true;
    // regalos, legendarios fijos e intercambios
    s.staticPokemonMod = S::StaticPokemonMod::RANDOM_MATCHING;
    s.inGameTradesMod = S::InGameTradesMod::RANDOMIZE_GIVEN;
    // evoluciones posibles sin intercambio y algo más fáciles
    s.changeImpossibleEvolutions = true;
    s.makeEvolutionsEasier = true;
    // compatibilidad de MT según el tipo, para que las MT sigan sirviendo
    s.tmsHmsCompatibilityMod = S::TMsHMsCompatibilityMod::RANDOM_PREFER_TYPE;
    // comodidad
    s.currentMiscTweaks = pr::misctweak::FASTEST_TEXT.value | pr::misctweak::RUNNING_SHOES_INDOORS.value;
    return s;
}

// Añade lo «completo»: habilidades, movimientos, MT, tutores y objetos aleatorios (sin cosas rotas).
void randomlockeFull(S& s) {
    s.abilitiesMod = S::AbilitiesMod::RANDOMIZE;
    s.abilitiesFollowEvolutions = true;
    s.banTrappingAbilities = s.banNegativeAbilities = s.banBadAbilities = true;
    s.movesetsMod = S::MovesetsMod::RANDOM_PREFER_SAME_TYPE;
    s.startWithGuaranteedMoves = true;
    s.guaranteedMoveCount = 4;
    s.movesetsForceGoodDamaging = true;
    s.movesetsGoodDamagingPercent = 50;
    s.blockBrokenMovesetMoves = true;
    s.tmsMod = S::TMsMod::RANDOM;
    s.keepFieldMoveTMs = true;
    s.blockBrokenTMMoves = true;
    s.moveTutorMovesMod = S::MoveTutorMovesMod::RANDOM;
    s.keepFieldMoveTutors = true;
    s.blockBrokenTutorMoves = true;
    s.fieldItemsMod = S::FieldItemsMod::RANDOM;
    s.banBadRandomFieldItems = true;
    s.guaranteeEvolutionItems = true;
}
}  // namespace

std::vector<Preset> builtinPresets() {
    std::vector<Preset> out;
    {
        // Randomlocke básico: lo justo para un nuzlocke aleatorio; el juego sigue siendo reconocible
        out.push_back(make("Randomlocke básico", randomlockeBase()));
    }
    {
        // Randomlocke completo: además habilidades, movimientos, MT, tutores y objetos aleatorios
        S s = randomlockeBase();
        randomlockeFull(s);
        out.push_back(make("Randomlocke completo", s));
    }
    {
        // Randomlocke difícil: el completo con rivales más fuertes y mejor preparados
        S s = randomlockeBase();
        randomlockeFull(s);
        s.trainersLevelModified = true;
        s.trainersLevelModifier = 15;
        s.trainersEvolveTheirPokemon = true;
        s.additionalBossTrainerPokemon = 1;
        s.randomizeHeldItemsForBossTrainerPokemon = true;
        s.sensibleItemsOnlyForTrainerPokemon = true;
        s.betterBossTrainerMovesets = s.betterImportantTrainerMovesets = true;
        out.push_back(make("Randomlocke difícil", s));
    }
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
