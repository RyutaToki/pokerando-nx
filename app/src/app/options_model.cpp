#include "app/options_model.hpp"

#include "romio/misc_tweak.hpp"

namespace app {

using S = pr::Settings;
namespace mt = pr::misctweak;

pr::Settings defaultSettings() {
    pr::Settings s;
    s.selectedEXPCurve = pr::ExpCurve::MEDIUM_FAST;
    return s;
}

void tweakForRom(pr::Settings& s, const pr::RomHandler& rh) {
    s.tweakForRom(rh);
    if (!s.selectedEXPCurve) {
        auto curves = rh.getExpCurves();
        if (!curves.empty()) s.selectedEXPCurve = curves[0];
    }
}

RomCaps RomCaps::from(const pr::RomHandler& rh) {
    RomCaps c;
    c.gen = rh.generationOfPokemon();
    c.abilities = rh.abilitiesPerSpecies() > 0;
    c.tutors = rh.hasMoveTutors();
    c.statics = rh.canChangeStaticPokemon();
    c.split = rh.hasPhysicalSpecialSplit();
    c.shops = rh.hasShopSupport();
    c.typeTable = rh.hasTypeEffectivenessSupport();
    c.palettes = rh.hasPokemonPaletteSupport();
    c.fourStartingMoves = rh.supportsFourStartingMoves();
    c.triangle = rh.hasStarterTypeTriangleSupport();
    c.bossPokemon = rh.canAddPokemonToBossTrainers();
    c.bossItems = rh.canAddHeldItemsToBossTrainers();
    c.trainerText = rh.canChangeTrainerText();
    c.tweaks = rh.miscTweaksAvailable();
    return c;
}

namespace {
auto always = [](const RomCaps&) { return true; };
auto on = [](const OptState&) { return true; };

ToggleDef tog(std::string label, std::function<bool(const OptState&)> get, std::function<void(OptState&, bool)> set,
              std::function<bool(const OptState&)> enabled = on, std::function<bool(const RomCaps&)> avail = always) {
    return ToggleDef{std::move(label), std::move(avail), std::move(enabled), std::move(get), std::move(set)};
}

ToggleDef tweak(std::string label, const pr::MiscTweak& t) {
    int v = t.value;
    return tog(std::move(label), [v](const OptState& o) { return (o.s.currentMiscTweaks & v) != 0; },
               [v](OptState& o, bool b) { o.s.currentMiscTweaks = b ? (o.s.currentMiscTweaks | v) : (o.s.currentMiscTweaks & ~v); }, on,
               [v](const RomCaps& c) { return (c.tweaks & v) != 0; });
}

bool randomStarters(const OptState& o) {
    return o.s.startersMod == S::StartersMod::COMPLETELY_RANDOM || o.s.startersMod == S::StartersMod::RANDOM_BASIC ||
           o.s.startersMod == S::StartersMod::RANDOM_WITH_TWO_EVOLUTIONS;
}
bool randomMovesets(const OptState& o) {
    return o.s.movesetsMod == S::MovesetsMod::COMPLETELY_RANDOM || o.s.movesetsMod == S::MovesetsMod::RANDOM_PREFER_SAME_TYPE;
}
bool randomTrainers(const OptState& o) { return o.s.trainersMod != S::TrainersMod::UNCHANGED; }
bool randomWild(const OptState& o) { return o.s.randomizeWildPokemon; }

ToggleDef starterType(std::string label, S::StartersTypeMod m) {
    return tog(std::move(label), [m](const OptState& o) { return o.s.startersTypeMod == m; },
               [m](OptState& o, bool b) { o.s.startersTypeMod = b ? m : S::StartersTypeMod::NONE; }, randomStarters);
}

std::vector<TabDef> build() {
    std::vector<TabDef> t;
    const S d;  // valores por defecto de FVX

    // ---------- 7.1 Pokémon base ----------
    {
        TabDef tab;
        tab.title = "Pokémon base";
        tab.available = always;
        tab.mode = ModeDef{"ESTADÍSTICAS BASE", {"Sin cambios", "Mezclar", "Aleatorias"},
                           [](const OptState& o) { return static_cast<int>(o.s.baseStatisticsMod); },
                           [](OptState& o, int i) { o.s.baseStatisticsMod = static_cast<S::BaseStatisticsMod>(i); }};
        auto bsOn = [](const OptState& o) { return o.s.baseStatisticsMod != S::BaseStatisticsMod::UNCHANGED; };
        auto evoOn = [](const OptState& o) { return o.s.evolutionsMod != S::EvolutionsMod::UNCHANGED; };
        tab.toggles = {
            tog("Seguir evoluciones", [](const OptState& o) { return o.s.baseStatsFollowEvolutions; },
                [](OptState& o, bool b) { o.s.baseStatsFollowEvolutions = b; }, bsOn),
            tog("Actualizar stats a Gen moderna", [](const OptState& o) { return o.s.updateBaseStats; },
                [](OptState& o, bool b) {
                    o.s.updateBaseStats = b;
                    o.s.updateBaseStatsToGeneration = b ? 9 : 0;
                }),
            tog("Tipos aleatorios", [](const OptState& o) { return o.s.speciesTypesMod != S::SpeciesTypesMod::UNCHANGED; },
                [](OptState& o, bool b) { o.s.speciesTypesMod = b ? S::SpeciesTypesMod::RANDOM_FOLLOW_EVOLUTIONS : S::SpeciesTypesMod::UNCHANGED; }),
            tog("Habilidades aleatorias", [](const OptState& o) { return o.s.abilitiesMod == S::AbilitiesMod::RANDOMIZE; },
                [](OptState& o, bool b) {
                    o.s.abilitiesMod = b ? S::AbilitiesMod::RANDOMIZE : S::AbilitiesMod::UNCHANGED;
                    o.s.abilitiesFollowEvolutions = b;
                },
                on, [](const RomCaps& c) { return c.abilities; }),
            tog("Evoluciones aleatorias", [](const OptState& o) { return o.s.evolutionsMod != S::EvolutionsMod::UNCHANGED; },
                [](OptState& o, bool b) { o.s.evolutionsMod = b ? S::EvolutionsMod::RANDOM : S::EvolutionsMod::UNCHANGED; }),
            tog("Evolucionar siempre a más fuerte", [](const OptState& o) { return o.s.evosForceGrowth; },
                [](OptState& o, bool b) { o.s.evosForceGrowth = b; }, evoOn),
            tog("Sin evoluciones convergentes", [](const OptState& o) { return o.s.evosNoConvergence; },
                [](OptState& o, bool b) { o.s.evosNoConvergence = b; }, evoOn),
            tog("Curvas de EXP iguales", [](const OptState& o) { return o.s.standardizeEXPCurves; },
                [](OptState& o, bool b) { o.s.standardizeEXPCurves = b; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.baseStatisticsMod = d.baseStatisticsMod;
            o.s.baseStatsFollowEvolutions = d.baseStatsFollowEvolutions;
            o.s.updateBaseStats = d.updateBaseStats;
            o.s.updateBaseStatsToGeneration = d.updateBaseStatsToGeneration;
            o.s.speciesTypesMod = d.speciesTypesMod;
            o.s.abilitiesMod = d.abilitiesMod;
            o.s.abilitiesFollowEvolutions = d.abilitiesFollowEvolutions;
            o.s.evolutionsMod = d.evolutionsMod;
            o.s.evosForceGrowth = d.evosForceGrowth;
            o.s.evosNoConvergence = d.evosNoConvergence;
            o.s.standardizeEXPCurves = d.standardizeEXPCurves;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.2 Iniciales y estáticos ----------
    {
        TabDef tab;
        tab.title = "Iniciales y estáticos";
        tab.available = always;
        tab.mode = ModeDef{"INICIALES", {"Sin cambios", "Elegir a mano", "Aleatorios", "Cualquier básico"},
                           [](const OptState& o) {
                               switch (o.s.startersMod) {
                                   case S::StartersMod::CUSTOM: return 1;
                                   case S::StartersMod::COMPLETELY_RANDOM:
                                   case S::StartersMod::RANDOM_WITH_TWO_EVOLUTIONS: return 2;
                                   case S::StartersMod::RANDOM_BASIC: return 3;
                                   default: return 0;
                               }
                           },
                           [](OptState& o, int i) {
                               static const S::StartersMod m[] = {S::StartersMod::UNCHANGED, S::StartersMod::CUSTOM,
                                                                  S::StartersMod::COMPLETELY_RANDOM, S::StartersMod::RANDOM_BASIC};
                               o.s.startersMod = m[i];
                           }};
        tab.toggles = {
            starterType("Fuego / Agua / Planta", S::StartersTypeMod::FIRE_WATER_GRASS),
            starterType("Triángulo de tipos", S::StartersTypeMod::TRIANGLE),
            starterType("Sin tipos repetidos", S::StartersTypeMod::UNIQUE),
            tog("Sin doble tipo", [](const OptState& o) { return o.s.startersNoDualTypes; },
                [](OptState& o, bool b) { o.s.startersNoDualTypes = b; }, randomStarters),
            tog("Legendarios y regalos aleatorios", [](const OptState& o) { return o.s.staticPokemonMod != S::StaticPokemonMod::UNCHANGED; },
                [](OptState& o, bool b) { o.s.staticPokemonMod = b ? S::StaticPokemonMod::RANDOM_MATCHING : S::StaticPokemonMod::UNCHANGED; },
                on, [](const RomCaps& c) { return c.statics; }),
            tog("Intercambios aleatorios", [](const OptState& o) { return o.s.inGameTradesMod != S::InGameTradesMod::UNCHANGED; },
                [](OptState& o, bool b) {
                    o.s.inGameTradesMod = b ? S::InGameTradesMod::RANDOMIZE_GIVEN_AND_REQUESTED : S::InGameTradesMod::UNCHANGED;
                }),
        };
        // el triángulo solo existe si hay 3 iniciales
        tab.toggles[1].available = [](const RomCaps& c) { return c.triangle; };
        tab.reset = [d](OptState& o) {
            o.s.startersMod = d.startersMod;
            o.s.startersTypeMod = d.startersTypeMod;
            o.s.startersNoDualTypes = d.startersNoDualTypes;
            o.s.customStarters = d.customStarters;
            o.s.staticPokemonMod = d.staticPokemonMod;
            o.s.inGameTradesMod = d.inGameTradesMod;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.3 Movimientos ----------
    {
        TabDef tab;
        tab.title = "Movimientos";
        tab.available = always;
        tab.mode = ModeDef{"MOVIMIENTOS POR NIVEL", {"Sin cambios", "Aleatorios", "Del mismo tipo", "Solo Metrónomo"},
                           [](const OptState& o) {
                               switch (o.s.movesetsMod) {
                                   case S::MovesetsMod::COMPLETELY_RANDOM: return 1;
                                   case S::MovesetsMod::RANDOM_PREFER_SAME_TYPE: return 2;
                                   case S::MovesetsMod::METRONOME_ONLY: return 3;
                                   default: return 0;
                               }
                           },
                           [](OptState& o, int i) {
                               static const S::MovesetsMod m[] = {S::MovesetsMod::UNCHANGED, S::MovesetsMod::COMPLETELY_RANDOM,
                                                                  S::MovesetsMod::RANDOM_PREFER_SAME_TYPE, S::MovesetsMod::METRONOME_ONLY};
                               o.s.movesetsMod = m[i];
                           }};
        tab.toggles = {
            tog("Potencia, precisión y PP aleatorios",
                [](const OptState& o) { return o.s.randomizeMovePowers || o.s.randomizeMoveAccuracies || o.s.randomizeMovePPs; },
                [](OptState& o, bool b) { o.s.randomizeMovePowers = o.s.randomizeMoveAccuracies = o.s.randomizeMovePPs = b; }),
            tog("Tipo de los movimientos aleatorio", [](const OptState& o) { return o.s.randomizeMoveTypes; },
                [](OptState& o, bool b) { o.s.randomizeMoveTypes = b; }),
            tog("Categoría física/especial aleatoria", [](const OptState& o) { return o.s.randomizeMoveCategory; },
                [](OptState& o, bool b) { o.s.randomizeMoveCategory = b; }, on, [](const RomCaps& c) { return c.split; }),
            tog("4 movimientos desde nivel 1", [](const OptState& o) { return o.s.startWithGuaranteedMoves && o.s.guaranteedMoveCount == 4; },
                [](OptState& o, bool b) {
                    o.s.startWithGuaranteedMoves = b;
                    o.s.guaranteedMoveCount = b ? 4 : 2;
                },
                randomMovesets, [](const RomCaps& c) { return c.fourStartingMoves; }),
            tog("Ordenar ataques por potencia", [](const OptState& o) { return o.s.reorderDamagingMoves; },
                [](OptState& o, bool b) { o.s.reorderDamagingMoves = b; },
                [](const OptState& o) { return o.s.movesetsMod != S::MovesetsMod::METRONOME_ONLY; }),
            tog("Prohibir movimientos rompejuegos", [](const OptState& o) { return o.s.blockBrokenMovesetMoves; },
                [](OptState& o, bool b) { o.s.blockBrokenMovesetMoves = o.s.blockBrokenTMMoves = o.s.blockBrokenTutorMoves = b; }),
            tog("Nombres de movimientos aleatorios", [](const OptState& o) { return o.s.randomizeMoveNames; },
                [](OptState& o, bool b) { o.s.randomizeMoveNames = b; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.movesetsMod = d.movesetsMod;
            o.s.randomizeMovePowers = d.randomizeMovePowers;
            o.s.randomizeMoveAccuracies = d.randomizeMoveAccuracies;
            o.s.randomizeMovePPs = d.randomizeMovePPs;
            o.s.randomizeMoveTypes = d.randomizeMoveTypes;
            o.s.randomizeMoveCategory = d.randomizeMoveCategory;
            o.s.startWithGuaranteedMoves = d.startWithGuaranteedMoves;
            o.s.guaranteedMoveCount = d.guaranteedMoveCount;
            o.s.reorderDamagingMoves = d.reorderDamagingMoves;
            o.s.blockBrokenMovesetMoves = d.blockBrokenMovesetMoves;
            o.s.blockBrokenTMMoves = d.blockBrokenTMMoves;
            o.s.blockBrokenTutorMoves = d.blockBrokenTutorMoves;
            o.s.randomizeMoveNames = d.randomizeMoveNames;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.4 Entrenadores ----------
    {
        TabDef tab;
        tab.title = "Entrenadores";
        tab.available = always;
        tab.mode = ModeDef{"EQUIPOS RIVALES", {"Sin cambios", "Aleatorios", "Mantener tipo del gimnasio", "Tema o tipo primario"},
                           [](const OptState& o) {
                               switch (o.s.trainersMod) {
                                   case S::TrainersMod::UNCHANGED: return 0;
                                   case S::TrainersMod::KEEP_THEMED: return 2;
                                   case S::TrainersMod::KEEP_THEME_OR_PRIMARY: return 3;
                                   default: return 1;
                               }
                           },
                           [](OptState& o, int i) {
                               static const S::TrainersMod m[] = {S::TrainersMod::UNCHANGED, S::TrainersMod::RANDOM,
                                                                  S::TrainersMod::KEEP_THEMED, S::TrainersMod::KEEP_THEME_OR_PRIMARY};
                               o.s.trainersMod = m[i];
                           }};
        tab.toggles = {
            tog("Usar solo Pokémon locales", [](const OptState& o) { return o.s.trainersUseLocalPokemon; },
                [](OptState& o, bool b) { o.s.trainersUseLocalPokemon = b; }, randomTrainers),
            tog("Liga con Pokémon únicos", [](const OptState& o) { return o.s.eliteFourUniquePokemonNumber > 0; },
                [](OptState& o, bool b) { o.s.eliteFourUniquePokemonNumber = b ? 1 : 0; }, randomTrainers),
            tog("Añadir Pokémon extra a jefes",
                [](const OptState& o) { return o.s.additionalBossTrainerPokemon > 0 || o.s.additionalImportantTrainerPokemon > 0; },
                [](OptState& o, bool b) { o.s.additionalBossTrainerPokemon = o.s.additionalImportantTrainerPokemon = b ? 1 : 0; }, on,
                [](const RomCaps& c) { return c.bossPokemon; }),
            tog("Combates dobles", [](const OptState& o) { return o.s.settingBattleStyle.isBattleStyleChanged(); },
                [](OptState& o, bool b) {
                    o.s.settingBattleStyle.mod = b ? pr::BattleStyle::Modification::SINGLE_STYLE : pr::BattleStyle::Modification::UNCHANGED;
                    o.s.settingBattleStyle.style = b ? pr::BattleStyle::Style::DOUBLE_BATTLE : pr::BattleStyle::Style::SINGLE_BATTLE;
                }),
            tog("Evolucionar sus Pokémon", [](const OptState& o) { return o.s.trainersEvolveTheirPokemon; },
                [](OptState& o, bool b) { o.s.trainersEvolveTheirPokemon = b; }),
            tog("Objetos equipados a jefes", [](const OptState& o) { return o.s.randomizeHeldItemsForBossTrainerPokemon; },
                [](OptState& o, bool b) {
                    o.s.randomizeHeldItemsForBossTrainerPokemon = b;
                    o.s.sensibleItemsOnlyForTrainerPokemon = b;
                },
                on, [](const RomCaps& c) { return c.bossItems; }),
            tog("Movimientos más listos en jefes",
                [](const OptState& o) { return o.s.betterBossTrainerMovesets; },
                [](OptState& o, bool b) { o.s.betterBossTrainerMovesets = o.s.betterImportantTrainerMovesets = b; }),
            tog("Nombres de entrenadores aleatorios", [](const OptState& o) { return o.s.randomizeTrainerNames; },
                [](OptState& o, bool b) { o.s.randomizeTrainerNames = b; }, on, [](const RomCaps& c) { return c.trainerText; }),
            tog("Rival con tus iniciales", [](const OptState& o) { return o.s.rivalCarriesStarterThroughout; },
                [](OptState& o, bool b) { o.s.rivalCarriesStarterThroughout = b; }, randomTrainers),
        };
        tab.slider = SliderDef{"Nivel de rivales", on, [](const OptState& o) { return o.s.trainersLevelModified ? o.s.trainersLevelModifier : 0; },
                               [](OptState& o, int v) {
                                   o.s.trainersLevelModified = v != 0;
                                   o.s.trainersLevelModifier = v;
                               }};
        tab.reset = [d](OptState& o) {
            o.s.trainersMod = d.trainersMod;
            o.s.trainersUseLocalPokemon = d.trainersUseLocalPokemon;
            o.s.eliteFourUniquePokemonNumber = d.eliteFourUniquePokemonNumber;
            o.s.additionalBossTrainerPokemon = d.additionalBossTrainerPokemon;
            o.s.additionalImportantTrainerPokemon = d.additionalImportantTrainerPokemon;
            o.s.settingBattleStyle = d.settingBattleStyle;
            o.s.trainersEvolveTheirPokemon = d.trainersEvolveTheirPokemon;
            o.s.randomizeHeldItemsForBossTrainerPokemon = d.randomizeHeldItemsForBossTrainerPokemon;
            o.s.sensibleItemsOnlyForTrainerPokemon = d.sensibleItemsOnlyForTrainerPokemon;
            o.s.betterBossTrainerMovesets = d.betterBossTrainerMovesets;
            o.s.betterImportantTrainerMovesets = d.betterImportantTrainerMovesets;
            o.s.randomizeTrainerNames = d.randomizeTrainerNames;
            o.s.rivalCarriesStarterThroughout = d.rivalCarriesStarterThroughout;
            o.s.trainersLevelModified = d.trainersLevelModified;
            o.s.trainersLevelModifier = d.trainersLevelModifier;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.5 Pokémon salvajes ----------
    {
        TabDef tab;
        tab.title = "Pokémon salvajes";
        tab.available = always;
        using Z = S::WildPokemonZoneMod;
        tab.mode = ModeDef{"MODO", {"Sin cambios", "Aleatorio", "Zona 1 a 1", "Global 1 a 1", "Familia a familia"},
                           [](const OptState& o) {
                               if (!o.s.randomizeWildPokemon) return 0;
                               switch (o.s.wildPokemonZoneMod) {
                                   case Z::NONE: return 1;
                                   case Z::GAME: return o.s.keepWildEvolutionFamilies ? 4 : 3;
                                   default: return 2;
                               }
                           },
                           [](OptState& o, int i) {
                               o.s.randomizeWildPokemon = i != 0;
                               o.s.wildPokemonZoneMod = i <= 1 ? Z::NONE : i == 2 ? Z::ENCOUNTER_SET : Z::GAME;
                               o.s.keepWildEvolutionFamilies = i == 4;
                               if (i >= 3) o.s.catchEmAllEncounters = false;  // FVX no lo permite en modo global
                           }};
        auto notGlobal = [](const OptState& o) { return o.s.randomizeWildPokemon && o.s.wildPokemonZoneMod != Z::GAME; };
        tab.toggles = {
            tog("Atrápalos a todos", [](const OptState& o) { return o.s.catchEmAllEncounters; },
                [](OptState& o, bool b) { o.s.catchEmAllEncounters = b; }, notGlobal),
            tog("Fuerza similar", [](const OptState& o) { return o.s.similarStrengthEncounters; },
                [](OptState& o, bool b) { o.s.similarStrengthEncounters = b; }, randomWild),
            tog("Mantener tipo primario", [](const OptState& o) { return o.s.wildPokemonTypeMod == S::WildPokemonTypeMod::KEEP_PRIMARY; },
                [](OptState& o, bool b) { o.s.wildPokemonTypeMod = b ? S::WildPokemonTypeMod::KEEP_PRIMARY : S::WildPokemonTypeMod::NONE; },
                randomWild),
            tog("Mantener tema de la zona", [](const OptState& o) { return o.s.keepWildTypeThemes; },
                [](OptState& o, bool b) { o.s.keepWildTypeThemes = b; }, randomWild),
            tog("Evitar legendarios", [](const OptState& o) { return o.s.blockWildLegendaries; },
                [](OptState& o, bool b) { o.s.blockWildLegendaries = b; }, randomWild),
            tog("Objetos equipados aleatorios", [](const OptState& o) { return o.s.randomizeWildPokemonHeldItems; },
                [](OptState& o, bool b) {
                    o.s.randomizeWildPokemonHeldItems = b;
                    o.s.banBadRandomWildPokemonHeldItems = b;
                }),
        };
        tab.slider = SliderDef{"Nivel de salvajes", on, [](const OptState& o) { return o.s.wildLevelsModified ? o.s.wildLevelModifier : 0; },
                               [](OptState& o, int v) {
                                   o.s.wildLevelsModified = v != 0;
                                   o.s.wildLevelModifier = v;
                               }};
        tab.reset = [d](OptState& o) {
            o.s.randomizeWildPokemon = d.randomizeWildPokemon;
            o.s.wildPokemonZoneMod = d.wildPokemonZoneMod;
            o.s.keepWildEvolutionFamilies = d.keepWildEvolutionFamilies;
            o.s.catchEmAllEncounters = d.catchEmAllEncounters;
            o.s.similarStrengthEncounters = d.similarStrengthEncounters;
            o.s.wildPokemonTypeMod = d.wildPokemonTypeMod;
            o.s.keepWildTypeThemes = d.keepWildTypeThemes;
            o.s.blockWildLegendaries = d.blockWildLegendaries;
            o.s.randomizeWildPokemonHeldItems = d.randomizeWildPokemonHeldItems;
            o.s.banBadRandomWildPokemonHeldItems = d.banBadRandomWildPokemonHeldItems;
            o.s.wildLevelsModified = d.wildLevelsModified;
            o.s.wildLevelModifier = d.wildLevelModifier;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.6 MT, MO y tutores ----------
    {
        TabDef tab;
        tab.title = "MT, MO y tutores";
        tab.available = always;
        using C = S::TMsHMsCompatibilityMod;
        using TC = S::MoveTutorsCompatibilityMod;
        tab.mode = ModeDef{"MOVIMIENTOS DE LAS MT", {"Sin cambios", "Aleatorios"},
                           [](const OptState& o) { return o.s.tmsMod == S::TMsMod::RANDOM ? 1 : 0; },
                           [](OptState& o, int i) { o.s.tmsMod = i ? S::TMsMod::RANDOM : S::TMsMod::UNCHANGED; }};
        tab.toggles = {
            tog("Compatibilidad de MT aleatoria",
                [](const OptState& o) { return o.s.tmsHmsCompatibilityMod == C::RANDOM_PREFER_TYPE || o.s.tmsHmsCompatibilityMod == C::COMPLETELY_RANDOM; },
                [](OptState& o, bool b) {
                    o.s.tmsHmsCompatibilityMod = b ? C::RANDOM_PREFER_TYPE : C::UNCHANGED;
                    o.s.moveTutorsCompatibilityMod = b ? TC::RANDOM_PREFER_TYPE : TC::UNCHANGED;
                }),
            tog("Tutores aleatorios", [](const OptState& o) { return o.s.moveTutorMovesMod == S::MoveTutorMovesMod::RANDOM; },
                [](OptState& o, bool b) { o.s.moveTutorMovesMod = b ? S::MoveTutorMovesMod::RANDOM : S::MoveTutorMovesMod::UNCHANGED; }, on,
                [](const RomCaps& c) { return c.tutors; }),
            tog("Compatibilidad total", [](const OptState& o) { return o.s.tmsHmsCompatibilityMod == C::FULL; },
                [](OptState& o, bool b) {
                    o.s.tmsHmsCompatibilityMod = b ? C::FULL : C::UNCHANGED;
                    o.s.moveTutorsCompatibilityMod = b ? TC::FULL : TC::UNCHANGED;
                }),
            tog("Mantener MT de movimientos de campo", [](const OptState& o) { return o.s.keepFieldMoveTMs; },
                [](OptState& o, bool b) { o.s.keepFieldMoveTMs = o.s.keepFieldMoveTutors = b; },
                [](const OptState& o) { return o.s.tmsMod == S::TMsMod::RANDOM; }),
            tweak("MT reutilizables", mt::REUSABLE_TMS),
            tweak("MO olvidables", mt::FORGETTABLE_HMS),
            tog("MO compatibles con todos", [](const OptState& o) { return o.s.fullHMCompat; },
                [](OptState& o, bool b) { o.s.fullHMCompat = b; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.tmsMod = d.tmsMod;
            o.s.tmsHmsCompatibilityMod = d.tmsHmsCompatibilityMod;
            o.s.moveTutorsCompatibilityMod = d.moveTutorsCompatibilityMod;
            o.s.moveTutorMovesMod = d.moveTutorMovesMod;
            o.s.keepFieldMoveTMs = d.keepFieldMoveTMs;
            o.s.keepFieldMoveTutors = d.keepFieldMoveTutors;
            o.s.fullHMCompat = d.fullHMCompat;
            int mask = mt::REUSABLE_TMS.value | mt::FORGETTABLE_HMS.value;
            o.s.currentMiscTweaks = (o.s.currentMiscTweaks & ~mask) | (d.currentMiscTweaks & mask);
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.7 Objetos ----------
    {
        TabDef tab;
        tab.title = "Objetos";
        tab.available = always;
        tab.mode = ModeDef{"OBJETOS DEL SUELO", {"Sin cambios", "Mezclar", "Aleatorios"},
                           [](const OptState& o) {
                               switch (o.s.fieldItemsMod) {
                                   case S::FieldItemsMod::SHUFFLE: return 1;
                                   case S::FieldItemsMod::RANDOM:
                                   case S::FieldItemsMod::RANDOM_EVEN: return 2;
                                   default: return 0;
                               }
                           },
                           [](OptState& o, int i) {
                               static const S::FieldItemsMod m[] = {S::FieldItemsMod::UNCHANGED, S::FieldItemsMod::SHUFFLE, S::FieldItemsMod::RANDOM};
                               o.s.fieldItemsMod = m[i];
                           }};
        tab.toggles = {
            tog("Tiendas aleatorias", [](const OptState& o) { return o.s.shopItemsMod == S::ShopItemsMod::RANDOM; },
                [](OptState& o, bool b) { o.s.shopItemsMod = b ? S::ShopItemsMod::RANDOM : S::ShopItemsMod::UNCHANGED; }, on,
                [](const RomCaps& c) { return c.shops; }),
            tog("Caramelos raros a 10 ₽", [](const OptState& o) { return o.s.addCheapRareCandiesToShops; },
                [](OptState& o, bool b) { o.s.addCheapRareCandiesToShops = b; }, on, [](const RomCaps& c) { return c.shops; }),
            tog("Objetos de Recogida aleatorios", [](const OptState& o) { return o.s.pickupItemsMod == S::PickupItemsMod::RANDOM; },
                [](OptState& o, bool b) { o.s.pickupItemsMod = b ? S::PickupItemsMod::RANDOM : S::PickupItemsMod::UNCHANGED; }, on,
                [](const RomCaps& c) { return c.abilities; }),
            tog("Prohibir objetos inútiles", [](const OptState& o) { return o.s.banBadRandomFieldItems; },
                [](OptState& o, bool b) { o.s.banBadRandomFieldItems = o.s.banBadRandomShopItems = o.s.banBadRandomPickupItems = b; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.fieldItemsMod = d.fieldItemsMod;
            o.s.shopItemsMod = d.shopItemsMod;
            o.s.addCheapRareCandiesToShops = d.addCheapRareCandiesToShops;
            o.s.pickupItemsMod = d.pickupItemsMod;
            o.s.banBadRandomFieldItems = d.banBadRandomFieldItems;
            o.s.banBadRandomShopItems = d.banBadRandomShopItems;
            o.s.banBadRandomPickupItems = d.banBadRandomPickupItems;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.8 Tipos ----------
    {
        TabDef tab;
        tab.title = "Tipos";
        tab.available = [](const RomCaps& c) { return c.typeTable; };
        tab.mode = ModeDef{"TABLA DE TIPOS", {"Sin cambios", "Aleatoria", "Equilibrada", "Mantener identidad", "Inversa"},
                           [](const OptState& o) { return static_cast<int>(o.s.typeEffectivenessMod); },
                           [](OptState& o, int i) { o.s.typeEffectivenessMod = static_cast<S::TypeEffectivenessMod>(i); }};
        tab.toggles = {
            tog("Actualizar efectividades a Gen moderna", [](const OptState& o) { return o.s.updateTypeEffectiveness; },
                [](OptState& o, bool b) { o.s.updateTypeEffectiveness = b; }),
            tog("Inversa con inmunidades aleatorias", [](const OptState& o) { return o.s.inverseTypesRandomImmunities; },
                [](OptState& o, bool b) { o.s.inverseTypesRandomImmunities = b; },
                [](const OptState& o) { return o.s.typeEffectivenessMod == S::TypeEffectivenessMod::INVERSE; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.typeEffectivenessMod = d.typeEffectivenessMod;
            o.s.updateTypeEffectiveness = d.updateTypeEffectiveness;
            o.s.inverseTypesRandomImmunities = d.inverseTypesRandomImmunities;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.9 Gráficos ----------
    {
        TabDef tab;
        tab.title = "Gráficos";
        tab.available = [](const RomCaps& c) { return c.palettes; };
        tab.mode = ModeDef{"COLORES DE LOS POKÉMON", {"Sin cambios", "Por tipo", "Por evolución", "Totalmente aleatorios"},
                           [](const OptState& o) {
                               if (o.s.pokemonPalettesMod != S::PokemonPalettesMod::RANDOM) return 0;
                               if (o.s.pokemonPalettesFollowTypes) return 1;
                               if (o.s.pokemonPalettesFollowEvolutions) return 2;
                               return 3;
                           },
                           [](OptState& o, int i) {
                               o.s.pokemonPalettesMod = i ? S::PokemonPalettesMod::RANDOM : S::PokemonPalettesMod::UNCHANGED;
                               o.s.pokemonPalettesFollowTypes = i == 1;
                               o.s.pokemonPalettesFollowEvolutions = i == 1 || i == 2;
                           }};
        tab.toggles = {
            tog("Shiny a partir del color normal", [](const OptState& o) { return o.s.pokemonPalettesShinyFromNormal; },
                [](OptState& o, bool b) { o.s.pokemonPalettesShinyFromNormal = b; },
                [](const OptState& o) { return o.s.pokemonPalettesMod == S::PokemonPalettesMod::RANDOM; }),
        };
        tab.reset = [d](OptState& o) {
            o.s.pokemonPalettesMod = d.pokemonPalettesMod;
            o.s.pokemonPalettesFollowTypes = d.pokemonPalettesFollowTypes;
            o.s.pokemonPalettesFollowEvolutions = d.pokemonPalettesFollowEvolutions;
            o.s.pokemonPalettesShinyFromNormal = d.pokemonPalettesShinyFromNormal;
        };
        t.push_back(std::move(tab));
    }

    // ---------- 7.10 Extras ----------
    {
        TabDef tab;
        tab.title = "Extras";
        tab.exclusive = true;
        tab.available = always;
        tab.shinyPanel = true;
        tab.toggles = {
            tweak("Texto rápido", mt::FASTEST_TEXT),
            tweak("Correr en interiores", mt::RUNNING_SHOES_INDOORS),
            tog("Evoluciones imposibles por nivel", [](const OptState& o) { return o.s.changeImpossibleEvolutions; },
                [](OptState& o, bool b) { o.s.changeImpossibleEvolutions = b; }),
            tog("Generar log de spoilers", [](const OptState& o) { return o.spoilerLog; }, [](OptState& o, bool b) { o.spoilerLog = b; }),
        };
        tab.reset = [d](OptState& o) {
            int mask = mt::FASTEST_TEXT.value | mt::RUNNING_SHOES_INDOORS.value;
            o.s.currentMiscTweaks = (o.s.currentMiscTweaks & ~mask) | (d.currentMiscTweaks & mask);
            o.s.changeImpossibleEvolutions = d.changeImpossibleEvolutions;
            o.shiny = 0;
            o.spoilerLog = true;
        };
        t.push_back(std::move(tab));
    }
    return t;
}
}  // namespace

const std::vector<TabDef>& tabs() {
    static const std::vector<TabDef> t = build();
    return t;
}

}  // namespace app
