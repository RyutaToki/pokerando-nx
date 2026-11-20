// Generado por tools/convert_constants.py desde GlobalConstants.java (FVX, GPLv3). No editar.
#pragma once
#include <cstdint>
#include <set>
#include <string_view>
#include <vector>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"

namespace pr::global {

inline const std::vector<int> normalMultihitMoves = { moves::armThrust, moves::barrage, moves::boneRush, moves::bulletSeed, moves::cometPunch, moves::doubleSlap, moves::furyAttack, moves::furySwipes, moves::icicleSpear, moves::pinMissile, moves::rockBlast, moves::spikeCannon, moves::tailSlap, moves::waterShuriken};
inline const std::vector<int> doubleHitMoves = { moves::bonemerang, moves::doubleHit, moves::doubleIronBash, moves::doubleKick, moves::dragonDarts, moves::dualChop, moves::gearGrind, moves::twineedle};
inline const std::vector<int> varyingPowerZMoves = { moves::breakneckBlitzPhysical, moves::breakneckBlitzSpecial, moves::allOutPummelingPhysical, moves::allOutPummelingSpecial, moves::supersonicSkystrikePhysical, moves::supersonicSkystrikeSpecial, moves::acidDownpourPhysical, moves::acidDownpourSpecial, moves::tectonicRagePhysical, moves::tectonicRageSpecial, moves::continentalCrushPhysical, moves::continentalCrushSpecial, moves::savageSpinOutPhysical, moves::savageSpinOutSpecial, moves::neverEndingNightmarePhysical, moves::neverEndingNightmareSpecial, moves::corkscrewCrashPhysical, moves::corkscrewCrashSpecial, moves::infernoOverdrivePhysical, moves::infernoOverdriveSpecial, moves::hydroVortexPhysical, moves::hydroVortexSpecial, moves::bloomDoomPhysical, moves::bloomDoomSpecial, moves::gigavoltHavocPhysical, moves::gigavoltHavocSpecial, moves::shatteredPsychePhysical, moves::shatteredPsycheSpecial, moves::subzeroSlammerPhysical, moves::subzeroSlammerSpecial, moves::devastatingDrakePhysical, moves::devastatingDrakeSpecial, moves::blackHoleEclipsePhysical, moves::blackHoleEclipseSpecial, moves::twinkleTacklePhysical, moves::twinkleTackleSpecial};
inline const std::vector<int> fixedPowerZMoves = { moves::catastropika, moves::sinisterArrowRaid, moves::maliciousMoonsault, moves::oceanicOperetta, moves::guardianOfAlola, moves::soulStealing7StarStrike, moves::stokedSparksurfer, moves::pulverizingPancake, moves::extremeEvoboost, moves::genesisSupernova, moves::tenMillionVoltThunderbolt, moves::lightThatBurnsTheSky, moves::searingSunrazeSmash, moves::menacingMoonrazeMaelstrom, moves::letsSnuggleForever, moves::splinteredStormshards, moves::clangorousSoulblaze};
inline const std::vector<int> xItems = {items::guardSpec, items::direHit, items::xAttack, items::xDefense, items::xSpeed, items::xAccuracy, items::xSpAtk, items::xSpDef};
inline const std::vector<int> regularShopItems = { items::pokeBall, items::greatBall, items::ultraBall, items::potion, items::superPotion,items::hyperPotion, items::maxPotion, items::antidote, items::burnHeal, items::iceHeal, items::awakening, items::paralyzeHeal, items::fullHeal, items::fullRestore, items::revive, items::repel, items::superRepel, items::maxRepel, items::escapeRope };
inline const std::vector<int> battleTrappingAbilities = {abilities::shadowTag, abilities::magnetPull, abilities::arenaTrap};
inline const std::vector<int> negativeAbilities = { abilities::defeatist, abilities::slowStart, abilities::truant, abilities::klutz, abilities::stall };
inline const std::vector<int> badAbilities = { abilities::minus, abilities::plus, abilities::anticipation, abilities::forewarn, abilities::frisk, abilities::honeyGather, abilities::auraBreak, abilities::receiver, abilities::powerOfAlchemy };
inline const std::vector<int> doubleBattleAbilities = { abilities::friendGuard, abilities::healer, abilities::telepathy, abilities::symbiosis, abilities::battery };
inline const std::vector<int> duplicateAbilities = { abilities::vitalSpirit, abilities::whiteSmoke, abilities::purePower, abilities::shellArmor, abilities::airLock, abilities::solidRock, abilities::ironBarbs, abilities::turboblaze, abilities::teravolt, abilities::emergencyExit, abilities::dazzling, abilities::tanglingHair, abilities::powerOfAlchemy, abilities::fullMetalBody, abilities::shadowShield, abilities::prismArmor, abilities::libero, abilities::stalwart };
inline const std::vector<int> noPowerNonStatusMoves = { moves::guillotine, moves::hornDrill, moves::sonicBoom, moves::lowKick, moves::counter, moves::seismicToss, moves::dragonRage, moves::fissure, moves::nightShade, moves::bide, moves::psywave, moves::superFang, moves::flail, moves::revenge, moves::returnTheMoveNotTheKeyword, moves::present, moves::frustration, moves::magnitude, moves::mirrorCoat, moves::beatUp, moves::spitUp, moves::sheerCold };
inline const std::vector<int> cannotBeObsoletedMoves = { moves::returnTheMoveNotTheKeyword, moves::frustration, moves::endeavor, moves::flail, moves::reversal, moves::hiddenPower, moves::storedPower, moves::smellingSalts, moves::fling, moves::powerTrip, moves::counter, moves::mirrorCoat, moves::superFang };
inline const std::vector<int> cannotObsoleteMoves = { moves::gearUp, moves::magneticFlux, moves::focusPunch, moves::explosion, moves::selfDestruct, moves::geomancy, moves::venomDrench };
inline const std::vector<int> doubleBattleMoves = { moves::followMe, moves::helpingHand, moves::ragePowder, moves::afterYou, moves::allySwitch, moves::healPulse, moves::quash, moves::ionDeluge, moves::matBlock, moves::aromaticMist, moves::electrify, moves::instruct, moves::spotlight, moves::decorate, moves::lifeDew, moves::coaching };
inline const std::vector<int> uselessMoves = { moves::splash, moves::celebrate, moves::holdHands, moves::teleport, moves::reflectType };
inline const std::vector<int> requiresOtherMove = { moves::spitUp, moves::swallow, moves::dreamEater, moves::nightmare };
inline constexpr int vanillaHappinessToEvolve = 220;
inline constexpr int easierHappinessToEvolve = 160;
inline constexpr int MIN_DAMAGING_MOVE_POWER = 50;
inline constexpr int HIGHEST_POKEMON_GEN = 9;
inline constexpr int LARGEST_NUMBER_OF_SPLIT_EVOS = 8;

// Pendiente de portar a mano:
//   boolean[] bannedRandomMoves
//   List<Integer> zMoves
//   Map<Integer, String> baseFormesWithFormeSuffixes

}  // namespace pr::global
