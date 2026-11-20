#include "random/misc_randomizers.hpp"

#include <algorithm>

namespace pr {

void EncounterHeldItemRandomizer::randomizeWildHeldItems() {
    std::vector<int> possible = (settings.banBadRandomWildPokemonHeldItems ? romHandler.getNonBadItems() : romHandler.getAllowedItems()).toVector();
    auto pick = [&] { return std::optional<int>(possible[random.nextInt(static_cast<int>(possible.size()))]); };
    for (Species* pk : romHandler.getSpeciesSetInclFormes().toVector()) {
        if (!pk->guaranteedHeldItem && !pk->commonHeldItem && !pk->rareHeldItem && !pk->darkGrassHeldItem) continue;
        auto setGuaranteed = [&] {
            pk->guaranteedHeldItem = pick();
            pk->commonHeldItem.reset();
            pk->rareHeldItem.reset();
            pk->darkGrassHeldItem.reset();
        };
        auto setCommonAndRare = [&] {
            pk->guaranteedHeldItem.reset();
            pk->commonHeldItem = pick();
            pk->rareHeldItem = pick();
            while (pk->rareHeldItem == pk->commonHeldItem) pk->rareHeldItem = pick();
        };
        double d = random.nextDouble();
        if (pk->guaranteedHeldItem) {
            if (d < 0.9) setGuaranteed();
            else setCommonAndRare();
        } else {
            if (d < 0.5) {
                pk->guaranteedHeldItem.reset();
                pk->commonHeldItem.reset();
                pk->rareHeldItem.reset();
                pk->darkGrassHeldItem.reset();
            } else if (d < 0.65) {
                pk->guaranteedHeldItem.reset();
                pk->commonHeldItem.reset();
                pk->rareHeldItem = pick();
            } else if (d < 0.8) {
                pk->guaranteedHeldItem.reset();
                pk->commonHeldItem = pick();
                pk->rareHeldItem.reset();
            } else if (d < 0.95 || !romHandler.hasGuaranteedWildHeldItems()) {
                setCommonAndRare();
            } else {
                setGuaranteed();
            }
        }
        if (romHandler.hasDarkGrassHeldItems() && !pk->guaranteedHeldItem) {
            if (random.nextDouble() < 0.5) pk->darkGrassHeldItem = pick();
            else pk->darkGrassHeldItem.reset();
        }
    }
    changesMade = true;
}

void IntroPokemonRandomizer::randomizeIntroPokemon() {
    auto& all = const_cast<SpeciesSet&>(rSpecService.getAll(true));
    Species* pk = all.getRandomSpecies(random);
    while (!romHandler.setIntroPokemon(pk)) pk = all.getRandomSpecies(random);
    introSpecies = pk;
    changesMade = true;
}

void MiscTweakRandomizer::applyMiscTweaks() {
    int selected = settings.currentMiscTweaks;
    int avail = romHandler.miscTweaksAvailable();
    std::vector<MiscTweak> toApply;
    for (const auto& mt : misctweak::allTweaks)
        if ((avail & mt.value) > 0 && (selected & mt.value) > 0) toApply.push_back(mt);
    // Collections.sort: mayor prioridad primero (estable)
    std::stable_sort(toApply.begin(), toApply.end(), [](const MiscTweak& a, const MiscTweak& b) { return a.priority > b.priority; });
    for (const auto& mt : toApply) {
        if (mt == misctweak::RANDOMIZE_CATCHING_TUTORIAL) randomizeCatchingTutorial();
        else if (mt == misctweak::RANDOMIZE_PC_POTION) randomizePCPotion();
        else romHandler.applyMiscTweak(mt);
    }
    changesMade = true;
}

void MiscTweakRandomizer::randomizeCatchingTutorial() {
    bool ok = false;
    int tries = 0;
    while (!ok && tries < 1000) {
        Species* a = rSpecService.randomSpecies(random);
        Species* b = rSpecService.randomSpecies(random);
        ok = romHandler.setCatchingTutorial(a, b);
        tries++;
    }
    if (tries == 1000) throw RandomizationException("Could not randomize catching tutorial in 1000 tries.");
}

void MiscTweakRandomizer::randomizePCPotion() {
    auto possible = romHandler.getNonBadItems().toVector();
    int item;
    do {
        item = possible[random.nextInt(static_cast<int>(possible.size()))];
    } while (romHandler.getItems()[item]->tm);
    romHandler.setPCPotionItem(item);
}

}  // namespace pr
