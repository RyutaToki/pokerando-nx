// Port de StaticEncounter (Pokémon estáticos y regalos).
#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "gamedata/encounter.hpp"

namespace pr {

struct StaticEncounter {
    explicit StaticEncounter(Species* s) : holder(s) {}
    StaticEncounter deepCopy() const;  // new StaticEncounter(original)

    SpeciesHolder holder;
    int level = 0;
    int maxLevel = 0;
    std::optional<int> heldItem;
    bool isEgg = false;
    bool resetMoves = false;
    bool restrictedPool = false;
    std::vector<Species*> restrictedList;
    std::vector<StaticEncounter> linkedEncounters;

    Species* species() const { return holder.species(); }
};

inline StaticEncounter StaticEncounter::deepCopy() const { return *this; }

}  // namespace pr
