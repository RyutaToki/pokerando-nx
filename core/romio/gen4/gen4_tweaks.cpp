// Ajustes de Gen 4: evoluciones imposibles/fáciles, quitar evoluciones fuera del conjunto, ajustes varios
// (MiscTweak), Pokémon de la introducción, tutorial de captura, captura garantizada y música de estáticos
// (port de Gen4RomHandler).
#include <algorithm>
#include <stdexcept>

#include "constants/global_constants.hpp"
#include "constants/item_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "romio/misc_tweak.hpp"
#include "romio/rom_functions.hpp"
#include "util/java_compat.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
void writeLong(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}

int overworldSpriteIDOfSpecies(int sp) {
    // Gen4Constants.getOverworldSpriteIDOfSpecies: primer sprite del mapa de cada especie
    auto amount = [](int s) {
        using namespace species;
        switch (s) {
            case venusaur: case pikachu: case meganium: case pichu: case wobbuffet: case steelix: case heracross:
            case combee: case shellos: case gastrodon: case gible: case gabite: case garchomp: case hippopotas:
            case hippowdon: case giratina: return 2;
            case unown: return 28;
            case deoxys: return 4;
            case burmy: case wormadam: return 3;
            case rotom: return 6;
            case arceus: return 18;
            default: return 1;
        }
    };
    int id = 297;  // Bulbasaur
    for (int i = 1; i < sp; i++) id += amount(i);
    return id;
}
}  // namespace

std::vector<int> Gen4RomHandler::getDoublesTrainerClasses() const { return romEntry_->getArray("DoublesTrainerClasses"); }

std::vector<int> Gen4RomHandler::getFieldMoves() const {
    return romEntry_->romType() == Type_HGSS ? hgssFieldMoves : dpptFieldMoves;
}

std::vector<int> Gen4RomHandler::getEarlyRequiredHMMoves() const {
    return romEntry_->romType() == Type_HGSS ? hgssEarlyRequiredHMMoves : dpptEarlyRequiredHMMoves;
}

std::string Gen4RomHandler::getPaletteFilesID() const { return "DP"; }  // también Pt y HGSS, como FVX

// ---------------- evoluciones ----------------

void Gen4RomHandler::removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) {
    using ET = EvolutionType;
    auto movesets = getMovesLearnt();
    for (Species* pk : pokes_) {
        if (!pk) continue;
        for (const auto& evo : pk->evolutionsFrom()) {
            if (romEntry_->romType() == Type_HGSS) {
                // en HGSS no hay los lugares especiales de Sinnoh
                if (evo->type() == ET::HIGH_BEAUTY) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::LEVEL, 35, useEstimatedLevels);
                }
                if (evo->type() == ET::MAGNETIC_FIELD) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::LEVEL, 40, useEstimatedLevels);
                }
                if (evo->type() == ET::MOSS_ROCK) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::STONE, items::leafStone, useEstimatedLevels);
                }
                if (evo->type() == ET::ICE_ROCK) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(ET::STONE, items::dawnStone, useEstimatedLevels);
                }
            }
            if (changeMoveEvos && evo->type() == ET::WITH_MOVE) {
                int move = evo->extraInfo();
                int levelLearntAt = 1;
                for (const auto& ml : movesets.at(evo->from()->number()))
                    if (ml.move == move) {
                        levelLearntAt = ml.level;
                        break;
                    }
                if (levelLearntAt == 1) levelLearntAt = useEstimatedLevels ? evo->estimatedEvoLvl() : 45;  // Piloswine
                markImprovedEvolutions(pk);
                evo->updateEvolutionMethod(ET::LEVEL, levelLearntAt);
            }
            if (evo->type() == ET::TRADE) {
                markImprovedEvolutions(pk);
                evo->updateEvolutionMethod(ET::LEVEL, 37, useEstimatedLevels);
            }
            if (evo->type() == ET::TRADE_ITEM) {
                markImprovedEvolutions(pk);
                if (evo->from()->number() == species::slowpoke)  // ya evoluciona por nivel: Piedra Agua
                    evo->updateEvolutionMethod(ET::STONE, items::waterStone, useEstimatedLevels);
                else
                    evo->updateEvolutionMethod(ET::ITEM, evo->extraInfo(), useEstimatedLevels);
            }
        }
    }
}

void Gen4RomHandler::makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) {
    // menos amistad para evolucionar
    int offset = find(arm9_, std::string(friendshipValueForEvoLocator));
    if (offset > 0) {
        for (int d : {0, 22, 44})
            if (arm9_.at(offset + d) == global::vanillaHappinessToEvolve)
                arm9_.at(offset + d) = static_cast<uint8_t>(global::easierHappinessToEvolve);
    }
    if (changeWithOtherEvos) {
        for (Species* pk : pokes_) {
            if (!pk) continue;
            for (const auto& evo : pk->evolutionsFrom())
                if (evo->type() == EvolutionType::WITH_OTHER) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(EvolutionType::LEVEL, 35, useEstimatedLevels);
                }
        }
    }
}

void Gen4RomHandler::removeEvosForPokemonPool(const SpeciesSet& included) {
    for (Species* pk : pokes_) {
        if (!pk) continue;
        std::vector<EvolutionPtr> keep;
        for (const auto& evo : pk->evolutionsFrom()) {
            if (included.contains(evo->from()) && included.contains(evo->to())) {
                keep.push_back(evo);
            } else {
                auto& to = evo->to()->evolutionsTo();
                auto it = std::find_if(to.begin(), to.end(), [&](const EvolutionPtr& e) { return e->sameAs(*evo); });
                if (it != to.end()) to.erase(it);
            }
        }
        auto& from = pk->evolutionsFrom();
        from.erase(std::remove_if(from.begin(), from.end(),
                                  [&](const EvolutionPtr& e) {
                                      return std::none_of(keep.begin(), keep.end(),
                                                          [&](const EvolutionPtr& k) { return k->sameAs(*e); });
                                  }),
                   from.end());
    }
    // tabla de bebés (qué sale de cada huevo)
    Bytes babies = readFile(romEntry_->getFile("BabyPokemon"));
    for (int i = 1; i <= pokemonCount; i++) {
        const Species* baby = pokes_[i];
        while (!baby->evolutionsTo().empty()) baby = baby->evolutionsTo().front()->from();
        writeWord(babies, i * 2, baby->number());
    }
    writeFile(romEntry_->getFile("BabyPokemon"), babies);
}

// ---------------- ajustes varios ----------------

void Gen4RomHandler::applyMiscTweak(const MiscTweak& t) {
    using namespace misctweak;
    if (t == LOWER_CASE_POKEMON_NAMES) {
        for (Species* pk : getSpeciesSet().toVector()) pk->setName(romfn::camelCase(pk->name()));
    } else if (t == FASTEST_TEXT) {
        genericIPSPatch(arm9_, "FastestTextTweak");
    } else if (t == BAN_LUCKY_EGG) {
        items_.at(items::luckyEgg).value().allowed = false;
    } else if (t == NATIONAL_DEX_AT_START) {
        int idx = romEntry_->getInt("NationalDexScriptOffset");
        Bytes script = scriptNarc_.files.at(idx);
        // en HGSS el parche añade una instrucción: el script crece 4 bytes
        if (romEntry_->romType() == Type_HGSS) script.resize(script.size() + 4, 0);
        genericIPSPatch(script, "NationalDexAtStartTweak");
        scriptNarc_.files.at(idx) = script;
    } else if (t == RUN_WITHOUT_RUNNING_SHOES) {
        std::string prefix(romEntry_->romType() == Type_HGSS ? runningShoesCheckPrefixHGSS : runningShoesCheckPrefixDPPt);
        int offset = find(arm9_, prefix);
        if (offset != 0) writeWord(arm9_, offset + 0xE, 0);  // anula el salto que exige las Deportivas (como FVX)
    } else if (t == FASTER_HP_AND_EXP_BARS) {
        // Gen 4 va a 30 FPS: se duplica lo que avanzan las barras
        Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
        std::string hp(hpBarSpeedPrefix), exp(expBarSpeedPrefix), both(bothBarsSpeedPrefix);
        int offset = find(ovl, hp);
        if (offset > 0) ovl.at(offset + static_cast<int>(hp.size()) / 2) = 0x02;
        offset = find(ovl, exp);
        if (offset > 0) {
            offset += static_cast<int>(exp.size()) / 2;
            ovl.at(offset) = 0xB0;  // add r0, r6, r6
            ovl.at(offset + 1) = 0x19;
        }
        offset = find(ovl, both);
        if (offset > 0) ovl.at(offset + static_cast<int>(both.size()) / 2) = 0x40;  // maxValue << 9
        writeOverlay(romEntry_->getInt("BattleOvlNumber"), ovl);
    } else if (t == FAST_DISTORTION_WORLD) {
        Bytes script = scriptNarc_.files.at(ptSpearPillarPortalScriptFile);
        script.resize(script.size() + 12, 0);
        genericIPSPatch(script, "FastDistortionWorldTweak");
        scriptNarc_.files.at(ptSpearPillarPortalScriptFile) = script;
    } else if (t == UPDATE_ROTOM_FORME_TYPING) {
        using namespace species::Gen4Formes;
        pokes_.at(rotomH)->setSecondaryType(Type::FIRE);
        pokes_.at(rotomW)->setSecondaryType(Type::WATER);
        pokes_.at(rotomFr)->setSecondaryType(Type::ICE);
        pokes_.at(rotomFa)->setSecondaryType(Type::FLYING);
        pokes_.at(rotomM)->setSecondaryType(Type::GRASS);
    } else if (t == REUSABLE_TMS) {
        int offset = romEntry_->getInt("TMMovesReusableFunctionOffset");
        if (offset == 0) return;
        if (arm9_.at(offset) != tmsReusableByteBefore)
            throw std::runtime_error("TMMovesReusableFunctionOffset parece incorrecto");
        arm9_.at(offset) = tmsReusableByteAfter;
        tmsReusable_ = true;
    } else if (t == FORGETTABLE_HMS) {
        applyForgettableHMsPatch();
    } else if (t == FAST_EGG_HATCHING) {
        for (Species* pk : getSpeciesSetInclFormes().toVector()) pk->breedingInfo.eggCycles = 0;
    } else if (t == NO_EV_YIELDS) {
        for (Species* pk : getSpeciesSetInclFormes().toVector()) pk->evYield = EVYield{};
    }
}

void Gen4RomHandler::applyForgettableHMsPatch() {
    // la llamada que dice si un movimiento es MO se sustituye por «r0 = 0» (C0 46 00 20)
    const Bytes r0False = {0xC0, 0x46, 0x00, 0x20};
    const auto& offsets = romEntry_->getArray("HMMovesForgettableFunctionOffsets");
    const int rt = romEntry_->romType();
    size_t expected = rt == Type_HGSS ? 3 : 2;
    if (offsets.size() != expected) throw std::runtime_error("Longitud inesperada de HMMovesForgettableFunctionOffsets");
    auto patch = [&](Bytes& d, int o) {
        if (d.at(o + 4) != 0x01 || d.at(o + 5) != 0x28)
            throw std::runtime_error("HMMovesForgettableFunctionOffsets parece incorrecto");
        std::copy(r0False.begin(), r0False.end(), d.begin() + o);
    };
    int battleOvl = rt == Type_DP ? 9 : (rt == Type_Plat ? 13 : 8);
    Bytes ol = readOverlay(battleOvl);
    patch(ol, offsets[0]);
    writeOverlay(battleOvl, ol);
    patch(arm9_, offsets[1]);
    if (rt == Type_HGSS) patch(arm9_, offsets[2]);
}

// ---------------- introducción y tutorial de captura ----------------

bool Gen4RomHandler::setIntroPokemon(Species* pk) {
    const int rt = romEntry_->romType();
    if (rt == Type_DP || rt == Type_Plat) {
        // las especies solo hembra necesitan otro parámetro al cargar el sprite: se descartan
        if (pk->genderRatio == 0xFE) return false;
        if (!pk->isBaseForme()) return false;
        Bytes ovl = readOverlay(romEntry_->getInt("IntroOvlNumber"));
        for (auto p : dpptIntroPrefixes) {
            std::string prefix(p);
            int offset = find(ovl, prefix);
            if (offset > 0) writeWord(ovl, offset + static_cast<int>(prefix.size()) / 2, pk->number());
        }
        writeOverlay(romEntry_->getInt("IntroOvlNumber"), ovl);
        return true;
    }
    // HGSS: se cambia el Marill de Eco/Lira
    if (!pk->isBaseForme()) return false;
    int spriteID = overworldSpriteIDOfSpecies(pk->number());
    Bytes field = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    std::string prefix(lyraEthanMarillSpritePrefix);
    int offset = find(field, prefix);
    if (offset > 0) {
        offset += static_cast<int>(prefix.size()) / 2;
        writeWord(field, offset, spriteID);
        bool big = std::find(hgssBigOverworldPokemon.begin(), hgssBigOverworldPokemon.end(), pk->number()) !=
                   hgssBigOverworldPokemon.end();
        writeWord(field, offset + 2, big ? 0x5208 : 0x4E27);
    }
    writeOverlay(romEntry_->getInt("FieldOvlNumber"), field);
    for (const auto& e : romEntry_->getMarillCryScriptEntries()) writeWord(scriptNarc_.files.at(e.file), e.offset, pk->number());
    std::map<std::string, std::string> reps{{jc::toUpperCase(pokes_[species::marill]->name()), pk->name()}};
    for (int textOffset : romEntry_->getArray("MarillTextFiles")) replaceAllStringsInEntry(textOffset, reps);
    if (romEntry_->hasTweakFile("NewCatchingTutorialSubroutineTweak")) {
        const std::string& tp = romEntry_->getString("CatchingTutorialMonTablePrefix");
        offset = find(arm9_, tp);
        if (offset > 0) writeWord(arm9_, offset + static_cast<int>(tp.size()) / 2, pk->number());
    }
    return true;
}

bool Gen4RomHandler::setCatchingTutorial(Species* opponent, Species* player) {
    int opponentOffset = romEntry_->getInt("CatchingTutorialOpponentMonOffset");
    if (romEntry_->hasTweakFile("NewCatchingTutorialSubroutineTweak")) {
        const std::string& tp = romEntry_->getString("CatchingTutorialMonTablePrefix");
        int offset = find(arm9_, tp);
        if (offset > 0) writeWord(arm9_, offset + static_cast<int>(tp.size()) / 2 + 4, opponent->number());
    } else if (romEntry_->romType() == Type_HGSS) {
        // HGSS no estadounidense: los dos Pokémon, solo hasta el 255; Marill de Eco/Lira a nivel 10
        int playerOffset = romEntry_->getInt("CatchingTutorialPlayerMonOffset");
        int levelOffset = romEntry_->getInt("CatchingTutorialPlayerLevelOffset");
        if (opponent->number() > 255 || player->number() > 255) return false;
        arm9_.at(opponentOffset) = static_cast<uint8_t>(opponent->number());
        arm9_.at(playerOffset) = static_cast<uint8_t>(player->number());
        arm9_.at(levelOffset) = 10;
    } else if (opponent) {
        writeLong(arm9_, opponentOffset, opponent->number());  // DPPt: solo el rival
    }
    return true;
}

void Gen4RomHandler::setPCPotionItem(int /*item*/) {
    throw std::logic_error("UnsupportedOperationException: setPCPotionItem no existe en Gen 4");
}

void Gen4RomHandler::enableGuaranteedPokemonCatching() {
    // se anulan «cmp r1, #0xFF; bcc ...»: siempre se captura
    Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
    int offset = find(ovl, std::string(perfectOddsBranchLocator));
    if (offset > 0) {
        for (int i = 0; i < 4; i++) ovl.at(offset + i) = 0;
        writeOverlay(romEntry_->getInt("BattleOvlNumber"), ovl);
    }
}

void Gen4RomHandler::applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) {
    std::vector<int> replaced;
    auto isReplaced = [&](int i) { return std::find(replaced.begin(), replaced.end(), i) != replaced.end(); };
    if (romEntry_->romType() == Type_HGSS) {
        const std::string& prefix = romEntry_->getString("IndexToMusicPrefix");
        int pool = find(arm9_, prefix);
        if (pool > 0) {
            pool += static_cast<int>(prefix.size()) / 2;
            for (const auto& [oldStatic, newMusic] : changes) {
                int i = pool;
                int indexEtc = readWord(arm9_, i);
                while ((indexEtc & 0x3FF) != oldStatic || isReplaced(i)) {
                    i += 2;
                    indexEtc = readWord(arm9_, i);
                }
                writeWord(arm9_, i, newMusic | (indexEtc & 0xFC00));
                replaced.push_back(i);
            }
        }
    } else {
        genericIPSPatch(arm9_, "NewIndexToMusicTweak");
        const std::string& prefix = romEntry_->getString("NewIndexToMusicPrefix");
        int pool = find(arm9_, prefix) + static_cast<int>(prefix.size()) / 2;
        for (const auto& [oldStatic, newMusic] : changes) {
            int i = pool;
            while (readWord(arm9_, i) != oldStatic || isReplaced(i)) i += 4;
            writeWord(arm9_, i, newMusic);
            replaced.push_back(i);
        }
    }
}

}  // namespace pr::gen4
