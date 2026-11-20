#include "gamedata/type_table.hpp"

#include <set>
#include <stdexcept>

namespace pr {

namespace {
using E = Effectiveness;
#include "gamedata/type_tables.inc"

TypeTable fromTable(const std::vector<Type>& types, const std::vector<std::vector<Effectiveness>>& table) {
    TypeTable t(types);
    for (size_t a = 0; a < types.size(); a++)
        for (size_t d = 0; d < types.size(); d++) t.setEffectiveness(types[a], types[d], table[a][d]);
    return t;
}
}  // namespace

const char* effectivenessName(Effectiveness e) {
    static const char* n[] = {"ZERO", "HALF", "NEUTRAL", "DOUBLE", "QUARTER", "QUADRUPLE"};
    return n[static_cast<int>(e)];
}

Effectiveness combine(Effectiveness a, Effectiveness b) {
    static const E table[4][4] = {{E::ZERO, E::ZERO, E::ZERO, E::ZERO},
                                  {E::ZERO, E::QUARTER, E::HALF, E::NEUTRAL},
                                  {E::ZERO, E::HALF, E::NEUTRAL, E::DOUBLE},
                                  {E::ZERO, E::NEUTRAL, E::DOUBLE, E::QUADRUPLE}};
    return table[static_cast<int>(a)][static_cast<int>(b)];
}

TypeTable::TypeTable(std::vector<Type> types) : types_(std::move(types)) {
    std::set<Type> uniq(types_.begin(), types_.end());
    if (uniq.size() != types_.size()) throw std::invalid_argument("Types must be unique");
    eff_.assign(types_.size(), std::vector<Effectiveness>(types_.size(), E::NEUTRAL));
    for (size_t i = 0; i < types_.size(); i++) typeIndex_[types_[i]] = static_cast<int>(i);
}

TypeTable TypeTable::vanillaGen1() { return fromTable(gen1Types, gen1Table); }
TypeTable TypeTable::vanillaGen2To5() { return fromTable(gen2To5Types, gen2To5Table); }
TypeTable TypeTable::vanillaGen6Plus() { return fromTable(gen6PlusTypes, gen6PlusTable); }

int TypeTable::index(Type t) const {
    auto it = typeIndex_.find(t);
    if (it == typeIndex_.end()) throw std::invalid_argument(std::string("Type ") + typeName(t) + " not supported by this TypeTable.");
    return it->second;
}

void TypeTable::setEffectiveness(Type a, Type d, Effectiveness e) {
    if (static_cast<int>(e) > 3) throw std::invalid_argument("Invalid Effectiveness");
    eff_[index(a)][index(d)] = e;
}

Effectiveness TypeTable::getEffectiveness(Type a, Type d) const { return eff_[index(a)][index(d)]; }

std::map<Type, Effectiveness> TypeTable::against(Type p, std::optional<Type> s) const {
    std::map<Type, Effectiveness> r;
    for (size_t i = 0; i < types_.size(); i++) {
        Effectiveness e = eff_[i][index(p)];
        if (s) e = combine(e, eff_[i][index(*s)]);
        r[types_[i]] = e;
    }
    return r;
}

std::vector<Type> TypeTable::whenAttacking(Type a, Effectiveness e) const {
    std::vector<Type> r;
    for (size_t i = 0; i < types_.size(); i++)
        if (eff_[index(a)][i] == e) r.push_back(types_[i]);
    return r;
}

std::vector<Type> TypeTable::whenDefending(Type d, Effectiveness e) const {
    std::vector<Type> r;
    for (size_t i = 0; i < types_.size(); i++)
        if (eff_[i][index(d)] == e) r.push_back(types_[i]);
    return r;
}

int TypeTable::nonNeutralEffectivenessCount() const {
    int c = 0;
    for (const auto& row : eff_)
        for (auto e : row) c += e != E::NEUTRAL;
    return c;
}

bool TypeTable::operator==(const TypeTable& o) const {
    std::set<Type> a(types_.begin(), types_.end()), b(o.types_.begin(), o.types_.end());
    if (a != b) return false;
    for (Type x : types_)
        for (Type y : types_)
            if (getEffectiveness(x, y) != o.getEffectiveness(x, y)) return false;
    return true;
}

}  // namespace pr
