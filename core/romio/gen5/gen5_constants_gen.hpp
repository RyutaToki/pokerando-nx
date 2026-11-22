// Generado por tools/convert_constants.py desde Gen5Constants.java (FVX, GPLv3). No editar.
// Las que necesitan código están en gen5_constants.hpp.
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

namespace pr::gen5 {

inline constexpr int Type_BW = 0;
inline constexpr int Type_BW2 = 1;
inline constexpr int CLASS_IRIS = 55;
inline constexpr int CLASS_DRAYDEN = 56;
inline constexpr int arm9Offset = 0x02004000;
inline constexpr int pokemonCount = 649;
inline constexpr int moveCount = 559;
inline constexpr int bw1FormeCount = 18;
inline constexpr int bw2FormeCount = 24;
inline constexpr int bw1formeOffset = 0;
inline constexpr int bw2formeOffset = 35;
inline constexpr int bw1NonPokemonBattleSpriteCount = 3;
inline constexpr int bw2NonPokemonBattleSpriteCount = 36;
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
inline constexpr int bsDarkGrassHeldItemOffset = 16;
inline constexpr int bsGenderRatioOffset = 18;
inline constexpr int bsEggCyclesOffset = 19;
inline constexpr int bsGrowthCurveOffset = 21;
inline constexpr int bsPrimaryEggGroupOffset = 22;
inline constexpr int bsSecondaryEggGroupOffset = 23;
inline constexpr int bsAbility1Offset = 24;
inline constexpr int bsAbility2Offset = 25;
inline constexpr int bsAbility3Offset = 26;
inline constexpr int bsFormeOffset = 28;
inline constexpr int bsFormeSpriteOffset = 30;
inline constexpr int bsFormeCountOffset = 32;
inline constexpr int bsTMHMCompatOffset = 40;
inline constexpr int bsMTCompatOffset = 60;
inline const std::vector<uint8_t> bw1NewStarterScript = { 0x24, 0x00, 0xA7, 0x02, 0xE7, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x00, 0x00, 0xF8, 0x01, 0x05, 0x00 };
inline constexpr std::string_view bw1StarterScriptMagic = "2400A702";
inline constexpr int bw1StarterTextOffset = 18;
inline constexpr int bw1CherenText1Offset = 26;
inline constexpr int bw1CherenText2Offset = 53;
inline const std::vector<uint8_t> bw2NewStarterScript = { 0x28, 0x00, 0xA1, 0x40, 0x04, 0x00, 0xDE, 0x00, 0x00, 0x00, 0xFD, 0x01, 0x05, 0x00 };
inline constexpr std::string_view bw2StarterScriptMagic = "2800A1400400";
inline constexpr int bw2StarterTextOffset = 37;
inline constexpr int bw2RivalTextOffset = 60;
inline constexpr int perSeasonEncounterDataLength = 232;
inline constexpr int bw1AreaDataEntryLength = 249;
inline constexpr int bw2AreaDataEntryLength = 345;
inline constexpr int bw1EncounterAreaCount = 61;
inline constexpr int bw2EncounterAreaCount = 85;
inline const std::vector<int> encountersOfEachType = { 12, 12, 12, 5, 5, 5, 5 };
inline const std::vector<int> habitatClassificationOfEachType = { 0, 0, 0, 1, 1, 2, 2 };
inline constexpr int bw2Route4AreaIndex = 40;
inline constexpr int bw2VictoryRoadAreaIndex = 76;
inline constexpr int bw2ReversalMountainAreaIndex = 73;
inline constexpr int b2Route4EncounterFile = 104;
inline constexpr int b2VRExclusiveRoom1 = 71;
inline constexpr int b2VRExclusiveRoom2 = 73;
inline constexpr int b2ReversalMountainStart = 49;
inline constexpr int b2ReversalMountainEnd = 54;
inline constexpr int w2Route4EncounterFile = 105;
inline constexpr int w2VRExclusiveRoom1 = 78;
inline constexpr int w2VRExclusiveRoom2 = 79;
inline constexpr int w2ReversalMountainStart = 55;
inline constexpr int w2ReversalMountainEnd = 60;
inline const std::vector<int> bw2HiddenHollowUnovaPokemon = {species::watchog, species::herdier, species::liepard, species::pansage, species::pansear, species::panpour, species::pidove, species::zebstrika, species::boldore, species::woobat, species::drilbur, species::audino, species::gurdurr, species::tympole, species::throh, species::sawk, species::leavanny, species::scolipede, species::cottonee, species::petilil, species::basculin, species::krookodile, species::maractus, species::crustle, species::scraggy, species::sigilyph, species::tirtouga, species::garbodor, species::minccino, species::gothorita, species::duosion, species::ducklett, species::vanillish, species::emolga, species::karrablast, species::alomomola, species::galvantula, species::klinklang, species::elgyem, species::litwick, species::axew, species::cubchoo, species::shelmet, species::stunfisk, species::mienfoo, species::druddigon, species::golett, species::pawniard, species::bouffalant, species::braviary, species::mandibuzz, species::heatmor, species::durant};
inline constexpr std::string_view tmDataPrefix = "87038803";
inline constexpr int tmCount = 95;
inline constexpr int hmCount = 6;
inline constexpr int tmBlockOneCount = 92;
inline constexpr int tmBlockOneOffset = items::tm01;
inline constexpr int tmBlockTwoOffset = items::tm93;
inline constexpr std::string_view bw1ItemPalettesPrefix = "E903EA03020003000400050006000700", bw2ItemPalettesPrefix = "FD03FE03020003000400050006000700";
inline constexpr int bw2MoveTutorCount = 60;
inline constexpr int bw2MoveTutorBytesPerEntry = 12;
inline constexpr int evolutionMethodCount = 27;
inline constexpr int highestAbilityIndex = abilities::teravolt;
inline constexpr int fossilPokemonFile = 877;
inline constexpr int fossilPokemonLevelOffset = 0x3F7;
inline constexpr std::string_view hmsForgettableBefore = "08 4A 00 23 59 00 51 18 B8 31 09 88 88 42 01 D1 01 20 70 47 59 1C 09 06 0B 0E 06 2B F2 D3 00 20 70 47 C0 46", hmsForgettableAfter = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 20 70 47 00 00 00 00 00 00";
inline const std::vector<int> uselessAbilities = {abilities::forecast, abilities::multitype, abilities::flowerGift, abilities::zenMode};
inline constexpr int normalItemSetVarCommand = 0x28;
inline constexpr int hiddenItemSetVarCommand = 0x2A;
inline constexpr int normalItemVarSet = 0x800C;
inline constexpr int hiddenItemVarSet = 0x8000;
inline constexpr int scriptListTerminator = 0xFD13;
inline constexpr int trappingEffect = 106;
inline constexpr int noDamageStatusQuality = 1;
inline constexpr int noDamageStatChangeQuality = 2;
inline constexpr int damageStatusQuality = 4;
inline constexpr int noDamageStatusAndStatChangeQuality = 5;
inline constexpr int damageTargetDebuffQuality = 6;
inline constexpr int damageUserBuffQuality = 7;
inline constexpr int damageAbsorbQuality = 8;
inline const std::vector<int> bw1IrregularFormes = { species::Gen5Formes::castformSu, species::Gen5Formes::castformR, species::Gen5Formes::castformSn, species::Gen5Formes::darmanitanZ, species::Gen5Formes::meloettaP };
inline const std::vector<int> bw2IrregularFormes = { species::Gen5Formes::castformSu, species::Gen5Formes::castformR, species::Gen5Formes::castformSn, species::Gen5Formes::darmanitanZ, species::Gen5Formes::meloettaP, species::Gen5Formes::kyuremW, species::Gen5Formes::kyuremB };
inline const std::vector<int> ignoreTrueCosmeticFormes = { species::cherrim, species::arceus, species::genesect, species::deerling, species::sawsbuck };
inline const std::vector<int> bw1MainPlaythroughTrainers = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 186, 187, 188, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 212, 213, 214, 215, 216, 217, 218, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 315, 316, 401, 402, 408, 409, 412, 413, 438, 439, 441, 442, 443, 445, 447, 450, 460, 461, 462, 465, 466, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 484, 485, 488, 489, 490, 501, 502, 503, 504, 505, 506, 513, 514, 515, 516, 517, 518, 519, 520, 526, 531, 532, 533, 534, 535, 536, 537, 538, 544, 545, 546, 549, 550, 552, 553, 554, 555, 556, 557, 582, 583, 584, 585, 586, 587, 600, 601, 602, 603, 604, 605, 606, 607, 610, 611, 612, 613};
inline const std::vector<int> bw2MainPlaythroughTrainers = { 4, 5, 6, 133, 134, 135, 136, 137, 138, 139, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 164, 165, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 237, 238, 239, 240, 242, 243, 244, 245, 247, 248, 249, 250, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 372, 373, 374, 375, 376, 377, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 497, 498, 499, 500, 501, 502, 503, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 580, 581, 583, 584, 585, 586, 587, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 679, 680, 681, 682, 683, 690, 691, 692, 703, 704, 705, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 745, 746, 747, 748, 749, 750, 751, 752, 754, 755, 756, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 786, 787, 788, 789, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812};
inline const std::vector<int> bw2DriftveilTrainerOffsets = {56, 57, 0, 1, 2, 3, 4, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77};
inline constexpr int normalTrainerNameLength = 813;
inline constexpr int normalTrainerClassLength = 236;
inline const std::vector<int> bw1MainGameShops = { 3, 5, 6, 8, 9, 12, 14, 17, 18, 19, 21, 22 };
inline const std::vector<std::string_view> bw1ShopNames = { "Primary 0 Badges", "Primary 2 Badges", "Primary 3 Badges", "Primary 5 Badges", "Primary 7 Badges", "Primary 8 Badges", "Accumula Secondary", "Striaton Secondary", "Nacrene Secondary", "Castelia Secondary", "Nimbasa Secondary (TMs)", "Driftveil Secondary", "Mistralton Secondary (TMs)", "Icirrus Secondary (TMs)", "Opelucid Secondary", "League Secondary", "Lacunosa Secondary", "Undella Secondary", "Black City/White Forest Secondary", "Nacrene/Shopping Mall 9 X Items", "Driftveil Herb Salesman", "Driftveil Incense Salesman", "Shopping Mall 9 F1", "Shopping Mall 9 TMs", "Shopping Mall 9 F2", "Shopping Mall 9 F3 Left"};
inline const std::vector<int> bw2MainGameShops = { 9, 11, 14, 15, 16, 18, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30, 31 };
inline const std::vector<std::string_view> bw2ShopNames = { "Primary 0 Badges", "Primary 1 Badges", "Primary 3 Badges", "Primary 5 Badges", "Primary 7 Badges", "Primary 8 Badges", "Accumula Secondary", "Striaton Secondary (TMs)", "Nacrene Secondary", "Castelia Secondary", "Nimbasa Secondary (TMs)", "Driftveil Secondary", "Mistralton Secondary (TMs)", "Icirrus Secondary", "Opelucid Secondary", "Victory Road Secondary", "Pokemon League Secondary", "Lacunosa Secondary (TMs)", "Undella Secondary", "Black City/White Forest Secondary", "Nacrene/Shopping Mall 9 X Items", "Driftveil Herb Salesman", "Driftveil Incense Salesman", "Shopping Mall 9 F1", "Shopping Mall 9 TMs", "Shopping Mall 9 F2", "Shopping Mall 9 F3 Left", "Aspertia Secondary", "Virbank Secondary", "Humilau Secondary", "Floccesy Secondary", "Lentimas Secondary"};
inline const std::vector<int> evolutionItems = {items::sunStone, items::moonStone, items::fireStone, items::thunderStone, items::waterStone, items::leafStone, items::shinyStone, items::duskStone, items::dawnStone, items::ovalStone, items::kingsRock, items::deepSeaTooth, items::deepSeaScale, items::metalCoat, items::dragonScale, items::upgrade, items::protector, items::electirizer, items::magmarizer, items::dubiousDisc, items::reaperCloth, items::razorClaw, items::razorFang, items::prismScale};
inline const std::vector<int> bw1RequiredFieldTMs = {items::tm02, items::tm03, items::tm05, items::tm06, items::tm09, items::tm12, items::tm13, items::tm19, items::tm22, items::tm24, items::tm26, items::tm29, items::tm30, items::tm35, items::tm36, items::tm39, items::tm41, items::tm46, items::tm47, items::tm50, items::tm52, items::tm53, items::tm55, items::tm58, items::tm61, items::tm63, items::tm65, items::tm66, items::tm71, items::tm80, items::tm81, items::tm84, items::tm85, items::tm86, items::tm90, items::tm91, items::tm92, items::tm93};
inline const std::vector<int> bw2RequiredFieldTMs = {items::tm01, items::tm02, items::tm03, items::tm05, items::tm06, items::tm12, items::tm13, items::tm19, items::tm22, items::tm26, items::tm28, items::tm29, items::tm30, items::tm36, items::tm39, items::tm41, items::tm46, items::tm47, items::tm50, items::tm52, items::tm53, items::tm56, items::tm58, items::tm61, items::tm63, items::tm65, items::tm66, items::tm67, items::tm69, items::tm71, items::tm80, items::tm81, items::tm84, items::tm85, items::tm86, items::tm90, items::tm91, items::tm92, items::tm93};
inline const std::vector<int> bw1EarlyRequiredHMMoves = {moves::cut};
inline const std::vector<int> fieldMoves = { moves::cut, moves::fly, moves::surf, moves::strength, moves::flash, moves::dig, moves::teleport, moves::waterfall, moves::sweetScent, moves::dive};
inline constexpr std::string_view shedinjaSpeciesLocator = "24010000";
inline constexpr std::string_view runningShoesPrefix = "01D0012008BD002008BD63";
inline constexpr std::string_view introGraphicPrefix = "5A0000010000001700000001000000", bw1IntroCryPrefix = "0021009101910291", bw2IntroCryLocator = "3D020000F8B51C1C";
inline constexpr std::string_view typeEffectivenessTableLocator = "0404040404020400";
inline constexpr std::string_view forceChallengeModeLocator = "816A406B0B1C07490022434090000858834201D1";
inline constexpr std::string_view pickupTableLocator = "19005C00DD00";
inline constexpr int numberOfPickupItems = 29;
inline constexpr std::string_view friendshipValueForEvoLocator = "DC282FD3";
inline constexpr std::string_view perfectOddsBranchLocator = "08DB002801D0012000E0";
inline constexpr std::string_view lowHealthMusicLocator = "00D10127";
// HashMap<Integer, Integer> en Java (pares en el orden del código fuente)
inline const std::vector<std::pair<int, int>> balancedItemPrices = {{items::masterBall, 3000}, {items::ultraBall, 1200}, {items::greatBall, 600}, {items::pokeBall, 200}, {items::safariBall, 500}, {items::netBall, 1000}, {items::diveBall, 1000}, {items::nestBall, 1000}, {items::repeatBall, 1000}, {items::timerBall, 1000}, {items::luxuryBall, 1000}, {items::premierBall, 200}, {items::duskBall, 1000}, {items::healBall, 300}, {items::quickBall, 1000}, {items::cherishBall, 200}, {items::fastBall, 300}, {items::levelBall, 300}, {items::lureBall, 300}, {items::heavyBall, 300}, {items::loveBall, 300}, {items::friendBall, 300}, {items::moonBall, 300}, {items::sportBall, 300}, {items::redApricorn, 20}, {items::blueApricorn, 20}, {items::yellowApricorn, 20}, {items::greenApricorn, 20}, {items::pinkApricorn, 20}, {items::whiteApricorn, 20}, {items::blackApricorn, 20}, {items::potion, 300}, {items::antidote, 100}, {items::burnHeal, 250}, {items::iceHeal, 250}, {items::awakening, 250}, {items::paralyzeHeal, 200}, {items::fullRestore, 3000}, {items::maxPotion, 2500}, {items::hyperPotion, 1200}, {items::superPotion, 700}, {items::fullHeal, 600}, {items::revive, 1500}, {items::maxRevive, 4000}, {items::freshWater, 400}, {items::sodaPop, 600}, {items::lemonade, 700}, {items::moomooMilk, 800}, {items::energyPowder, 400}, {items::energyRoot, 1100}, {items::healPowder, 450}, {items::revivalHerb, 2800}, {items::ether, 3000}, {items::maxEther, 4500}, {items::elixir, 15000}, {items::maxElixir, 18000}, {items::lavaCookie, 450}, {items::berryJuice, 100}, {items::sacredAsh, 10000}, {items::oldGateau, 450}, {items::rageCandyBar, 15000}, {items::casteliacone, 450}, {items::hpUp, 9800}, {items::protein, 9800}, {items::iron, 9800}, {items::carbos, 9800}, {items::calcium, 9800}, {items::rareCandy, 10000}, {items::ppUp, 9800}, {items::zinc, 9800}, {items::ppMax, 24900}, {items::healthFeather, 3000}, {items::muscleFeather, 3000}, {items::resistFeather, 3000}, {items::geniusFeather, 3000}, {items::cleverFeather, 3000}, {items::swiftFeather, 3000}, {items::prettyFeather, 200}, {items::guardSpec, 700}, {items::direHit, 650}, {items::xAttack, 500}, {items::xDefense, 550}, {items::xSpeed, 350}, {items::xAccuracy, 950}, {items::xSpAtk, 350}, {items::xSpDef, 350}, {items::pokeDoll, 1000}, {items::fluffyTail, 1000}, {items::blueFlute, 20}, {items::yellowFlute, 20}, {items::redFlute, 20}, {items::blackFlute, 20}, {items::whiteFlute, 20}, {items::shoalSalt, 20}, {items::shoalShell, 20}, {items::redShard, 400}, {items::blueShard, 400}, {items::yellowShard, 400}, {items::greenShard, 400}, {items::superRepel, 500}, {items::maxRepel, 700}, {items::escapeRope, 550}, {items::repel, 350}, {items::sunStone, 3000}, {items::moonStone, 3000}, {items::fireStone, 3000}, {items::thunderStone, 3000}, {items::waterStone, 3000}, {items::leafStone, 3000}, {items::shinyStone, 3000}, {items::duskStone, 3000}, {items::dawnStone, 3000}, {items::ovalStone, 3000}, {items::prismScale, 3000}, {items::tinyMushroom, 500}, {items::bigMushroom, 5000}, {items::pearl, 1400}, {items::bigPearl, 7500}, {items::stardust, 2000}, {items::starPiece, 9800}, {items::nugget, 10000}, {items::heartScale, 5000}, {items::rareBone, 10000}, {items::honey, 500}, {items::growthMulch, 200}, {items::dampMulch, 200}, {items::stableMulch, 200}, {items::gooeyMulch, 200}, {items::rootFossil, 5000}, {items::clawFossil, 5000}, {items::helixFossil, 5000}, {items::domeFossil, 5000}, {items::oldAmber, 8000}, {items::armorFossil, 5000}, {items::skullFossil, 5000}, {items::coverFossil, 5000}, {items::plumeFossil, 5000}, {items::oddKeystone, 2100}, {items::griseousOrb, 10000}, {items::adamantOrb, 10000}, {items::lustrousOrb, 10000}, {items::mail1, 50}, {items::mail2, 50}, {items::mail3, 50}, {items::mail4, 50}, {items::mail5, 50}, {items::mail6, 50}, {items::mail7, 50}, {items::mail8, 50}, {items::mail9, 50}, {items::mail10, 50}, {items::mail11, 50}, {items::mail12, 50}, {items::chestoBerry, 250}, {items::pechaBerry, 100}, {items::rawstBerry, 250}, {items::aspearBerry, 250}, {items::leppaBerry, 3000}, {items::oranBerry, 50}, {items::lumBerry, 500}, {items::sitrusBerry, 500}, {items::figyBerry, 100}, {items::wikiBerry, 100}, {items::magoBerry, 100}, {items::aguavBerry, 100}, {items::iapapaBerry, 100}, {items::razzBerry, 500}, {items::blukBerry, 500}, {items::nanabBerry, 500}, {items::wepearBerry, 500}, {items::pinapBerry, 500}, {items::pomegBerry, 500}, {items::kelpsyBerry, 500}, {items::qualotBerry, 500}, {items::hondewBerry, 500}, {items::grepaBerry, 500}, {items::tamatoBerry, 500}, {items::cornnBerry, 500}, {items::magostBerry, 500}, {items::rabutaBerry, 500}, {items::nomelBerry, 500}, {items::spelonBerry, 500}, {items::pamtreBerry, 500}, {items::watmelBerry, 500}, {items::durinBerry, 500}, {items::belueBerry, 500}, {items::occaBerry, 1000}, {items::passhoBerry, 1000}, {items::wacanBerry, 1000}, {items::rindoBerry, 1000}, {items::yacheBerry, 1000}, {items::chopleBerry, 1000}, {items::kebiaBerry, 1000}, {items::shucaBerry, 1000}, {items::cobaBerry, 1000}, {items::payapaBerry, 1000}, {items::tangaBerry, 1000}, {items::chartiBerry, 1000}, {items::kasibBerry, 1000}, {items::habanBerry, 1000}, {items::colburBerry, 1000}, {items::babiriBerry, 1000}, {items::chilanBerry, 1000}, {items::liechiBerry, 1000}, {items::ganlonBerry, 1000}, {items::salacBerry, 1000}, {items::petayaBerry, 1000}, {items::apicotBerry, 1000}, {items::lansatBerry, 1000}, {items::starfBerry, 1000}, {items::enigmaBerry, 1000}, {items::micleBerry, 1000}, {items::custapBerry, 1000}, {items::jabocaBerry, 1000}, {items::rowapBerry, 1000}, {items::brightPowder, 3000}, {items::machoBrace, 3000}, {items::expShare, 6000}, {items::quickClaw, 4500}, {items::choiceBand, 10000}, {items::kingsRock, 5000}, {items::silverPowder, 2000}, {items::amuletCoin, 15000}, {items::cleanseTag, 1000}, {items::soulDew, 200}, {items::deepSeaTooth, 3000}, {items::deepSeaScale, 3000}, {items::smokeBall, 200}, {items::everstone, 200}, {items::focusBand, 3000}, {items::luckyEgg, 10000}, {items::scopeLens, 5000}, {items::metalCoat, 3000}, {items::leftovers, 10000}, {items::dragonScale, 3000}, {items::lightBall, 100}, {items::softSand, 2000}, {items::hardStone, 2000}, {items::miracleSeed, 2000}, {items::blackGlasses, 2000}, {items::blackBelt, 2000}, {items::magnet, 2000}, {items::mysticWater, 2000}, {items::sharpBeak, 2000}, {items::poisonBarb, 2000}, {items::neverMeltIce, 2000}, {items::spellTag, 2000}, {items::twistedSpoon, 2000}, {items::charcoal, 2000}, {items::dragonFang, 2000}, {items::silkScarf, 2000}, {items::upgrade, 3000}, {items::shellBell, 6000}, {items::seaIncense, 2000}, {items::laxIncense, 3000}, {items::luckyPunch, 10}, {items::metalPowder, 10}, {items::thickClub, 500}, {items::leek, 200}, {items::redScarf, 100}, {items::blueScarf, 100}, {items::pinkScarf, 100}, {items::greenScarf, 100}, {items::yellowScarf, 100}, {items::wideLens, 1500}, {items::expertBelt, 6000}, {items::lightClay, 1500}, {items::lifeOrb, 10000}, {items::toxicOrb, 1500}, {items::flameOrb, 1500}, {items::quickPowder, 10}, {items::zoomLens, 1500}, {items::metronome, 3000}, {items::ironBall, 1000}, {items::laggingTail, 1000}, {items::destinyKnot, 1500}, {items::blackSludge, 5000}, {items::icyRock, 200}, {items::smoothRock, 200}, {items::heatRock, 200}, {items::dampRock, 200}, {items::gripClaw, 1500}, {items::choiceScarf, 10000}, {items::stickyBarb, 1500}, {items::powerBracer, 3000}, {items::powerBelt, 3000}, {items::powerLens, 3000}, {items::powerBand, 3000}, {items::powerAnklet, 3000}, {items::powerWeight, 3000}, {items::shedShell, 500}, {items::bigRoot, 1500}, {items::choiceSpecs, 10000}, {items::flamePlate, 2000}, {items::splashPlate, 2000}, {items::zapPlate, 2000}, {items::meadowPlate, 2000}, {items::iciclePlate, 2000}, {items::fistPlate, 2000}, {items::toxicPlate, 2000}, {items::earthPlate, 2000}, {items::skyPlate, 2000}, {items::mindPlate, 2000}, {items::insectPlate, 2000}, {items::stonePlate, 2000}, {items::spookyPlate, 2000}, {items::dracoPlate, 2000}, {items::dreadPlate, 2000}, {items::ironPlate, 2000}, {items::oddIncense, 2000}, {items::rockIncense, 2000}, {items::fullIncense, 1000}, {items::waveIncense, 2000}, {items::roseIncense, 2000}, {items::luckIncense, 15000}, {items::pureIncense, 1000}, {items::protector, 3000}, {items::electirizer, 3000}, {items::magmarizer, 3000}, {items::dubiousDisc, 3000}, {items::reaperCloth, 3000}, {items::razorClaw, 5000}, {items::razorFang, 5000}, {items::eviolite, 10000}, {items::floatStone, 1000}, {items::rockyHelmet, 6000}, {items::airBalloon, 1000}, {items::redCard, 1000}, {items::ringTarget, 1000}, {items::absorbBulb, 1000}, {items::cellBattery, 1000}, {items::ejectButton, 1000}, {items::fireGem, 1000}, {items::waterGem, 1000}, {items::electricGem, 1000}, {items::grassGem, 1000}, {items::iceGem, 1000}, {items::fightingGem, 1000}, {items::poisonGem, 1000}, {items::groundGem, 1000}, {items::flyingGem, 1000}, {items::psychicGem, 1000}, {items::bugGem, 1000}, {items::rockGem, 1000}, {items::ghostGem, 1000}, {items::dragonGem, 1000}, {items::darkGem, 1000}, {items::steelGem, 1000}, {items::normalGem, 1000}, {items::passOrb, 200}, {items::dreamBall, 1000}, {items::pokeToy, 1000}, {items::tm01, 10000}, {items::tm02, 10000}, {items::tm03, 10000}, {items::tm04, 10000}, {items::tm05, 10000}, {items::tm06, 10000}, {items::tm07, 20000}, {items::tm08, 10000}, {items::tm09, 10000}, {items::tm10, 10000}, {items::tm11, 20000}, {items::tm12, 10000}, {items::tm13, 10000}, {items::tm14, 20000}, {items::tm15, 20000}, {items::tm16, 20000}, {items::tm17, 10000}, {items::tm18, 20000}, {items::tm19, 10000}, {items::tm20, 20000}, {items::tm21, 10000}, {items::tm22, 10000}, {items::tm23, 10000}, {items::tm24, 10000}, {items::tm25, 20000}, {items::tm26, 10000}, {items::tm27, 10000}, {items::tm28, 10000}, {items::tm29, 10000}, {items::tm30, 10000}, {items::tm31, 10000}, {items::tm32, 10000}, {items::tm33, 20000}, {items::tm34, 10000}, {items::tm35, 10000}, {items::tm36, 10000}, {items::tm37, 20000}, {items::tm38, 20000}, {items::tm39, 10000}, {items::tm40, 10000}, {items::tm41, 10000}, {items::tm42, 10000}, {items::tm43, 10000}, {items::tm44, 10000}, {items::tm45, 10000}, {items::tm46, 10000}, {items::tm47, 10000}, {items::tm48, 10000}, {items::tm49, 10000}, {items::tm50, 10000}, {items::tm51, 10000}, {items::tm52, 10000}, {items::tm53, 10000}, {items::tm54, 10000}, {items::tm55, 10000}, {items::tm56, 10000}, {items::tm57, 10000}, {items::tm58, 10000}, {items::tm59, 10000}, {items::tm60, 10000}, {items::tm61, 10000}, {items::tm62, 10000}, {items::tm63, 10000}, {items::tm64, 10000}, {items::tm65, 10000}, {items::tm66, 10000}, {items::tm67, 10000}, {items::tm68, 20000}, {items::tm69, 10000}, {items::tm70, 10000}, {items::tm71, 10000}, {items::tm72, 10000}, {items::tm73, 10000}, {items::tm74, 10000}, {items::tm75, 10000}, {items::tm76, 10000}, {items::tm77, 10000}, {items::tm78, 10000}, {items::tm79, 10000}, {items::tm80, 10000}, {items::tm81, 10000}, {items::tm82, 10000}, {items::tm83, 10000}, {items::tm84, 10000}, {items::tm85, 10000}, {items::tm86, 10000}, {items::tm87, 10000}, {items::tm88, 10000}, {items::tm89, 10000}, {items::tm90, 10000}, {items::tm91, 10000}, {items::tm92, 10000}, {items::tm93, 10000}, {items::tm94, 10000}, {items::tm95, 10000}};
inline const std::vector<int> bw1WildFileToAreaMap = { 2, 6, 8, 18, 18, 19, 19, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 29, 36, 57, 59, 60, 38, 39, 40, 30, 30, 41, 42, 43, 31, 31, 31, 44, 33, 33, 33, 33, 45, 34, 46, 32, 32, 32, 47, 47, 48, 49, 50, 51, 35, 52, 53, 37, 55, 12, 54, };
inline const std::vector<int> bw2WildFileToAreaMap = { 2, 4, 8, 59, 61, 63, 19, 19, 20, 20, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 76, 27, 27, 27, 27, 27, 70, 70, 70, 70, 70, 29, 35, 71, 71, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77, 77, 79, 79, 79, 79, 79, 79, 79, 79, 79, 78, 78, -1, 55, 57, 58, 37, 38, 39, 30, 30, 40, 40, 41, 42, 31, 31, 31, 43, 32, 32, 32, 32, 44, 33, 45, 46, 47, 48, 49, 34, 50, 51, 36, 53, 66, 67, 69, 75, 12, 52, 68, };
inline const std::vector<std::string_view> locationTagsTraverseOrderBW = { "ROUTE 1", "ROUTE 2", "STRIATON CITY", "DREAMYARD", "ROUTE 3", "WELLSPRING CAVE", "PINWHEEL FOREST", "CASTELIA CITY", "ROUTE 4", "DESERT RESORT", "ROUTE 5", "RELIC CASTLE", "ROUTE 5", "DRIFTVEIL DRAWBRIDGE", "DRIFTVEIL CITY", "COLD STORAGE", "ROUTE 6", "CHARGESTONE CAVE", "ROUTE 7", "CELESTIAL TOWER", "ROUTE 17", "ROUTE 18", "P2 LABORATORY", "MISTRALTON CAVE", "TWIST MOUNTAIN", "ICIRRUS CITY", "DRAGONSPIRAL TOWER", "ROUTE 8", "MOOR OF ICIRRUS", "ROUTE 9", "ROUTE 10", "VICTORY ROAD", "CHALLENGER'S CAVE", "ROUTE 11", "VILLAGE BRIDGE", "ROUTE 12", "ROUTE 13", "GIANT CHASM", "UNDELLA TOWN", "UNDELLA BAY", "ROUTE 14", "ABUNDANT SHRINE", "ROUTE 15", "MARVELOUS BRIDGE", "ROUTE 16", "LOSTLORN FOREST" };
inline const std::vector<std::string_view> locationTagsTraverseOrderBW2 = { "ASPERTIA CITY", "ROUTE 19", "ROUTE 20", "FLOCCESY RANCH", "VIRBANK CITY", "VIRBANK COMPLEX", "CASTELIA CITY", "CASTELIA SEWERS", "ROUTE 4", "DESERT RESORT", "RELIC CASTLE", "ROUTE 5", "ROUTE 16", "LOSTLORN FOREST", "DRIFTVEIL DRAWBRIDGE", "ROUTE 6", "RELIC PASSAGE", "CLAY TUNNEL", "MISTRALTON CAVE", "CHARGESTONE CAVE", "ROUTE 7", "CELESTIAL TOWER", "REVERSAL MOUNTAIN", "STRANGE HOUSE", "UNDELLA TOWN", "ROUTE 13", "UNDELLA BAY", "ROUTE 14", "ABUNDANT SHRINE", "ROUTE 12", "VILLAGE BRIDGE", "ROUTE 11", "ROUTE 9", "SEASIDE CAVE", "ROUTE 21", "ROUTE 15", "MARVELOUS BRIDGE", "HUMILAU CITY", "ROUTE 22", "GIANT CHASM", "ROUTE 23", "VICTORY ROAD", "ROUTE 8", "MOOR OF ICIRRUS", "ICIRRUS CITY", "DRAGONSPIRAL TOWER", "TWIST MOUNTAIN", "PINWHEEL FOREST", "ROUTE 3", "WELLSPRING CAVE", "STRIATON CITY", "DREAMYARD", "ROUTE 2", "ROUTE 1", "ROUTE 17", "ROUTE 18", "P2 LABORATORY", "NATURE PRESERVE" };

// Pendiente de portar a mano:
//   String[] encounterTypeNames
//   EncounterType[] encounterTypeValues
//   Map<Integer,List<Integer>> abilityVariations
//   MoveCategory[] moveCategoryIndices
//   Type[] typeTable
//   Map<Integer,Map<Integer,String>> formeSuffixesByBaseForme
//   Map<Integer,String> dummyFormeSuffixes
//   Map<Integer,Map<Integer,Integer>> absolutePokeNumsByBaseForme
//   Map<Integer,Integer> dummyAbsolutePokeNums
//   List<Integer> emptyPlaythroughTrainers
//   List<Integer> bw2EarlyRequiredHMMoves
//   List<Integer> consumableHeldItems
//   List<Integer> allHeldItems
//   List<Integer> generalPurposeConsumableItems
//   List<Integer> generalPurposeItems
//   Map<Type, Integer> consumableTypeBoostingItems
//   Map<Integer, List<Integer>> moveBoostingItems
//   Map<Integer, List<Integer>> abilityBoostingItems
//   Map<Integer, List<Integer>> speciesBoostingItems
//   Map<Type, List<Integer>> typeBoostingItems
//   Map<Type, Integer> weaknessReducingBerries
//   EvolutionType[] evolutionTypeTable
//   Set<Integer> bannedItems
//   Set<Integer> badItemsBW2
//   Set<Integer> badItemsBW1
//   Set<Integer> opShopItems
//   int[][] habitatListEntries
//   Map<Integer, Integer> mugshotTrainerMapBW
//   Map<Integer, Integer> mugshotTrainerMapBW2
//   HashMap<String, Type> gymAndEliteThemesBW
//   HashMap<String, Type> gymAndEliteThemesBW2

}  // namespace pr::gen5
