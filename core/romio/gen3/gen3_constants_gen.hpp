// Generado por tools/convert_constants.py desde Gen3Constants.java (FVX, GPLv3). No editar.
#pragma once
#include <cstdint>
#include <set>
#include <string_view>
#include <vector>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"

namespace pr::gen3 {

inline constexpr int RomType_Ruby = 0;
inline constexpr int RomType_Sapp = 1;
inline constexpr int RomType_Em = 2;
inline constexpr int RomType_FRLG = 3;
inline constexpr int size8M = 0x800000;
inline constexpr int size16M = 0x1000000;
inline constexpr int size32M = 0x2000000;
inline constexpr std::string_view unofficialEmeraldROMName = "YJencrypted";
inline constexpr int romNameOffset = 0xA0;
inline constexpr int romCodeOffset = 0xAC;
inline constexpr int romVersionOffset = 0xBC;
inline constexpr int headerChecksumOffset = 0xBD;
inline constexpr int pokemonCount = 386;
inline constexpr int unownFormeCount = 28;
inline constexpr int unownBIndex = 413;
inline constexpr std::string_view rsPokemonNamesPointerSuffix = "30B50025084CC8F7";
inline constexpr std::string_view wildPokemonPointerPrefix = "0348048009E00000FFFF0000", mapBanksPointerPrefix = "80180068890B091808687047", frlgMapLabelsPointerPrefix = "AC470000AE470000B0470000", rseMapLabelsPointerPrefix = "C078288030BC01BC00470000", pokedexOrderPointerPrefix = "0448814208D0481C0004000C05E00000";
inline constexpr int pokemonFrontImagesPointer = 0x128;
inline constexpr int pokemonBackImagesPointer = 0x12C;
inline constexpr int pokemonNormalPalettesPointer = 0x130;
inline constexpr int pokemonShinyPalettesPointer = 0x134;
inline constexpr int pokemonIconSpritesPointer = 0x138;
inline constexpr int pokemonIconPalettesPointer = 0x13C;
inline constexpr int pokemonNamesPointer = 0x144;
inline constexpr int moveNamesPointer = 0x148;
inline constexpr int decorationNamesPointer = 0x14C;
inline constexpr int pokemonStatsPointer = 0x1BC;
inline constexpr int abilityNamesPointer = 0x1C0;
inline constexpr int abilityDescriptionsPointer = 0x1C4;
inline constexpr int itemDataPointer = 0x1C8;
inline constexpr int moveDataPointer = 0x1CC;
inline constexpr int ballSpritesPointer = 0x1D0;
inline constexpr int ballPalettesPointer = 0x1D4;
inline const std::vector<uint8_t> emptyPokemonSig = { 0x32, 0x96, 0x32, 0x96, 0x96, 0x32, 0x00, 0x00, 0x03, 0x01, 0xAA, 0x0A, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x78, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 };
inline constexpr int baseStatsEntrySize = 0x1C;
inline constexpr int bsHPOffset = 0;
inline constexpr int bsAttackOffset = 1;
inline constexpr int bsDefenseOffset = 2;
inline constexpr int bsSpeedOffset = 3;
inline constexpr int bsSpAtkOffset = 4;
inline constexpr int bsSpDefOffset = 5;
inline constexpr int bsPrimaryTypeOffset = 6;
inline constexpr int bsSecondaryTypeOffset = 7;
inline constexpr int bsCatchRateOffset = 8;
inline constexpr int bsEVYieldOffset = 10;
inline constexpr int bsCommonHeldItemOffset = 12;
inline constexpr int bsRareHeldItemOffset = 14;
inline constexpr int bsGenderRatioOffset = 16;
inline constexpr int bsEggCyclesOffset = 17;
inline constexpr int bsGrowthCurveOffset = 19;
inline constexpr int bsPrimaryEggGroupOffset = 20;
inline constexpr int bsSecondaryEggGroupOffset = 21;
inline constexpr int bsAbility1Offset = 22;
inline constexpr int bsAbility2Offset = 23;
inline constexpr uint8_t textTerminator = 0xFF;
inline constexpr uint8_t textVariable = 0xFD;
inline constexpr uint8_t textPadding = 0x00;
inline constexpr uint8_t freeSpaceByte = 0xFF;
inline constexpr int unusedSpaceChunkLength = 0x100;
inline constexpr int unusedSpaceFrontMargin = 0x10;
inline constexpr int rseStarter2Offset = 2;
inline constexpr int rseStarter3Offset = 4;
inline constexpr int frlgStarter2Offset = 515;
inline constexpr int frlgStarter3Offset = 461;
inline constexpr int frlgStarterRepeatOffset = 5;
inline constexpr int frlgStarterItemsOffset = 218;
inline constexpr int gbaAddRxOpcode = 0x30;
inline constexpr int gbaUnconditionalJumpOpcode = 0xE0;
inline constexpr int gbaSetRxOpcode = 0x20;
inline constexpr int gbaCmpRxOpcode = 0x28;
inline constexpr int gbaNopOpcode = 0x46C0;
inline constexpr int gbaR0 = 0;
inline constexpr int gbaR1 = 1;
inline constexpr int gbaR2 = 2;
inline constexpr int gbaR3 = 3;
inline constexpr int gbaR4 = 4;
inline constexpr int gbaR5 = 5;
inline constexpr int gbaR6 = 6;
inline constexpr int gbaR7 = 7;
inline constexpr int walkingSlots = 12;
inline constexpr int surfingSlots = 5;
inline constexpr int rockSmashSlots = 5;
inline constexpr int fishingSlots = 10;
inline const std::vector<uint8_t> vanillaMovesLearntTerminator = {0xFF, 0xFF};
inline const std::vector<uint8_t> jamboMovesLearntTerminator = {0x00, 0x00, 0xFF};
inline constexpr int tmCount = 50;
inline constexpr int hmCount = 8;
inline const std::vector<int> hmMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::rockSmash, moves::waterfall, moves::dive};
inline constexpr int rseItemDescCharsPerLine = 18;
inline constexpr int frlgItemDescCharsPerLine = 24;
inline constexpr int regularTextboxCharsPerLine = 36;
inline constexpr int pointerSearchRadius = 500;
inline constexpr int itemDataDescriptionOffset = 0x14;
inline constexpr std::string_view deoxysObeyCode = "CD21490088420FD0";
inline constexpr int mewObeyOffsetFromDeoxysObey = 0x16;
inline constexpr std::string_view levelEvoKantoDexCheckCode = "972814DD";
inline constexpr std::string_view stoneEvoKantoDexCheckCode = "972808D9";
inline constexpr int levelEvoKantoDexJumpAmount = 0x14;
inline constexpr int stoneEvoKantoDexJumpAmount = 0x08;
inline constexpr std::string_view rsPokedexScriptIdentifier = "326629010803";
inline constexpr int rsNatDexScriptLength = 44;
inline constexpr std::string_view rsNatDexScriptPart1 = "31720167";
inline constexpr std::string_view rsNatDexScriptPart2 = "32662901082B00801102006B02021103016B020211DABE4E020211675A6A02022A008003";
inline constexpr std::string_view frlgPokedexScriptIdentifier = "292908258101";
inline constexpr int frlgNatDexScriptLength = 10;
inline constexpr std::string_view frlgNatDexScript = "292908258101256F0103";
inline constexpr std::string_view frlgNatDexFlagChecker = "260D809301210D800100";
inline constexpr std::string_view frlgE4FlagChecker = "2B2C0800000000000000";
inline constexpr std::string_view frlgOaksLabKantoDexChecker = "257D011604800000260D80D400";
inline constexpr std::string_view frlgOaksLabFix = "257D011604800100";
inline constexpr std::string_view frlgOakOutsideHouseCheck = "1604800000260D80D4001908800580190980068083000880830109802109803C";
inline constexpr std::string_view frlgOakOutsideHouseFix = "1604800100";
inline constexpr std::string_view frlgOakAideCheckPrefix = "00B5064800880028";
inline constexpr std::string_view ePokedexScriptIdentifier = "3229610825F00129E40816CD40010003";
inline constexpr int eNatDexScriptLength = 27;
inline constexpr std::string_view eNatDexScriptPart1 = "31720167";
inline constexpr std::string_view eNatDexScriptPart2 = "3229610825F00129E40825F30116CD40010003";
inline constexpr std::string_view friendshipValueForEvoLocator = "DB2900D8";
inline constexpr std::string_view perfectOddsBranchLocator = "FE2E2FD90020";
inline constexpr int unhackedMaxPokedex = 411;
inline constexpr int unhackedRealPokedex = 386;
inline constexpr int hoennPokesStart = 252;
inline constexpr int evolutionMethodCount = 15;
inline constexpr int cacophonyIndex = 76;
inline constexpr int airLockIndex = 77;
inline constexpr int highestAbilityIndex = 77;
inline constexpr int emMeteorFallsStevenIndex = 804;
inline constexpr int emBrendanFrontImageIndex = 71;
inline constexpr int frlgRedFrontImageIndex = 135;
inline constexpr int rsTrainerFrontPalettesOffset = 0x298;
inline constexpr int emTrainerFrontPalettesOffset = 0x2E8;
inline constexpr int frlgTrainerFrontPalettesOffset = 0x4A0;
inline constexpr int rsTrainerBackPalettesOffset = 0x18;
inline constexpr int emTrainerBackPalettesOffset = 0x40;
inline constexpr int frlgTrainerBackPalettesOffset = 0x30;
inline constexpr int brendanMapIconPaletteOffset = -0x20;
inline constexpr int mayMapIconImageOffset = 0xA0;
inline constexpr int mayMapIconPaletteOffset = 0x80;
inline constexpr int redMapIconPalettePointerOffset = 0xC4;
inline constexpr int leafMapIconImagePointerOffset = -0x30;
inline constexpr int leafMapIconPalettePointerOffset = 0xE0;
inline const std::vector<int> uselessAbilities = {abilities::forecast, cacophonyIndex};
inline constexpr int frlgMapLabelsStart = 0x58;
inline constexpr int noDamageSleepEffect = 1;
inline constexpr int damagePoisonEffect = 2;
inline constexpr int damageAbsorbEffect = 3;
inline constexpr int damageBurnEffect = 4;
inline constexpr int damageFreezeEffect = 5;
inline constexpr int damageParalyzeEffect = 6;
inline constexpr int dreamEaterEffect = 8;
inline constexpr int noDamageAtkPlusOneEffect = 10;
inline constexpr int noDamageDefPlusOneEffect = 11;
inline constexpr int noDamageSpAtkPlusOneEffect = 13;
inline constexpr int noDamageEvasionPlusOneEffect = 16;
inline constexpr int noDamageAtkMinusOneEffect = 18;
inline constexpr int noDamageDefMinusOneEffect = 19;
inline constexpr int noDamageSpeMinusOneEffect = 20;
inline constexpr int noDamageAccuracyMinusOneEffect = 23;
inline constexpr int noDamageEvasionMinusOneEffect = 24;
inline constexpr int flinchEffect = 31;
inline constexpr int toxicEffect = 33;
inline constexpr int razorWindEffect = 39;
inline constexpr int bindingEffect = 42;
inline constexpr int increasedCritEffect = 43;
inline constexpr int damageRecoil25PercentEffect = 48;
inline constexpr int noDamageConfusionEffect = 49;
inline constexpr int noDamageAtkPlusTwoEffect = 50;
inline constexpr int noDamageDefPlusTwoEffect = 51;
inline constexpr int noDamageSpePlusTwoEffect = 52;
inline constexpr int noDamageSpAtkPlusTwoEffect = 53;
inline constexpr int noDamageSpDefPlusTwoEffect = 54;
inline constexpr int noDamageAtkMinusTwoEffect = 58;
inline constexpr int noDamageDefMinusTwoEffect = 59;
inline constexpr int noDamageSpeMinusTwoEffect = 60;
inline constexpr int noDamageSpDefMinusTwoEffect = 62;
inline constexpr int noDamagePoisonEffect = 66;
inline constexpr int noDamageParalyzeEffect = 67;
inline constexpr int damageAtkMinusOneEffect = 68;
inline constexpr int damageDefMinusOneEffect = 69;
inline constexpr int damageSpeMinusOneEffect = 70;
inline constexpr int damageSpAtkMinusOneEffect = 71;
inline constexpr int damageSpDefMinusOneEffect = 72;
inline constexpr int damageAccuracyMinusOneEffect = 73;
inline constexpr int skyAttackEffect = 75;
inline constexpr int damageConfusionEffect = 76;
inline constexpr int twineedleEffect = 77;
inline constexpr int rechargeEffect = 80;
inline constexpr int snoreEffect = 92;
inline constexpr int trappingEffect = 106;
inline constexpr int minimizeEffect = 108;
inline constexpr int swaggerEffect = 118;
inline constexpr int damageBurnAndThawUserEffect = 125;
inline constexpr int damageUserDefPlusOneEffect = 138;
inline constexpr int damageUserAtkPlusOneEffect = 139;
inline constexpr int damageUserAllPlusOneEffect = 140;
inline constexpr int skullBashEffect = 145;
inline constexpr int twisterEffect = 146;
inline constexpr int futureSightAndDoomDesireEffect = 148;
inline constexpr int flinchWithMinimizeBonusEffect = 150;
inline constexpr int solarbeamEffect = 151;
inline constexpr int thunderEffect = 152;
inline constexpr int semiInvulnerableEffect = 155;
inline constexpr int defenseCurlEffect = 156;
inline constexpr int fakeOutEffect = 158;
inline constexpr int spitUpEffect = 161;
inline constexpr int flatterEffect = 166;
inline constexpr int noDamageBurnEffect = 167;
inline constexpr int chargeEffect = 174;
inline constexpr int damageUserAtkAndDefMinusOneEffect = 182;
inline constexpr int damageRecoil33PercentEffect = 198;
inline constexpr int teeterDanceEffect = 199;
inline constexpr int blazeKickEffect = 200;
inline constexpr int poisonFangEffect = 202;
inline constexpr int damageUserSpAtkMinusTwoEffect = 204;
inline constexpr int noDamageAtkAndDefMinusOneEffect = 205;
inline constexpr int noDamageDefAndSpDefPlusOneEffect = 206;
inline constexpr int noDamageAtkAndDefPlusOneEffect = 208;
inline constexpr int poisonTailEffect = 209;
inline constexpr int noDamageSpAtkAndSpDefPlusOneEffect = 211;
inline constexpr int noDamageAtkAndSpePlusOneEffect = 212;
inline const std::vector<int> soundMoves = {moves::growl, moves::roar, moves::sing, moves::supersonic, moves::screech, moves::snore, moves::uproar, moves::metalSound, moves::grassWhistle, moves::hyperVoice, moves::perishSong, moves::healBell};
inline const std::vector<int> rsRequiredFieldTMs = { items::tm01, items::tm02, items::tm06, items::tm07, items::tm11, items::tm18, items::tm22, items::tm23, items::tm26, items::tm30, items::tm37, items::tm48 };
inline const std::vector<int> eRequiredFieldTMs = { items::tm02, items::tm06, items::tm07, items::tm11, items::tm18, items::tm22, items::tm23, items::tm30, items::tm37, items::tm48 };
inline const std::vector<int> frlgRequiredFieldTMs = { items::tm01, items::tm02, items::tm07, items::tm08, items::tm09, items::tm11, items::tm12, items::tm14, items::tm17, items::tm18, items::tm21, items::tm22, items::tm25, items::tm32, items::tm36, items::tm37, items::tm40, items::tm41, items::tm44, items::tm46, items::tm47, items::tm48, items::tm49, items::tm50 };
inline const std::vector<int> rseFieldMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::dig, moves::teleport, moves::waterfall, moves::rockSmash, moves::sweetScent, moves::dive, moves::secretPower};
inline const std::vector<int> frlgFieldMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::dig, moves::teleport, moves::waterfall, moves::rockSmash, moves::sweetScent};
inline const std::vector<uint8_t> shopTerminator = {0x00, 0x00, 0x6C, 0x02};
inline const std::vector<int> evolutionItems = {items::sunStone, items::moonStone, items::fireStone, items::thunderStone, items::waterStone, items::leafStone};
inline constexpr int nonNeutralEffectivenessCount = 110;

// Pendiente de portar a mano:
//   Type[] typeTable
//   Map<Integer,List<Integer>> abilityVariations
//   List<Integer> rseEarlyRequiredHMMoves
//   List<Integer> frlgEarlyRequiredHMMoves
//   List<Integer> consumableHeldItems
//   List<Integer> allHeldItems
//   List<Integer> generalPurposeConsumableItems
//   List<Integer> generalPurposeItems
//   Map<Type, List<Integer>> typeBoostingItems
//   Map<Integer, List<Integer>> speciesBoostingItems
//   Set<Integer> bannedItems
//   Set<Integer> opShopItems
//   HashMap<String, Type> gymAndEliteThemesRS
//   HashMap<String, Type> gymAndEliteThemesEm
//   HashMap<String, Type> gymAndEliteThemesFRLG
//   Map<Integer,Integer> balancedItemPrices
//   Map<Integer, Integer> itemIDToStandardMap
//   Map<Integer, Integer> itemIDToInternalMap

}  // namespace pr::gen3
