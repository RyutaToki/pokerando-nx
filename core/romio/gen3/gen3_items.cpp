// Objetos, tiendas, Recogida, objetos del suelo e intercambios de Gen 3.
#include <algorithm>

#include "constants/gb_types.hpp"
#include "constants/item_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

ItemSet Gen3RomHandler::itemIdsToSet(const std::vector<int>& ids) const {
    // Collectors.toSet(): HashSet<Item>; Item.hashCode() es su id
    ItemSet s;
    for (int id : ids) {
        if (id < static_cast<int>(items_.size()) && items_[id]) s.add(id);
    }
    return s;
}
ItemSet Gen3RomHandler::getEvolutionItems() const { return itemIdsToSet(evolutionItems); }
ItemSet Gen3RomHandler::getAllConsumableHeldItems() const { return itemIdsToSet(consumableHeldItems); }

// opShopItems es un HashSet<Integer>: se construye en el mismo orden de inserción que en Java
// para reproducir su orden de iteración (hay colisiones de cubeta con tablas pequeñas)
std::vector<int> opShopItemsInJavaOrder() {
    JavaHashSet<int> set;
    set.add(items::rareCandy);
    for (int i = items::tinyMushroom; i <= items::bigMushroom; i++) set.add(i);
    for (int i = items::pearl; i <= items::nugget; i++) set.add(i);
    set.add(items::luckyEgg);
    return set.toVector();
}

ItemSet Gen3RomHandler::getOPShopItems() const { return itemIdsToSet(opShopItemsInJavaOrder()); }

ItemSet Gen3RomHandler::getRequiredFieldTMs() const {
    switch (romEntry_->romType()) {
        case RomType_FRLG: return itemIdsToSet(frlgRequiredFieldTMs);
        case RomType_Ruby:
        case RomType_Sapp: return itemIdsToSet(rsRequiredFieldTMs);
        default: return itemIdsToSet(eRequiredFieldTMs);  // Esmeralda tiene algunas MT de Recogida
    }
}

std::vector<int> Gen3RomHandler::getFieldItems() const {
    std::vector<int> out;
    for (int offset : itemOffs_) {
        const Item& it = items_.at(itemIDToStandard(readWord(offset))).value();
        if (it.allowed) out.push_back(it.id);
    }
    return out;
}

void Gen3RomHandler::setFieldItems(const std::vector<int>& fieldItems) {
    // checkFieldItemsTMsReplaceTMs
    auto current = getFieldItems();
    for (size_t i = 0; i < current.size(); i++) {
        if (items_.at(current[i])->tm != items_.at(fieldItems.at(i))->tm)
            throw std::invalid_argument("TMs must replace TMs, non-TMs must replace non-TMs");
    }
    auto it = fieldItems.begin();
    for (int offset : itemOffs_) {
        const Item& cur = items_.at(itemIDToStandard(readWord(offset))).value();
        if (cur.allowed) writeWord(offset, itemIDToInternal(*it++));
    }
}

std::vector<Shop> Gen3RomHandler::getShops() const {
    const std::vector<std::string_view>& names =
        romEntry_->romType() == RomType_FRLG ? frlgShopNames : (romEntry_->romType() == RomType_Em ? emShopNames : rsShopNames);
    std::vector<int> mainGame = romEntry_->getArray("MainGameShops");
    std::vector<int> skip = romEntry_->getArray("SkipShops");
    std::vector<int> ptrs = romEntry_->getArray("ShopPointerOffsets");
    auto has = [](const std::vector<int>& v, int x) { return std::find(v.begin(), v.end(), x) != v.end(); };
    std::vector<Shop> shops;
    for (size_t i = 0; i < ptrs.size(); i++) {
        int offset = readPointer(ptrs[i]);
        Shop shop;
        for (int val = readWord(offset); val != 0; val = readWord(offset)) {
            shop.items.push_back(items_.at(itemIDToStandard(val)).value().id);
            offset += 2;
        }
        shop.name = std::string(names.at(i));
        shop.isMainGame = has(mainGame, static_cast<int>(i));
        shop.isSpecialShop = !has(skip, static_cast<int>(i));
        shops.push_back(shop);
    }
    return shops;
}

void Gen3RomHandler::setShops(const std::vector<Shop>& shops) {
    std::vector<int> ptrs = romEntry_->getArray("ShopPointerOffsets");
    if (shops.size() != ptrs.size())
        throw std::runtime_error("Wrong amount of shops. Should be " + std::to_string(ptrs.size()));
    for (size_t i = 0; i < shops.size(); i++) {
        std::vector<uint8_t> data;
        for (int item : shops[i].items) {
            int v = itemIDToInternal(item);
            data.push_back(static_cast<uint8_t>(v % 0x100));
            data.push_back(static_cast<uint8_t>((v / 0x100) % 0x100));
        }
        data.insert(data.end(), shopTerminator.begin(), shopTerminator.end());
        rewriteData(ptrs[i], data, [this](int old) {
            // lengthOfDataWithTerminatorAt(old, shopTerminator): primera aparición del terminador
            auto it = std::search(rom_.cbegin() + old, rom_.cend(), shopTerminator.cbegin(), shopTerminator.cend());
            return static_cast<int>(it - (rom_.cbegin() + old)) + static_cast<int>(shopTerminator.size());
        });
    }
}

std::vector<int> Gen3RomHandler::getShopPrices() const {
    int itemData = romEntry_->getInt("ItemData");
    int entrySize = romEntry_->getInt("ItemEntrySize");
    int count = romEntry_->getInt("ItemCount");
    std::vector<int> prices(items_.size(), 0);
    for (int internal = 1; internal < count; internal++) {
        prices.at(itemIDToStandard(internal)) = readWord(itemData + internal * entrySize + 16);
    }
    return prices;
}

void Gen3RomHandler::setShopPrices(const std::vector<int>& prices) {
    if (prices.size() != items_.size()) throw std::invalid_argument("prices.size() must equals items.size()");
    int itemData = romEntry_->getInt("ItemData");
    int entrySize = romEntry_->getInt("ItemEntrySize");
    int count = romEntry_->getInt("ItemCount");
    for (int internal = 1; internal < count; internal++) {
        writeWord(itemData + internal * entrySize + 16, prices[itemIDToStandard(internal)]);
    }
}

void Gen3RomHandler::setBalancedShopPrices() {
    auto prices = getShopPrices();
    for (auto [id, price] : balancedItemPrices) {
        if (id < static_cast<int>(prices.size())) prices[id] = price;
    }
    setShopPrices(prices);
}

std::vector<PickupItem> Gen3RomHandler::getPickupItems() {
    std::vector<PickupItem> out;
    int count = romEntry_->getInt("PickupItemCount");
    int size = romEntry_->romType() == RomType_Em ? 2 : 4;
    if (pickupItemsTableOffset_ == 0) {
        int off = find(romEntry_->getString("PickupTableStartLocator"));
        if (off > 0) pickupItemsTableOffset_ = off;
    }
    if (pickupItemsTableOffset_ > 0) {
        for (int i = 0; i < count; i++) {
            out.emplace_back(items_.at(itemIDToStandard(readWord(pickupItemsTableOffset_ + size * i))).value().id);
        }
    }
    if (out.empty()) return out;
    for (int lr = 0; lr < 10; lr++) {
        switch (romEntry_->romType()) {
            case RomType_Ruby:
            case RomType_Sapp:
                out.at(0).probabilities[lr] = 30;
                out.at(7).probabilities[lr] = 5;
                out.at(8).probabilities[lr] = 4;
                out.at(9).probabilities[lr] = 1;
                for (int i = 1; i < 7; i++) out.at(i).probabilities[lr] = 10;
                break;
            case RomType_FRLG:
                out.at(0).probabilities[lr] = 15;
                for (int i = 1; i < 7; i++) out.at(i).probabilities[lr] = 10;
                for (int i = 7; i < 11; i++) out.at(i).probabilities[lr] = 5;
                for (int i = 11; i < 16; i++) out.at(i).probabilities[lr] = 1;
                break;
            default: {
                int c = lr, r = 18 + lr;
                out.at(c).probabilities[lr] = 30;
                for (int i = 1; i < 7; i++) out.at(c + i).probabilities[lr] = 10;
                out.at(c + 7).probabilities[lr] = 4;
                out.at(c + 8).probabilities[lr] = 4;
                out.at(r).probabilities[lr] = 1;
                out.at(r + 1).probabilities[lr] = 1;
            }
        }
    }
    return out;
}

void Gen3RomHandler::setPickupItems(const std::vector<PickupItem>& pickupItems) {
    int size = romEntry_->romType() == RomType_Em ? 2 : 4;
    if (pickupItemsTableOffset_ <= 0) return;
    for (size_t i = 0; i < pickupItems.size(); i++) {
        writeWord(pickupItemsTableOffset_ + size * static_cast<int>(i), itemIDToInternal(pickupItems[i].item));
    }
}

std::vector<InGameTrade> Gen3RomHandler::getInGameTrades() const {
    std::vector<InGameTrade> trades;
    int table = romEntry_->getInt("TradeTableOffset");
    int tableSize = romEntry_->getInt("TradeTableSize");
    std::vector<int> unused = romEntry_->getArray("TradesUnused");
    size_t unusedOffset = 0;
    for (int entry = 0; entry < tableSize; entry++) {
        if (unusedOffset < unused.size() && unused[unusedOffset] == entry) {
            unusedOffset++;
            continue;
        }
        int o = table + entry * 60;
        InGameTrade t(pokesInternal_.at(readWord(o + 56)), pokesInternal_.at(readWord(o + 12)));
        t.nickname = readVariableLengthString(o);
        t.ivs.resize(6);
        for (int i = 0; i < 6; i++) t.ivs[i] = rom_.at(o + 14 + i);
        t.otId = readWord(o + 24);
        t.heldItem = itemAt(readWord(o + 40));
        t.otName = readVariableLengthString(o + 43);
        trades.push_back(t);
    }
    return trades;
}

void Gen3RomHandler::setInGameTrades(const std::vector<InGameTrade>& trades) {
    int table = romEntry_->getInt("TradeTableOffset");
    int tableSize = romEntry_->getInt("TradeTableSize");
    std::vector<int> unused = romEntry_->getArray("TradesUnused");
    size_t unusedOffset = 0;
    int tradeOffset = 0;
    for (int entry = 0; entry < tableSize; entry++) {
        if (unusedOffset < unused.size() && unused[unusedOffset] == entry) {
            unusedOffset++;
            continue;
        }
        const InGameTrade& t = trades.at(tradeOffset++);
        int o = table + entry * 60;
        writeFixedLengthString(t.nickname, o, 12);
        writeWord(o + 12, pokedexToInternal_[t.givenSpecies()->number()]);
        for (int i = 0; i < 6; i++) writeByte(o + 14 + i, static_cast<uint8_t>(t.ivs.at(i)));
        writeWord(o + 24, t.otId);
        writeWord(o + 40, t.heldItem ? itemIDToInternal(*t.heldItem) : 0);
        writeFixedLengthString(t.otName.value_or(""), o + 43, 11);
        writeWord(o + 56, pokedexToInternal_[t.requestedSpecies->number()]);
    }
}

std::vector<std::optional<int>> Gen3RomHandler::getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                                         const std::array<int, 4>& pokeMoves) const {
    std::vector<int> ids(generalPurposeConsumableItems.begin(), generalPurposeConsumableItems.end());
    if (!consumableOnly) ids.insert(ids.end(), generalPurposeItems.begin(), generalPurposeItems.end());
    for (int idx : pokeMoves) {
        const auto& move = moves_.at(idx);
        if (!move) continue;
        bool phys = gb::isPhysicalType(move->type);
        if (move->power > 0) {
            ids.push_back(phys ? items::liechiBerry : items::petayaBerry);
            if (!consumableOnly) {
                auto boost = typeBoostingItems(move->type);
                ids.insert(ids.end(), boost.begin(), boost.end());
                if (phys) ids.push_back(items::choiceBand);
            }
        }
    }
    if (!consumableOnly) {
        auto speciesItems = speciesBoostingItems(tp.species()->number());
        for (int i = 0; i < 6; i++) ids.insert(ids.end(), speciesItems.begin(), speciesItems.end());  // más probables
    }
    std::vector<std::optional<int>> out;
    for (int id : ids) out.push_back(items_.at(id) ? std::optional<int>(id) : std::nullopt);
    return out;
}

std::vector<int> typeBoostingItems(std::optional<Type> t) {
    if (!t) return {};
    switch (*t) {
        case Type::BUG: return {items::silverPowder};
        case Type::DARK: return {items::blackGlasses};
        case Type::DRAGON: return {items::dragonFang};
        case Type::ELECTRIC: return {items::magnet};
        case Type::FIGHTING: return {items::blackBelt};
        case Type::FIRE: return {items::charcoal};
        case Type::FLYING: return {items::sharpBeak};
        case Type::GHOST: return {items::spellTag};
        case Type::GRASS: return {items::miracleSeed};
        case Type::GROUND: return {items::softSand};
        case Type::ICE: return {items::neverMeltIce};
        case Type::NORMAL: return {items::silkScarf};
        case Type::POISON: return {items::poisonBarb};
        case Type::PSYCHIC: return {items::twistedSpoon};
        case Type::ROCK: return {items::hardStone};
        case Type::STEEL: return {items::metalCoat};
        case Type::WATER: return {items::mysticWater, items::seaIncense};
        default: throw std::logic_error("typeBoostingItems: tipo sin entrada");  // NPE en Java
    }
}

std::vector<int> speciesBoostingItems(int speciesNumber) {
    using namespace species;
    switch (speciesNumber) {
        case latias:
        case latios: return {items::soulDew};
        case clamperl: return {items::deepSeaTooth, items::deepSeaScale};
        case pikachu: return {items::lightBall};
        case chansey: return {items::luckyPunch};
        case ditto: return {items::metalPowder};
        case cubone:
        case marowak: return {items::thickClub};
        case farfetchd: return {items::leek};
        default: return {};
    }
}

std::vector<int> allHeldItems() {
    std::vector<int> l = {items::brightPowder, items::quickClaw, items::choiceBand, items::kingsRock, items::silverPowder,
                          items::focusBand, items::scopeLens, items::metalCoat, items::leftovers, items::softSand,
                          items::hardStone, items::miracleSeed, items::blackGlasses, items::blackBelt, items::magnet,
                          items::mysticWater, items::sharpBeak, items::poisonBarb, items::neverMeltIce, items::spellTag,
                          items::twistedSpoon, items::charcoal, items::dragonFang, items::silkScarf, items::shellBell,
                          items::seaIncense, items::laxIncense};
    l.insert(l.end(), consumableHeldItems.begin(), consumableHeldItems.end());
    return l;
}

// HashMap<Integer, List<Integer>>: el orden de iteración por clave entera
std::vector<std::pair<int, std::vector<int>>> abilityVariations() {
    JavaHashMap<int, std::vector<int>> m;
    m.put(abilities::insomnia, {abilities::insomnia, abilities::vitalSpirit});
    m.put(abilities::clearBody, {abilities::clearBody, abilities::whiteSmoke});
    m.put(abilities::hugePower, {abilities::hugePower, abilities::purePower});
    m.put(abilities::battleArmor, {abilities::battleArmor, abilities::shellArmor});
    m.put(abilities::cloudNine, {abilities::cloudNine, airLockIndex});
    std::vector<std::pair<int, std::vector<int>>> out;
    m.forEach([&](const int& k, const std::vector<int>& v) { out.emplace_back(k, v); });
    return out;
}

}  // namespace pr::gen3

namespace pr::gen3 {
std::vector<int> Gen3RomHandler::getUselessAbilities() const { return gen3::uselessAbilities; }
}  // namespace pr::gen3
