#include "random/wild_encounter_randomizer.hpp"

#include <algorithm>
#include <set>
#include <stack>

namespace pr {

using S = Settings;

// --- AreaInfo ---

void WildEncounterRandomizer::AreaInfo::addTypeTheme(std::optional<Type> theme, int count) {
    if (count < 1) throw std::invalid_argument("Number of Species in theme cannot be less than 1!");
    if (theme) possibleThemes[*theme] += count;
}

std::optional<Type> WildEncounterRandomizer::AreaInfo::getTheme(bool defaultToPrimary) const {
    if (possibleThemes.empty()) return defaultToPrimary ? species->primaryType(true) : std::nullopt;
    std::optional<Type> best;
    int bestCount = 0;
    for (auto [t, c] : possibleThemes) {
        if (c > bestCount) {
            bestCount = c;
            best = t;
        } else if (c == bestCount) {
            if (species->primaryType(true) == t) best = t;  // empate: preferir el tipo primario
        }
    }
    return best;
}

// --- InnerRandomizer ---

struct WildEncounterRandomizer::Inner {
    WildEncounterRandomizer& o;
    bool randomTypeThemes, keepTypeThemes, keepPrimaryType, needsTypes, catchEmAll, similarStrength,
        balanceLowLevelEncounters, basicPokemonOnly, sameEvoStage, keepEvolutions, noPrematureEvolutions;
    bool useMapping = false;
    std::map<Type, SetPtr> allowedByType, remainingByType;
    SetPtr allowed;
    SpeciesSet banned;
    SetPtr remaining;
    JavaHashMap<Species*, Species*, JavaHashCode<Species*>, SpeciesPtrEq> zoneMap;
    std::optional<InfoMap> areaInfo;  // null hasta la primera zona con mapeo

    Inner(WildEncounterRandomizer& owner, const SpeciesSet& allowedIn, const SpeciesSet& bannedIn, bool rtt, bool ktt, bool kpt,
          bool cea, bool ss, bool blle, bool bpo, bool ses, bool ke, bool npe)
        : o(owner), randomTypeThemes(rtt), keepTypeThemes(ktt), keepPrimaryType(kpt), catchEmAll(cea), similarStrength(ss),
          balanceLowLevelEncounters(blle), basicPokemonOnly(bpo), sameEvoStage(ses), keepEvolutions(ke), banned(bannedIn) {
        if (rtt && kpt) throw std::invalid_argument("Can't use keepPrimaryType with randomTypeThemes.");
        if (bpo && ses) throw std::invalid_argument("Can't use basicPokemonOnly with sameEvoStage.");
        noPrematureEvolutions = !ses && !ke && npe;
        needsTypes = kpt || ktt || rtt;
        allowed = std::make_shared<SpeciesSet>(bpo && !ke ? allowedIn.filterBasic(false) : allowedIn);
        if (needsTypes) {
            // (sic) FVX ordena por tipo el conjunto "allowed" original, no el filtrado
            for (auto& [t, set] : allowedIn.sortByType(false, o.typeService.getTypes()))
                allowedByType[t] = std::make_shared<SpeciesSet>(set);
        }
        refillRemainingSpecies();
    }

    void refillRemainingSpecies() {
        remaining = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(*allowed));
        if (needsTypes) {
            remainingByType.clear();
            for (Type t : o.typeService.getTypes()) remainingByType[t] = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(*allowedByType.at(t)));
        }
    }

    void randomEncounters(const std::vector<EncounterArea*>& areas) {
        useMapping = false;
        std::vector<Zone> zones;
        for (auto* a : areas) zones.push_back({a});
        randomizeZones(zones);
    }
    void area1to1Encounters(const std::vector<EncounterArea*>& areas) {
        useMapping = true;
        std::vector<Zone> zones;
        for (auto* a : areas) zones.push_back({a});
        randomizeZones(zones);
    }

    static std::vector<Zone> groupByMapIndex(const std::vector<EncounterArea*>& areas) {
        JavaHashMap<int, Zone> m;
        for (auto* a : areas) m.getOrInsert(a->mapIndex).push_back(a);
        std::vector<Zone> out;
        m.forEach([&](const int&, const Zone& z) { out.push_back(z); });
        return out;
    }
    static std::vector<Zone> groupByEncounterType(const Zone& areas) {
        // AVISO: en FVX es HashMap<EncounterType, ...> (orden no reproducible en Java); aquí, orden del enum
        std::map<int, Zone> m;
        for (auto* a : areas) m[a->encounterType ? static_cast<int>(*a->encounterType) : -1].push_back(a);
        std::vector<Zone> out;
        for (auto& [k, z] : m) out.push_back(z);
        return out;
    }
    static std::vector<Zone> groupByLocation(const std::vector<EncounterArea*>& areas) {
        JavaHashMap<std::string, Zone> m;
        int untagged = 1;
        for (auto* a : areas) {
            std::string tag = a->locationTag ? *a->locationTag : "UNTAGGED-" + std::to_string(untagged++);
            m.getOrInsert(tag).push_back(a);
        }
        std::vector<Zone> out;
        m.forEach([&](const std::string&, const Zone& z) { out.push_back(z); });
        return out;
    }
    static std::vector<Zone> splitZones(const std::vector<Zone>& zones) {
        std::vector<Zone> out;
        for (const auto& z : zones)
            for (auto& sub : groupByEncounterType(z)) out.push_back(sub);
        return out;
    }

    void map1to1Encounters(const std::vector<EncounterArea*>& areas, bool split) {
        useMapping = true;
        auto zones = groupByMapIndex(areas);
        randomizeZones(split ? splitZones(zones) : zones);
    }
    void location1to1Encounters(const std::vector<EncounterArea*>& areas, bool split) {
        useMapping = true;
        auto zones = groupByLocation(areas);
        randomizeZones(split ? splitZones(zones) : zones);
    }
    void game1to1Encounters(const std::vector<EncounterArea*>& areas, bool split) {
        useMapping = true;
        std::vector<Zone> zones = split ? groupByEncounterType(areas) : std::vector<Zone>{areas};
        randomizeZones(zones);
    }

    void randomizeZones(std::vector<Zone> zones) {
        o.random.shuffle(zones);  // si no, las zonas grandes tenderían a ir primero
        for (const auto& zone : zones) {
            auto zoneType = pickZoneType(zone);
            if (useMapping) {
                zoneMap = {};
                setupAreaInfoMap(zone);
                if (keepEvolutions) spreadThemesThroughFamilies();
            }
            for (auto* area : zone) randomizeArea(*area, zoneType);
            if (useMapping && !catchEmAll) refillRemainingSpecies();
        }
    }

    void randomizeArea(EncounterArea& area, std::optional<Type> zoneType) {
        for (auto& encPtr : area.encounters) {
            Encounter& enc = *encPtr;
            Species* current = enc.species();
            Species* replacement;
            if (useMapping && zoneMap.containsKey(current)) {
                replacement = *zoneMap.get(current);
            } else {
                if (keepEvolutions && mapHasFamilyMember(current)) {
                    replacement = pickFamilyMemberReplacement(current);
                } else {
                    SetPtr allowedFor = setupAllowedForReplacement(current, area, zoneType, enc.level);
                    replacement = pickReplacement(current, *allowedFor);
                }
                if (useMapping) zoneMap.put(current, replacement);
                if (useMapping || catchEmAll) removeFromRemaining(replacement);
            }
            enc.holder.setSpecies(replacement);
            o.randomizeCosmeticForme(enc);
        }
        if (area.forceMultipleSpecies) enforceMultipleSpecies(area);
    }

    Species* pickFamilyMemberReplacement(Species* toReplace) {
        auto info = *areaInfo->get(toReplace);
        for (Species* relative : info->getFamily().toVector()) {
            if (zoneMap.containsKey(relative)) return pickFamilyMemberReplacementInner(toReplace, relative);
        }
        throw std::invalid_argument("Tried to pick family member replacement for non-mapped Species!");
    }

    Species* pickFamilyMemberReplacementInner(Species* toReplace, Species* relative) {
        auto info = *areaInfo->get(toReplace);
        int relation = relative->relation(toReplace, true);
        Species* relRep = *zoneMap.get(relative);
        if (!relRep) throw std::invalid_argument("Relative had a null replacement!");
        SpeciesSet possible = relRep->relativesAtPosition(relation, false);
        possible.retainAll(*remaining);
        possible.removeAll(info->getBannedForReplacement());
        if (!possible.empty()) return pickReplacement(toReplace, possible);
        possible = relRep->relativesAtPosition(relation, false);
        possible.retainAll(*allowed);
        possible.removeAll(info->getBannedForReplacement());
        if (!possible.empty()) return pickReplacement(toReplace, possible);
        throw std::logic_error("Chose a family that is invalid!");
    }

    bool mapHasFamilyMember(Species* current) {
        auto info = *areaInfo->get(current);
        for (Species* r : info->getFamily().toVector())
            if (zoneMap.containsKey(r)) return true;
        return false;
    }

    std::optional<Type> pickZoneType(const Zone& zone) {
        std::optional<Type> picked;
        if (keepTypeThemes) {
            std::set<Type> possible;
            for (int i = 0; i <= static_cast<int>(Type::FAIRY); i++) possible.insert(static_cast<Type>(i));
            for (auto* area : zone) {
                auto shared = area->getSpeciesInArea().getSharedTypes(true);
                std::set<Type> inter;
                for (Type t : possible)
                    if (shared.count(t)) inter.insert(t);
                possible = inter;
                if (possible.empty()) break;
            }
            if (!possible.empty()) {
                auto it = possible.begin();
                picked = *it++;
                if (it != possible.end()) {
                    if (picked == Type::NORMAL) {
                        picked = *it;  // preferir no Normal
                    } else {
                        auto preferred = zone.front()->encounters.front()->species()->primaryType(true);
                        if (picked != preferred) picked = *it;
                    }
                }
            }
        }
        if (!picked && randomTypeThemes) {
            picked = pickRandomTypeWithSpeciesRemaining();
            // con "atrápalos a todos" los vetados (p. ej. Unown) no cambian: el tema debe ser su tipo
            if (catchEmAll) {
                SpeciesSet bannedInArea = SpeciesSet::copyOf(banned);
                SpeciesSet inZone;
                for (auto* area : zone) inZone.addAll(area->getSpeciesInArea());
                bannedInArea.retainAll(inZone);
                auto theme = bannedInArea.getSharedType(false);
                if (theme) picked = theme;
            }
        }
        return picked;
    }

    std::optional<Type> findAreaType(const EncounterArea& area) {
        if (keepTypeThemes) return area.getSpeciesInArea().getSharedType(true);
        return std::nullopt;
    }

    Type pickRandomTypeWithSpeciesRemaining() {
        auto types = o.typeService.getTypes();
        o.random.shuffle(types);
        Type t;
        do {
            t = types.front();
            types.erase(types.begin());
        } while (remainingByType.at(t)->empty() && !types.empty());
        if (types.empty() && remainingByType.at(t)->empty())
            throw std::logic_error("RemainingByType contained no Species of any valid type!");
        return t;
    }

    SetPtr setupAllowedForReplacement(Species* current, EncounterArea& area, std::optional<Type> zoneType, int level) {
        SetPtr r = !areaInfo ? setupNoInfoMap(current, area, zoneType, level) : setupUsingInfoMap(current, zoneType, level);
        if (r->empty())
            throw RandomizationException("Could not find a wild Species replacement for " + current->fullName() + " in area " +
                                         area.displayName + "!");
        return r;
    }

    SetPtr pool(std::optional<Type> t, bool fromRemaining) {
        if (fromRemaining) return t ? remainingByType.at(*t) : remaining;
        return t ? allowedByType.at(*t) : allowed;
    }

    SetPtr setupUsingInfoMap(Species* current, std::optional<Type> theme, int level) {
        auto infoPtr = areaInfo->get(current);
        if (!infoPtr) throw std::logic_error("Info was null for encounter's species!");
        auto info = *infoPtr;
        auto typeFor = theme ? theme : info->getTheme(keepPrimaryType);
        bool needsInner = !info->bannedForReplacement.empty() || keepEvolutions || sameEvoStage || noPrematureEvolutions;
        SetPtr p = pool(typeFor, true);
        if (needsInner) p = innerInfoMap(*info, p, level);
        if (!p->empty()) return p;
        p = pool(typeFor, false);
        if (needsInner) p = innerInfoMap(*info, p, level);
        return p;
    }

    SetPtr setupNoInfoMap(Species* current, EncounterArea& area, std::optional<Type> theme, int level) {
        auto typeFor = theme ? theme : (keepPrimaryType ? current->primaryType(true) : std::nullopt);
        SpeciesSet areaBanned = area.getBannedSpecies();
        bool needsInner = !areaBanned.empty() || keepEvolutions || sameEvoStage || noPrematureEvolutions;
        SetPtr p = pool(typeFor, true);
        if (needsInner) p = innerNoMap(p, current, areaBanned, level);
        if (!p->empty()) return p;
        p = pool(typeFor, false);
        if (needsInner) p = innerNoMap(p, current, areaBanned, level);
        return p;
    }

    SetPtr innerInfoMap(const AreaInfo& info, SetPtr start, int level) {
        SetPtr a = start;
        if (!info.bannedForReplacement.empty()) {
            a = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(*start));
            a->removeAll(info.getBannedForReplacement());
        }
        if (sameEvoStage) {
            int stage = info.species->stagesBefore(true);
            a = std::make_shared<SpeciesSet>(a->filter([stage](Species* s) { return s->stagesBefore(false) == stage; }));
        }
        if (keepEvolutions) a = std::make_shared<SpeciesSet>(setupAllowedForFamily(*a, info));
        if (noPrematureEvolutions)
            a = std::make_shared<SpeciesSet>(a->filter([level](Species* s) { return s->isLegalEvolutionAtLevel(level, 1); }));
        return a;
    }

    SetPtr innerNoMap(SetPtr start, Species* current, const SpeciesSet& areaBanned, int level) {
        SetPtr a = start;
        if (!areaBanned.empty()) {
            a = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(*start));
            a->removeAll(areaBanned);
        }
        if (sameEvoStage) {
            int stage = current->stagesBefore(true);
            a = std::make_shared<SpeciesSet>(a->filter([stage](Species* s) { return s->stagesBefore(false) == stage; }));
        }
        if (noPrematureEvolutions)
            a = std::make_shared<SpeciesSet>(a->filter([level](Species* s) { return s->isLegalEvolutionAtLevel(level, 1); }));
        return a;
    }

    Species* pickReplacement(Species* current, SpeciesSet& allowedFor) {
        if (allowedFor.empty()) throw std::invalid_argument("No allowed Species to pick as replacement.");
        // con "atrápalos a todos", los Pokémon vetados no se cambian (si no, podrían ser imposibles de conseguir)
        if (catchEmAll && banned.contains(current)) return current;
        if (similarStrength) {
            if (balanceLowLevelEncounters && areaInfo) {
                auto info = *areaInfo->get(current);
                int bst = std::min(current->bst(true), info->lowestLevel * 10 + 250);
                return allowedFor.getRandomSimilarStrengthSpecies(bst, o.random);
            }
            return allowedFor.getRandomSimilarStrengthSpecies(current, o.random);
        }
        return allowedFor.getRandomSpecies(o.random);
    }

    void removeFromRemaining(Species* r) {
        remaining->remove(r);
        if (needsTypes) {
            remainingByType.at(*r->primaryType())->remove(r);
            if (r->hasSecondaryType(false)) remainingByType.at(*r->secondaryType())->remove(r);
        }
        if (remaining->empty()) refillRemainingSpecies();
    }

    void enforceMultipleSpecies(EncounterArea& area) {
        auto distinctCount = [&] {
            std::vector<const Encounter*> d;
            for (auto& e : area.encounters) {
                bool found = std::any_of(d.begin(), d.end(), [&](const Encounter* x) {
                    return x->level == e->level && x->maxLevel == e->maxLevel && x->holder == e->holder &&
                           x->isSOS == e->isSOS && x->sosType == e->sosType;
                });
                if (!found) d.push_back(e.get());
            }
            return d.size();
        };
        while (distinctCount() == 1) area.encounters.front()->holder.setSpecies(o.rSpecService.randomSpecies(o.random));
    }

    void spreadThemesThroughFamilies() {
        SpeciesSet completed;
        if (!areaInfo) throw std::logic_error("Cannot spread themes before determining themes!");
        std::vector<std::shared_ptr<AreaInfo>> infos;
        areaInfo->forEach([&](Species* const&, const std::shared_ptr<AreaInfo>& i) { infos.push_back(i); });
        for (const auto& info : infos) {
            Species* poke = info->species;
            if (completed.contains(poke)) continue;
            SpeciesSet family = info->getFamily();
            completed.addAll(family);
            std::map<Type, int> familyThemes;
            for (Species* rel : family.toVector()) {
                auto ri = areaInfo->get(rel);
                if (!ri) throw std::invalid_argument("Cannot spread themes among Species without theme information!");
                for (auto [t, c] : (*ri)->possibleThemes) familyThemes[t] += c;
            }
            for (Species* rel : family.toVector()) (*areaInfo->get(rel))->possibleThemes = familyThemes;
        }
    }

    SpeciesSet setupAllowedForFamily(SpeciesSet potentially, const AreaInfo& info) {
        SpeciesSet family = info.getFamily();
        Species* match = info.species;
        int before = family.getNumberEvoStagesBefore(match, true);
        int after = family.getNumberEvoStagesAfter(match, true);
        potentially = potentially.filter([&](Species* s) { return s->stagesBefore(false) >= before && s->stagesAfter(false) >= after; });
        if (basicPokemonOnly) potentially.removeIf([&](Species* s) { return s->stagesBefore(false) > before; });
        for (Species* relative : family.toVector()) {
            int relation = match->relation(relative, true);
            potentially = potentially.filter([&](Species* p) {
                SpeciesSet same = p->relativesAtPositionSameBranch(relation, false);
                same.retainAll(*allowed);
                same.removeAll((*areaInfo->get(relative))->getBannedForReplacement());
                return !same.empty();
            });
        }
        std::vector<Species*> used;
        zoneMap.forEach([&](Species* const&, Species* const& v) { used.push_back(v); });
        SpeciesSet usedSet = SpeciesSet::of(used);
        SpeciesSet without = potentially.filter([&](Species* p) { return !p->family(false).containsAny(usedSet); });
        return without.empty() ? potentially : without;
    }

    void setupAreaInfoMap(const Zone& areas) {
        SpeciesSet existing;
        areaInfo.emplace();
        for (auto* area : areas) {
            auto theme = findAreaType(*area);
            SpeciesSet inArea = area->getSpeciesInArea();
            int areaSize = inArea.size();
            for (Species* sp : inArea.toVector()) {
                auto existingInfo = areaInfo->get(sp);
                std::shared_ptr<AreaInfo> info;
                if (!existingInfo) {
                    info = std::make_shared<AreaInfo>(sp);
                    areaInfo->put(sp, info);
                    if (keepEvolutions) {
                        existing.add(sp);
                        SpeciesSet fam = existing.filterFamily(sp, true);
                        if (fam.size() > 1)
                            for (Species* r : fam.toVector()) (*areaInfo->get(r))->family.addAll(fam);
                    }
                } else {
                    info = *existingInfo;
                }
                info->addTypeTheme(theme, areaSize);
                info->bannedForReplacement.addAll(area->getBannedSpecies());
            }
            if (balanceLowLevelEncounters) {
                for (auto& e : area->encounters) {
                    auto info = *areaInfo->get(e->species());
                    info->lowestLevel = std::min((e->level + e->maxLevel) / 2, info->lowestLevel);
                }
            }
        }
    }
};

// --- WildEncounterRandomizer ---

void WildEncounterRandomizer::randomizeEncounters() {
    bool useTimeOfDay = settings.useTimeBasedEncounters;
    int levelModifier = settings.wildLevelsModified ? settings.wildLevelModifier : 0;
    auto areas = romHandler.getEncounters(useTimeOfDay);
    if (!settings.randomizeWildPokemon) {
        applyLevelModifier(levelModifier, areas);
        romHandler.setEncounters(useTimeOfDay, areas);
        return;
    }
    auto prepped = prepEncounterAreas(areas);
    SpeciesSet banned = getBannedForWildEncounters(settings.banIrregularAltFormes, settings.abilitiesMod == S::AbilitiesMod::RANDOMIZE);
    SpeciesSet allowed = SpeciesSet::copyOf(rSpecService.getSpecies(settings.blockWildLegendaries, settings.allowWildAltFormes, false));
    allowed.removeAll(banned);
    applyLevelModifier(levelModifier, areas);
    Inner ir(*this, allowed, banned, settings.wildPokemonTypeMod == S::WildPokemonTypeMod::RANDOM_THEMES, settings.keepWildTypeThemes,
             settings.wildPokemonTypeMod == S::WildPokemonTypeMod::KEEP_PRIMARY, settings.catchEmAllEncounters,
             settings.similarStrengthEncounters, settings.balanceShakingGrass,
             settings.wildPokemonEvolutionMod == S::WildPokemonEvolutionMod::BASIC_ONLY,
             settings.wildPokemonEvolutionMod == S::WildPokemonEvolutionMod::KEEP_STAGE, settings.keepWildEvolutionFamilies,
             settings.banPrematureEvos);
    bool split = settings.splitWildZoneByEncounterTypes;
    switch (settings.wildPokemonZoneMod) {
        case S::WildPokemonZoneMod::NONE:
            if (romHandler.isORAS()) throw std::logic_error("ORAS no está soportado");
            ir.randomEncounters(prepped);
            break;
        case S::WildPokemonZoneMod::ENCOUNTER_SET: ir.area1to1Encounters(prepped); break;
        case S::WildPokemonZoneMod::MAP: ir.map1to1Encounters(prepped, split); break;
        case S::WildPokemonZoneMod::NAMED_LOCATION: ir.location1to1Encounters(prepped, split); break;
        case S::WildPokemonZoneMod::GAME: ir.game1to1Encounters(prepped, split); break;
    }
    romHandler.setEncounters(useTimeOfDay, areas);
    changesMade = true;
}

std::vector<EncounterArea*> WildEncounterRandomizer::prepEncounterAreas(std::vector<EncounterArea>& areas) {
    std::vector<EncounterArea*> prepped;
    for (auto& a : areas) {
        if (a.encounterType == EncounterType::UNUSED || (a.locationTag && *a.locationTag == "UNUSED")) continue;
        prepped.push_back(&a);
    }
    // las formas casi cosméticas cuentan como su forma base
    for (auto* a : prepped)
        for (auto& e : a->encounters)
            if (e->species()->isEssentiallyCosmetic()) e->holder.setSpecies(e->species()->baseForme());
    random.shuffle(prepped);
    return prepped;
}

SpeciesSet WildEncounterRandomizer::getBannedForWildEncounters(bool banIrregular, bool abilitiesRandomized) {
    SpeciesSet banned;
    banned.addAll(romHandler.getBannedForWildEncounters());
    banned.addAll(rSpecService.getBannedFormesForPlayerPokemon());
    if (!abilitiesRandomized) banned.addAll(rSpecService.getAbilityDependentFormes());
    if (banIrregular) banned.addAll(romHandler.getIrregularFormes());
    return banned;
}

void WildEncounterRandomizer::applyLevelModifier(int mod, std::vector<EncounterArea>& areas) {
    if (mod == 0) return;
    for (auto& a : areas)
        for (auto& e : a.encounters) {
            e->level = applyPercentageLevelModifier(e->level, mod);
            e->maxLevel = applyPercentageLevelModifier(e->maxLevel, mod);
        }
}

void WildEncounterRandomizer::randomizeCosmeticForme(Encounter& enc) {
    auto& sh = enc.holder;
    if (sh.isAltFormeAllowed() && sh.species()->isBaseForme()) {
        Species* base = sh.species()->baseForme();
        sh.setFormeNumber(base->randomCosmeticFormeNumber(random));
    }
}

void WildEncounterRandomizer::changeCatchRates() {
    int level = settings.minimumCatchRateLevel;
    if (level == 5) {
        romHandler.enableGuaranteedPokemonCatching();
        return;
    }
    int normal, legendary;
    switch (level) {
        case 2: normal = 128; legendary = 64; break;
        case 3: normal = 200; legendary = 100; break;
        case 4: normal = legendary = 255; break;
        default: normal = 75; legendary = 37;
    }
    minimumCatchRate(normal, legendary);
}

void WildEncounterRandomizer::minimumCatchRate(int rn, int rl) {
    for (Species* sp : romHandler.getSpeciesSetInclFormes().toVector())
        sp->catchRate = std::max(sp->catchRate, sp->isLegendary() ? rl : rn);
}

}  // namespace pr
