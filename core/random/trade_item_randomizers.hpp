// Port de TradeRandomizer e ItemRandomizer.
#pragma once
#include "random/randomizer.hpp"

namespace pr {

class TradeRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeIngameTrades();
};

class ItemRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeFieldItems();
    void shuffleShopItems();
    void randomizeShopItems();
    void addCheapRareCandiesToShops();
    void randomizePickupItems();
    bool fieldChangesMade = false, shopChangesMade = false, pickupChangesMade = false;

private:
    void randomizeTMFieldItems(std::vector<int>& tms);
    void randomizeNonTMFieldItems(std::vector<int>& nonTMs);
};

}  // namespace pr
