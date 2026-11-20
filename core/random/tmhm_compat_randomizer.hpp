// Port de TMHMTutorCompatibilityRandomizer.
#pragma once
#include <map>

#include "random/randomizer.hpp"

namespace pr {

class TMHMTutorCompatibilityRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    using Compat = std::map<int, std::vector<bool>>;  // TreeMap<Species, boolean[]>

    void randomizeTMHMCompatibility();
    void fullTMHMCompatibility();
    void ensureTMCompatSanity();
    void ensureTMEvolutionSanity();
    void fullHMCompatibility();
    void copyTMCompatibilityToCosmeticFormes();
    void randomizeMoveTutorCompatibility();
    void fullMoveTutorCompatibility();
    void ensureMoveTutorCompatSanity();
    void ensureMoveTutorEvolutionSanity();
    void copyMoveTutorCompatibilityToCosmeticFormes();
    bool tmhmChangesMade = false, tutorChangesMade = false;

private:
    void randomizeCompat(Species* pk, std::vector<bool>& flags, const std::vector<int>& moveIDs, const std::vector<int>& prioritized,
                         bool preferSameType);
    void copyUp(Species* from, Species* to, const std::vector<bool>& prev, std::vector<bool>& to_, const std::vector<int>& moveIDs,
                bool preferSameType);
    double probability(Species* pk, const Move& mv, bool requiredEarly, bool preferSameType);
    void randomizeGeneric(Compat& compat, const std::vector<int>& moves, const std::vector<int>& prioritized, bool preferSameType,
                          bool followEvolutions, bool copySplit);
    void sanity(Compat& compat, const std::vector<int>& moves);
    void evoSanity(Compat& compat);
    void cosmetic(Compat& compat);
};

}  // namespace pr
