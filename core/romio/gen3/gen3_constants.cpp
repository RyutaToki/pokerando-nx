#include "romio/gen3/gen3_constants.hpp"

#include <stdexcept>
#include <unordered_map>

namespace pr::gen3 {

std::optional<Type> typeFromByte(int b) {
    switch (b & 0xFF) {
        case 0x00: return Type::NORMAL;
        case 0x01: return Type::FIGHTING;
        case 0x02: return Type::FLYING;
        case 0x03: return Type::POISON;
        case 0x04: return Type::GROUND;
        case 0x05: return Type::ROCK;
        case 0x06: return Type::BUG;
        case 0x07: return Type::GHOST;
        case 0x08: return Type::STEEL;
        case 0x0A: return Type::FIRE;
        case 0x0B: return Type::WATER;
        case 0x0C: return Type::GRASS;
        case 0x0D: return Type::ELECTRIC;
        case 0x0E: return Type::PSYCHIC;
        case 0x0F: return Type::ICE;
        case 0x10: return Type::DRAGON;
        case 0x11: return Type::DARK;
        default: return std::nullopt;
    }
}

int typeToByte(std::optional<Type> t) {
    if (!t) return 0x09;  // tipo ???
    switch (*t) {
        case Type::NORMAL: return 0x00;
        case Type::FIGHTING: return 0x01;
        case Type::FLYING: return 0x02;
        case Type::POISON: return 0x03;
        case Type::GROUND: return 0x04;
        case Type::ROCK: return 0x05;
        case Type::BUG: return 0x06;
        case Type::GHOST: return 0x07;
        case Type::STEEL: return 0x08;
        case Type::FIRE: return 0x0A;
        case Type::WATER: return 0x0B;
        case Type::GRASS: return 0x0C;
        case Type::ELECTRIC: return 0x0D;
        case Type::PSYCHIC: return 0x0E;
        case Type::ICE: return 0x0F;
        case Type::DRAGON: return 0x10;
        case Type::DARK: return 0x11;
        default: return 0;  // FAIRY no existe en Gen 3
    }
}

namespace {
struct ItemMaps {
    std::unordered_map<int, int> toStandard, toInternal;
    ItemMaps() {
        static constexpr int pairs[][2] = {
#include "romio/gen3/gen3_item_map.inc"
        };
        for (auto& p : pairs) {
            toStandard[p[0]] = p[1];
            toInternal[p[1]] = p[0];
        }
    }
};
const ItemMaps& itemMaps() {
    static const ItemMaps m;
    return m;
}

void addBetween(std::set<int>& s, int a, int b) {
    for (int i = a; i <= b; i++) s.insert(i);
}
}  // namespace

int itemIDToStandard(int id) {
    auto& m = itemMaps().toStandard;
    auto it = m.find(id);
    return it == m.end() ? items::UNIQUE_OFFSET + id : it->second;
}

int itemIDToInternal(int id) {
    if (id >= items::UNIQUE_OFFSET) return id - items::UNIQUE_OFFSET;
    return itemMaps().toInternal.at(id);
}

int getLastItemID(int romType) {
    switch (romType) {
        case RomType_Ruby:
        case RomType_Sapp: return items::Gen3::lastRS;
        case RomType_FRLG: return items::Gen3::lastFRLG;
        case RomType_Em: return items::Gen3::lastEm;
        default: throw std::invalid_argument("Invalid RomType");
    }
}

const std::set<int>& bannedItems() {
    static const std::set<int> s = [] {
        std::set<int> r;
        addBetween(r, items::Gen3::first, items::Gen3::last);
        addBetween(r, items::hm01, items::hm08);
        return r;
    }();
    return s;
}

const std::set<int>& badItems(int romType) {
    static const std::set<int> rse = [] {
        std::set<int> r = {items::lightBall, items::oranBerry, items::soulDew};
        addBetween(r, items::mail1, items::mail12);
        addBetween(r, items::figyBerry, items::belueBerry);
        addBetween(r, items::liechiBerry, items::enigmaBerry);
        addBetween(r, items::luckyPunch, items::leek);
        addBetween(r, items::redScarf, items::yellowScarf);
        return r;
    }();
    static const std::set<int> frlg = [] {
        std::set<int> r = rse;
        addBetween(r, items::shoalSalt, items::greenShard);
        return r;
    }();
    return romType == RomType_FRLG ? frlg : rse;
}

}  // namespace pr::gen3
