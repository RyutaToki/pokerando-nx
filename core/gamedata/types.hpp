// Modelos de datos básicos (port de com.uprfvx.romio.gamedata).
#pragma once
#include <cstddef>
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

// DoubleStream.sum() de Java (suma compensada de Kahan, igual que el JDK)
double javaDoubleSum(const double* v, size_t n);

// Port de BaseStats / ShedinjaBaseStats. Los stats se recalculan a partir de proporciones y un BST
// con el mismo algoritmo (y la misma cola de prioridad) que FVX.
struct BaseStats {
    static constexpr int STAT_MAX = 255;
    int hp = 0, attack = 0, defense = 0, spatk = 0, spdef = 0, speed = 0;
    bool shedinja = false;  // ShedinjaBaseStats: sin PS; getHp() = 1 y el BST cuenta x6/5
    int bst_ = 0;
    int bstForPowerLevels_ = 0;  // solo Shedinja
    double hpRatio = 0, attackRatio = 0, defenseRatio = 0, spatkRatio = 0, spdefRatio = 0, speedRatio = 0;

    BaseStats() = default;
    BaseStats(int hp, int attack, int defense, int spatk, int spdef, int speed);
    static BaseStats makeShedinja(int attack, int defense, int spatk, int spdef, int speed);

    int getHp() const { return shedinja ? 1 : hp; }
    int bst() const;  // getBST()
    void setBST(int bst);
    void setStatRatios(double hp, double attack, double defense, double spatk, double spdef, double speed);
    void setStatRatiosShedinja(double attack, double defense, double spatk, double spdef, double speed);
    double getAttackSpecialAttackRatio() const { return static_cast<double>(attack) / (static_cast<double>(attack) + spatk); }
    int getMaxBST() const { return STAT_MAX * 6; }

private:
    void calculateStats();
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
