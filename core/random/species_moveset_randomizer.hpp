// Port de SpeciesMovesetRandomizer.
#pragma once
#include <map>

#include "random/randomizer.hpp"

namespace pr {

class SpeciesMovesetRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeMovesLearnt();
    void randomizeEggMoves();
    void orderDamagingMovesByDamage();
    void metronomeOnlyMode();

private:
    struct MoveSets {
        std::vector<Move*> valid, validDamaging;
        std::map<Type, std::vector<Move*>> validType, validTypeDamaging;  // HashMap<Type, ...> en Java
    };
    MoveSets createSetsOfMoves(bool noBroken);
    std::optional<Type> pickTypeOfMove(Species* pk);
    Move* pickMove(MoveSets& sets, bool attemptDamaging, std::optional<Type> typeOfMove, double atkSpAtkRatio,
                   const std::vector<int>& learnt);
};

}  // namespace pr
