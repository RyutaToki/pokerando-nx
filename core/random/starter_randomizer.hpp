// Port de StarterRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class StarterRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeStarters();
    void randomizeStarterHeldItems();

private:
    Type chooseTypeForStarters(int numNeeded, std::map<Type, SpeciesSet>& byType);
    std::vector<Species*> chooseStartersFireWaterGrass(std::map<Type, SpeciesSet>& byType);
    std::vector<Species*> chooseTypeTriangleStarters(std::map<Type, SpeciesSet>& byType, bool firstTrio);
    std::vector<Species*> chooseStartersOfTypes(std::map<Type, SpeciesSet>& byType, const std::vector<Type>& types);
    std::vector<Species*> chooseStartersBasic(int numberPicks, SpeciesSet& available);
    std::vector<Species*> chooseUniqueTypeStarters(const SpeciesSet& alreadyPicked, int numberPicks, SpeciesSet available);
    SpeciesSet getAvailableSet(const std::vector<Species*>& alreadyChosen);
    std::vector<Species*> getCustomStarters(const std::vector<int>& indices);
    std::vector<std::vector<Type>> findTypeTriangles();
};

}  // namespace pr
