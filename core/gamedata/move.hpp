// Port de Move y sus enums (com.uprfvx.romio.gamedata).
#pragma once
#include <array>
#include <optional>
#include <string>

#include "constants/global_constants.hpp"
#include "gamedata/types.hpp"

namespace pr {

enum class StatChangeMoveType { NONE_OR_UNKNOWN, NO_DAMAGE_TARGET, NO_DAMAGE_USER, NO_DAMAGE_ALL, NO_DAMAGE_ALLY, DAMAGE_TARGET, DAMAGE_USER };
enum class StatChangeType { NONE, ATTACK, DEFENSE, SPECIAL_ATTACK, SPECIAL_DEFENSE, SPEED, ACCURACY, EVASION, ALL, SPECIAL };
enum class StatusMoveType { NONE_OR_UNKNOWN, NO_DAMAGE, DAMAGE };
enum class StatusType { NONE, PARALYZE, SLEEP, FREEZE, BURN, POISON, CONFUSION, TOXIC_POISON };
enum class CriticalChance { NONE, NORMAL, INCREASED, GUARANTEED };
enum class MoveCategory { PHYSICAL, SPECIAL, STATUS };

const char* statChangeMoveTypeName(StatChangeMoveType t);
const char* statChangeTypeName(StatChangeType t);
const char* statusMoveTypeName(StatusMoveType t);
const char* statusTypeName(StatusType t);
const char* criticalChanceName(CriticalChance c);
const char* moveCategoryName(MoveCategory c);

struct StatChange {
    StatChangeType type = StatChangeType::NONE;
    int stages = 0;
    double percentChance = 0;
    bool operator==(const StatChange&) const = default;
};

struct Move {
    std::string name;
    int number = 0;
    int internalId = 0;
    int power = 0;
    int pp = 0;
    double hitratio = 0;
    std::optional<Type> type;
    MoveCategory category = MoveCategory::PHYSICAL;
    StatChangeMoveType statChangeMoveType = StatChangeMoveType::NONE_OR_UNKNOWN;
    std::array<StatChange, 3> statChanges{};
    StatusMoveType statusMoveType = StatusMoveType::NONE_OR_UNKNOWN;
    StatusType statusType = StatusType::NONE;
    CriticalChance criticalChance = CriticalChance::NORMAL;
    double statusPercentChance = 0;
    double flinchPercentChance = 0;
    int recoilPercent = 0;
    int absorbPercent = 0;
    int priority = 0;
    bool makesContact = false;
    bool isChargeMove = false;
    bool isRechargeMove = false;
    bool isPunchMove = false;
    bool isSoundMove = false;
    bool isTrapMove = false;  // ataques que atrapan (Constricción) y que impiden huir (Mal de Ojo)
    int effectIndex = 0;
    int target = 0;
    double hitCount = 1;  // solo se usa al randomizar la potencia

    bool hasSpecificStatChange(StatChangeType t, bool isPositive) const {
        for (auto& sc : statChanges) {
            if (sc.type == t && (isPositive ^ (sc.stages < 0))) return true;
        }
        return false;
    }
    bool hasBeneficialStatChange() const {
        return (statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET && statChanges[0].stages < 0) ||
               (statChangeMoveType == StatChangeMoveType::DAMAGE_USER && statChanges[0].stages > 0);
    }
    bool isGoodDamaging(int perfectAccuracy) const {
        return (power * hitCount) >= 2 * global::MIN_DAMAGING_MOVE_POWER ||
               ((power * hitCount) >= global::MIN_DAMAGING_MOVE_POWER && (hitratio >= 90 || hitratio == perfectAccuracy));
    }
};

}  // namespace pr
