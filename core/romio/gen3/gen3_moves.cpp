// Lectura y escritura de movimientos de Gen 3 (port de Gen3RomHandler.loadMoves y
// los métodos que interpretan el efecto de cada movimiento).
#include <algorithm>
#include <cmath>

#include "constants/gb_types.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

void Gen3RomHandler::loadMoves() {
    int moveCount = romEntry_->getInt("MoveCount");
    moves_.assign(moveCount + 1, std::nullopt);
    int offs = romEntry_->getInt("MoveData");
    int nameoffs = romEntry_->getInt("MoveNames");
    int namelen = romEntry_->getInt("MoveNameLength");
    auto has = [](const auto& list, int v) { return std::find(list.begin(), list.end(), v) != list.end(); };
    for (int i = 1; i <= moveCount; i++) {
        moves_[i].emplace();
        Move& m = *moves_[i];
        int b = offs + i * 0xC;
        m.name = readFixedLengthString(nameoffs + i * namelen, namelen);
        m.number = i;
        m.internalId = i;
        m.effectIndex = rom_.at(b);
        m.hitratio = rom_.at(b + 3);
        m.power = rom_.at(b + 1);
        m.pp = rom_.at(b + 4);
        m.type = typeFromByte(rom_.at(b + 2));
        m.target = rom_.at(b + 6);
        m.category = gb::isPhysicalType(m.type) ? MoveCategory::PHYSICAL : MoveCategory::SPECIAL;
        if (m.power == 0 && !has(global::noPowerNonStatusMoves, i)) m.category = MoveCategory::STATUS;
        m.priority = static_cast<int8_t>(rom_.at(b + 7));
        int flags = rom_.at(b + 8);
        m.makesContact = (flags & 1) != 0;
        m.isSoundMove = has(soundMoves, m.number);
        if (i == moves::swift) perfectAccuracy_ = static_cast<int>(m.hitratio);
        if (has(global::normalMultihitMoves, i)) m.hitCount = 3;
        else if (has(global::doubleHitMoves, i)) m.hitCount = 2;
        else if (i == moves::tripleKick) m.hitCount = 2.71;  // suponiendo que el primer golpe acierta

        int secondaryEffectChance = rom_.at(b + 5);
        loadStatChangesFromEffect(m, secondaryEffectChance);
        loadStatusFromEffect(m, secondaryEffectChance);
        loadMiscMoveInfoFromEffect(m, secondaryEffectChance);
    }
}

void Gen3RomHandler::loadStatChangesFromEffect(Move& move, int secondaryEffectChance) {
        switch (move.effectIndex) {
            case noDamageAtkPlusOneEffect:
            case noDamageDefPlusOneEffect:
            case noDamageSpAtkPlusOneEffect:
            case noDamageEvasionPlusOneEffect:
            case noDamageAtkMinusOneEffect:
            case noDamageDefMinusOneEffect:
            case noDamageSpeMinusOneEffect:
            case noDamageAccuracyMinusOneEffect:
            case noDamageEvasionMinusOneEffect:
            case noDamageAtkPlusTwoEffect:
            case noDamageDefPlusTwoEffect:
            case noDamageSpePlusTwoEffect:
            case noDamageSpAtkPlusTwoEffect:
            case noDamageSpDefPlusTwoEffect:
            case noDamageAtkMinusTwoEffect:
            case noDamageDefMinusTwoEffect:
            case noDamageSpeMinusTwoEffect:
            case noDamageSpDefMinusTwoEffect:
            case minimizeEffect:
            case swaggerEffect:
            case defenseCurlEffect:
            case flatterEffect:
            case chargeEffect:
            case noDamageAtkAndDefMinusOneEffect:
            case noDamageDefAndSpDefPlusOneEffect:
            case noDamageAtkAndDefPlusOneEffect:
            case noDamageSpAtkAndSpDefPlusOneEffect:
            case noDamageAtkAndSpePlusOneEffect:
                if (move.target == 16) {
                    move.statChangeMoveType = StatChangeMoveType::NO_DAMAGE_USER;
                } else {
                    move.statChangeMoveType = StatChangeMoveType::NO_DAMAGE_TARGET;
                }
                break;

            case damageAtkMinusOneEffect:
            case damageDefMinusOneEffect:
            case damageSpeMinusOneEffect:
            case damageSpAtkMinusOneEffect:
            case damageSpDefMinusOneEffect:
            case damageAccuracyMinusOneEffect:
                move.statChangeMoveType = StatChangeMoveType::DAMAGE_TARGET;
                break;

            case damageUserDefPlusOneEffect:
            case damageUserAtkPlusOneEffect:
            case damageUserAllPlusOneEffect:
            case damageUserAtkAndDefMinusOneEffect:
            case damageUserSpAtkMinusTwoEffect:
                move.statChangeMoveType = StatChangeMoveType::DAMAGE_USER;
                break;

            default:
                // Move does not have a stat-changing effect
                return;
        }

        switch (move.effectIndex) {
            case noDamageAtkPlusOneEffect:
            case damageUserAtkPlusOneEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = 1;
                break;
            }
            case noDamageDefPlusOneEffect:
            case damageUserDefPlusOneEffect:
            case defenseCurlEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = 1;
                break;
            }
            case noDamageSpAtkPlusOneEffect:
            case flatterEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_ATTACK;
                move.statChanges[0].stages = 1;
                break;
            }
            case noDamageEvasionPlusOneEffect:
            case minimizeEffect: {
                move.statChanges[0].type = StatChangeType::EVASION;
                move.statChanges[0].stages = 1;
                break;
            }
            case noDamageAtkMinusOneEffect:
            case damageAtkMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = -1;
                break;
            }
            case noDamageDefMinusOneEffect:
            case damageDefMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = -1;
                break;
            }
            case noDamageSpeMinusOneEffect:
            case damageSpeMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::SPEED;
                move.statChanges[0].stages = -1;
                break;
            }
            case noDamageAccuracyMinusOneEffect:
            case damageAccuracyMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::ACCURACY;
                move.statChanges[0].stages = -1;
                break;
            }
            case noDamageEvasionMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::EVASION;
                move.statChanges[0].stages = -1;
                break;
            }
            case noDamageAtkPlusTwoEffect:
            case swaggerEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = 2;
                break;
            }
            case noDamageDefPlusTwoEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = 2;
                break;
            }
            case noDamageSpePlusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPEED;
                move.statChanges[0].stages = 2;
                break;
            }
            case noDamageSpAtkPlusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_ATTACK;
                move.statChanges[0].stages = 2;
                break;
            }
            case noDamageSpDefPlusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[0].stages = 2;
                break;
            }
            case noDamageAtkMinusTwoEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = -2;
                break;
            }
            case noDamageDefMinusTwoEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = -2;
                break;
            }
            case noDamageSpeMinusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPEED;
                move.statChanges[0].stages = -2;
                break;
            }
            case noDamageSpDefMinusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[0].stages = -2;
                break;
            }
            case damageSpAtkMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_ATTACK;
                move.statChanges[0].stages = -1;
                break;
            }
            case damageSpDefMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[0].stages = -1;
                break;
            }
            case damageUserAllPlusOneEffect: {
                move.statChanges[0].type = StatChangeType::ALL;
                move.statChanges[0].stages = 1;
                break;
            }
            case chargeEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[0].stages = 1;
                break;
            }
            case damageUserAtkAndDefMinusOneEffect:
            case noDamageAtkAndDefMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = -1;
                move.statChanges[1].type = StatChangeType::DEFENSE;
                move.statChanges[1].stages = -1;
                break;
            }
            case damageUserSpAtkMinusTwoEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_ATTACK;
                move.statChanges[0].stages = -2;
                break;
            }
            case noDamageDefAndSpDefPlusOneEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = 1;
                move.statChanges[1].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[1].stages = 1;
                break;
            }
            case noDamageAtkAndDefPlusOneEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = 1;
                move.statChanges[1].type = StatChangeType::DEFENSE;
                move.statChanges[1].stages = 1;
                break;
            }
            case noDamageSpAtkAndSpDefPlusOneEffect: {
                move.statChanges[0].type = StatChangeType::SPECIAL_ATTACK;
                move.statChanges[0].stages = 1;
                move.statChanges[1].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[1].stages = 1;
                break;
            }
            case noDamageAtkAndSpePlusOneEffect: {
                move.statChanges[0].type = StatChangeType::ATTACK;
                move.statChanges[0].stages = 1;
                move.statChanges[1].type = StatChangeType::SPEED;
                move.statChanges[1].stages = 1;
                break;
            }
        }

        if (move.statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET || move.statChangeMoveType == StatChangeMoveType::DAMAGE_USER) {
            for (size_t i = 0; i < move.statChanges.size(); i++) {
                if (move.statChanges[i].type != StatChangeType::NONE) {
                    move.statChanges[i].percentChance = secondaryEffectChance;
                    if (move.statChanges[i].percentChance == 0.0) {
                        move.statChanges[i].percentChance = 100.0;
                    }
                }
            }
        }
    }

void Gen3RomHandler::loadStatusFromEffect(Move& move, int secondaryEffectChance) {
        if (move.number == moves::bounce) {
            // GF hardcoded this, so we have to as well
            move.statusMoveType = StatusMoveType::DAMAGE;
            move.statusType = StatusType::PARALYZE;
            move.statusPercentChance = secondaryEffectChance;
            return;
        }

        switch (move.effectIndex) {
            case noDamageSleepEffect:
            case toxicEffect:
            case noDamageConfusionEffect:
            case noDamagePoisonEffect:
            case noDamageParalyzeEffect:
            case noDamageBurnEffect:
            case swaggerEffect:
            case flatterEffect:
            case teeterDanceEffect:
                move.statusMoveType = StatusMoveType::NO_DAMAGE;
                break;

            case damagePoisonEffect:
            case damageBurnEffect:
            case damageFreezeEffect:
            case damageParalyzeEffect:
            case damageConfusionEffect:
            case twineedleEffect:
            case damageBurnAndThawUserEffect:
            case thunderEffect:
            case blazeKickEffect:
            case poisonFangEffect:
            case poisonTailEffect:
                move.statusMoveType = StatusMoveType::DAMAGE;
                break;

            default:
                // Move does not have a status effect
                return;
        }

        switch (move.effectIndex) {
            case noDamageSleepEffect:
                move.statusType = StatusType::SLEEP;
                break;
            case damagePoisonEffect:
            case noDamagePoisonEffect:
            case twineedleEffect:
            case poisonTailEffect:
                move.statusType = StatusType::POISON;
                break;
            case damageBurnEffect:
            case damageBurnAndThawUserEffect:
            case noDamageBurnEffect:
            case blazeKickEffect:
                move.statusType = StatusType::BURN;
                break;
            case damageFreezeEffect:
                move.statusType = StatusType::FREEZE;
                break;
            case damageParalyzeEffect:
            case noDamageParalyzeEffect:
            case thunderEffect:
                move.statusType = StatusType::PARALYZE;
                break;
            case toxicEffect:
            case poisonFangEffect:
                move.statusType = StatusType::TOXIC_POISON;
                break;
            case noDamageConfusionEffect:
            case damageConfusionEffect:
            case swaggerEffect:
            case flatterEffect:
            case teeterDanceEffect:
                move.statusType = StatusType::CONFUSION;
                break;
        }

        if (move.statusMoveType == StatusMoveType::DAMAGE) {
            move.statusPercentChance = secondaryEffectChance;
            if (move.statusPercentChance == 0.0) {
                move.statusPercentChance = 100.0;
            }
        }
    }

void Gen3RomHandler::loadMiscMoveInfoFromEffect(Move& move, int secondaryEffectChance) {
        switch (move.effectIndex) {
            case increasedCritEffect:
            case blazeKickEffect:
            case poisonTailEffect:
                move.criticalChance = CriticalChance::INCREASED;
                break;

            case futureSightAndDoomDesireEffect:
            case spitUpEffect:
                move.criticalChance = CriticalChance::NONE;
                [[fallthrough]];

            case flinchEffect:
            case snoreEffect:
            case twisterEffect:
            case flinchWithMinimizeBonusEffect:
            case fakeOutEffect:
                move.flinchPercentChance = secondaryEffectChance;
                break;

            case damageAbsorbEffect:
            case dreamEaterEffect:
                move.absorbPercent = 50;
                break;

            case damageRecoil25PercentEffect:
                move.recoilPercent = 25;
                break;

            case damageRecoil33PercentEffect:
                move.recoilPercent = 33;
                break;

            case bindingEffect:
            case trappingEffect:
                move.isTrapMove = true;
                break;

            case razorWindEffect:
            case skullBashEffect:
            case solarbeamEffect:
            case semiInvulnerableEffect:
                move.isChargeMove = true;
                break;

            case rechargeEffect:
                move.isRechargeMove = true;
                break;

            case skyAttackEffect:
                move.criticalChance = CriticalChance::INCREASED;
                move.flinchPercentChance = secondaryEffectChance;
                move.isChargeMove = true;
                break;
        }
    }


}  // namespace pr::gen3
