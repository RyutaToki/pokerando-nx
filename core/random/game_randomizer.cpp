#include "random/game_randomizer.hpp"

namespace pr {

using S = Settings;

GameRandomizer::GameRandomizer(Settings& settings, RomHandler& rh)
    : settings_(settings), rh_(rh), bs_(rh, settings, nonCosmetic_), movesets_(rh, settings, nonCosmetic_), evo_(rh, settings, nonCosmetic_), encItems_(rh, settings, nonCosmetic_), intro_(rh, settings, nonCosmetic_), misc_(rh, settings, nonCosmetic_), moveData_(rh, settings, nonCosmetic_), tmt_(rh, settings, nonCosmetic_), comp_(rh, settings, nonCosmetic_), trainerMoves_(rh, settings, nonCosmetic_), moveNames_(rh, settings, cosmetic_), trainerNames_(rh, settings, cosmetic_), typeEff_(rh, settings, nonCosmetic_), typeEffUpdater_(rh), moveUpdater_(rh), bsUpdater_(rh), types_(rh, settings, nonCosmetic_), abilities_(rh, settings, nonCosmetic_), starter_(rh, settings, nonCosmetic_), statics_(rh, settings, nonCosmetic_), trades_(rh, settings, nonCosmetic_), items_(rh, settings, nonCosmetic_), wild_(rh, settings, nonCosmetic_), trainerPoke_(rh, settings, nonCosmetic_) {}

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
    if (settings_.updateTypeEffectiveness) typeEffUpdater_.updateTypeEffectiveness();
    if (settings_.updateMoves) moveUpdater_.updateMoves(settings_.updateMovesToGeneration);
    if (settings_.updateBaseStats) bsUpdater_.updateSpeciesStats(settings_.updateBaseStatsToGeneration);
}

void GameRandomizer::applyRandomizers() {
    switch (settings_.typeEffectivenessMod) {
        case S::TypeEffectivenessMod::RANDOM: typeEff_.randomizeTypeEffectiveness(false); break;
        case S::TypeEffectivenessMod::RANDOM_BALANCED: typeEff_.randomizeTypeEffectiveness(true); break;
        case S::TypeEffectivenessMod::KEEP_IDENTITIES: typeEff_.randomizeTypeEffectivenessKeepIdentities(); break;
        case S::TypeEffectivenessMod::INVERSE: typeEff_.invertTypeEffectiveness(settings_.inverseTypesRandomImmunities); break;
        default: break;
    }
    if (settings_.randomizeMovePowers) moveData_.randomizeMovePowers();
    if (settings_.randomizeMoveAccuracies) moveData_.randomizeMoveAccuracies();
    if (settings_.randomizeMovePPs) moveData_.randomizeMovePPs();
    if (settings_.randomizeMoveTypes) moveData_.randomizeMoveTypes();
    if (settings_.randomizeMoveNames) moveNames_.randomizeMoveNames();
    if (settings_.randomizeMoveCategory && rh_.hasPhysicalSpecialSplit()) moveData_.randomizeMoveCategory();
    if (settings_.currentMiscTweaks != 0) misc_.applyMiscTweaks();
    if (settings_.standardizeEXPCurves) bs_.standardizeEXPCurves();
    if (settings_.evolutionsMod != S::EvolutionsMod::UNCHANGED) evo_.randomizeEvolutions();
    if (settings_.bstMod != S::BSTMod::UNCHANGED) bs_.randomizeBSTs();
    if (settings_.adjustEvolutionLevels) evo_.adjustEvolutionLevels();
    if (settings_.speciesTypesMod != S::SpeciesTypesMod::UNCHANGED) types_.randomizeSpeciesTypes();
    if (settings_.randomizeWildPokemonHeldItems) encItems_.randomizeWildHeldItems();
    if (settings_.baseStatisticsMod == S::BaseStatisticsMod::SHUFFLE) bs_.shuffleSpeciesStats();
    else if (settings_.baseStatisticsMod == S::BaseStatisticsMod::RANDOM) bs_.randomizeSpeciesStats();
    if (settings_.abilitiesMod == S::AbilitiesMod::RANDOMIZE) abilities_.randomizeAbilities();
    maybeApplyEvolutionImprovements();
    if (settings_.startersMod != S::StartersMod::UNCHANGED) starter_.randomizeStarters();
    if (settings_.randomizeStartersHeldItems) starter_.randomizeStarterHeldItems();
    if (settings_.movesetsMod != S::MovesetsMod::UNCHANGED && settings_.movesetsMod != S::MovesetsMod::METRONOME_ONLY) {
        movesets_.randomizeMovesLearnt();
        movesets_.randomizeEggMoves();
    }
    if (settings_.reorderDamagingMoves) movesets_.orderDamagingMovesByDamage();
    if (settings_.movesetsMod != S::MovesetsMod::METRONOME_ONLY && settings_.tmsMod == S::TMsMod::RANDOM) tmt_.randomizeTMMoves();
    {
        using C = S::TMsHMsCompatibilityMod;
        if (settings_.tmsHmsCompatibilityMod == C::COMPLETELY_RANDOM || settings_.tmsHmsCompatibilityMod == C::RANDOM_PREFER_TYPE)
            comp_.randomizeTMHMCompatibility();
        else if (settings_.tmsHmsCompatibilityMod == C::FULL) comp_.fullTMHMCompatibility();
        if (settings_.tmLevelUpMoveSanity) {
            comp_.ensureTMCompatSanity();
            if (settings_.tmsFollowEvolutions) comp_.ensureTMEvolutionSanity();
        }
        if (settings_.fullHMCompat) comp_.fullHMCompatibility();
        if (comp_.tmhmChangesMade) comp_.copyTMCompatibilityToCosmeticFormes();
    }
    if (rh_.hasMoveTutors()) {
        if (settings_.movesetsMod != S::MovesetsMod::METRONOME_ONLY && settings_.moveTutorMovesMod == S::MoveTutorMovesMod::RANDOM)
            tmt_.randomizeMoveTutorMoves();
        using C = S::MoveTutorsCompatibilityMod;
        if (settings_.moveTutorsCompatibilityMod == C::COMPLETELY_RANDOM || settings_.moveTutorsCompatibilityMod == C::RANDOM_PREFER_TYPE)
            comp_.randomizeMoveTutorCompatibility();
        else if (settings_.moveTutorsCompatibilityMod == C::FULL) comp_.fullMoveTutorCompatibility();
        if (settings_.tutorLevelUpMoveSanity) {
            comp_.ensureMoveTutorCompatSanity();
            if (settings_.tutorFollowEvolutions) comp_.ensureMoveTutorEvolutionSanity();
        }
        if (comp_.tutorChangesMade) comp_.copyMoveTutorCompatibilityToCosmeticFormes();
    }
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
        trainerMoves_.randomizeTrainerMovesets();
    // maybeFixTrainerZCrystals
    if (movesets_.isChangesMade() || trainerPoke_.isChangesMade() || trainerMoves_.isChangesMade()) trainerPoke_.randomUsableZCrystals();
    if (settings_.randomizeHeldItemsForBossTrainerPokemon || settings_.randomizeHeldItemsForImportantTrainerPokemon ||
        settings_.randomizeHeldItemsForRegularTrainerPokemon)
        trainerPoke_.randomizeTrainerHeldItems();
    if (rh_.canChangeTrainerText()) {
        if (settings_.randomizeTrainerClassNames) trainerNames_.randomizeTrainerClassNames();
        if (settings_.randomizeTrainerNames) trainerNames_.randomizeTrainerNames();
    }
    if (settings_.movesetsMod == S::MovesetsMod::METRONOME_ONLY) movesets_.metronomeOnlyMode();
    if (rh_.canChangeStaticPokemon()) {
        if (settings_.staticPokemonMod != S::StaticPokemonMod::UNCHANGED) statics_.randomizeStaticPokemon();
        else if (settings_.staticLevelModified) statics_.onlyChangeStaticLevels();
    }
    if (settings_.inGameTradesMod != S::InGameTradesMod::UNCHANGED) trades_.randomizeIngameTrades();
    if (settings_.fieldItemsMod != S::FieldItemsMod::UNCHANGED) items_.randomizeFieldItems();
    if (settings_.shopItemsMod == S::ShopItemsMod::SHUFFLE) items_.shuffleShopItems();
    else if (settings_.shopItemsMod == S::ShopItemsMod::RANDOM) items_.randomizeShopItems();
    if (settings_.balanceShopPrices) rh_.setBalancedShopPrices();
    if (settings_.addCheapRareCandiesToShops) items_.addCheapRareCandiesToShops();
    if (settings_.pickupItemsMod == S::PickupItemsMod::RANDOM) items_.randomizePickupItems();
    if (settings_.pokemonPalettesMod == S::PokemonPalettesMod::RANDOM) notPorted("paletas");
    if (settings_.randomizeIntroMon) intro_.randomizeIntroPokemon();
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
