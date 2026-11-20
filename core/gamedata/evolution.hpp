// Port de EvolutionType, Evolution y MoveLearnt.
#pragma once
#include <memory>
#include <optional>

namespace pr {

class Species;

enum class EvolutionType {
    LEVEL, STONE, TRADE,
    TRADE_ITEM, HAPPINESS, HAPPINESS_DAY, HAPPINESS_NIGHT,
    LEVEL_ATTACK_HIGHER, LEVEL_DEFENSE_HIGHER, LEVEL_ATK_DEF_SAME,
    LEVEL_LOW_PV, LEVEL_HIGH_PV, LEVEL_CREATE_EXTRA, LEVEL_IS_EXTRA, HIGH_BEAUTY,
    STONE_MALE_ONLY, STONE_FEMALE_ONLY, ITEM_DAY, ITEM_NIGHT, WITH_MOVE, WITH_OTHER,
    LEVEL_MALE_ONLY, LEVEL_FEMALE_ONLY, MAGNETIC_FIELD, MOSS_ROCK, ICE_ROCK,
    TRADE_SPECIAL,
    FAIRY_AFFECTION, LEVEL_WITH_DARK, LEVEL_UPSIDE_DOWN, LEVEL_RAIN, LEVEL_DAY, LEVEL_NIGHT,
    LEVEL_GAME_THIS, LEVEL_GAME_OTHER, LEVEL_GAME_THIS_DAY, LEVEL_GAME_THIS_NIGHT, LEVEL_GAME_OTHER_DAY,
    LEVEL_GAME_OTHER_NIGHT, SNOWY, LEVEL_DUSK, LEVEL_ULTRA, STONE_ULTRA,
    ITEM, NONE
};

const char* evolutionTypeName(EvolutionType t);
bool usesLevelThreshold(EvolutionType t);
bool usesItem(EvolutionType t);
bool usesLocation(EvolutionType t);
bool isDayType(EvolutionType t);
bool isGameSpecific(EvolutionType t);
std::optional<EvolutionType> oppositeTime(EvolutionType t);
std::optional<EvolutionType> timeless(EvolutionType t);
inline bool usesMove(EvolutionType t) { return t == EvolutionType::WITH_MOVE; }
inline bool usesSpecies(EvolutionType t) { return t == EvolutionType::WITH_OTHER; }
inline bool usesTime(EvolutionType t) { return timeless(t).has_value(); }

class Evolution {
public:
    Evolution(Species* from, Species* to, EvolutionType type, int extra)
        : from_(from), to_(to), type_(type), extraInfo_(extra) {
        if (usesLevelThreshold(type)) estimatedEvoLvl_ = extra;
    }
    Evolution(Species* from, Species* to, EvolutionType type, int extra, int estimated)
        : from_(from), to_(to), type_(type), extraInfo_(extra), estimatedEvoLvl_(estimated) {}

    Species* from() const { return from_; }
    Species* to() const { return to_; }
    void setFrom(Species* s) { from_ = s; }
    void setTo(Species* s) { to_ = s; }
    EvolutionType type() const { return type_; }
    int extraInfo() const { return extraInfo_; }
    int estimatedEvoLvl() const { return estimatedEvoLvl_; }
    void setEstimatedEvoLvl(int l) { estimatedEvoLvl_ = l; }

    void updateEvolutionMethod(EvolutionType type, int extraInfo) {
        if (usesLevelThreshold(type) && (!usesLevelThreshold(type_) || extraInfo != estimatedEvoLvl_)) {
            estimatedEvoLvl_ = extraInfo;
        }
        type_ = type;
        extraInfo_ = extraInfo;
    }
    void updateEvolutionMethod(EvolutionType type, int extraInfo, bool useEstimatedLevels) {
        updateEvolutionMethod(type, (useEstimatedLevels && usesLevelThreshold(type)) ? estimatedEvoLvl_ : extraInfo);
    }

    // equals() de Java: mismo tipo, dato extra, origen y destino
    bool sameAs(const Evolution& o) const;
    int javaHashCode() const;
    int compareTo(const Evolution& o) const;

private:
    Species* from_;
    Species* to_;
    EvolutionType type_;
    int extraInfo_;
    int estimatedEvoLvl_ = 0;
};

using EvolutionPtr = std::shared_ptr<Evolution>;

struct MoveLearnt {
    int move = 0;
    int level = 0;
    bool operator==(const MoveLearnt&) const = default;
};

}  // namespace pr
