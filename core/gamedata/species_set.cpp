#include <algorithm>
#include <deque>
#include <stdexcept>

#include "gamedata/species.hpp"
#include "util/java_random.hpp"

namespace pr {

namespace {
constexpr double kCacheResetFactor = 0.5;
constexpr int kSSMinimumPool = 5;
constexpr int kSSMinimumPoolFactor = 4;
constexpr Type kAllTypes[] = {Type::NORMAL, Type::FIGHTING, Type::FLYING, Type::GRASS, Type::WATER, Type::FIRE,
                              Type::ROCK, Type::GROUND, Type::PSYCHIC, Type::BUG, Type::DRAGON, Type::ELECTRIC,
                              Type::GHOST, Type::POISON, Type::ICE, Type::STEEL, Type::DARK, Type::FAIRY};
}  // namespace

void SpeciesSet::checkModifiable() const {
    if (unmodifiable_) throw std::logic_error("UnmodifiableSetException");
}

bool SpeciesSet::add(Species* sp) {
    if (!sp || contains(sp)) return false;
    checkModifiable();
    randomCache_.reset();
    set_.add(sp);
    return true;
}

bool SpeciesSet::remove(Species* sp) {
    checkModifiable();
    return sp && set_.remove(sp);
}

void SpeciesSet::clear() {
    checkModifiable();
    set_.clear();
    randomCache_.reset();
}

bool SpeciesSet::removeAll(const SpeciesSet& c) {
    checkModifiable();
    bool modified = false;
    if (size() > c.size()) {
        for (Species* e : c.toVector()) modified |= set_.remove(e);
    } else {
        modified = set_.removeIf([&](Species* const& s) { return c.contains(s); });
    }
    return modified;
}

bool SpeciesSet::retainAll(const SpeciesSet& c) {
    checkModifiable();
    return set_.removeIf([&](Species* const& s) { return !c.contains(s); });
}

bool SpeciesSet::removeIf(const std::function<bool(Species*)>& pred) {
    checkModifiable();
    return set_.removeIf([&](Species* const& s) { return pred(s); });
}

SpeciesSet SpeciesSet::filter(const std::function<bool(Species*)>& pred) const {
    SpeciesSet r;
    forEach([&](Species* s) {
        if (pred(s)) r.add(s);
    });
    return r;
}

bool SpeciesSet::containsAny(const SpeciesSet& other) const {
    const SpeciesSet& iter = size() < other.size() ? *this : other;
    const SpeciesSet& look = size() < other.size() ? other : *this;
    for (Species* s : iter.toVector()) {
        if (look.contains(s)) return true;
    }
    return false;
}

SpeciesSet SpeciesSet::buildSetSingle(const std::function<Species*(Species*)>& f) const {
    SpeciesSet r;
    forEach([&](Species* s) { r.add(f(s)); });
    return r;
}
SpeciesSet SpeciesSet::buildSet(const std::function<SpeciesSet(Species*)>& f) const {
    SpeciesSet r;
    forEach([&](Species* s) { r.addAll(f(s)); });
    return r;
}
SpeciesSet SpeciesSet::filterBuiltSetSingle(const std::function<Species*(Species*)>& f) const {
    SpeciesSet b = buildSetSingle(f);
    b.retainAll(*this);
    return b;
}
SpeciesSet SpeciesSet::filterBuiltSet(const std::function<SpeciesSet(Species*)>& f) const {
    SpeciesSet b = buildSet(f);
    b.retainAll(*this);
    return b;
}
bool SpeciesSet::addBuiltSetSingle(const std::function<Species*(Species*)>& f) { return addAll(buildSetSingle(f)); }
bool SpeciesSet::addBuiltSet(const std::function<SpeciesSet(Species*)>& f) { return addAll(buildSet(f)); }
bool SpeciesSet::removeBuiltSetSingle(const std::function<Species*(Species*)>& f) { return removeAll(buildSetSingle(f)); }
bool SpeciesSet::removeBuiltSet(const std::function<SpeciesSet(Species*)>& f) { return removeAll(buildSet(f)); }

SpeciesSet SpeciesSet::filterByType(Type type, bool useOriginal) const {
    return filter([&](Species* p) { return p->hasType(type, useOriginal); });
}

std::map<Type, SpeciesSet> SpeciesSet::sortByType(bool useOriginal) const {
    std::map<Type, SpeciesSet> m;
    forEach([&](Species* s) {
        m[*s->primaryType(useOriginal)].add(s);
        if (s->hasSecondaryType(useOriginal)) m[*s->secondaryType(useOriginal)].add(s);
    });
    return m;
}

std::map<Type, SpeciesSet> SpeciesSet::sortByType(bool useOriginal, const std::vector<Type>& sortInto) const {
    std::set<Type> types(sortInto.begin(), sortInto.end());
    std::map<Type, SpeciesSet> m;
    for (Type t : types) m[t];
    forEach([&](Species* s) {
        auto p = s->primaryType(useOriginal);
        if (p && types.count(*p)) m[*p].add(s);
        auto q = s->secondaryType(useOriginal);
        if (q && types.count(*q)) m[*q].add(s);
    });
    return m;
}

std::optional<Type> SpeciesSet::getSharedType(bool useOriginal) const {
    if (empty()) return std::nullopt;
    auto v = toVector();
    auto primary = v[0]->primaryType(useOriginal);
    auto secondary = v[0]->secondaryType(useOriginal);
    for (size_t i = 1; i < v.size(); i++) {
        Species* spec = v[i];
        if (secondary && !spec->hasType(secondary, useOriginal)) secondary.reset();
        if (!spec->hasType(primary, useOriginal)) {
            primary = secondary;
            secondary.reset();
        }
        if (!primary) return std::nullopt;
    }
    if (primary == Type::NORMAL && secondary) return secondary;
    return primary;
}

std::set<Type> SpeciesSet::getSharedTypes(bool useOriginal) const {
    std::set<Type> shared;
    if (empty()) return shared;
    shared.insert(std::begin(kAllTypes), std::end(kAllTypes));
    for (Species* spec : toVector()) {
        for (auto it = shared.begin(); it != shared.end();) {
            if (!spec->hasType(*it, useOriginal)) it = shared.erase(it);
            else ++it;
        }
        if (shared.empty()) break;
    }
    return shared;
}

SpeciesSet SpeciesSet::filterEvolutions(Species* species, bool useOriginal) const {
    SpeciesSet e = species->evolvedSpecies(useOriginal);
    e.retainAll(*this);
    return e;
}
SpeciesSet SpeciesSet::filterPreEvolutions(Species* species, bool useOriginal) const {
    SpeciesSet e = species->preEvolvedSpecies(useOriginal);
    e.retainAll(*this);
    return e;
}
bool SpeciesSet::hasEvolutions(Species* species, bool useOriginal) const {
    for (Species* e : species->evolvedSpecies(useOriginal).toVector())
        if (contains(e)) return true;
    return false;
}
bool SpeciesSet::hasPreEvolutions(Species* species, bool useOriginal) const {
    for (Species* e : species->preEvolvedSpecies(useOriginal).toVector())
        if (contains(e)) return true;
    return false;
}

SpeciesSet SpeciesSet::filterFirstEvolutionAvailable(bool contiguous, bool useOriginal) const {
    SpeciesSet first;
    for (Species* species : toVector()) {
        if (contiguous) {
            if (!hasPreEvolutions(species, useOriginal)) first.add(species);
        } else {
            SpeciesSet checked;
            std::deque<Species*> toCheck{species};
            bool hasPrevo = false;
            while (!toCheck.empty()) {
                Species* checking = toCheck.front();
                toCheck.pop_front();
                if (checked.contains(checking)) continue;
                if (hasPreEvolutions(checking, useOriginal)) {
                    hasPrevo = true;
                    break;
                }
                checked.add(checking);
                // (sic) FVX añade las preevoluciones de "species", no de "checking"
                for (Species* p : species->preEvolvedSpecies(useOriginal).toVector()) toCheck.push_back(p);
            }
            if (!hasPrevo) first.add(species);
        }
    }
    return first;
}

SpeciesSet SpeciesSet::filterBasic(bool o) const {
    return filter([&](Species* p) { return p->preEvolvedSpecies(o).empty(); });
}
SpeciesSet SpeciesSet::filterFinalEvos(bool o) const {
    return filter([&](Species* p) { return p->evolvedSpecies(o).empty(); });
}
SpeciesSet SpeciesSet::filterStandalone(bool o) const {
    return filter([&](Species* p) { return p->preEvolvedSpecies(o).empty() && p->evolvedSpecies(o).empty(); });
}
SpeciesSet SpeciesSet::filterMiddleEvos(bool o) const {
    return filter([&](Species* p) { return !p->preEvolvedSpecies(o).empty() && !p->evolvedSpecies(o).empty(); });
}
SpeciesSet SpeciesSet::filterSplitEvolutions(bool o) const {
    return filter([&](Species* p) {
        for (Species* pre : p->preEvolvedSpecies(o).toVector())
            if (pre->evolvedSpecies(o).size() > 1) return true;
        return false;
    });
}

SpeciesSet SpeciesSet::filterEvoLinesAtLeastLength(int length, bool allowGaps, bool useOriginal) const {
    if (length < 1) throw std::invalid_argument("Invalid evolutionary line length.");
    if (length == 1) return copyOf(*this);
    SpeciesSet valid;
    SpeciesSet firstInLines = filterFirstEvolutionAvailable(!allowGaps, useOriginal);
    JavaHashMap<Species*, SpeciesSet, JavaHashCode<Species*>, SpeciesPtrEq> current;
    for (Species* s : firstInLines.toVector()) current.put(s, SpeciesSet());
    for (int i = 2; !current.empty(); i++) {
        JavaHashMap<Species*, SpeciesSet, JavaHashCode<Species*>, SpeciesPtrEq> next;
        for (Species* spec : current.keys()) {
            if (!allowGaps && !contains(spec)) continue;
            SpeciesSet lineSoFar = copyOf(*current.get(spec));
            if (contains(spec)) {
                lineSoFar.add(spec);
                if (i >= length) valid.addAll(lineSoFar);
            }
            for (Species* evo : spec->evolvedSpecies(useOriginal).toVector()) next.put(evo, lineSoFar);
        }
        current = std::move(next);
    }
    return valid;
}

SpeciesSet SpeciesSet::filterHasEvoStages(int before, int after, bool useOriginal) const {
    if (before < 0 || after < 0) throw std::invalid_argument("Cannot have a negative number of evolutions!");
    if (before == 0 && after == 0) return copyOf(*this);
    return filter([&](Species* p) {
        if (isInEvoCycle(p, useOriginal)) return getNumberEvoStagesAfter(p, useOriginal) >= before + 1 + after;
        return getNumberEvoStagesBefore(p, useOriginal) >= before && getNumberEvoStagesAfter(p, useOriginal) >= after;
    });
}

int SpeciesSet::getNumberEvoStagesBefore(Species* species, bool useOriginal) const {
    int n = 0;
    SpeciesSet current, checked;
    current.add(species);
    while (!current.empty()) {
        SpeciesSet prev;
        for (Species* s : current.toVector()) {
            if (checked.contains(s)) continue;
            prev.addAll(filterPreEvolutions(s, useOriginal));
            checked.add(s);
        }
        if (!prev.empty()) n++;
        current = std::move(prev);
    }
    return n;
}

int SpeciesSet::getNumberEvoStagesAfter(Species* species, bool useOriginal) const {
    int n = 0;
    SpeciesSet current, checked;
    current.add(species);
    while (!current.empty()) {
        SpeciesSet next;
        for (Species* s : current.toVector()) {
            if (checked.contains(s)) continue;
            next.addAll(filterEvolutions(s, useOriginal));
            checked.add(s);
        }
        if (!next.empty()) n++;
        current = std::move(next);
    }
    return n;
}

int SpeciesSet::getLongestEvoLine(Species* species, bool useOriginal) const {
    if (isInEvoCycle(species, useOriginal)) return getNumberEvoStagesAfter(species, useOriginal);
    return getNumberEvoStagesBefore(species, useOriginal) + 1 + getNumberEvoStagesAfter(species, useOriginal);
}

bool SpeciesSet::isInEvoCycle(Species* species, bool useOriginal) const {
    SpeciesSet current, checked;
    current.addAll(species->evolvedSpecies(useOriginal));
    while (!current.empty()) {
        SpeciesSet next;
        for (Species* s : current.toVector()) {
            if (s == species) return true;
            if (checked.contains(s)) continue;
            next.addAll(filterEvolutions(s, useOriginal));
            checked.add(s);
        }
        current = std::move(next);
    }
    return false;
}

bool SpeciesSet::addFamily(Species* species, bool useOriginal) { return addAll(species->family(useOriginal)); }
bool SpeciesSet::removeFamily(Species* species, bool useOriginal) { return removeAll(species->family(useOriginal)); }
SpeciesSet SpeciesSet::filterFamily(Species* species, bool useOriginal) const {
    SpeciesSet f = species->family(useOriginal);
    f.retainAll(*this);
    return f;
}
SpeciesSet SpeciesSet::buildFullFamilies(bool useOriginal) const {
    SpeciesSet all;
    for (Species* s : toVector())
        if (!all.contains(s)) all.addFamily(s, useOriginal);
    return all;
}
bool SpeciesSet::addFullFamilies(bool useOriginal) { return addAll(buildFullFamilies(useOriginal)); }
bool SpeciesSet::addAllFamilies(const SpeciesSet& source, bool useOriginal) {
    bool any = false;
    for (Species* s : source.toVector()) any |= addFamily(s, useOriginal);
    return any;
}
bool SpeciesSet::retainAllFamilies(const SpeciesSet& source, bool useOriginal) {
    return retainAll(source.buildFullFamilies(useOriginal));
}

SpeciesSet SpeciesSet::filterContiguousFamily(Species* species, bool useOriginal) const {
    SpeciesSet fam;
    if (!contains(species)) return fam;
    std::deque<Species*> toCheck{species};
    while (!toCheck.empty()) {
        Species* c = toCheck.front();
        toCheck.pop_front();
        if (fam.contains(c)) continue;
        fam.add(c);
        for (Species* s : filterEvolutions(c, useOriginal).toVector()) toCheck.push_back(s);
        for (Species* s : filterPreEvolutions(c, useOriginal).toVector()) toCheck.push_back(s);
    }
    return fam;
}

Species* SpeciesSet::getRandomSpecies(JavaRandom& random, bool removePicked) {
    if (empty()) throw std::logic_error("Tried to choose a random member of an empty set!");
    if (!randomCache_) randomCache_ = toVector();
    if (static_cast<double>(size()) / static_cast<double>(randomCache_->size()) > kCacheResetFactor) {
        randomCache_ = toVector();
    }
    while (true) {
        int choice = random.nextInt(static_cast<int>(randomCache_->size()));
        Species* spec = (*randomCache_)[choice];
        if (!contains(spec)) continue;
        if (removePicked) remove(spec);
        return spec;
    }
}

Species* SpeciesSet::getRandomSimilarStrengthSpecies(int bst, JavaRandom& random) {
    if (bst < 0) throw std::invalid_argument("Cannot find similar to a negative BST!");
    return getRandomSimilarStrengthSpecies(nullptr, false, bst, random);
}

Species* SpeciesSet::getRandomSimilarStrengthSpecies(Species* match, JavaRandom& random) {
    return getRandomSimilarStrengthSpecies(match, false, -1, random);
}

Species* SpeciesSet::getRandomSimilarStrengthSpecies(Species* match, bool notSameSpecies, int overrideBST,
                                                     JavaRandom& random) {
    SpeciesSet pool = copyOf(*this);
    if (notSameSpecies) pool.remove(match);
    if (pool.empty()) {
        if (empty()) throw std::logic_error("Attempted to choose a Pokemon Species from an empty set!");
        return match;
    }
    int minimumPool = std::min(kSSMinimumPool, pool.size() / kSSMinimumPoolFactor);
    if (minimumPool < 1) minimumPool = 1;
    if (minimumPool >= pool.size()) return pool.getRandomSpecies(random);
    int matchBST = overrideBST < 0 ? match->bst(true) : overrideBST;
    int minTarget = matchBST - matchBST / 10;
    int maxTarget = matchBST + matchBST / 10;
    SpeciesSet canPick;
    while (canPick.size() < minimumPool) {
        pool.set_.removeIf([&](Species* const& spec) {
            if (spec->bst(false) >= minTarget && spec->bst(false) <= maxTarget) {
                canPick.add(spec);
                return true;
            }
            return false;
        });
        minTarget -= matchBST / 20;
        maxTarget += matchBST / 20;
    }
    return canPick.getRandomSpecies(random);
}

}  // namespace pr
