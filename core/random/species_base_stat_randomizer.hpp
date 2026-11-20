// Port de SpeciesBaseStatRandomizer (BST, stats base y curvas de experiencia).
#pragma once
#include <memory>

#include "random/randomizer.hpp"

namespace pr {

class SpeciesBaseStatRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeBSTs();
    void shuffleSpeciesStats();
    void randomizeSpeciesStats();
    void standardizeEXPCurves();

private:
    using SpeciesMap = JavaHashMap<Species*, double, JavaHashCode<Species*>, SpeciesPtrEq>;
    void randomlyModifyBSTsByPercentage();
    void applyBSTModifier(Species* pk, double modifier);
    void shuffleBSTs();
    void shuffleBSTsOfGroup(const SpeciesSet& group, bool evolutionSanity);
    void copyShuffledBSTsToAltFormes();
    void fullyRandomizeBSTs();
    void randomizeStatsWithinBST(Species* pk);
    void assignNewStatsForEvolution(Species* from, Species* to);
    void copyRandomizedStatsUpEvolution(Species* from, Species* to);
    void applyShuffledOrderToStats(Species* pk);

    JavaHashMap<Species*, std::shared_ptr<std::vector<int>>, JavaHashCode<Species*>, SpeciesPtrEq> shuffledStatsOrders_;
};

}  // namespace pr
