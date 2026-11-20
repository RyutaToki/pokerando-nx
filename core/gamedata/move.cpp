#include "gamedata/move.hpp"

namespace pr {
const char* statChangeMoveTypeName(StatChangeMoveType t) {
    static const char* n[] = {"NONE_OR_UNKNOWN", "NO_DAMAGE_TARGET", "NO_DAMAGE_USER", "NO_DAMAGE_ALL", "NO_DAMAGE_ALLY", "DAMAGE_TARGET", "DAMAGE_USER"};
    return n[static_cast<int>(t)];
}
const char* statChangeTypeName(StatChangeType t) {
    static const char* n[] = {"NONE", "ATTACK", "DEFENSE", "SPECIAL_ATTACK", "SPECIAL_DEFENSE", "SPEED", "ACCURACY", "EVASION", "ALL", "SPECIAL"};
    return n[static_cast<int>(t)];
}
const char* statusMoveTypeName(StatusMoveType t) {
    static const char* n[] = {"NONE_OR_UNKNOWN", "NO_DAMAGE", "DAMAGE"};
    return n[static_cast<int>(t)];
}
const char* statusTypeName(StatusType t) {
    static const char* n[] = {"NONE", "PARALYZE", "SLEEP", "FREEZE", "BURN", "POISON", "CONFUSION", "TOXIC_POISON"};
    return n[static_cast<int>(t)];
}
const char* criticalChanceName(CriticalChance c) {
    static const char* n[] = {"NONE", "NORMAL", "INCREASED", "GUARANTEED"};
    return n[static_cast<int>(c)];
}
const char* moveCategoryName(MoveCategory c) {
    static const char* n[] = {"PHYSICAL", "SPECIAL", "STATUS"};
    return n[static_cast<int>(c)];
}
}  // namespace pr
