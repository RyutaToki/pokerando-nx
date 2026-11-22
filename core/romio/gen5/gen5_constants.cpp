#include "romio/gen5/gen5_constants.hpp"

#include <map>
#include <stdexcept>

namespace pr::gen5 {

std::optional<Type> typeFromByte(int b) {
    static const Type t[] = {Type::NORMAL, Type::FIGHTING, Type::FLYING, Type::POISON,  Type::GROUND, Type::ROCK,
                             Type::BUG,    Type::GHOST,    Type::STEEL,  Type::FIRE,    Type::WATER,  Type::GRASS,
                             Type::ELECTRIC, Type::PSYCHIC, Type::ICE,   Type::DRAGON,  Type::DARK};
    if (b < 0 || b >= static_cast<int>(std::size(t))) throw std::out_of_range("Index " + std::to_string(b) + " out of bounds for length 17");
    return t[b];
}

int typeToByte(std::optional<Type> t) {
    if (!t) return 0x00;  // no hay tipo ???: normal
    switch (*t) {
        case Type::NORMAL: return 0x00;
        case Type::FIGHTING: return 0x01;
        case Type::FLYING: return 0x02;
        case Type::POISON: return 0x03;
        case Type::GROUND: return 0x04;
        case Type::ROCK: return 0x05;
        case Type::BUG: return 0x06;
        case Type::GHOST: return 0x07;
        case Type::FIRE: return 0x09;
        case Type::WATER: return 0x0A;
        case Type::GRASS: return 0x0B;
        case Type::ELECTRIC: return 0x0C;
        case Type::PSYCHIC: return 0x0D;
        case Type::ICE: return 0x0E;
        case Type::DRAGON: return 0x0F;
        case Type::STEEL: return 0x08;
        case Type::DARK: return 0x10;
        default: return 0;
    }
}

MoveCategory moveCategoryFromByte(int b) {
    static const MoveCategory t[] = {MoveCategory::STATUS, MoveCategory::PHYSICAL, MoveCategory::SPECIAL};
    if (b < 0 || b > 2) throw std::out_of_range("moveCategoryIndices");
    return t[b];
}

int moveCategoryToByte(MoveCategory c) {
    switch (c) {
        case MoveCategory::PHYSICAL: return 1;
        case MoveCategory::SPECIAL: return 2;
        default: return 0;
    }
}

namespace {
const EvolutionType kEvoTable[] = {
    EvolutionType::HAPPINESS,         EvolutionType::HAPPINESS_DAY,      EvolutionType::HAPPINESS_NIGHT,
    EvolutionType::LEVEL,             EvolutionType::TRADE,              EvolutionType::TRADE_ITEM,
    EvolutionType::TRADE_SPECIAL,     EvolutionType::STONE,              EvolutionType::LEVEL_ATTACK_HIGHER,
    EvolutionType::LEVEL_ATK_DEF_SAME, EvolutionType::LEVEL_DEFENSE_HIGHER, EvolutionType::LEVEL_LOW_PV,
    EvolutionType::LEVEL_HIGH_PV,     EvolutionType::LEVEL_CREATE_EXTRA, EvolutionType::LEVEL_IS_EXTRA,
    EvolutionType::HIGH_BEAUTY,       EvolutionType::STONE_MALE_ONLY,    EvolutionType::STONE_FEMALE_ONLY,
    EvolutionType::ITEM_DAY,          EvolutionType::ITEM_NIGHT,         EvolutionType::WITH_MOVE,
    EvolutionType::WITH_OTHER,        EvolutionType::LEVEL_MALE_ONLY,    EvolutionType::LEVEL_FEMALE_ONLY,
    EvolutionType::MAGNETIC_FIELD,    EvolutionType::MOSS_ROCK,          EvolutionType::ICE_ROCK};
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

int getMapIndexForLocationEvolution(EvolutionType et, int romType) {
    switch (et) {
        case EvolutionType::MAGNETIC_FIELD: return romType == Type_BW ? 41 : 20;  // Cueva Electrorroca
        case EvolutionType::MOSS_ROCK: return romType == Type_BW ? 5 : 8;         // Bosque Azulona
        case EvolutionType::ICE_ROCK: return romType == Type_BW ? 44 : 23;        // Monte Tuerca
        default: throw std::invalid_argument("not a valid EvolutionType for this game.");
    }
}

int getAreaDataEntryLength(int romType) {
    return romType == Type_BW ? bw1AreaDataEntryLength : romType == Type_BW2 ? bw2AreaDataEntryLength : 0;
}
int getEncounterAreaCount(int romType) {
    return romType == Type_BW ? bw1EncounterAreaCount : romType == Type_BW2 ? bw2EncounterAreaCount : 0;
}
const std::vector<int>& getWildFileToAreaMap(int romType) {
    static const std::vector<int> none;
    return romType == Type_BW ? bw1WildFileToAreaMap : romType == Type_BW2 ? bw2WildFileToAreaMap : none;
}
const std::vector<int>& getMainGameShops(int romType) {
    static const std::vector<int> none;
    return romType == Type_BW ? bw1MainGameShops : romType == Type_BW2 ? bw2MainGameShops : none;
}
const std::vector<int>& getIrregularFormes(int romType) {
    static const std::vector<int> none;
    return romType == Type_BW ? bw1IrregularFormes : romType == Type_BW2 ? bw2IrregularFormes : none;
}
int getFormeCount(int romType) { return romType == Type_BW ? bw1FormeCount : romType == Type_BW2 ? bw2FormeCount : 0; }
int getFormeOffset(int romType) { return romType == Type_BW ? bw1formeOffset : romType == Type_BW2 ? bw2formeOffset : 0; }
int getNonPokemonBattleSpriteCount(int romType) {
    return romType == Type_BW ? bw1NonPokemonBattleSpriteCount : romType == Type_BW2 ? bw2NonPokemonBattleSpriteCount : 0;
}
const std::vector<std::string_view>& getLocationTagsTraverseOrder(int romType) {
    return romType == Type_BW ? locationTagsTraverseOrderBW : locationTagsTraverseOrderBW2;
}

std::string getFormeSuffixByBaseForme(int baseForme, int formNum) {
    using namespace species;
    static const std::map<int, std::vector<std::string>> m = {
        {castform, {"-Sunny", "-Rainy", "-Snowy"}},
        {deoxys, {"-Attack", "-Defense", "-Speed"}},
        {wormadam, {"-Sandy", "-Trash"}},
        {rotom, {"-Heat", "-Wash", "-Frost", "-Fan", "-Mow"}},
        {giratina, {"-Origin"}},
        {shaymin, {"-Sky"}},
        {basculin, {"-Blue"}},
        {darmanitan, {"-Zen"}},
        {tornadus, {"-Therian"}},
        {thundurus, {"-Therian"}},
        {landorus, {"-Therian"}},
        {kyurem, {"-White", "-Black"}},
        {keldeo, {"-Resolute"}},
        {meloetta, {"-Pirouette"}}};
    auto it = m.find(baseForme);
    if (it == m.end()) return "";  // dummy {0: ""}
    if (formNum < 1 || formNum > static_cast<int>(it->second.size())) return "";
    return it->second[formNum - 1];
}

int getAbsolutePokeNumByBaseForme(int baseForme, int formNum) {
    using namespace species;
    namespace F = species::Gen5Formes;
    static const std::map<int, std::vector<int>> m = {
        {deoxys, {F::deoxysA, F::deoxysD, F::deoxysS}},
        {wormadam, {F::wormadamS, F::wormadamT}},
        {shaymin, {F::shayminS}},
        {giratina, {F::giratinaO}},
        {rotom, {F::rotomH, F::rotomW, F::rotomFr, F::rotomFa, F::rotomM}},
        {castform, {F::castformSu, F::castformR, F::castformSn}},
        {basculin, {F::basculinB}},
        {darmanitan, {F::darmanitanZ}},
        {meloetta, {F::meloettaP}},
        {kyurem, {F::kyuremW, F::kyuremB}},
        {keldeo, {F::keldeoCosmetic1}},
        {tornadus, {F::tornadusT}},
        {thundurus, {F::thundurusT}},
        {landorus, {F::landorusT}}};
    auto it = m.find(baseForme);
    if (it == m.end()) return formNum == 255 ? 0 : baseForme;  // dummy {255: 0}
    if (formNum < 1 || formNum > static_cast<int>(it->second.size())) return baseForme;
    return it->second[formNum - 1];
}

namespace {
void addBetween(JavaHashSet<int>& s, int a, int b) {
    for (int i = a; i <= b; i++) s.add(i);
}

JavaHashSet<int> makeBadItemsBW2() {
    JavaHashSet<int> r;
    for (int i : {items::oddKeystone, items::griseousOrb, items::adamantOrb, items::lustrousOrb, items::soulDew, items::lightBall,
                  items::oranBerry, items::quickPowder, items::passOrb, items::prettyFeather})
        r.add(i);
    addBetween(r, items::growthMulch, items::gooeyMulch);
    addBetween(r, items::mail1, items::mail12);
    addBetween(r, items::figyBerry, items::belueBerry);
    addBetween(r, items::luckyPunch, items::leek);
    addBetween(r, items::redScarf, items::yellowScarf);
    return r;
}
}  // namespace

const JavaHashSet<int>& bannedItems() {
    static const JavaHashSet<int> s = [] {
        JavaHashSet<int> r;
        for (int i : {items::libertyPass, items::propCase, items::dragonSkull, items::lightStone, items::darkStone}) r.add(i);
        addBetween(r, items::explorerKit, items::tidalBell);
        addBetween(r, items::dataCard01, items::enigmaStone);
        addBetween(r, items::xtransceiverMale, items::revealGlass);
        addBetween(r, items::tea, items::autograph);
        addBetween(r, items::unused120, items::unused133);
        addBetween(r, items::hm01, items::hm08);
        addBetween(r, items::direHit2, items::direHit3);
        return r;
    }();
    return s;
}

const JavaHashSet<int>& getBadItems(int romType) {
    static const JavaHashSet<int> bw2 = makeBadItemsBW2();
    static const JavaHashSet<int> bw1 = [] {
        JavaHashSet<int> r = makeBadItemsBW2();  // new HashSet<>(badItemsBW2): mismo contenido
        addBetween(r, items::redShard, items::greenShard);
        return r;
    }();
    return romType == Type_BW2 ? bw2 : bw1;
}

const JavaHashSet<int>& opShopItems() {
    static const JavaHashSet<int> s = [] {
        JavaHashSet<int> r;
        r.add(items::lavaCookie);
        r.add(items::berryJuice);
        r.add(items::rareCandy);
        r.add(items::oldGateau);
        addBetween(r, items::blueFlute, items::shoalShell);
        addBetween(r, items::tinyMushroom, items::nugget);
        r.add(items::rareBone);
        addBetween(r, items::lansatBerry, items::rowapBerry);
        r.add(items::luckyEgg);
        r.add(items::prettyFeather);
        addBetween(r, items::balmMushroom, items::casteliacone);
        return r;
    }();
    return s;
}

}  // namespace pr::gen5
