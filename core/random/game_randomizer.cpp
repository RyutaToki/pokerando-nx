#include "random/game_randomizer.hpp"

namespace pr {

using S = Settings;

GameRandomizer::GameRandomizer(Settings& settings, RomHandler& rh)
    : settings_(settings), rh_(rh), starter_(rh, settings, nonCosmetic_), wild_(rh, settings, nonCosmetic_), trainerPoke_(rh, settings, nonCosmetic_) {}

void GameRandomizer::notPorted(const char* what) {
    throw std::runtime_error(std::string("Opción aún no portada: ") + what);
}

void GameRandomizer::randomize(const std::string& outFile, int64_t seed) {
    nonCosmetic_.setSeed(seed);
    cosmetic_.setSeed(seed);
    setupSpeciesRestrictions();
    applyUpdaters();
    applyRandomizers();
    // CheckValueCalculator: en FVX se calcula siempre; leer los estáticos puede aplicar parches
    // a la ROM (p. ej. el de los errantes de Esmeralda), así que hay que hacerlo igual.
    checkValue_ = calculateCheckValue();
    rh_.saveRomTo(outFile);
}

void GameRandomizer::setupSpeciesRestrictions() {
    rh_.getRestrictedSpeciesService().setRestrictions(settings_.currentRestrictions);
    if (settings_.limitPokemon) rh_.removeEvosForPokemonPool(rh_.getRestrictedSpeciesService().getAll(false));
}

void GameRandomizer::applyUpdaters() {
    if (settings_.updateTypeEffectiveness) notPorted("actualizar efectividades de tipos");
    if (settings_.updateMoves) notPorted("actualizar movimientos");
    if (settings_.updateBaseStats) notPorted("actualizar stats base");
}

void GameRandomizer::applyRandomizers() {
    if (settings_.typeEffectivenessMod != S::TypeEffectivenessMod::UNCHANGED) notPorted("tabla de tipos");
    if (settings_.randomizeMovePowers || settings_.randomizeMoveAccuracies || settings_.randomizeMovePPs ||
        settings_.randomizeMoveTypes || settings_.randomizeMoveNames)
        notPorted("datos de movimientos");
    if (settings_.currentMiscTweaks != 0) notPorted("ajustes varios");
    if (settings_.standardizeEXPCurves) notPorted("curvas de EXP");
    if (settings_.evolutionsMod != S::EvolutionsMod::UNCHANGED) notPorted("evoluciones");
    if (settings_.bstMod != S::BSTMod::UNCHANGED) notPorted("BST");
    if (settings_.adjustEvolutionLevels) notPorted("ajustar niveles de evolución");
    if (settings_.speciesTypesMod != S::SpeciesTypesMod::UNCHANGED) notPorted("tipos de Pokémon");
    if (settings_.randomizeWildPokemonHeldItems) notPorted("objetos de salvajes");
    if (settings_.baseStatisticsMod != S::BaseStatisticsMod::UNCHANGED) notPorted("stats base");
    if (settings_.abilitiesMod == S::AbilitiesMod::RANDOMIZE) notPorted("habilidades");
    maybeApplyEvolutionImprovements();
    if (settings_.startersMod != S::StartersMod::UNCHANGED) starter_.randomizeStarters();
    if (settings_.randomizeStartersHeldItems) starter_.randomizeStarterHeldItems();
    if (settings_.movesetsMod != S::MovesetsMod::UNCHANGED || settings_.reorderDamagingMoves) notPorted("movimientos por nivel");
    if (settings_.tmsMod == S::TMsMod::RANDOM) notPorted("MT");
    if (settings_.tmsHmsCompatibilityMod != S::TMsHMsCompatibilityMod::UNCHANGED || settings_.tmLevelUpMoveSanity ||
        settings_.fullHMCompat)
        notPorted("compatibilidad MT/MO");
    if (rh_.hasMoveTutors() && (settings_.moveTutorMovesMod == S::MoveTutorMovesMod::RANDOM ||
                                settings_.moveTutorsCompatibilityMod != S::MoveTutorsCompatibilityMod::UNCHANGED ||
                                settings_.tutorLevelUpMoveSanity))
        notPorted("tutores");
    if (settings_.useMinimumCatchRate) wild_.changeCatchRates();
    if (settings_.randomizeWildPokemon || settings_.wildLevelsModified) wild_.randomizeEncounters();
    {
        auto& tp = trainerPoke_;
        if (settings_.trainersLevelModified) tp.applyTrainerLevelModifier();
        bool added = settings_.additionalRegularTrainerPokemon > 0 || settings_.additionalImportantTrainerPokemon > 0 ||
                     settings_.additionalBossTrainerPokemon > 0;
        if (added) tp.addTrainerPokemon();
        if (settings_.settingBattleStyle.isBattleStyleChanged()) tp.modifyBattleStyle();
        if ((settings_.trainersMod != S::TrainersMod::UNCHANGED || settings_.startersMod != S::StartersMod::UNCHANGED) &&
            settings_.rivalCarriesStarterThroughout)
            tp.makeRivalCarryStarter();
        if (settings_.trainersMod != S::TrainersMod::UNCHANGED || added) tp.randomizeTrainerPokes();
        else if (settings_.trainersEvolveTheirPokemon) tp.evolveTrainerPokemonAsFarAsLegal();
    }
    if (settings_.betterBossTrainerMovesets || settings_.betterImportantTrainerMovesets || settings_.betterRegularTrainerMovesets)
        notPorted("movimientos de entrenadores");
    // maybeFixTrainerZCrystals
    if (trainerPoke_.isChangesMade()) trainerPoke_.randomUsableZCrystals();
    if (settings_.randomizeHeldItemsForBossTrainerPokemon || settings_.randomizeHeldItemsForImportantTrainerPokemon ||
        settings_.randomizeHeldItemsForRegularTrainerPokemon)
        trainerPoke_.randomizeTrainerHeldItems();
    if (settings_.randomizeTrainerClassNames || settings_.randomizeTrainerNames) notPorted("nombres de entrenadores");
    if (settings_.movesetsMod == S::MovesetsMod::METRONOME_ONLY) notPorted("solo Metrónomo");
    if (rh_.canChangeStaticPokemon() && (settings_.staticPokemonMod != S::StaticPokemonMod::UNCHANGED || settings_.staticLevelModified))
        notPorted("estáticos");
    if (settings_.inGameTradesMod != S::InGameTradesMod::UNCHANGED) notPorted("intercambios");
    if (settings_.fieldItemsMod != S::FieldItemsMod::UNCHANGED) notPorted("objetos del suelo");
    if (settings_.shopItemsMod != S::ShopItemsMod::UNCHANGED) notPorted("tiendas");
    if (settings_.balanceShopPrices) rh_.setBalancedShopPrices();
    if (settings_.addCheapRareCandiesToShops) notPorted("Caramelos Raros baratos");
    if (settings_.pickupItemsMod == S::PickupItemsMod::RANDOM) notPorted("Recogida");
    if (settings_.pokemonPalettesMod == S::PokemonPalettesMod::RANDOM) notPorted("paletas");
    if (settings_.randomizeIntroMon) notPorted("Pokémon de la introducción");
}

void GameRandomizer::maybeApplyEvolutionImprovements() {
    bool useEstimated = settings_.useEstimatedLevelsForEvolutionImprovements();
    if (settings_.changeImpossibleEvolutions)
        rh_.removeImpossibleEvolutions(settings_.movesetsMod != S::MovesetsMod::UNCHANGED, useEstimated);
    if (settings_.makeEvolutionsEasier) {
        rh_.condenseLevelEvolutions(settings_.makeEvolutionsEasierLvl);
        rh_.makeEvolutionsEasier(settings_.randomizeWildPokemon, useEstimated);
    }
    if (settings_.removeTimeBasedEvolutions) rh_.removeTimeBasedEvolutions();
}

}  // namespace pr

namespace pr {

int GameRandomizer::calculateCheckValue() {
    uint32_t cv = 0;
    auto add = [&](std::initializer_list<int> values) {
        for (int v : values) {
            cv = (cv << 3) | (cv >> 29);  // Integer.rotateLeft(cv, 3)
            cv ^= static_cast<uint32_t>(v);
        }
    };
    for (Species* pk : rh_.getSpecies()) {
        if (!pk) continue;
        const auto& bs = pk->baseStats();
        add({bs.getHp(), bs.attack, bs.defense, bs.speed, bs.spatk, bs.spdef});
        add({pk->ability1, pk->ability2, pk->ability3});
    }
    for (const auto& t : rh_.getTrainers())
        for (const auto& tp : t->pokemon) add({tp->level, tp->species()->number()});
    bool timeBased = settings_.useTimeBasedEncounters || (!settings_.randomizeWildPokemon && settings_.wildLevelsModified);
    for (const auto& area : rh_.getEncounters(timeBased))
        for (const auto& e : area.encounters) add({e->level, e->species()->number()});
    for (int m : rh_.getTMMoves()) add({m});
    for (int m : rh_.getMoveTutorMoves()) add({m});
    for (const auto& se : rh_.getStaticPokemon()) add({se.species()->number()});
    return static_cast<int>(cv);
}

}  // namespace pr
