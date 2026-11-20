#include "random/copy_up_evolutions_helper.hpp"

#include <set>
#include <stack>
#include <stdexcept>

namespace pr {

CopyUpEvolutionsHelper::Options& CopyUpEvolutionsHelper::Options::build() {
    if (!noEvoAction) noEvoAction = basicAction;
    if (!splitAction) splitAction = evolvedAction;
    if (!noEvoAction) noEvoAction = [](Species*) {};
    if (!basicAction) basicAction = [](Species*) {};
    if (!evolvedAction) evolvedAction = [](Species*, Species*, bool) {};
    if (!splitAction) splitAction = [](Species*, Species*, bool) {};
    if (!altFormeAction) altFormeAction = [](Species*, Species*) {};
    if (!cosmeticAction) cosmeticAction = [](Species*, Species*) {};
    return *this;
}

CopyUpEvolutionsHelper::Rel CopyUpEvolutionsHelper::findRelationInto(const SpeciesSet& all, Species* to) const {
    bool anyValid = false;
    for (const auto& e : to->evolutionsTo())
        if (all.contains(e->from())) anyValid = true;
    if (anyValid && !to->isEssentiallyCosmetic()) return {to->evolutionsTo().front()->from(), to};
    if (!to->isBaseForme() && all.contains(to->conceptualBaseForme())) return {to->conceptualBaseForme(), to};
    throw std::logic_error("Argument to is neither an evolved Pokémon nor an alt forme!!");
}

bool CopyUpEvolutionsHelper::isBasic(const SpeciesSet& all, Species* s, const Options& o) const {
    for (const auto& e : s->evolutionsTo())
        if (all.contains(e->from())) return false;
    if (o.treatMegasAsEvos && s->isMegaEvolution() && all.contains(s->conceptualBaseForme())) return false;
    return true;
}

bool CopyUpEvolutionsHelper::isSplitEvo(const SpeciesSet& all, Species* s, const Options& o) const {
    for (const auto& e : s->evolutionsTo()) {
        if (all.contains(e->from())) {
            std::set<int> distinct;
            for (const auto& e2 : e->from()->evolutionsFrom())
                if (all.contains(e2->to())) distinct.insert(e2->to()->number());
            if (distinct.size() > 1) return true;
        }
    }
    if (o.treatMegasAsEvos && s->isMegaEvolution() && all.contains(s->conceptualBaseForme())) {
        if (s->conceptualBaseForme()->altFormes().filter([](Species* x) { return x->isMegaEvolution(); }).size() > 1) return true;
    }
    return false;
}

bool CopyUpEvolutionsHelper::isFinalEvo(const SpeciesSet& all, Species* s, const Options& o) const {
    if (isBasic(all, s, o)) return false;
    for (const auto& e : s->evolutionsFrom())
        if (all.contains(e->to())) return false;
    return true;
}

bool CopyUpEvolutionsHelper::isBaseForme(const SpeciesSet& all, Species* s) const {
    return s->isBaseForme() || !all.contains(s->conceptualBaseForme());
}

void CopyUpEvolutionsHelper::apply(Options o) {
    o.build();
    SpeciesSet all = supplier_();
    SpeciesSet basic, allEvos, splitEvos, finalEvos;
    if (o.evolutionSanity) {
        basic = all.filter([&](Species* s) { return isBasic(all, s, o); }).filter([&](Species* s) { return isBaseForme(all, s); });
        allEvos = all.filter([&](Species* s) { return !isBasic(all, s, o); });
        splitEvos = all.filter([&](Species* s) { return isSplitEvo(all, s, o); });
        finalEvos = all.filter([&](Species* s) { return isFinalEvo(all, s, o); });
    } else {
        basic = all.filter([&](Species* s) { return isBaseForme(all, s); });
    }
    JavaHashSet<Species*, JavaHashCode<Species*>, SpeciesPtrEq> processed;
    if (!o.copySplitEvos) basic.addAll(splitEvos);
    for (Species* pk : basic.toVector()) {
        if (o.evolutionSanity) o.basicAction(pk);
        else o.noEvoAction(pk);
        processed.add(pk);
    }
    for (Species* pk : all.toVector()) {
        if (processed.contains(pk)) continue;
        std::stack<Rel> stack;
        Rel rel = findRelationInto(all, pk);
        while (!processed.contains(rel.from)) {
            stack.push(rel);
            rel = findRelationInto(all, rel.from);
        }
        stack.push(rel);
        while (!stack.empty()) {
            rel = stack.top();
            stack.pop();
            if (rel.to->isEssentiallyCosmetic()) {
                o.cosmeticAction(rel.from, rel.to);
            } else if (o.copySplitEvos && splitEvos.contains(rel.to)) {
                o.splitAction(rel.from, rel.to, finalEvos.contains(rel.to));
            } else if (allEvos.contains(rel.to)) {
                o.evolvedAction(rel.from, rel.to, finalEvos.contains(rel.to));
            } else if (!rel.to->isBaseForme()) {
                o.altFormeAction(rel.from, rel.to);
            } else {
                throw std::logic_error("relación no apta para ninguna acción de copia");
            }
            processed.add(rel.to);
        }
    }
}

}  // namespace pr
