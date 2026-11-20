#pragma once
#include <vector>

#include "gamedata/trainer.hpp"

namespace pr::gen3 {
void trainerTagsRS(std::vector<TrainerPtr>& trs, int romType);
void trainerTagsE(std::vector<TrainerPtr>& trs);
void trainerTagsFRLG(std::vector<TrainerPtr>& trs);
void setMultiBattleStatusEm(std::vector<TrainerPtr>& trs);
}  // namespace pr::gen3
