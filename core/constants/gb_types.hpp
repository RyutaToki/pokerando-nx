// GBConstants.physicalTypes (en Gen 1-3 la categoría depende del tipo).
#pragma once
#include <optional>

#include "gamedata/types.hpp"

namespace pr::gb {
inline bool isPhysicalType(std::optional<Type> t) {
    if (!t) return false;
    switch (*t) {
        case Type::NORMAL: case Type::FIGHTING: case Type::POISON: case Type::GROUND: case Type::FLYING:
        case Type::BUG: case Type::ROCK: case Type::GHOST: case Type::STEEL: return true;
        default: return false;
    }
}
}  // namespace pr::gb
