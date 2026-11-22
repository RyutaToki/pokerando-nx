// Objetos de Gen 5: conjuntos de objetos, objetos de campo, tiendas, precios, Recogida, intercambios y
// objetos equipables «sensatos» para entrenadores (port de Gen5RomHandler y Gen5Constants).
#include <algorithm>
#include <stdexcept>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"
#include "text/gen5_text.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int readLong(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeLong(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
int readRelativePointer(const Bytes& d, int o) { return readLong(d, o) + o + 4; }

constexpr int ITCM_END = 0x1FFFFFF, ITCM_LENGTH = 0x8000, ITCM_RAM_ADDRESS = 0x1000000;

std::vector<int> concat(std::vector<int> a, std::initializer_list<int> b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

const std::vector<int>& consumableHeldItems5() {
    using namespace items;
    static const auto l = concat(gen4::consumableHeldItems, {airBalloon, redCard, absorbBulb, cellBattery, ejectButton, fireGem, waterGem,
                                                            electricGem, grassGem, iceGem, fightingGem, poisonGem, groundGem, flyingGem,
                                                            psychicGem, bugGem, rockGem, ghostGem, dragonGem, darkGem, steelGem, normalGem});
    return l;
}
const std::vector<int>& allHeldItems5() {
    using namespace items;
    static const auto l = concat(concat(gen4::allHeldItems(), {airBalloon, redCard, absorbBulb, cellBattery, ejectButton, fireGem, waterGem,
                                                                electricGem, grassGem, iceGem, fightingGem, poisonGem, groundGem, flyingGem,
                                                                psychicGem, bugGem, rockGem, ghostGem, dragonGem, darkGem, steelGem, normalGem}),
                                 {eviolite, floatStone, rockyHelmet, ringTarget, bindingBand});
    return l;
}
const std::vector<int>& generalPurposeConsumableItems5() {
    using namespace items;
    static const auto l = concat(gen4::generalPurposeConsumableItems, {redCard, absorbBulb, cellBattery, ejectButton});
    return l;
}
const std::vector<int>& generalPurposeItems5() {
    static const auto l = concat(gen4::generalPurposeItems, {items::floatStone, items::rockyHelmet});
    return l;
}
// Gen5Constants.consumableTypeBoostingItems: las gemas
int consumableTypeBoostingItem(std::optional<Type> t) {
    using namespace items;
    if (!t) throw std::logic_error("consumableTypeBoostingItems: null");
    switch (*t) {
        case Type::FIRE: return fireGem;
        case Type::WATER: return waterGem;
        case Type::ELECTRIC: return electricGem;
        case Type::GRASS: return grassGem;
        case Type::ICE: return iceGem;
        case Type::FIGHTING: return fightingGem;
        case Type::POISON: return poisonGem;
        case Type::GROUND: return groundGem;
        case Type::FLYING: return flyingGem;
        case Type::PSYCHIC: return psychicGem;
        case Type::BUG: return bugGem;
        case Type::ROCK: return rockGem;
        case Type::GHOST: return ghostGem;
        case Type::DRAGON: return dragonGem;
        case Type::DARK: return darkGem;
        case Type::STEEL: return steelGem;
        case Type::NORMAL: return normalGem;
        default: throw std::logic_error("consumableTypeBoostingItems: tipo sin entrada");
    }
}
// Gen5Constants.moveBoostingItems: los de Gen 4 con algunos cambios
std::vector<int> moveBoostingItems5(int move) {
    using namespace items;
    switch (move) {
        case moves::trick:
        case moves::switcheroo: return {toxicOrb, flameOrb, ringTarget};
        case moves::bind:
        case moves::clamp:
        case moves::fireSpin:
        case moves::magmaStorm:
        case moves::sandTomb:
        case moves::whirlpool:
        case moves::wrap: return {gripClaw, bindingBand};
        case moves::hornLeech: return {bigRoot};
        default: {
            const auto* l = gen4::moveBoostingItems(move);
            return l ? *l : std::vector<int>{};
        }
    }
}
}  // namespace

// ---------------- punteros de la ARM9 (AbstractDSRomHandler) ----------------

int Gen5RomHandler::itcmSrcOffset() const { return readLong(arm9_, tcmCopyingPointersOffset_ + 8) - gen5::arm9Offset; }

bool Gen5RomHandler::isARM9Pointer(const Bytes& data, int offset) const {
    int pointer = readLong(data, offset);
    return pointer <= ITCM_END || (pointer >= gen5::arm9Offset && pointer < gen5::arm9Offset + static_cast<int>(arm9_.size()));
}

int Gen5RomHandler::readARM9Pointer(const Bytes& data, int offset) const {
    int pointer = readLong(data, offset);
    if (pointer <= ITCM_END) return pointer % ITCM_LENGTH + itcmSrcOffset();
    if (pointer >= gen5::arm9Offset && pointer < gen5::arm9Offset + static_cast<int>(arm9_.size())) return pointer - gen5::arm9Offset;
    throw std::runtime_error("Invalid pointer! Not a RAM location for either ARM9 or ITCM");
}

void Gen5RomHandler::writeARM9Pointer(Bytes& data, int offset, int pointer) const {
    if (tcmCopyingPointersOffset_ == -1) throw std::logic_error("tcmCopyingPointersOffset has not been initialized");
    int itcmSizeOffset = readLong(arm9_, tcmCopyingPointersOffset_) - gen5::arm9Offset + 4;
    int itcmSize = readLong(arm9_, itcmSizeOffset);
    int src = itcmSrcOffset();
    if (pointer >= src && pointer < src + itcmSize) writeLong(data, offset, pointer - src + ITCM_RAM_ADDRESS);
    else if (pointer < static_cast<int>(arm9_.size())) writeLong(data, offset, pointer + gen5::arm9Offset);
    else throw std::runtime_error("Invalid pointer! Not an offset in the ARM9.bin file");
}

// ---------------- conjuntos de objetos ----------------

ItemSet Gen5RomHandler::getEvolutionItems() const { return idsToItemSet(evolutionItems); }
ItemSet Gen5RomHandler::getAllHeldItems() const { return idsToItemSet(allHeldItems5()); }
ItemSet Gen5RomHandler::getAllConsumableHeldItems() const { return idsToItemSet(consumableHeldItems5()); }
ItemSet Gen5RomHandler::getRequiredFieldTMs() const {
    return idsToItemSet(romEntry_->romType() == Type_BW ? bw1RequiredFieldTMs : bw2RequiredFieldTMs);
}

// ---------------- objetos de campo ----------------

// Recorre las listas de scripts de objetos normales y ocultos. visit(script, offset del objeto) para cada uno.
template <class Fn>
static void walkFieldItems(std::vector<Bytes>& files, const Gen5RomEntry& re, bool isSet, Fn&& visit) {
    const auto& skipTable = re.getArray("ItemBallsSkip");
    const auto& skipTableH = re.getArray("HiddenItemsSkip");
    for (int pass = 0; pass < 2; pass++) {
        bool hidden = pass == 1;
        Bytes& scripts = files.at(re.getInt(hidden ? "HiddenItemsScriptOffset" : "ItemBallsScriptOffset"));
        int setVar = hidden ? hiddenItemSetVarCommand : normalItemSetVarCommand;
        int varSet = hidden ? hiddenItemVarSet : normalItemVarSet;
        int offset = 0, skipOffset = 0;
        while (true) {
            if (readWord(scripts, offset) == scriptListTerminator) break;
            int offsetInFile = readRelativePointer(scripts, offset);
            // (sic) al leer los ocultos FVX comprueba el límite antes de avanzar
            if (hidden && !isSet) {
                if (offsetInFile > static_cast<int>(scripts.size())) break;
                offset += 4;
            } else {
                offset += 4;
                if (offsetInFile > static_cast<int>(scripts.size())) break;
            }
            // (sic) en los ocultos se compara con la longitud de la tabla normal
            if (skipOffset < static_cast<int>(skipTable.size()) && (hidden ? skipTableH.at(skipOffset) : skipTable[skipOffset]) == offset / 4 - 1) {
                skipOffset++;
                continue;
            }
            int command = readWord(scripts, offsetInFile + 2);
            int variable = readWord(scripts, offsetInFile + 4);
            if (command == setVar && variable == varSet) visit(scripts, offsetInFile + 6);
        }
    }
}

std::vector<int> Gen5RomHandler::getFieldItemIds() const {
    std::vector<int> out;
    auto files = scriptNarc_.files;  // solo se lee
    walkFieldItems(files, *romEntry_, false, [&](Bytes& s, int o) { out.push_back(readWord(s, o)); });
    return out;
}

void Gen5RomHandler::setFieldItemIds(const std::vector<int>& fieldItems) {
    auto it = fieldItems.begin();
    walkFieldItems(scriptNarc_.files, *romEntry_, true, [&](Bytes& s, int o) {
        if (it == fieldItems.end()) throw std::runtime_error("NoSuchElementException");
        writeWord(s, o, *it++);
    });
}

std::vector<int> Gen5RomHandler::getFieldItems() const {
    std::vector<int> out;
    for (int id : getFieldItemIds())
        if (items_.at(id).value().allowed) out.push_back(id);
    return out;
}

void Gen5RomHandler::setFieldItems(const std::vector<int>& fieldItems) {
    // checkFieldItemsTMsReplaceTMs
    auto current = getFieldItems();
    for (size_t i = 0; i < current.size(); i++)
        if (items_.at(current[i])->tm != items_.at(fieldItems.at(i))->tm)
            throw std::invalid_argument("TMs must replace TMs, non-TMs must replace non-TMs");
    auto ids = getFieldItemIds();
    auto it = fieldItems.begin();
    for (auto& id : ids)
        if (items_.at(id)->allowed) id = *it++;
    setFieldItemIds(ids);
}

// ---------------- tiendas ----------------

std::vector<Shop> Gen5RomHandler::getShops() const {
    int shopCount = romEntry_->getInt("ShopCount");
    std::vector<Shop> shops;
    const auto& mainGame = getMainGameShops(romEntry_->romType());
    auto checkItem = [&](int id) {
        if (!items_.at(id)) throw std::runtime_error("objeto nulo en una tienda");  // NPE más tarde en Java
        return id;
    };
    if (romEntry_->romType() == Type_BW) {
        int overlayNum = romEntry_->getInt("ShopItemOvlNumber");
        int pointerTableOffset = romEntry_->getInt("ShopPointerTableOffset");
        int sizeTableOffset = romEntry_->getInt("ShopSizeTableOffset");
        Bytes ovl = readOverlay(overlayNum);
        for (int i = 0; i < shopCount; i++) {
            Shop shop;
            int itemsSize = static_cast<int8_t>(ovl.at(sizeTableOffset + i));
            // normalmente están en el overlay, pero si se han movido a la ARM9 se leen de allí
            int pointerOffset = pointerTableOffset + i * 4;
            const Bytes* source;
            int itemsOffset;
            if (isARM9Pointer(ovl, pointerOffset)) {
                itemsOffset = readARM9Pointer(ovl, pointerOffset);
                source = &arm9_;
            } else {
                itemsOffset = readLong(ovl, pointerOffset) - baseRom_->getOverlayAddress(overlayNum);
                source = &ovl;
            }
            for (int j = 0; j < itemsSize; j++) shop.items.push_back(checkItem(readWord(*source, itemsOffset + j * 2)));
            shop.name = std::string(shopNames_.at(i));
            shop.isMainGame = std::find(mainGame.begin(), mainGame.end(), i) != mainGame.end();
            shop.isSpecialShop = true;
            shops.push_back(shop);
        }
    } else {
        for (int i = 0; i < shopCount; i++) {
            Shop shop;
            const Bytes& data = shopNarc_.files.at(i);
            for (size_t j = 0; j < data.size(); j += 2) shop.items.push_back(checkItem(readWord(data, static_cast<int>(j))));
            shop.name = std::string(shopNames_.at(i));
            shop.isMainGame = std::find(mainGame.begin(), mainGame.end(), i) != mainGame.end();
            shop.isSpecialShop = true;
            shops.push_back(shop);
        }
    }
    for (int i : romEntry_->getArray("TMShops")) shops.at(i).isSpecialShop = false;
    for (int i : romEntry_->getArray("RegularShops")) shops.at(i).isSpecialShop = false;
    return shops;
}

void Gen5RomHandler::setShops(const std::vector<Shop>& shops) {
    int shopCount = romEntry_->getInt("ShopCount");
    if (static_cast<int>(shops.size()) != shopCount)
        throw std::invalid_argument("shops.size() must be: " + std::to_string(shopCount) + ", is: " + std::to_string(shops.size()));
    if (romEntry_->romType() == Type_BW) {
        int overlayNum = romEntry_->getInt("ShopItemOvlNumber");
        int pointerTableOffset = romEntry_->getInt("ShopPointerTableOffset");
        int sizeTableOffset = romEntry_->getInt("ShopSizeTableOffset");
        Bytes ovl = readOverlay(overlayNum);
        for (int i = 0; i < shopCount; i++) {
            const auto& shopItems = shops[i].items;
            int oldItemsSize = static_cast<int8_t>(ovl.at(sizeTableOffset + i));
            ovl.at(sizeTableOffset + i) = static_cast<uint8_t>(shopItems.size());
            // siempre se mueven a la ARM9/ITCM (los datos viejos del overlay quedan sin usar, como en FVX)
            int pointerOffset = pointerTableOffset + i * 4;
            if (isARM9Pointer(ovl, pointerOffset)) arm9FreedSpace_.free(readARM9Pointer(ovl, pointerOffset), oldItemsSize * 2);
            int itemsOffset = arm9FreedSpace_.findAndUnfree(static_cast<int>(shopItems.size()) * 2);
            writeARM9Pointer(ovl, pointerOffset, itemsOffset);
            for (size_t j = 0; j < shopItems.size(); j++) writeWord(arm9_, itemsOffset + static_cast<int>(j) * 2, shopItems[j]);
        }
        writeOverlay(overlayNum, ovl);
    } else {
        // los tamaños se guardan aparte (y repetidos) en otro archivo
        std::string sizesName = romEntry_->getFile("ShopSizes");
        int sizesOffset = romEntry_->getInt("ShopSizesOffset");
        Bytes sizesFile = readFile(sizesName);
        for (size_t i = 0; i < shops.size(); i++) sizesFile.at(i + sizesOffset) = static_cast<uint8_t>(shops[i].items.size());
        writeFile(sizesName, sizesFile);
        for (size_t i = 0; i < shops.size(); i++) {
            Bytes shop(shops[i].items.size() * 2, 0);
            for (size_t j = 0; j < shops[i].items.size(); j++) writeWord(shop, static_cast<int>(j * 2), shops[i].items[j]);
            shopNarc_.files.at(i) = std::move(shop);
        }
        writeNARC(romEntry_->getFile("ShopItems"), shopNarc_);
    }
}

std::vector<int> Gen5RomHandler::getShopPrices() const {
    std::vector<int> prices{0};
    auto narc = readNARC(romEntry_->getFile("ItemData"));
    for (size_t i = 1; i < narc.files.size(); i++) prices.push_back(readWord(narc.files[i], 0) * 10);
    // (FVX vuelve a escribir el NARC aquí sin cambios)
    const_cast<Gen5RomHandler*>(this)->writeNARC(romEntry_->getFile("ItemData"), narc);
    return prices;
}

// los precios se guardan en múltiplos de 10: la última cifra se pierde
void Gen5RomHandler::setShopPrices(const std::vector<int>& prices) {
    auto narc = readNARC(romEntry_->getFile("ItemData"));
    for (size_t i = 1; i < narc.files.size(); i++) writeWord(narc.files[i], 0, prices.at(i) / 10);
    writeNARC(romEntry_->getFile("ItemData"), narc);
}

void Gen5RomHandler::setBalancedShopPrices() {
    auto prices = getShopPrices();
    for (const auto& [k, v] : balancedItemPrices)
        if (k < static_cast<int>(prices.size())) prices[k] = v;
    setShopPrices(prices);
}

// ---------------- Recogida ----------------

std::vector<PickupItem> Gen5RomHandler::getPickupItems() {
    std::vector<PickupItem> out;
    Bytes ovl = readOverlay(romEntry_->getInt("PickupOvlNumber"));
    if (pickupItemsTableOffset_ == 0) {
        int offset = find(ovl, pickupTableLocator);
        if (offset > 0) pickupItemsTableOffset_ = offset;
    }
    if (pickupItemsTableOffset_ > 0) {
        for (int i = 0; i < numberOfPickupItems; i++) {
            int id = readWord(ovl, pickupItemsTableOffset_ + 2 * i);
            if (!items_.at(id)) throw std::runtime_error("objeto nulo en la Recogida");
            out.emplace_back(id);
        }
    }
    if (!out.empty()) {
        for (int lr = 0; lr < 10; lr++) {
            int rare = lr, common = 11 + lr;
            out.at(common).probabilities[lr] = 30;
            for (int i = 1; i < 7; i++) out.at(common + i).probabilities[lr] = 10;
            out.at(common + 7).probabilities[lr] = 4;
            out.at(common + 8).probabilities[lr] = 4;
            out.at(rare).probabilities[lr] = 1;
            out.at(rare + 1).probabilities[lr] = 1;
        }
    }
    return out;
}

void Gen5RomHandler::setPickupItems(const std::vector<PickupItem>& pickupItems) {
    if (pickupItemsTableOffset_ <= 0) return;
    int ovlNum = romEntry_->getInt("PickupOvlNumber");
    Bytes ovl = readOverlay(ovlNum);
    for (int i = 0; i < numberOfPickupItems; i++) writeWord(ovl, pickupItemsTableOffset_ + 2 * i, pickupItems.at(i).item);
    writeOverlay(ovlNum, ovl);
}

// ---------------- intercambios ----------------

std::vector<InGameTrade> Gen5RomHandler::getInGameTrades() const {
    std::vector<InGameTrade> trades;
    auto narc = readNARC(romEntry_->getFile("InGameTrades"));
    auto strings = getStrings(false, romEntry_->getInt("IngameTradesTextOffset"));
    const auto& unused = romEntry_->getArray("TradesUnused");
    size_t unusedOffset = 0;
    for (int entry = 0; entry < static_cast<int>(narc.files.size()); entry++) {
        if (unusedOffset < unused.size() && unused[unusedOffset] == entry) {
            unusedOffset++;
            continue;
        }
        const Bytes& f = narc.files[entry];
        InGameTrade t(pokes_.at(readLong(f, 0x5C)), pokes_.at(readLong(f, 4)));
        t.nickname = strings.at(entry * 2);
        t.ivs.resize(6);
        for (int iv = 0; iv < 6; iv++) t.ivs[iv] = readLong(f, 0x10 + iv * 4);
        t.otId = readWord(f, 0x34);
        int item = readLong(f, 0x4C);
        if (items_.at(item)) t.heldItem = item;
        t.otName = strings.at(entry * 2 + 1);
        trades.push_back(t);
    }
    return trades;
}

void Gen5RomHandler::setInGameTrades(const std::vector<InGameTrade>& trades) {
    size_t tradeOffset = 0;
    auto oldTrades = getInGameTrades();
    auto narc = readNARC(romEntry_->getFile("InGameTrades"));
    int textIdx = romEntry_->getInt("IngameTradesTextOffset");
    auto strings = getStrings(false, textIdx);
    const auto& unused = romEntry_->getArray("TradesUnused");
    size_t unusedOffset = 0;
    for (int i = 0; i < static_cast<int>(narc.files.size()); i++) {
        if (unusedOffset < unused.size() && unused[unusedOffset] == i) {
            unusedOffset++;
            continue;
        }
        Bytes& f = narc.files[i];
        const InGameTrade& t = trades.at(tradeOffset++);
        strings.at(i * 2) = t.nickname;
        strings.at(i * 2 + 1) = t.otName.value_or("null");
        writeLong(f, 4, t.givenSpecies()->number());
        writeLong(f, 8, 0);  // sin forma
        for (int iv = 0; iv < 6; iv++) writeLong(f, 0x10 + iv * 4, t.ivs.at(iv));
        writeLong(f, 0x2C, 0xFF);  // naturaleza aleatoria
        writeWord(f, 0x34, t.otId);
        writeLong(f, 0x4C, t.heldItem.value_or(0));
        writeLong(f, 0x5C, t.requestedSpecies->number());
        const auto& scripts = romEntry_->getTradeScripts();
        if (!scripts.empty()) {
            const TradeScript& ts = scripts.at(i - unusedOffset);
            for (size_t k = 0; k < ts.requestedOffsets.size(); k++) {
                Bytes& file = scriptNarc_.files.at(ts.fileNum);
                writeWord(file, ts.requestedOffsets[k], t.requestedSpecies->number());
                writeWord(file, ts.givenOffsets[k], t.givenSpecies()->number());
            }
        }
    }
    writeNARC(romEntry_->getFile("InGameTrades"), narc);
    setStrings(false, textIdx, strings);
    // lo que dice cada persona del intercambio
    unusedOffset = 0;
    const auto& textOffsets = romEntry_->getArray("IngameTradePersonTextOffsets");
    for (int tr = 0; tr < static_cast<int>(textOffsets.size()); tr++) {
        if (unusedOffset < unused.size() && unused[unusedOffset] == tr + 24) {
            unusedOffset++;
            continue;
        }
        if (textOffsets[tr] > 0) {
            if (tr + 24 >= static_cast<int>(oldTrades.size()) || tr + 24 >= static_cast<int>(trades.size())) break;
            const InGameTrade& o = oldTrades[tr + 24];
            const InGameTrade& n = trades[tr + 24];
            std::map<std::string, std::string> reps;  // TreeMap
            reps[o.givenSpecies()->name()] = n.givenSpecies()->name();
            if (o.requestedSpecies != n.requestedSpecies) reps[o.requestedSpecies->name()] = n.requestedSpecies->name();
            replaceAllStringsInEntry(textOffsets[tr], reps);
        }
    }
}

namespace {
// String.replaceAll(regex, replacement) para nombres de Pokémon: solo «.» tiene significado especial
// (cualquier carácter salvo saltos de línea); cualquier otro metacarácter se rechaza para no fallar en silencio.
std::u16string javaReplaceAll(const std::u16string& s, const std::u16string& pattern, const std::u16string& repl) {
    for (char16_t c : pattern)
        if (std::u16string_view(u"\\[](){}*+?^$|").find(c) != std::u16string_view::npos)
            throw std::logic_error("replaceAll: metacarácter de regex no admitido en un nombre");
    for (char16_t c : repl)
        if (c == u'$' || c == u'\\') throw std::logic_error("replaceAll: carácter especial en el reemplazo");
    if (pattern.empty()) throw std::logic_error("replaceAll: patrón vacío");
    auto isLineTerm = [](char16_t c) { return c == u'\n' || c == u'\r' || c == 0x85 || c == 0x2028 || c == 0x2029; };
    std::u16string out;
    size_t i = 0;
    while (i < s.size()) {
        bool match = i + pattern.size() <= s.size();
        for (size_t k = 0; match && k < pattern.size(); k++) {
            char16_t p = pattern[k], c = s[i + k];
            match = p == u'.' ? !isLineTerm(c) : p == c;
        }
        if (match) {
            out += repl;
            i += pattern.size();
        } else {
            out.push_back(s[i++]);
        }
    }
    return out;
}
}  // namespace

void Gen5RomHandler::replaceAllStringsInEntry(int entry, const std::map<std::string, std::string>& replacements) {
    auto strings = getStrings(true, entry);
    for (auto& str : strings) {
        std::u16string ns = text5::toU16(str);
        for (const auto& [k, v] : replacements) ns = javaReplaceAll(ns, text5::toU16(k), text5::toU16(v));
        str = text5::fromU16(ns);
    }
    setStrings(true, entry, strings);
}

// ---------------- objetos para entrenadores ----------------

std::vector<std::optional<int>> Gen5RomHandler::getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                                         const std::array<int, 4>& pokeMoves) const {
    std::vector<int> ids = generalPurposeConsumableItems5();
    int boost = 6;  // algunos objetos muy buenos, más frecuentes
    if (!consumableOnly) {
        boost = 8;
        const auto& g = generalPurposeItems5();
        ids.insert(ids.end(), g.begin(), g.end());
    }
    for (int idx : pokeMoves) {
        const auto& move = moves_.at(idx);
        if (!move) continue;
        if (move->category == MoveCategory::PHYSICAL) {
            ids.push_back(items::liechiBerry);
            ids.push_back(consumableTypeBoostingItem(move->type));
            if (!consumableOnly) {
                auto b = gen4::typeBoostingItems(move->type);
                ids.insert(ids.end(), b.begin(), b.end());
                ids.push_back(items::choiceBand);
                ids.push_back(items::muscleBand);
            }
        }
        if (move->category == MoveCategory::SPECIAL) {
            ids.push_back(items::petayaBerry);
            ids.push_back(consumableTypeBoostingItem(move->type));
            if (!consumableOnly) {
                auto b = gen4::typeBoostingItems(move->type);
                ids.insert(ids.end(), b.begin(), b.end());
                ids.push_back(items::wiseGlasses);
                ids.push_back(items::choiceSpecs);
            }
        }
        if (!consumableOnly) {
            auto mb = moveBoostingItems5(idx);
            ids.insert(ids.end(), mb.begin(), mb.end());
        }
    }
    // En Java es un HashMap<Type, ...> (orden según el hash de identidad del enum, distinto en cada
    // ejecución de FVX); aquí se sigue el orden del enum.
    const Species* sp = tp.species();
    auto byType = getTypeTable().against(*sp->primaryType(false), sp->secondaryType(false));
    for (const auto& [t, e] : byType) {
        if (e == Effectiveness::DOUBLE) ids.push_back(gen4::weaknessReducingBerry(t));
        else if (e == Effectiveness::QUADRUPLE)
            for (int i = 0; i < boost; i++) ids.push_back(gen4::weaknessReducingBerry(t));
    }
    auto eff = [&](Type t) -> std::optional<Effectiveness> {
        auto it = byType.find(t);
        return it == byType.end() ? std::nullopt : std::optional<Effectiveness>(it->second);
    };
    if (eff(Type::NORMAL) == Effectiveness::NEUTRAL) ids.push_back(items::chilanBerry);
    int ability = getAbilityForTrainerPokemon(tp);
    if (ability == abilities::levitate) {
        auto it = std::find(ids.begin(), ids.end(), items::shucaBerry);
        if (it != ids.end()) ids.erase(it);
    } else if (eff(Type::GROUND) == Effectiveness::DOUBLE || eff(Type::GROUND) == Effectiveness::QUADRUPLE) {
        ids.push_back(items::airBalloon);
    }
    if (!consumableOnly) {
        if (ability == abilities::guts) ids.insert(ids.end(), {items::flameOrb, items::toxicOrb});
        if (ability == abilities::magicGuard) ids.insert(ids.end(), {items::stickyBarb, items::lifeOrb});
        if (sp->primaryType(false) == Type::POISON || sp->secondaryType(false) == Type::POISON) ids.push_back(items::blackSludge);
        auto si = gen4::speciesBoostingItems(sp->number());
        if (!si.empty())
            for (int i = 0; i < boost; i++) ids.insert(ids.end(), si.begin(), si.end());
        if (!sp->evolutionsFrom().empty() && tp.level >= 20) ids.push_back(items::eviolite);  // nivel mínimo, como el veto a Superguarda
    }
    std::vector<std::optional<int>> out;
    for (int id : ids) out.push_back(items_.at(id) ? std::optional<int>(id) : std::nullopt);
    return out;
}

}  // namespace pr::gen5
