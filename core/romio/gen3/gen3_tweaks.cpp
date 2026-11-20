// Tabla de tipos, ajustes varios, evoluciones imposibles, Pokédex Nacional, introducción,
// tutorial de captura y poción del PC en Gen 3.
#include <algorithm>
#include <cstdio>

#include "constants/item_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

// ---------------- tabla de tipos ----------------

TypeTable Gen3RomHandler::getTypeTable() const {
    TypeTable t(TypeTable::vanillaGen2To5().types());
    int cur = romEntry_->getInt("TypeEffectivenessOffset");
    auto sb = [&](int o) { return static_cast<int>(static_cast<int8_t>(rom_.at(o))); };
    int attacking = sb(cur);
    while (attacking != static_cast<int8_t>(gb::typeTableTerminator)) {
        if (sb(cur) != static_cast<int8_t>(gb::typeTableForesightTerminator)) {
            int defending = sb(cur + 1);
            int effInternal = sb(cur + 2);
            std::optional<Effectiveness> e;
            switch (effInternal) {
                case 20: e = Effectiveness::DOUBLE; break;
                case 10: e = Effectiveness::NEUTRAL; break;
                case 5: e = Effectiveness::HALF; break;
                case 0: e = Effectiveness::ZERO; break;
            }
            if (e) t.setEffectiveness(*typeFromByte(attacking), *typeFromByte(defending), *e);
        }
        cur += 3;
        attacking = sb(cur);
    }
    return t;
}

void Gen3RomHandler::setTypeTable(const TypeTable& t) {
    if (t.nonNeutralEffectivenessCount() > nonNeutralEffectivenessCount) {
        throw std::invalid_argument("Too many non-neutral Effectiveness-es. Was " +
                                    std::to_string(t.nonNeutralEffectivenessCount()) + ", has to be at most " +
                                    std::to_string(nonNeutralEffectivenessCount));
    }
    int offset = romEntry_->getInt("TypeEffectivenessOffset");
    std::vector<uint8_t> mainPart, ghost;
    for (Type a : t.types()) {
        for (Type d : t.types()) {
            Effectiveness e = t.getEffectiveness(a, d);
            if (e == Effectiveness::NEUTRAL) continue;
            auto& part = (d == Type::GHOST && e == Effectiveness::ZERO) ? ghost : mainPart;
            uint8_t internal = e == Effectiveness::DOUBLE ? 20 : (e == Effectiveness::HALF ? 5 : 0);
            part.push_back(static_cast<uint8_t>(typeToByte(a)));
            part.push_back(static_cast<uint8_t>(typeToByte(d)));
            part.push_back(internal);
        }
    }
    writeBytes(offset, mainPart);
    offset += static_cast<int>(mainPart.size());
    writeBytes(offset, {gb::typeTableForesightTerminator, gb::typeTableForesightTerminator, 0x00});
    offset += 3;
    if (!ghost.empty()) writeBytes(offset, ghost);
    offset += static_cast<int>(ghost.size());
    writeBytes(offset, {gb::typeTableTerminator, gb::typeTableTerminator, 0x00});
}

// ---------------- evoluciones ----------------

void Gen3RomHandler::removeImpossibleEvolutions(bool /*changeMoveEvos*/, bool useEstimatedLevels) {
    attemptObedienceEvolutionPatches();
    using ET = EvolutionType;
    for (Species* pk : pokes_) {
        if (!pk) continue;
        for (const auto& evo : pk->evolutionsFrom()) {
            // no son intercambios, pero son imposibles sin intercambiar
            if (romEntry_->romType() == RomType_FRLG) {
                if (evo->type() == ET::HAPPINESS_DAY) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::STONE, items::sunStone, useEstimatedLevels);
                }
                if (evo->type() == ET::HAPPINESS_NIGHT) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::STONE, items::moonStone, useEstimatedLevels);
                }
                if (evo->type() == ET::HIGH_BEAUTY) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::LEVEL, 35, useEstimatedLevels);
                }
            }
            if (evo->type() == ET::TRADE) {
                markImprovedEvolutions(pk);
                evo->updateEvolutionMethod(ET::LEVEL, 37, useEstimatedLevels);
            }
            if (evo->type() == ET::TRADE_ITEM) {
                markImprovedEvolutions(pk);
                int from = evo->from()->number();
                if (from == species::poliwhirl) {
                    evo->updateEvolutionMethod(ET::LEVEL, 37, useEstimatedLevels);
                } else if (from == species::slowpoke) {
                    evo->updateEvolutionMethod(ET::STONE, items::waterStone, useEstimatedLevels);
                } else if (from == species::seadra) {
                    evo->updateEvolutionMethod(ET::LEVEL, 40, useEstimatedLevels);
                } else if (from == species::clamperl && evo->extraInfo() == items::deepSeaTooth) {
                    evo->updateEvolutionMethod(ET::LEVEL, 30, useEstimatedLevels);
                } else if (from == species::clamperl && evo->extraInfo() == items::deepSeaScale) {
                    evo->updateEvolutionMethod(ET::STONE, items::waterStone, useEstimatedLevels);
                } else {
                    evo->updateEvolutionMethod(ET::LEVEL, 30, useEstimatedLevels);  // Onix, Scyther, Porygon
                }
            }
        }
    }
}

void Gen3RomHandler::makeEvolutionsEasier(bool /*changeWithOtherEvos*/, bool /*useEstimatedLevels*/) {
    // reducir la amistad necesaria para evolucionar
    int offset = find(friendshipValueForEvoLocator);
    if (offset <= 0) return;
    auto vanilla = static_cast<uint8_t>(global::vanillaHappinessToEvolve - 1);
    auto easier = static_cast<uint8_t>(global::easierHappinessToEvolve - 1);
    if (rom_.at(offset) == vanilla) writeByte(offset, easier);
    if (romEntry_->romType() != RomType_FRLG) {  // RF/VH no tienen evoluciones por hora
        if (rom_.at(offset + 38) == vanilla) writeByte(offset + 38, easier);
        if (rom_.at(offset + 66) == vanilla) writeByte(offset + 66, easier);
    }
}

void Gen3RomHandler::removeEvosForPokemonPool(const SpeciesSet& included) {
    for (Species* pk : pokes_) {
        if (!pk) continue;
        std::vector<EvolutionPtr> keep;
        for (const auto& evo : pk->evolutionsFrom()) {
            if (included.contains(evo->from()) && included.contains(evo->to())) {
                keep.push_back(evo);
            } else {
                auto& to = evo->to()->evolutionsTo();
                auto it = std::find_if(to.begin(), to.end(), [&](const EvolutionPtr& e) { return e->sameAs(*evo); });
                if (it != to.end()) to.erase(it);  // List.remove(Object): primera coincidencia por equals
            }
        }
        // retainAll(keepEvos): conserva las que sean "equals" a alguna de keep
        auto& from = pk->evolutionsFrom();
        from.erase(std::remove_if(from.begin(), from.end(),
                                  [&](const EvolutionPtr& e) {
                                      return std::none_of(keep.begin(), keep.end(),
                                                          [&](const EvolutionPtr& k) { return k->sameAs(*e); });
                                  }),
                   from.end());
    }
}

// ---------------- ajustes ----------------

int Gen3RomHandler::miscTweaksAvailable() const {
    using namespace misctweak;
    int a = LOWER_CASE_POKEMON_NAMES.value;
    if (romEntry_->getInt("NationalDexTweakPossible") != 0) a |= NATIONAL_DEX_AT_START.value;
    if (romEntry_->getInt("RunIndoorsTweakOffset") > 0) a |= RUNNING_SHOES_INDOORS.value;
    if (romEntry_->getInt("TextSpeedValuesOffset") > 0 || romEntry_->hasTweakFile("InstantTextTweak")) a |= FASTEST_TEXT.value;
    if (romEntry_->getInt("CatchingTutorialOpponentMonOffset") > 0 || romEntry_->getInt("CatchingTutorialPlayerMonOffset") > 0)
        a |= RANDOMIZE_CATCHING_TUTORIAL.value;
    if (romEntry_->getInt("PCPotionOffset") != 0) a |= RANDOMIZE_PC_POTION.value;
    a |= BAN_LUCKY_EGG.value;
    a |= RUN_WITHOUT_RUNNING_SHOES.value;
    if (romEntry_->romType() == RomType_FRLG) a |= BALANCE_STATIC_LEVELS.value;
    if (!romEntry_->getArray("TMMovesReusableFunctionOffsets").empty()) a |= REUSABLE_TMS.value;
    if (!romEntry_->getArray("HMMovesForgettableFunctionOffsets").empty()) a |= FORGETTABLE_HMS.value;
    a |= FAST_EGG_HATCHING.value;
    a |= NO_EV_YIELDS.value;
    return a;
}

void Gen3RomHandler::applyMiscTweak(const MiscTweak& t) {
    using namespace misctweak;
    if (t == RUNNING_SHOES_INDOORS) {
        if (int o = romEntry_->getInt("RunIndoorsTweakOffset"); o != 0) writeByte(o, 0x00);
    } else if (t == FASTEST_TEXT) {
        if (romEntry_->hasTweakFile("InstantTextTweak")) {
            romfn::applyPatch(rom_, romEntry_->getTweakFile("InstantTextTweak"));
        } else if (int o = romEntry_->getInt("TextSpeedValuesOffset"); o > 0) {
            writeBytes(o, {4, 1, 0});  // lento = medio, medio = rápido, rápido = instantáneo
        }
    } else if (t == LOWER_CASE_POKEMON_NAMES) {
        for (Species* pk : getSpeciesSet().toVector()) pk->setName(romfn::camelCase(pk->name()));
    } else if (t == NATIONAL_DEX_AT_START) {
        patchForNationalDex();
    } else if (t == BAN_LUCKY_EGG) {
        items_.at(items::luckyEgg).value().allowed = false;
    } else if (t == RUN_WITHOUT_RUNNING_SHOES) {
        int offset = find(getRunningShoesCheckPrefix());
        if (offset != 0) writeWord(offset + 0x12, 0);  // anula el salto que exige tener las Deportivas
    } else if (t == BALANCE_STATIC_LEVELS) {
        for (int o : romEntry_->getArray("FossilLevelOffsets")) writeWord(o, 30);
    } else if (t == REUSABLE_TMS) {
        auto hmCompare = static_cast<uint8_t>(itemIDToInternal(items::hm01) / 2);
        for (int o : romEntry_->getArray("TMMovesReusableFunctionOffsets")) {
            if (rom_.at(o) != hmCompare) throw std::runtime_error("TMMovesReusableFunctionOffsets parece incorrecto");
            writeByte(o, 0);
        }
        tmsReusable_ = true;
    } else if (t == FORGETTABLE_HMS) {
        for (int o : romEntry_->getArray("HMMovesForgettableFunctionOffsets")) {
            if (rom_.at(o) != 1) throw std::runtime_error("HMMovesForgettableFunctionOffsets parece incorrecto");
            writeByte(o, 0);
        }
    } else if (t == FAST_EGG_HATCHING) {
        for (Species* pk : getSpeciesSet().toVector()) pk->breedingInfo.eggCycles = 0;
    } else if (t == NO_EV_YIELDS) {
        for (Species* pk : getSpeciesSet().toVector()) pk->evYield = EVYield{};
    }
}

std::string_view Gen3RomHandler::getRunningShoesCheckPrefix() const {
    switch (romEntry_->romType()) {
        case RomType_Ruby:
        case RomType_Sapp: return runningShoesCheckPrefixRS;
        case RomType_FRLG: return runningShoesCheckPrefixFRLG;
        default: return runningShoesCheckPrefixE;
    }
}

void Gen3RomHandler::writeHexString(std::string_view hex, int offset) {
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        rom_.at(offset + i / 2) = static_cast<uint8_t>(std::stoi(std::string(hex.substr(i, 2)), nullptr, 16));
    }
}

std::vector<int> Gen3RomHandler::findMultiple(std::string_view hex) const {
    std::vector<uint8_t> needle;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) needle.push_back(static_cast<uint8_t>(std::stoi(std::string(hex.substr(i, 2)), nullptr, 16)));
    std::vector<int> out;
    auto it = rom_.cbegin();
    while (true) {
        it = std::search(it, rom_.cend(), needle.cbegin(), needle.cend());
        if (it == rom_.cend()) break;
        out.push_back(static_cast<int>(it - rom_.cbegin()));
        it += static_cast<long>(needle.size());
    }
    return out;
}

std::string Gen3RomHandler::pointerToHexString(int pointer) const {
    char hex[9];
    std::snprintf(hex, sizeof hex, "%08X", static_cast<unsigned>(pointer + 0x08000000));
    return {hex[6], hex[7], hex[4], hex[5], hex[2], hex[3], hex[0], hex[1]};
}

void Gen3RomHandler::patchForNationalDex() {
    auto fail = [](const std::string& m) { throw std::runtime_error("Patch for National Dex at Start of Game unsuccessful. " + m); };
    int rt = romEntry_->romType();
    if (rt == RomType_Ruby || rt == RomType_Sapp || rt == RomType_Em) {
        bool em = rt == RomType_Em;
        int pkDex = find(em ? ePokedexScriptIdentifier : rsPokedexScriptIdentifier);
        if (pkDex < 0) fail("Could not find script.");
        int textPointer = readPointer(pkDex - 4);
        int pointerLoc = find(pointerToHexString(pkDex - 8));
        if (pointerLoc < 0) fail("Could not find pointer to script.");
        int space = findAndUnfreeSpace(em ? eNatDexScriptLength : rsNatDexScriptLength);
        writePointer(pointerLoc, space);
        writeHexString(em ? eNatDexScriptPart1 : rsNatDexScriptPart1, space);
        writePointer(space + 4, textPointer);
        writeHexString(em ? eNatDexScriptPart2 : rsNatDexScriptPart2, space + 8);
    } else if (rt == RomType_FRLG) {
        int pkDex = find(frlgPokedexScriptIdentifier);
        if (pkDex < 0) fail("Could not find script.");
        int space = findAndUnfreeSpace(frlgNatDexScriptLength);
        writeByte(pkDex, 4);
        writePointer(pkDex + 1, space);
        writeByte(pkDex + 5, 0);  // NOP
        writeHexString(frlgNatDexScript, space);
        for (int o : findMultiple(frlgNatDexFlagChecker)) writeHexString(frlgE4FlagChecker, o);
        if (int o = find(frlgOaksLabKantoDexChecker); o > 0) writeHexString(frlgOaksLabFix, o);
        if (int o = find(frlgOakOutsideHouseCheck); o > 0) writeHexString(frlgOakOutsideHouseFix, o);
        if (int o = find(frlgOakAideCheckPrefix); o > 0) {
            o += static_cast<int>(frlgOakAideCheckPrefix.size() / 2);
            writeByte(o + 1, 0xE0);  // bne -> salto incondicional
        }
    } else {
        throw std::logic_error("Invalid ROM Type");
    }
}

// ---------------- introducción, tutorial, poción ----------------

bool Gen3RomHandler::setIntroPokemon(Species* pk) {
    int imageTable = romEntry_->getInt("PokemonFrontImages");
    int paletteTable = romEntry_->getInt("PokemonNormalPalettes");
    int cry = romEntry_->getInt("IntroCryOffset");
    int image = romEntry_->getInt("IntroImageOffset");
    int palette = romEntry_->getInt("IntroPaletteOffset");
    int other = romEntry_->getInt("IntroOtherOffset");
    int intro = pokedexToInternal_[pk->number()];
    int rt = romEntry_->romType();
    if (rt == RomType_FRLG) {
        if (intro > 255) return false;
        writeByte(cry, static_cast<uint8_t>(intro));
        writeByte(other, static_cast<uint8_t>(intro));
        writePointer(image, imageTable + intro * 8);
        writePointer(image + 4, paletteTable + intro * 8);
    } else if (rt == RomType_Ruby || rt == RomType_Sapp) {
        if (intro == 1 || intro > 510) return false;
        if (intro > 255) {
            rom_[cry] = 0xFF;
            rom_[cry + 1] = gbaSetRxOpcode | gbaR0;
            rom_[cry + 2] = static_cast<uint8_t>(intro - 0xFF);
            rom_[cry + 3] = gbaAddRxOpcode | gbaR0;
            rom_[other] = 0xFF;
            rom_[other + 1] = gbaSetRxOpcode | gbaR4;
            rom_[other + 2] = static_cast<uint8_t>(intro - 0xFF);
            rom_[other + 3] = gbaAddRxOpcode | gbaR4;
        } else {
            rom_[cry] = static_cast<uint8_t>(intro);
            rom_[cry + 1] = gbaSetRxOpcode | gbaR0;
            writeWord(cry + 2, gbaNopOpcode);
            rom_[other] = static_cast<uint8_t>(intro);
            rom_[other + 1] = gbaSetRxOpcode | gbaR4;
            writeWord(other + 2, gbaNopOpcode);
        }
        writePointer(image, imageTable + intro * 8);
        writePointer(palette, paletteTable + intro * 8);
    } else {
        writeWord(image, intro);  // Esmeralda: cualquier Pokémon
        writeWord(cry, intro);
    }
    return true;
}

bool Gen3RomHandler::setCatchingTutorial(Species* opponent, Species* player) {
    auto writeMon = [&](int off, int value) {
        if (value > 255) {
            writeBytes(off, {0xFF, static_cast<uint8_t>(gbaSetRxOpcode | gbaR1), static_cast<uint8_t>(value - 0xFF),
                             static_cast<uint8_t>(gbaAddRxOpcode | gbaR1)});
        } else {
            writeBytes(off, {static_cast<uint8_t>(value), static_cast<uint8_t>(gbaSetRxOpcode | gbaR1)});
            writeWord(off + 2, gbaNopOpcode);
        }
    };
    if (int opp = romEntry_->getInt("CatchingTutorialOpponentMonOffset"); opp > 0) {
        if (opponent->genderRatio > 0xFD) return false;  // solo especies que pueden ser macho
        int v = pokedexToInternal_[opponent->number()];
        if (romEntry_->romType() == RomType_FRLG) {
            if (opponent->number() > 255) return false;
            writeBytes(opp, {static_cast<uint8_t>(v), static_cast<uint8_t>(gbaSetRxOpcode | gbaR1)});
        } else {
            if (opponent->number() > 510) return false;
            writeMon(opp, v);
        }
    }
    if (int ply = romEntry_->getInt("CatchingTutorialPlayerMonOffset"); ply > 0) {
        if (player->number() > 510) return false;
        writeMon(ply, pokedexToInternal_[player->number()]);
    }
    return true;
}

void Gen3RomHandler::setPCPotionItem(int item) {
    int o = romEntry_->getInt("PCPotionOffset");
    if (o == 0) return;
    if (!items_.at(item)->allowed) throw std::invalid_argument("item not allowed for PC Potion: " + items_.at(item)->name);
    writeWord(o, itemIDToInternal(item));
}

void Gen3RomHandler::enableGuaranteedPokemonCatching() {
    int offset = find(perfectOddsBranchLocator);
    // anula "cmp r6, #0xFE; bls ..." para que la captura siempre funcione
    if (offset > 0) writeBytes(offset, {0x00, 0x00, 0x00, 0x00});
}

void Gen3RomHandler::applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) {
    if (!romEntry_->hasTweakFile("NewIndexToMusicTweak")) return;
    romfn::applyPatch(rom_, romEntry_->getTweakFile("NewIndexToMusicTweak"));
    int pool = romEntry_->getInt("NewIndexToMusicPoolOffset");
    if (pool <= 0) return;
    std::vector<int> replaced;
    for (const auto& [oldStatic, newStatic] : changes) {
        int i = pool;
        int index = internalToPokedex_.at(readWord(i));
        while (index != oldStatic || std::find(replaced.begin(), replaced.end(), i) != replaced.end()) {
            i += 4;
            index = internalToPokedex_.at(readWord(i));
        }
        writeWord(i, pokedexToInternal_[newStatic]);
        replaced.push_back(i);
    }
}

ItemSet Gen3RomHandler::getAllHeldItems() const { return itemIdsToSet(allHeldItems()); }

std::vector<int> Gen3RomHandler::getFieldMoves() const {
    return romEntry_->romType() == RomType_FRLG ? frlgFieldMoves : rseFieldMoves;
}

std::vector<int> Gen3RomHandler::getEarlyRequiredHMMoves() const {
    return romEntry_->romType() == RomType_FRLG ? frlgEarlyRequiredHMMoves : rseEarlyRequiredHMMoves;
}

}  // namespace pr::gen3
