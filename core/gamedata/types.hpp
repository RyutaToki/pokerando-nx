// Modelos de datos básicos (port de com.uprfvx.romio.gamedata).
#pragma once
#include <optional>
#include <string>

namespace pr {

enum class Type {
    NORMAL, FIGHTING, FLYING, GRASS, WATER, FIRE, ROCK, GROUND, PSYCHIC, BUG, DRAGON, ELECTRIC, GHOST,
    POISON, ICE, STEEL, DARK, FAIRY
};
const char* typeName(Type t);  // nombre como en Java (NORMAL, FIGHTING, ...)

enum class ExpCurve { SLOW, MEDIUM_SLOW, MEDIUM_FAST, FAST, ERRATIC, FLUCTUATING };
std::optional<ExpCurve> expCurveFromByte(int b);
int expCurveToByte(ExpCurve c);
const char* expCurveName(ExpCurve c);

enum class EggGroup {
    MONSTER, BUG, FLYING, FIELD, FAIRY, GRASS, HUMAN_LIKE, MINERAL, AMORPHOUS, DRAGON,
    WATER_1, WATER_2, WATER_3, DITTO, UNDISCOVERED
};
EggGroup eggGroupFromID(int id);  // lanza std::out_of_range como Java
int eggGroupToID(EggGroup g);
const char* eggGroupName(EggGroup g);

struct EVYield {
    int hp = 0, attack = 0, defense = 0, spatk = 0, spdef = 0, speed = 0;
    static EVYield fromWord(int word);
    int toWord() const;
};

struct BaseStats {
    int hp = 0, attack = 0, defense = 0, spatk = 0, spdef = 0, speed = 0;
    bool shedinja = false;  // ShedinjaBaseStats: getHp() devuelve 1 y el BST cuenta x6/5
    int getHp() const { return shedinja ? 1 : hp; }
    int bst() const;
};

struct BreedingInfo {
    EggGroup primary = EggGroup::UNDISCOVERED;
    std::optional<EggGroup> secondary;
    int eggCycles = 0;
};

struct Item {
    int id = 0;
    std::string name;
    bool allowed = true;
    bool bad = false;
    bool tm = false;
};

}  // namespace pr
