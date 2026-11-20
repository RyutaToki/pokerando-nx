#include "settings/settings.hpp"

#include <algorithm>
#include <stdexcept>

#include "romio/rom_handler.hpp"
#include "util/crc32.hpp"

namespace pr {

namespace {

using BSTMod = Settings::BSTMod;
using BaseStatisticsMod = Settings::BaseStatisticsMod;
using ExpCurveMod = Settings::ExpCurveMod;
using AbilitiesMod = Settings::AbilitiesMod;
using StartersMod = Settings::StartersMod;
using StartersTypeMod = Settings::StartersTypeMod;
using SpeciesTypesMod = Settings::SpeciesTypesMod;
using EvolutionsMod = Settings::EvolutionsMod;
using MovesetsMod = Settings::MovesetsMod;
using TrainersMod = Settings::TrainersMod;
using WildPokemonZoneMod = Settings::WildPokemonZoneMod;
using WildPokemonTypeMod = Settings::WildPokemonTypeMod;
using WildPokemonEvolutionMod = Settings::WildPokemonEvolutionMod;
using StaticPokemonMod = Settings::StaticPokemonMod;
using TotemPokemonMod = Settings::TotemPokemonMod;
using AllyPokemonMod = Settings::AllyPokemonMod;
using AuraMod = Settings::AuraMod;
using TMsMod = Settings::TMsMod;
using TMsHMsCompatibilityMod = Settings::TMsHMsCompatibilityMod;
using MoveTutorMovesMod = Settings::MoveTutorMovesMod;
using MoveTutorsCompatibilityMod = Settings::MoveTutorsCompatibilityMod;
using InGameTradesMod = Settings::InGameTradesMod;
using FieldItemsMod = Settings::FieldItemsMod;
using ShopItemsMod = Settings::ShopItemsMod;
using PickupItemsMod = Settings::PickupItemsMod;
using TypeEffectivenessMod = Settings::TypeEffectivenessMod;
using PokemonPalettesMod = Settings::PokemonPalettesMod;
constexpr int LENGTH_OF_SETTINGS_DATA = Settings::LENGTH_OF_SETTINGS_DATA;

int makeByteSelected(std::initializer_list<bool> bools) {
    if (bools.size() > 8) throw std::invalid_argument("Can't set more than 8 bits in a byte!");
    int v = 0, state = 1;
    for (bool b : bools) {
        v |= b ? state : 0;
        state *= 2;
    }
    return v;
}

bool restoreState(int8_t b, int index) {
    if (index >= 8) throw std::invalid_argument("Can't read more than 8 bits from a byte!");
    return ((static_cast<uint8_t>(b) >> index) & 1) == 1;
}

// restoreEnum: el i-ésimo bit indicado corresponde al i-ésimo valor del enum
template <class E>
E restoreEnum(int8_t b, std::initializer_list<int> indices) {
    int index = -1, i = 0;
    for (int idx : indices) {
        if (restoreState(b, idx)) {
            if (index >= 0) throw std::logic_error("Only one value may be chosen!");
            index = i;
        }
        i++;
    }
    return static_cast<E>(index >= 0 ? index : 0);
}

void writeFullInt(std::vector<uint8_t>& out, int v) {
    for (int i = 0; i < 4; i++) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
}
void writeFullIntBigEndian(std::vector<uint8_t>& out, int v) {
    for (int i = 3; i >= 0; i--) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
}
void write2ByteIntBigEndian(std::vector<uint8_t>& out, int v) {  // (sic) en FVX es little endian
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}
int read2ByteInt(const std::vector<int8_t>& d, int i) {
    return static_cast<uint8_t>(d.at(i)) | (static_cast<uint8_t>(d.at(i + 1)) << 8);
}
int readFullInt(const std::vector<int8_t>& d, int i) {
    return static_cast<int>(static_cast<uint32_t>(static_cast<uint8_t>(d.at(i))) |
                            (static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 1))) << 8) |
                            (static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 2))) << 16) |
                            (static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 3))) << 24));
}
int readFullIntBigEndian(const std::vector<int8_t>& d, int i) {
    return static_cast<int>((static_cast<uint32_t>(static_cast<uint8_t>(d.at(i))) << 24) |
                            (static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 1))) << 16) |
                            (static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 2))) << 8) |
                            static_cast<uint32_t>(static_cast<uint8_t>(d.at(i + 3))));
}
std::optional<Type> typeFromInt(int i) {
    if (i < 0 || i > static_cast<int>(Type::FAIRY)) return std::nullopt;
    return static_cast<Type>(i);
}

constexpr char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const std::vector<uint8_t>& d) {
    std::string out;
    size_t i = 0;
    for (; i + 2 < d.size(); i += 3) {
        uint32_t n = (d[i] << 16) | (d[i + 1] << 8) | d[i + 2];
        out += kB64[(n >> 18) & 63];
        out += kB64[(n >> 12) & 63];
        out += kB64[(n >> 6) & 63];
        out += kB64[n & 63];
    }
    if (i + 1 == d.size()) {
        uint32_t n = d[i] << 16;
        out += kB64[(n >> 18) & 63];
        out += kB64[(n >> 12) & 63];
        out += "==";
    } else if (i + 2 == d.size()) {
        uint32_t n = (d[i] << 16) | (d[i + 1] << 8);
        out += kB64[(n >> 18) & 63];
        out += kB64[(n >> 12) & 63];
        out += kB64[(n >> 6) & 63];
        out += '=';
    }
    return out;
}

std::vector<int8_t> base64Decode(const std::string& s) {
    auto val = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    std::vector<int8_t> out;
    uint32_t buf = 0;
    int bits = 0;
    for (char c : s) {
        if (c == '=') break;
        int v = val(c);
        if (v < 0) throw std::invalid_argument("Illegal base64 character");
        buf = (buf << 6) | static_cast<uint32_t>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<int8_t>((buf >> bits) & 0xFF));
        }
    }
    return out;
}

bool hasInvalidChecksum(const std::vector<int8_t>& data) {
    int n = static_cast<int>(data.size()) - Settings::LENGTH_OF_CHECKSUM - Settings::LENGTH_OF_END_PADDING;
    if (n < 0) return true;
    int crc = readFullIntBigEndian(data, n);
    return static_cast<int>(crc32(reinterpret_cast<const uint8_t*>(data.data()), n)) != crc;
}

}  // namespace

std::string Settings::toString() const { return std::to_string(VERSION) + toStringWithoutVersion(); }

Settings Settings::fromString(const std::string& withVersion) {
    if (withVersion.size() < 3) throw std::invalid_argument("settings string demasiado corta");
    int version = std::stoi(withVersion.substr(0, 3));
    if (version != VERSION) {
        // TODO(port): SettingsUpdater para versiones antiguas de FVX
        throw std::invalid_argument("Esta settings string es de otra versión de FVX (" + std::to_string(version) +
                                    "); solo se admite la " + std::to_string(VERSION));
    }
    return fromStringWithoutVersion(withVersion.substr(3));
}

std::string Settings::toStringWithoutVersion() const {
    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(makeByteSelected({changeImpossibleEvolutions, updateMoves, updateMovesLegacy, randomizeTrainerNames, randomizeTrainerClassNames, makeEvolutionsEasier, removeTimeBasedEvolutions, estimateLevelForEvolutionImprovements})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({baseStatsFollowEvolutions, baseStatisticsMod == BaseStatisticsMod::RANDOM, baseStatisticsMod == BaseStatisticsMod::SHUFFLE, baseStatisticsMod == BaseStatisticsMod::UNCHANGED, standardizeEXPCurves, updateBaseStats, baseStatsFollowMegaEvolutions, assignEvoStatsRandomly})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({speciesTypesMod == SpeciesTypesMod::RANDOM_FOLLOW_EVOLUTIONS, speciesTypesMod == SpeciesTypesMod::COMPLETELY_RANDOM, speciesTypesMod == SpeciesTypesMod::UNCHANGED, false, false, false, typesFollowMegaEvolutions, dualTypeOnly})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({abilitiesMod == AbilitiesMod::UNCHANGED, abilitiesMod == AbilitiesMod::RANDOMIZE, allowWonderGuard, abilitiesFollowEvolutions, banTrappingAbilities, banNegativeAbilities, banBadAbilities, abilitiesFollowMegaEvolutions})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({startersMod == StartersMod::CUSTOM, startersMod == StartersMod::COMPLETELY_RANDOM, startersMod == StartersMod::UNCHANGED, startersMod == StartersMod::RANDOM_WITH_TWO_EVOLUTIONS, randomizeStartersHeldItems, banBadRandomStarterHeldItems, allowStarterAltFormes, startersMod == StartersMod::RANDOM_BASIC})));
    write2ByteIntBigEndian(out, customStarters[0]);
    write2ByteIntBigEndian(out, customStarters[1]);
    write2ByteIntBigEndian(out, customStarters[2]);
    out.push_back(static_cast<uint8_t>(makeByteSelected({movesetsMod == MovesetsMod::COMPLETELY_RANDOM, movesetsMod == MovesetsMod::RANDOM_PREFER_SAME_TYPE, movesetsMod == MovesetsMod::UNCHANGED, movesetsMod == MovesetsMod::METRONOME_ONLY, startWithGuaranteedMoves, reorderDamagingMoves}) | ((guaranteedMoveCount - 2) << 6)));
    out.push_back(static_cast<uint8_t>((movesetsForceGoodDamaging ? 0x80 : 0) | movesetsGoodDamagingPercent));
    out.push_back(static_cast<uint8_t>(makeByteSelected({trainersMod == TrainersMod::UNCHANGED, trainersMod == TrainersMod::RANDOM, trainersMod == TrainersMod::DISTRIBUTED, trainersMod == TrainersMod::MAINPLAYTHROUGH, trainersMod == TrainersMod::TYPE_THEMED, trainersMod == TrainersMod::TYPE_THEMED_ELITE4_GYMS, trainersMod == TrainersMod::KEEP_THEMED, trainersMod == TrainersMod::KEEP_THEME_OR_PRIMARY})));
    out.push_back(static_cast<uint8_t>(trainersEvolutionLevelModifier - 28));
    out.push_back(static_cast<uint8_t>(makeByteSelected({!randomizeWildPokemon, wildPokemonZoneMod == WildPokemonZoneMod::NONE, wildPokemonZoneMod == WildPokemonZoneMod::ENCOUNTER_SET, wildPokemonZoneMod == WildPokemonZoneMod::GAME, keepWildEvolutionFamilies, wildPokemonZoneMod == WildPokemonZoneMod::NAMED_LOCATION, wildPokemonZoneMod == WildPokemonZoneMod::MAP, splitWildZoneByEncounterTypes})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({false, similarStrengthEncounters, catchEmAllEncounters, false, false, false, false, false})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({wildPokemonTypeMod == WildPokemonTypeMod::NONE, wildPokemonTypeMod == WildPokemonTypeMod::KEEP_PRIMARY, wildPokemonTypeMod == WildPokemonTypeMod::RANDOM_THEMES, keepWildTypeThemes, wildPokemonEvolutionMod == WildPokemonEvolutionMod::NONE, wildPokemonEvolutionMod == WildPokemonEvolutionMod::BASIC_ONLY, wildPokemonEvolutionMod == WildPokemonEvolutionMod::KEEP_STAGE, false})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({useTimeBasedEncounters, useMinimumCatchRate, blockWildLegendaries, randomizeWildPokemonHeldItems, banBadRandomWildPokemonHeldItems, balanceShakingGrass, false, false})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({staticPokemonMod == StaticPokemonMod::UNCHANGED, staticPokemonMod == StaticPokemonMod::RANDOM_MATCHING, staticPokemonMod == StaticPokemonMod::COMPLETELY_RANDOM, staticPokemonMod == StaticPokemonMod::SIMILAR_STRENGTH, limitMainGameLegendaries, limit600, allowStaticAltFormes, swapStaticMegaEvos})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({tmsHmsCompatibilityMod == TMsHMsCompatibilityMod::COMPLETELY_RANDOM, tmsHmsCompatibilityMod == TMsHMsCompatibilityMod::RANDOM_PREFER_TYPE, tmsHmsCompatibilityMod == TMsHMsCompatibilityMod::UNCHANGED, tmsMod == TMsMod::RANDOM, tmsMod == TMsMod::UNCHANGED, tmLevelUpMoveSanity, keepFieldMoveTMs, tmsHmsCompatibilityMod == TMsHMsCompatibilityMod::FULL})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({fullHMCompat, tmsFollowEvolutions, tutorFollowEvolutions})));
    out.push_back(static_cast<uint8_t>((tmsForceGoodDamaging ? 0x80 : 0) | tmsGoodDamagingPercent));
    out.push_back(static_cast<uint8_t>(makeByteSelected({moveTutorsCompatibilityMod == MoveTutorsCompatibilityMod::COMPLETELY_RANDOM, moveTutorsCompatibilityMod == MoveTutorsCompatibilityMod::RANDOM_PREFER_TYPE, moveTutorsCompatibilityMod == MoveTutorsCompatibilityMod::UNCHANGED, moveTutorMovesMod == MoveTutorMovesMod::RANDOM, moveTutorMovesMod == MoveTutorMovesMod::UNCHANGED, tutorLevelUpMoveSanity, keepFieldMoveTutors, moveTutorsCompatibilityMod == MoveTutorsCompatibilityMod::FULL})));
    out.push_back(static_cast<uint8_t>((tutorsForceGoodDamaging ? 0x80 : 0) | tutorsGoodDamagingPercent));
    out.push_back(static_cast<uint8_t>(makeByteSelected({inGameTradesMod == InGameTradesMod::RANDOMIZE_GIVEN_AND_REQUESTED, inGameTradesMod == InGameTradesMod::RANDOMIZE_GIVEN, randomizeInGameTradesItems, randomizeInGameTradesIVs, randomizeInGameTradesNicknames, randomizeInGameTradesOTs, inGameTradesMod == InGameTradesMod::UNCHANGED})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({fieldItemsMod == FieldItemsMod::RANDOM, fieldItemsMod == FieldItemsMod::SHUFFLE, fieldItemsMod == FieldItemsMod::UNCHANGED, banBadRandomFieldItems, fieldItemsMod == FieldItemsMod::RANDOM_EVEN})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({randomizeMovePowers, randomizeMoveAccuracies, randomizeMovePPs, randomizeMoveTypes, randomizeMoveCategory, correctStaticMusic, randomizeMoveNames})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({evolutionsMod == EvolutionsMod::UNCHANGED, evolutionsMod == EvolutionsMod::RANDOM, evosSimilarStrength, evosSameTyping, evosMaxThreeStages, evosForceChange, evosAllowAltFormes, evolutionsMod == EvolutionsMod::RANDOM_EVERY_LEVEL})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({trainersUsePokemonOfSimilarStrength, rivalCarriesStarterThroughout, trainersMatchTypingDistribution, trainersBlockLegendaries, trainersBlockEarlyWonderGuard, swapTrainerMegaEvos, shinyChance, trainersAvoidDuplicates})));
    int restrictionsVal = !currentRestrictions ? -1 : currentRestrictions->toInt();
    writeFullInt(out, restrictionsVal);
    writeFullIntBigEndian(out, currentMiscTweaks);
    out.push_back(static_cast<uint8_t>(trainersLevelModifier - 28));
    out.push_back(static_cast<uint8_t>(makeByteSelected({shopItemsMod == ShopItemsMod::RANDOM, shopItemsMod == ShopItemsMod::SHUFFLE, shopItemsMod == ShopItemsMod::UNCHANGED, banBadRandomShopItems, banRegularShopItems, banOPShopItems, false, guaranteeEvolutionItems})));
    out.push_back(static_cast<uint8_t>(wildLevelModifier - 28));
    out.push_back(static_cast<uint8_t>(makeByteSelected({ expCurveMod == ExpCurveMod::LEGENDARIES, expCurveMod == ExpCurveMod::STRONG_LEGENDARIES, expCurveMod == ExpCurveMod::ALL, blockBrokenMovesetMoves, blockBrokenTMMoves, blockBrokenTutorMoves, allowTrainerAlternateFormes, allowWildAltFormes})));
    out.push_back(static_cast<uint8_t>((0) | (additionalBossTrainerPokemon << 1) | (additionalImportantTrainerPokemon << 4) | (weighDuplicateAbilitiesTogether ? 0x80 : 0)));
    out.push_back(static_cast<uint8_t>(additionalRegularTrainerPokemon | ((auraMod == AuraMod::UNCHANGED) ? 0x8 : 0) | ((auraMod == AuraMod::RANDOM) ? 0x10 : 0) | ((auraMod == AuraMod::SAME_STRENGTH) ? 0x20 : 0) | (evolutionMovesForAll ? 0x40 : 0) | (guaranteeXItems ? 0x80 : 0)));
    out.push_back(static_cast<uint8_t>(makeByteSelected({ totemPokemonMod == TotemPokemonMod::UNCHANGED, totemPokemonMod == TotemPokemonMod::RANDOM, totemPokemonMod == TotemPokemonMod::SIMILAR_STRENGTH, allyPokemonMod == AllyPokemonMod::UNCHANGED, allyPokemonMod == AllyPokemonMod::RANDOM, allyPokemonMod == AllyPokemonMod::SIMILAR_STRENGTH, randomizeTotemHeldItems, allowTotemAltFormes})));
    out.push_back(static_cast<uint8_t>(totemLevelModifier - 28));
    out.push_back(static_cast<uint8_t>(updateBaseStatsToGeneration));
    out.push_back(static_cast<uint8_t>(updateMovesToGeneration));
    out.push_back(static_cast<uint8_t>(expCurveToByte(selectedEXPCurve.value())));
    out.push_back(static_cast<uint8_t>(staticLevelModifier - 28));
    out.push_back(static_cast<uint8_t>(makeByteSelected({randomizeHeldItemsForBossTrainerPokemon, randomizeHeldItemsForImportantTrainerPokemon, randomizeHeldItemsForRegularTrainerPokemon, consumableItemsOnlyForTrainerPokemon, sensibleItemsOnlyForTrainerPokemon, highestLevelOnlyGetsItemsForTrainerPokemon, ensureTwoAbilities, trainersUseLocalPokemon})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({pickupItemsMod == PickupItemsMod::RANDOM, pickupItemsMod == PickupItemsMod::UNCHANGED, banBadRandomPickupItems, banIrregularAltFormes})));
    out.push_back(static_cast<uint8_t>(eliteFourUniquePokemonNumber | ((minimumCatchRateLevel - 1) << 3)));
    out.push_back(static_cast<uint8_t>(makeByteSelected({startersTypeMod == StartersTypeMod::NONE, startersTypeMod == StartersTypeMod::FIRE_WATER_GRASS, startersTypeMod == StartersTypeMod::TRIANGLE, startersTypeMod == StartersTypeMod::UNIQUE, startersTypeMod == StartersTypeMod::SINGLE_TYPE, false, startersNoLegendaries, startersNoDualTypes})));
    if (startersSingleType) {
    out.push_back(static_cast<uint8_t>(static_cast<int>(*startersSingleType) + 1));
    }
    else {
    out.push_back(static_cast<uint8_t>(0));
    }
    out.push_back(static_cast<uint8_t>(makeByteSelected({pokemonPalettesMod == PokemonPalettesMod::UNCHANGED, pokemonPalettesMod == PokemonPalettesMod::RANDOM, pokemonPalettesFollowTypes, pokemonPalettesFollowEvolutions, pokemonPalettesShinyFromNormal})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({typeEffectivenessMod == TypeEffectivenessMod::UNCHANGED, typeEffectivenessMod == TypeEffectivenessMod::RANDOM, typeEffectivenessMod == TypeEffectivenessMod::RANDOM_BALANCED, typeEffectivenessMod == TypeEffectivenessMod::KEEP_IDENTITIES, typeEffectivenessMod == TypeEffectivenessMod::INVERSE, inverseTypesRandomImmunities, updateTypeEffectiveness})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({evosForceGrowth, evosNoConvergence, adjustEvolutionLevels, false, false, false, false, false})));
    auto highEndByte = static_cast<uint8_t>(((startersBSTMinimum >> 8) & 0x0F) + ((startersBSTMaximum >> 4) & 0xF0));
    out.push_back(static_cast<uint8_t>(highEndByte));
    out.push_back(static_cast<uint8_t>((int8_t) startersBSTMinimum));
    out.push_back(static_cast<uint8_t>((int8_t) startersBSTMaximum));
    out.push_back(static_cast<uint8_t>(makeByteSelected({diverseTypesForBossTrainers, diverseTypesForImportantTrainers, diverseTypesForRegularTrainers, betterBossTrainerMovesets, betterImportantTrainerMovesets, betterRegularTrainerMovesets, false, false})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({settingBattleStyle.mod == BattleStyle::Modification::UNCHANGED, settingBattleStyle.mod == BattleStyle::Modification::RANDOM, settingBattleStyle.mod == BattleStyle::Modification::SINGLE_STYLE}) | (makeByteSelected({settingBattleStyle.style == BattleStyle::Style::SINGLE_BATTLE, settingBattleStyle.style == BattleStyle::Style::DOUBLE_BATTLE, settingBattleStyle.style == BattleStyle::Style::TRIPLE_BATTLE, settingBattleStyle.style == BattleStyle::Style::ROTATION_BATTLE}) << 3)));
    out.push_back(static_cast<uint8_t>(makeByteSelected({trainersEvolveTheirPokemon, banPrematureEvos, trainersLevelModified, wildLevelsModified, totemLevelsModified, staticLevelModified})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({balanceShopPrices, addCheapRareCandiesToShops, false, false, false, false, false, false})));
    out.push_back(static_cast<uint8_t>(makeByteSelected({randomizeIntroMon, raceMode, false, limitPokemon, false, false, false, false})));
    out.push_back(static_cast<uint8_t>(makeEvolutionsEasierLvl));
    out.push_back(static_cast<uint8_t>(makeByteSelected({bstMod == BSTMod::UNCHANGED, bstMod == BSTMod::RANDOM_BUFF_NERF, bstMod == BSTMod::SHUFFLE, bstMod == BSTMod::RANDOM, bstFollowEvolutions, bstShuffleSwapLegendaries, false, false})));
    out.push_back(static_cast<uint8_t>(bstBuffNerfMaxPercentage));
    
    out.push_back(static_cast<uint8_t>(romName.size()));
    out.insert(out.end(), romName.begin(), romName.end());
    
    
    
    writeFullIntBigEndian(out, static_cast<int>(crc32(out.data(), out.size())));
    writeFullInt(out, 0);
    return base64Encode(out);
}

Settings Settings::fromStringWithoutVersion(const std::string& settingsString) {
    std::vector<int8_t> data = base64Decode(settingsString);
    if (hasInvalidChecksum(data)) {
    throw std::invalid_argument("Malformed input string");
    }
    Settings settings;
    settings.changeImpossibleEvolutions = restoreState(data[0], 0);
    settings.updateMoves = restoreState(data[0], 1);
    settings.updateMovesLegacy = restoreState(data[0], 2);
    settings.randomizeTrainerNames = restoreState(data[0], 3);
    settings.randomizeTrainerClassNames = restoreState(data[0], 4);
    settings.makeEvolutionsEasier = restoreState(data[0], 5);
    settings.removeTimeBasedEvolutions = restoreState(data[0], 6);
    settings.estimateLevelForEvolutionImprovements = restoreState(data[0], 7);
    settings.baseStatisticsMod = restoreEnum<BaseStatisticsMod>(data[1], {3, 2, 1 });
    settings.standardizeEXPCurves = restoreState(data[1], 4);
    settings.baseStatsFollowEvolutions = restoreState(data[1], 0);
    settings.updateBaseStats = restoreState(data[1], 5);
    settings.baseStatsFollowMegaEvolutions = restoreState(data[1],6);
    settings.assignEvoStatsRandomly = restoreState(data[1],7);
    settings.speciesTypesMod = restoreEnum<SpeciesTypesMod>(data[2], {2, 0, 1 });
    settings.typesFollowMegaEvolutions = restoreState(data[2],6);
    settings.dualTypeOnly = restoreState(data[2], 7);
    settings.abilitiesMod = restoreEnum<AbilitiesMod>(data[3], {0, 1 });
    settings.allowWonderGuard = restoreState(data[3], 2);
    settings.abilitiesFollowEvolutions = restoreState(data[3], 3);
    settings.banTrappingAbilities = restoreState(data[3], 4);
    settings.banNegativeAbilities = restoreState(data[3], 5);
    settings.banBadAbilities = restoreState(data[3], 6);
    settings.abilitiesFollowMegaEvolutions = restoreState(data[3],7);
    settings.startersMod = restoreEnum<StartersMod>(data[4], {2, 0, 1, 3, 7 });
    settings.randomizeStartersHeldItems = restoreState(data[4], 4);
    settings.banBadRandomStarterHeldItems = restoreState(data[4], 5);
    settings.allowStarterAltFormes = restoreState(data[4],6);
    settings.customStarters = {read2ByteInt(data, 5), read2ByteInt(data, 7), read2ByteInt(data, 9)};
    settings.movesetsMod = restoreEnum<MovesetsMod>(data[11], {2, 1, 0, 3 });
    settings.startWithGuaranteedMoves = restoreState(data[11], 4);
    settings.reorderDamagingMoves = restoreState(data[11], 5);
    settings.guaranteedMoveCount = ((data[11] & 0xC0) >> 6) + 2;
    settings.movesetsForceGoodDamaging = restoreState(data[12], 7);
    settings.movesetsGoodDamagingPercent = data[12] & 0x7F;
    settings.trainersMod = restoreEnum<TrainersMod>(data[13], {0, 1, 2, 3, 4, 5, 6, 7 });
    settings.trainersEvolutionLevelModifier = data[14] + 28;
    settings.randomizeWildPokemon = !restoreState(data[15], 0);
    settings.wildPokemonZoneMod = restoreEnum<WildPokemonZoneMod>(data[15], {1, 2, 6, 5, 3 });
    settings.keepWildEvolutionFamilies = restoreState(data[15], 4);
    settings.splitWildZoneByEncounterTypes = restoreState(data[15], 7);
    settings.similarStrengthEncounters = restoreState(data[16], 1);
    settings.catchEmAllEncounters = restoreState(data[16], 2);
    settings.wildPokemonTypeMod = restoreEnum<WildPokemonTypeMod>(data[17], {0, 2, 1 });
    settings.keepWildTypeThemes = restoreState(data[17], 3);
    settings.wildPokemonEvolutionMod = restoreEnum<WildPokemonEvolutionMod>(data[17], {4, 5, 6 });
    settings.useTimeBasedEncounters = restoreState(data[18], 0);
    settings.useMinimumCatchRate = restoreState(data[18], 1);
    settings.blockWildLegendaries = restoreState(data[18], 2);
    settings.randomizeWildPokemonHeldItems = restoreState(data[18], 3);
    settings.banBadRandomWildPokemonHeldItems = restoreState(data[18], 4);
    settings.balanceShakingGrass = restoreState(data[18], 5);
    settings.staticPokemonMod = restoreEnum<StaticPokemonMod>(data[19], {0, 1, 2, 3 });
    settings.limitMainGameLegendaries = restoreState(data[19], 4);
    settings.limit600 = restoreState(data[19], 5);
    settings.allowStaticAltFormes = restoreState(data[19], 6);
    settings.swapStaticMegaEvos = restoreState(data[19], 7);
    settings.tmsMod = restoreEnum<TMsMod>(data[20], {4, 3 });
    settings.tmsHmsCompatibilityMod = restoreEnum<TMsHMsCompatibilityMod>(data[20], {2, 1, 0, 7 });
    settings.tmLevelUpMoveSanity = restoreState(data[20], 5);
    settings.keepFieldMoveTMs = restoreState(data[20], 6);
    settings.fullHMCompat = restoreState(data[21], 0);
    settings.tmsFollowEvolutions = restoreState(data[21], 1);
    settings.tutorFollowEvolutions = restoreState(data[21], 2);
    settings.tmsForceGoodDamaging = restoreState(data[22], 7);
    settings.tmsGoodDamagingPercent = data[22] & 0x7F;
    settings.moveTutorMovesMod = restoreEnum<MoveTutorMovesMod>(data[23], {4, 3 });
    settings.moveTutorsCompatibilityMod = restoreEnum<MoveTutorsCompatibilityMod>(data[23], {2, 1, 0, 7 });
    settings.tutorLevelUpMoveSanity = restoreState(data[23], 5);
    settings.keepFieldMoveTutors = restoreState(data[23], 6);
    settings.tutorsForceGoodDamaging = restoreState(data[24], 7);
    settings.tutorsGoodDamagingPercent = data[24] & 0x7F;
    settings.inGameTradesMod = restoreEnum<InGameTradesMod>(data[25], {6, 1, 0 });
    settings.randomizeInGameTradesItems = restoreState(data[25], 2);
    settings.randomizeInGameTradesIVs = restoreState(data[25], 3);
    settings.randomizeInGameTradesNicknames = restoreState(data[25], 4);
    settings.randomizeInGameTradesOTs = restoreState(data[25], 5);
    settings.fieldItemsMod = restoreEnum<FieldItemsMod>(data[26], {2, 1, 0, 4 });
    settings.banBadRandomFieldItems = restoreState(data[26], 3);
    settings.randomizeMovePowers = restoreState(data[27], 0);
    settings.randomizeMoveAccuracies = restoreState(data[27], 1);
    settings.randomizeMovePPs = restoreState(data[27], 2);
    settings.randomizeMoveTypes = restoreState(data[27], 3);
    settings.randomizeMoveCategory = restoreState(data[27], 4);
    settings.correctStaticMusic = restoreState(data[27], 5);
    settings.randomizeMoveNames = restoreState(data[27], 6);
    settings.evolutionsMod = restoreEnum<EvolutionsMod>(data[28], {0, 1, 7 });
    settings.evosSimilarStrength = restoreState(data[28], 2);
    settings.evosSameTyping = restoreState(data[28], 3);
    settings.evosMaxThreeStages = restoreState(data[28], 4);
    settings.evosForceChange = restoreState(data[28], 5);
    settings.evosAllowAltFormes = restoreState(data[28],6);
    settings.trainersUsePokemonOfSimilarStrength = restoreState(data[29], 0);
    settings.rivalCarriesStarterThroughout = restoreState(data[29], 1);
    settings.trainersMatchTypingDistribution = restoreState(data[29], 2);
    settings.trainersBlockLegendaries = restoreState(data[29], 3);
    settings.trainersBlockEarlyWonderGuard = restoreState(data[29], 4);
    settings.swapTrainerMegaEvos = restoreState(data[29], 5);
    settings.shinyChance = restoreState(data[29], 6);
    settings.trainersAvoidDuplicates = restoreState(data[29], 7);
    int genLimit = readFullInt(data, 30);
    GenRestrictions restrictions(genLimit);
    settings.currentRestrictions = restrictions;
    int codeTweaks = readFullIntBigEndian(data, 34);
    settings.currentMiscTweaks = codeTweaks;
    settings.trainersLevelModifier = data[38] + 28;
    settings.shopItemsMod = restoreEnum<ShopItemsMod>(data[39], {2, 1, 0});
    settings.banBadRandomShopItems = restoreState(data[39],3);
    settings.banRegularShopItems = restoreState(data[39],4);
    settings.banOPShopItems = restoreState(data[39],5);
    settings.guaranteeEvolutionItems = restoreState(data[39],7);
    settings.wildLevelModifier = data[40] + 28;
    settings.expCurveMod = restoreEnum<ExpCurveMod>(data[41], {0,1,2});
    settings.blockBrokenMovesetMoves = restoreState(data[41],3);
    settings.blockBrokenTMMoves = restoreState(data[41],4);
    settings.blockBrokenTutorMoves = restoreState(data[41],5);
    settings.allowTrainerAlternateFormes = restoreState(data[41],6);
    settings.allowWildAltFormes = restoreState(data[41],7);
    settings.additionalBossTrainerPokemon = (data[42] & 0xE) >> 1;
    settings.additionalImportantTrainerPokemon = (data[42] & 0x70) >> 4;
    settings.weighDuplicateAbilitiesTogether = restoreState(data[42], 7);
    settings.additionalRegularTrainerPokemon = (data[43] & 0x7);
    settings.auraMod = restoreEnum<AuraMod>(data[43], {3,4,5});
    settings.evolutionMovesForAll = restoreState(data[43],6);
    settings.guaranteeXItems = restoreState(data[43],7);
    settings.totemPokemonMod = restoreEnum<TotemPokemonMod>(data[44], {0,1,2});
    settings.allyPokemonMod = restoreEnum<AllyPokemonMod>(data[44], {3,4,5});
    settings.randomizeTotemHeldItems = restoreState(data[44],6);
    settings.allowTotemAltFormes = restoreState(data[44],7);
    settings.totemLevelModifier = data[45] + 28;
    settings.updateBaseStatsToGeneration = data[46];
    settings.updateMovesToGeneration = data[47];
    settings.selectedEXPCurve = expCurveFromByte(data[48]);
    settings.staticLevelModifier = data[49] + 28;
    settings.randomizeHeldItemsForBossTrainerPokemon = restoreState(data[50], 0);
    settings.randomizeHeldItemsForImportantTrainerPokemon = restoreState(data[50], 1);
    settings.randomizeHeldItemsForRegularTrainerPokemon = restoreState(data[50], 2);
    settings.consumableItemsOnlyForTrainerPokemon = restoreState(data[50], 3);
    settings.sensibleItemsOnlyForTrainerPokemon = restoreState(data[50], 4);
    settings.highestLevelOnlyGetsItemsForTrainerPokemon = restoreState(data[50], 5);
    settings.ensureTwoAbilities = restoreState(data[50], 6);
    settings.trainersUseLocalPokemon = restoreState(data[50], 7);
    settings.pickupItemsMod = restoreEnum<PickupItemsMod>(data[51], {1, 0});
    settings.banBadRandomPickupItems = restoreState(data[51], 2);
    settings.banIrregularAltFormes = restoreState(data[51], 3);
    settings.eliteFourUniquePokemonNumber = data[52] & 0x7;
    settings.minimumCatchRateLevel = ((data[52] & 0x38) >> 3) + 1;
    settings.startersTypeMod = restoreEnum<StartersTypeMod>(data[53], {0, 1, 2, 3, 4 });
    settings.startersNoLegendaries = restoreState(data[53], 6);
    settings.startersNoDualTypes = restoreState(data[53], 7);
    if(data[54] == 0) {
    settings.startersSingleType = std::nullopt;
    }
    else {
    settings.startersSingleType = typeFromInt((data[54] & 0x1F) - 1);
    }
    settings.pokemonPalettesMod = restoreEnum<PokemonPalettesMod>(data[55], {0, 1 });
    settings.pokemonPalettesFollowTypes = restoreState(data[55], 2);
    settings.pokemonPalettesFollowEvolutions = restoreState(data[55], 3);
    settings.pokemonPalettesShinyFromNormal = restoreState(data[55], 4);
    settings.typeEffectivenessMod = restoreEnum<TypeEffectivenessMod>(data[56], {0, 1, 2, 3, 4 });
    settings.inverseTypesRandomImmunities = restoreState(data[56], 5);
    settings.updateTypeEffectiveness = restoreState(data[56], 6);
    settings.evosForceGrowth = restoreState(data[57], 0);
    settings.evosNoConvergence = restoreState(data[57], 1);
    settings.adjustEvolutionLevels = restoreState(data[57], 2);
    settings.startersBSTMinimum = ((static_cast<uint8_t>(data[58]) & 0x0F) << 8) + static_cast<uint8_t>(data[59]);
    settings.startersBSTMaximum = ((static_cast<uint8_t>(data[58]) & 0xF0) << 4) + static_cast<uint8_t>(data[60]);
    settings.diverseTypesForBossTrainers = restoreState(data[61], 0);
    settings.diverseTypesForImportantTrainers = restoreState(data[61], 1);
    settings.diverseTypesForRegularTrainers = restoreState(data[61], 2);
    settings.betterBossTrainerMovesets = restoreState(data[61], 3);
    settings.betterImportantTrainerMovesets = restoreState(data[61], 4);
    settings.betterRegularTrainerMovesets = restoreState(data[61], 5);
    settings.settingBattleStyle.mod = restoreEnum<BattleStyle::Modification>(data[62], {0, 1, 2});
    settings.settingBattleStyle.style = restoreEnum<BattleStyle::Style>(data[62], {3, 4, 5, 6});
    settings.trainersEvolveTheirPokemon = restoreState(data[63], 0);
    settings.banPrematureEvos = restoreState(data[63], 1);
    settings.trainersLevelModified = restoreState(data[63], 2);
    settings.wildLevelsModified = restoreState(data[63], 3);
    settings.totemLevelsModified = restoreState(data[63],4);
    settings.staticLevelModified = restoreState(data[63], 5);
    settings.balanceShopPrices = restoreState(data[64],0);
    settings.addCheapRareCandiesToShops = restoreState(data[64], 1);
    settings.randomizeIntroMon = restoreState(data[65], 0);
    settings.raceMode = restoreState(data[65], 1);
    settings.limitPokemon = restoreState(data[65], 3);
    settings.makeEvolutionsEasierLvl = data[66] & 0x7F;
    settings.bstMod = restoreEnum<BSTMod>(data[67], {0, 1, 2, 3});
    settings.bstFollowEvolutions = restoreState(data[67], 4);
    settings.bstShuffleSwapLegendaries = restoreState(data[67], 5);
    settings.bstBuffNerfMaxPercentage = data[68];
    int romNameLength = data[LENGTH_OF_SETTINGS_DATA] & 0xFF;
    std::string romName(reinterpret_cast<const char*>(data.data()) + LENGTH_OF_SETTINGS_DATA + 1, romNameLength);
    settings.romName = romName;
    return settings;
}

Settings::TweakForROMFeedback Settings::tweakForRom(const RomHandler& rh) {
    TweakForROMFeedback fb;
    if (!rh.canSetIntroPokemon()) randomizeIntroMon = false;
    // (Gen 5: updateMovesLegacy no se aplica; se añadirá con Gen 5)
    const auto& romSpecies = rh.hasStarterAltFormes() ? rh.getSpeciesInclFormes() : rh.getSpecies();
    auto romStarters = rh.getStarters();
    for (int s = 0; s < 3; s++) {
        if (customStarters[s] < 0 || customStarters[s] >= static_cast<int>(romSpecies.size())) {
            fb.changedStarter = true;
            if (s >= static_cast<int>(romStarters.size())) {
                customStarters[s] = 1;
            } else {
                auto it = std::find(romSpecies.begin(), romSpecies.end(), romStarters[s]);
                customStarters[s] = it == romSpecies.end() ? -1 : static_cast<int>(it - romSpecies.begin());
            }
        }
    }
    if (!rh.canGiveCustomMovesetsToBossTrainers()) betterBossTrainerMovesets = false;
    if (!rh.canGiveCustomMovesetsToImportantTrainers()) betterImportantTrainerMovesets = false;
    if (!rh.canGiveCustomMovesetsToRegularTrainers()) betterRegularTrainerMovesets = false;
    if (!rh.canAddPokemonToBossTrainers()) additionalBossTrainerPokemon = 0;
    if (!rh.canAddPokemonToImportantTrainers()) additionalImportantTrainerPokemon = 0;
    if (!rh.canAddPokemonToRegularTrainers()) additionalRegularTrainerPokemon = 0;
    if (!rh.canAddHeldItemsToBossTrainers()) randomizeHeldItemsForBossTrainerPokemon = false;
    if (!rh.canAddHeldItemsToImportantTrainers()) randomizeHeldItemsForImportantTrainerPokemon = false;
    if (!rh.canAddHeldItemsToRegularTrainers()) randomizeHeldItemsForRegularTrainerPokemon = false;
    if (rh.generationOfPokemon() == 1 || (rh.generationOfPokemon() == 3 && !rh.isRomValid())) {
        currentRestrictions.reset();
        limitPokemon = false;
    } else {
        currentRestrictions->limitToGen(rh.generationOfPokemon());
    }
    if (rh.generationOfPokemon() != 5 && trainersMod == TrainersMod::MAINPLAYTHROUGH) trainersMod = TrainersMod::RANDOM;
    int old = currentMiscTweaks;
    currentMiscTweaks &= rh.miscTweaksAvailable();
    if (old != currentMiscTweaks) fb.removedCodeTweaks = true;
    if (rh.abilitiesPerSpecies() == 0) {
        abilitiesMod = AbilitiesMod::UNCHANGED;
        allowWonderGuard = false;
    }
    if (!rh.supportsStarterHeldItems()) {
        randomizeStartersHeldItems = false;
        banBadRandomStarterHeldItems = false;
    }
    if (!rh.supportsFourStartingMoves()) startWithGuaranteedMoves = false;
    if (rh.generationOfPokemon() <= 2) trainersBlockEarlyWonderGuard = false;
    if (!rh.hasTimeBasedEncounters()) useTimeBasedEncounters = false;
    if (rh.generationOfPokemon() == 1) {
        randomizeWildPokemonHeldItems = false;
        banBadRandomWildPokemonHeldItems = false;
        randomizeInGameTradesItems = false;
        randomizeInGameTradesIVs = false;
        randomizeInGameTradesOTs = false;
    }
    if (!rh.canChangeStaticPokemon()) staticPokemonMod = StaticPokemonMod::UNCHANGED;
    if (!rh.hasMoveTutors()) {
        moveTutorMovesMod = MoveTutorMovesMod::UNCHANGED;
        moveTutorsCompatibilityMod = MoveTutorsCompatibilityMod::UNCHANGED;
        tutorLevelUpMoveSanity = false;
        keepFieldMoveTutors = false;
    }
    if (!rh.hasPhysicalSpecialSplit()) randomizeMoveCategory = false;
    if (!rh.hasShopSupport()) {
        shopItemsMod = ShopItemsMod::UNCHANGED;
        balanceShopPrices = false;
    }
    if (!rh.canChangeShopSizes()) addCheapRareCandiesToShops = false;
    return fb;
}

}  // namespace pr
