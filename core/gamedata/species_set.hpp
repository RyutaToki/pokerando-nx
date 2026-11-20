// Port de SpeciesSet: un HashSet<Species> de Java con utilidades para los randomizadores.
// El orden de iteración reproduce el de Java (ver util/java_hash.hpp).
#pragma once
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <vector>

#include "gamedata/types.hpp"
#include "util/java_hash.hpp"

namespace pr {

class Species;
class JavaRandom;
template <>
struct JavaHashCode<Species*> {
    int operator()(const Species* s) const;
};
struct SpeciesPtrEq {
    bool operator()(const Species* a, const Species* b) const;
};

class SpeciesSet {
public:
    SpeciesSet() = default;
    explicit SpeciesSet(Species* single) { add(single); }
    // new SpeciesSet(collection): reserva para c.size() (nulos incluidos) y omite los nulos
    template <class Range>
    static SpeciesSet of(const Range& c) {
        SpeciesSet s;
        int n = static_cast<int>(std::size(c));
        s.set_ = Set::withMappings(n > 12 ? n : 12);
        for (Species* sp : c) s.add(sp);
        return s;
    }
    static SpeciesSet copyOf(const SpeciesSet& o) { return of(o.toVector()); }
    template <class Range>
    static SpeciesSet unmodifiable(const Range& c) {
        SpeciesSet s = of(c);
        s.unmodifiable_ = true;
        return s;
    }

    // --- operaciones básicas (como HashSet / SpeciesSet) ---
    bool add(Species* sp);
    template <class Range>
    bool addAll(const Range& c) {
        bool changed = false;
        for (Species* sp : c) changed |= add(sp);
        return changed;
    }
    bool addAll(const SpeciesSet& o) { return addAll(o.toVector()); }
    bool remove(Species* sp);
    bool contains(Species* sp) const { return sp && set_.contains(sp); }
    int size() const { return set_.size(); }
    bool empty() const { return set_.empty(); }
    void clear();
    bool removeAll(const SpeciesSet& c);
    template <class Range>
    bool removeAllRange(const Range& c) {
        bool changed = false;
        for (Species* sp : c) changed |= remove(sp);
        return changed;
    }
    bool retainAll(const SpeciesSet& c);
    bool removeIf(const std::function<bool(Species*)>& pred);
    std::vector<Species*> toVector() const { return set_.toVector(); }
    template <class F>
    void forEach(F&& f) const {
        set_.forEach([&](Species* const& s) { f(s); });
    }
    bool isUnmodifiable() const { return unmodifiable_; }

    SpeciesSet filter(const std::function<bool(Species*)>& pred) const;
    bool containsAny(const SpeciesSet& other) const;
    SpeciesSet buildSetSingle(const std::function<Species*(Species*)>& f) const;
    SpeciesSet buildSet(const std::function<SpeciesSet(Species*)>& f) const;
    SpeciesSet filterBuiltSetSingle(const std::function<Species*(Species*)>& f) const;
    SpeciesSet filterBuiltSet(const std::function<SpeciesSet(Species*)>& f) const;
    bool addBuiltSetSingle(const std::function<Species*(Species*)>& f);
    bool addBuiltSet(const std::function<SpeciesSet(Species*)>& f);
    bool removeBuiltSetSingle(const std::function<Species*(Species*)>& f);
    bool removeBuiltSet(const std::function<SpeciesSet(Species*)>& f);

    // --- tipos ---
    SpeciesSet filterByType(Type type, bool useOriginal) const;
    // EnumMap<Type, SpeciesSet>: ordenado por el orden del enum
    std::map<Type, SpeciesSet> sortByType(bool useOriginal) const;
    std::map<Type, SpeciesSet> sortByType(bool useOriginal, const std::vector<Type>& sortInto) const;
    std::optional<Type> getSharedType(bool useOriginal) const;
    std::set<Type> getSharedTypes(bool useOriginal) const;  // EnumSet: orden del enum

    // --- evoluciones y familias ---
    SpeciesSet filterEvolutions(Species* species, bool useOriginal) const;
    SpeciesSet filterPreEvolutions(Species* species, bool useOriginal) const;
    bool hasEvolutions(Species* species, bool useOriginal) const;
    bool hasPreEvolutions(Species* species, bool useOriginal) const;
    SpeciesSet filterFirstEvolutionAvailable(bool contiguous, bool useOriginal) const;
    SpeciesSet filterBasic(bool useOriginal) const;
    SpeciesSet filterFinalEvos(bool useOriginal) const;
    SpeciesSet filterStandalone(bool useOriginal) const;
    SpeciesSet filterMiddleEvos(bool useOriginal) const;
    SpeciesSet filterSplitEvolutions(bool useOriginal) const;
    SpeciesSet filterEvoLinesAtLeastLength(int length, bool allowGaps, bool useOriginal) const;
    SpeciesSet filterHasEvoStages(int before, int after, bool useOriginal) const;
    int getNumberEvoStagesBefore(Species* species, bool useOriginal) const;
    int getNumberEvoStagesAfter(Species* species, bool useOriginal) const;
    int getLongestEvoLine(Species* species, bool useOriginal) const;
    bool isInEvoCycle(Species* species, bool useOriginal) const;
    bool addFamily(Species* species, bool useOriginal);
    bool removeFamily(Species* species, bool useOriginal);
    SpeciesSet filterFamily(Species* species, bool useOriginal) const;
    SpeciesSet buildFullFamilies(bool useOriginal) const;
    bool addFullFamilies(bool useOriginal);
    bool addAllFamilies(const SpeciesSet& source, bool useOriginal);
    bool retainAllFamilies(const SpeciesSet& source, bool useOriginal);
    SpeciesSet filterContiguousFamily(Species* species, bool useOriginal) const;

    // --- aleatorios ---
    Species* getRandomSpecies(JavaRandom& random, bool removePicked = false);
    Species* getRandomSimilarStrengthSpecies(int bst, JavaRandom& random);
    Species* getRandomSimilarStrengthSpecies(Species* match, JavaRandom& random);
    Species* getRandomSimilarStrengthSpecies(Species* match, bool notSameSpecies, int overrideBST, JavaRandom& random);

private:
    using Set = JavaHashSet<Species*, JavaHashCode<Species*>, SpeciesPtrEq>;
    void checkModifiable() const;

    Set set_;
    std::optional<std::vector<Species*>> randomCache_;
    bool unmodifiable_ = false;
};

}  // namespace pr
