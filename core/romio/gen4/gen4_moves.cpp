// Movimientos de Gen 4 (port de Gen4RomHandler.loadMoves/saveMoves y de los métodos que
// interpretan el efecto de cada movimiento).
#include <algorithm>
#include <cmath>

#include "constants/global_constants.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "romio/rom_functions.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
template <class L>
bool has(const L& list, int v) { return std::find(list.begin(), list.end(), v) != list.end(); }
}  // namespace

void Gen4RomHandler::loadMoves() {
    moveNarc_ = readNARC(romEntry_->getFile("MoveData"));
    moves_.assign(moveCount + 1, std::nullopt);
    auto moveNames = getStrings(romEntry_->getInt("MoveNamesTextOffset"));
    for (int i = 1; i <= moveCount; i++) {
        const Bytes& d = moveNarc_.files.at(i);
        moves_[i].emplace();
        Move& m = *moves_[i];
        m.name = moveNames.at(i);
        m.number = i;
        m.internalId = i;
        m.effectIndex = readWord(d, 0);
        m.hitratio = d.at(5);
        m.power = d.at(3);
        m.pp = d.at(6);
        m.type = typeFromByte(d.at(4));
        m.target = readWord(d, 8);
        m.category = moveCategoryFromByte(d.at(2));
        m.priority = static_cast<int8_t>(d.at(10));
        int flags = d.at(11);
        m.makesContact = (flags & 1) != 0;
        m.isPunchMove = has(punchMoves, m.number);
        m.isSoundMove = has(soundMoves, m.number);
        if (i == moves::swift) perfectAccuracy_ = static_cast<int>(m.hitratio);
        if (has(global::normalMultihitMoves, i)) m.hitCount = 3;
        else if (has(global::doubleHitMoves, i)) m.hitCount = 2;
        else if (i == moves::tripleKick) m.hitCount = 2.71;  // suponiendo que el primer golpe acierta
        int secondaryEffectChance = d.at(7);
        loadStatChangesFromEffect(m, secondaryEffectChance);
        loadStatusFromEffect(m, secondaryEffectChance);
        loadMiscMoveInfoFromEffect(m, secondaryEffectChance);
    }
}

void Gen4RomHandler::saveMoves() {
    int namesIdx = romEntry_->getInt("MoveNamesTextOffset");
    auto moveNames = getStrings(namesIdx);
    for (int i = 1; i <= moveCount; i++) {
        Bytes& data = moveNarc_.files.at(i);
        const Move& m = *moves_[i];
        moveNames.at(i) = m.name;
        writeWord(data, 0, m.effectIndex);
        data[2] = static_cast<uint8_t>(moveCategoryToByte(m.category));
        data[3] = static_cast<uint8_t>(m.power);
        data[4] = static_cast<uint8_t>(typeToByte(m.type));
        int hitratio = static_cast<int>(std::llround(m.hitratio));  // Math.round
        if (hitratio < 0) hitratio = 0;
        if (hitratio > 100) hitratio = 100;
        data[5] = static_cast<uint8_t>(hitratio);
        data[6] = static_cast<uint8_t>(m.pp);
    }
    // mensajes de combate con el nombre del movimiento
    int battleIdx = romEntry_->getInt("BattleMoveNamesTextOffset");
    auto battleMsgs = getStrings(battleIdx);
    auto oldMoveNames = getStrings(namesIdx);
    for (int i = 1; i <= moveCount; i++) {
        const std::string& oldName = oldMoveNames.at(i);
        const std::string& newName = moves_[i]->name;
        int base = i * 3;
        for (int j = 0; j < 3; j++)
            if (base + j < static_cast<int>(battleMsgs.size())) battleMsgs[base + j] = romfn::replaceAll(battleMsgs[base + j], oldName, newName);
    }
    setStrings(namesIdx, moveNames);
    setStrings(battleIdx, battleMsgs);
    writeNARC(romEntry_->getFile("MoveData"), moveNarc_);
}

void Gen4RomHandler::loadStatChangesFromEffect(Move& move, int secondaryEffectChance) {
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
        case noDamageSpAtkMinusTwoEffect:
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
        case damageSpDefMinusTwoEffect:
            move.statChangeMoveType = StatChangeMoveType::DAMAGE_TARGET;
            break;

        case damageUserDefPlusOneEffect:
        case damageUserAtkPlusOneEffect:
        case damageUserAllPlusOneEffect:
        case damageUserAtkAndDefMinusOneEffect:
        case damageUserSpAtkMinusTwoEffect:
        case damageUserSpeMinusOneEffect:
        case damageUserDefAndSpDefMinusOneEffect:
        case damageUserSpAtkPlusOneEffect:
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
            case flatterEffect:
            case damageUserSpAtkPlusOneEffect: {
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
            case damageSpeMinusOneEffect:
            case damageUserSpeMinusOneEffect: {
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
            case noDamageSpDefMinusTwoEffect:
            case damageSpDefMinusTwoEffect: {
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
            case damageUserSpAtkMinusTwoEffect:
            case noDamageSpAtkMinusTwoEffect: {
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
            case damageUserDefAndSpDefMinusOneEffect: {
                move.statChanges[0].type = StatChangeType::DEFENSE;
                move.statChanges[0].stages = -1;
                move.statChanges[1].type = StatChangeType::SPECIAL_DEFENSE;
                move.statChanges[1].stages = -1;
                break;
            }
        }

        if (move.statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET
                || move.statChangeMoveType == StatChangeMoveType::DAMAGE_USER) {
            for (int i = 0; i < static_cast<int>(move.statChanges.size()); i++) {
                if (move.statChanges[i].type != StatChangeType::NONE) {
                    move.statChanges[i].percentChance = secondaryEffectChance;
                    if (move.statChanges[i].percentChance == 0.0) {
                        move.statChanges[i].percentChance = 100.0;
                    }
                }
            }
        }
    }

void Gen4RomHandler::loadStatusFromEffect(Move& move, int secondaryEffectChance) {
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
        case damagePoisonWithIncreasedCritEffect:
        case flareBlitzEffect:
        case blizzardEffect:
        case voltTackleEffect:
        case bounceEffect:
        case chatterEffect:
        case fireFangEffect:
        case iceFangEffect:
        case thunderFangEffect:
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
        case damagePoisonWithIncreasedCritEffect:
            move.statusType = StatusType::POISON;
            break;
        case damageBurnEffect:
        case damageBurnAndThawUserEffect:
        case noDamageBurnEffect:
        case blazeKickEffect:
        case flareBlitzEffect:
        case fireFangEffect:
            move.statusType = StatusType::BURN;
            break;
        case damageFreezeEffect:
        case blizzardEffect:
        case iceFangEffect:
            move.statusType = StatusType::FREEZE;
            break;
        case damageParalyzeEffect:
        case noDamageParalyzeEffect:
        case thunderEffect:
        case voltTackleEffect:
        case bounceEffect:
        case thunderFangEffect:
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
        case chatterEffect:
            move.statusType = StatusType::CONFUSION;
            break;
        }

        if (move.statusMoveType == StatusMoveType::DAMAGE) {
            move.statusPercentChance = secondaryEffectChance;
            if (move.statusPercentChance == 0.0) {
                if (move.number == moves::chatter) {
                    move.statusPercentChance = 1.0;
                } else {
                    move.statusPercentChance = 100.0;
                }
            }
        }
    }

void Gen4RomHandler::loadMiscMoveInfoFromEffect(Move& move, int secondaryEffectChance) {
        switch (move.effectIndex) {
        case increasedCritEffect:
        case blazeKickEffect:
        case damagePoisonWithIncreasedCritEffect:
            move.criticalChance = CriticalChance::INCREASED;
            break;

        case futureSightAndDoomDesireEffect:
            move.criticalChance = CriticalChance::NONE;

        case flinchEffect:
        case snoreEffect:
        case twisterEffect:
        case stompEffect:
        case fakeOutEffect:
        case fireFangEffect:
        case iceFangEffect:
        case thunderFangEffect:
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
        case flareBlitzEffect:
        case voltTackleEffect:
            move.recoilPercent = 33;
            break;

        case damageRecoil50PercentEffect:
            move.recoilPercent = 50;
            break;

        case bindingEffect:
        case trappingEffect:
            move.isTrapMove = true;
            break;

        case skullBashEffect:
        case solarbeamEffect:
        case flyEffect:
        case diveEffect:
        case digEffect:
        case bounceEffect:
        case shadowForceEffect:
            move.isChargeMove = true;
            break;

        case rechargeEffect:
            move.isRechargeMove = true;
            break;

        case razorWindEffect:
            move.criticalChance = CriticalChance::INCREASED;
            move.isChargeMove = true;
            break;

        case skyAttackEffect:
            move.criticalChance = CriticalChance::INCREASED;
            move.flinchPercentChance = secondaryEffectChance;
            move.isChargeMove = true;
            break;
        }
    }

}  // namespace pr::gen4
