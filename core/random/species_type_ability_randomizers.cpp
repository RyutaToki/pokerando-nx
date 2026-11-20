#include "random/species_type_ability_randomizers.hpp"

#include <algorithm>

#include "constants/ability_ids.hpp"
#include "constants/global_constants.hpp"

namespace pr {

namespace {
constexpr double GSTC_NO_EVO = 0.5, GSTC_HAS_EVO = 0.35, GSTC_MIDDLE_EVO = 0.15, GSTC_FINAL_EVO = 0.25, GSTC_MEGA_EVO = 0.25,
                 GSTC_ALT_FORME = 0.5;
}

void SpeciesTypeRandomizer::randomizeSpeciesTypes() {
    bool evoSanity = settings.speciesTypesMod == Settings::SpeciesTypesMod::RANDOM_FOLLOW_EVOLUTIONS;
    bool dual = settings.dualTypeOnly;
    CopyUpEvolutionsHelper::Options o(
        [this, dual](Species* pk) {
            pk->setPrimaryType(typeService.randomType(random));
            pk->setSecondaryType(std::nullopt);
            assignRandomSecondaryType(pk, pk->evolutionsFrom().size() == 1 ? GSTC_HAS_EVO : GSTC_NO_EVO, dual);
        },
        [this](Species* from, Species* to, bool toFinal) {
            to->setPrimaryType(from->primaryType());
            to->setSecondaryType(from->secondaryType());
            if (!to->secondaryType()) {
                double chance = toFinal ? GSTC_FINAL_EVO : GSTC_MIDDLE_EVO;
                if (to->isMegaEvolution()) chance = GSTC_MEGA_EVO;
                assignRandomSecondaryType(to, chance, false);
            }
        });
    o.withNoEvoAction([this, dual](Species* pk) {
         pk->setPrimaryType(typeService.randomType(random));
         pk->setSecondaryType(std::nullopt);
         assignRandomSecondaryType(pk, GSTC_NO_EVO, dual);
     })
        .withAltFormeAction([this](Species* b, Species* a) {
            a->setPrimaryType(b->primaryType());
            a->setSecondaryType(b->secondaryType());
            assignRandomSecondaryType(a, GSTC_ALT_FORME, false);
        })
        .withCosmeticAction([](Species* b, Species* a) {
            a->setPrimaryType(b->primaryType());
            a->setSecondaryType(b->secondaryType());
        })
        .withEvolutionSanity(evoSanity)
        .withTreatMegasAsEvos(settings.typesFollowMegaEvolutions);
    copyUpEvolutionsHelper.apply(o);
    changesMade = true;
}

void SpeciesTypeRandomizer::assignRandomSecondaryType(Species* sp, double chance, bool dual) {
    if (random.nextDouble() < chance || dual) {
        do {
            sp->setSecondaryType(typeService.randomType(random));
        } while (sp->secondaryType() == sp->primaryType());
    }
}

void SpeciesAbilityRandomizer::randomizeAbilities() {
    if (romHandler.abilitiesPerSpecies() == 0) return;
    bool hasHidden = romHandler.abilitiesPerSpecies() == 3;
    bool weigh = settings.weighDuplicateAbilitiesTogether;
    bool ensureTwo = settings.ensureTwoAbilities;
    std::vector<int> banned = romHandler.getUselessAbilities();
    auto addAll = [&](const std::vector<int>& v) { banned.insert(banned.end(), v.begin(), v.end()); };
    if (!settings.allowWonderGuard) banned.push_back(abilities::wonderGuard);
    if (settings.banTrappingAbilities) addAll(global::battleTrappingAbilities);
    if (settings.banNegativeAbilities) addAll(global::negativeAbilities);
    if (settings.banBadAbilities) {
        addAll(global::badAbilities);
        if (!settings.settingBattleStyle.isOnlyMultiBattles()) addAll(global::doubleBattleAbilities);
    }
    if (weigh) {
        addAll(global::duplicateAbilities);
        if (romHandler.generationOfPokemon() == 3) banned.push_back(77);  // Gen3Constants.airLockIndex
    }
    int maxAbility = romHandler.highestAbilityIndex();
    auto basic = [=, this](Species* pk) {
        if (pk->ability1 == abilities::wonderGuard || pk->ability2 == abilities::wonderGuard || pk->ability3 == abilities::wonderGuard)
            return;
        pk->ability1 = pickRandomAbility(maxAbility, banned, weigh, {});
        if (ensureTwo || random.nextDouble() < 0.5) pk->ability2 = pickRandomAbility(maxAbility, banned, weigh, {pk->ability1});
        else pk->ability2 = 0;
        if (hasHidden) pk->ability3 = pickRandomAbility(maxAbility, banned, weigh, {pk->ability1, pk->ability2});
    };
    CopyUpEvolutionsHelper::Options o(basic, [](Species* from, Species* to, bool) {
        if (to->ability1 != abilities::wonderGuard && to->ability2 != abilities::wonderGuard && to->ability3 != abilities::wonderGuard)
            to->copyBaseFormeAbilities(*from);
    });
    o.withCosmeticAction([](Species* b, Species* a) { a->copyBaseFormeAbilities(*b); })
        .withAltFormeBasicAction(basic)
        .withEvolutionSanity(settings.abilitiesFollowEvolutions)
        .withTreatMegasAsEvos(settings.abilitiesFollowMegaEvolutions);
    copyUpEvolutionsHelper.apply(o);
    changesMade = true;
}

int SpeciesAbilityRandomizer::pickRandomAbilityVariation(int selected, const std::vector<int>& already) {
    int n = selected;
    while (true) {
        for (const auto& [base, vars] : romHandler.getAbilityVariations()) {
            if (selected == base) {
                n = vars[random.nextInt(static_cast<int>(vars.size()))];
                break;
            }
        }
        if (std::find(already.begin(), already.end(), n) == already.end()) break;
    }
    return n;
}

int SpeciesAbilityRandomizer::pickRandomAbility(int maxAbility, const std::vector<int>& banned, bool useVariations,
                                                const std::vector<int>& already) {
    while (true) {
        int n = random.nextInt(maxAbility) + 1;
        if (std::find(banned.begin(), banned.end(), n) != banned.end()) continue;
        if (std::find(already.begin(), already.end(), n) != already.end()) continue;
        if (useVariations) n = pickRandomAbilityVariation(n, already);
        return n;
    }
}

}  // namespace pr
