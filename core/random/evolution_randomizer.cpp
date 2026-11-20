#include "random/evolution_randomizer.hpp"

#include <algorithm>
#include <functional>

#include "constants/species_ids.hpp"

namespace pr {

namespace {

struct Inner {
    static constexpr int MAX_TRIES = 1000, DEFAULT_STAGE_LIMIT = 10;
    JavaRandom& random;
    const SpeciesSet& pool;
    const SpeciesSet& banned;
    bool similarStrength, sameType, noConvergence, forceChange, forceGrowth, evolveEveryLevel;
    int stageLimit;
    std::map<int, std::vector<EvolutionPtr>> originals;  // HashMap<Species, List<Evolution>> (solo get)

    bool hasTo(const std::vector<EvolutionPtr>& evos, Species* to) const {
        return std::any_of(evos.begin(), evos.end(), [&](const EvolutionPtr& e) { return e->to()->number() == to->number(); });
    }

    bool isCyclic(Species* pk, SpeciesSet& visited, SpeciesSet& rec) {
        if (!visited.contains(pk)) {
            visited.add(pk);
            rec.add(pk);
            for (const auto& ev : std::vector<EvolutionPtr>(pk->evolutionsFrom())) {
                if (!visited.contains(ev->to()) && isCyclic(ev->to(), visited, rec)) return true;
                if (rec.contains(ev->to())) return true;
            }
        }
        rec.remove(pk);
        return false;
    }
    bool createsCycle(Species* from, Species* to) {
        auto temp = std::make_shared<Evolution>(from, to, EvolutionType::NONE, 0);
        from->evolutionsFrom().push_back(temp);
        SpeciesSet visited, rec;
        bool r = isCyclic(from, visited, rec);
        auto& v = from->evolutionsFrom();
        auto it = std::find_if(v.begin(), v.end(), [&](const EvolutionPtr& e) { return e->sameAs(*temp); });
        if (it != v.end()) v.erase(it);
        return r;
    }
    int numPre(Species* pk, int depth) {
        if (pk->evolutionsTo().empty()) return 0;
        if (depth == stageLimit - 1) return 1;
        int m = 0;
        for (const auto& ev : pk->evolutionsTo()) m = std::max(m, numPre(ev->from(), depth + 1) + 1);
        return m;
    }
    int numEvos(Species* pk, int depth) {
        if (pk->evolutionsFrom().empty()) return originals.at(pk->number()).empty() ? 0 : 1;
        if (depth == stageLimit - 1) return 1;
        int m = 0;
        for (const auto& ev : pk->evolutionsFrom()) m = std::max(m, numEvos(ev->to(), depth + 1) + 1);
        return m;
    }
    bool breaksStageLimit(Species* from, Species* to) { return numPre(from, 0) + numEvos(to, 0) + 2 > stageLimit; }

    SpeciesSet findPossible(Species* from, const Evolution& evo) {
        return pool.filter([&](Species* to) {
            if (banned.contains(to)) return false;
            if (to->number() == from->number()) return false;
            if (to->growthCurve != from->growthCurve) return false;
            if (hasTo(from->evolutionsFrom(), to)) return false;
            if (!evolveEveryLevel) {
                if (createsCycle(from, to)) return false;
                if (breaksStageLimit(from, to)) return false;
            }
            if (noConvergence && !to->evolutionsTo().empty()) return false;
            if (forceChange && hasTo(originals.at(from->number()), to)) return false;
            if (forceGrowth && !(to->bst(false) > from->bst(false))) return false;
            if (sameType) {
                if (from->number() == species::eevee && !evolveEveryLevel) {
                    if (!to->hasSharedType(*evo.to())) return false;
                } else if (!to->hasSharedType(*from)) {
                    return false;
                }
            }
            return true;
        });
    }

    bool inner() {
        for (Species* pk : pool.toVector()) {
            pk->evolutionsFrom().clear();
            pk->evolutionsTo().clear();
        }
        for (Species* from : pool.toVector()) {
            std::vector<EvolutionPtr> origEvos;
            if (evolveEveryLevel) origEvos = {std::make_shared<Evolution>(from, from, EvolutionType::LEVEL, 0)};
            else origEvos = originals.at(from->number());
            for (const auto& evo : origEvos) {
                SpeciesSet possible = findPossible(from, *evo);
                if (possible.empty()) return false;
                Species* picked = similarStrength ? possible.getRandomSimilarStrengthSpecies(evo->to(), random)
                                                  : possible.getRandomSpecies(random);
                auto ne = evolveEveryLevel ? std::make_shared<Evolution>(from, picked, EvolutionType::LEVEL, 1)
                                           : std::make_shared<Evolution>(from, picked, evo->type(), evo->extraInfo(), evo->estimatedEvoLvl());
                from->evolutionsFrom().push_back(ne);
                picked->evolutionsTo().push_back(ne);
            }
        }
        return true;
    }
};

}  // namespace

void EvolutionRandomizer::randomizeEvolutions() {
    bool evolveEveryLevel = settings.evolutionsMod == Settings::EvolutionsMod::RANDOM_EVERY_LEVEL;
    SpeciesSet pool = rSpecService.getSpecies(false, romHandler.altFormesCanHaveDifferentEvolutions(), false);
    SpeciesSet banned = SpeciesSet::copyOf(rSpecService.getBannedFormesForPlayerPokemon());
    if (settings.abilitiesMod != Settings::AbilitiesMod::RANDOMIZE) banned.addAll(rSpecService.getAbilityDependentFormes());
    if (settings.banIrregularAltFormes) banned.addAll(romHandler.getIrregularFormes());
    if (evolveEveryLevel && settings.evosSimilarStrength) throw std::invalid_argument("Can't use evolveEveryLevel and similarStrength together.");
    if (evolveEveryLevel && settings.evosMaxThreeStages) throw std::invalid_argument("Can't use evolveEveryLevel and limitToThreeStages together.");
    if (evolveEveryLevel && settings.evosForceGrowth) throw std::invalid_argument("Can't use evolveEveryLevel and forceGrowth together.");
    Inner in{random, pool, banned, settings.evosSimilarStrength, settings.evosSameTyping, settings.evosNoConvergence,
             settings.evosForceChange, settings.evosForceGrowth, evolveEveryLevel,
             settings.evosMaxThreeStages ? 3 : Inner::DEFAULT_STAGE_LIMIT, {}};
    for (Species* pk : pool.toVector()) in.originals[pk->number()] = pk->evolutionsFrom();
    bool ok = false;
    int tries = 0;
    while (!ok && tries < Inner::MAX_TRIES) {
        ok = in.inner();
        tries++;
    }
    if (tries == Inner::MAX_TRIES) {
        throw RandomizationException(std::string("Could not randomize Evolutions in 1000 tries.") +
                                     (settings.standardizeEXPCurves ? "" : " Try using the \"Standardize EXP Curves\" option."));
    }
    changesMade = true;
}

}  // namespace pr
