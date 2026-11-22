// Constantes de Gen 5. Las simples se generan de Gen5Constants.java (gen5_constants_gen.hpp);
// aquí van las que necesitan código.
#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "gamedata/encounter.hpp"
#include "gamedata/evolution.hpp"
#include "gamedata/move.hpp"
#include "gamedata/types.hpp"
#include "romio/gen5/gen5_constants_gen.hpp"
#include "util/java_hash.hpp"

namespace pr::gen5 {

// no son final en Java y el conversor no las recoge
inline constexpr std::string_view blackBoxLegendaryCheckPrefix1 = "79F6BAEF07B0F0BDC046",
                                  blackBoxLegendaryCheckPrefix2 = "DEDB0020C04302B0F8BDC046",
                                  whiteBoxLegendaryCheckPrefix1 = "00F0FEF8002070BD",
                                  whiteBoxLegendaryCheckPrefix2 = "64F62EF970BD0000";

inline const std::vector<std::string_view> encounterTypeNames = {"Grass/Cave",    "Doubles Grass", "Shaking Spots", "Surfing",
                                                                 "Surfing Spots", "Fishing",       "Fishing Spots"};
inline const std::vector<EncounterType> encounterTypeValues = {EncounterType::WALKING, EncounterType::WALKING, EncounterType::AMBUSH,
                                                               EncounterType::SURFING, EncounterType::AMBUSH,  EncounterType::FISHING,
                                                               EncounterType::AMBUSH};

std::optional<Type> typeFromByte(int b);  // typeTable[b]; fuera de rango lanza
int typeToByte(std::optional<Type> t);
MoveCategory moveCategoryFromByte(int b);
int moveCategoryToByte(MoveCategory c);
int evolutionTypeToIndex(EvolutionType t);
EvolutionType evolutionTypeFromIndex(int index);
int getMapIndexForLocationEvolution(EvolutionType et, int romType);

int getAreaDataEntryLength(int romType);
int getEncounterAreaCount(int romType);
const std::vector<int>& getWildFileToAreaMap(int romType);
const std::vector<int>& getMainGameShops(int romType);
const std::vector<int>& getIrregularFormes(int romType);
int getFormeCount(int romType);
int getFormeOffset(int romType);
int getNonPokemonBattleSpriteCount(int romType);
std::string getFormeSuffixByBaseForme(int baseForme, int formNum);
int getAbsolutePokeNumByBaseForme(int baseForme, int formNum);
const std::vector<std::string_view>& getLocationTagsTraverseOrder(int romType);

const JavaHashSet<int>& bannedItems();
const JavaHashSet<int>& getBadItems(int romType);
const JavaHashSet<int>& opShopItems();

}  // namespace pr::gen5
