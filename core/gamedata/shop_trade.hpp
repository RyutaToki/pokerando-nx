// Port de Shop, PickupItem e InGameTrade.
#pragma once
#include <array>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/encounter.hpp"

namespace pr {

struct Shop {
    std::vector<int> items;  // ids estándar
    std::string name;
    bool isMainGame = false;
    bool isSpecialShop = false;
};

struct PickupItem {
    static constexpr int PROBABILITY_SLOTS = 10;
    explicit PickupItem(int item) : item(item) {}
    int item;
    std::array<int, PROBABILITY_SLOTS> probabilities{};
};

struct InGameTrade {
    InGameTrade(Species* requested, Species* given) : requestedSpecies(requested), givenHolder(given) {}
    Species* requestedSpecies;
    SpeciesHolder givenHolder;
    std::string nickname;
    std::optional<std::string> otName;
    int otId = 0;
    std::vector<int> ivs;
    std::optional<int> heldItem;
    Species* givenSpecies() const { return givenHolder.species(); }
};

}  // namespace pr
