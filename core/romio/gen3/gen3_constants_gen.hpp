// Generado por tools/convert_constants.py desde Gen3Constants.java (FVX, GPLv3). No editar.
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
inline constexpr std::string_view runningShoesCheckPrefixRS = "0440002C1DD08620", runningShoesCheckPrefixFRLG = "02200540002D29D0", runningShoesCheckPrefixE = "0640002E1BD08C20";
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
inline const std::vector<int> rseEarlyRequiredHMMoves = {moves::rockSmash};
inline const std::vector<int> frlgEarlyRequiredHMMoves = {moves::cut};
inline const std::vector<uint8_t> shopTerminator = {0x00, 0x00, 0x6C, 0x02};
inline const std::vector<std::string_view> rsShopNames = { "Slateport Vitamins", "Slateport TMs", "Oldale Poké Mart (Before Pokédex)", "Oldale Poké Mart (After Pokédex)", "Lavaridge Herbs", "Lavaridge Poké Mart", "Fallarbor Poké Mart", "Verdanturf Poké Mart", "Petalburg Poké Mart (Before 4 Badges)", "Petalburg Poké Mart (After 4 Badges)", "Slateport Poké Mart", "Mauville Poké Mart", "Rustboro Poké Mart (Before Delivering Devon Goods)", "Rustboro Poké Mart (After Delivering Devon Goods)", "Fortree Poké Mart", "Lilycove Department Store 2F Left", "Lilycove Department Store 2F Right", "Lilycove Department Store 3F Left", "Lilycove Department Store 3F Right", "Lilycove Department Store 4F Left (TMs)", "Lilycove Department Store 4F Right (TMs)", "Mossdeep Poké Mart", "Sootopolis Poké Mart", "Pokémon League Poké Mart" };
inline const std::vector<std::string_view> frlgShopNames = { "Trainer Tower Poké Mart", "Two Island Market Stall (Initial)", "Two Island Market Stall (After Saving Lostelle)", "Two Island Market Stall (After Hall of Fame)", "Two Island Market Stall (After Ruby/Sapphire Quest)", "Viridian Poké Mart", "Pewter Poké Mart", "Cerulean Poké Mart", "Lavender Poké Mart", "Vermillion Poké Mart", "Celadon Department 2F South", "Celadon Department 2F North (TMs)", "Celadon Department 4F", "Celadon Department 5F South", "Celadon Department 5F North", "Fuchsia Poké Mart", "Cinnabar Poké Mart", "Indigo Plateau Poké Mart", "Saffron Poké Mart", "Seven Island Poké Mart", "Three Island Poké Mart", "Four Island Poké Mart", "Six Island Poké Mart" };
inline const std::vector<std::string_view> emShopNames = { "Slateport Vitamins", "Slateport TMs", "Oldale Poké Mart (Before Pokédex)", "Oldale Poké Mart (After Pokédex)", "Lavaridge Herbs", "Lavaridge Poké Mart", "Fallarbor Poké Mart", "Verdanturf Poké Mart", "Petalburg Poké Mart (Before 4 Badges)", "Petalburg Poké Mart (After 4 Badges)", "Slateport Poké Mart", "Mauville Poké Mart", "Rustboro Poké Mart (Before Delivering Devon Goods)", "Rustboro Poké Mart (After Delivering Devon Goods)", "Fortree Poké Mart", "Lilycove Department Store 2F Left", "Lilycove Department Store 2F Right", "Lilycove Department Store 3F Left", "Lilycove Department Store 3F Right", "Lilycove Department Store 4F Left (TMs)", "Lilycove Department Store 4F Right (TMs)", "Mossdeep Poké Mart", "Sootopolis Poké Mart", "Pokémon League Poké Mart", "Battle Frontier Poké Mart", "Trainer Hill Poké Mart (Before Hall of Fame)", "Trainer Hill Poké Mart (After Hall of Fame)" };
inline const std::vector<int> evolutionItems = {items::sunStone, items::moonStone, items::fireStone, items::thunderStone, items::waterStone, items::leafStone};
inline const std::vector<int> consumableHeldItems = { items::cheriBerry, items::chestoBerry, items::pechaBerry, items::rawstBerry, items::aspearBerry, items::leppaBerry, items::oranBerry, items::persimBerry, items::lumBerry, items::sitrusBerry, items::figyBerry, items::wikiBerry, items::magoBerry, items::aguavBerry, items::iapapaBerry, items::liechiBerry, items::ganlonBerry, items::salacBerry, items::petayaBerry, items::apicotBerry, items::lansatBerry, items::starfBerry, items::berryJuice, items::whiteHerb, items::mentalHerb };
inline const std::vector<int> generalPurposeConsumableItems = { items::cheriBerry, items::chestoBerry, items::pechaBerry, items::rawstBerry, items::aspearBerry, items::leppaBerry, items::oranBerry, items::persimBerry, items::lumBerry, items::sitrusBerry, items::ganlonBerry, items::salacBerry, items::apicotBerry, items::lansatBerry, items::starfBerry, items::berryJuice, items::whiteHerb, items::mentalHerb };
inline const std::vector<int> generalPurposeItems = { items::brightPowder, items::quickClaw, items::kingsRock, items::focusBand, items::scopeLens, items::leftovers, items::shellBell, items::laxIncense };
inline constexpr int nonNeutralEffectivenessCount = 110;
inline const std::vector<std::string_view> locationTagsTraverseOrderRSE = { "ROUTE 101", "ROUTE 103", "ROUTE 102", "PETALBURG CITY", "ROUTE 104", "PETALBURG WOODS", "ROUTE 116", "RUSTURF TUNNEL", "DEWFORD TOWN", "GRANITE CAVE", "ROUTE 109", "SLATEPORT CITY", "ROUTE 110", "ALTERING CAVE", "MAUVILLE CITY", "NEW MAUVILLE", "ROUTE 117", "ROUTE 111", "MIRAGE TOWER", "ROUTE 112", "FIERY PATH", "ROUTE 113", "ROUTE 114", "METEOR FALLS", "ROUTE 115", "JAGGED PASS", "ROUTE 118", "ROUTE 119", "ROUTE 120", "ROUTE 121", "SAFARI ZONE", "LILYCOVE CITY", "ROUTE 122", "MT. PYRE", "ROUTE 123", "MAGMA HIDEOUT", "ROUTE 124", "MOSSDEEP CITY", "UNDERWATER", "ROUTE 125", "SHOAL CAVE", "ROUTE 127", "ROUTE 128", "SEAFLOOR CAVERN", "ROUTE 126", "SOOTOPOLIS CITY", "CAVE OF ORIGIN", "ROUTE 129", "ROUTE 130", "ROUTE 131", "PACIFIDLOG TOWN", "ROUTE 132", "ROUTE 133", "ROUTE 134", "ROUTE 105", "ROUTE 106", "ROUTE 107", "ROUTE 108", "ABANDONED SHIP", "EVER GRANDE CITY", "VICTORY ROAD", "SKY PILLAR", "DESERT UNDERPASS", "ARTISAN CAVE" };
inline const std::vector<std::string_view> locationTagsTraverseOrderFRLG = { "PALLET TOWN", "ROUTE 1", "VIRIDIAN CITY", "ROUTE 22", "ROUTE 2", "VIRIDIAN FOREST", "ROUTE 3", "MT. MOON", "ROUTE 4", "CERULEAN CITY", "ROUTE 24", "ROUTE 25", "ROUTE 5", "ROUTE 6", "VERMILION CITY", "S.S. ANNE", "ROUTE 11", "DIGLETT'S CAVE", "ROUTE 9", "ROUTE 10", "ROCK TUNNEL", "ROUTE 8", "ROUTE 7", "CELADON CITY", "POKEMON TOWER", "ROUTE 16", "ROUTE 17", "ROUTE 18", "FUCHSIA CITY", "SAFARI ZONE", "ROUTE 15", "ROUTE 14", "ROUTE 13", "ROUTE 12", "POWER PLANT", "ROUTE 19", "ROUTE 20", "SEAFOAM ISLANDS", "CINNABAR ISLAND", "POKEMON MANSION", "ROUTE 21", "ONE ISLAND", "TREASURE BEACH", "THREE ISLE PORT", "BOND BRIDGE", "BERRY FOREST", "CAPE BRINK", "KINDLE ROAD", "MT. EMBER", "ROUTE 23", "VICTORY ROAD", "FOUR ISLAND", "ICEFALL CAVE", "WATER PATH", "GREEN PATH", "PATTERN BUSH", "RUIN VALLEY", "OUTCAST ISLAND", "ALTERING CAVE", "FIVE ISLAND", "FIVE ISLE MEADOW", "MEMORIAL PILLAR", "WATER LABYRINTH", "RESORT GORGEOUS", "LOST CAVE", "TRAINER TOWER", "CANYON ENTRANCE", "SEVAULT CANYON", "TANOBY RUINS", "TANOBY CHAMBERS", "CERULEAN CAVE" };
// HashMap<Integer, Integer> en Java (pares en el orden del código fuente)
inline const std::vector<std::pair<int, int>> balancedItemPrices = {{items::masterBall, 3000}, {items::safariBall, 500}, {items::freshWater, 400}, {items::sodaPop, 600}, {items::lemonade, 700}, {items::moomooMilk, 800}, {items::energyPowder, 400}, {items::energyRoot, 1100}, {items::ether, 3000}, {items::maxEther, 4500}, {items::elixir, 15000}, {items::maxElixir, 18000}, {items::lavaCookie, 450}, {items::blueFlute, 20}, {items::yellowFlute, 20}, {items::redFlute, 20}, {items::blackFlute, 20}, {items::whiteFlute, 20}, {items::sacredAsh, 10000}, {items::redShard, 400}, {items::blueShard, 400}, {items::yellowShard, 400}, {items::greenShard, 400}, {items::rareCandy, 10000}, {items::ppMax, 24900}, {items::sunStone, 3000}, {items::moonStone, 3000}, {items::fireStone, 3000}, {items::thunderStone, 3000}, {items::waterStone, 3000}, {items::leafStone, 3000}, {items::heartScale, 5000}, {items::mail12, 50}, {items::cheriBerry, 200}, {items::chestoBerry, 250}, {items::pechaBerry, 100}, {items::rawstBerry, 250}, {items::aspearBerry, 250}, {items::leppaBerry, 3000}, {items::oranBerry, 50}, {items::persimBerry, 200}, {items::lumBerry, 500}, {items::sitrusBerry, 500}, {items::figyBerry, 100}, {items::wikiBerry, 100}, {items::magoBerry, 100}, {items::aguavBerry, 100}, {items::iapapaBerry, 100}, {items::razzBerry, 500}, {items::blukBerry, 500}, {items::nanabBerry, 500}, {items::wepearBerry, 500}, {items::pinapBerry, 500}, {items::pomegBerry, 500}, {items::kelpsyBerry, 500}, {items::qualotBerry, 500}, {items::hondewBerry, 500}, {items::grepaBerry, 500}, {items::tamatoBerry, 500}, {items::cornnBerry, 500}, {items::magostBerry, 500}, {items::rabutaBerry, 500}, {items::nomelBerry, 500}, {items::spelonBerry, 500}, {items::pamtreBerry, 500}, {items::watmelBerry, 500}, {items::durinBerry, 500}, {items::belueBerry, 500}, {items::liechiBerry, 1000}, {items::ganlonBerry, 1000}, {items::salacBerry, 1000}, {items::petayaBerry, 1000}, {items::apicotBerry, 1000}, {items::lansatBerry, 1000}, {items::starfBerry, 1000}, {items::enigmaBerry, 1000}, {items::brightPowder, 3000}, {items::whiteHerb, 1000}, {items::expShare, 6000}, {items::quickClaw, 4500}, {items::sootheBell, 1000}, {items::mentalHerb, 1000}, {items::choiceBand, 10000}, {items::kingsRock, 5000}, {items::silverPowder, 2000}, {items::amuletCoin, 15000}, {items::cleanseTag, 1000}, {items::deepSeaTooth, 3000}, {items::deepSeaScale, 3000}, {items::focusBand, 3000}, {items::luckyEgg, 10000}, {items::scopeLens, 5000}, {items::metalCoat, 3000}, {items::leftovers, 10000}, {items::dragonScale, 3000}, {items::softSand, 2000}, {items::hardStone, 2000}, {items::miracleSeed, 2000}, {items::blackGlasses, 2000}, {items::blackBelt, 2000}, {items::magnet, 2000}, {items::mysticWater, 2000}, {items::sharpBeak, 2000}, {items::poisonBarb, 2000}, {items::neverMeltIce, 2000}, {items::spellTag, 2000}, {items::twistedSpoon, 2000}, {items::charcoal, 2000}, {items::dragonFang, 2000}, {items::silkScarf, 2000}, {items::upgrade, 3000}, {items::shellBell, 6000}, {items::seaIncense, 2000}, {items::laxIncense, 3000}, {items::tm04, 1500}, {items::tm07, 2000}, {items::tm08, 1500}, {items::tm09, 2000}, {items::tm10, 2000}, {items::tm12, 1500}, {items::tm16, 2000}, {items::tm17, 2000}, {items::tm20, 2000}, {items::tm29, 3000}, {items::tm32, 1000}, {items::tm33, 2000}, {items::tm36, 3000}, {items::tm39, 2000}, {items::tm41, 1500}, {items::tm43, 2000}, {items::tm46, 2000}, {items::tm49, 1500}, {items::tm50, 5500}};
// HashMap<Integer, Integer> en Java (pares en el orden del código fuente)
inline const std::vector<std::pair<int, int>> itemIDToStandardMap = {{0, items::none}, {1, items::masterBall}, {2, items::ultraBall}, {3, items::greatBall}, {4, items::pokeBall}, {5, items::safariBall}, {6, items::netBall}, {7, items::diveBall}, {8, items::nestBall}, {9, items::repeatBall}, {10, items::timerBall}, {11, items::luxuryBall}, {12, items::premierBall}, {13, items::potion}, {14, items::antidote}, {15, items::burnHeal}, {16, items::iceHeal}, {17, items::awakening}, {18, items::paralyzeHeal}, {19, items::fullRestore}, {20, items::maxPotion}, {21, items::hyperPotion}, {22, items::superPotion}, {23, items::fullHeal}, {24, items::revive}, {25, items::maxRevive}, {26, items::freshWater}, {27, items::sodaPop}, {28, items::lemonade}, {29, items::moomooMilk}, {30, items::energyPowder}, {31, items::energyRoot}, {32, items::healPowder}, {33, items::revivalHerb}, {34, items::ether}, {35, items::maxEther}, {36, items::elixir}, {37, items::maxElixir}, {38, items::lavaCookie}, {39, items::blueFlute}, {40, items::yellowFlute}, {41, items::redFlute}, {42, items::blackFlute}, {43, items::whiteFlute}, {44, items::berryJuice}, {45, items::sacredAsh}, {46, items::shoalSalt}, {47, items::shoalShell}, {48, items::redShard}, {49, items::blueShard}, {50, items::yellowShard}, {51, items::greenShard}, {63, items::hpUp}, {64, items::protein}, {65, items::iron}, {66, items::carbos}, {67, items::calcium}, {68, items::rareCandy}, {69, items::ppUp}, {70, items::zinc}, {71, items::ppMax}, {73, items::guardSpec}, {74, items::direHit}, {75, items::xAttack}, {76, items::xDefense}, {77, items::xSpeed}, {78, items::xAccuracy}, {79, items::xSpAtk}, {80, items::pokeDoll}, {81, items::fluffyTail}, {83, items::superRepel}, {84, items::maxRepel}, {85, items::escapeRope}, {86, items::repel}, {93, items::sunStone}, {94, items::moonStone}, {95, items::fireStone}, {96, items::thunderStone}, {97, items::waterStone}, {98, items::leafStone}, {103, items::tinyMushroom}, {104, items::bigMushroom}, {106, items::pearl}, {107, items::bigPearl}, {108, items::stardust}, {109, items::starPiece}, {110, items::nugget}, {111, items::heartScale}, {121, items::mail1}, {122, items::mail2}, {123, items::mail3}, {124, items::mail4}, {125, items::mail5}, {126, items::mail6}, {127, items::mail7}, {128, items::mail8}, {129, items::mail9}, {130, items::mail10}, {131, items::mail11}, {132, items::mail12}, {133, items::cheriBerry}, {134, items::chestoBerry}, {135, items::pechaBerry}, {136, items::rawstBerry}, {137, items::aspearBerry}, {138, items::leppaBerry}, {139, items::oranBerry}, {140, items::persimBerry}, {141, items::lumBerry}, {142, items::sitrusBerry}, {143, items::figyBerry}, {144, items::wikiBerry}, {145, items::magoBerry}, {146, items::aguavBerry}, {147, items::iapapaBerry}, {148, items::razzBerry}, {149, items::blukBerry}, {150, items::nanabBerry}, {151, items::wepearBerry}, {152, items::pinapBerry}, {153, items::pomegBerry}, {154, items::kelpsyBerry}, {155, items::qualotBerry}, {156, items::hondewBerry}, {157, items::grepaBerry}, {158, items::tamatoBerry}, {159, items::cornnBerry}, {160, items::magostBerry}, {161, items::rabutaBerry}, {162, items::nomelBerry}, {163, items::spelonBerry}, {164, items::pamtreBerry}, {165, items::watmelBerry}, {166, items::durinBerry}, {167, items::belueBerry}, {168, items::liechiBerry}, {169, items::ganlonBerry}, {170, items::salacBerry}, {171, items::petayaBerry}, {172, items::apicotBerry}, {173, items::lansatBerry}, {174, items::starfBerry}, {175, items::enigmaBerry}, {179, items::brightPowder}, {180, items::whiteHerb}, {181, items::machoBrace}, {182, items::expShare}, {183, items::quickClaw}, {184, items::sootheBell}, {185, items::mentalHerb}, {186, items::choiceBand}, {187, items::kingsRock}, {188, items::silverPowder}, {189, items::amuletCoin}, {190, items::cleanseTag}, {191, items::soulDew}, {192, items::deepSeaTooth}, {193, items::deepSeaScale}, {194, items::smokeBall}, {195, items::everstone}, {196, items::focusBand}, {197, items::luckyEgg}, {198, items::scopeLens}, {199, items::metalCoat}, {200, items::leftovers}, {201, items::dragonScale}, {202, items::lightBall}, {203, items::softSand}, {204, items::hardStone}, {205, items::miracleSeed}, {206, items::blackGlasses}, {207, items::blackBelt}, {208, items::magnet}, {209, items::mysticWater}, {210, items::sharpBeak}, {211, items::poisonBarb}, {212, items::neverMeltIce}, {213, items::spellTag}, {214, items::twistedSpoon}, {215, items::charcoal}, {216, items::dragonFang}, {217, items::silkScarf}, {218, items::upgrade}, {219, items::shellBell}, {220, items::seaIncense}, {221, items::laxIncense}, {222, items::luckyPunch}, {223, items::metalPowder}, {224, items::thickClub}, {225, items::leek}, {254, items::redScarf}, {255, items::blueScarf}, {256, items::pinkScarf}, {257, items::greenScarf}, {258, items::yellowScarf}, {289, items::tm01}, {290, items::tm02}, {291, items::tm03}, {292, items::tm04}, {293, items::tm05}, {294, items::tm06}, {295, items::tm07}, {296, items::tm08}, {297, items::tm09}, {298, items::tm10}, {299, items::tm11}, {300, items::tm12}, {301, items::tm13}, {302, items::tm14}, {303, items::tm15}, {304, items::tm16}, {305, items::tm17}, {306, items::tm18}, {307, items::tm19}, {308, items::tm20}, {309, items::tm21}, {310, items::tm22}, {311, items::tm23}, {312, items::tm24}, {313, items::tm25}, {314, items::tm26}, {315, items::tm27}, {316, items::tm28}, {317, items::tm29}, {318, items::tm30}, {319, items::tm31}, {320, items::tm32}, {321, items::tm33}, {322, items::tm34}, {323, items::tm35}, {324, items::tm36}, {325, items::tm37}, {326, items::tm38}, {327, items::tm39}, {328, items::tm40}, {329, items::tm41}, {330, items::tm42}, {331, items::tm43}, {332, items::tm44}, {333, items::tm45}, {334, items::tm46}, {335, items::tm47}, {336, items::tm48}, {337, items::tm49}, {338, items::tm50}, {339, items::hm01}, {340, items::hm02}, {341, items::hm03}, {342, items::hm04}, {343, items::hm05}, {344, items::hm06}, {345, items::hm07}, {346, items::hm08}};

// Pendiente de portar a mano:
//   Type[] typeTable
//   Map<Integer,List<Integer>> abilityVariations
//   List<Integer> allHeldItems
//   Map<Type, List<Integer>> typeBoostingItems
//   Map<Integer, List<Integer>> speciesBoostingItems
//   EvolutionType[] evolutionTypeTable
//   Set<Integer> bannedItems
//   Set<Integer> badItemsRSE
//   Set<Integer> badItemsFRLG
//   Set<Integer> opShopItems
//   HashMap<String, Type> gymAndEliteThemesRS
//   HashMap<String, Type> gymAndEliteThemesEm
//   HashMap<String, Type> gymAndEliteThemesFRLG
//   Map<Integer, Integer> itemIDToInternalMap

}  // namespace pr::gen3
