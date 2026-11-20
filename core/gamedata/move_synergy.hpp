// Port de MoveSynergy (FVX): sinergias entre movimientos, habilidades y estadísticas.
#pragma once
#include <optional>
#include <vector>

#include "gamedata/move.hpp"
#include "gamedata/type_table.hpp"
#include "gamedata/types.hpp"

namespace pr::MoveSynergy {

std::vector<Move*> getSoftAbilityMoveSynergy(int ability, const std::vector<Move*>& moveList, std::optional<Type> pkType1,
                                             std::optional<Type> pkType2);
std::vector<Move*> getSoftAbilityMoveAntiSynergy(int ability, const std::vector<Move*>& moveList);
std::vector<Move*> getHardAbilityMoveSynergy(int ability, std::optional<Type> pkType1, std::optional<Type> pkType2,
                                             const std::vector<Move*>& moveList, int generation, int perfectAccuracy);
std::vector<Move*> getHardAbilityMoveAntiSynergy(int ability, const std::vector<Move*>& moveList);
std::vector<Move*> getStatMoveSynergy(const BaseStats& bs, const std::vector<Move*>& moveList);
std::vector<Move*> getStatMoveAntiSynergy(const BaseStats& bs, const std::vector<Move*>& moveList);
std::vector<Move*> getMoveSynergy(const Move* mv1, const std::vector<Move*>& moveList, int generation);
std::vector<Move*> getSoftMoveSynergy(const Move* mv1, const std::vector<Move*>& moveList, const TypeTable& typeTable);
std::vector<Move*> getHardMoveAntiSynergy(const Move* mv1, const std::vector<Move*>& moveList);
std::vector<Move*> getSoftMoveAntiSynergy(const Move* mv1, const std::vector<Move*>& moveList);
std::vector<Move*> requiresOtherMove(const Move* mv1, const std::vector<Move*>& moveList);

}  // namespace pr::MoveSynergy
