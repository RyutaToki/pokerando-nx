#include "random/type_effectiveness_randomizer.hpp"

#include <algorithm>
#include <list>
#include <set>

namespace pr {

void TypeEffectivenessRandomizer::randomizeTypeEffectiveness(bool balanced) {
    balanced_ = balanced;
    old_ = romHandler.getTypeTable();
    table_ = TypeTable(old_->types());
    effCounts_.fill(0);
    for (Type a : old_->types())
        for (Type d : old_->types()) effCounts_[int(old_->getEffectiveness(a, d))]++;
    if (balanced) {
        for (Effectiveness e : {Effectiveness::ZERO, Effectiveness::HALF, Effectiveness::DOUBLE}) {
            maxAtk_[int(e)] = 0;
            maxDef_[int(e)] = 0;
            for (Type t : old_->types()) {
                maxAtk_[int(e)] = std::max(maxAtk_[int(e)], int(old_->whenAttacking(t, e).size()));
                maxDef_[int(e)] = std::max(maxDef_[int(e)], int(old_->whenDefending(t, e).size()));
            }
        }
    }
    tries_ = 0;
    placeEffectiveness(Effectiveness::ZERO);
    placeEffectiveness(Effectiveness::HALF);
    placeEffectiveness(Effectiveness::DOUBLE);
    romHandler.setTypeTable(*table_);
    changesMade = true;
}

void TypeEffectivenessRandomizer::placeEffectiveness(Effectiveness e) {
    while (effCounts_[int(e)] > 0 && tries_ <= 10000) {
        Type a = typeService.randomType(random);
        Type d = typeService.randomType(random);
        if (isValidPlacement(a, d, e)) {
            table_->setEffectiveness(a, d, e);
            effCounts_[int(e)]--;
        }
        tries_++;
    }
    if (tries_ > 10000) throw RandomizationException("Couldn't randomize Type Effectiveness");
}

bool TypeEffectivenessRandomizer::isValidPlacement(Type a, Type d, Effectiveness e) {
    if (table_->getEffectiveness(a, d) != Effectiveness::NEUTRAL) return false;
    if (balanced_) {
        if (int(table_->whenAttacking(a, e).size()) == maxAtk_[int(e)]) return false;
        if (int(table_->whenDefending(d, e).size()) == maxDef_[int(e)]) return false;
    }
    return true;
}

void TypeEffectivenessRandomizer::randomizeTypeEffectivenessKeepIdentities() {
    TypeTable tt = romHandler.getTypeTable();
    int swaps = 0;
    while (swaps < 10000) {
        Type a = typeService.randomType(random);
        Type b = typeService.randomType(random);
        int chunkSize = random.nextInt(static_cast<int>(tt.types().size()));
        std::set<Type> chunk;  // HashSet en Java; el orden no afecta al resultado
        while (static_cast<int>(chunk.size()) < chunkSize) chunk.insert(typeService.randomType(random));
        std::array<int, 6> ca{}, cb{};
        for (Type t : chunk) {
            ca[int(tt.getEffectiveness(t, a))]++;
            cb[int(tt.getEffectiveness(t, b))]++;
        }
        if (ca == cb) {
            for (Type t : chunk) {
                Effectiveness s = tt.getEffectiveness(t, a);
                tt.setEffectiveness(t, a, tt.getEffectiveness(t, b));
                tt.setEffectiveness(t, b, s);
            }
            swaps++;
        }
    }
    romHandler.setTypeTable(tt);
    changesMade = true;
}

void TypeEffectivenessRandomizer::invertTypeEffectiveness(bool randomImmunities) {
    TypeTable tt = romHandler.getTypeTable();
    int imm = 0;
    std::vector<std::pair<Type, Type>> se;  // LinkedList en Java
    for (Type a : tt.types()) {
        for (Type d : tt.types()) {
            Effectiveness e = tt.getEffectiveness(a, d);
            if (e == Effectiveness::ZERO) {
                tt.setEffectiveness(a, d, Effectiveness::DOUBLE);
                imm++;
            } else if (e == Effectiveness::HALF) {
                tt.setEffectiveness(a, d, Effectiveness::DOUBLE);
            } else if (e == Effectiveness::DOUBLE) {
                tt.setEffectiveness(a, d, Effectiveness::HALF);
                se.emplace_back(a, d);
            }
        }
    }
    if (randomImmunities) {
        if (static_cast<int>(se.size()) < imm) throw RandomizationException("Too few super-effectives to turn into immunities...");
        for (int i = 0; i < imm; i++) {
            int k = random.nextInt(static_cast<int>(se.size()));
            auto p = se[k];
            se.erase(se.begin() + k);
            tt.setEffectiveness(p.first, p.second, Effectiveness::ZERO);
        }
    }
    romHandler.setTypeTable(tt);
    changesMade = true;
}

}  // namespace pr
