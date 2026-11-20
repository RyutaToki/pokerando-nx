#include "random/species_base_stat_randomizer.hpp"

#include <algorithm>
#include <cmath>

#include "constants/species_ids.hpp"

namespace pr {

namespace {
constexpr float WISHIWASHI_SCHOOL_MULTIPLIER = 1.8f;
constexpr int MEGA_BST_BOOST = 100, SUNKERN_BST = 180, ARCEUS_BST = 720, MIN_HP = 20, MIN_NON_HP_STAT = 10;
double javaRoundD(double x) { return std::floor(x + 0.5); }  // (double) Math.round(x)

void copyBST(Species* base, Species* alt) { alt->baseStats().setBST(base->bst(false)); }
void giveMegaBoost(Species* base, Species* alt) {
    if (alt->isMegaEvolution()) alt->baseStats().setBST(base->bst(false) + MEGA_BST_BOOST);
}
}  // namespace

void SpeciesBaseStatRandomizer::randomizeBSTs() {
    switch (settings.bstMod) {
        case Settings::BSTMod::RANDOM_BUFF_NERF: randomlyModifyBSTsByPercentage(); break;
        case Settings::BSTMod::SHUFFLE: shuffleBSTs(); break;
        case Settings::BSTMod::RANDOM: fullyRandomizeBSTs(); break;
        default: break;
    }
    changesMade = true;
}

void SpeciesBaseStatRandomizer::randomlyModifyBSTsByPercentage() {
    double maxMod = static_cast<double>(settings.bstBuffNerfMaxPercentage) / 100;
    auto mods = std::make_shared<SpeciesMap>();
    auto copyMod = [this, mods](Species* from, Species* to) {
        double m = *mods->get(from);
        mods->put(to, m);
        applyBSTModifier(to, m);
    };
    CopyUpEvolutionsHelper::Options o(
        [this, mods, maxMod](Species* pk) {
            double m = random.nextDouble(1.0 - maxMod, 1.0 + maxMod);
            mods->put(pk, m);
            applyBSTModifier(pk, m);
        },
        [copyMod](Species* from, Species* to, bool) { copyMod(from, to); });
    o.withEvolutionSanity(settings.bstFollowEvolutions)
        .withCopySplitEvos(true)
        .withAltFormeAction([copyMod](Species* b, Species* a) {
            copyMod(b, a);
            giveMegaBoost(b, a);
        })
        .withCosmeticAction([mods](Species* b, Species* a) {
            copyBST(b, a);
            mods->put(a, *mods->get(b));
        });
    copyUpEvolutionsHelper.apply(o);
}

void SpeciesBaseStatRandomizer::applyBSTModifier(Species* pk, double m) {
    int newBST = static_cast<int>(pk->bst(false) * m);
    newBST = std::min(newBST, pk->baseStats().getMaxBST());
    pk->baseStats().setBST(newBST);
}

void SpeciesBaseStatRandomizer::shuffleBSTs() {
    bool evoSanity = settings.bstFollowEvolutions;
    std::vector<SpeciesSet> groups{SpeciesSet::copyOf(romHandler.getSpeciesSet())};
    if (evoSanity) {
        std::vector<SpeciesSet> ng;
        for (const auto& g : groups) {
            JavaHashMap<int, SpeciesSet> byLen;
            for (Species* pk : g.filterBasic(true).toVector()) {
                int len = pk->stagesAfter(true);
                byLen.putIfAbsent(len, SpeciesSet());
                byLen.get(len)->addAll(pk->family(true).filter([&](Species* s) { return g.contains(s); }));
            }
            byLen.forEach([&](const int&, const SpeciesSet& s) { ng.push_back(s); });
        }
        groups = std::move(ng);
    }
    if (settings.bstShuffleSwapLegendaries) {
        std::vector<SpeciesSet> ng;
        for (const auto& g : groups) {
            ng.push_back(g.filter([](Species* s) { return s->isLegendary(); }));
            ng.push_back(g.filter([](Species* s) { return !s->isLegendary(); }));
        }
        groups = std::move(ng);
    }
    for (const auto& g : groups) shuffleBSTsOfGroup(g, evoSanity);
    copyShuffledBSTsToAltFormes();
}

void SpeciesBaseStatRandomizer::shuffleBSTsOfGroup(const SpeciesSet& group, bool evoSanity) {
    SpeciesSet basics = evoSanity ? group.filterBasic(true) : group;
    auto donators = std::make_shared<JavaHashMap<Species*, Species*, JavaHashCode<Species*>, SpeciesPtrEq>>(basics.size());
    auto keys = basics.toVector();
    auto values = keys;
    random.shuffle(values);
    for (size_t i = 0; i < keys.size(); i++) donators->put(keys[i], values[i]);
    CopyUpEvolutionsHelper cueh([group] { return group; });
    CopyUpEvolutionsHelper::Options o(
        [donators](Species* pk) { pk->baseStats().setBST((*donators->get(pk))->bst(true)); },
        [donators](Species* from, Species* to, bool) {
            Species* fromDon = *donators->get(from);
            Species* toDon = fromDon->evolutionsFrom().front()->to();
            donators->put(to, toDon);
            to->baseStats().setBST(toDon->bst(true));
        });
    o.withEvolutionSanity(evoSanity).withCopySplitEvos(true);
    cueh.apply(o);
}

void SpeciesBaseStatRandomizer::copyShuffledBSTsToAltFormes() {
    auto carry = [](Species* base, Species* alt) {
        float mult = static_cast<float>(alt->bst(true)) / base->bst(true);
        if (base->number() == species::wishiwashi) mult = WISHIWASHI_SCHOOL_MULTIPLIER;
        int newBST = static_cast<int>(base->bst(false) * mult);
        newBST = std::min(newBST, alt->baseStats().getMaxBST());
        alt->baseStats().setBST(newBST);
        giveMegaBoost(base, alt);
    };
    CopyUpEvolutionsHelper::Options o(nullptr, [carry](Species*, Species* to, bool) {
        if (!to->isBaseForme()) carry(to->conceptualBaseForme(), to);
    });
    o.withCopySplitEvos(true).withAltFormeAction(carry).withCosmeticAction(copyBST);
    copyUpEvolutionsHelper.apply(o);
}

void SpeciesBaseStatRandomizer::fullyRandomizeBSTs() {
    for (Species* pk : romHandler.getSpeciesSetInclFormes().toVector()) {
        if (pk->isEssentiallyCosmetic()) continue;
        pk->baseStats().setBST(random.nextInt(SUNKERN_BST, ARCEUS_BST));
    }
    CopyUpEvolutionsHelper::Options o(nullptr, nullptr);
    o.withAltFormeAction(giveMegaBoost).withCosmeticAction(copyBST);
    copyUpEvolutionsHelper.apply(o);
}

void SpeciesBaseStatRandomizer::shuffleSpeciesStats() {
    shuffledStatsOrders_ = {};
    auto copyUp = [this](Species* from, Species* to) { shuffledStatsOrders_.put(to, *shuffledStatsOrders_.get(from)); };
    CopyUpEvolutionsHelper::Options o(
        [this](Species* pk) {
            auto order = std::make_shared<std::vector<int>>(std::vector<int>{0, 1, 2, 3, 4, 5});
            random.shuffle(*order);
            shuffledStatsOrders_.put(pk, order);
        },
        [copyUp](Species* f, Species* t, bool) { copyUp(f, t); });
    o.withAltFormeAction(copyUp)
        .withCosmeticAction(copyUp)
        .withEvolutionSanity(settings.baseStatsFollowEvolutions)
        .withCopySplitEvos(false)
        .withTreatMegasAsEvos(settings.baseStatsFollowMegaEvolutions);
    copyUpEvolutionsHelper.apply(o);
    for (Species* pk : romHandler.getSpeciesSetInclFormes().toVector()) applyShuffledOrderToStats(pk);
    changesMade = true;
}

void SpeciesBaseStatRandomizer::applyShuffledOrderToStats(Species* pk) {
    auto p = shuffledStatsOrders_.get(pk);
    if (!p) return;
    auto& order = **p;  // lista compartida: en Java Shedinja la modifica para todos los que la comparten
    BaseStats& bs = pk->baseStats();
    std::vector<int> stats{bs.getHp(), bs.attack, bs.defense, bs.spatk, bs.spdef, bs.speed};
    if (bs.shedinja) {
        for (size_t i = 0; i < order.size(); i++)
            if (order[i] == 0) order[i] = order[0];
        bs.setStatRatiosShedinja(stats[order[1]], stats[order[2]], stats[order[3]], stats[order[4]], stats[order[5]]);
    } else {
        bs.setStatRatios(stats[order[0]], stats[order[1]], stats[order[2]], stats[order[3]], stats[order[4]], stats[order[5]]);
    }
}

void SpeciesBaseStatRandomizer::randomizeSpeciesStats() {
    auto randomEs = [this](Species* f, Species* t, bool) { assignNewStatsForEvolution(f, t); };
    auto copyEs = [this](Species* f, Species* t, bool) { copyRandomizedStatsUpEvolution(f, t); };
    CopyUpEvolutionsHelper::Options o([this](Species* pk) { randomizeStatsWithinBST(pk); },
                                      settings.assignEvoStatsRandomly ? CopyUpEvolutionsHelper::EvolvedAction(randomEs)
                                                                      : CopyUpEvolutionsHelper::EvolvedAction(copyEs));
    o.withEvolutionSanity(settings.baseStatsFollowEvolutions)
        .withCopySplitEvos(true)
        .withTreatMegasAsEvos(settings.baseStatsFollowMegaEvolutions)
        .withSplitAction(randomEs)
        .withCosmeticAction([this](Species* f, Species* t) { copyRandomizedStatsUpEvolution(f, t); });
    copyUpEvolutionsHelper.apply(o);
    changesMade = true;
}

void SpeciesBaseStatRandomizer::randomizeStatsWithinBST(Species* pk) {
    BaseStats& bs = pk->baseStats();
    if (bs.shedinja) {
        int bst = bs.bst() - MIN_NON_HP_STAT * 5;
        double a = random.nextDouble(), d = random.nextDouble(), sa = random.nextDouble(), sd = random.nextDouble(),
               s = random.nextDouble();
        double tot = a + d + sa + sd + s;
        bs.setStatRatiosShedinja(a / tot * bst + MIN_NON_HP_STAT, d / tot * bst + MIN_NON_HP_STAT, sa / tot * bst + MIN_NON_HP_STAT,
                                 sd / tot * bst + MIN_NON_HP_STAT, s / tot * bst + MIN_NON_HP_STAT);
    } else {
        int bst = bs.bst() - (MIN_HP + MIN_NON_HP_STAT * 5);
        double h = random.nextDouble(), a = random.nextDouble(), d = random.nextDouble(), sa = random.nextDouble(),
               sd = random.nextDouble(), s = random.nextDouble();
        double tot = h + a + d + sa + sd + s;
        bs.setStatRatios(h / tot * bst + MIN_HP, a / tot * bst + MIN_NON_HP_STAT, d / tot * bst + MIN_NON_HP_STAT,
                         sa / tot * bst + MIN_NON_HP_STAT, sd / tot * bst + MIN_NON_HP_STAT, s / tot * bst + MIN_NON_HP_STAT);
    }
}

void SpeciesBaseStatRandomizer::assignNewStatsForEvolution(Species* from, Species* to) {
    const BaseStats& f = from->baseStats();
    BaseStats& t = to->baseStats();
    double diff = t.bst() - f.bst();
    if (t.shedinja) {
        double a = random.nextDouble(), d = random.nextDouble(), sa = random.nextDouble(), sd = random.nextDouble(),
               s = random.nextDouble();
        double tot = a + d + sa + sd + s;
        t.setStatRatiosShedinja(f.attack + javaRoundD(a / tot * diff), f.defense + javaRoundD(d / tot * diff),
                                f.spatk + javaRoundD(sa / tot * diff), f.spdef + javaRoundD(sd / tot * diff),
                                f.speed + javaRoundD(s / tot * diff));
    } else {
        double h = random.nextDouble(), a = random.nextDouble(), d = random.nextDouble(), sa = random.nextDouble(),
               sd = random.nextDouble(), s = random.nextDouble();
        double tot = h + a + d + sa + sd + s;
        t.setStatRatios(std::max<double>(MIN_HP, f.getHp() + javaRoundD(h / tot * diff)),
                        std::max<double>(MIN_NON_HP_STAT, f.attack + javaRoundD(a / tot * diff)),
                        std::max<double>(MIN_NON_HP_STAT, f.defense + javaRoundD(d / tot * diff)),
                        std::max<double>(MIN_NON_HP_STAT, f.spatk + javaRoundD(sa / tot * diff)),
                        std::max<double>(MIN_NON_HP_STAT, f.spdef + javaRoundD(sd / tot * diff)),
                        std::max<double>(MIN_NON_HP_STAT, f.speed + javaRoundD(s / tot * diff)));
    }
}

void SpeciesBaseStatRandomizer::copyRandomizedStatsUpEvolution(Species* from, Species* to) {
    const BaseStats& f = from->baseStats();
    to->baseStats().setStatRatios(f.getHp(), f.attack, f.defense, f.spatk, f.spdef, f.speed);
}

void SpeciesBaseStatRandomizer::standardizeEXPCurves() {
    auto mod = settings.expCurveMod;
    auto curve = settings.selectedEXPCurve;
    for (Species* pk : romHandler.getSpeciesSetInclFormes().toVector()) {
        switch (mod) {
            case Settings::ExpCurveMod::LEGENDARIES: pk->growthCurve = pk->isLegendary() ? ExpCurve::SLOW : *curve; break;
            case Settings::ExpCurveMod::STRONG_LEGENDARIES: pk->growthCurve = pk->isStrongLegendary() ? ExpCurve::SLOW : *curve; break;
            case Settings::ExpCurveMod::ALL: pk->growthCurve = curve; break;
        }
    }
}

}  // namespace pr
