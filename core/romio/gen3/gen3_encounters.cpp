// Mapas y encuentros salvajes de Gen 3 (port de Gen3RomHandler.determineMapBankSizes,
// preprocessMaps, getEncounters, setEncounters, attemptObedienceEvolutionPatches y del
// Gen3EncounterAreaTagger).
#include <algorithm>
#include <set>

#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_enc_tags_gen.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

int Gen3RomHandler::readPointerOrInvalid(int offset) const {
    int pointer = static_cast<int>(static_cast<uint32_t>(readLong(offset)) - 0x8000000u);
    if (pointer < 0 || pointer > static_cast<int>(rom_.size())) return -1;
    return pointer;
}

void Gen3RomHandler::determineMapBankSizes() {
    int mbpsOffset = romEntry_->getInt("MapHeaders");
    std::vector<int> mapBankOffsets;
    int offset = mbpsOffset;
    while (true) {
        bool valid = true;
        for (int mb : mapBankOffsets) {
            if (mbpsOffset < mb && offset >= mb) {
                valid = false;
                break;
            }
        }
        if (!valid) break;
        int newMB = readPointerOrInvalid(offset);
        if (newMB == -1) break;
        mapBankOffsets.push_back(newMB);
        offset += 4;
    }
    int bankCount = static_cast<int>(mapBankOffsets.size());
    std::vector<int> bankMapCounts(bankCount);
    for (int bank = 0; bank < bankCount; bank++) {
        int base = mapBankOffsets[bank];
        int count = 0;
        offset = base;
        while (true) {
            bool valid = true;
            for (int mb : mapBankOffsets) {
                if (base < mb && offset >= mb) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
            if (base < mbpsOffset && offset >= mbpsOffset) break;
            if (readPointerOrInvalid(offset) == -1) break;
            count++;
            offset += 4;
        }
        bankMapCounts[bank] = count;
    }
    romEntry_->putInt("MapBankCount", bankCount);
    romEntry_->putArray("MapBankSizes", bankMapCounts);
}

void Gen3RomHandler::preprocessMaps() {
    itemOffs_.clear();
    int bankCount = romEntry_->getInt("MapBankCount");
    std::vector<int> bankMapCounts = romEntry_->getArray("MapBankSizes");
    int itemBall = romEntry_->getInt("ItemBallPic");
    mapNames_.assign(bankCount, {});
    int mbpsOffset = romEntry_->getInt("MapHeaders");
    int mapLabels = romEntry_->getInt("MapLabels");
    std::map<int, std::string> mapLabelsM;

    // prepareEventTextEntriesByBankAndMap: punteros a las entradas de la copia del RomEntry
    std::vector<std::vector<std::vector<EventTextEntry*>>> byBankAndMap(bankCount);
    for (int b = 0; b < bankCount; b++) byBankAndMap[b].resize(bankMapCounts[b]);
    for (auto* list : {&romEntry_->starterTexts(), &romEntry_->tmTexts(), &romEntry_->moveTutorTexts()}) {
        for (auto& ete : *list) byBankAndMap.at(ete.mapBank).at(ete.mapNumber).push_back(&ete);
    }

    for (int bank = 0; bank < bankCount; bank++) {
        int bankOffset = readPointer(mbpsOffset + bank * 4);
        mapNames_[bank].assign(bankMapCounts[bank], "");
        for (int map = 0; map < bankMapCounts[bank]; map++) {
            int mhOffset = readPointer(bankOffset + map * 4);
            int mapLabel = rom_.at(mhOffset + 0x14);
            if (auto it = mapLabelsM.find(mapLabel); it != mapLabelsM.end()) {
                mapNames_[bank][map] = it->second;
            } else {
                if (romEntry_->romType() == RomType_FRLG) {
                    mapNames_[bank][map] = readVariableLengthString(readPointer(mapLabels + (mapLabel - frlgMapLabelsStart) * 4));
                } else {
                    mapNames_[bank][map] = readVariableLengthString(readPointer(mapLabels + mapLabel * 8 + 4));
                }
                mapLabelsM[mapLabel] = mapNames_[bank][map];
            }

            int eventOffset = readPointerOrInvalid(mhOffset + 4);
            if (eventOffset == -1) continue;
            int pCount = rom_.at(eventOffset);
            int spCount = rom_.at(eventOffset + 3);
            if (pCount > 0) {
                int peopleOffset = readPointer(eventOffset + 4);
                for (int p = 0; p < pCount; p++) {
                    int pSprite = static_cast<int8_t>(rom_.at(peopleOffset + p * 24 + 1));
                    int pointerOffset = peopleOffset + p * 24 + 16;
                    if (pSprite == itemBall && readPointerOrInvalid(pointerOffset) != -1) {
                        int so = readPointer(pointerOffset);
                        if (rom_[so] == 0x1A && rom_[so + 1] == 0x00 && rom_[so + 2] == 0x80 && rom_[so + 5] == 0x1A &&
                            rom_[so + 6] == 0x01 && rom_[so + 7] == 0x80 && rom_[so + 10] == 0x09 &&
                            (rom_[so + 11] == 0x00 || rom_[so + 11] == 0x01)) {
                            itemOffs_.push_back(so + 3);  // script de Poké Ball con objeto
                        }
                    }
                }
                for (EventTextEntry* ete : byBankAndMap[bank][map]) {
                    int scriptOffset = readPointer(peopleOffset + (ete->personNum - 1) * 24 + 16);
                    const auto& rel = ete->relativePointerOffsets;
                    for (size_t i = 0; i + 1 < rel.size(); i++) scriptOffset = readPointer(scriptOffset + rel[i]);
                    ete->actualPointerOffset = scriptOffset + rel.back();
                }
            }
            if (spCount > 0) {
                int signpostsOffset = readPointer(eventOffset + 16);
                for (int sp = 0; sp < spCount; sp++) {
                    int spType = static_cast<int8_t>(rom_.at(signpostsOffset + sp * 12 + 5));
                    if (spType >= 5 && spType <= 7) {
                        // objeto oculto (el id 0 son monedas)
                        if (readWord(signpostsOffset + sp * 12 + 8) != 0) itemOffs_.push_back(signpostsOffset + sp * 12 + 8);
                    }
                }
            }
        }
    }
}

namespace {
struct AreaSlot {
    int pointerFieldOffset;
    int slots;
    const char* suffix;
    EncounterType type;
};
constexpr AreaSlot kAreaSlots[] = {{4, walkingSlots, " Grass/Cave", EncounterType::WALKING},
                                   {8, surfingSlots, " Surfing", EncounterType::SURFING},
                                   {12, rockSmashSlots, " Rock Smash", EncounterType::INTERACT},
                                   {16, fishingSlots, " Fishing", EncounterType::FISHING}};
}  // namespace

EncounterArea Gen3RomHandler::readEncounterArea(int offset, int numOfEntries, const std::string& name,
                                                EncounterType type) const {
    EncounterArea area;
    area.rate = static_cast<int8_t>(rom_.at(offset));
    area.displayName = name;
    area.encounterType = type;
    int dataOffset = readPointer(offset + 4);
    for (int i = 0; i < numOfEntries; i++) {
        Species* pk = pokesInternal_.at(readWord(dataOffset + i * 4 + 2));
        auto enc = std::make_shared<Encounter>(pk, rom_.at(dataOffset + i * 4));
        enc->maxLevel = static_cast<int8_t>(rom_.at(dataOffset + i * 4 + 1));
        area.encounters.push_back(enc);
    }
    return area;
}

std::vector<EncounterArea> Gen3RomHandler::getEncounters(bool /*useTimeOfDay*/) const {
    int offs = romEntry_->getInt("WildPokemon");
    std::vector<EncounterArea> areas;
    std::set<int> seen;
    while (true) {
        int bank = rom_.at(offs), map = rom_.at(offs + 1);
        if (bank == 0xFF && map == 0xFF) break;
        const std::string& mapName = mapNames_.at(bank).at(map);
        for (const auto& slot : kAreaSlots) {
            int o = readPointerOrInvalid(offs + slot.pointerFieldOffset);
            if (o != -1 && rom_.at(o) != 0 && !seen.count(readPointer(o + 4))) {
                areas.push_back(readEncounterArea(o, slot.slots, mapName + slot.suffix, slot.type));
                seen.insert(readPointer(o + 4));
            }
        }
        offs += 20;
    }
    std::vector<int> banned = romEntry_->getArray("BattleTrappersBanned");
    if (!banned.empty()) {
        // zonas donde no puede haber Pokémon con Sombra Trampa, Trampa Arena, etc.
        JavaHashSet<Species*, JavaHashCode<Species*>, SpeciesPtrEq> trappers;
        for (Species* pk : getSpecies()) {
            if (hasBattleTrappingAbility(pk)) trappers.add(pk);
        }
        for (int idx : banned) areas.at(idx).banAllSpecies(trappers.toVector());
    }
    tagEncounterAreas(areas);
    return areas;
}

bool Gen3RomHandler::hasBattleTrappingAbility(const Species* s) const {
    if (!s) return false;
    const auto& b = global::battleTrappingAbilities;
    return std::find(b.begin(), b.end(), s->ability1) != b.end() || std::find(b.begin(), b.end(), s->ability2) != b.end();
}

void Gen3RomHandler::tagEncounterAreas(std::vector<EncounterArea>& areas) const {
    const std::vector<std::string_view>* tags;
    const std::vector<int>* postGame;
    switch (romEntry_->romType()) {
        case RomType_Ruby:
        case RomType_Sapp: tags = &tags::locationTagsRS; postGame = &tags::rsPostGameEncounterAreas; break;
        case RomType_Em: tags = &tags::locationTagsEm; postGame = &tags::emPostGameEncounterAreas; break;
        case RomType_FRLG: tags = &tags::locationTagsFRLG; postGame = &tags::frlgPostGameEncounterAreas; break;
        default: throw std::logic_error("Unexpected value for romType");
    }
    if (areas.size() != tags->size()) {
        throw std::invalid_argument("Unexpected amount of encounter areas. Expected: " + std::to_string(tags->size()) +
                                    ", was: " + std::to_string(areas.size()));
    }
    for (size_t i = 0; i < areas.size(); i++) areas[i].locationTag = std::string((*tags)[i]);
    for (int idx : *postGame) areas.at(idx).postGame = true;
}

std::vector<EncounterArea> Gen3RomHandler::getSortedEncounters(bool useTimeOfDay) const {
    const auto& order = romEntry_->romType() == RomType_FRLG ? locationTagsTraverseOrderFRLG : locationTagsTraverseOrderRSE;
    auto idx = [&](const EncounterArea& a) {
        auto it = std::find(order.begin(), order.end(), a.locationTag ? std::string_view(*a.locationTag) : std::string_view());
        return it == order.end() ? -1 : static_cast<int>(it - order.begin());
    };
    auto areas = getEncounters(useTimeOfDay);
    std::stable_sort(areas.begin(), areas.end(), [&](const EncounterArea& a, const EncounterArea& b) { return idx(a) < idx(b); });
    return areas;
}

void Gen3RomHandler::setEncounters(bool /*useTimeOfDay*/, const std::vector<EncounterArea>& areas) {
    attemptObedienceEvolutionPatches();  // permitir que Deoxys/Mew obedezcan en E/RF/VH
    int offs = romEntry_->getInt("WildPokemon");
    auto it = areas.begin();
    std::set<int> seen;
    while (true) {
        int bank = rom_.at(offs), map = rom_.at(offs + 1);
        if (bank == 0xFF && map == 0xFF) break;
        for (const auto& slot : kAreaSlots) {
            int o = readPointerOrInvalid(offs + slot.pointerFieldOffset);
            if (o != -1 && rom_.at(o) != 0 && !seen.count(readPointer(o + 4))) {
                writeEncounterArea(o, slot.slots, *it++);
                seen.insert(readPointer(o + 4));
            }
        }
        offs += 20;
    }
}

void Gen3RomHandler::writeEncounterArea(int offset, int numOfEntries, const EncounterArea& area) {
    int dataOffset = readPointer(offset + 4);
    for (int i = 0; i < numOfEntries; i++) {
        const Encounter& enc = *area.encounters.at(i);
        writeWord(dataOffset + i * 4, enc.level | (enc.maxLevel << 8));
        writeWord(dataOffset + i * 4 + 2, pokedexToInternal_[enc.species()->number()]);
    }
}

SpeciesSet Gen3RomHandler::getBannedForWildEncounters() const {
    SpeciesSet banned;
    // en RF/VH el juego se cuelga si aparece Unown fuera de las Ruinas Tanoby
    if (romEntry_->romType() == RomType_FRLG) banned.add(pokes_.at(species::unown));
    return banned;
}

void Gen3RomHandler::attemptObedienceEvolutionPatches() {
    if (havePatchedObedience_) return;
    havePatchedObedience_ = true;
    int deoxysOb = find(deoxysObeyCode);
    if (deoxysOb > 0) {
        writeBytes(deoxysOb, {0x00, static_cast<uint8_t>(gbaSetRxOpcode | gbaR1), 0x00,
                              static_cast<uint8_t>(gbaSetRxOpcode | gbaR1)});
        if (readWord(deoxysOb + mewObeyOffsetFromDeoxysObey) == (((gbaCmpRxOpcode | gbaR0) << 8) | species::mew)) {
            writeWord(deoxysOb + mewObeyOffsetFromDeoxysObey, ((gbaCmpRxOpcode | gbaR0) << 8) | 0);
        }
    }
    if (romEntry_->romType() == RomType_FRLG) {
        int evoJump = find(levelEvoKantoDexCheckCode);
        if (evoJump > 0) {
            writeWord(evoJump, gbaNopOpcode);
            writeWord(evoJump + 2, (gbaUnconditionalJumpOpcode << 8) | levelEvoKantoDexJumpAmount);
        }
        int stoneJump = find(stoneEvoKantoDexCheckCode);
        if (stoneJump > 0) {
            writeWord(stoneJump, gbaNopOpcode);
            writeWord(stoneJump + 2, (gbaUnconditionalJumpOpcode << 8) | stoneEvoKantoDexJumpAmount);
        }
    }
}

}  // namespace pr::gen3
