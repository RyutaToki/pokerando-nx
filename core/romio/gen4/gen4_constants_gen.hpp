// Generado por tools/convert_constants.py desde Gen4Constants.java (FVX, GPLv3).
// Retocado a mano: marshGrassEncounterIndicesDP/Pt (dos listas en una declaración).
#pragma once
#include <cstdint>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"

namespace pr::gen4 {

inline constexpr int Type_DP = 0;
inline constexpr int Type_Plat = 1;
inline constexpr int Type_HGSS = 2;
inline constexpr int arm9Offset = 0x02000000;
inline constexpr int pokemonCount = 493;
inline constexpr int moveCount = 467;
inline constexpr int dpFormeCount = 5;
inline constexpr int platHgSsFormeCount = 12;
inline constexpr int formeOffset = 2;
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
inline constexpr int bsTMHMCompatOffset = 28;
inline constexpr std::string_view starterCriesPrefix = "0004000C10BD0000000000000000000000E000000000000000E0000000000200";
inline const std::vector<uint8_t> hgssStarterCodeSuffix = { 0x03, 0x03, 0x1A, 0x12, 0x1, 0x23, 0x0, 0x0 };
inline const std::vector<int> hgssFilesWithRivalScript = { 7, 23, 96, 110, 819, 850, 866 };
inline const std::vector<uint8_t> hgssRivalScriptMagic = { 0xCE, 0x00, 0x0C, 0x80, 0x11, 0x00, 0x0C, 0x80, 152, 0, 0x1C, 0x00, 0x05 };
inline const std::vector<int> ptFilesWithRivalScript = { 31, 36, 112, 123, 186, 427, 429, 1096 };
inline const std::vector<int> dpFilesWithRivalScript = { 34, 90, 118, 180, 195, 394 };
inline const std::vector<uint8_t> dpptRivalScriptMagic = { 0xDE, 0x00, 0x0C, 0x80, 0x11, 0x00, 0x0C, 0x80, 0x83, 0x01, 0x1C, 0x00, 0x01 };
inline const std::vector<uint8_t> dpptTagBattleScriptMagic1 = { 0xDE, 0x00, 0x0C, 0x80, 0x28, 0x00, 0x04, 0x80 };
inline const std::vector<uint8_t> dpptTagBattleScriptMagic2 = { 0x11, 0x00, 0x0C, 0x80, 0x86, 0x01, 0x1C, 0x00, 0x01 };
inline const std::vector<int> ptFilesWithTagScript = { 2, 136, 201, 236 };
inline const std::vector<int> dpFilesWithTagScript = { 2, 131, 230 };
inline constexpr int dpStarterStringIndex = 19;
inline constexpr int ptStarterStringIndex = 36;
inline constexpr int fossilCount = 7;
inline constexpr std::string_view dpptTMDataPrefix = "D100D200D300D400", hgssTMDataPrefix = "1E003200";
inline constexpr int tmCount = 92;
inline constexpr int hmCount = 8;
inline constexpr int tmItemOffset = items::tm01;
inline constexpr uint8_t tmsReusableByteBefore = 0xD1;
inline constexpr uint8_t tmsReusableByteAfter = 0xE0;
inline constexpr int dpptTextCharsPerLine = 38;
inline constexpr int hgssTextCharsPerLine = 36;
inline constexpr std::string_view dpItemPalettesPrefix = "8D018E01210132018D018F0122013301", pthgssItemPalettesPrefix = "8D018E01210133018D018F0122013401";
inline constexpr int ptSpearPillarPortalScriptFile = 237;
inline constexpr int evolutionMethodCount = 26;
inline constexpr int highestAbilityIndex = abilities::badDreams;
inline constexpr int mysteryEggCommandLength = 216;
inline constexpr int mysteryEggImprovementIdentifierOffset = 30;
inline constexpr uint8_t mysteryEggImprovementIdentifier = 0x57;
inline const std::vector<int> consumableHeldItems = { items::cheriBerry, items::chestoBerry, items::pechaBerry, items::rawstBerry, items::aspearBerry, items::leppaBerry, items::oranBerry, items::persimBerry, items::lumBerry, items::sitrusBerry, items::figyBerry, items::wikiBerry, items::magoBerry, items::aguavBerry, items::iapapaBerry, items::occaBerry, items::passhoBerry, items::wacanBerry, items::rindoBerry, items::yacheBerry, items::chopleBerry, items::kebiaBerry, items::shucaBerry, items::cobaBerry, items::payapaBerry, items::tangaBerry, items::chartiBerry, items::kasibBerry, items::habanBerry, items::colburBerry, items::babiriBerry, items::chilanBerry, items::liechiBerry, items::ganlonBerry, items::salacBerry, items::petayaBerry, items::apicotBerry, items::lansatBerry, items::starfBerry, items::enigmaBerry, items::micleBerry, items::custapBerry, items::jabocaBerry, items::rowapBerry, items::berryJuice, items::whiteHerb, items::mentalHerb, items::powerHerb, items::focusSash};
inline const std::vector<int> generalPurposeConsumableItems = { items::cheriBerry, items::chestoBerry, items::pechaBerry, items::rawstBerry, items::aspearBerry, items::leppaBerry, items::oranBerry, items::persimBerry, items::lumBerry, items::sitrusBerry, items::ganlonBerry, items::salacBerry, items::apicotBerry, items::lansatBerry, items::starfBerry, items::enigmaBerry, items::micleBerry, items::custapBerry, items::jabocaBerry, items::rowapBerry, items::berryJuice, items::whiteHerb, items::mentalHerb, items::focusSash};
inline const std::vector<int> generalPurposeItems = { items::brightPowder, items::quickClaw, items::kingsRock, items::focusBand, items::scopeLens, items::leftovers, items::shellBell, items::laxIncense, items::wideLens, items::expertBelt, items::lifeOrb, items::zoomLens, items::destinyKnot, items::shedShell, items::razorClaw, items::razorFang};
inline const std::vector<int> uselessAbilities = {abilities::forecast, abilities::multitype, abilities::flowerGift};
inline constexpr int dpptSetVarScript = 0x28;
inline constexpr int hgssSetVarScript = 0x29;
inline constexpr int scriptListTerminator = 0xFD13;
inline constexpr int itemScriptVariable = 0x8008;
inline const std::vector<std::string_view> dpSpecialShopNames = { "Jubilife Secondary", "Oreburgh Secondary", "Floaroma Secondary", "Eterna Secondary", "Eterna Herbs", "Hearthome Secondary", "Solaceon Secondary", "Pastoria Secondary", "Veilstone Department Store 1F (Right)", "Veilstone Department Store 1F (Left)", "Veilstone Department Store 2F (Top)", "Veilstone Department Store 2F (Middle)", "Veilstone Department Store 3F (Top)", "Veilstone Department Store 3F (Bottom)", "Celestic Secondary", "Snowpoint Secondary", "Canalave Secondary", "Sunyshore Secondary", "Pokemon League Secondary" };
inline const std::vector<std::string_view> hgssSpecialShopNames = { "Cherrygrove Secondary", "Violet Secondary", "Azalea Secondary", "Goldenrod Department Store 2F (Lower)", "Goldenrod Department Store 2F (Upper)", "Goldenrod Department Store 3F", "Goldenrod Department Store 4F", "Goldenrod Department Store 5F", "Goldenrod Herb Shop", "Ecruteak Secondary", "Olivine Secondary", "Cianwood Pharmacy", "Blackthorn Secondary", "Unused Secondary", "Safari Zone Gate Southwest", "Saffron Secondary", "Lavender Secondary", "Cerulean Secondary", "Celadon Department Store 2F (Left)", "Celadon Department Store 2F (Right)", "Celadon Department Store 3F", "Celadon Department Store 4F", "Celadon Department Store 5F (Left)", "Celadon Department Store 5F (Right)", "Fuchsia Secondary", "Pewter Secondary", "Viridian Secondary", "Mt. Moon Square", "Mahogany Before Hideout", "Mahogany After Hideout" };
inline const std::vector<int> dpMainGameShops = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
inline const std::vector<int> hgssMainGameShops = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 30 };
inline const std::vector<int> evolutionItems = {items::sunStone, items::moonStone, items::fireStone, items::thunderStone, items::waterStone, items::leafStone, items::shinyStone, items::duskStone, items::dawnStone, items::ovalStone, items::kingsRock, items::deepSeaTooth, items::deepSeaScale, items::metalCoat, items::dragonScale, items::upgrade, items::protector, items::electirizer, items::magmarizer, items::dubiousDisc, items::reaperCloth, items::razorClaw, items::razorFang};
inline constexpr std::string_view lyraEthanMarillSpritePrefix = "274E0604C301274E0704E101274E0804";
inline const std::vector<int> hgssBigOverworldPokemon = { species::steelix, species::lugia, species::hoOh, species::wailord, species::kyogre, species::groudon, species::rayquaza, species::dialga, species::palkia, species::regigigas, species::giratina, species::arceus };
inline constexpr int dpptWaterAreaSize = 44;
inline const std::vector<int> dpptAlternateSlots = { 0, 1, 2, 3, 2, 3, 4, 5, 10, 11, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9 };
inline constexpr int hgssGoodRodReplacementIndex = 3;
inline constexpr int hgssSuperRodReplacementIndex = 1;
inline constexpr double stabMultiplier = 1.5;
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
inline constexpr int stompEffect = 150;
inline constexpr int solarbeamEffect = 151;
inline constexpr int thunderEffect = 152;
inline constexpr int flyEffect = 155;
inline constexpr int defenseCurlEffect = 156;
inline constexpr int fakeOutEffect = 158;
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
inline constexpr int damagePoisonWithIncreasedCritEffect = 209;
inline constexpr int noDamageSpAtkAndSpDefPlusOneEffect = 211;
inline constexpr int noDamageAtkAndSpePlusOneEffect = 212;
inline constexpr int damageUserSpeMinusOneEffect = 218;
inline constexpr int damageUserDefAndSpDefMinusOneEffect = 229;
inline constexpr int flareBlitzEffect = 253;
inline constexpr int diveEffect = 255;
inline constexpr int digEffect = 256;
inline constexpr int blizzardEffect = 260;
inline constexpr int voltTackleEffect = 262;
inline constexpr int bounceEffect = 263;
inline constexpr int noDamageSpAtkMinusTwoEffect = 265;
inline constexpr int chatterEffect = 267;
inline constexpr int damageRecoil50PercentEffect = 269;
inline constexpr int damageSpDefMinusTwoEffect = 271;
inline constexpr int shadowForceEffect = 272;
inline constexpr int fireFangEffect = 273;
inline constexpr int iceFangEffect = 274;
inline constexpr int thunderFangEffect = 275;
inline constexpr int damageUserSpAtkPlusOneEffect = 276;
inline const std::vector<int> soundMoves = {moves::growl, moves::roar, moves::sing, moves::supersonic, moves::screech, moves::snore, moves::uproar, moves::metalSound, moves::grassWhistle, moves::hyperVoice, moves::bugBuzz, moves::chatter, moves::perishSong, moves::healBell};
inline const std::vector<int> punchMoves = {moves::icePunch, moves::firePunch, moves::thunderPunch, moves::machPunch, moves::focusPunch, moves::dizzyPunch, moves::dynamicPunch, moves::hammerArm, moves::megaPunch, moves::cometPunch, moves::meteorMash, moves::shadowPunch, moves::drainPunch, moves::bulletPunch, moves::skyUppercut};
inline const std::vector<int> dpRequiredFieldTMs = {items::tm02, items::tm03, items::tm05, items::tm09, items::tm12, items::tm19, items::tm23, items::tm28, items::tm34, items::tm39, items::tm41, items::tm43, items::tm46, items::tm47, items::tm49, items::tm50, items::tm62, items::tm69, items::tm79, items::tm80, items::tm82, items::tm84, items::tm85, items::tm87};
inline const std::vector<int> ptRequiredFieldTMs = {items::tm02, items::tm03, items::tm05, items::tm07, items::tm09, items::tm11, items::tm12, items::tm18, items::tm19, items::tm23, items::tm28, items::tm34, items::tm37, items::tm39, items::tm41, items::tm43, items::tm46, items::tm47, items::tm49, items::tm50, items::tm62, items::tm69, items::tm79, items::tm80, items::tm82, items::tm84, items::tm85, items::tm87};
inline const std::vector<int> dpptFieldMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::dig, moves::teleport, moves::waterfall, moves::rockSmash, moves::sweetScent, moves::defog, moves::rockClimb};
inline const std::vector<int> hgssFieldMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::dig, moves::teleport, moves::whirlpool, moves::waterfall, moves::rockSmash, moves::headbutt, moves::sweetScent, moves::rockClimb};
inline const std::vector<int> dpptEarlyRequiredHMMoves = {moves::rockSmash, moves::cut};
inline const std::vector<int> hgssEarlyRequiredHMMoves = {moves::cut};
inline constexpr std::string_view shedinjaSpeciesLocator = "492080000090281C0521";
inline constexpr int ilexForestScriptFile = 92;
inline constexpr int ilexForestStringsFile = 115;
inline const std::vector<int> headbuttTutorScriptOffsets = {0xF55, 0xFC5, 0x100A, 0x104C};
inline constexpr std::string_view doubleBattleFixPrefixDP = "022912D90221214201", doubleBattleFixPrefixPt = "022919D90221214205", doubleBattleFixPrefixHGSS = "2C2815D00221214201";
inline constexpr std::string_view feebasLevelPrefixDPPt = "019813B0F0BD", honeyTreeLevelPrefixDPPt = "F0BDF0B589B0051C0C1C";
inline constexpr std::string_view runningShoesCheckPrefixDPPt = "281C0C24", runningShoesCheckPrefixHGSS = "301C0C24";
inline constexpr std::string_view distortionWorldGroundCheckPrefix = "23D849187944C988090409148F44";
inline const std::vector<std::string_view> dpptIntroPrefixes = {"381CF8BDC046", "08B0F8BD"};
inline constexpr std::string_view hpBarSpeedPrefix = "0CD106200090", expBarSpeedPrefix = "011C00D101212E6C", bothBarsSpeedPrefix = "70BD90421DDA";
inline constexpr std::string_view dpptEggMoveTablePrefix = "40016601";
inline constexpr std::string_view typeEffectivenessTableLocator = "000505000805";
inline constexpr int trophyGardenGrassEncounterIndexDP = 304;
inline constexpr int trophyGardenGrassEncounterIndexPt = 308;
inline const std::vector<int> marshGrassEncounterIndicesDP = {76, 82, 88, 94, 100, 102};
inline const std::vector<int> marshGrassEncounterIndicesPt = {76, 82, 88, 94, 100, 106};
inline constexpr std::string_view pickupTableLocator = "110012001A000300", rarePickupTableLocator = "19005C00DD00";
inline constexpr int numberOfCommonPickupItems = 18;
inline constexpr int numberOfRarePickupItems = 11;
inline constexpr std::string_view friendshipValueForEvoLocator = "DC286AD3";
inline constexpr std::string_view perfectOddsBranchLocator = "FF2901D30425";
inline const std::vector<int> dpptOverworldDexMaps = { 1, 2, 3, 4, 5, -1, -1, 6, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, -1, -1, -1, -1, -1, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, };
inline const std::vector<int> dpptDungeonDexMaps = { -1, -1, -1, -1, -1, 1, 1, -1, 2, -1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 8, 8, -1, 9, 9, 10, -1, -1, -1, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 11, 11, 11, 11, 11, 12, 12, 13, 13, 13, 14, -1, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 18, 19, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 20, 20, 20, 20, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };
inline const std::vector<int> hgssOverworldDexMaps = { 1, 2, 3, 4, 5, 6, -1, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1, -1, -1, 9, 10, -1, -1, 11, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 15, 16, 17, 18, -1, -1, -1, -1, -1, -1, -1, -1, 19, 20, -1, -1, -1, -1, 21, 22, 23, -1, -1, -1, -1, -1, 24, -1, 25, 26, -1, -1, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 28, -1, -1, -1, -1, -1, -1, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, -1, -1, -1, -1, -1, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1, -1, -1, -1, 64, -1, -1, -1, -1, -1, };
inline const std::vector<int> hgssDungeonDexMaps = { -1, -1, -1, -1, -1, -1, 1, 1, -1, 2, 2, 2, 2, 2, 3, 3, 3, -1, 4, 4, 5, -1, -1, 6, -1, -1, -1, -1, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, -1, -1, -1, -1, -1, 9, 9, -1, 9, -1, 9, -1, -1, -1, -1, 10, 10, 10, 10, -1, -1, -1, 11, 11, 11, 11, -1, -1, 12, -1, -1, 13, 13, -1, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 17, 18, 8, -1, 16, 16, 16, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 14, 20, 20, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 22, 23, 21, 21, -1, 24, -1, 25, 25, 25, };
inline const std::vector<int> hgssHeadbuttOverworldDexMaps = { 43, 44, 45, 46, 47, 48, 49, 50, 53, 29, 54, 55, 56, 59, 61, 63, 40, 41, 42, 2, 4, 5, 7, 8, 9, 10, 11, 12, 14, 15, 20, 21, 23, 25, 26, 32, 33, 65, 34, 35, 36, 37, 1, 3, 6, 66, 13, 22, 28, 60, -1, -1, -1, 27, 39, 67, 64, 57, -1, };
inline const std::vector<int> hgssHeadbuttDungeonDexMaps = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 5, 24, -1, -1, -1, -1, -1, 16, };
inline constexpr int pokedexAreaDataSize = 495;
inline constexpr int dpptMtCoronetDexIndex = 3;
inline constexpr int dpptGreatMarshDexIndex = 4;
inline constexpr int dpptTrophyGardenDexIndex = 14;
inline constexpr int dpptFloaromaMeadowDexIndex = 21;
inline const std::vector<int> dpptOverworldHoneyTreeDexIndicies = {6, 7, 17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31, 34, 36, 37, 50};
inline const std::vector<int> partnerTrainerIndices = {608, 609, 610, 611, 612};
// HashMap<Integer, Integer> en Java (pares en el orden del código fuente)
inline const std::vector<std::pair<int, int>> balancedItemPrices = {{items::masterBall, 3000}, {items::ultraBall, 1200}, {items::greatBall, 600}, {items::pokeBall, 200}, {items::safariBall, 500}, {items::netBall, 1000}, {items::diveBall, 1000}, {items::nestBall, 1000}, {items::repeatBall, 1000}, {items::timerBall, 1000}, {items::luxuryBall, 1000}, {items::premierBall, 200}, {items::duskBall, 1000}, {items::healBall, 300}, {items::quickBall, 1000}, {items::cherishBall, 200}, {items::fastBall, 300}, {items::levelBall, 300}, {items::lureBall, 300}, {items::heavyBall, 300}, {items::loveBall, 300}, {items::friendBall, 300}, {items::moonBall, 300}, {items::sportBall, 300}, {items::redApricorn, 20}, {items::yellowApricorn, 20}, {items::blueApricorn, 20}, {items::greenApricorn, 20}, {items::pinkApricorn, 20}, {items::whiteApricorn, 20}, {items::blackApricorn, 20}, {items::potion, 300}, {items::antidote, 100}, {items::burnHeal, 250}, {items::iceHeal, 250}, {items::awakening, 250}, {items::paralyzeHeal, 200}, {items::fullRestore, 3000}, {items::maxPotion, 2500}, {items::hyperPotion, 1200}, {items::superPotion, 700}, {items::fullHeal, 600}, {items::revive, 1500}, {items::maxRevive, 4000}, {items::freshWater, 400}, {items::sodaPop, 600}, {items::lemonade, 700}, {items::moomooMilk, 800}, {items::energyPowder, 400}, {items::energyRoot, 1100}, {items::healPowder, 450}, {items::revivalHerb, 2800}, {items::ether, 3000}, {items::maxEther, 4500}, {items::elixir, 15000}, {items::maxElixir, 18000}, {items::lavaCookie, 450}, {items::berryJuice, 100}, {items::sacredAsh, 10000}, {items::oldGateau, 450}, {items::hpUp, 9800}, {items::protein, 9800}, {items::iron, 9800}, {items::carbos, 9800}, {items::calcium, 9800}, {items::rareCandy, 10000}, {items::ppUp, 9800}, {items::zinc, 9800}, {items::ppMax, 24900}, {items::guardSpec, 700}, {items::direHit, 650}, {items::xAttack, 500}, {items::xDefense, 550}, {items::xSpeed, 350}, {items::xAccuracy, 950}, {items::xSpAtk, 350}, {items::xSpDef, 350}, {items::pokeDoll, 1000}, {items::fluffyTail, 1000}, {items::blueFlute, 20}, {items::yellowFlute, 20}, {items::redFlute, 20}, {items::blackFlute, 20}, {items::whiteFlute, 20}, {items::shoalSalt, 20}, {items::shoalShell, 20}, {items::redShard, 400}, {items::blueShard, 400}, {items::yellowShard, 400}, {items::greenShard, 400}, {items::superRepel, 500}, {items::maxRepel, 700}, {items::escapeRope, 550}, {items::repel, 350}, {items::sunStone, 3000}, {items::moonStone, 3000}, {items::fireStone, 3000}, {items::thunderStone, 3000}, {items::waterStone, 3000}, {items::leafStone, 3000}, {items::shinyStone, 3000}, {items::duskStone, 3000}, {items::dawnStone, 3000}, {items::ovalStone, 3000}, {items::protector, 3000}, {items::electirizer, 3000}, {items::magmarizer, 3000}, {items::dubiousDisc, 3000}, {items::reaperCloth, 3000}, {items::razorClaw, 5000}, {items::razorFang, 5000}, {items::tinyMushroom, 500}, {items::bigMushroom, 5000}, {items::pearl, 1400}, {items::bigPearl, 7500}, {items::stardust, 2000}, {items::starPiece, 9800}, {items::nugget, 10000}, {items::heartScale, 5000}, {items::rareBone, 10000}, {items::honey, 500}, {items::growthMulch, 200}, {items::dampMulch, 200}, {items::stableMulch, 200}, {items::gooeyMulch, 200}, {items::rootFossil, 5000}, {items::clawFossil, 5000}, {items::helixFossil, 5000}, {items::domeFossil, 5000}, {items::oldAmber, 8000}, {items::armorFossil, 5000}, {items::skullFossil, 5000}, {items::oddKeystone, 2100}, {items::griseousOrb, 10000}, {items::adamantOrb, 10000}, {items::lustrousOrb, 10000}, {items::mail1, 50}, {items::mail2, 50}, {items::mail3, 50}, {items::mail4, 50}, {items::mail5, 50}, {items::mail6, 50}, {items::mail7, 50}, {items::mail8, 50}, {items::mail9, 50}, {items::mail10, 50}, {items::mail11, 50}, {items::mail12, 50}, {items::chestoBerry, 250}, {items::pechaBerry, 100}, {items::rawstBerry, 250}, {items::aspearBerry, 250}, {items::leppaBerry, 3000}, {items::oranBerry, 50}, {items::lumBerry, 500}, {items::sitrusBerry, 500}, {items::figyBerry, 100}, {items::wikiBerry, 100}, {items::magoBerry, 100}, {items::aguavBerry, 100}, {items::iapapaBerry, 100}, {items::razzBerry, 500}, {items::blukBerry, 500}, {items::nanabBerry, 500}, {items::wepearBerry, 500}, {items::pinapBerry, 500}, {items::pomegBerry, 500}, {items::kelpsyBerry, 500}, {items::qualotBerry, 500}, {items::hondewBerry, 500}, {items::grepaBerry, 500}, {items::tamatoBerry, 500}, {items::cornnBerry, 500}, {items::magostBerry, 500}, {items::rabutaBerry, 500}, {items::nomelBerry, 500}, {items::spelonBerry, 500}, {items::pamtreBerry, 500}, {items::watmelBerry, 500}, {items::durinBerry, 500}, {items::belueBerry, 500}, {items::occaBerry, 1000}, {items::passhoBerry, 1000}, {items::wacanBerry, 1000}, {items::rindoBerry, 1000}, {items::yacheBerry, 1000}, {items::chopleBerry, 1000}, {items::kebiaBerry, 1000}, {items::shucaBerry, 1000}, {items::cobaBerry, 1000}, {items::payapaBerry, 1000}, {items::tangaBerry, 1000}, {items::chartiBerry, 1000}, {items::kasibBerry, 1000}, {items::habanBerry, 1000}, {items::colburBerry, 1000}, {items::babiriBerry, 1000}, {items::chilanBerry, 1000}, {items::liechiBerry, 1000}, {items::ganlonBerry, 1000}, {items::salacBerry, 1000}, {items::petayaBerry, 1000}, {items::apicotBerry, 1000}, {items::lansatBerry, 1000}, {items::starfBerry, 1000}, {items::enigmaBerry, 1000}, {items::micleBerry, 1000}, {items::custapBerry, 1000}, {items::jabocaBerry, 1000}, {items::rowapBerry, 1000}, {items::brightPowder, 3000}, {items::machoBrace, 3000}, {items::expShare, 6000}, {items::quickClaw, 4500}, {items::choiceBand, 10000}, {items::kingsRock, 5000}, {items::silverPowder, 2000}, {items::amuletCoin, 15000}, {items::cleanseTag, 1000}, {items::soulDew, 200}, {items::deepSeaTooth, 3000}, {items::deepSeaScale, 3000}, {items::smokeBall, 200}, {items::everstone, 200}, {items::focusBand, 3000}, {items::luckyEgg, 10000}, {items::scopeLens, 5000}, {items::metalCoat, 3000}, {items::leftovers, 10000}, {items::dragonScale, 3000}, {items::lightBall, 100}, {items::softSand, 2000}, {items::hardStone, 2000}, {items::miracleSeed, 2000}, {items::blackGlasses, 2000}, {items::blackBelt, 2000}, {items::magnet, 2000}, {items::mysticWater, 2000}, {items::sharpBeak, 2000}, {items::poisonBarb, 2000}, {items::neverMeltIce, 2000}, {items::spellTag, 2000}, {items::twistedSpoon, 2000}, {items::charcoal, 2000}, {items::dragonFang, 2000}, {items::silkScarf, 2000}, {items::upgrade, 3000}, {items::shellBell, 6000}, {items::seaIncense, 2000}, {items::laxIncense, 3000}, {items::luckyPunch, 10}, {items::metalPowder, 10}, {items::thickClub, 500}, {items::leek, 200}, {items::redScarf, 100}, {items::blueScarf, 100}, {items::pinkScarf, 100}, {items::greenScarf, 100}, {items::yellowScarf, 100}, {items::wideLens, 1500}, {items::expertBelt, 6000}, {items::lightClay, 1500}, {items::lifeOrb, 10000}, {items::toxicOrb, 1500}, {items::flameOrb, 1500}, {items::quickPowder, 10}, {items::zoomLens, 1500}, {items::metronome, 3000}, {items::ironBall, 1000}, {items::laggingTail, 1000}, {items::destinyKnot, 1500}, {items::blackSludge, 5000}, {items::icyRock, 200}, {items::smoothRock, 200}, {items::heatRock, 200}, {items::dampRock, 200}, {items::gripClaw, 1500}, {items::choiceScarf, 10000}, {items::stickyBarb, 1500}, {items::powerBracer, 3000}, {items::powerBelt, 3000}, {items::powerLens, 3000}, {items::powerBand, 3000}, {items::powerAnklet, 3000}, {items::powerWeight, 3000}, {items::shedShell, 500}, {items::bigRoot, 1500}, {items::choiceSpecs, 10000}, {items::flamePlate, 2000}, {items::splashPlate, 2000}, {items::zapPlate, 2000}, {items::meadowPlate, 2000}, {items::iciclePlate, 2000}, {items::fistPlate, 2000}, {items::toxicPlate, 2000}, {items::earthPlate, 2000}, {items::skyPlate, 2000}, {items::mindPlate, 2000}, {items::insectPlate, 2000}, {items::stonePlate, 2000}, {items::spookyPlate, 2000}, {items::dracoPlate, 2000}, {items::dreadPlate, 2000}, {items::ironPlate, 2000}, {items::oddIncense, 2000}, {items::rockIncense, 2000}, {items::fullIncense, 1000}, {items::waveIncense, 2000}, {items::roseIncense, 2000}, {items::luckIncense, 15000}, {items::pureIncense, 1000}, {items::tm01, 3000}, {items::tm02, 3000}, {items::tm03, 3000}, {items::tm04, 1500}, {items::tm05, 1000}, {items::tm06, 3000}, {items::tm07, 2000}, {items::tm08, 1500}, {items::tm09, 2000}, {items::tm10, 2000}, {items::tm11, 2000}, {items::tm12, 1500}, {items::tm13, 3000}, {items::tm14, 5500}, {items::tm15, 7500}, {items::tm16, 2000}, {items::tm17, 2000}, {items::tm18, 2000}, {items::tm19, 3000}, {items::tm20, 2000}, {items::tm21, 1000}, {items::tm22, 3000}, {items::tm23, 3000}, {items::tm24, 3000}, {items::tm25, 5500}, {items::tm26, 3000}, {items::tm27, 1000}, {items::tm28, 2000}, {items::tm29, 3000}, {items::tm30, 3000}, {items::tm31, 3000}, {items::tm32, 1000}, {items::tm33, 2000}, {items::tm34, 3000}, {items::tm35, 3000}, {items::tm36, 3000}, {items::tm37, 2000}, {items::tm38, 5500}, {items::tm39, 2000}, {items::tm40, 3000}, {items::tm41, 1500}, {items::tm42, 3000}, {items::tm43, 2000}, {items::tm44, 3000}, {items::tm45, 3000}, {items::tm46, 2000}, {items::tm47, 3000}, {items::tm48, 3000}, {items::tm49, 1500}, {items::tm50, 5500}, {items::tm51, 2000}, {items::tm52, 5500}, {items::tm53, 3000}, {items::tm54, 2000}, {items::tm55, 3000}, {items::tm56, 2000}, {items::tm57, 3000}, {items::tm58, 2000}, {items::tm59, 3000}, {items::tm60, 3000}, {items::tm61, 2000}, {items::tm62, 3000}, {items::tm63, 2000}, {items::tm64, 7500}, {items::tm65, 3000}, {items::tm66, 3000}, {items::tm67, 1000}, {items::tm68, 7500}, {items::tm69, 1500}, {items::tm70, 1000}, {items::tm71, 3000}, {items::tm72, 3000}, {items::tm73, 2000}, {items::tm74, 3000}, {items::tm75, 1500}, {items::tm76, 2000}, {items::tm77, 1500}, {items::tm78, 1500}, {items::tm79, 3000}, {items::tm80, 3000}, {items::tm81, 3000}, {items::tm82, 1000}, {items::tm83, 2000}, {items::tm84, 3000}, {items::tm85, 3000}, {items::tm86, 3000}, {items::tm87, 1500}, {items::tm88, 3000}, {items::tm89, 3000}, {items::tm90, 2000}, {items::tm91, 3000}, {items::tm92, 5500}, {items::mysteryEgg, 0}};
inline constexpr uint8_t typeTableTerminator = 0xFF;
inline constexpr uint8_t typeTableForesightTerminator = 0xFE;
inline constexpr int nonNeutralEffectivenessCount = 110;
inline constexpr int trophyGardenMapIndex = 117;
inline constexpr int nationalParkMapIndex = 23;
inline constexpr int nationalParkBadMapIndex = 24;
inline constexpr int mtCoronetFeebasLakeMapIndex = 10;
inline const std::vector<std::string_view> locationTagsTraverseOrderDPPt = { "TWINLEAF TOWN", "ROUTE 201", "LAKE VERITY", "ROUTE 202", "ROUTE 203", "OREBURGH GATE", "OREBURGH MINE", "ROUTE 204", "RAVAGED PATH", "VALLEY WINDWORKS", "ROUTE 205", "ETERNA FOREST", "OLD CHATEAU", "ETERNA CITY", "ROUTE 206", "WAYWARD CAVE", "ROUTE 207", "ROUTE 208", "ROUTE 209", "SOLACEON RUINS", "ROUTE 210", "ROUTE 215", "ROUTE 214", "MANIAC TUNNEL", "VALOR LAKEFRONT", "ROUTE 213", "PASTORIA CITY", "GREAT MARSH", "ROUTE 212", "TROPHY GARDEN", "CELESTIC TOWN", "FUEGO IRONWORKS", "ROUTE 219", "ROUTE 220", "ROUTE 221", "ROUTE 218", "CANALAVE CITY", "IRON ISLAND", "LAKE VALOR", "ROUTE 211", "MT. CORONET", "ROUTE 216", "ROUTE 217", "ACUITY LAKEFRONT", "LAKE ACUITY", "ROUTE 222", "SUNYSHORE CITY", "ROUTE 223", "VICTORY ROAD", "POKEMON LEAGUE", "ROUTE 224", "ROUTE 230", "ROUTE 229", "RESORT AREA", "ROUTE 228", "ROUTE 226", "ROUTE 227", "ROUTE 225", "STARK MOUNTAIN", "SNOWPOINT TEMPLE", "SENDOFF SPRING", "TURNBACK CAVE", "HONEY TREE", "UNKNOWN" };
inline const std::vector<std::string_view> locationTagsTraverseOrderHGSS = { "NEW BARK TOWN", "ROUTE 29", "ROUTE 46", "CHERRYGROVE CITY", "ROUTE 30", "ROUTE 31", "DARK CAVE", "VIOLET CITY", "SPROUT TOWER", "ROUTE 32", "RUINS OF ALPH", "UNION CAVE", "ROUTE 33", "AZALEA TOWN", "SLOWPOKE WELL", "ILEX FOREST", "ROUTE 34", "GOLDENROD CITY", "ROUTE 35", "NATIONAL PARK", "ROUTE 36", "ROUTE 37", "ECRUTEAK CITY", "BURNED TOWER", "ROUTE 38", "ROUTE 39", "OLIVINE CITY", "ROUTE 40", "ROUTE 41", "CIANWOOD CITY", "CLIFF EDGE GATE", "ROUTE 47", "CLIFF CAVE", "ROUTE 48", "SAFARI ZONE", "ROUTE 42", "MT. MORTAR", "ROUTE 43", "LAKE OF RAGE", "ROUTE 44", "ICE PATH", "BLACKTHORN CITY", "DRAGON'S DEN", "ROUTE 45", "WHIRL ISLANDS", "BELL TOWER", "ROUTE 27", "TOHJO FALLS", "ROUTE 26", "VICTORY ROAD", "VERMILION CITY", "ROUTE 6", "ROUTE 7", "ROUTE 8", "ROCK TUNNEL", "ROUTE 10", "ROUTE 9", "CERULEAN CITY", "ROUTE 24", "ROUTE 25", "ROUTE 5", "CELADON CITY", "ROUTE 16", "ROUTE 17", "ROUTE 18", "FUCHSIA CITY", "ROUTE 15", "ROUTE 14", "ROUTE 13", "ROUTE 12", "ROUTE 11", "DIGLETT'S CAVE", "ROUTE 2", "VIRIDIAN FOREST", "ROUTE 3", "PEWTER CITY", "MT. MOON", "ROUTE 4", "VIRIDIAN CITY", "ROUTE 1", "PALLET TOWN", "ROUTE 21", "CINNABAR ISLAND", "ROUTE 20", "SEAFOAM ISLANDS", "ROUTE 19", "ROUTE 22", "ROUTE 28", "MT. SILVER", "MT. SILVER CAVE", "CERULEAN CAVE", "BUG CATCHING CONTEST" };

// Pendiente de portar a mano:
//   List<Integer> allHeldItems
//   Map<Type, List<Integer>> typeBoostingItems
//   Map<Integer, List<Integer>> moveBoostingItems
//   Map<Type, Integer> weaknessReducingBerries
//   Map<Integer, List<Integer>> speciesBoostingItems
//   Map<Integer, List<Integer>> abilityBoostingItems
//   Map<Integer,List<Integer>> abilityVariations
//   List<String> ptSpecialShopNames
//   List<Integer> ptMainGameShops
//   Map<Integer,FormeInfo> formeMappings
//   Map<Integer,Integer> cosmeticForms
//   Map<Integer,Map<Integer,String>> formeSuffixesByBaseForme
//   Map<Integer,String> dummyFormeSuffixes
//   Map<Integer,Map<Integer,Integer>> absolutePokeNumsByBaseForme
//   Map<Integer,Integer> dummyAbsolutePokeNums
//   Map<Integer, int[][]> otherPokemonGraphicsImagesDP
//   Map<Integer, int[][]> otherPokemonGraphicsImagesPt
//   Map<Integer, int[][]> otherPokemonGraphicsImagesHGSS
//   Map<Integer, int[][]> otherPokemonGraphicsPalettesDP
//   Map<Integer, int[][]> otherPokemonGraphicsPalettesPt
//   Map<Integer, int[][]> otherPokemonGraphicsPalettesHGSS
//   String[] dpptWaterSlotSetNames
//   String[] hgssTimeOfDayNames
//   String[] hgssNonWalkingAreaNames
//   EncounterType[] hgssNonWalkingAreaTypes
//   MoveCategory[] moveCategoryIndices
//   Set<Integer> bannedItems
//   Set<Integer> badItems
//   Set<Integer> opShopItems
//   Type[] typeTable
//   EvolutionType[] evolutionTypeTable
//   HashMap<String, Type> gymAndEliteThemesDP
//   HashMap<String, Type> gymAndEliteThemesPt
//   HashMap<String, Type> gymAndEliteThemesHGSS

}  // namespace pr::gen4
