// Port de MoveDataRandomizer y TMTutorMoveRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class MoveDataRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeMovePowers();
    void randomizeMovePPs();
    void randomizeMoveAccuracies();
    void randomizeMoveTypes();
    void randomizeMoveCategory();
};

class TMTutorMoveRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeTMMoves();
    void randomizeMoveTutorMoves();
    bool tmChangesMade = false, tutorChangesMade = false;

private:
    std::vector<int> pickMoves(int count, const std::vector<int>& oldMoves, bool noBroken, bool preserveField,
                               double goodDamagingPct, const std::vector<int>& extraBanned);
};

}  // namespace pr
