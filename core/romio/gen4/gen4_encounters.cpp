// Pokémon salvajes de Gen 4 (port de Gen4RomHandler.getEncounters y sus métodos de lectura, de
// loadWildMapNames y de Gen4EncounterAreaTagger).
#include <algorithm>

#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_enc_tags_gen.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
int readLong(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}

const char* const kDpptWaterSlotSetNames[] = {"Surfing", "Filler", "Old Rod", "Good Rod", "Super Rod"};
const char* const kHgssTimeOfDayNames[] = {"Morning", "Day", "Night"};
const char* const kHgssNonWalkingAreaNames[] = {"", "Surfing", "Rock Smash", "Old Rod", "Good Rod", "Super Rod"};
const EncounterType kHgssNonWalkingAreaTypes[] = {EncounterType::UNUSED, EncounterType::SURFING, EncounterType::INTERACT,
                                                  EncounterType::FISHING, EncounterType::FISHING, EncounterType::FISHING};

void addEnc(EncounterArea& a, Species* sp, int level, int maxLevel = 0) {
    auto e = std::make_shared<Encounter>(sp, level);
    e->maxLevel = maxLevel;
    a.encounters.push_back(e);
}

void setIdentifiers(EncounterArea& a, const std::string& name, int mapIndex, EncounterType type) {
    a.displayName = name;
    a.mapIndex = mapIndex;
    a.encounterType = type;
}
}  // namespace

void Gen4RomHandler::loadWildMapNames() const {
    wildMapNames_ = JavaHashMap<int, std::string>();
    headbuttMapNames_.clear();
    Bytes internalNames = readFile(romEntry_->getFile("MapTableFile"));
    int numMapHeaders = static_cast<int>(internalNames.size()) / 16;
    int baseMHOffset = romEntry_->getInt("MapTableARM9Offset");
    auto allMapNames = getStrings(romEntry_->getInt("MapNamesTextOffset"));
    int mapNameIndexSize = romEntry_->getInt("MapTableNameIndexSize");
    for (int map = 0; map < numMapHeaders; map++) {
        int base = baseMHOffset + map * 24;
        int mapNameIndex = mapNameIndexSize == 2 ? readWord(arm9_, base + 18) : arm9_.at(base + 18);
        const std::string& mapName = allMapNames.at(mapNameIndex);
        if (romEntry_->romType() == Type_HGSS) {
            int wildSet = arm9_.at(base);
            if (wildSet != 255) wildMapNames_.put(wildSet, mapName);
            headbuttMapNames_[map] = mapName;
        } else {
            int wildSet = readWord(arm9_, base + 14);
            if (wildSet != 65535) wildMapNames_.put(wildSet, mapName);
        }
    }
    loadedWildMapNames_ = true;
}

std::vector<EncounterArea> Gen4RomHandler::getEncounters(bool useTimeOfDay) const {
    if (!loadedWildMapNames_) loadWildMapNames();
    std::vector<EncounterArea> areas;
    if (romEntry_->romType() == Type_HGSS) {
        readMainEncountersHGSS(areas, useTimeOfDay);
        readHeadbuttEncounters(areas);
        readBugCatchingContestEncounters(areas);
    } else {
        readMainEncountersDPPt(areas, useTimeOfDay);
        readExtraEncountersDPPt(areas);
    }
    tagEncounterAreas(areas, useTimeOfDay);
    return areas;
}

void Gen4RomHandler::tagEncounterAreas(std::vector<EncounterArea>& areas, bool useTimeOfDay) const {
    const std::vector<std::string_view>* tags;
    const std::vector<int>* postGame;
    const std::vector<int>* partial = nullptr;
    switch (romEntry_->romType()) {
        case Type_DP: tags = &tags::locationTagsDP; postGame = &tags::dpPostGameEncounterAreas; break;
        case Type_Plat: tags = &tags::locationTagsPt; postGame = &tags::platPostGameEncounterAreas; break;
        case Type_HGSS:
            tags = useTimeOfDay ? &tags::locationTagsUseTimeHGSS : &tags::locationTagsNoTimeHGSS;
            postGame = useTimeOfDay ? &tags::hgssPostGameEncounterAreasTOD : &tags::hgssPostGameEncounterAreasNoTOD;
            partial = useTimeOfDay ? &tags::hgssPartialPostGameTOD : &tags::hgssPartialPostGameNoTOD;
            break;
        default: throw std::logic_error("Unexpected value for romType: " + std::to_string(romEntry_->romType()));
    }
    if (areas.size() != tags->size())
        throw std::invalid_argument("Unexpected amount of encounter areas. Expected: " + std::to_string(tags->size()) +
                                    ", was: " + std::to_string(areas.size()));
    for (size_t i = 0; i < areas.size(); i++) areas[i].locationTag = std::string((*tags)[i]);
    for (int i : *postGame) areas.at(i).postGame = true;
    if (partial)
        for (int i : *partial) areas.at(i).partiallyPostGameCutoff = tags::headbuttPartialPostgameCutoff;
}

std::vector<EncounterArea> Gen4RomHandler::getSortedEncounters(bool useTimeOfDay) const {
    const auto& order = romEntry_->romType() == Type_HGSS ? tags::locationTagsTraverseOrderHGSS : tags::locationTagsTraverseOrderDPPt;
    auto areas = getEncounters(useTimeOfDay);
    auto indexOf = [&](const EncounterArea& a) {
        if (!a.locationTag) return -1;
        auto it = std::find(order.begin(), order.end(), std::string_view(*a.locationTag));
        return it == order.end() ? -1 : static_cast<int>(it - order.begin());
    };
    std::stable_sort(areas.begin(), areas.end(), [&](const EncounterArea& x, const EncounterArea& y) { return indexOf(x) < indexOf(y); });
    return areas;
}

// ---------------- DPPt ----------------

void Gen4RomHandler::readMainEncountersDPPt(std::vector<EncounterArea>& areas, bool useTimeOfDay) const {
    auto data = readNARC(romEntry_->getFile("WildPokemon"));
    int c = -1;
    for (const Bytes& b : data.files) {
        c++;
        if (!wildMapNames_.containsKey(c)) wildMapNames_.put(c, "? Unknown ?");
        std::string mapName = *wildMapNames_.get(c);
        int walkingRate = readLong(b, 0);
        if (walkingRate != 0) {
            EncounterArea walking;
            for (int i = 0; i < 12; i++) addEnc(walking, pokes_.at(readLong(b, 4 + 4 + i * 8)), readLong(b, 4 + i * 8));
            walking.displayName = mapName + " Grass/Cave";
            walking.encounterType = EncounterType::WALKING;
            walking.rate = walkingRate;
            walking.mapIndex = c;
            if (useTimeOfDay) {
                for (int i = 0; i < 4; i++) {
                    int pknum = readLong(b, 108 + 4 * i);
                    if (pknum >= 1 && pknum <= pokemonCount)
                        addEnc(walking, pokes_[pknum], walking.encounters.at(dpptAlternateSlots[i + 2])->level);
                }
            }
            areas.push_back(walking);
            EncounterArea conds;
            conds.displayName = mapName + " Swarm/Radar/GBA";
            conds.encounterType = EncounterType::SPECIAL;
            conds.rate = walkingRate;
            conds.mapIndex = c;
            for (int i = 0; i < 20; i++) {
                if (i >= 2 && i <= 5) continue;
                int offs = 100 + i * 4 + (i >= 10 ? 24 : 0);
                int pknum = readLong(b, offs);
                if (pknum >= 1 && pknum <= pokemonCount)
                    addEnc(conds, pokes_[pknum], walking.encounters.at(dpptAlternateSlots[i])->level);
            }
            if (!conds.encounters.empty()) areas.push_back(conds);
        }
        int startOffset = 204;
        for (int i = 0; i < 5; i++) {
            if (i == 1) continue;
            int offset = startOffset + i * dpptWaterAreaSize;
            int rate = readLong(b, offset);
            if (rate == 0) continue;
            EncounterArea sea;
            for (int k = 0; k < 5; k++) {
                int v = readLong(b, offset + 4 + k * 8);
                addEnc(sea, pokes_.at(readLong(b, offset + 4 + 4 + k * 8)), v >> 8, v & 0xFF);
            }
            sea.displayName = mapName + " " + kDpptWaterSlotSetNames[i];
            sea.encounterType = i == 0 ? EncounterType::SURFING : EncounterType::FISHING;
            sea.mapIndex = c;
            sea.rate = rate;
            areas.push_back(sea);
        }
    }
}

namespace {
EncounterArea readExtraArea(const std::vector<Species*>& pokes, const Bytes& data, int offset, int amount) {
    EncounterArea a;
    a.rate = 1;
    for (int i = 0; i < amount; i++) addEnc(a, pokes.at(readLong(data, offset + i * 4)), 1);
    return a;
}
}  // namespace

void Gen4RomHandler::readExtraEncountersDPPt(std::vector<EncounterArea>& areas) const {
    auto extra = readNARC(romEntry_->getFile("ExtraEncounters"));
    Bytes ovl = readOverlay(romEntry_->getInt("EncounterOvlNumber"));

    // Feebas de las casillas del Monte Corona
    {
        EncounterArea area = readExtraArea(pokes_, extra.files.at(0), 0, 1);
        int offset = find(ovl, std::string(feebasLevelPrefixDPPt));
        if (offset > 0) {
            offset += static_cast<int>(feebasLevelPrefixDPPt.size()) / 2;
            for (auto& e : area.encounters) {
                e->maxLevel = static_cast<int8_t>(ovl.at(offset));
                e->level = static_cast<int8_t>(ovl.at(offset + 4));
            }
        }
        setIdentifiers(area, "Mt. Coronet Feebas Tiles", mtCoronetFeebasLakeMapIndex, EncounterType::SPECIAL);
        areas.push_back(area);
    }
    // árboles de miel
    const auto& honeyTreeOffsets = romEntry_->getArray("HoneyTreeOffsets");
    for (size_t i = 0; i < honeyTreeOffsets.size(); i++) {
        EncounterArea area = readExtraArea(pokes_, extra.files.at(honeyTreeOffsets[i]), 0, 6);
        int offset = find(ovl, std::string(honeyTreeLevelPrefixDPPt));
        if (offset > 0) {
            offset += static_cast<int>(honeyTreeLevelPrefixDPPt.size()) / 2;
            int8_t level = (ovl.at(offset + 46) == 0x0B && ovl.at(offset + 47) == 0x2E) ? 5 : static_cast<int8_t>(ovl.at(offset + 46));
            for (auto& e : area.encounters) {
                e->maxLevel = static_cast<int8_t>(ovl.at(offset + 102));
                e->level = level;
            }
        }
        area.displayName = "Honey Tree Group " + std::to_string(i + 1);
        area.encounterType = EncounterType::INTERACT;
        areas.push_back(area);
    }
    // Pokémon rotatorios del Jardín Trofeo (Sr. Fondi)
    {
        EncounterArea area = readExtraArea(pokes_, extra.files.at(8), 0, 16);
        int idx = romEntry_->romType() == Type_DP ? trophyGardenGrassEncounterIndexDP : trophyGardenGrassEncounterIndexPt;
        const EncounterArea& grass = areas.at(idx);
        int l1 = grass.encounters.at(6)->level, l2 = grass.encounters.at(7)->level;
        for (auto& e : area.encounters) {
            e->level = std::min(l1, l2);
            if (l1 != l2) e->maxLevel = std::max(l1, l2);
        }
        setIdentifiers(area, "Trophy Garden Rotating Pokemon (via Mr. Backlot)", trophyGardenMapIndex, EncounterType::SPECIAL);
        area.forceMultipleSpecies = true;
        areas.push_back(area);
    }
    // Pokémon rotatorios del Gran Pantano
    const int marshOffsets[] = {9, 10};
    for (int i = 0; i < 2; i++) {
        EncounterArea area = readExtraArea(pokes_, extra.files.at(marshOffsets[i]), 0, 32);
        int level = 100, maxLevel = 0;
        const auto& idxs = romEntry_->romType() == Type_DP ? marshGrassEncounterIndicesDP : marshGrassEncounterIndicesPt;
        for (int idx : idxs) {
            const EncounterArea& g = areas.at(idx);
            for (int s : {6, 7}) {
                int cur = g.encounters.at(s)->level;
                if (cur < level) level = cur;
                if (cur > maxLevel) maxLevel = cur;
            }
        }
        for (auto& e : area.encounters) {
            e->level = level;
            e->maxLevel = maxLevel;
        }
        std::string status = i == 0 ? "(Post-National Dex)" : "(Pre-National Dex)";
        setIdentifiers(area, "Great Marsh Rotating Pokemon " + status, -2, EncounterType::SPECIAL);
        areas.push_back(area);
    }
}

// ---------------- HGSS ----------------

namespace {
EncounterArea readOptionalAreaHGSS(const std::vector<Species*>& pokes, const Bytes& data, int offset, int amount) {
    EncounterArea a;
    a.rate = 1;
    for (int i = 0; i < amount; i++) {
        int pk = readWord(data, offset + i * 2);
        if (pk != 0) addEnc(a, pokes.at(pk), 1);
    }
    return a;
}
}  // namespace

void Gen4RomHandler::readMainEncountersHGSS(std::vector<EncounterArea>& areas, bool useTimeOfDay) const {
    auto data = readNARC(romEntry_->getFile("WildPokemon"));
    const int amounts[] = {0, 5, 2, 5, 5, 5};
    int mapIndex = -1;
    for (const Bytes& b : data.files) {
        mapIndex++;
        bool badMapIndex = false;
        if (mapIndex == nationalParkBadMapIndex) {
            mapIndex = nationalParkMapIndex;
            badMapIndex = true;
        }
        if (!wildMapNames_.containsKey(mapIndex)) wildMapNames_.put(mapIndex, "? Unknown ?");
        std::string mapName = *wildMapNames_.get(mapIndex);
        int rates[6];
        for (int i = 0; i < 6; i++) rates[i] = b.at(i);
        int walkingLevels[12];
        for (int i = 0; i < 12; i++) walkingLevels[i] = b.at(8 + i);
        Species* walkingPokes[3][12];
        for (int t = 0; t < 3; t++)
            for (int i = 0; i < 12; i++) walkingPokes[t][i] = pokes_.at(readWord(b, 20 + t * 24 + i * 2));
        if (rates[0] != 0) {
            auto stitch = [&](int t, const std::string& name) {
                EncounterArea a;
                for (int i = 0; i < 12; i++) addEnc(a, walkingPokes[t][i], walkingLevels[i]);
                a.rate = rates[0];
                setIdentifiers(a, name, mapIndex, EncounterType::WALKING);
                areas.push_back(a);
            };
            if (!useTimeOfDay) stitch(1, mapName + " Grass/Cave");
            else
                for (int t = 0; t < 3; t++) stitch(t, mapName + " " + kHgssTimeOfDayNames[t] + " Grass/Cave");
        }
        EncounterArea radio = readOptionalAreaHGSS(pokes_, b, 92, 4);
        setIdentifiers(radio, mapName + " Hoenn/Sinnoh Radio", mapIndex, EncounterType::SPECIAL);
        if (!radio.encounters.empty()) areas.push_back(radio);
        int offset = 100;
        for (int i = 1; i < 6; i++) {
            if (rates[i] != 0) {
                EncounterArea sea;
                for (int k = 0; k < amounts[i]; k++) {
                    int w = readWord(b, offset + k * 4);
                    addEnc(sea, pokes_.at(readWord(b, offset + 2 + k * 4)), w & 0xFF, w >> 8);
                }
                setIdentifiers(sea, mapName + " " + kHgssNonWalkingAreaNames[i], mapIndex, kHgssNonWalkingAreaTypes[i]);
                sea.rate = rates[i];
                areas.push_back(sea);
            }
            offset += 4 * amounts[i];
        }
        EncounterArea swarm = readOptionalAreaHGSS(pokes_, b, offset, 2);
        setIdentifiers(swarm, mapName + " Swarms", mapIndex, EncounterType::SPECIAL);
        if (!swarm.encounters.empty()) areas.push_back(swarm);
        EncounterArea nightFish = readOptionalAreaHGSS(pokes_, b, offset + 4, 1);
        setIdentifiers(nightFish, mapName + " Night Fishing Replacement", mapIndex, EncounterType::FISHING);
        if (!nightFish.encounters.empty()) areas.push_back(nightFish);
        EncounterArea fishSwarm = readOptionalAreaHGSS(pokes_, b, offset + 6, 1);
        setIdentifiers(fishSwarm, mapName + " Fishing Swarm", mapIndex, EncounterType::SPECIAL);
        if (!fishSwarm.encounters.empty()) areas.push_back(fishSwarm);
        if (badMapIndex) mapIndex = nationalParkBadMapIndex;
    }
}

void Gen4RomHandler::readHeadbuttEncounters(std::vector<EncounterArea>& areas) const {
    auto data = readNARC(romEntry_->getFile("HeadbuttPokemon"));
    int mapID = -1;
    int lastCreatedID = wildMapNames_.size() - 1;
    for (const Bytes& b : data.files) {
        mapID++;
        if (b.size() == 4) continue;
        auto hn = headbuttMapNames_.find(mapID);
        std::string mapName = hn == headbuttMapNames_.end() ? "null" : hn->second;
        EncounterArea area;
        area.rate = 1;
        for (int i = 0; i < 18; i++) {
            int pk = readWord(b, 4 + i * 4);
            if (pk != 0) addEnc(area, pokes_.at(pk), readWord(b, 4 + 2 + i * 4) & 0xFF, static_cast<int8_t>(b.at(4 + 3 + i * 4)));
        }
        // el mapa 24 es una versión sin usar de la Ruta 16
        if (!area.encounters.empty() && mapID != 24) {
            area.displayName = mapName + " Headbutt";
            area.encounterType = EncounterType::INTERACT;
            bool found = false;
            wildMapNames_.forEach([&](const int& k, const std::string& v) {
                if (!found && v == mapName) {
                    area.mapIndex = k;
                    found = true;
                }
            });
            if (area.mapIndex == -1) {
                do {
                    lastCreatedID++;
                } while (wildMapNames_.containsKey(lastCreatedID));
                area.mapIndex = lastCreatedID;
            }
            areas.push_back(area);
        }
    }
}

void Gen4RomHandler::readBugCatchingContestEncounters(std::vector<EncounterArea>& areas) const {
    Bytes d = readFile(romEntry_->getFile("BCCWilds"));
    auto read = [&](int offset, const std::string& name) {
        EncounterArea a;
        a.rate = 1;
        for (int i = 0; i < 10; i++) {
            int pk = readWord(d, offset + i * 8);
            if (pk != 0) addEnc(a, pokes_.at(pk), static_cast<int8_t>(d.at(offset + 2 + i * 8)), static_cast<int8_t>(d.at(offset + 3 + i * 8)));
        }
        setIdentifiers(a, name, nationalParkMapIndex, EncounterType::SPECIAL);
        if (!a.encounters.empty()) areas.push_back(a);
    };
    read(0, "Bug Catching Contest (Pre-National Dex)");
    read(80, "Bug Catching Contest (Post-National Dex, Tuesdays)");
    read(160, "Bug Catching Contest (Post-National Dex, Thursdays)");
    read(240, "Bug Catching Contest (Post-National Dex, Saturdays)");
}

}  // namespace pr::gen4
