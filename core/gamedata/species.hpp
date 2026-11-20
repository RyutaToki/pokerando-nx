// Port parcial de Species (se irán añadiendo campos según avance el port).
#pragma once
#include <optional>
#include <string>

#include "gamedata/types.hpp"

namespace pr {

class Species {
public:
    explicit Species(int number) : number_(number) {}

    int number() const { return number_; }
    const std::string& name() const { return name_; }
    void setName(std::string n) { name_ = std::move(n); }
    int generation() const { return generation_; }
    void setGeneration(int g) { generation_ = g; }

    std::optional<Type> primaryType(bool useOriginal = false) const { return useOriginal ? origPrimary_ : primary_; }
    std::optional<Type> secondaryType(bool useOriginal = false) const { return useOriginal ? origSecondary_ : secondary_; }
    void setPrimaryType(std::optional<Type> t) {
        primary_ = t;
        if (!hasSetPrimary_) { origPrimary_ = t; hasSetPrimary_ = true; }
    }
    // Igual que en Java: si coincide con el primario, se guarda como "sin tipo secundario"
    void setSecondaryType(std::optional<Type> t) {
        if (t && t == primary_) t.reset();
        secondary_ = t;
        if (!hasSetSecondary_) { origSecondary_ = t; hasSetSecondary_ = true; }
    }

    BaseStats& baseStats() { return baseStats_; }
    const BaseStats& baseStats() const { return baseStats_; }
    void setBaseStats(const BaseStats& bs) {
        baseStats_ = bs;
        if (originalBST_ == -1) originalBST_ = bs.bst();
    }

    int ability1 = 0, ability2 = 0, ability3 = 0;
    int catchRate = 0;
    int expYield = 0;
    EVYield evYield;
    // índices de objeto estándar; nullopt = ninguno
    std::optional<int> guaranteedHeldItem, commonHeldItem, rareHeldItem, darkGrassHeldItem;
    int genderRatio = 0;
    std::optional<ExpCurve> growthCurve;
    BreedingInfo breedingInfo;

private:
    std::string name_;
    int number_;
    int generation_ = -1;
    std::optional<Type> primary_, secondary_, origPrimary_, origSecondary_;
    bool hasSetPrimary_ = false, hasSetSecondary_ = false;
    BaseStats baseStats_;
    int originalBST_ = -1;
};

}  // namespace pr
