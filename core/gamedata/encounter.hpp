// Port de SpeciesHolder, Encounter, EncounterType y EncounterArea.
#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/species.hpp"
#include "util/java_hash.hpp"

namespace pr {

enum class EncounterType { WALKING, SURFING, FISHING, INTERACT, AMBUSH, SPECIAL, UNUSED };
const char* encounterTypeName(EncounterType t);

class SpeciesHolder {
public:
    explicit SpeciesHolder(Species* baseSpecies);
    Species* species() const { return baseSpecies_->getForme(formeNumber_); }
    void setSpecies(Species* s);
    bool isAltFormeAllowed() const { return altFormeAllowed_; }
    void setAltFormeAllowed() { altFormeAllowed_ = true; }
    int formeNumber() const;
    void setFormeNumber(int f);
    bool operator==(const SpeciesHolder& o) const {
        return formeNumber_ == o.formeNumber_ && altFormeAllowed_ == o.altFormeAllowed_ &&
               baseSpecies_->number() == o.baseSpecies_->number();
    }

private:
    Species* baseSpecies_;
    int formeNumber_ = 0;
    bool altFormeAllowed_ = false;
};

struct Encounter {
    Encounter(Species* species, int level);
    SpeciesHolder holder;
    int level = 0;
    int maxLevel = 0;
    bool isSOS = false;
    int sosType = 0;
    Species* species() const { return holder.species(); }
};

using EncounterPtr = std::shared_ptr<Encounter>;

class EncounterArea {
public:
    // EncounterArea extiende ArrayList<Encounter> en Java: las áreas "aplanadas" comparten
    // los mismos objetos Encounter, así que se guardan como punteros compartidos.
    std::vector<EncounterPtr> encounters;

    int rate = 0;
    std::string displayName;
    int mapIndex = -1;
    std::optional<std::string> locationTag;
    std::optional<EncounterType> encounterType;
    bool postGame = false;
    int partiallyPostGameCutoff = -1;
    bool forceMultipleSpecies = false;

    bool isPartiallyPostGame() const { return partiallyPostGameCutoff != -1; }
    SpeciesSet getBannedSpecies() const { return SpeciesSet::unmodifiable(bannedSpecies_.toVector()); }
    void banSpecies(Species* s) { bannedSpecies_.add(s); }
    template <class Range>
    void banAllSpecies(const Range& r) {
        for (Species* s : r) bannedSpecies_.add(s);
    }
    void banAllSpecies(const SpeciesSet& s) { banAllSpecies(s.toVector()); }
    SpeciesSet getSpeciesInArea() const;

    static std::vector<EncounterArea> flattenLocations(const std::vector<EncounterArea>& areas);
    static std::vector<EncounterArea> flattenEncounterTypesInMaps(const std::vector<EncounterArea>& areas);

private:
    JavaHashSet<Species*, JavaHashCode<Species*>, SpeciesPtrEq> bannedSpecies_;
};

}  // namespace pr
