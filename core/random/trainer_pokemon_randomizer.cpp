#include "random/trainer_pokemon_randomizer.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"

namespace pr {

using S = Settings;
using TM = Settings::TrainersMod;

void TrainerPokemonRandomizer::applyTrainerLevelModifier() {
    int mod = settings.trainersLevelModifier;
    for (const auto& t : romHandler.getTrainers()) {
        if (t->isFirstRivalOrFriend() && mod > 0) continue;
        if (mod != 0)
            for (auto& tp : t->pokemon) tp->level = applyPercentageLevelModifier(tp->level, mod);
    }
    changesMade = true;
}

void TrainerPokemonRandomizer::randomizeTrainerPokes() {
    bool usePowerLevels = settings.trainersUsePokemonOfSimilarStrength;
    bool avoidDuplicates = settings.trainersAvoidDuplicates;
    bool weightByFrequency = settings.trainersMatchTypingDistribution;
    bool useLocal = settings.trainersUseLocalPokemon;
    bool noLegendaries = settings.trainersBlockLegendaries;
    bool noEarlyWG = settings.trainersBlockEarlyWonderGuard;
    bool isTypeThemed = settings.trainersMod == TM::TYPE_THEMED;
    bool isTypeThemedE4Gym = settings.trainersMod == TM::TYPE_THEMED_ELITE4_GYMS;
    bool keepTypeThemes = settings.trainersMod == TM::KEEP_THEMED;
    bool keepThemeOrPrimary = settings.trainersMod == TM::KEEP_THEME_OR_PRIMARY;
    bool distribution = settings.trainersMod == TM::DISTRIBUTED;
    bool mainPlaythrough = settings.trainersMod == TM::MAINPLAYTHROUGH;
    bool includeFormes = settings.allowTrainerAlternateFormes;
    bool banIrregular = settings.banIrregularAltFormes;
    bool noPremature = settings.banPrematureEvos;
    bool swapMegaEvos = settings.swapTrainerMegaEvos;
    bool shinyChance = settings.shinyChance;
    bool abilitiesRandomized = settings.abilitiesMod == S::AbilitiesMod::RANDOMIZE;
    int e4Unique = settings.eliteFourUniquePokemonNumber;
    bool evolveFar = settings.trainersEvolveTheirPokemon;
    int evoMod = settings.trainersEvolutionLevelModifier;
    bool forceChallenge = (settings.currentMiscTweaks & misctweak::FORCE_CHALLENGE_MODE.value) > 0;
    bool rivalCarriesStarter = settings.rivalCarriesStarterThroughout;
    bool bossDiv = settings.diverseTypesForBossTrainers, impDiv = settings.diverseTypesForImportantTrainers,
         regDiv = settings.diverseTypesForRegularTrainers;
    bool skipOriginal = settings.trainersMod == TM::UNCHANGED;
    // con UNCHANGED solo se randomizan los Pokémon añadidos
    if (skipOriginal) {
        keepTypeThemes = true;
        banIrregular = true;
    }
    bool hasAnyTypeTheme = isTypeThemed || isTypeThemedE4Gym || keepTypeThemes || keepThemeOrPrimary;

    cachedByType_.clear();
    cachedAll_ = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(rSpecService.getSpecies(noLegendaries, includeFormes, false)));
    if (useLocal) {
        cachedAll_->retainAll(romHandler.getMainGameWildPokemonSpecies(settings.useTimeBasedEncounters).buildFullFamilies(false));
    }
    SpeciesSet banned = SpeciesSet::copyOf(romHandler.getBannedFormesForTrainerPokemon());
    if (!abilitiesRandomized) banned.addAll(rSpecService.getAbilityDependentFormes());
    if (banIrregular) banned.addAll(romHandler.getIrregularFormes());
    cachedAll_->removeAll(banned);
    std::optional<SpeciesSet> wonderGuard;
    if (noEarlyWG) {
        wonderGuard = cachedAll_->filter([](Species* p) {
            return p->ability1 == abilities::wonderGuard || p->ability2 == abilities::wonderGuard ||
                   p->ability3 == abilities::wonderGuard;
        });
    }
    SpeciesSet altFormes = cachedAll_->filter([](Species* p) { return !p->isBaseForme(); });
    auto& trainers = romHandler.getTrainers();
    if (hasAnyTypeTheme) {
        for (auto& [t, set] : cachedAll_->sortByType(false)) cachedByType_[t] = std::make_shared<SpeciesSet>(set);
        typeWeightings_.clear();
        totalTypeWeighting_ = 0;
        // getTrainerGroups (TreeMap<String, List<Trainer>>)
        std::map<std::string, std::vector<Trainer*>> groups;
        for (const auto& t : trainers) {
            std::string g = t->tag.value_or("");
            if (auto d = g.find('-'); d != std::string::npos) g = g.substr(0, d);
            if (jc::startsWith(g, "GYM") || jc::startsWith(g, "ELITE") ||
                ((jc::startsWith(g, "CHAMPION") || jc::startsWith(g, "THEMED")) && !isTypeThemedE4Gym)) {
                groups[g].push_back(t.get());
            } else if (jc::startsWith(g, "GIO")) {
                groups["GYM8"].push_back(t.get());  // Giovanni va con su gimnasio
            }
        }
        std::vector<std::string> names;
        for (auto& [k, v] : groups) names.push_back(k);
        auto themes = pickGroupTypeThemes(keepTypeThemes || keepThemeOrPrimary, names);
        for (auto& [g, list] : groups) {
            auto it = themes.find(g);
            if (it == themes.end()) continue;
            for (Trainer* t : list) trainerTypes_[t->index] = it->second;
        }
    }
    std::vector<Trainer*> scrambled;
    for (const auto& t : trainers) scrambled.push_back(t.get());
    random.shuffle(scrambled);  // orden menos predecible; el orden real se conserva al guardar

    bool e4UniqueOn = e4Unique > 0;
    SpeciesSet bannedFromUnique;
    std::vector<int> e4Indices = romHandler.getEliteFourTrainers(forceChallenge);
    auto isE4 = [&](int idx) { return std::find(e4Indices.begin(), e4Indices.end(), idx) != e4Indices.end(); };
    SetPtr e4Exceptions;
    if (e4UniqueOn) {
        // Alto Mando al principio (List.sort es estable)
        auto indexOf = [&](Trainer* t) {
            for (size_t i = 0; i < trainers.size(); i++)
                if (trainers[i].get() == t) return static_cast<int>(i);
            return -1;
        };
        std::stable_sort(scrambled.begin(), scrambled.end(), [&](Trainer* a, Trainer* b) {
            return isE4(indexOf(a) + 1) && !isE4(indexOf(b) + 1);
        });
        if (rivalCarriesStarter) {
            auto starters = romHandler.getStarters();
            int n = std::min(static_cast<int>(starters.size()), 3);
            for (int i = 0; i < n; i++) bannedFromUnique.addFamily(starters[i], false);
        }
        if (useLocal) {
            e4Exceptions = std::make_shared<SpeciesSet>(SpeciesSet::copyOf(rSpecService.getSpecies(noLegendaries, includeFormes, false)));
            e4Exceptions->removeAll(banned);
            e4Exceptions->removeAll(*cachedAll_);  // solo los no locales
        }
    }
    std::vector<int> mainPlaythroughTrainers = romHandler.getMainPlaythroughTrainers();
    auto inMain = [&](int idx) {
        return std::find(mainPlaythroughTrainers.begin(), mainPlaythroughTrainers.end(), idx) != mainPlaythroughTrainers.end();
    };

    for (Trainer* t : scrambled) {
        auto typeForTrainer = getTypeForTrainer(*t, isTypeThemed, weightByFrequency, noLegendaries, includeFormes,
                                                keepTypeThemes, keepThemeOrPrimary);
        std::vector<TrainerPokemonPtr> list = t->pokemon;
        bool skipStarter = rivalCarriesStarter && t->tag && (jc::contains(*t->tag, "RIVAL") || jc::contains(*t->tag, "FRIEND"));
        bool e4Track = e4UniqueOn && isE4(t->index);
        auto sortDesc = [&] {
            std::reverse(list.begin(), list.end());
            std::stable_sort(list.begin(), list.end(), [](const TrainerPokemonPtr& a, const TrainerPokemonPtr& b) { return b->level < a->level; });
        };
        if (t->forceStarterPosition != -1 && skipStarter) {
            auto starter = list.at(t->forceStarterPosition);
            list.erase(list.begin() + t->forceStarterPosition);
            if (e4Track) sortDesc();
            list.insert(list.begin(), starter);
        } else if (e4Track || skipStarter) {
            sortDesc();
        }
        bool forceTypeDiverse = (t->isBoss() && bossDiv) || (t->isImportant() && impDiv) || (!t->isBoss() && !t->isImportant() && regDiv);
        std::set<Type> usedTypes;
        SpeciesSet alreadyPlaced;
        auto updateUsedTypes = [&](Species* sp) {
            if (forceTypeDiverse && !typeForTrainer) {
                usedTypes.insert(*sp->primaryType());
                if (sp->hasSecondaryType(false)) usedTypes.insert(*sp->secondaryType());
            }
        };
        if ((avoidDuplicates || forceTypeDiverse) && skipOriginal) {
            for (auto& tp : list) {
                if (tp->isAddedTeamMember) continue;
                if (avoidDuplicates) alreadyPlaced.add(tp->species());
                updateUsedTypes(tp->species());
            }
        }
        for (size_t idx = 0; idx < list.size(); idx++) {
            TrainerPokemon& tp = *list[idx];
            // indexOf(tp) en Java: primera aparición del mismo objeto
            int indexOfTp = static_cast<int>(std::find(list.begin(), list.end(), list[idx]) - list.begin());
            bool e4SetUnique = e4Track && e4Unique > indexOfTp;
            bool swapThisMega = false;  // canMegaEvolve: no hay megas en Gen 3-5
            (void)swapMegaEvos;
            Species* oldSp = tp.species();
            Species* newSp;
            int tpLevel = tp.level;
            double evoLvlModifier = 1 + evoMod / 100.0;
            if (skipStarter) {
                newSp = oldSp;  // ya se puso el inicial del rival
                skipStarter = false;
            } else if (skipOriginal && !tp.isAddedTeamMember) {
                if (evolveFar) {
                    newSp = evolveAsFarAsLegal(oldSp, tpLevel, evoLvlModifier);
                    tp.holder.setSpecies(newSp);
                    randomizeCosmeticForme(tp);
                    tp.abilitySlot = getValidAbilitySlotFromOriginal(*newSp, tp.abilitySlot);
                } else {
                    newSp = oldSp;
                }
            } else {
                SetPtr cacheReplacement;
                bool wgAllowed = !noEarlyWG || tpLevel >= 20;
                SpeciesSet bannedForReplacement = SpeciesSet::copyOf(usedAsUnique_);
                if (e4SetUnique) {
                    bannedForReplacement.addAll(bannedFromUnique);
                    cacheReplacement = e4Exceptions;
                }
                if (!wgAllowed) bannedForReplacement.addAll(*wonderGuard);
                if (!tp.holder.isAltFormeAllowed()) bannedForReplacement.addAll(altFormes);
                newSp = pickTrainerPokeReplacement(
                    oldSp, tpLevel, usePowerLevels, alreadyPlaced, noPremature,
                    (keepThemeOrPrimary && !typeForTrainer) ? oldSp->primaryType(true) : typeForTrainer,
                    distribution || (mainPlaythrough && inMain(t->index)), swapThisMega, cacheReplacement, evolveFar, usedTypes,
                    bannedForReplacement);
                tp.holder.setSpecies(newSp);
                randomizeCosmeticForme(tp);
                tp.abilitySlot = getRandomAbilitySlot(*newSp);
                tp.resetMoves = true;
            }
            if (distribution || (mainPlaythrough && inMain(t->index))) placementHistory_.getOrInsert(newSp) += 1;
            if (avoidDuplicates) alreadyPlaced.add(newSp);
            if (e4SetUnique) usedAsUnique_.add(newSp);
            if (e4Track) bannedFromUnique.add(newSp);
            updateUsedTypes(newSp);
            if (shinyChance && random.nextInt(256) == 0) tp.IVs = tp.IVs | (1 << 30);
        }
    }
    changesMade = true;
}

std::optional<Type> TrainerPokemonRandomizer::getTypeForTrainer(const Trainer& t, bool isTypeThemed, bool weightByFrequency,
                                                                bool noLegendaries, bool includeFormes, bool keepTypeThemes,
                                                                bool keepThemeOrPrimary) {
    std::optional<Type> type;
    if (auto it = trainerTypes_.find(t.index); it != trainerTypes_.end()) type = it->second;
    if (!type && isTypeThemed) {
        type = pickType(weightByFrequency, noLegendaries, includeFormes);
        if (t.tag && *t.tag == "UBER") {  // los "uber" no pueden repetir tipo
            while (usedUberTypes_.count(*type)) type = pickType(weightByFrequency, noLegendaries, includeFormes);
            usedUberTypes_.insert(*type);
        }
    }
    if ((keepTypeThemes || keepThemeOrPrimary) && !type) {
        SpeciesSet sp;
        for (const auto& tp : t.pokemon) sp.add(tp->species());
        type = sp.getSharedType(true);
    }
    return type;
}

std::map<std::string, Type> TrainerPokemonRandomizer::pickGroupTypeThemes(bool useOriginal, const std::vector<std::string>& groupNames) {
    std::map<std::string, Type> out;
    if (useOriginal) {
        for (auto& [k, v] : romHandler.getGymAndEliteTypeThemes()) out[k] = v;
        return out;
    }
    std::set<Type> usedGymTypes;
    auto remaining = typeService.getTypes();
    random.shuffle(remaining);
    std::vector<std::string> shuffled = groupNames;
    random.shuffle(shuffled);  // (sic) FVX baraja pero luego recorre groupNames
    std::vector<std::string> post8;
    for (const auto& g : groupNames) {
        if ((jc::startsWith(g, "GYM1") && g != "GYM1") || g == "GYM9") {
            post8.push_back(g);  // gimnasios a partir del 9.º, al final por si faltan tipos
            continue;
        }
        if (jc::startsWith(g, "THEMED")) {
            out[g] = typeService.randomType(random);
            continue;
        }
        if (remaining.empty()) throw RandomizationException("Unexpected amount of Elite/Champions; could not assign types to all!");
        Type t = remaining.front();
        remaining.erase(remaining.begin());
        out[g] = t;
        if (jc::startsWith(g, "GYM")) usedGymTypes.insert(t);
        if (g == "CHAMPION") usedUberTypes_.insert(t);
    }
    for (const auto& g : post8) {
        Type t;
        if (!remaining.empty()) {
            t = remaining.front();
            remaining.erase(remaining.begin());
        } else {
            do {
                t = typeService.randomType(random);
            } while (usedGymTypes.count(t));
        }
        out[g] = t;
        usedGymTypes.insert(t);
    }
    return out;
}

Species* TrainerPokemonRandomizer::pickTrainerPokeReplacement(Species* current, int level, bool usePowerLevels,
                                                              SpeciesSet& alreadyPlaced, bool noPremature,
                                                              std::optional<Type> type, bool usePlacementHistory,
                                                              bool swapMegaEvos, SetPtr useInsteadOfCached, bool evolveFar,
                                                              const std::set<Type>& bannedTypes, const SpeciesSet& bannedPokemon) {
    SetPtr cacheOrReplacement = useInsteadOfCached ? useInsteadOfCached : cachedAll_;
    SetPtr pickFrom;
    if (swapMegaEvos) {
        auto megas = std::make_shared<SpeciesSet>();
        for (Species* m : rSpecService.getMegaEvolutions().toVector())
            if (m->needsMegaEvolutionItem()) megas->add(m->baseForme());
        pickFrom = megas;
    } else {
        pickFrom = cacheOrReplacement;
    }
    if (type && !cachedByType_.empty()) {
        SetPtr ofType;
        if (!useInsteadOfCached) {
            auto it = cachedByType_.find(*type);
            if (it == cachedByType_.end())
                throw RandomizationException(std::string("No Pokemon of type ") + typeName(*type) + " available for trainers!");
            ofType = it->second;
        } else {
            ofType = std::make_shared<SpeciesSet>(useInsteadOfCached->filterByType(*type, false));
        }
        if (swapMegaEvos) {
            auto f = std::make_shared<SpeciesSet>(ofType->filter([&](Species* s) { return pickFrom->contains(s); }));
            pickFrom = f->empty() ? ofType : f;
        } else {
            pickFrom = ofType;
        }
    }
    if (!bannedTypes.empty()) {
        pickFrom = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* sp) {
            return !bannedTypes.count(*sp->primaryType()) && (!sp->hasSecondaryType(false) || !bannedTypes.count(*sp->secondaryType()));
        }));
    }
    double evoLvlModifier = 1 + settings.trainersEvolutionLevelModifier / 100.0;
    if (noPremature) pickFrom = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* p) { return p->isLegalEvolutionAtLevel(level, evoLvlModifier); }));
    if (evolveFar) pickFrom = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* p) { return !p->hasLegalEvolutionAtLevel(level, evoLvlModifier); }));
    if (usePlacementHistory) {
        double avg = getPlacementAverage();
        auto below = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* p) {
            const int* h = placementHistory_.get(p);
            return (h ? *h : 0) < avg * 2;
        }));
        if (!below->empty()) pickFrom = below;
    }
    if (pickFrom->empty() && useInsteadOfCached) {
        return pickTrainerPokeReplacement(current, level, usePowerLevels, alreadyPlaced, noPremature, type, usePlacementHistory,
                                          swapMegaEvos, nullptr, evolveFar, bannedTypes, bannedPokemon);
    }
    auto without = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* p) { return !bannedPokemon.contains(p); }));
    if (!without->empty()) {
        pickFrom = without;
    } else if (useInsteadOfCached) {
        Species* cachePick = pickTrainerPokeReplacement(current, level, usePowerLevels, alreadyPlaced, noPremature, type,
                                                        usePlacementHistory, swapMegaEvos, nullptr, evolveFar, bannedTypes, bannedPokemon);
        if (without->contains(cachePick)) return cachePick;
    }
    // evitar duplicados: tiene que ser el último filtro
    if (!alreadyPlaced.empty()) {
        auto notPlaced = std::make_shared<SpeciesSet>(pickFrom->filter([&](Species* p) { return !alreadyPlaced.contains(p); }));
        if (!notPlaced->empty()) pickFrom = notPlaced;
        else alreadyPlaced.clear();
    }
    return usePowerLevels ? pickFrom->getRandomSimilarStrengthSpecies(current, random) : pickFrom->getRandomSpecies(random);
}

Type TrainerPokemonRandomizer::pickType(bool weightByFrequency, bool noLegendaries, bool allowAltFormes) {
    if (totalTypeWeighting_ == 0) initTypeWeightings(noLegendaries, allowAltFormes);
    if (weightByFrequency) {
        int pick = random.nextInt(totalTypeWeighting_);
        int pos = 0;
        for (auto [t, w] : typeWeightings_) {
            if (pos + w > pick) return t;
            pos += w;
        }
        throw std::logic_error("pickType: sin tipo");  // Java devolvería null
    }
    Type picked;
    do {
        picked = typeService.randomType(random);
    } while (typeWeightings_.at(picked) == 0);
    return picked;
}

void TrainerPokemonRandomizer::initTypeWeightings(bool noLegendaries, bool allowAltFormes) {
    auto byType = rSpecService.getSpecies(noLegendaries, allowAltFormes, true).sortByType(false);
    for (Type t : typeService.getTypes()) {
        int n = byType.at(t).size();  // NPE en Java si no hay ninguno
        typeWeightings_[t] = n;
        totalTypeWeighting_ += n;
    }
}

int TrainerPokemonRandomizer::getRandomAbilitySlot(const Species& s) {
    int per = romHandler.abilitiesPerSpecies();
    if (per == 0) return 0;
    const int abil[3] = {s.ability1, s.ability2, s.ability3};
    int slot = random.nextInt(per);
    while (abil[slot] == 0) slot = random.nextInt(per);
    return slot + 1;
}

int TrainerPokemonRandomizer::getValidAbilitySlotFromOriginal(const Species& s, int original) const {
    if (original == 2 && s.ability2 == 0) return 1;
    return original;
}

int TrainerPokemonRandomizer::getLevelOfStarter(const std::string& tag) const {
    for (const auto& t : romHandler.getTrainers()) {
        if (t->tag && *t->tag == tag) {
            int highest = t->pokemon.front()->level;
            int n = static_cast<int>(t->pokemon.size());
            for (int i = 1; i < n; i++) {
                int bonus = i == n - 1 ? 2 : 0;
                if (t->pokemon[i]->level + bonus > highest) highest = t->pokemon[i]->level;
            }
            return highest;
        }
    }
    return 0;
}

void TrainerPokemonRandomizer::changeStarterWithTag(const std::string& tag, const std::map<int, Species*>& byLevel, int abilitySlot) {
    for (const auto& t : romHandler.getTrainers()) {
        if (!t->tag || *t->tag != tag) continue;
        TrainerPokemon* best = t->pokemon.front().get();
        if (t->forceStarterPosition >= 0) {
            best = t->pokemon.at(t->forceStarterPosition).get();
        } else {
            // el de más nivel; el último tiene +2 de prioridad
            int n = static_cast<int>(t->pokemon.size());
            for (int i = 1; i < n; i++) {
                int bonus = i == n - 1 ? 2 : 0;
                if (t->pokemon[i]->level + bonus > best->level) best = t->pokemon[i].get();
            }
        }
        auto it = byLevel.upper_bound(best->level);  // floorEntry
        if (it == byLevel.begin()) throw std::logic_error("floorEntry nulo");
        --it;
        best->holder.setSpecies(it->second);
        randomizeCosmeticForme(*best);
        best->resetMoves = true;
        best->abilitySlot = abilitySlot;
    }
}

Species* TrainerPokemonRandomizer::fullyEvolve(Species* s) {
    SpeciesSet seen(s);
    while (!s->evolutionsFrom().empty()) {
        bool cyclic = false;
        for (const auto& e : s->evolutionsFrom())
            if (seen.contains(e->to())) cyclic = true;
        if (cyclic) break;
        int i = random.nextInt(static_cast<int>(s->evolutionsFrom().size()));
        s = s->evolutionsFrom()[i]->to();
        seen.add(s);
    }
    return s;
}

void TrainerPokemonRandomizer::randomizeCosmeticForme(TrainerPokemon& tp) {
    auto& sh = tp.holder;
    if (sh.isAltFormeAllowed() && sh.species()->isBaseForme())
        sh.setFormeNumber(sh.species()->baseForme()->randomCosmeticFormeNumber(random));
}

double TrainerPokemonRandomizer::getPlacementAverage() const {
    if (placementHistory_.empty()) return 0;
    long sum = 0;
    placementHistory_.forEach([&](Species* const&, const int& v) { sum += v; });
    return static_cast<double>(sum) / placementHistory_.size();
}

void TrainerPokemonRandomizer::makeRivalCarryStarter() {
    rivalCarriesStarterUpdate("RIVAL", 1);
    rivalCarriesStarterUpdate("FRIEND", 2);
    changesMade = true;
}

void TrainerPokemonRandomizer::rivalCarriesStarterUpdate(const std::string& prefix, int offset) {
    int highest = 0;
    for (const auto& t : romHandler.getTrainers()) {
        if (t->tag && jc::startsWith(*t->tag, prefix)) {
            const std::string& tag = *t->tag;
            highest = std::max(highest, std::stoi(tag.substr(prefix.size(), tag.find('-') - prefix.size())));
        }
    }
    if (highest == 0) return;
    auto starters = romHandler.getStarters();
    if (romHandler.isYellow()) throw std::logic_error("Amarillo no está soportado");
    for (int variant = 0; variant < 3; variant++) {
        Species* st = starters.at((variant + offset) % 3);
        auto byLevel = getEvolutionsByLevel(st, 1, 100);
        int ability = getRandomAbilitySlot(*st);
        while (ability == 3) ability = getRandomAbilitySlot(*st);  // los iniciales nunca tienen habilidad oculta
        for (int enc = 0; enc <= highest; enc++)
            changeStarterWithTag(prefix + std::to_string(enc) + "-" + std::to_string(variant), byLevel, ability);
    }
}

std::map<int, Species*> TrainerPokemonRandomizer::getEvolutionsByLevel(Species* base, int initialLevel, int maxLevel) {
    bool forceFull = settings.trainersEvolveTheirPokemon;
    int fullLevel = static_cast<int>(std::ceil((1 + settings.trainersEvolutionLevelModifier / 100.0) * romHandler.getHighestEvoLvl()));
    std::map<int, Species*> evos;
    evos[initialLevel] = base;
    int cur = initialLevel;
    Species* sp = base;
    if (forceFull && maxLevel < fullLevel) maxLevel = fullLevel;
    while (cur < maxLevel && !sp->evolutionsFrom().empty()) {
        const auto& possible = sp->evolutionsFrom();
        const auto& chosen = possible[random.nextInt(static_cast<int>(possible.size()))];
        int level = chosen->estimatedEvoLvl();
        if (level >= maxLevel) break;
        cur = level;
        sp = chosen->to();
        evos[cur] = sp;
    }
    if (forceFull) evos[fullLevel] = fullyEvolve(sp);
    return evos;
}

void TrainerPokemonRandomizer::evolveTrainerPokemonAsFarAsLegal() {
    double mod = 1 + settings.trainersEvolutionLevelModifier / 100.0;
    for (const auto& t : romHandler.getTrainers()) {
        for (auto& tp : t->pokemon) {
            Species* n = evolveAsFarAsLegal(tp->species(), tp->level, mod);
            if (n != tp->species()) {
                tp->holder.setSpecies(n);
                randomizeCosmeticForme(*tp);
                tp->abilitySlot = getValidAbilitySlotFromOriginal(*n, tp->abilitySlot);
            }
        }
    }
    changesMade = true;
}

Species* TrainerPokemonRandomizer::evolveAsFarAsLegal(Species* s, int level, double mod) {
    std::vector<EvolutionPtr> possible;
    for (const auto& e : s->evolutionsFrom())
        if (level >= mod * e->estimatedEvoLvl()) possible.push_back(e);
    if (possible.empty()) return s;
    return evolveAsFarAsLegal(possible[random.nextInt(static_cast<int>(possible.size()))]->to(), level, mod);
}

void TrainerPokemonRandomizer::addTrainerPokemon() {
    for (const auto& t : romHandler.getTrainers()) {
        int additional;
        if (t->isBoss()) additional = settings.additionalBossTrainerPokemon;
        else if (t->isImportant()) {
            if (t->shouldNotGetBuffs()) continue;
            additional = settings.additionalImportantTrainerPokemon;
        } else additional = settings.additionalRegularTrainerPokemon;
        if (additional == 0) continue;
        std::vector<TrainerPokemonPtr> original;
        int lowest = 100, highest = 0;
        bool dupHighest = false;
        for (auto& tp : t->pokemon) {
            int l = tp->level;
            if (l == highest) dupHighest = true;
            if (l < lowest) lowest = l;
            if (l > highest) {
                highest = l;
                dupHighest = false;
            }
            original.push_back(tp);
        }
        // en combates múltiples, como mucho 3 para no tener 6 contra 12
        int maxPokemon = t->multiBattleStatus != Trainer::MultiBattleStatus::NEVER ? 3 : 6;
        int originalSize = static_cast<int>(original.size());
        int upper = dupHighest ? highest : highest - 1;  // conservar al "as" del entrenador
        for (int i = 0; i < additional; i++) {
            if (static_cast<int>(t->pokemon.size()) >= maxPokemon) break;
            int secondToLast = static_cast<int>(t->pokemon.size()) - 1;
            auto np = std::make_shared<TrainerPokemon>(*original[random.nextInt(originalSize)]);
            np->level = random.nextInt(std::max(upper, lowest) - lowest + 1) + lowest;
            np->heldItem.reset();
            np->isAddedTeamMember = true;
            t->pokemon.insert(t->pokemon.begin() + secondToLast, np);
        }
        if (t->forceStarterPosition != -1 && t->forceStarterPosition == originalSize - 1)
            t->forceStarterPosition = std::min(t->forceStarterPosition + additional, maxPokemon - 1);
    }
    changesMade = true;
}

void TrainerPokemonRandomizer::modifyBattleStyle() {
    const auto& style = settings.settingBattleStyle;
    if (style.mod == BattleStyle::Modification::UNCHANGED) return;
    for (const auto& tr : romHandler.getTrainers()) {
        if (tr->multiBattleStatus == Trainer::MultiBattleStatus::ALWAYS || tr->shouldNotGetBuffs()) continue;
        auto ts = std::make_shared<BattleStyle>(style);
        if (ts->mod == BattleStyle::Modification::RANDOM) {
            int count = 4;
            int gen = romHandler.generationOfPokemon();
            if (gen < 5 || gen > 6) count = 2;  // sin triples ni rotatorios
            ts->style = static_cast<BattleStyle::Style>(random.nextInt(count));
        }
        tr->currBattleStyle = ts;
        while (static_cast<int>(tr->pokemon.size()) < ts->requiredPokemonCount())
            tr->pokemon.push_back(std::make_shared<TrainerPokemon>(*tr->pokemon.front()));
        tr->forcedDoubleBattle = true;
    }
    romHandler.makeDoubleBattleModePossible();
    changesMade = true;
}

void TrainerPokemonRandomizer::randomizeTrainerHeldItems() {
    bool boss = settings.randomizeHeldItemsForBossTrainerPokemon, imp = settings.randomizeHeldItemsForImportantTrainerPokemon,
         reg = settings.randomizeHeldItemsForRegularTrainerPokemon;
    auto movesets = romHandler.getMovesLearnt();
    for (const auto& t : romHandler.getTrainers()) {
        if (t->shouldNotGetBuffs()) continue;
        if (!reg && !t->isImportant() && !t->isBoss()) continue;
        if (!imp && t->isImportant()) continue;
        if (!boss && t->isBoss()) continue;
        auto movesFor = [&](const TrainerPokemon& tp) {
            return tp.resetMoves ? romHandler.getMovesAtLevel(*tp.species(), movesets, tp.level) : tp.moves;
        };
        if (settings.highestLevelOnlyGetsItemsForTrainerPokemon) {
            int maxLevel = -1;
            TrainerPokemon* hp = nullptr;
            for (auto& tp : t->pokemon) {
                if (tp->level > maxLevel) {
                    hp = tp.get();
                    maxLevel = tp->level;
                }
            }
            if (!hp) continue;
            randomizeHeldItem(*hp, movesFor(*hp));
        } else {
            for (auto& tp : t->pokemon) {
                auto ms = movesFor(*tp);
                randomizeHeldItem(*tp, ms);
                if (t->requiresUniqueHeldItems)
                    while (!t->pokemonHaveUniqueHeldItems()) randomizeHeldItem(*tp, ms);
            }
        }
    }
    changesMade = true;
}

void TrainerPokemonRandomizer::randomizeHeldItem(TrainerPokemon& tp, const std::array<int, 4>& moveset) {
    if (tp.hasZCrystal) return;
    if (tp.hasMegaStone && settings.swapTrainerMegaEvos) return;
    std::vector<std::optional<int>> choose;
    if (settings.sensibleItemsOnlyForTrainerPokemon) {
        choose = romHandler.getSensibleHeldItemsFor(tp, settings.consumableItemsOnlyForTrainerPokemon, moveset);
    } else {
        auto set = settings.consumableItemsOnlyForTrainerPokemon ? romHandler.getAllConsumableHeldItems() : romHandler.getAllHeldItems();
        for (int id : set.toVector()) choose.push_back(id);
    }
    tp.heldItem = choose[random.nextInt(static_cast<int>(choose.size()))];
}

void TrainerPokemonRandomizer::randomUsableZCrystals() {
    // Los cristales Z solo existen en Gen 7: en Gen 3-5 ningún objeto equipado lo es.
    static const std::vector<int> z = {items::normaliumZHeld, items::fightiniumZHeld, items::flyiniumZHeld, items::poisoniumZHeld,
                                       items::groundiumZHeld, items::rockiumZHeld, items::buginiumZHeld, items::ghostiumZHeld,
                                       items::steeliumZHeld, items::firiumZHeld, items::wateriumZHeld, items::grassiumZHeld,
                                       items::electriumZHeld, items::psychiumZHeld, items::iciumZHeld, items::dragoniumZHeld,
                                       items::darkiniumZHeld, items::fairiumZHeld};
    for (const auto& tr : romHandler.getTrainers())
        for (auto& tp : tr->pokemon)
            if (tp->heldItem && std::find(z.begin(), z.end(), *tp->heldItem) != z.end())
                throw std::logic_error("cristal Z en un juego sin cristales Z");
}

}  // namespace pr
