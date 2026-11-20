#include "random/starter_randomizer.hpp"

#include <algorithm>

namespace pr {

using SM = Settings::StartersMod;
using STM = Settings::StartersTypeMod;

void StarterRandomizer::randomizeStarters() {
    bool useCustom = settings.startersMod == SM::CUSTOM;
    bool typeFwg = settings.startersTypeMod == STM::FIRE_WATER_GRASS;
    bool typeUnique = settings.startersTypeMod == STM::UNIQUE;
    bool typeTriangle = settings.startersTypeMod == STM::TRIANGLE;
    bool typeSingle = settings.startersTypeMod == STM::SINGLE_TYPE;
    bool hasTypeRestriction = typeFwg || typeUnique || typeTriangle || typeSingle;
    std::optional<Type> singleType = settings.startersSingleType;
    int starterCount = romHandler.starterCount();
    std::vector<Species*> picked;
    if (useCustom) {
        auto custom = getCustomStarters(settings.customStarters);
        picked.insert(picked.end(), custom.begin(), custom.end());
        if (static_cast<int>(picked.size()) == starterCount) {
            romHandler.setStarters(picked);
            return;
        }
        if (static_cast<int>(picked.size()) > starterCount) throw std::logic_error("Custom starter list exceeded starter count?!");
    }
    SpeciesSet choosable = getAvailableSet(picked);
    if (choosable.size() < starterCount - static_cast<int>(picked.size())) throw RandomizationException("Not enough valid starters");

    if (!hasTypeRestriction) {
        auto more = chooseStartersBasic(starterCount - static_cast<int>(picked.size()), choosable);
        picked.insert(picked.end(), more.begin(), more.end());
    } else if (typeUnique) {
        // de tres en tres, para no quedarse sin tipos en juegos con muchos iniciales
        while (static_cast<int>(picked.size()) < starterCount) {
            int start = (static_cast<int>(picked.size()) / 3) * 3;
            int end = std::min(start + 3, starterCount);
            int sizeOfTrio = end - start;
            int pickedInTrio = static_cast<int>(picked.size()) - start;
            SpeciesSet inTrio;
            for (int i = 0; i < pickedInTrio; i++) inTrio.add(picked[start + i]);
            auto more = chooseUniqueTypeStarters(inTrio, sizeOfTrio, choosable);
            picked.insert(picked.end(), more.begin(), more.end());
            choosable.removeAllRange(picked);
        }
    } else {
        auto byType = choosable.sortByType(false);
        if (typeTriangle) {
            picked = chooseTypeTriangleStarters(byType, true);
            while (static_cast<int>(picked.size()) < starterCount) {
                choosable.removeAllRange(picked);
                byType = choosable.sortByType(false);
                auto more = chooseTypeTriangleStarters(byType, false);
                picked.insert(picked.end(), more.begin(), more.end());
            }
        } else if (typeFwg) {
            picked = chooseStartersFireWaterGrass(byType);
            while (static_cast<int>(picked.size()) < starterCount) {
                choosable.removeAllRange(picked);
                byType = choosable.sortByType(false);
                auto more = chooseStartersFireWaterGrass(byType);
                picked.insert(picked.end(), more.begin(), more.end());
            }
        } else if (typeSingle) {
            if (!singleType) singleType = chooseTypeForStarters(starterCount - static_cast<int>(picked.size()), byType);
            auto more = chooseStartersBasic(starterCount - static_cast<int>(picked.size()), byType.at(*singleType));
            picked.insert(picked.end(), more.begin(), more.end());
        }
    }
    romHandler.setStarters(picked);
    changesMade = true;
}

Type StarterRandomizer::chooseTypeForStarters(int numNeeded, std::map<Type, SpeciesSet>& byType) {
    auto types = typeService.getTypes();
    random.shuffle(types);
    for (Type t : types) {
        if (byType.at(t).size() > numNeeded) return t;  // NPE en Java si el tipo no tiene Pokémon
    }
    throw RandomizationException("No type has " + std::to_string(numNeeded) + " starters available!");
}

std::vector<Species*> StarterRandomizer::chooseStartersFireWaterGrass(std::map<Type, SpeciesSet>& byType) {
    return chooseStartersOfTypes(byType, romHandler.getStandardTypeTriangle());
}

std::vector<Species*> StarterRandomizer::chooseTypeTriangleStarters(std::map<Type, SpeciesSet>& byType, bool firstTrio) {
    // AVISO: en FVX es un HashSet<List<Type>> cuyo orden depende de hashes de identidad (no
    // reproducible entre ejecuciones de Java); aquí se usa el orden en que se encuentran.
    auto list = findTypeTriangles();
    if (list.empty()) throw RandomizationException("Could not find any type triangles");
    random.shuffle(list);
    std::optional<std::vector<Species*>> picks;
    while (!picks && !list.empty()) {
        auto triangle = list[random.nextInt(static_cast<int>(list.size()))];
        try {
            picks = chooseStartersOfTypes(byType, triangle);
        } catch (const RandomizationException&) {
            // (sic) FVX lo quita del conjunto, no de la lista
            picks.reset();
        }
        if (firstTrio) romHandler.setStarterTypeTriangle(triangle);
    }
    if (!picks) throw RandomizationException("No valid starter set with a type triangle could be found!");
    return *picks;
}

std::vector<Species*> StarterRandomizer::chooseStartersOfTypes(std::map<Type, SpeciesSet>& byType, const std::vector<Type>& types) {
    std::vector<Species*> chosen;
    for (Type type : types) {
        auto it = byType.find(type);
        if (it == byType.end()) throw RandomizationException(std::string("No valid starters of type ") + typeName(type) + "found!");
        SpeciesSet ofType = SpeciesSet::copyOf(it->second);
        bool noPick = true;
        while (noPick && !ofType.empty()) {
            Species* p = ofType.getRandomSpecies(random, true);
            std::optional<Type> other = p->primaryType() == type ? p->secondaryType() : p->primaryType();
            bool inTypes = other && std::find(types.begin(), types.end(), *other) != types.end();
            if (!inTypes) {
                noPick = false;
                chosen.push_back(p);
            }
        }
        if (noPick) throw RandomizationException(std::string("No valid starter of type ") + typeName(type) + " found!");
    }
    return chosen;
}

std::vector<Species*> StarterRandomizer::chooseStartersBasic(int n, SpeciesSet& available) {
    if (available.size() < n) throw RandomizationException("Not enough starters to choose from!");
    std::vector<Species*> picks;
    while (static_cast<int>(picks.size()) < n) {
        Species* p = available.getRandomSpecies(random);
        picks.push_back(p);
        available.remove(p);
    }
    return picks;
}

std::vector<Species*> StarterRandomizer::chooseUniqueTypeStarters(const SpeciesSet& alreadyPicked, int n, SpeciesSet available) {
    available = SpeciesSet::copyOf(available);
    auto removeShared = [&](Species* picked) {
        available.removeIf([&](Species* poke) {
            return poke->hasType(picked->primaryType(), false) || poke->hasType(picked->secondaryType(), false);
        });
    };
    for (Species* p : alreadyPicked.toVector()) removeShared(p);
    std::vector<Species*> picks;
    while (static_cast<int>(picks.size()) < n) {
        Species* p = available.getRandomSpecies(random);
        picks.push_back(p);
        available.remove(p);
        removeShared(p);
    }
    random.shuffle(picks);  // si no, los tipos con más Pokémon saldrían antes
    return picks;
}

SpeciesSet StarterRandomizer::getAvailableSet(const std::vector<Species*>& alreadyChosen) {
    bool abilitiesUnchanged = settings.abilitiesMod == Settings::AbilitiesMod::UNCHANGED;
    bool allowAlt = settings.allowStarterAltFormes;
    bool noLegendaries = settings.startersNoLegendaries;
    bool triStageOnly = settings.startersMod == SM::RANDOM_WITH_TWO_EVOLUTIONS;
    bool basicOnly = triStageOnly || settings.startersMod == SM::RANDOM_BASIC;
    int bstMin = settings.startersBSTMinimum;
    int bstMax = settings.startersBSTMaximum == 0 ? 1530 : settings.startersBSTMaximum;
    SpeciesSet available;
    if (allowAlt) {
        available = SpeciesSet::copyOf(noLegendaries ? rSpecService.getNonLegendaries(true) : rSpecService.getAll(true));
        if (abilitiesUnchanged) available.removeAll(rSpecService.getAbilityDependentFormes());
        if (settings.banIrregularAltFormes) available.removeAll(romHandler.getIrregularFormes());
        available.removeIf([](Species* s) { return s->isEssentiallyCosmetic(); });
    } else {
        available = SpeciesSet::copyOf(noLegendaries ? rSpecService.getNonLegendaries(false) : rSpecService.getAll(false));
    }
    available.removeAllRange(alreadyChosen);  // removeAll(List)
    if (settings.startersNoDualTypes) available.removeIf([](Species* s) { return s->hasSecondaryType(false); });
    if (basicOnly) available = available.filterBasic(false);
    if (triStageOnly) available.removeIf([](Species* s) { return s->stagesAfter(false) < 2; });
    if (bstMin != 0 || bstMax != 1530) available.removeIf([&](Species* s) { return s->bst(false) < bstMin || s->bst(false) > bstMax; });
    return available;
}

std::vector<Species*> StarterRandomizer::getCustomStarters(const std::vector<int>& indices) {
    std::vector<Species*> romSpecies;
    for (Species* s : romHandler.getSpeciesInclFormes())
        if (!s || !s->isEssentiallyCosmetic()) romSpecies.push_back(s);
    std::vector<Species*> out;
    for (int i : indices)
        if (i != 0) out.push_back(romSpecies.at(i));
    return out;
}

std::vector<std::vector<Type>> StarterRandomizer::findTypeTriangles() {
    TypeTable tt = romHandler.getTypeTable();
    std::vector<std::vector<Type>> out;
    auto removeOne = [](std::vector<Type>& v, Type t) {
        auto it = std::find(v.begin(), v.end(), t);
        if (it != v.end()) v.erase(it);
    };
    for (Type one : tt.types()) {
        auto se1 = tt.superEffectiveWhenAttacking(one);
        removeOne(se1, one);
        for (Type two : se1) {
            auto se2 = tt.superEffectiveWhenAttacking(two);
            removeOne(se2, one);
            removeOne(se2, two);
            for (Type three : se2) {
                auto se3 = tt.superEffectiveWhenAttacking(three);
                if (std::find(se3.begin(), se3.end(), one) != se3.end()) {
                    std::vector<Type> tri{three, two, one};
                    if (std::find(out.begin(), out.end(), tri) == out.end()) out.push_back(tri);
                }
            }
        }
    }
    return out;
}

void StarterRandomizer::randomizeStarterHeldItems() {
    auto old = romHandler.getStarterHeldItems();
    std::vector<int> possible = (settings.banBadRandomStarterHeldItems ? romHandler.getNonBadItems() : romHandler.getAllowedItems()).toVector();
    std::vector<std::optional<int>> items;
    for (size_t i = 0; i < old.size(); i++) items.push_back(possible[random.nextInt(static_cast<int>(possible.size()))]);
    romHandler.setStarterHeldItems(items);
    changesMade = true;
}

}  // namespace pr
