#include "gamedata/types.hpp"

#include <stdexcept>

namespace pr {

const char* typeName(Type t) {
    static const char* names[] = {"NORMAL", "FIGHTING", "FLYING", "GRASS", "WATER", "FIRE", "ROCK", "GROUND",
                                  "PSYCHIC", "BUG", "DRAGON", "ELECTRIC", "GHOST", "POISON", "ICE", "STEEL",
                                  "DARK", "FAIRY"};
    return names[static_cast<int>(t)];
}

std::optional<ExpCurve> expCurveFromByte(int b) {
    switch (static_cast<int8_t>(b)) {
        case 0: return ExpCurve::MEDIUM_FAST;
        case 1: return ExpCurve::ERRATIC;
        case 2: return ExpCurve::FLUCTUATING;
        case 3: return ExpCurve::MEDIUM_SLOW;
        case 4: return ExpCurve::FAST;
        case 5: return ExpCurve::SLOW;
        default: return std::nullopt;
    }
}

int expCurveToByte(ExpCurve c) {
    switch (c) {
        case ExpCurve::MEDIUM_FAST: return 0;
        case ExpCurve::ERRATIC: return 1;
        case ExpCurve::FLUCTUATING: return 2;
        case ExpCurve::MEDIUM_SLOW: return 3;
        case ExpCurve::FAST: return 4;
        case ExpCurve::SLOW: return 5;
    }
    return 0;
}

const char* expCurveName(ExpCurve c) {
    static const char* names[] = {"SLOW", "MEDIUM_SLOW", "MEDIUM_FAST", "FAST", "ERRATIC", "FLUCTUATING"};
    return names[static_cast<int>(c)];
}

namespace {
constexpr EggGroup kEggInOrder[] = {
    EggGroup::MONSTER, EggGroup::WATER_1, EggGroup::BUG, EggGroup::FLYING, EggGroup::FIELD,
    EggGroup::FAIRY, EggGroup::GRASS, EggGroup::HUMAN_LIKE, EggGroup::WATER_3, EggGroup::MINERAL,
    EggGroup::AMORPHOUS, EggGroup::WATER_2, EggGroup::DITTO, EggGroup::DRAGON, EggGroup::UNDISCOVERED};
}

EggGroup eggGroupFromID(int id) {
    if (id < 1 || id > 15) throw std::out_of_range("id de grupo huevo fuera de rango");
    return kEggInOrder[id - 1];
}

int eggGroupToID(EggGroup g) {
    for (int i = 0; i < 15; i++) {
        if (kEggInOrder[i] == g) return i + 1;
    }
    return 0;
}

const char* eggGroupName(EggGroup g) {
    static const char* names[] = {"MONSTER", "BUG", "FLYING", "FIELD", "FAIRY", "GRASS", "HUMAN_LIKE",
                                  "MINERAL", "AMORPHOUS", "DRAGON", "WATER_1", "WATER_2", "WATER_3",
                                  "DITTO", "UNDISCOVERED"};
    return names[static_cast<int>(g)];
}

EVYield EVYield::fromWord(int w) {
    EVYield e;
    e.hp = w & 3;
    e.attack = (w >> 2) & 3;
    e.defense = (w >> 4) & 3;
    e.speed = (w >> 6) & 3;
    e.spatk = (w >> 8) & 3;
    e.spdef = (w >> 10) & 3;
    return e;
}

int EVYield::toWord() const {
    return hp | (attack << 2) | (defense << 4) | (speed << 6) | (spatk << 8) | (spdef << 10);
}

int BaseStats::bst() const {
    if (shedinja) return (attack + defense + spatk + spdef + speed) * 6 / 5;
    return hp + attack + defense + spatk + spdef + speed;
}

}  // namespace pr
