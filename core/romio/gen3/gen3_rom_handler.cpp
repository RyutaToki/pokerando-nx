#include "romio/gen3/gen3_rom_handler.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <stdexcept>

#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "util/crc32.hpp"
#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr::gen3 {

namespace {

bool romCodeMatches(const std::vector<uint8_t>& rom, const std::string& code) {
    for (size_t i = 0; i < code.size(); i++) {
        if (rom.at(romCodeOffset + i) != static_cast<uint8_t>(code[i])) return false;
    }
    return true;
}

bool romNameMatches(const std::vector<uint8_t>& rom, std::string_view name) {
    for (size_t i = 0; i < name.size(); i++) {
        if (rom.at(romNameOffset + i) != static_cast<uint8_t>(name[i])) return false;
    }
    return true;
}

bool detectRomInner(std::vector<uint8_t>& rom, size_t romSize) {
    if (romSize != size8M && romSize != size16M && romSize != size32M) return false;
    if (rom.size() < 0x100) return false;
    // Caso especial: traducción no oficial de Esmeralda
    if (romNameMatches(rom, unofficialEmeraldROMName)) {
        rom[romCodeOffset] = 'B';
        rom[romCodeOffset + 1] = 'P';
        rom[romCodeOffset + 2] = 'E';
        rom[romCodeOffset + 3] = 'T';
        rom[headerChecksumOffset] = 0x66;
    }
    for (auto& re : Gen3RomEntry::all()) {
        if (romCodeMatches(rom, re->romCode()) && rom[romVersionOffset] == re->version()) return true;
    }
    return false;
}

int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument("hex inválido");
}

}  // namespace

bool Gen3RomHandler::isLoadable(const std::vector<uint8_t>& header, size_t fileSize) {
    if (fileSize > 32 * 1024 * 1024 || header.empty()) return false;
    std::vector<uint8_t> copy = header;
    return detectRomInner(copy, fileSize);
}

void Gen3RomHandler::loadRom(std::vector<uint8_t> bytes, const std::string& fileName) {
    if (!detectRomInner(bytes, bytes.size())) throw std::runtime_error("No se reconoce la ROM.");
    rom_ = std::move(bytes);
    originalRom_ = rom_;
    loadedFileName_ = fileName;
    actualCRC32_ = crc32(rom_.data(), rom_.size());

    // midLoadingSetUp (parte común)
    initRomEntry();
    initTextTables();
    isRomHack_ = false;
    jamboMovesetHack_ = false;
    // TODO(port): basicBPRE10HackSupport (solo hacks de Rojo Fuego U 1.0)
    determineMapBankSizes();
    preprocessMaps();
    freeAllUnusedSpace();

    // loadGameData
    loadItems();
    loadSpeciesStats();
    loadEvolutions();
    loadMoves();
    loadPokemonPalettes();
    loadTrainerClasses();
    loadTrainers();
    loadAbilityNames();
    estimateEvolutionLevels();
}

void Gen3RomHandler::saveRom(const std::string& fileName) {
    prepareSaveRom();
    saveRomFileOnly(fileName);
}

void Gen3RomHandler::saveRomFileOnly(const std::string& fileName) const {
    std::ofstream f(fileName, std::ios::binary);
    if (!f) throw std::runtime_error("No se puede escribir en " + fileName);
    f.write(reinterpret_cast<const char*>(rom_.data()), static_cast<std::streamsize>(rom_.size()));
    if (!f) throw std::runtime_error("Error al escribir " + fileName);
}

bool Gen3RomHandler::isRomValid() const {
    return romEntry_->expectedCRC32() == static_cast<int64_t>(actualCRC32_);
}

void Gen3RomHandler::initRomEntry() {
    for (auto& re : Gen3RomEntry::all()) {
        if (romCodeMatches(rom_, re->romCode()) && rom_[0xBC] == re->version()) {
            romEntry_ = std::make_unique<Gen3RomEntry>(*re);  // copia para poder modificarla
            break;
        }
    }
    auto& e = *romEntry_;
    // addPointerBlocksToRomEntry
    if (e.getInt("HasPointerBlock1") == 1) {
        e.putInt("PokemonFrontImages", readPointer(pokemonFrontImagesPointer));
        e.putInt("PokemonBackImages", readPointer(pokemonBackImagesPointer));
        e.putInt("PokemonNormalPalettes", readPointer(pokemonNormalPalettesPointer));
        e.putInt("PokemonShinyPalettes", readPointer(pokemonShinyPalettesPointer));
        e.putInt("PokemonIconSprites", readPointer(pokemonIconSpritesPointer));
        e.putInt("PokemonIconPalettes", readPointer(pokemonIconPalettesPointer));
        e.putInt("PokemonNames", readPointer(pokemonNamesPointer));
        e.putInt("MoveNames", readPointer(moveNamesPointer));
        e.putInt("DecorationNames", readPointer(decorationNamesPointer));
    } else {
        int baseNomOffset = find(rsPokemonNamesPointerSuffix);
        e.putInt("PokemonNames", readPointer(baseNomOffset - 4));
    }
    if (e.getInt("HasPointerBlock2") == 1) {
        e.putInt("PokemonStats", readPointer(pokemonStatsPointer));
        e.putInt("AbilityNames", readPointer(abilityNamesPointer));
        e.putInt("AbilityDescriptions", readPointer(abilityDescriptionsPointer));
        e.putInt("ItemData", readPointer(itemDataPointer));
        e.putInt("MoveData", readPointer(moveDataPointer));
        e.putInt("BallSpritesPointer", readPointer(ballSpritesPointer));
        e.putInt("BallPalettesPointer", readPointer(ballPalettesPointer));
    }
    // addMoveTutorInfoToRomEntry
    if (e.romType() == RomType_Em || e.romType() == RomType_FRLG) {
        e.putInt("MoveTutorCompatibility", e.getInt("MoveTutorData") + e.getInt("MoveTutorMoves") * 2);
    }
    // addTrainerGraphicsInfoToRomEntry
    auto addRelative = [&](const std::string& newKey, const std::string& baseKey, int offset) {
        int base = e.getInt(baseKey);
        if (base != 0 && e.getInt(newKey) == 0) e.putInt(newKey, base + offset);
    };
    int front = 0, back = 0;
    switch (e.romType()) {
        case RomType_Ruby:
        case RomType_Sapp: front = rsTrainerFrontPalettesOffset; back = rsTrainerBackPalettesOffset; break;
        case RomType_Em: front = emTrainerFrontPalettesOffset; back = emTrainerBackPalettesOffset; break;
        case RomType_FRLG: front = frlgTrainerFrontPalettesOffset; back = frlgTrainerBackPalettesOffset; break;
        default: throw std::runtime_error("Invalid romType");
    }
    addRelative("TrainerFrontPalettes", "TrainerFrontImages", front);
    addRelative("TrainerBackPalettes", "TrainerBackImages", back);
    if (e.romType() == RomType_FRLG) {
        addRelative("RedMapIconPalettePointer", "RedMapIconImagePointer", redMapIconPalettePointerOffset);
        addRelative("LeafMapIconImagePointer", "RedMapIconImagePointer", leafMapIconImagePointerOffset);
        addRelative("LeafMapIconPalettePointer", "RedMapIconImagePointer", leafMapIconPalettePointerOffset);
    } else {
        addRelative("BrendanMapIconPalette", "BrendanMapIconImage", brendanMapIconPaletteOffset);
        addRelative("MayMapIconImage", "BrendanMapIconImage", mayMapIconImageOffset);
        addRelative("MayMapIconPalette", "BrendanMapIconImage", mayMapIconPaletteOffset);
    }
}

void Gen3RomHandler::initTextTables() {
    tb_.fill(std::nullopt);
    d_.clear();
    loadTextTable(romEntry_->tableFile());
}

void Gen3RomHandler::loadTextTable(const std::string& filename) {
    auto text = embeddedFile("texttables/" + filename + ".tbl");
    if (!text) throw std::runtime_error("Text table not found: " + filename);
    size_t pos = 0;
    while (pos < text->size()) {
        size_t nl = text->find('\n', pos);
        std::string line(text->substr(pos, nl == std::string_view::npos ? std::string_view::npos : nl - pos));
        pos = nl == std::string_view::npos ? text->size() : nl + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (jc::trim(line).empty()) continue;
        auto r = jc::split(line, "=", 2);
        int32_t idx;
        if (r.size() < 2 || !jc::parseIntRadix(r[0], 16, idx)) throw std::runtime_error("Línea de tabla inválida: " + line);
        auto value = jc::utf8ToU32(r[1]);
        tb_[idx & 0xFF] = value;
        d_[value] = static_cast<uint8_t>(idx);
    }
}

std::string Gen3RomHandler::readString(int offset, int maxLength) const {
    std::u32string out;
    char buf[8];
    for (int i = 0; i < maxLength; i++) {
        uint8_t c = rom_.at(offset + i);
        if (tb_[c]) {
            out += *tb_[c];
        } else if (c == textTerminator) {
            break;
        } else if (c == textVariable) {
            int next = rom_.at(offset + i + 1);
            std::snprintf(buf, sizeof buf, "\\v%02X", next);
            out += jc::utf8ToU32(buf);
            i++;
        } else {
            std::snprintf(buf, sizeof buf, "\\x%02X", c);
            out += jc::utf8ToU32(buf);
        }
    }
    return jc::u32ToUtf8(out);
}

std::vector<uint8_t> Gen3RomHandler::translateString(const std::string& textUtf8) const {
    std::vector<uint8_t> out;
    std::u32string text = jc::utf8ToU32(textUtf8);
    auto hexByte = [](const std::u32string& s) {
        return static_cast<uint8_t>(hexDigit(static_cast<char>(s[2])) * 16 + hexDigit(static_cast<char>(s[3])));
    };
    while (!text.empty()) {
        int i = std::max(0, 4 - static_cast<int>(text.size()));
        if (text.size() >= 2 && text[0] == U'\\' && text[1] == U'x') {
            out.push_back(hexByte(text));
            text = text.substr(4);
        } else if (text.size() >= 2 && text[0] == U'\\' && text[1] == U'v') {
            out.push_back(textVariable);
            out.push_back(hexByte(text));
            text = text.substr(4);
        } else {
            while (!(d_.count(text.substr(0, 4 - i)) || i == 4)) i++;
            if (i == 4) {
                text = text.substr(1);
            } else {
                out.push_back(d_.at(text.substr(0, 4 - i)));
                text = text.substr(4 - i);
            }
        }
    }
    return out;
}

int Gen3RomHandler::readLong(int o) const {
    return static_cast<int>(static_cast<uint32_t>(rom_.at(o)) | (static_cast<uint32_t>(rom_.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(rom_.at(o + 2)) << 16) | (static_cast<uint32_t>(rom_.at(o + 3)) << 24));
}

int Gen3RomHandler::readPointer(int offset) const {
    int pointer = static_cast<int>(static_cast<uint32_t>(readLong(offset)) - 0x8000000u);
    if (pointer < 0 || pointer > static_cast<int>(rom_.size())) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "No valid pointer at 0x%x.", offset);
        throw std::invalid_argument(buf);
    }
    return pointer;
}

int Gen3RomHandler::find(std::string_view hex) const {
    if (hex.size() % 2 != 0) return -3;
    std::vector<uint8_t> needle(hex.size() / 2);
    for (size_t i = 0; i < needle.size(); i++) needle[i] = static_cast<uint8_t>(hexDigit(hex[2 * i]) * 16 + hexDigit(hex[2 * i + 1]));
    // RomFunctions.search: coincidencias sin solapar
    int found = -1, count = 0;
    auto it = rom_.begin();
    while (true) {
        it = std::search(it, rom_.end(), needle.begin(), needle.end());
        if (it == rom_.end()) break;
        if (count++ == 0) found = static_cast<int>(it - rom_.begin());
        it += static_cast<long>(needle.size());
    }
    if (count == 0) return -1;
    if (count > 1) return -2;
    return found;
}

bool Gen3RomHandler::matches(int offset, const uint8_t* sig, size_t len) const {
    for (size_t i = 0; i < len; i++) {
        if (offset + i >= rom_.size() || rom_[offset + i] != sig[i]) return false;
    }
    return true;
}

void Gen3RomHandler::loadItems() {
    auto& e = *romEntry_;
    int nameOffs = e.getInt("ItemData");
    int structLen = e.getInt("ItemEntrySize");
    int internalCount = e.getInt("ItemCount");
    int lastItemID = getLastItemID(e.romType());
    items_.assign(lastItemID + 1, std::nullopt);
    for (int internal = 1; internal <= internalCount; internal++) {
        int id = itemIDToStandard(internal);
        Item it;
        it.id = id;
        it.name = readVariableLengthString(nameOffs + structLen * internal);
        items_.at(id) = it;
    }
    for (int id : bannedItems()) {
        if (id < static_cast<int>(items_.size()) && items_[id]) items_[id]->allowed = false;
    }
    for (int i = items::tm01; i < items::tm01 + tmCount; i++) items_.at(i).value().tm = true;
    for (int id : badItems(e.romType())) {
        if (id < static_cast<int>(items_.size())) items_[id].value().bad = true;
    }
}

void Gen3RomHandler::loadPokemonNames() {
    auto& e = *romEntry_;
    int offs = e.getInt("PokemonNames");
    int nameLen = e.getInt("PokemonNameLength");
    int n = e.getInt("PokemonCount");
    pokeNames_.assign(n + 1, "");
    for (int i = 1; i <= n; i++) pokeNames_[i] = readFixedLengthString(offs + i * nameLen, nameLen);
}

void Gen3RomHandler::loadPokedexOrder() {
    auto& e = *romEntry_;
    int pdOffset = e.getInt("PokedexOrder");
    int n = e.getInt("PokemonCount");
    int maxPokedex = 0;
    internalToPokedex_.assign(n + 1, 0);
    pokedexToInternal_.assign(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        int dex = readWord(pdOffset + (i - 1) * 2);
        if (dex != 0) {
            internalToPokedex_[i] = dex;
            if (pokedexToInternal_.at(dex) == 0) pokedexToInternal_[dex] = i;
            maxPokedex = std::max(maxPokedex, dex);
        }
    }
    if (maxPokedex == unhackedMaxPokedex) {
        int offs = e.getInt("PokemonStats");
        int usedSlots = 0;
        for (int i = 0; i < unhackedMaxPokedex - unhackedRealPokedex; i++) {
            int slot = hoennPokesStart + i;
            int pokeOffs = offs + slot * baseStatsEntrySize;
            std::string lower = jc::toLower(pokeNames_.at(slot));
            if (!matches(pokeOffs, emptyPokemonSig.data(), emptyPokemonSig.size()) && !jc::contains(lower, "unused") &&
                lower != "?" && lower != "-") {
                usedSlots++;
                pokedexToInternal_.at(unhackedRealPokedex + usedSlots) = slot;
                internalToPokedex_[slot] = unhackedRealPokedex + usedSlots;
            } else {
                internalToPokedex_[slot] = 0;
            }
        }
        for (int i = usedSlots + 1; i <= unhackedMaxPokedex - unhackedRealPokedex; i++) {
            pokedexToInternal_.at(unhackedRealPokedex + i) = 0;
        }
        if (usedSlots > 0) isRomHack_ = true;
        pokedexCount_ = unhackedRealPokedex + usedSlots;
    } else {
        isRomHack_ = true;
        pokedexCount_ = maxPokedex;
    }
}

void Gen3RomHandler::loadSpeciesStats() {
    loadPokemonNames();
    loadPokedexOrder();
    auto& e = *romEntry_;
    pokes_.assign(pokedexCount_ + 1, nullptr);
    int n = e.getInt("PokemonCount");
    pokesInternal_.assign(n + 1, nullptr);
    speciesStorage_.clear();
    int offs = e.getInt("PokemonStats");
    for (int i = 1; i <= n; i++) {
        speciesStorage_.push_back(std::make_unique<Species>(internalToPokedex_[i]));
        Species* pk = speciesStorage_.back().get();
        pk->setName(pokeNames_[i]);
        if (pk->number() != 0) pokes_.at(pk->number()) = pk;
        pokesInternal_[i] = pk;
        loadBasicPokeStats(*pk, offs + i * baseStatsEntrySize);
        pk->setGeneration(generationOf(*pk));
    }
    // Las formas alternativas de Deoxys tienen stats fijos en el código; FVX usa esos.
    if (e.romType() == RomType_FRLG || e.romType() == RomType_Em) {
        std::string prefix = e.getString("DeoxysStatPrefix");
        int offset = find(prefix);
        if (offset > 0) {
            offset += static_cast<int>(prefix.size() / 2);
            BaseStats bs;
            bs.hp = readWord(offset) & 0xFF;
            bs.attack = readWord(offset + 2) & 0xFF;
            bs.defense = readWord(offset + 4) & 0xFF;
            bs.spatk = readWord(offset + 8) & 0xFF;
            bs.spdef = readWord(offset + 10) & 0xFF;
            bs.speed = readWord(offset + 6) & 0xFF;
            pokes_.at(species::deoxys)->setBaseStats(bs);
        }
    }
    constructPokemonList();
}

void Gen3RomHandler::constructPokemonList() {
    if (!isRomHack_) {
        speciesList_ = pokes_;
    } else {
        speciesList_.assign(1, nullptr);
        for (size_t i = 1; i < pokes_.size(); i++) {
            Species* pk = pokes_[i];
            if (pk) {
                std::string lower = jc::toLower(pk->name());
                if (!jc::contains(lower, "unused") && lower != "?") speciesList_.push_back(pk);
            }
        }
    }
    numRealPokemon_ = static_cast<int>(speciesList_.size()) - 1;
}

int Gen3RomHandler::generationOf(const Species& pk) const {
    if (pk.number() >= species::treecko) return 3;
    if (pk.number() >= species::chikorita) return 2;
    return 1;
}

void Gen3RomHandler::loadBasicPokeStats(Species& pk, int o) {
    BaseStats bs;
    bs.shedinja = pk.number() == species::shedinja;
    bs.hp = bs.shedinja ? 0 : rom_.at(o + bsHPOffset);
    bs.attack = rom_.at(o + bsAttackOffset);
    bs.defense = rom_.at(o + bsDefenseOffset);
    bs.spatk = rom_.at(o + bsSpAtkOffset);
    bs.spdef = rom_.at(o + bsSpDefOffset);
    bs.speed = rom_.at(o + bsSpeedOffset);
    pk.setBaseStats(bs);

    pk.setPrimaryType(typeFromByte(rom_.at(o + bsPrimaryTypeOffset)));
    pk.setSecondaryType(typeFromByte(rom_.at(o + bsSecondaryTypeOffset)));
    if (pk.secondaryType() == pk.primaryType()) pk.setSecondaryType(std::nullopt);
    pk.catchRate = rom_.at(o + bsCatchRateOffset);
    pk.evYield = EVYield::fromWord(readWord(o + bsEVYieldOffset));
    pk.growthCurve = expCurveFromByte(rom_.at(o + bsGrowthCurveOffset));
    pk.ability1 = rom_.at(o + bsAbility1Offset);
    pk.ability2 = rom_.at(o + bsAbility2Offset);

    auto itemAt = [&](int off) -> std::optional<int> {
        int id = itemIDToStandard(readWord(off));
        const auto& it = items_.at(id);
        return it ? std::optional<int>(it->id) : std::nullopt;
    };
    auto item1 = itemAt(o + bsCommonHeldItemOffset);
    auto item2 = itemAt(o + bsRareHeldItemOffset);
    if (item1 == item2) {
        pk.guaranteedHeldItem = item1;
    } else {
        pk.commonHeldItem = item1;
        pk.rareHeldItem = item2;
    }
    pk.genderRatio = rom_.at(o + bsGenderRatioOffset);
    pk.breedingInfo.primary = eggGroupFromID(rom_.at(o + bsPrimaryEggGroupOffset));
    EggGroup second = eggGroupFromID(rom_.at(o + bsSecondaryEggGroupOffset));
    // como el constructor de BreedingInfo: si coincide con el primario, no hay secundario
    pk.breedingInfo.secondary = second == pk.breedingInfo.primary ? std::nullopt : std::optional<EggGroup>(second);
    pk.breedingInfo.eggCycles = rom_.at(o + bsEggCyclesOffset);
}

void Gen3RomHandler::loadAbilityNames() {
    auto& e = *romEntry_;
    int offs = e.getInt("AbilityNames");
    int len = e.getInt("AbilityNameLength");
    abilityNames_.assign(gen3::highestAbilityIndex + 1, "");
    for (int i = 0; i <= gen3::highestAbilityIndex; i++) abilityNames_[i] = readFixedLengthString(offs + len * i, len);
}

}  // namespace pr::gen3
