#include "gamedata/encounter.hpp"

#include <cstdio>
#include <stdexcept>

namespace pr {

const char* encounterTypeName(EncounterType t) {
    static const char* n[] = {"WALKING", "SURFING", "FISHING", "INTERACT", "AMBUSH", "SPECIAL", "UNUSED"};
    return n[static_cast<int>(t)];
}

SpeciesHolder::SpeciesHolder(Species* base) : baseSpecies_(base) {
    if (!base->isBaseForme()) throw std::invalid_argument(base->numberAndFullName() + " is not a base forme.");
}

void SpeciesHolder::setSpecies(Species* s) {
    if (!s->isBaseForme() && !altFormeAllowed_)
        throw std::invalid_argument("Species " + s->numberAndFullName() + " could not be set. Alt formes are not allowed.");
    baseSpecies_ = s->baseForme();
    formeNumber_ = s->formeNumber();
}

int SpeciesHolder::formeNumber() const {
    if (!altFormeAllowed_) throw std::invalid_argument("Alt formes are not allowed.");
    return formeNumber_;
}

void SpeciesHolder::setFormeNumber(int f) {
    if (!altFormeAllowed_) throw std::invalid_argument("Alt formes are not allowed.");
    if (!baseSpecies_->isValidFormeNumber(f)) throw std::invalid_argument("formeNumber no válido");
    formeNumber_ = f;
}

Encounter::Encounter(Species* species, int lvl) : holder(species), level(lvl) {
    if (lvl < 0) throw std::invalid_argument("level must be non-negative");
}

SpeciesSet EncounterArea::getSpeciesInArea() const {
    SpeciesSet s;
    for (const auto& e : encounters) s.add(e->species());
    return s;
}

std::vector<EncounterArea> EncounterArea::flattenLocations(const std::vector<EncounterArea>& areas) {
    // groupAreasByLocation: HashMap<String, List<EncounterArea>> (orden de String.hashCode de Java)
    JavaHashMap<std::string, std::vector<const EncounterArea*>> grouped;
    int untagged = 1;
    for (const auto& a : areas) {
        std::string tag = a.locationTag ? *a.locationTag : "UNTAGGED-" + std::to_string(untagged++);
        grouped.getOrInsert(tag).push_back(&a);
    }
    std::vector<EncounterArea> out;
    grouped.forEach([&](const std::string& key, const std::vector<const EncounterArea*>& list) {
        EncounterArea f;
        f.displayName = "All of location " + key;
        for (const EncounterArea* a : list) {
            if (a->encounterType == EncounterType::UNUSED) continue;
            f.encounters.insert(f.encounters.end(), a->encounters.begin(), a->encounters.end());
            f.banAllSpecies(a->getBannedSpecies());
        }
        out.push_back(std::move(f));
    });
    return out;
}

std::vector<EncounterArea> EncounterArea::flattenEncounterTypesInMaps(const std::vector<EncounterArea>& areas) {
    // groupAreasByMapIndex: HashMap<Integer, ...>
    JavaHashMap<int, std::vector<const EncounterArea*>> grouped;
    for (const auto& a : areas) grouped.getOrInsert(a.mapIndex).push_back(&a);
    std::vector<EncounterArea> out;
    int unnamed = 1;
    grouped.forEach([&](const int& mapIdx, const std::vector<const EncounterArea*>& list) {
        // AVISO: FVX usa HashMap<EncounterType, ...> (orden no determinista en Java);
        // aquí se usa el orden del enum.
        std::map<int, std::vector<const EncounterArea*>> byType;
        for (const EncounterArea* a : list) byType[a->encounterType ? static_cast<int>(*a->encounterType) : -1].push_back(a);
        std::string mapName = list[0]->locationTag ? *list[0]->locationTag : "Unknown Map " + std::to_string(unnamed++);
        for (auto& [t, typeList] : byType) {
            EncounterArea f;
            f.displayName = mapName + "-" + (t >= 0 ? encounterTypeName(static_cast<EncounterType>(t)) : "null");
            if (t >= 0) f.encounterType = static_cast<EncounterType>(t);
            f.mapIndex = mapIdx;
            for (const EncounterArea* a : typeList) {
                f.encounters.insert(f.encounters.end(), a->encounters.begin(), a->encounters.end());
                f.banAllSpecies(a->getBannedSpecies());
            }
            out.push_back(std::move(f));
        }
    });
    return out;
}

}  // namespace pr
