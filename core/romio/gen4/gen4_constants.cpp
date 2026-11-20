#include "romio/gen4/gen4_constants.hpp"

#include <stdexcept>

namespace pr::gen4 {

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
    if (!t) return 0x09;  // ???
    switch (*t) {
        case Type::NORMAL: return 0x00;
        case Type::FIGHTING: return 0x01;
        case Type::FLYING: return 0x02;
        case Type::POISON: return 0x03;
        case Type::GROUND: return 0x04;
        case Type::ROCK: return 0x05;
        case Type::BUG: return 0x06;
        case Type::GHOST: return 0x07;
        case Type::FIRE: return 0x0A;
        case Type::WATER: return 0x0B;
        case Type::GRASS: return 0x0C;
        case Type::ELECTRIC: return 0x0D;
        case Type::PSYCHIC: return 0x0E;
        case Type::ICE: return 0x0F;
        case Type::DRAGON: return 0x10;
        case Type::STEEL: return 0x08;
        case Type::DARK: return 0x11;
        default: return 0;
    }
}

MoveCategory moveCategoryFromByte(int b) {
    static const MoveCategory t[] = {MoveCategory::PHYSICAL, MoveCategory::SPECIAL, MoveCategory::STATUS};
    if (b < 0 || b > 2) throw std::out_of_range("moveCategoryIndices");
    return t[b];
}

int moveCategoryToByte(MoveCategory c) {
    switch (c) {
        case MoveCategory::PHYSICAL: return 0;
        case MoveCategory::SPECIAL: return 1;
        default: return 2;
    }
}

namespace {
const EvolutionType kEvoTable[] = {
    EvolutionType::HAPPINESS, EvolutionType::HAPPINESS_DAY, EvolutionType::HAPPINESS_NIGHT, EvolutionType::LEVEL,
    EvolutionType::TRADE, EvolutionType::TRADE_ITEM, EvolutionType::STONE, EvolutionType::LEVEL_ATTACK_HIGHER,
    EvolutionType::LEVEL_ATK_DEF_SAME, EvolutionType::LEVEL_DEFENSE_HIGHER, EvolutionType::LEVEL_LOW_PV,
    EvolutionType::LEVEL_HIGH_PV, EvolutionType::LEVEL_CREATE_EXTRA, EvolutionType::LEVEL_IS_EXTRA,
    EvolutionType::HIGH_BEAUTY, EvolutionType::STONE_MALE_ONLY, EvolutionType::STONE_FEMALE_ONLY,
    EvolutionType::ITEM_DAY, EvolutionType::ITEM_NIGHT, EvolutionType::WITH_MOVE,
    EvolutionType::WITH_OTHER, EvolutionType::LEVEL_MALE_ONLY, EvolutionType::LEVEL_FEMALE_ONLY,
    EvolutionType::MAGNETIC_FIELD, EvolutionType::MOSS_ROCK, EvolutionType::ICE_ROCK};
}  // namespace

int evolutionTypeToIndex(EvolutionType t) {
    for (size_t i = 0; i < std::size(kEvoTable); i++)
        if (kEvoTable[i] == t) return static_cast<int>(i) + 1;
    return -1;
}

EvolutionType evolutionTypeFromIndex(int index) {
    if (index == -1) return EvolutionType::NONE;
    if (index < 1 || index > static_cast<int>(std::size(kEvoTable))) throw std::out_of_range("evolutionTypeTable");
    return kEvoTable[index - 1];
}

int getFormeCount(int romType) {
    if (romType == Type_DP) return dpFormeCount;
    if (romType == Type_Plat || romType == Type_HGSS) return platHgSsFormeCount;
    return 0;
}

const std::map<int, FormeInfo>& formeMappings() {
    static const std::map<int, FormeInfo> m = [] {
        std::map<int, FormeInfo> r;
        auto put = [&](int base, std::initializer_list<int> alts) {
            int i = 0;
            for (int a : alts) r[a + formeOffset] = FormeInfo{base, ++i};
        };
        using namespace species;
        put(deoxys, {Gen4Formes::deoxysA, Gen4Formes::deoxysD, Gen4Formes::deoxysS});
        put(wormadam, {Gen4Formes::wormadamS, Gen4Formes::wormadamT});
        put(giratina, {Gen4Formes::giratinaO});
        put(shaymin, {Gen4Formes::shayminS});
        put(rotom, {Gen4Formes::rotomH, Gen4Formes::rotomW, Gen4Formes::rotomFr, Gen4Formes::rotomFa, Gen4Formes::rotomM});
        return r;
    }();
    return m;
}

int cosmeticFormsOf(int sp) {
    switch (sp) {
        case species::unown: return 28;
        case species::burmy: return 3;
        case species::shellos: return 2;
        case species::gastrodon: return 2;
        default: return 0;
    }
}

std::string getFormeSuffixByBaseForme(int baseForme, int formNum) {
    static const std::map<int, std::vector<std::string>> m = {
        {species::deoxys, {"-Attack", "-Defense", "-Speed"}},
        {species::wormadam, {"-Sandy", "-Trash"}},
        {species::rotom, {"-Heat", "-Wash", "-Frost", "-Fan", "-Mow"}},
        {species::giratina, {"-Origin"}},
        {species::shaymin, {"-Sky"}}};
    auto it = m.find(baseForme);
    if (it == m.end()) return "";  // dummy {0: ""}
    if (formNum < 1 || formNum > static_cast<int>(it->second.size())) return "";
    return it->second[formNum - 1];
}

int getAbsolutePokeNumByBaseForme(int baseForme, int formNum) {
    using namespace species;
    static const std::map<int, std::vector<int>> m = {
        {deoxys, {Gen4Formes::deoxysA, Gen4Formes::deoxysD, Gen4Formes::deoxysS}},
        {wormadam, {Gen4Formes::wormadamS, Gen4Formes::wormadamT}},
        {giratina, {Gen4Formes::giratinaO}},
        {shaymin, {Gen4Formes::shayminS}},
        {rotom, {Gen4Formes::rotomH, Gen4Formes::rotomW, Gen4Formes::rotomFr, Gen4Formes::rotomFa, Gen4Formes::rotomM}}};
    auto it = m.find(baseForme);
    if (it == m.end()) return formNum == 255 ? 0 : baseForme;  // dummy {255: 0}
    if (formNum < 1 || formNum > static_cast<int>(it->second.size())) return baseForme;
    return it->second[formNum - 1];
}

namespace {
void addBetween(JavaHashSet<int>& s, int a, int b) {
    for (int i = a; i <= b; i++) s.add(i);
}
}  // namespace

const JavaHashSet<int>& bannedItems() {
    static const JavaHashSet<int> s = [] {
        JavaHashSet<int> r;
        addBetween(r, items::explorerKit, items::enigmaStone);
        addBetween(r, items::griseousOrb, items::sweetHeart);
        addBetween(r, items::hm01, items::hm08);
        return r;
    }();
    return s;
}

const JavaHashSet<int>& badItems() {
    static const JavaHashSet<int> s = [] {
        JavaHashSet<int> r;
        for (int i : {items::oddKeystone, items::griseousOrb, items::soulDew, items::lightBall, items::oranBerry, items::quickPowder,
                      items::shoalSalt, items::shoalShell, items::adamantOrb, items::lustrousOrb})
            r.add(i);
        addBetween(r, items::growthMulch, items::gooeyMulch);
        addBetween(r, items::mail1, items::mail12);
        addBetween(r, items::figyBerry, items::belueBerry);
        addBetween(r, items::luckyPunch, items::leek);
        addBetween(r, items::redScarf, items::yellowScarf);
        return r;
    }();
    return s;
}

const JavaHashSet<int>& opShopItems() {
    static const JavaHashSet<int> s = [] {
        JavaHashSet<int> r;
        r.add(items::rareCandy);
        addBetween(r, items::tinyMushroom, items::nugget);
        r.add(items::rareBone);
        r.add(items::luckyEgg);
        return r;
    }();
    return s;
}

}  // namespace pr::gen4
