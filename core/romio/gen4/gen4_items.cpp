// Objetos de Gen 4: conjuntos de objetos, objetos de campo, tiendas, precios, Recogida, intercambios y
// objetos equipables «sensatos» para entrenadores (port de Gen4RomHandler y Gen4Constants).
#include <algorithm>
#include <stdexcept>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "util/java_compat.hpp"

namespace pr::gen4 {

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

}  // namespace

// tablas de Gen4Constants que también usa Gen 5 (declaradas en gen4_constants.hpp)
std::vector<int> allHeldItems() {
    using namespace items;
    std::vector<int> l = {brightPowder, quickClaw, choiceBand, kingsRock, silverPowder, focusBand, scopeLens, metalCoat,
                          leftovers, softSand, hardStone, miracleSeed, blackGlasses, blackBelt, magnet, mysticWater,
                          sharpBeak, poisonBarb, neverMeltIce, spellTag, twistedSpoon, charcoal, dragonFang, silkScarf,
                          shellBell, seaIncense, laxIncense, wideLens, muscleBand, wiseGlasses, expertBelt, lightClay,
                          lifeOrb, toxicOrb, flameOrb, zoomLens, metronome, ironBall, laggingTail, destinyKnot,
                          blackSludge, icyRock, smoothRock, heatRock, dampRock, gripClaw, choiceScarf, stickyBarb,
                          shedShell, bigRoot, choiceSpecs, flamePlate, splashPlate, zapPlate, meadowPlate, iciclePlate,
                          fistPlate, toxicPlate, earthPlate, skyPlate, mindPlate, insectPlate, stonePlate, spookyPlate,
                          dracoPlate, dreadPlate, ironPlate, oddIncense, rockIncense, fullIncense, waveIncense,
                          roseIncense, razorClaw, razorFang};
    l.insert(l.end(), consumableHeldItems.begin(), consumableHeldItems.end());
    return l;
}

// Gen4Constants.typeBoostingItems (null -> lista vacía)
std::vector<int> typeBoostingItems(std::optional<Type> t) {
    using namespace items;
    if (!t) return {};
    switch (*t) {
        case Type::BUG: return {silverPowder, insectPlate};
        case Type::DARK: return {blackGlasses, dreadPlate};
        case Type::DRAGON: return {dragonFang, dracoPlate};
        case Type::ELECTRIC: return {magnet, zapPlate};
        case Type::FIGHTING: return {blackBelt, fistPlate};
        case Type::FIRE: return {charcoal, flamePlate};
        case Type::FLYING: return {sharpBeak, skyPlate};
        case Type::GHOST: return {spellTag, spookyPlate};
        case Type::GRASS: return {miracleSeed, meadowPlate, roseIncense};
        case Type::GROUND: return {softSand, earthPlate};
        case Type::ICE: return {neverMeltIce, iciclePlate};
        case Type::NORMAL: return {silkScarf};
        case Type::POISON: return {poisonBarb, toxicPlate};
        case Type::PSYCHIC: return {twistedSpoon, mindPlate, oddIncense};
        case Type::ROCK: return {hardStone, stonePlate, rockIncense};
        case Type::STEEL: return {metalCoat, ironPlate};
        case Type::WATER: return {mysticWater, seaIncense, splashPlate, waveIncense};
        default: throw std::logic_error("typeBoostingItems: tipo sin entrada");  // NPE en Java
    }
}

const std::vector<int>* moveBoostingItems(int move) {
    using namespace items;
    static const std::vector<int> powerHerbL{powerHerb}, fling{toxicOrb, flameOrb, ironBall},
        trick{toxicOrb, flameOrb, fullIncense, laggingTail}, ironBallL{ironBall}, orbs{toxicOrb, flameOrb},
        clay{lightClay}, icy{icyRock}, smooth{smoothRock}, heat{heatRock}, damp{dampRock}, grip{gripClaw}, root{bigRoot};
    switch (move) {
        case moves::bounce: case moves::dig: case moves::dive: case moves::fly: case moves::razorWind:
        case moves::skullBash: case moves::skyAttack: case moves::solarBeam: return &powerHerbL;
        case moves::fling: return &fling;
        case moves::trick: case moves::switcheroo: return &trick;
        case moves::trickRoom: return &ironBallL;
        case moves::facade: case moves::psychoShift: return &orbs;
        case moves::lightScreen: case moves::reflect: return &clay;
        case moves::hail: return &icy;
        case moves::sandstorm: return &smooth;
        case moves::sunnyDay: return &heat;
        case moves::rainDance: return &damp;
        case moves::bind: case moves::clamp: case moves::fireSpin: case moves::magmaStorm: case moves::outrage:
        case moves::sandTomb: case moves::uproar: case moves::whirlpool: case moves::wrap: return &grip;
        case moves::absorb: case moves::aquaRing: case moves::drainPunch: case moves::dreamEater: case moves::gigaDrain:
        case moves::ingrain: case moves::leechLife: case moves::leechSeed: case moves::megaDrain: return &root;
        default: return nullptr;
    }
}

int weaknessReducingBerry(Type t) {
    using namespace items;
    switch (t) {
        case Type::FIRE: return occaBerry;
        case Type::WATER: return passhoBerry;
        case Type::ELECTRIC: return wacanBerry;
        case Type::GRASS: return rindoBerry;
        case Type::ICE: return yacheBerry;
        case Type::FIGHTING: return chopleBerry;
        case Type::POISON: return kebiaBerry;
        case Type::GROUND: return shucaBerry;
        case Type::FLYING: return cobaBerry;
        case Type::PSYCHIC: return payapaBerry;
        case Type::BUG: return tangaBerry;
        case Type::ROCK: return chartiBerry;
        case Type::GHOST: return kasibBerry;
        case Type::DRAGON: return habanBerry;
        case Type::DARK: return colburBerry;
        case Type::STEEL: return babiriBerry;
        case Type::NORMAL: return chilanBerry;
        default: throw std::logic_error("weaknessReducingBerries: tipo sin entrada");  // null en Java
    }
}

std::vector<int> speciesBoostingItems(int n) {
    using namespace items;
    switch (n) {
        case species::dialga: return {adamantOrb};
        case species::palkia: return {lustrousOrb};
        case species::latias:
        case species::latios: return {soulDew};
        case species::clamperl: return {deepSeaTooth, deepSeaScale};
        case species::pikachu: return {lightBall};
        case species::chansey: return {luckyPunch};
        case species::ditto: return {metalPowder, quickPowder};
        case species::cubone:
        case species::marowak: return {thickClub};
        case species::farfetchd: return {leek};
        default: return {};
    }
}

namespace {
const std::vector<std::string>& specialShopNames(int romType) {
    static const std::vector<std::string> dp = {
        "Jubilife Secondary", "Oreburgh Secondary", "Floaroma Secondary", "Eterna Secondary", "Eterna Herbs",
        "Hearthome Secondary", "Solaceon Secondary", "Pastoria Secondary", "Veilstone Department Store 1F (Right)",
        "Veilstone Department Store 1F (Left)", "Veilstone Department Store 2F (Top)",
        "Veilstone Department Store 2F (Middle)", "Veilstone Department Store 3F (Top)",
        "Veilstone Department Store 3F (Bottom)", "Celestic Secondary", "Snowpoint Secondary", "Canalave Secondary",
        "Sunyshore Secondary", "Pokemon League Secondary"};
    static const std::vector<std::string> pt = [] {
        auto l = dp;
        l.push_back("Veilstone Department Store B1F Berries");
        return l;
    }();
    static const std::vector<std::string> hgss = {
        "Cherrygrove Secondary", "Violet Secondary", "Azalea Secondary", "Goldenrod Department Store 2F (Lower)",
        "Goldenrod Department Store 2F (Upper)", "Goldenrod Department Store 3F", "Goldenrod Department Store 4F",
        "Goldenrod Department Store 5F", "Goldenrod Herb Shop", "Ecruteak Secondary", "Olivine Secondary",
        "Cianwood Pharmacy", "Blackthorn Secondary", "Unused Secondary", "Safari Zone Gate Southwest",
        "Saffron Secondary", "Lavender Secondary", "Cerulean Secondary", "Celadon Department Store 2F (Left)",
        "Celadon Department Store 2F (Right)", "Celadon Department Store 3F", "Celadon Department Store 4F",
        "Celadon Department Store 5F (Left)", "Celadon Department Store 5F (Right)", "Fuchsia Secondary",
        "Pewter Secondary", "Viridian Secondary", "Mt. Moon Square", "Mahogany Before Hideout",
        "Mahogany After Hideout"};
    if (romType == Type_DP) return dp;
    if (romType == Type_Plat) return pt;
    if (romType == Type_HGSS) return hgss;
    throw std::invalid_argument("Invalid RomType");
}

std::vector<int> mainGameShops(int romType) {
    std::vector<int> dp;
    for (int i = 0; i < 20; i++) dp.push_back(i);
    if (romType == Type_DP) return dp;
    if (romType == Type_Plat) {
        dp.push_back(20);
        return dp;
    }
    if (romType == Type_HGSS) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 30};
    throw std::invalid_argument("Invalid RomType");
}
}  // namespace

// ---------------- punteros de la ARM9 (AbstractDSRomHandler) ----------------

int Gen4RomHandler::itcmSrcOffset() const {
    int p = tcmCopyingPointersOffset_ + 8;
    return readLong(arm9_, p) - gen4::arm9Offset;
}

int Gen4RomHandler::readARM9Pointer(int offset) const {
    int pointer = readLong(arm9_, offset);
    if (pointer <= ITCM_END) return pointer % ITCM_LENGTH + itcmSrcOffset();
    if (pointer >= gen4::arm9Offset && pointer < gen4::arm9Offset + static_cast<int>(arm9_.size()))
        return pointer - gen4::arm9Offset;
    throw std::runtime_error("Invalid pointer! Not a RAM location for either ARM9 or ITCM");
}

void Gen4RomHandler::writeARM9Pointer(int offset, int pointer) {
    if (tcmCopyingPointersOffset_ == -1) throw std::logic_error("tcmCopyingPointersOffset has not been initialized");
    int itcmSizeOffset = readLong(arm9_, tcmCopyingPointersOffset_) - gen4::arm9Offset + 4;
    int itcmSize = readLong(arm9_, itcmSizeOffset);
    int src = itcmSrcOffset();
    if (pointer >= src && pointer < src + itcmSize) writeLong(arm9_, offset, pointer - src + ITCM_RAM_ADDRESS);
    else if (pointer < static_cast<int>(arm9_.size())) writeLong(arm9_, offset, pointer + gen4::arm9Offset);
    else throw std::runtime_error("Invalid pointer! Not an offset in the ARM9.bin file");
}

// ---------------- conjuntos de objetos ----------------

ItemSet Gen4RomHandler::itemIdsToSet(const std::vector<int>& ids) const {
    ItemSet s;
    for (int id : ids)
        if (id < static_cast<int>(items_.size()) && items_[id]) s.add(id);
    return s;
}

ItemSet Gen4RomHandler::getEvolutionItems() const { return itemIdsToSet(evolutionItems); }
ItemSet Gen4RomHandler::getOPShopItems() const { return itemIdsToSet(opShopItems().toVector()); }
ItemSet Gen4RomHandler::getAllConsumableHeldItems() const { return itemIdsToSet(consumableHeldItems); }
ItemSet Gen4RomHandler::getAllHeldItems() const { return itemIdsToSet(allHeldItems()); }

ItemSet Gen4RomHandler::getRequiredFieldTMs() const {
    if (romEntry_->romType() == Type_DP) return itemIdsToSet(dpRequiredFieldTMs);
    if (romEntry_->romType() == Type_Plat) return itemIdsToSet(ptRequiredFieldTMs);
    return {};
}

// ---------------- objetos del suelo ----------------

std::vector<int> Gen4RomHandler::getFieldItemIds() const {
    std::vector<int> out;
    const Bytes& scripts = scriptNarc_.files.at(romEntry_->getInt("ItemBallsScriptOffset"));
    const std::vector<int>& skip = romEntry_->getArray("ItemBallsSkip");
    int setVar = romEntry_->romType() == Type_HGSS ? hgssSetVarScript : dpptSetVarScript;
    size_t skipIdx = 0;
    for (int offset = 0;;) {
        if (readWord(scripts, offset) == scriptListTerminator) break;
        int inFile = readRelativePointer(scripts, offset);
        offset += 4;
        if (skipIdx < skip.size() && skip[skipIdx] == offset / 4 - 1) {
            skipIdx++;
            continue;
        }
        if (readWord(scripts, inFile) == setVar && readWord(scripts, inFile + 2) == itemScriptVariable)
            out.push_back(readWord(scripts, inFile + 4));
    }
    int hiTable = romEntry_->getInt("HiddenItemTableOffset");
    int hiCount = romEntry_->getInt("HiddenItemCount");
    for (int i = 0; i < hiCount; i++) out.push_back(readWord(arm9_, hiTable + i * 8));
    return out;
}

void Gen4RomHandler::writeFieldItemsIds(const std::vector<int>& fieldItems) {
    auto it = fieldItems.begin();
    Bytes& scripts = scriptNarc_.files.at(romEntry_->getInt("ItemBallsScriptOffset"));
    const std::vector<int>& skip = romEntry_->getArray("ItemBallsSkip");
    int setVar = romEntry_->romType() == Type_HGSS ? hgssSetVarScript : dpptSetVarScript;
    size_t skipIdx = 0;
    for (int offset = 0;;) {
        if (readWord(scripts, offset) == scriptListTerminator) break;
        int inFile = readRelativePointer(scripts, offset);
        offset += 4;
        if (skipIdx < skip.size() && skip[skipIdx] == offset / 4 - 1) {
            skipIdx++;
            continue;
        }
        if (readWord(scripts, inFile) == setVar && readWord(scripts, inFile + 2) == itemScriptVariable)
            writeWord(scripts, inFile + 4, *it++);
    }
    int hiTable = romEntry_->getInt("HiddenItemTableOffset");
    int hiCount = romEntry_->getInt("HiddenItemCount");
    for (int i = 0; i < hiCount; i++) writeWord(arm9_, hiTable + i * 8, *it++);
}

std::vector<int> Gen4RomHandler::getFieldItems() const {
    std::vector<int> out;
    for (int id : getFieldItemIds())
        if (items_.at(id).value().allowed) out.push_back(id);
    return out;
}

void Gen4RomHandler::setFieldItems(const std::vector<int>& fieldItems) {
    // checkFieldItemsTMsReplaceTMs
    auto current = getFieldItems();
    for (size_t i = 0; i < current.size(); i++)
        if (items_.at(current[i])->tm != items_.at(fieldItems.at(i))->tm)
            throw std::invalid_argument("TMs must replace TMs, non-TMs must replace non-TMs");
    auto ids = getFieldItemIds();
    auto it = fieldItems.begin();
    for (auto& id : ids)
        if (items_.at(id)->allowed) id = *it++;
    writeFieldItemsIds(ids);
}

// ---------------- tiendas ----------------

bool Gen4RomHandler::hasShopSupport() const { return true; }
bool Gen4RomHandler::canChangeShopSizes() const { return true; }

std::vector<Shop> Gen4RomHandler::getShops() const {
    std::vector<Shop> shops;
    auto checkItem = [&](int id) {
        if (id == 0 || id >= static_cast<int>(items_.size())) throw std::runtime_error("Invalid item in shop.");
        return id;
    };
    // tienda progresiva (gana objetos con las medallas)
    {
        int offset = readARM9Pointer(romEntry_->getInt("ProgressiveShopPointerOffset"));
        int size = arm9_.at(romEntry_->getInt("ProgressiveShopSizeOffset"));
        Shop shop;
        for (int i = 0; i < size; i++) shop.items.push_back(checkItem(readWord(arm9_, offset + i * 4)));
        shop.name = "Progressive";
        shop.isMainGame = true;
        shop.isSpecialShop = false;
        shops.push_back(shop);
    }
    // tiendas especiales
    const auto& names = specialShopNames(romEntry_->romType());
    auto mainGame = mainGameShops(romEntry_->romType());
    int table = readARM9Pointer(romEntry_->getInt("SpecialShopsPointerOffset"));
    int count = romEntry_->getInt("SpecialShopCount");
    for (int i = 0; i < count; i++) {
        int offset = readARM9Pointer(table + 4 * i);
        Shop shop;
        for (int id = readWord(arm9_, offset); id != 0xFFFF; id = readWord(arm9_, offset)) {
            shop.items.push_back(checkItem(id));
            offset += 2;
        }
        shop.name = names.at(i);
        shop.isMainGame = std::find(mainGame.begin(), mainGame.end(), i) != mainGame.end();
        shop.isSpecialShop = true;
        shops.push_back(shop);
    }
    for (int i : romEntry_->getArray("TMShops")) shops.at(i).isSpecialShop = false;
    return shops;
}

void Gen4RomHandler::setShops(const std::vector<Shop>& shops) {
    auto checkItem = [&](int id) {
        if (id == 0 || id >= static_cast<int>(items_.size())) throw std::runtime_error("Invalid item to write.");
        return id;
    };
    // progresiva: los objetos que ya estaban conservan su requisito de medallas
    {
        int pointerOffset = romEntry_->getInt("ProgressiveShopPointerOffset");
        int sizeOffset = romEntry_->getInt("ProgressiveShopSizeOffset");
        int offset = readARM9Pointer(pointerOffset);
        int size = arm9_.at(sizeOffset);
        std::map<int, int> values;  // HashMap en Java; solo se consulta
        for (int i = 0; i < size; i++) values[readWord(arm9_, offset + i * 4)] = readWord(arm9_, offset + i * 4 + 2);
        const auto& items = shops.front().items;
        if (static_cast<int>(items.size()) != size) {
            arm9FreedSpace_.free(offset, size * 4);
            offset = arm9FreedSpace_.findAndUnfree(static_cast<int>(items.size()) * 4);
            if (offset < 0) throw std::runtime_error("No hay espacio libre en la ARM9 para la tienda progresiva");
            writeARM9Pointer(pointerOffset, offset);
            arm9_.at(sizeOffset) = static_cast<uint8_t>(items.size());
        }
        for (int id : items) {
            writeWord(arm9_, offset, checkItem(id));
            offset += 2;
            auto v = values.find(id);
            writeWord(arm9_, offset, v == values.end() ? 1 : v->second);
            offset += 2;
        }
    }
    // especiales: se liberan todas y se vuelven a colocar
    int table = readARM9Pointer(romEntry_->getInt("SpecialShopsPointerOffset"));
    int count = romEntry_->getInt("SpecialShopCount");
    for (int i = 0; i < count; i++) {
        int offset = readARM9Pointer(table + 4 * i);
        int size = 0;
        while (readWord(arm9_, offset + size * 2) != 0xFFFF) size++;
        arm9FreedSpace_.free(offset, (size + 1) * 2);
    }
    for (int i = 0; i < count; i++) {
        const auto& items = shops.at(i + 1).items;
        int offset = arm9FreedSpace_.findAndUnfree((static_cast<int>(items.size()) + 1) * 2);
        if (offset < 0) throw std::runtime_error("No hay espacio libre en la ARM9 para las tiendas");
        writeARM9Pointer(table + 4 * i, offset);
        for (int id : items) {
            writeWord(arm9_, offset, checkItem(id));
            offset += 2;
        }
        writeWord(arm9_, offset, 0xFFFF);
    }
}

std::vector<int> Gen4RomHandler::getShopPrices() const {
    std::vector<int> prices{0};
    // en DP los id 112-134 no se usan; en Pt y HGSS el 112 es el Griseous Orb
    int startOfUnused = romEntry_->romType() == Type_DP ? 112 : 113;
    auto narc = readNARC(romEntry_->getFile("ItemData"));
    for (int i = 1; i < static_cast<int>(narc.files.size()); i++) {
        if (i == startOfUnused)
            for (int j = startOfUnused; j < items::adamantOrb; j++) prices.push_back(0);
        prices.push_back(readWord(narc.files[i], 0));
    }
    if (romEntry_->romType() == Type_HGSS) prices.push_back(0);  // Enigma Stone (como FVX)
    return prices;
}

void Gen4RomHandler::setShopPrices(const std::vector<int>& prices) {
    int startOfUnused = romEntry_->romType() == Type_DP ? 112 : 113;
    auto narc = readNARC(romEntry_->getFile("ItemData"));
    int itemID = 1;
    for (int i = 1; i < static_cast<int>(narc.files.size()); i++) {
        writeWord(narc.files[i], 0, prices.at(itemID));
        itemID++;
        if (itemID == startOfUnused) itemID = 135;
    }
    writeNARC(romEntry_->getFile("ItemData"), narc);
}

void Gen4RomHandler::setBalancedShopPrices() {
    auto prices = getShopPrices();
    // HashMap<Integer, Integer>: el orden no importa, cada clave se escribe una vez
    for (const auto& [k, v] : balancedItemPrices)
        if (k < static_cast<int>(prices.size())) prices[k] = v;
    setShopPrices(prices);
}

// ---------------- Recogida ----------------

std::vector<PickupItem> Gen4RomHandler::getPickupItems() {
    std::vector<PickupItem> out;
    Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
    if (pickupItemsTableOffset_ == 0) {
        int o = find(ovl, std::string(pickupTableLocator));
        if (o > 0) pickupItemsTableOffset_ = o;
    }
    if (rarePickupItemsTableOffset_ == 0) {
        int o = find(ovl, std::string(rarePickupTableLocator));
        if (o > 0) rarePickupItemsTableOffset_ = o;
    }
    if (pickupItemsTableOffset_ > 0 && rarePickupItemsTableOffset_ > 0) {
        for (int i = 0; i < numberOfCommonPickupItems; i++) out.emplace_back(readWord(ovl, pickupItemsTableOffset_ + 2 * i));
        for (int i = 0; i < numberOfRarePickupItems; i++) out.emplace_back(readWord(ovl, rarePickupItemsTableOffset_ + 2 * i));
    }
    if (!out.empty()) {
        for (int lr = 0; lr < 10; lr++) {
            int common = lr, rare = 18 + lr;
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

void Gen4RomHandler::setPickupItems(const std::vector<PickupItem>& items) {
    if (pickupItemsTableOffset_ <= 0 || rarePickupItemsTableOffset_ <= 0) return;
    Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
    auto it = items.begin();
    for (int i = 0; i < numberOfCommonPickupItems; i++) writeWord(ovl, pickupItemsTableOffset_ + 2 * i, (it++)->item);
    for (int i = 0; i < numberOfRarePickupItems; i++) writeWord(ovl, rarePickupItemsTableOffset_ + 2 * i, (it++)->item);
    writeOverlay(romEntry_->getInt("BattleOvlNumber"), ovl);
}

// ---------------- intercambios ----------------

std::vector<InGameTrade> Gen4RomHandler::getInGameTrades() const {
    std::vector<InGameTrade> trades;
    auto narc = readNARC(romEntry_->getFile("InGameTrades"));
    const auto& sp = romEntry_->getArray("StaticPokemonTrades");
    auto strings = getStrings(romEntry_->getInt("IngameTradesTextOffset"));
    int count = static_cast<int>(narc.files.size());
    for (int i = 0; i < count; i++) {
        if (std::find(sp.begin(), sp.end(), i) != sp.end()) continue;
        const Bytes& f = narc.files[i];
        InGameTrade t(pokes_.at(readLong(f, 0x4C)), pokes_.at(readLong(f, 0)));
        t.nickname = strings.at(i);
        t.ivs.resize(6);
        for (int iv = 0; iv < 6; iv++) t.ivs[iv] = readLong(f, 4 + iv * 4);
        t.otId = readWord(f, 0x20);
        t.otName = strings.at(i + count);
        int item = readLong(f, 0x3C);
        if (items_.at(item)) t.heldItem = item;
        trades.push_back(t);
    }
    return trades;
}

void Gen4RomHandler::setInGameTrades(const std::vector<InGameTrade>& trades) {
    auto oldTrades = getInGameTrades();
    auto narc = readNARC(romEntry_->getFile("InGameTrades"));
    const auto& sp = romEntry_->getArray("StaticPokemonTrades");
    int textIdx = romEntry_->getInt("IngameTradesTextOffset");
    auto strings = getStrings(textIdx);
    int count = static_cast<int>(narc.files.size());
    size_t tradeOffset = 0;
    for (int i = 0; i < count; i++) {
        if (std::find(sp.begin(), sp.end(), i) != sp.end()) continue;
        Bytes& f = narc.files[i];
        const InGameTrade& t = trades.at(tradeOffset++);
        strings.at(i) = t.nickname;
        strings.at(i + count) = t.otName.value_or("");
        writeLong(f, 0, t.givenSpecies()->number());
        for (int iv = 0; iv < 6; iv++) writeLong(f, 4 + iv * 4, t.ivs.at(iv));
        writeWord(f, 0x20, t.otId);
        writeLong(f, 0x3C, t.heldItem ? *t.heldItem : 0);
        writeLong(f, 0x4C, t.requestedSpecies->number());
        if (f.size() > 0x50) writeLong(f, 0x50, 0);  // sin género fijo
    }
    writeNARC(romEntry_->getFile("InGameTrades"), narc);
    setStrings(textIdx, strings);
    // lo que dice cada persona del intercambio
    const auto& textOffsets = romEntry_->getArray("IngameTradePersonTextOffsets");
    for (size_t tr = 0; tr < textOffsets.size(); tr++) {
        if (textOffsets[tr] <= 0) continue;
        if (tr >= oldTrades.size() || tr >= trades.size()) break;
        const InGameTrade& o = oldTrades[tr];
        const InGameTrade& n = trades[tr];
        std::map<std::string, std::string> reps;
        reps[jc::toUpperCase(o.givenSpecies()->name())] = n.givenSpecies()->name();
        if (o.requestedSpecies != n.requestedSpecies)
            reps[jc::toUpperCase(o.requestedSpecies->name())] = n.requestedSpecies->name();
        replaceAllStringsInEntry(textOffsets[tr], reps);
        if (romEntry_->romType() == Type_HGSS && tr == 6) replaceAllStringsInEntry(textOffsets[tr] + 1, reps);
    }
}

// ---------------- objetos para entrenadores ----------------

std::vector<std::optional<int>> Gen4RomHandler::getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                                         const std::array<int, 4>& pokeMoves) const {
    std::vector<int> ids(generalPurposeConsumableItems.begin(), generalPurposeConsumableItems.end());
    int boost = 6;  // algunos objetos muy buenos, más frecuentes
    if (!consumableOnly) {
        boost = 8;
        ids.insert(ids.end(), generalPurposeItems.begin(), generalPurposeItems.end());
    }
    for (int idx : pokeMoves) {
        const auto& move = moves_.at(idx);
        if (!move) continue;
        if (move->category == MoveCategory::PHYSICAL) {
            ids.push_back(items::liechiBerry);
            if (!consumableOnly) {
                auto b = typeBoostingItems(move->type);
                ids.insert(ids.end(), b.begin(), b.end());
                ids.push_back(items::choiceBand);
                ids.push_back(items::muscleBand);
            }
        }
        if (move->category == MoveCategory::SPECIAL) {
            ids.push_back(items::petayaBerry);
            if (!consumableOnly) {
                auto b = typeBoostingItems(move->type);
                ids.insert(ids.end(), b.begin(), b.end());
                ids.push_back(items::wiseGlasses);
                ids.push_back(items::choiceSpecs);
            }
        }
        if (!consumableOnly)
            if (const auto* mb = moveBoostingItems(idx)) ids.insert(ids.end(), mb->begin(), mb->end());
    }
    // En Java es un HashMap<Type, ...> (orden según el hash de identidad del enum, distinto en cada
    // ejecución de FVX); aquí se sigue el orden del enum.
    const Species* sp = tp.species();
    auto byType = getTypeTable().against(*sp->primaryType(false), sp->secondaryType(false));
    for (const auto& [t, e] : byType) {
        if (e == Effectiveness::DOUBLE) ids.push_back(weaknessReducingBerry(t));
        else if (e == Effectiveness::QUADRUPLE)
            for (int i = 0; i < boost; i++) ids.push_back(weaknessReducingBerry(t));
    }
    auto normal = byType.find(Type::NORMAL);
    if (normal != byType.end() && normal->second == Effectiveness::NEUTRAL) ids.push_back(items::chilanBerry);

    int ability = getAbilityForTrainerPokemon(tp);
    if (ability == abilities::levitate) {
        auto it = std::find(ids.begin(), ids.end(), items::shucaBerry);
        if (it != ids.end()) ids.erase(it);
    }
    if (!consumableOnly) {
        if (ability == abilities::guts) ids.insert(ids.end(), {items::flameOrb, items::toxicOrb});
        if (ability == abilities::magicGuard) ids.insert(ids.end(), {items::stickyBarb, items::lifeOrb});
        if (sp->primaryType(false) == Type::POISON || sp->secondaryType(false) == Type::POISON)
            ids.push_back(items::blackSludge);
        auto si = speciesBoostingItems(sp->number());
        if (!si.empty())
            for (int i = 0; i < boost; i++) ids.insert(ids.end(), si.begin(), si.end());
    }
    std::vector<std::optional<int>> out;
    for (int id : ids) out.push_back(items_.at(id) ? std::optional<int>(id) : std::nullopt);
    return out;
}

}  // namespace pr::gen4
