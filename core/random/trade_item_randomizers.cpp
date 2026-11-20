#include "random/trade_item_randomizers.hpp"

#include <algorithm>

#include "constants/item_ids.hpp"
#include "random/custom_names.hpp"

namespace pr {

namespace {
template <class T>
bool contains(const std::vector<T>& v, const T& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
}  // namespace

void TradeRandomizer::randomizeIngameTrades() {
    bool randomizeRequest = settings.inGameTradesMod == Settings::InGameTradesMod::RANDOMIZE_GIVEN_AND_REQUESTED;
    bool randomNickname = settings.randomizeInGameTradesNicknames;
    bool randomOT = settings.randomizeInGameTradesOTs;
    CustomNamesSet names = CustomNamesSet::readEmbedded();
    std::vector<std::string> trainerNames, nicknames;
    if (randomOT) {
        int maxOT = romHandler.maxTradeOTNameLength();
        for (const auto& n : names.trainerNames)
            if (romHandler.internalStringLength(n) <= maxOT && !contains(trainerNames, n)) trainerNames.push_back(n);
    }
    if (randomNickname) {
        int maxNN = romHandler.maxTradeNicknameLength();
        for (const auto& n : names.pokemonNicknames)
            if (romHandler.internalStringLength(n) <= maxNN && !contains(nicknames, n)) nicknames.push_back(n);
    }
    auto trades = romHandler.getInGameTrades();
    std::vector<Species*> usedRequests, usedGivens;
    std::vector<std::string> usedOTs, usedNicknames;
    auto possibleItems = romHandler.getAllowedItems().toVector();
    int nickCount = static_cast<int>(nicknames.size()), trnameCount = static_cast<int>(trainerNames.size());
    for (auto& trade : trades) {
        Species* oldGiven = trade.givenSpecies();
        Species* given = rSpecService.randomSpecies(random);
        while (contains(usedGivens, given)) given = rSpecService.randomSpecies(random);
        usedGivens.push_back(given);
        trade.givenHolder.setSpecies(given);
        auto& sh = trade.givenHolder;
        if (sh.isAltFormeAllowed() && sh.species()->isBaseForme()) sh.setFormeNumber(sh.species()->baseForme()->randomCosmeticFormeNumber(random));
        if (oldGiven == trade.requestedSpecies) {
            trade.requestedSpecies = given;
        } else if (randomizeRequest && trade.requestedSpecies) {
            Species* req = rSpecService.randomSpecies(random);
            while (contains(usedRequests, req) || req == given) req = rSpecService.randomSpecies(random);
            usedRequests.push_back(req);
            trade.requestedSpecies = req;
        }
        if (randomNickname && nickCount > static_cast<int>(usedNicknames.size())) {
            std::string nn = nicknames[random.nextInt(nickCount)];
            while (contains(usedNicknames, nn)) nn = nicknames[random.nextInt(nickCount)];
            usedNicknames.push_back(nn);
            trade.nickname = nn;
        } else if (jc::equalsIgnoreCase(trade.nickname, oldGiven->name())) {
            trade.nickname = trade.givenSpecies()->name();
        }
        if (randomOT && trnameCount > static_cast<int>(usedOTs.size())) {
            std::string ot = trainerNames[random.nextInt(trnameCount)];
            while (contains(usedOTs, ot)) ot = trainerNames[random.nextInt(trnameCount)];
            usedOTs.push_back(ot);
            trade.otName = ot;
            trade.otId = random.nextInt(65536);
        }
        if (settings.randomizeInGameTradesIVs) {
            int maxIV = romHandler.hasDVs() ? 16 : 32;
            for (auto& iv : trade.ivs) iv = random.nextInt(maxIV);
        }
        if (settings.randomizeInGameTradesItems) trade.heldItem = possibleItems[random.nextInt(static_cast<int>(possibleItems.size()))];
    }
    romHandler.setInGameTrades(trades);
    changesMade = true;
}

void ItemRandomizer::randomizeFieldItems() {
    fieldChangesMade = true;
    auto current = romHandler.getFieldItems();
    const auto& items = romHandler.getItems();
    std::vector<int> tms, nonTMs;  // Stack
    for (int it : current) (items[it]->tm ? tms : nonTMs).push_back(it);
    switch (settings.fieldItemsMod) {
        case Settings::FieldItemsMod::SHUFFLE:
            random.shuffle(tms);
            random.shuffle(nonTMs);
            break;
        case Settings::FieldItemsMod::RANDOM:
        case Settings::FieldItemsMod::RANDOM_EVEN:
            randomizeTMFieldItems(tms);
            randomizeNonTMFieldItems(nonTMs);
            break;
        default: break;
    }
    std::vector<int> combined;
    for (int it : current) {
        auto& st = items[it]->tm ? tms : nonTMs;
        combined.push_back(st.back());
        st.pop_back();
    }
    romHandler.setFieldItems(combined);
}

void ItemRandomizer::randomizeTMFieldItems(std::vector<int>& tms) {
    std::vector<int> allTMs;
    for (const auto& it : romHandler.getItems())
        if (it && it->tm) allTMs.push_back(it->id);
    ItemSet required = romHandler.getRequiredFieldTMs();
    int needed = static_cast<int>(tms.size());
    ItemSet newTMs = ItemSet::fromCollection(required.toVector());
    while (newTMs.size() < needed && newTMs.size() < static_cast<int>(allTMs.size()))
        newTMs.add(allTMs[random.nextInt(static_cast<int>(allTMs.size()))]);
    if (newTMs.size() != needed) throw RandomizationException("Could not randomize TM field items, too many TMs requested.");
    tms = newTMs.toVector();
    random.shuffle(tms);
}

void ItemRandomizer::randomizeNonTMFieldItems(std::vector<int>& nonTMs) {
    bool even = settings.fieldItemsMod == Settings::FieldItemsMod::RANDOM_EVEN;
    auto possible = (settings.banBadRandomFieldItems ? romHandler.getNonBadItems() : romHandler.getAllowedItems()).toVector();
    const auto& items = romHandler.getItems();
    possible.erase(std::remove_if(possible.begin(), possible.end(), [&](int i) { return items[i]->tm; }), possible.end());
    ItemSet uniqueNoSell = !settings.balanceShopPrices ? romHandler.getMegaStones() : ItemSet();
    auto removeFirst = [&](int x) {
        auto it = std::find(possible.begin(), possible.end(), x);
        if (it != possible.end()) possible.erase(it);
    };
    int needed = static_cast<int>(nonTMs.size());
    nonTMs.clear();
    if (even) {
        std::vector<int> remaining;
        random.shuffle(remaining);
        for (int i = 0; i < needed; i++) {
            if (remaining.empty()) {
                remaining = possible;
                random.shuffle(remaining);
            }
            int chosen = remaining.back();
            remaining.pop_back();
            nonTMs.push_back(chosen);
            if (uniqueNoSell.contains(chosen)) removeFirst(chosen);
        }
    } else {
        for (int i = 0; i < needed; i++) {
            int chosen = possible[random.nextInt(static_cast<int>(possible.size()))];
            nonTMs.push_back(chosen);
            if (uniqueNoSell.contains(chosen)) removeFirst(chosen);
        }
    }
}

void ItemRandomizer::shuffleShopItems() {
    shopChangesMade = true;
    auto shops = romHandler.getShops();
    std::vector<int> all;
    for (const auto& s : shops)
        if (s.isSpecialShop) all.insert(all.end(), s.items.begin(), s.items.end());
    random.shuffle(all);
    auto it = all.begin();
    for (auto& s : shops) {
        if (!s.isSpecialShop) continue;
        for (auto& i : s.items) i = *it++;
    }
    romHandler.setShops(shops);
}

void ItemRandomizer::randomizeShopItems() {
    shopChangesMade = true;
    const auto& items = romHandler.getItems();
    // setupPossible
    ItemSet possible = ItemSet::fromCollection((settings.banBadRandomShopItems ? romHandler.getNonBadItems() : romHandler.getAllowedItems()).toVector());
    possible.removeIf([&](const int& i) { return items[i]->tm; });
    if (settings.banRegularShopItems) {
        auto r = romHandler.getRegularShopItems();
        possible.removeIf([&](const int& i) { return r.contains(i); });
    }
    if (settings.banOPShopItems) {
        auto op = romHandler.getOPShopItems();
        possible.removeIf([&](const int& i) { return op.contains(i); });
    }
    // setupGuaranteed
    ItemSet guaranteed;
    if (settings.guaranteeEvolutionItems) guaranteed.addAll(romHandler.getEvolutionItems().toVector());
    if (settings.guaranteeXItems) guaranteed.addAll(romHandler.getXItems().toVector());
    auto shops = romHandler.getShops();
    // setupNewItems
    std::vector<int> newItems = guaranteed.toVector();
    int count = 0;
    for (const auto& s : shops)
        if (s.isSpecialShop) count += static_cast<int>(s.items.size());
    count -= guaranteed.size();
    possible.removeIf([&](const int& i) { return guaranteed.contains(i); });
    std::vector<int> remaining;
    random.shuffle(remaining);
    for (int i = 0; i < count; i++) {
        if (remaining.empty()) {
            remaining = possible.toVector();
            random.shuffle(remaining);
        }
        newItems.push_back(remaining.back());
        remaining.pop_back();
    }
    // placeNewItems
    std::vector<Shop*> mainGame, nonMain;
    for (auto& s : shops)
        if (s.isSpecialShop) (s.isMainGame ? mainGame : nonMain).push_back(&s);
    random.shuffle(newItems);
    for (Shop* s : nonMain) {
        size_t pos = 0;
        for (auto& slot : s->items) {
            while (guaranteed.contains(newItems.at(pos))) pos++;
            slot = newItems[pos];
            newItems.erase(newItems.begin() + static_cast<long>(pos));
        }
    }
    random.shuffle(newItems);
    for (Shop* s : mainGame) {
        for (auto& slot : s->items) {
            slot = newItems.at(0);
            newItems.erase(newItems.begin());
        }
    }
    if (!newItems.empty()) throw std::logic_error("newItems has not been emptied");
    romHandler.setShops(shops);
}

void ItemRandomizer::addCheapRareCandiesToShops() {
    shopChangesMade = true;
    auto shops = romHandler.getShops();
    for (auto& s : shops) s.items.push_back(items::rareCandy);
    romHandler.setShops(shops);
    auto prices = romHandler.getShopPrices();
    prices.at(items::rareCandy) = 10;
    romHandler.setShopPrices(prices);
}

void ItemRandomizer::randomizePickupItems() {
    pickupChangesMade = true;
    auto possible = (settings.banBadRandomPickupItems ? romHandler.getNonBadItems() : romHandler.getAllowedItems()).toVector();
    const auto& items = romHandler.getItems();
    if (!romHandler.canTMsBeHeld() || romHandler.isTMsReusable())
        possible.erase(std::remove_if(possible.begin(), possible.end(), [&](int i) { return items[i]->tm; }), possible.end());
    auto current = romHandler.getPickupItems();
    std::vector<PickupItem> out;
    for (const auto& c : current) {
        PickupItem p(possible[random.nextInt(static_cast<int>(possible.size()))]);
        p.probabilities = c.probabilities;
        out.push_back(p);
    }
    romHandler.setPickupItems(out);
}

}  // namespace pr
