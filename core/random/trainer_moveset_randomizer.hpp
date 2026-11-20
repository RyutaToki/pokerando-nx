// Port de TrainerMovesetRandomizer ("mejores movimientos" para entrenadores).
#pragma once
#include <map>
#include <optional>

#include "random/randomizer.hpp"

namespace pr {

class TrainerMovesetRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeTrainerMovesets();

private:
    using MoveList = std::vector<Move*>;
    std::optional<std::map<int, std::vector<MoveLearnt>>> allLevelUp_;
    std::optional<std::map<int, std::vector<int>>> allEgg_;
    std::optional<std::map<int, std::vector<bool>>> allTMCompat_, allTutorCompat_;
    std::optional<std::vector<int>> allTMMoves_, allTutorMoves_;

    MoveList poolAtLevel(const TrainerPokemon& tp, bool cyclic);
    MoveList trimMoveList(TrainerPokemon& tp, MoveList moves, bool multiOnly);
    MoveList obsoleteMoves(const MoveList& moves);
    MoveList abilitySynergies(const TrainerPokemon& tp, Species* pk, MoveList moves, double hardBias, double softBias);
    MoveList statSynergies(Species* pk, MoveList moves, double bias);
    double atkSpatkRatio(const TrainerPokemon& tp, Species* pk);
    bool hasAbilities() { return romHandler.abilitiesPerSpecies() != 0; }
};

}  // namespace pr
