#include "random/static_pokemon_randomizer.hpp"

#include <algorithm>

namespace pr {

namespace {
StaticEncounter cloneStatic(const StaticEncounter& old) {
    StaticEncounter s = old.deepCopy();
    s.resetMoves = true;
    for (auto& l : s.linkedEncounters) l.resetMoves = true;
    return s;
}
bool inList(const std::vector<Species*>& l, Species* s) {
    return std::any_of(l.begin(), l.end(), [&](Species* x) { return x->number() == s->number(); });
}
}  // namespace

void StaticPokemonRandomizer::onlyChangeStaticLevels() {
    int mod = settings.staticLevelModifier;
    auto cur = romHandler.getStaticPokemon();
    for (auto& se : cur) {
        if (se.isEgg) continue;
        se.level = applyPercentageLevelModifier(se.level, mod);
        for (auto& l : se.linkedEncounters)
            if (!l.isEgg) l.level = applyPercentageLevelModifier(l.level, mod);
    }
    romHandler.setStaticPokemon(cur);
}

Species* StaticPokemonRandomizer::getRestricted(const SpeciesSet& full, SpeciesSet& left, const StaticEncounter& old) {
    SpeciesSet pool = left.filter([&](Species* p) { return inList(old.restrictedList, p); });
    if (pool.empty()) pool = full.filter([&](Species* p) { return inList(old.restrictedList, p); });
    Species* n = pool.getRandomSpecies(random);
    left.remove(n);
    return n;
}

Species* StaticPokemonRandomizer::pickPowerLvlReplacement(SpeciesSet& pool, Species* current, bool banSame, bool limitBST) {
    if (limitBST) {
        int maxBST = current->bst(false);
        SpeciesSet f = pool.filter([&](Species* p) { return p->bst(true) <= maxBST; });
        return f.getRandomSimilarStrengthSpecies(current, banSame, -1, random);
    }
    return pool.getRandomSimilarStrengthSpecies(current, banSame, -1, random);
}

void StaticPokemonRandomizer::randomizeCosmeticForme(StaticEncounter& se) {
    auto& sh = se.holder;
    if (sh.isAltFormeAllowed() && sh.species()->isBaseForme())
        sh.setFormeNumber(sh.species()->baseForme()->randomCosmeticFormeNumber(random));
}

void StaticPokemonRandomizer::randomizeStaticPokemon() {
    using SPM = Settings::StaticPokemonMod;
    bool swapLegendaries = settings.staticPokemonMod == SPM::RANDOM_MATCHING;
    bool similar = settings.staticPokemonMod == SPM::SIMILAR_STRENGTH;
    bool allowAlt = settings.allowStaticAltFormes;
    int levelMod = settings.staticLevelModified ? settings.staticLevelModifier : 0;
    auto current = romHandler.getStaticPokemon();
    std::vector<StaticEncounter> replacements;
    SpeciesSet banned = SpeciesSet::copyOf(romHandler.getBannedForStaticPokemon());
    banned.addAll(rSpecService.getBannedFormesForPlayerPokemon());
    if (settings.abilitiesMod != Settings::AbilitiesMod::RANDOMIZE) banned.addAll(rSpecService.getAbilityDependentFormes());
    if (settings.banIrregularAltFormes) banned.addAll(romHandler.getIrregularFormes());
    // (en Gen 3-5 no hay megaevoluciones, así que no hace falta "swapMegaEvos")
    JavaHashMap<int, int> musicChanges;
    std::vector<int> changeMusic;
    if (settings.correctStaticMusic) changeMusic = romHandler.getSpecialMusicStatics();
    auto noteMusic = [&](const StaticEncounter& old, Species* n) {
        if (std::find(changeMusic.begin(), changeMusic.end(), old.species()->number()) != changeMusic.end())
            musicChanges.put(old.species()->number(), n->number());
    };
    auto notCosmetic = [](Species* p) { return !p->isEssentiallyCosmetic(); };

    if (swapLegendaries) {
        SpeciesSet legLeft = SpeciesSet::copyOf(rSpecService.getLegendaries(allowAlt));
        if (allowAlt) legLeft = legLeft.filter(notCosmetic);
        SpeciesSet nonLeft = SpeciesSet::copyOf(rSpecService.getNonLegendaries(allowAlt));
        if (allowAlt) nonLeft = nonLeft.filter(notCosmetic);
        SpeciesSet ubLeft = SpeciesSet::copyOf(rSpecService.getUltrabeasts(false));
        legLeft.removeAll(ubLeft);
        nonLeft.removeAll(ubLeft);
        legLeft.removeAll(banned);
        nonLeft.removeAll(banned);
        ubLeft.removeAll(banned);
        SpeciesSet legPool = SpeciesSet::copyOf(legLeft), nonPool = SpeciesSet::copyOf(nonLeft), ubPool = SpeciesSet::copyOf(ubLeft);
        for (const auto& old : current) {
            StaticEncounter ns = cloneStatic(old);
            Species* n;
            auto pickFrom = [&](SpeciesSet& left, SpeciesSet& pool) {
                if (old.restrictedPool) {
                    n = getRestricted(pool, left, old);
                } else {
                    n = left.getRandomSpecies(random);
                    left.remove(n);
                }
                ns.holder.setSpecies(n);
                randomizeCosmeticForme(ns);
                if (left.empty()) left.addAll(pool);
            };
            if (old.species()->isLegendary()) pickFrom(legLeft, legPool);
            else if (rSpecService.getUltrabeasts(false).contains(old.species())) pickFrom(ubLeft, ubPool);
            else pickFrom(nonLeft, nonPool);
            replacements.push_back(ns);
            noteMusic(old, n);
        }
    } else if (similar) {
        SpeciesSet inclFormes = rSpecService.getAll(true).filter(notCosmetic);
        SpeciesSet left = SpeciesSet::copyOf(!allowAlt ? rSpecService.getAll(false) : inclFormes);
        left.removeAll(banned);
        SpeciesSet pool = SpeciesSet::copyOf(left);
        auto mainLeg = romHandler.getMainGameLegendaries();
        for (const auto& old : current) {
            StaticEncounter ns = cloneStatic(old);
            Species* n;
            Species* oldPK = old.species();
            if (oldPK->bst(true) >= 600 && settings.limit600) {
                if (old.restrictedPool) {
                    n = getRestricted(pool, left, old);
                } else {
                    n = left.getRandomSpecies(random);
                    left.remove(n);
                }
            } else {
                bool limitBST = settings.limitMainGameLegendaries &&
                                std::find(mainLeg.begin(), mainLeg.end(), oldPK->baseForme()->number()) != mainLeg.end();
                if (old.restrictedPool) {
                    SpeciesSet rp = left.filter([&](Species* p) { return inList(old.restrictedList, p); });
                    if (rp.empty()) rp = pool.filter([&](Species* p) { return inList(old.restrictedList, p); });
                    n = pickPowerLvlReplacement(rp, oldPK, false, limitBST);
                } else {
                    n = pickPowerLvlReplacement(left, oldPK, true, limitBST);
                }
                left.remove(n);
            }
            ns.holder.setSpecies(n);
            randomizeCosmeticForme(ns);
            if (left.empty()) left.addAll(pool);
            replacements.push_back(ns);
            noteMusic(old, n);
        }
    } else {
        SpeciesSet inclFormes = rSpecService.getAll(true).filter(notCosmetic);
        SpeciesSet left = SpeciesSet::copyOf(!allowAlt ? rSpecService.getAll(false) : inclFormes);
        left.removeAll(banned);
        SpeciesSet pool = SpeciesSet::copyOf(left);
        for (const auto& old : current) {
            StaticEncounter ns = cloneStatic(old);
            Species* n;
            if (old.restrictedPool) {
                n = getRestricted(pool, left, old);
            } else {
                n = left.getRandomSpecies(random);
                left.remove(n);
            }
            ns.holder.setSpecies(n);
            randomizeCosmeticForme(ns);
            left.remove(n);
            if (left.empty()) left.addAll(pool);
            replacements.push_back(ns);
            noteMusic(old, n);
        }
    }
    if (levelMod != 0) {
        for (auto& se : replacements) {
            if (se.isEgg) continue;
            se.level = applyPercentageLevelModifier(se.level, levelMod);
            se.maxLevel = applyPercentageLevelModifier(se.maxLevel, levelMod);
            for (auto& l : se.linkedEncounters) {
                if (l.isEgg) continue;
                l.level = applyPercentageLevelModifier(l.level, levelMod);
                l.maxLevel = applyPercentageLevelModifier(l.maxLevel, levelMod);
            }
        }
    }
    if (!musicChanges.empty()) {
        std::vector<std::pair<int, int>> v;
        musicChanges.forEach([&](const int& k, const int& val) { v.emplace_back(k, val); });
        romHandler.applyCorrectStaticMusic(v);
    }
    romHandler.setStaticPokemon(replacements);
    changesMade = true;
}

}  // namespace pr
