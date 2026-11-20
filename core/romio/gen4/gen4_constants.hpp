// Constantes de Gen 4. Las simples se generan de Gen4Constants.java (gen4_constants_gen.hpp);
// aquí van las que necesitan código.
#pragma once
#include <map>
#include <optional>
#include <string>

#include "gamedata/evolution.hpp"
#include "gamedata/move.hpp"
#include "gamedata/types.hpp"
#include "romio/gen4/gen4_constants_gen.hpp"
#include "util/java_hash.hpp"

namespace pr::gen4 {

std::optional<Type> typeFromByte(int b);
int typeToByte(std::optional<Type> t);
MoveCategory moveCategoryFromByte(int b);
int moveCategoryToByte(MoveCategory c);
int evolutionTypeToIndex(EvolutionType t);
EvolutionType evolutionTypeFromIndex(int index);
int getFormeCount(int romType);

struct FormeInfo {
    int baseForme = 0, formeNumber = 0;
};
// TreeMap<Integer, FormeInfo>: id del archivo de stats -> forma
const std::map<int, FormeInfo>& formeMappings();
int cosmeticFormsOf(int species);  // cosmeticForms.getOrDefault(species, 0)
std::string getFormeSuffixByBaseForme(int baseForme, int formNum);
int getAbsolutePokeNumByBaseForme(int baseForme, int formNum);

const JavaHashSet<int>& bannedItems();
const JavaHashSet<int>& badItems();
const JavaHashSet<int>& opShopItems();

}  // namespace pr::gen4
