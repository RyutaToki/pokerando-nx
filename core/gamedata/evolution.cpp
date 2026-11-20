#include "gamedata/evolution.hpp"

#include "gamedata/species.hpp"

namespace pr {

using ET = EvolutionType;

const char* evolutionTypeName(EvolutionType t) {
    static const char* n[] = {
        "LEVEL", "STONE", "TRADE", "TRADE_ITEM", "HAPPINESS", "HAPPINESS_DAY", "HAPPINESS_NIGHT",
        "LEVEL_ATTACK_HIGHER", "LEVEL_DEFENSE_HIGHER", "LEVEL_ATK_DEF_SAME", "LEVEL_LOW_PV", "LEVEL_HIGH_PV",
        "LEVEL_CREATE_EXTRA", "LEVEL_IS_EXTRA", "HIGH_BEAUTY", "STONE_MALE_ONLY", "STONE_FEMALE_ONLY", "ITEM_DAY",
        "ITEM_NIGHT", "WITH_MOVE", "WITH_OTHER", "LEVEL_MALE_ONLY", "LEVEL_FEMALE_ONLY", "MAGNETIC_FIELD",
        "MOSS_ROCK", "ICE_ROCK", "TRADE_SPECIAL", "FAIRY_AFFECTION", "LEVEL_WITH_DARK", "LEVEL_UPSIDE_DOWN",
        "LEVEL_RAIN", "LEVEL_DAY", "LEVEL_NIGHT", "LEVEL_GAME_THIS", "LEVEL_GAME_OTHER", "LEVEL_GAME_THIS_DAY",
        "LEVEL_GAME_THIS_NIGHT", "LEVEL_GAME_OTHER_DAY", "LEVEL_GAME_OTHER_NIGHT", "SNOWY", "LEVEL_DUSK",
        "LEVEL_ULTRA", "STONE_ULTRA", "ITEM", "NONE"};
    return n[static_cast<int>(t)];
}

bool usesLevelThreshold(EvolutionType t) {
    switch (t) {
        case ET::LEVEL: case ET::LEVEL_ATTACK_HIGHER: case ET::LEVEL_DEFENSE_HIGHER: case ET::LEVEL_ATK_DEF_SAME:
        case ET::LEVEL_LOW_PV: case ET::LEVEL_HIGH_PV: case ET::LEVEL_CREATE_EXTRA: case ET::LEVEL_IS_EXTRA:
        case ET::LEVEL_MALE_ONLY: case ET::LEVEL_FEMALE_ONLY: case ET::LEVEL_WITH_DARK: case ET::LEVEL_UPSIDE_DOWN:
        case ET::LEVEL_RAIN: case ET::LEVEL_DAY: case ET::LEVEL_NIGHT: case ET::LEVEL_GAME_THIS:
        case ET::LEVEL_GAME_OTHER: case ET::LEVEL_GAME_THIS_DAY: case ET::LEVEL_GAME_OTHER_DAY:
        case ET::LEVEL_GAME_THIS_NIGHT: case ET::LEVEL_GAME_OTHER_NIGHT: case ET::LEVEL_DUSK: case ET::LEVEL_ULTRA:
            return true;
        default: return false;
    }
}

bool usesItem(EvolutionType t) {
    switch (t) {
        case ET::STONE: case ET::TRADE_ITEM: case ET::STONE_MALE_ONLY: case ET::STONE_FEMALE_ONLY:
        case ET::ITEM_DAY: case ET::ITEM_NIGHT: case ET::STONE_ULTRA: case ET::ITEM: return true;
        default: return false;
    }
}

bool usesLocation(EvolutionType t) {
    return t == ET::MAGNETIC_FIELD || t == ET::MOSS_ROCK || t == ET::ICE_ROCK || t == ET::SNOWY;
}

bool isDayType(EvolutionType t) {
    return t == ET::HAPPINESS_DAY || t == ET::ITEM_DAY || t == ET::LEVEL_DAY || t == ET::LEVEL_GAME_THIS_DAY ||
           t == ET::LEVEL_GAME_OTHER_DAY;
}

bool isGameSpecific(EvolutionType t) {
    return t == ET::LEVEL_GAME_THIS || t == ET::LEVEL_GAME_OTHER || t == ET::LEVEL_GAME_THIS_DAY ||
           t == ET::LEVEL_GAME_OTHER_DAY || t == ET::LEVEL_GAME_THIS_NIGHT || t == ET::LEVEL_GAME_OTHER_NIGHT;
}

std::optional<EvolutionType> oppositeTime(EvolutionType t) {
    switch (t) {
        case ET::HAPPINESS_DAY: return ET::HAPPINESS_NIGHT;
        case ET::HAPPINESS_NIGHT: return ET::HAPPINESS_DAY;
        case ET::ITEM_DAY: return ET::ITEM_NIGHT;
        case ET::ITEM_NIGHT: return ET::ITEM_DAY;
        case ET::LEVEL_DAY: return ET::LEVEL_NIGHT;
        case ET::LEVEL_NIGHT: return ET::LEVEL_DAY;
        case ET::LEVEL_GAME_THIS_DAY: return ET::LEVEL_GAME_THIS_NIGHT;
        case ET::LEVEL_GAME_THIS_NIGHT: return ET::LEVEL_GAME_THIS_DAY;
        case ET::LEVEL_GAME_OTHER_DAY: return ET::LEVEL_GAME_OTHER_NIGHT;
        case ET::LEVEL_GAME_OTHER_NIGHT: return ET::LEVEL_GAME_OTHER_DAY;
        default: return std::nullopt;
    }
}

std::optional<EvolutionType> timeless(EvolutionType t) {
    switch (t) {
        case ET::HAPPINESS_DAY: case ET::HAPPINESS_NIGHT: return ET::HAPPINESS;
        case ET::ITEM_DAY: case ET::ITEM_NIGHT: return ET::ITEM;
        case ET::LEVEL_DAY: case ET::LEVEL_NIGHT: case ET::LEVEL_DUSK: return ET::LEVEL;
        case ET::LEVEL_GAME_THIS_DAY: case ET::LEVEL_GAME_THIS_NIGHT: return ET::LEVEL_GAME_THIS;
        case ET::LEVEL_GAME_OTHER_DAY: case ET::LEVEL_GAME_OTHER_NIGHT: return ET::LEVEL_GAME_OTHER;
        default: return std::nullopt;
    }
}

bool Evolution::sameAs(const Evolution& o) const {
    return type_ == o.type_ && extraInfo_ == o.extraInfo_ && from_->number() == o.from_->number() &&
           to_->number() == o.to_->number();
}

int Evolution::javaHashCode() const {
    int r = 1;
    r = 31 * r + from_->number();
    r = 31 * r + to_->number();
    r = 31 * r + static_cast<int>(type_);
    return r;
}

int Evolution::compareTo(const Evolution& o) const {
    if (from_->number() != o.from_->number()) return from_->number() < o.from_->number() ? -1 : 1;
    if (to_->number() != o.to_->number()) return to_->number() < o.to_->number() ? -1 : 1;
    int a = static_cast<int>(type_), b = static_cast<int>(o.type_);
    return a < b ? -1 : (a > b ? 1 : 0);
}

}  // namespace pr
