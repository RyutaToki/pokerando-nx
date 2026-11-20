// Port de Effectiveness y TypeTable.
#pragma once
#include <map>
#include <optional>
#include <vector>

#include "gamedata/types.hpp"

namespace pr {

enum class Effectiveness { ZERO, HALF, NEUTRAL, DOUBLE, QUARTER, QUADRUPLE };
const char* effectivenessName(Effectiveness e);
Effectiveness combine(Effectiveness a, Effectiveness b);

class TypeTable {
public:
    explicit TypeTable(std::vector<Type> types);
    TypeTable(const TypeTable&) = default;
    TypeTable& operator=(const TypeTable&) = default;

    static TypeTable vanillaGen1();
    static TypeTable vanillaGen2To5();
    static TypeTable vanillaGen6Plus();

    const std::vector<Type>& types() const { return types_; }
    void setEffectiveness(Type attacker, Type defender, Effectiveness e);
    Effectiveness getEffectiveness(Type attacker, Type defender) const;
    // HashMap<Type, Effectiveness> en Java; aquí ordenado por el enum
    std::map<Type, Effectiveness> against(Type primary, std::optional<Type> secondary) const;
    std::vector<Type> whenAttacking(Type attacker, Effectiveness e) const;
    std::vector<Type> whenDefending(Type defender, Effectiveness e) const;
    std::vector<Type> immuneWhenAttacking(Type t) const { return whenAttacking(t, Effectiveness::ZERO); }
    std::vector<Type> notVeryEffectiveWhenAttacking(Type t) const { return whenAttacking(t, Effectiveness::HALF); }
    std::vector<Type> superEffectiveWhenAttacking(Type t) const { return whenAttacking(t, Effectiveness::DOUBLE); }
    std::vector<Type> immuneWhenDefending(Type t) const { return whenDefending(t, Effectiveness::ZERO); }
    std::vector<Type> notVeryEffectiveWhenDefending(Type t) const { return whenDefending(t, Effectiveness::HALF); }
    std::vector<Type> superEffectiveWhenDefending(Type t) const { return whenDefending(t, Effectiveness::DOUBLE); }
    int nonNeutralEffectivenessCount() const;
    bool operator==(const TypeTable& o) const;

private:
    int index(Type t) const;
    std::vector<Type> types_;
    std::vector<std::vector<Effectiveness>> eff_;
    std::map<Type, int> typeIndex_;
};

}  // namespace pr
