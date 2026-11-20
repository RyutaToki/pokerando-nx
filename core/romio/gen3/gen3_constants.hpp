// Constantes de Gen 3. Las simples se generan de Gen3Constants.java
// (gen3_constants_gen.hpp); aquí van las que necesitan código.
#pragma once
#include <optional>
#include <set>

#include "constants/global_constants.hpp"
#include "constants/gb_constants.hpp"
#include "gamedata/types.hpp"
#include "romio/gen3/gen3_constants_gen.hpp"

namespace pr::gen3 {

// typeTable[byte] -> tipo (nullopt si el byte no es un tipo; 0x09 es el tipo "???")
std::optional<Type> typeFromByte(int b);
int typeToByte(std::optional<Type> t);  // nullopt -> 0x09 (tipo ???)

int itemIDToStandard(int internalId);
int itemIDToInternal(int standardId);
int getLastItemID(int romType);
const std::set<int>& bannedItems();
const std::set<int>& badItems(int romType);

}  // namespace pr::gen3
