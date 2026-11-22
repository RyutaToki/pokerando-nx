// Encuentros salvajes de Gen 5 (port de Gen5RomHandler.getEncounters/setEncounters, las zonas de la
// Pokédex y Gen5EncounterAreaTagger). Las cuatro estaciones son el «horario» de esta generación.
#include <algorithm>
#include <stdexcept>

#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_enc_tags_gen.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
}  // namespace

void Gen5RomHandler::loadWildMapNames() const {
    wildMapNames_ = JavaHashMap<int, std::string>();
    Bytes mapHeaderData = readNARC(romEntry_->getFile("MapTableFile")).files.at(0);
    int numMapHeaders = static_cast<int>(mapHeaderData.size()) / 48;
    auto allMapNames = getStrings(false, romEntry_->getInt("MapNamesTextOffset"));
    for (int map = 0; map < numMapHeaders; map++) {
        int base = map * 48;
        const std::string& mapName = allMapNames.at(mapHeaderData.at(base + 26));
        if (romEntry_->romType() == Type_BW2) {
            int wildSet = mapHeaderData.at(base + 20);
            if (wildSet != 255) wildMapNames_.put(wildSet, mapName);
        } else {
            int wildSet = readWord(mapHeaderData, base + 20);
            if (wildSet != 65535) wildMapNames_.put(wildSet, mapName);
        }
    }
    loadedWildMapNames_ = true;
}

void Gen5RomHandler::processEncounterEntry(std::vector<EncounterArea>& areas, const Bytes& entry, int startOffset, int idx) const {
    if (!wildMapNames_.containsKey(idx)) wildMapNames_.put(idx, "? Unknown ?");
    std::string mapName = *wildMapNames_.get(idx);
    int offset = 8;
    for (int i = 0; i < 7; i++) {
        int rate = entry.at(startOffset + i);
        if (rate != 0) {
            EncounterArea area;
            int base = startOffset + offset;
            for (int k = 0; k < encountersOfEachType[i]; k++) {
                int w = readWord(entry, base + k * 4);
                int speciesID = w & 0x7FF;
                int forme = w >> 11;
                int level = entry.at(base + 2 + k * 4);
                int maxLevel = entry.at(base + 3 + k * 4);
                // Negro 2 tiene un Corphish con «forma 1» por error (junto a los datos de Basculin)
                if (forme == 30 || forme == 31 || speciesID == species::corphish) forme = 0;
                auto enc = std::make_shared<Encounter>(pokes_.at(speciesID), level);
                enc->holder.setAltFormeAllowed();
                enc->holder.setFormeNumber(forme);
                enc->maxLevel = maxLevel;
                area.encounters.push_back(enc);
            }
            area.rate = rate;
            area.displayName = mapName + " " + std::string(encounterTypeNames[i]);
            area.mapIndex = idx;
            area.encounterType = encounterTypeValues[i];
            areas.push_back(std::move(area));
        }
        offset += encountersOfEachType[i] * 4;
    }
}

std::vector<EncounterArea> Gen5RomHandler::getEncounters(bool useTimeOfDay) const {
    if (!loadedWildMapNames_) loadWildMapNames();
    auto narc = readNARC(romEntry_->getFile("WildPokemon"));
    std::vector<EncounterArea> areas;
    int idx = -1;
    for (const Bytes& entry : narc.files) {
        idx++;
        if (static_cast<int>(entry.size()) > perSeasonEncounterDataLength && useTimeOfDay) {
            for (int i = 0; i < 4; i++) processEncounterEntry(areas, entry, i * perSeasonEncounterDataLength, idx);
        } else {
            processEncounterEntry(areas, entry, 0, idx);
        }
    }
    // Gen5EncounterAreaTagger
    bool bw = romEntry_->romType() == Type_BW;
    const auto& tagList = bw ? (useTimeOfDay ? tags::locationTagsUseTimeBW : tags::locationTagsNoTimeBW)
                             : (useTimeOfDay ? tags::locationTagsUseTimeBW2 : tags::locationTagsNoTimeBW2);
    const auto& postGame = bw ? (useTimeOfDay ? tags::bwPostGameEncounterAreasTOD : tags::bwPostGameEncounterAreasNoTOD)
                              : (useTimeOfDay ? tags::b2w2PostGameEncounterAreasTOD : tags::b2w2PostGameEncounterAreasNoTOD);
    if (areas.size() != tagList.size())
        throw std::invalid_argument("Unexpected amount of encounter areas. Expected: " + std::to_string(tagList.size()) +
                                    ", was: " + std::to_string(areas.size()));
    for (size_t i = 0; i < areas.size(); i++) areas[i].locationTag = std::string(tagList[i]);
    for (int i : postGame) areas.at(i).postGame = true;
    // banAltFormesIfNeeded (aquí todos los encuentros admiten formas, así que no banea nada)
    for (auto& area : areas) {
        bool allAllowed = std::all_of(area.encounters.begin(), area.encounters.end(),
                                      [](const EncounterPtr& e) { return e->holder.isAltFormeAllowed(); });
        if (!allAllowed) {
            for (Species* pk : speciesInclFormes_)
                if (pk && !pk->isBaseForme()) area.banSpecies(pk);
        }
    }
    return areas;
}

std::vector<EncounterArea> Gen5RomHandler::getSortedEncounters(bool useTimeOfDay) const {
    const auto& order = getLocationTagsTraverseOrder(romEntry_->romType());
    auto areas = getEncounters(useTimeOfDay);
    auto indexOf = [&](const EncounterArea& a) {
        if (!a.locationTag) return -1;
        auto it = std::find(order.begin(), order.end(), std::string_view(*a.locationTag));
        return it == order.end() ? -1 : static_cast<int>(it - order.begin());
    };
    std::stable_sort(areas.begin(), areas.end(), [&](const EncounterArea& x, const EncounterArea& y) { return indexOf(x) < indexOf(y); });
    return areas;
}

void Gen5RomHandler::writeEncounterEntry(AreaIter& it, AreaIter end, Bytes& entry, int startOffset) {
    int offset = 8;
    for (int i = 0; i < 7; i++) {
        int rate = entry.at(startOffset + i);
        if (rate != 0) {
            if (it == end) throw std::runtime_error("NoSuchElementException");
            const EncounterArea& area = *it++;
            for (int j = 0; j < encountersOfEachType[i]; j++) {
                if (j >= static_cast<int>(area.encounters.size())) throw std::runtime_error("NoSuchElementException");
                const Encounter& enc = *area.encounters[j];
                int o = startOffset + offset + j * 4;
                writeWord(entry, o, (enc.holder.formeNumber() << 11) + enc.species()->baseNumber());
                entry.at(o + 2) = static_cast<uint8_t>(enc.level);
                entry.at(o + 3) = static_cast<uint8_t>(enc.maxLevel);
            }
        }
        offset += encountersOfEachType[i] * 4;
    }
}

void Gen5RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) {
    auto narc = readNARC(romEntry_->getFile("WildPokemon"));
    auto it = areas.begin();
    for (Bytes& entry : narc.files) {
        writeEncounterEntry(it, areas.end(), entry, 0);
        if (entry.size() > 232) {
            if (useTimeOfDay) {
                for (int i = 1; i < 4; i++) writeEncounterEntry(it, areas.end(), entry, i * 232);
            } else {
                // se copia la primavera a las otras tres estaciones
                std::copy_n(entry.begin(), 232, entry.begin() + 232);
                std::copy_n(entry.begin(), 232, entry.begin() + 464);
                std::copy_n(entry.begin(), 232, entry.begin() + 696);
            }
        }
    }
    writeNARC(romEntry_->getFile("WildPokemon"), narc);
    updatePokedexAreaData(narc);
    // (la lista de hábitats está desactivada en FVX: el juego se cuelga si hay demasiadas entradas)
}

void Gen5RomHandler::updateAreaDataFromEncounterEntry(const Bytes& entry, int startOffset, std::vector<Bytes>& areaData, int season,
                                                      int fileNumber) const {
    int encounterAreaCount = getEncounterAreaCount(romEntry_->romType());
    const auto& wildFileToAreaMap = getWildFileToAreaMap(romEntry_->romType());
    const std::string& code = romEntry_->romCode();
    bool bw2 = romEntry_->romType() == Type_BW2;
    int offset = 8;
    for (int i = 0; i < 7; i++) {
        int rate = entry.at(startOffset + i);
        if (rate != 0) {
            for (int e = 0; e < encountersOfEachType[i]; e++) {
                int o = startOffset + offset + e * 4;
                Species* pk = pokes_.at(entry.at(o) + ((entry.at(o + 1) & 0x03) << 8));
                Bytes& pokeFile = areaData.at(pk->baseNumber() - 1);
                int areaIndex = wildFileToAreaMap.at(fileNumber);
                // zonas exclusivas de la otra versión
                if (bw2 && areaIndex == bw2Route4AreaIndex) {
                    if ((fileNumber == b2Route4EncounterFile && code.at(2) == 'D') || (fileNumber == w2Route4EncounterFile && code.at(2) == 'E'))
                        areaIndex = -1;
                }
                if (bw2 && areaIndex == bw2VictoryRoadAreaIndex) {
                    if (code.at(2) == 'D') {
                        if (fileNumber == b2VRExclusiveRoom1 || fileNumber == b2VRExclusiveRoom2) areaIndex = -1;
                    } else {
                        if (fileNumber == w2VRExclusiveRoom1 || fileNumber == w2VRExclusiveRoom2) areaIndex = -1;
                    }
                }
                if (bw2 && areaIndex == bw2ReversalMountainAreaIndex) {
                    if (code.at(2) == 'D') {
                        if (fileNumber >= b2ReversalMountainStart && fileNumber <= b2ReversalMountainEnd) areaIndex = -1;
                    } else {
                        if (fileNumber >= w2ReversalMountainStart && fileNumber <= w2ReversalMountainEnd) areaIndex = -1;
                    }
                }
                if (areaIndex != -1) pokeFile.at(season * (encounterAreaCount + 1) + 2 + areaIndex) |= static_cast<uint8_t>(1 << i);
            }
        }
        offset += encountersOfEachType[i] * 4;
    }
}

void Gen5RomHandler::updatePokedexAreaData(const nds::NARCArchive& encounterNARC) {
    auto areaNARC = readNARC(romEntry_->getFile("PokedexAreaData"));
    int entryLength = getAreaDataEntryLength(romEntry_->romType());
    int encounterAreaCount = getEncounterAreaCount(romEntry_->romType());
    std::vector<Bytes> newFiles(pokemonCount, Bytes(entryLength, 0));
    for (int i = 0; i < static_cast<int>(encounterNARC.files.size()); i++) {
        const Bytes& encEntry = encounterNARC.files[i];
        bool seasonal = static_cast<int>(encEntry.size()) > perSeasonEncounterDataLength;
        for (int season = 0; season < 4; season++)
            updateAreaDataFromEncounterEntry(encEntry, seasonal ? season * perSeasonEncounterDataLength : 0, newFiles, season, i);
    }
    for (int i = 0; i < pokemonCount; i++) {
        Bytes& file = newFiles[i];
        for (int season = 0; season < 4; season++) {
            bool unobtainable = true;
            for (int enc = 0; enc < encounterAreaCount; enc++) {
                if (file.at(season * (encounterAreaCount + 1) + enc + 2) != 0) {
                    unobtainable = false;
                    break;
                }
            }
            if (unobtainable) file.at(season * (encounterAreaCount + 1) + 1) = 1;
        }
        bool seasonalDependent = false;
        for (int enc = 0; enc < encounterAreaCount; enc++) {
            uint8_t spring = file.at(enc + 2);
            uint8_t summer = file.at((encounterAreaCount + 1) + enc + 2);
            uint8_t autumn = file.at(2 * (encounterAreaCount + 1) + enc + 2);
            uint8_t winter = file.at(3 * (encounterAreaCount + 1) + enc + 2);
            if (!(spring == summer && spring == autumn && spring == winter)) {
                seasonalDependent = true;
                break;
            }
        }
        if (!seasonalDependent) file.at(0) = 1;
        areaNARC.files.at(i) = file;
    }
    writeNARC(romEntry_->getFile("PokedexAreaData"), areaNARC);
}

SpeciesSet Gen5RomHandler::getBannedForWildEncounters() const { return SpeciesSet(); }
SpeciesSet Gen5RomHandler::getBannedFormesForTrainerPokemon() const { return SpeciesSet(); }

}  // namespace pr::gen5
