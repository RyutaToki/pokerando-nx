#!/usr/bin/env python3
"""Prueba de estrés: combinaciones aleatorias de opciones, comparando la ROM de FVX y la del núcleo C++.
Uso: tests/fuzz_rand.py <rom> <número de pruebas> [semilla del generador]"""
import random, subprocess, sys

BOOLS = """changeImpossibleEvolutions estimateLevelForEvolutionImprovements makeEvolutionsEasier removeTimeBasedEvolutions
raceMode randomizeIntroMon limitPokemon banPrematureEvos dualTypeOnly bstFollowEvolutions bstShuffleSwapLegendaries
baseStatsFollowEvolutions assignEvoStatsRandomly updateBaseStats standardizeEXPCurves allowWonderGuard
abilitiesFollowEvolutions banTrappingAbilities banNegativeAbilities banBadAbilities weighDuplicateAbilitiesTogether
ensureTwoAbilities startersNoDualTypes startersNoLegendaries randomizeStartersHeldItems limitMainGameLegendaries limit600
banBadRandomStarterHeldItems evosSimilarStrength evosSameTyping evosMaxThreeStages evosForceChange evosForceGrowth
evosNoConvergence adjustEvolutionLevels randomizeMovePowers randomizeMoveAccuracies randomizeMovePPs randomizeMoveTypes
randomizeMoveNames updateMoves startWithGuaranteedMoves reorderDamagingMoves movesetsForceGoodDamaging
blockBrokenMovesetMoves evolutionMovesForAll rivalCarriesStarterThroughout trainersUsePokemonOfSimilarStrength
trainersAvoidDuplicates trainersMatchTypingDistribution trainersBlockLegendaries trainersUseLocalPokemon
trainersBlockEarlyWonderGuard trainersEnforceDistribution trainersEnforceMainPlaythrough randomizeTrainerNames
randomizeTrainerClassNames trainersEvolveTheirPokemon trainersLevelModified randomizeHeldItemsForBossTrainerPokemon
randomizeHeldItemsForImportantTrainerPokemon randomizeHeldItemsForRegularTrainerPokemon
consumableItemsOnlyForTrainerPokemon sensibleItemsOnlyForTrainerPokemon highestLevelOnlyGetsItemsForTrainerPokemon
diverseTypesForBossTrainers diverseTypesForImportantTrainers diverseTypesForRegularTrainers betterBossTrainerMovesets
betterImportantTrainerMovesets betterRegularTrainerMovesets randomizeWildPokemon splitWildZoneByEncounterTypes
keepWildTypeThemes keepWildEvolutionFamilies similarStrengthEncounters catchEmAllEncounters useTimeBasedEncounters
blockWildLegendaries useMinimumCatchRate randomizeWildPokemonHeldItems banBadRandomWildPokemonHeldItems
balanceShakingGrass wildLevelsModified staticLevelModified correctStaticMusic tmLevelUpMoveSanity keepFieldMoveTMs
fullHMCompat tmsForceGoodDamaging blockBrokenTMMoves tmsFollowEvolutions tutorLevelUpMoveSanity keepFieldMoveTutors
tutorsForceGoodDamaging blockBrokenTutorMoves tutorFollowEvolutions randomizeInGameTradesNicknames
randomizeInGameTradesOTs randomizeInGameTradesIVs randomizeInGameTradesItems banBadRandomFieldItems
banBadRandomShopItems banRegularShopItems banOPShopItems guaranteeEvolutionItems guaranteeXItems balanceShopPrices
addCheapRareCandiesToShops banBadRandomPickupItems inverseTypesRandomImmunities updateTypeEffectiveness
pokemonPalettesFollowTypes pokemonPalettesFollowEvolutions pokemonPalettesShinyFromNormal""".split()

ENUMS = {
    "bstMod": "UNCHANGED RANDOM_BUFF_NERF SHUFFLE RANDOM", "baseStatisticsMod": "UNCHANGED SHUFFLE RANDOM",
    "expCurveMod": "LEGENDARIES STRONG_LEGENDARIES ALL", "abilitiesMod": "UNCHANGED RANDOMIZE",
    "startersMod": "UNCHANGED COMPLETELY_RANDOM RANDOM_WITH_TWO_EVOLUTIONS RANDOM_BASIC",
    "startersTypeMod": "NONE FIRE_WATER_GRASS UNIQUE SINGLE_TYPE",  # TRIANGLE: FVX depende de hashes de identidad
    "speciesTypesMod": "UNCHANGED RANDOM_FOLLOW_EVOLUTIONS COMPLETELY_RANDOM",
    "evolutionsMod": "UNCHANGED RANDOM RANDOM_EVERY_LEVEL",
    "movesetsMod": "UNCHANGED RANDOM_PREFER_SAME_TYPE COMPLETELY_RANDOM METRONOME_ONLY",
    "trainersMod": "UNCHANGED RANDOM DISTRIBUTED MAINPLAYTHROUGH TYPE_THEMED TYPE_THEMED_ELITE4_GYMS KEEP_THEMED KEEP_THEME_OR_PRIMARY",
    "wildPokemonZoneMod": "NONE ENCOUNTER_SET MAP NAMED_LOCATION GAME", "wildPokemonTypeMod": "NONE RANDOM_THEMES KEEP_PRIMARY",
    "wildPokemonEvolutionMod": "NONE BASIC_ONLY KEEP_STAGE",
    "staticPokemonMod": "UNCHANGED RANDOM_MATCHING COMPLETELY_RANDOM SIMILAR_STRENGTH", "tmsMod": "UNCHANGED RANDOM",
    "tmsHmsCompatibilityMod": "UNCHANGED RANDOM_PREFER_TYPE COMPLETELY_RANDOM FULL", "moveTutorMovesMod": "UNCHANGED RANDOM",
    "moveTutorsCompatibilityMod": "UNCHANGED RANDOM_PREFER_TYPE COMPLETELY_RANDOM FULL",
    "inGameTradesMod": "UNCHANGED RANDOMIZE_GIVEN RANDOMIZE_GIVEN_AND_REQUESTED",
    "fieldItemsMod": "UNCHANGED SHUFFLE RANDOM RANDOM_EVEN", "shopItemsMod": "UNCHANGED SHUFFLE RANDOM",
    "pickupItemsMod": "UNCHANGED RANDOM", "typeEffectivenessMod": "UNCHANGED RANDOM RANDOM_BALANCED KEEP_IDENTITIES INVERSE",
    "pokemonPalettesMod": "UNCHANGED RANDOM",
}
INTS = {
    "makeEvolutionsEasierLvl": (20, 60), "bstBuffNerfMaxPercentage": (0, 100), "updateBaseStatsToGeneration": (6, 9),
    "updateMovesToGeneration": (4, 9), "guaranteedMoveCount": (2, 4), "movesetsGoodDamagingPercent": (0, 100),
    "trainersLevelModifier": (-50, 50), "eliteFourUniquePokemonNumber": (0, 2), "additionalBossTrainerPokemon": (0, 5),
    "additionalImportantTrainerPokemon": (0, 5), "additionalRegularTrainerPokemon": (0, 5),
    "minimumCatchRateLevel": (1, 5), "wildLevelModifier": (-50, 50), "staticLevelModifier": (-50, 50),
    "tmsGoodDamagingPercent": (0, 100), "tutorsGoodDamagingPercent": (0, 100),
}

rom, n = sys.argv[1], int(sys.argv[2])
if rom.lower().endswith(".nds"):
    # Gen 4: FVX ordena las bayas de los objetos «sensatos» con un HashMap<Type> (hash de identidad,
    # irreproducible), así que esa opción no puede dar la misma ROM
    BOOLS.remove("sensibleItemsOnlyForTrainerPokemon")
    # Igual al separar las zonas por tipo de encuentro: FVX agrupa con un HashMap<EncounterType> y el orden
    # depende del hash de identidad (cambia con la ROM y las opciones). En Gen 5 cada mapa tiene varios tipos.
    BOOLS.remove("splitWildZoneByEncounterTypes")
gen = random.Random(int(sys.argv[3]) if len(sys.argv) > 3 else None)
stats = {"OK": 0, "AMBOS": 0, "MAL": 0}
for k in range(n):
    args = [f"{b}=1" for b in BOOLS if gen.random() < 0.2]
    args += [f"{e}={gen.choice(v.split())}" for e, v in ENUMS.items() if gen.random() < 0.6]
    args += [f"{i}={gen.randint(*r)}" for i, r in INTS.items() if gen.random() < 0.5]
    seed = gen.randint(0, 2**31)
    try:
        r = subprocess.run(["tests/compare_rand.sh", rom, str(seed)] + args, capture_output=True, text=True, timeout=900)
    except subprocess.TimeoutExpired:
        subprocess.run(["pkill", "-f", f"Randomize .* {seed} "])  # FVX colgado
        print(f"[{k + 1}/{n}] TIEMPO AGOTADO (semilla {seed}) {' '.join(args)}", flush=True)
        stats["MAL"] += 1
        continue
    line = (r.stdout.strip().splitlines() or [r.stderr.strip()[-300:]])[-1]
    if r.returncode == 0:
        stats["OK"] += 1
        print(f"[{k + 1}/{n}] OK semilla {seed}", flush=True)
    elif r.returncode == 2:
        stats["AMBOS"] += 1
        print(f"[{k + 1}/{n}] ambos fallan igual (semilla {seed}): {line.split(': ', 1)[-1][:200]}", flush=True)
    else:
        stats["MAL"] += 1
        print(f"[{k + 1}/{n}] {line}", flush=True)
print(stats)
