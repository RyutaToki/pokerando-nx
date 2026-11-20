// Iniciales, estáticos/errantes, MT/MO, tutores y textos de eventos de Gen 3
// (port de las partes correspondientes de Gen3RomHandler).
#include <algorithm>
#include <cstring>

#include "constants/item_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"
#include "romio/rom_functions.hpp"

namespace pr::gen3 {

// ---------------- iniciales ----------------

std::vector<Species*> Gen3RomHandler::getStarters() const {
    int base = romEntry_->getInt("StarterPokemon");
    bool rse = romEntry_->romType() != RomType_FRLG;
    return {pokesInternal_.at(readWord(base)),
            pokesInternal_.at(readWord(base + (rse ? rseStarter2Offset : frlgStarter2Offset))),
            pokesInternal_.at(readWord(base + (rse ? rseStarter3Offset : frlgStarter3Offset)))};
}

bool Gen3RomHandler::setStarters(const std::vector<Species*>& starters) {
    if (starters.size() != 3) throw std::invalid_argument("Wrong amount of starters, must be 3.");
    attemptObedienceEvolutionPatches();
    int base = romEntry_->getInt("StarterPokemon");
    int s0 = pokedexToInternal_[starters[0]->number()];
    int s1 = pokedexToInternal_[starters[1]->number()];
    int s2 = pokedexToInternal_[starters[2]->number()];
    if (romEntry_->romType() != RomType_FRLG) {
        writeWord(base, s0);
        writeWord(base + rseStarter2Offset, s1);
        writeWord(base + rseStarter3Offset, s2);
    } else {
        writeWord(base, s0);
        writeWord(base + frlgStarterRepeatOffset, s1);
        writeWord(base + frlgStarter2Offset, s1);
        writeWord(base + frlgStarter2Offset + frlgStarterRepeatOffset, s2);
        writeWord(base + frlgStarter3Offset, s2);
        writeWord(base + frlgStarter3Offset + frlgStarterRepeatOffset, s0);
    }
    writeEventText(romEntry_->starterTexts(), [&](int id) {
        Species* st = starters.at(id);
        auto t = st->primaryType();
        if (t == Type::NORMAL && st->secondaryType()) t = st->secondaryType();
        return std::vector<std::string>{st->name(), typeName(*t)};
    }, {"[starter]", "[type]"}, "Starter");
    return true;
}

std::vector<std::optional<int>> Gen3RomHandler::getStarterHeldItems() const {
    if (romEntry_->romType() == RomType_FRLG) {
        int base = romEntry_->getInt("StarterPokemon");
        return {itemAt(readWord(base + frlgStarterItemsOffset))};
    }
    int base = romEntry_->getInt("StarterItems");
    int i1 = rom_.at(base), i2 = rom_.at(base + 2);
    return {itemAt(i2 == 0 ? i1 : i2 + 0xFF)};
}

void Gen3RomHandler::setStarterHeldItems(const std::vector<std::optional<int>>& items) {
    if (items.size() != 1) return;
    int internalID = items[0] ? itemIDToInternal(*items[0]) : 0;
    if (romEntry_->romType() == RomType_FRLG) {
        writeWord(romEntry_->getInt("StarterPokemon") + frlgStarterItemsOffset, internalID);
    } else {
        int base = romEntry_->getInt("StarterItems");
        if (internalID <= 0xFF) {
            rom_[base] = static_cast<uint8_t>(internalID & 0xFF);
            rom_[base + 2] = 0;
        } else {
            rom_[base] = 0xFF;
            rom_[base + 2] = static_cast<uint8_t>((internalID - 0xFF) & 0xFF);
        }
        rom_[base + 3] = gbaAddRxOpcode | gbaR2;
    }
}

// ---------------- estáticos ----------------

std::vector<StaticEncounter> Gen3RomHandler::getStaticPokemon() {
    std::vector<StaticEncounter> statics;
    const auto& here = romEntry_->staticPokemon();
    std::vector<int> eggOffsets = romEntry_->getArray("StaticEggPokemonOffsets");
    for (size_t i = 0; i < here.size(); i++) {
        StaticEncounter se(pokesInternal_.at(readWord(here[i].speciesOffsets.at(0))));
        se.level = staticLevel(here[i], 0);
        se.isEgg = std::find(eggOffsets.begin(), eggOffsets.end(), static_cast<int>(i)) != eggOffsets.end();
        statics.push_back(se);
    }
    if (romEntry_->hasTweakFile("StaticFirstBattleTweak")) {
        // combate inicial contra Poochyena/Zigzagoon en RZE
        int speciesOffset = romEntry_->getInt("StaticFirstBattleSpeciesOffset");
        int sp = readWord(speciesOffset);
        if (sp == 0xFFFF) {
            romfn::applyPatch(rom_, romEntry_->getTweakFile("StaticFirstBattleTweak"));
            sp = readWord(speciesOffset);
        }
        StaticEncounter se(pokesInternal_.at(sp));
        se.level = static_cast<int8_t>(rom_.at(romEntry_->getInt("StaticFirstBattleLevelOffset")));
        statics.push_back(se);
    } else if (romEntry_->hasTweakFile("GhostMarowakTweak")) {
        std::vector<int> offs = romEntry_->getArray("GhostMarowakSpeciesOffsets");
        int sp = readWord(offs.at(0));
        if (sp == 0xFFFF) {
            romfn::applyPatch(rom_, romEntry_->getTweakFile("GhostMarowakTweak"));
            sp = readWord(offs.at(0));
        }
        StaticEncounter se(pokesInternal_.at(sp));
        se.level = static_cast<int8_t>(rom_.at(romEntry_->getArray("GhostMarowakLevelOffsets").at(0)));
        statics.push_back(se);
    }
    getRoamers(statics);
    return statics;
}

int Gen3RomHandler::staticLevel(const StaticPokemonOffsets& sp, int i) const {
    if (static_cast<int>(sp.levelOffsets.size()) <= i) return 1;
    return static_cast<int8_t>(rom_.at(sp.levelOffsets[i]));
}

void Gen3RomHandler::setStaticSpecies(const StaticPokemonOffsets& sp, Species* s) {
    int value = pokedexToInternal_[s->number()];
    for (int off : sp.speciesOffsets) writeWord(off, value);
}

void Gen3RomHandler::setStaticLevel(const StaticPokemonOffsets& sp, int level, int i) {
    // puede no tener nivel (p. ej. un huevo)
    if (static_cast<int>(sp.levelOffsets.size()) > i) rom_.at(sp.levelOffsets[i]) = static_cast<uint8_t>(level);
}

bool Gen3RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& statics) {
    attemptObedienceEvolutionPatches();
    const auto& here = romEntry_->staticPokemon();
    int roamerSize = static_cast<int>(romEntry_->roamingPokemon().size());
    // en Esmeralda los errantes están enlazados a los de Isla Sur y no cuentan
    if (romEntry_->romType() == RomType_Em) roamerSize = 0;
    int hardcoded = (romEntry_->hasTweakFile("StaticFirstBattleTweak") || romEntry_->hasTweakFile("GhostMarowakTweak")) ? 1 : 0;
    if (statics.size() != here.size() + hardcoded + roamerSize) return false;
    for (size_t i = 0; i < here.size(); i++) {
        setStaticSpecies(here[i], statics[i].species());
        setStaticLevel(here[i], statics[i].level, 0);
    }
    if (romEntry_->hasTweakFile("StaticFirstBattleTweak")) {
        const auto& fb = statics.at(romEntry_->getInt("StaticFirstBattleOffset"));
        writeWord(romEntry_->getInt("StaticFirstBattleSpeciesOffset"), pokedexToInternal_[fb.species()->number()]);
        writeByte(romEntry_->getInt("StaticFirstBattleLevelOffset"), static_cast<uint8_t>(fb.level));
    } else if (romEntry_->hasTweakFile("GhostMarowakTweak")) {
        const auto& gm = statics.at(romEntry_->getInt("GhostMarowakOffset"));
        for (int off : romEntry_->getArray("GhostMarowakSpeciesOffsets")) writeWord(off, pokedexToInternal_[gm.species()->number()]);
        for (int off : romEntry_->getArray("GhostMarowakLevelOffsets")) writeByte(off, static_cast<uint8_t>(gm.level));
        // el juego intenta que sea hembra; si no puede serlo se quedaría en bucle
        int gr = gm.species()->genderRatio;
        if (gr == 0 || gr == 0xFF) writeByte(romEntry_->getInt("GhostMarowakGenderOffset"), static_cast<uint8_t>(gr));
    }
    setRoamers(statics);
    return true;
}

void Gen3RomHandler::getRoamers(std::vector<StaticEncounter>& statics) {
    const auto& roamers = romEntry_->roamingPokemon();
    auto make = [&](const StaticPokemonOffsets& r) {
        StaticEncounter se(pokesInternal_.at(readWord(r.speciesOffsets.at(0))));
        se.level = staticLevel(r, 0);
        return se;
    };
    switch (romEntry_->romType()) {
        case RomType_Ruby:
            if (readWord(roamers.at(0).speciesOffsets.at(0)) == 0) applyRubyRoamerPatch();
            statics.push_back(make(roamers.at(0)));
            break;
        case RomType_Sapp:
            statics.push_back(make(roamers.at(0)));
            break;
        case RomType_FRLG:
            if (!romEntry_->hasTweakFile("RoamingPokemonTweak")) break;
            if (readWord(roamers.at(0).speciesOffsets.at(0)) == 0xFFFF)
                romfn::applyPatch(rom_, romEntry_->getTweakFile("RoamingPokemonTweak"));
            for (const auto& r : roamers) statics.push_back(make(r));
            break;
        case RomType_Em: {
            if (readWord(roamers.at(0).speciesOffsets.at(0)) >= static_cast<int>(pokesInternal_.size())) applyEmeraldRoamerPatch();
            std::vector<int> southern = romEntry_->getArray("StaticSouthernIslandOffsets");
            for (size_t i = 0; i < roamers.size(); i++) {
                // enlazar cada errante con el estático de Isla Sur para que salgan iguales
                statics.at(southern.at(i)).linkedEncounters.push_back(make(roamers[i]));
            }
            break;
        }
    }
}

void Gen3RomHandler::setRoamers(const std::vector<StaticEncounter>& statics) {
    const auto& roamers = romEntry_->roamingPokemon();
    auto apply = [&](const StaticPokemonOffsets& r, const StaticEncounter& se) {
        setStaticSpecies(r, se.species());
        for (size_t j = 0; j < r.levelOffsets.size(); j++) setStaticLevel(r, se.level, static_cast<int>(j));
    };
    int rt = romEntry_->romType();
    if (rt == RomType_Ruby || rt == RomType_Sapp) {
        apply(roamers.at(0), statics.back());
    } else if (rt == RomType_FRLG && romEntry_->hasTweakFile("RoamingPokemonTweak")) {
        for (size_t i = 0; i < roamers.size(); i++) apply(roamers[i], statics.at(statics.size() - 3 + i));
    } else if (rt == RomType_Em) {
        std::vector<int> southern = romEntry_->getArray("StaticSouthernIslandOffsets");
        for (size_t i = 0; i < roamers.size(); i++) apply(roamers[i], statics.at(southern.at(i)).linkedEncounters.at(0));
    }
}

void Gen3RomHandler::writeFullInt(int offset, int value) {
    rom_.at(offset) = static_cast<uint8_t>(value & 0xFF);
    rom_.at(offset + 1) = static_cast<uint8_t>((value >> 8) & 0xFF);
    rom_.at(offset + 2) = static_cast<uint8_t>((value >> 16) & 0xFF);
    rom_.at(offset + 3) = static_cast<uint8_t>((value >> 24) & 0xFF);
}

void Gen3RomHandler::applyRubyRoamerPatch() {
    int offset = romEntry_->getInt("FindMapsWithMonFunctionStartOffset");
    writeByte(offset + 22, 0x57);
    writeFullInt(offset + 128, pokedexToInternal_[species::latios]);
    writeBytes(offset + 12, {0x00, 0x00, 0x1C, 0x48});
    offset = romEntry_->getInt("CreateInitialRoamerMonFunctionStartOffset");
    writeBytes(offset + 182, {0x00, 0xBD});
    writeFullInt(offset + 184, pokedexToInternal_[species::latios]);
    writeBytes(offset + 10, {0x00, 0x00, 0x2A, 0x4E});
}

void Gen3RomHandler::applyEmeraldRoamerPatch() {
    int offset = romEntry_->getInt("CreateInitialRoamerMonFunctionStartOffset");
    writeByte(offset + 14, 0x41);
    writeFullInt(offset + 28, pokedexToInternal_[species::latios]);
    writeBytes(offset + 8, {0x00, 0x28, 0x04, 0x4B});
    writeBytes(offset + 48, {0x1A, 0x46, 0x00, 0x00});
}

// ---------------- MT / MO / tutores ----------------

std::vector<int> Gen3RomHandler::getTMMoves() const {
    std::vector<int> tms;
    int offset = romEntry_->getInt("TmMoves");
    for (int i = 1; i <= tmCount; i++) tms.push_back(readWord(offset + (i - 1) * 2));
    return tms;
}

std::vector<int> Gen3RomHandler::getHMMoves() const { return hmMoves; }

void Gen3RomHandler::setTMMoves(const std::vector<int>& moveIndexes) {
    // writeTMMoves
    int offset = romEntry_->getInt("TmMoves");
    for (int i = 1; i <= tmCount; i++) writeWord(offset + (i - 1) * 2, moveIndexes.at(i - 1));
    int other = romEntry_->getInt("TmMovesDuplicate");
    if (other > 0) std::memmove(rom_.data() + other, rom_.data() + offset, tmCount * 2);  // E/RF/VH tienen dos tablas
    // writeTMItemPalettes
    int ii = romEntry_->getInt("ItemImages");
    if (ii > 0) {
        std::vector<int> pals = romEntry_->getArray("TmPals");
        for (int i = 0; i < tmCount; i++) {
            int typeID = typeToByte(moves_.at(moveIndexes[i])->type);
            writePointer(ii + itemIDToInternal(items::tm01 + i) * 8 + 4, pals.at(typeID));
        }
    }
    // writeTMItemText
    if (romEntry_->getInt("MoveDescriptions") > 0) {
        int idOffset = romEntry_->getInt("ItemData");
        int mdOffset = romEntry_->getInt("MoveDescriptions");
        int entrySize = romEntry_->getInt("ItemEntrySize");
        for (int i = 0; i < tmCount; i++) {
            int itemDescPtr = idOffset + itemIDToInternal(items::tm01 + i) * entrySize + itemDataDescriptionOffset;
            int moveDescPtr = mdOffset + (moveIndexes[i] - 1) * 4;
            if (romEntry_->romType() == RomType_FRLG) {
                // en RF/VH se comparten los textos de movimiento y de MT
                writePointer(itemDescPtr, readPointer(moveDescPtr));
            } else {
                std::string moveDesc = readVariableLengthString(readPointer(moveDescPtr));
                std::string newDesc = romfn::rewriteDescriptionForNewLineSize(moveDesc, "\\n", rseItemDescCharsPerLine, ssd());
                rewriteVariableLengthString(itemDescPtr, newDesc);
            }
        }
    }
    // writeTMText
    writeEventText(romEntry_->tmTexts(), [&](int id) {
        return std::vector<std::string>{moves_.at(moveIndexes.at(id - 1))->name};
    }, {"[move]"}, "TM");
}

std::map<int, std::vector<bool>> Gen3RomHandler::getTMHMCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    int offset = romEntry_->getInt("PokemonTMHMCompat");
    for (int i = 1; i <= numRealPokemon_; i++) {
        const Species* pk = speciesList_[i];
        int co = offset + pokedexToInternal_[pk->number()] * 8;
        std::vector<bool> flags(tmCount + hmCount + 1);
        for (int j = 0; j < 8; j++) readByteIntoFlags(flags, j * 8 + 1, co + j);
        compat[pk->number()] = flags;
    }
    return compat;
}

void Gen3RomHandler::setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) {
    int offset = romEntry_->getInt("PokemonTMHMCompat");
    for (const auto& [num, flags] : data) {
        int co = offset + pokedexToInternal_[num] * 8;
        for (int j = 0; j < 8; j++) writeByte(co + j, getByteFromFlags(flags, j * 8 + 1));
    }
}

bool Gen3RomHandler::hasMoveTutors() const {
    return romEntry_->romType() == RomType_Em || romEntry_->romType() == RomType_FRLG;
}

std::vector<int> Gen3RomHandler::getMoveTutorMoves() const {
    if (!hasMoveTutors()) return {};
    std::vector<int> mts;
    int count = romEntry_->getInt("MoveTutorMoves");
    int offset = romEntry_->getInt("MoveTutorData");
    for (int i = 0; i < count; i++) mts.push_back(readWord(offset + i * 2));
    return mts;
}

void Gen3RomHandler::setMoveTutorMoves(const std::vector<int>& moveIndexes) {
    if (!hasMoveTutors()) return;
    int count = romEntry_->getInt("MoveTutorMoves");
    int offset = romEntry_->getInt("MoveTutorData");
    if (count != static_cast<int>(moveIndexes.size())) throw std::invalid_argument("Wrong amount of move tutor moves.");
    for (int i = 0; i < count; i++) writeWord(offset + i * 2, moveIndexes[i]);
    writeEventText(romEntry_->moveTutorTexts(), [&](int id) {
        return std::vector<std::string>{moves_.at(moveIndexes.at(id))->name};
    }, {"[move]"}, "MoveTutor");
}

std::map<int, std::vector<bool>> Gen3RomHandler::getMoveTutorCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    if (!hasMoveTutors()) return compat;
    int count = romEntry_->getInt("MoveTutorMoves");
    int offset = romEntry_->getInt("MoveTutorCompatibility");
    int bytes = ((count + 7) & ~7) / 8;
    for (int i = 1; i <= numRealPokemon_; i++) {
        const Species* pk = speciesList_[i];
        int co = offset + pokedexToInternal_[pk->number()] * bytes;
        std::vector<bool> flags(count + 1);
        for (int j = 0; j < bytes; j++) readByteIntoFlags(flags, j * 8 + 1, co + j);
        compat[pk->number()] = flags;
    }
    return compat;
}

void Gen3RomHandler::setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) {
    if (!hasMoveTutors()) return;
    int count = romEntry_->getInt("MoveTutorMoves");
    int offset = romEntry_->getInt("MoveTutorCompatibility");
    int bytes = ((count + 7) & ~7) / 8;
    for (const auto& [num, flags] : data) {
        int co = offset + pokedexToInternal_[num] * bytes;
        for (int j = 0; j < bytes; j++) writeByte(co + j, getByteFromFlags(flags, j * 8 + 1));
    }
}

void Gen3RomHandler::readByteIntoFlags(std::vector<bool>& flags, int offsetIntoFlags, int offsetIntoROM) const {
    int b = rom_.at(offsetIntoROM);
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++)
        flags[offsetIntoFlags + i] = ((b >> i) & 1) == 1;
}

uint8_t Gen3RomHandler::getByteFromFlags(const std::vector<bool>& flags, int offsetIntoFlags) const {
    int b = 0;
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++)
        b |= (flags[offsetIntoFlags + i] ? 1 : 0) << i;
    return static_cast<uint8_t>(b);
}

// ---------------- textos ----------------

romfn::StringSizeDeterminer Gen3RomHandler::ssd() const {
    return [this](const std::string& s) { return static_cast<int>(translateString(s).size()); };
}

std::vector<uint8_t> Gen3RomHandler::variableLengthStringToBytes(const std::string& s) const {
    auto t = translateString(s);
    t.push_back(0xFF);
    return t;
}

int Gen3RomHandler::lengthOfStringAt(int offset) const {
    int len = 0;
    while (rom_.at(offset + len) != textTerminator) len++;
    return len + 1;
}

void Gen3RomHandler::rewriteVariableLengthString(int pointerOffset, const std::string& s,
                                                 const std::vector<int>& secondaryPointerOffsets) {
    rewriteData(pointerOffset, variableLengthStringToBytes(s), [this](int off) { return lengthOfStringAt(off); },
                secondaryPointerOffsets);
}

std::vector<int> Gen3RomHandler::searchForPointerCopies(int pointerOffset) const {
    std::vector<uint8_t> needle(rom_.begin() + pointerOffset, rom_.begin() + pointerOffset + 4);
    int minO = std::max(0, pointerOffset - pointerSearchRadius);
    int maxO = std::min(static_cast<int>(rom_.size()), pointerOffset + pointerSearchRadius);
    std::vector<int> found;
    auto it = rom_.cbegin() + minO;
    auto end = rom_.cbegin() + maxO;
    while (true) {
        it = std::search(it, end, needle.cbegin(), needle.cend());
        if (it == end) break;
        found.push_back(static_cast<int>(it - rom_.cbegin()));
        it += 4;
    }
    return found;
}

void Gen3RomHandler::writeEventText(const std::vector<EventTextEntry>& entries,
                                    const std::function<std::vector<std::string>(int)>& idToReplacers,
                                    const std::vector<std::string>& targets, const std::string& description) {
    for (const auto& ete : entries) {
        auto replacers = idToReplacers(ete.id);
        // HashMap<String, String> en Java: orden según String.hashCode
        JavaHashMap<std::string, std::string> map;
        for (size_t i = 0; i < targets.size(); i++) map.put(targets[i], romfn::replaceAll(replacers[i], " ", "_"));
        std::vector<std::pair<std::string, std::string>> repl;
        map.forEach([&](const std::string& k, const std::string& v) { repl.emplace_back(k, v); });
        std::string text = romfn::formatTextWithReplacements(ete.templateText, &repl, "\\n", "\\l", "\\p",
                                                             regularTextboxCharsPerLine, ssd());
        for (const auto& r : replacers) text = romfn::replaceAll(text, romfn::replaceAll(r, "_", " "), r);
        if (ete.actualPointerOffset == -1) throw std::logic_error("actualPointerOffset has not been set");
        int pointerOffset = ete.actualPointerOffset;
        try {
            rewriteVariableLengthString(pointerOffset, text, searchForPointerCopies(pointerOffset));
        } catch (const std::exception& e) {
            throw std::runtime_error("Couldn't insert new " + description + " text. " + e.what());
        }
    }
}

}  // namespace pr::gen3
