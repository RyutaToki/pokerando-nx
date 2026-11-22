// Ajustes de Gen 5: tabla de tipos, ajustes varios (MiscTweak), evoluciones imposibles/fáciles,
// quitar evoluciones fuera del conjunto, Pokémon de la introducción, captura garantizada y música
// de los estáticos (port de Gen5RomHandler).
#include <algorithm>
#include <stdexcept>

#include "constants/global_constants.hpp"
#include "constants/item_ids.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"
#include "romio/misc_tweak.hpp"
#include "romio/rom_functions.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int readLong(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeFullInt(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
int hexLen(std::string_view s) { return static_cast<int>(s.size()) / 2; }
}  // namespace

// ---------------- tabla de tipos ----------------

TypeTable Gen5RomHandler::getTypeTable() const {
    if (typeTable_) return *typeTable_;
    TypeTable t(TypeTable::vanillaGen2To5().types());
    Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
    int tableOffset = romEntry_->getInt("TypeEffectivenessOffset");
    int width = static_cast<int>(t.types().size());
    for (Type a : t.types()) {
        for (Type d : t.types()) {
            int v = static_cast<int8_t>(ovl.at(tableOffset + typeToByte(a) * width + typeToByte(d)));
            switch (v) {
                case 8: t.setEffectiveness(a, d, Effectiveness::DOUBLE); break;
                case 4: t.setEffectiveness(a, d, Effectiveness::NEUTRAL); break;
                case 2: t.setEffectiveness(a, d, Effectiveness::HALF); break;
                case 0: t.setEffectiveness(a, d, Effectiveness::ZERO); break;
                default: break;  // null en Java (no pasa en las ROMs originales)
            }
        }
    }
    typeTable_ = t;
    return t;
}

void Gen5RomHandler::setTypeTable(const TypeTable& t) {
    typeTable_ = t;
    int ovlNum = romEntry_->getInt("BattleOvlNumber");
    Bytes ovl = readOverlay(ovlNum);
    int tableOffset = romEntry_->getInt("TypeEffectivenessOffset");
    int width = static_cast<int>(t.types().size());
    for (Type a : t.types()) {
        for (Type d : t.types()) {
            uint8_t v = 0;
            switch (t.getEffectiveness(a, d)) {
                case Effectiveness::DOUBLE: v = 8; break;
                case Effectiveness::NEUTRAL: v = 4; break;
                case Effectiveness::HALF: v = 2; break;
                default: v = 0; break;
            }
            ovl.at(tableOffset + typeToByte(a) * width + typeToByte(d)) = v;
        }
    }
    writeOverlay(ovlNum, ovl);
}

// ---------------- ajustes varios ----------------

int Gen5RomHandler::miscTweaksAvailable() const {
    using namespace misctweak;
    int a = 0;
    if (romEntry_->hasTweakFile("FastestTextTweak")) a |= FASTEST_TEXT.value;
    a |= BAN_LUCKY_EGG.value;
    a |= NO_FREE_LUCKY_EGG.value;
    a |= BAN_BIG_MANIAC_ITEMS.value;
    if (romEntry_->romType() == Type_BW) a |= BALANCE_STATIC_LEVELS.value;
    if (romEntry_->hasTweakFile("NationalDexAtStartTweak")) a |= NATIONAL_DEX_AT_START.value;
    a |= RUN_WITHOUT_RUNNING_SHOES.value;
    if (romEntry_->romType() == Type_BW2) a |= FORCE_CHALLENGE_MODE.value;
    a |= DISABLE_LOW_HP_MUSIC.value;
    if (romEntry_->getInt("HMMovesForgettableFunctionOffset") != 0) a |= FORGETTABLE_HMS.value;
    a |= FAST_EGG_HATCHING.value;
    a |= NO_EV_YIELDS.value;
    return a;
}

void Gen5RomHandler::applyMiscTweak(const MiscTweak& t) {
    using namespace misctweak;
    if (t == FASTEST_TEXT) {
        genericIPSPatch(arm9_, "FastestTextTweak");
    } else if (t == BAN_LUCKY_EGG) {
        items_.at(items::luckyEgg).value().allowed = false;
    } else if (t == NO_FREE_LUCKY_EGG) {
        removeFreeLuckyEgg();
    } else if (t == BAN_BIG_MANIAC_ITEMS) {
        for (int i = 0; i < 4; i++) {
            items_.at(items::balmMushroom + i).value().allowed = false;  // Seta Aroma, Maxipepita, Sarta Perlas, Fragmento Cometa
            items_.at(items::relicVase + i).value().allowed = false;     // reliquias
        }
        for (int i = 0; i < 7; i++) items_.at(items::lansatBerry + i).value().allowed = false;  // bayas raras
    } else if (t == BALANCE_STATIC_LEVELS) {
        writeWord(scriptNarc_.files.at(fossilPokemonFile), fossilPokemonLevelOffset, 20);
    } else if (t == NATIONAL_DEX_AT_START) {
        // el parche espera el script con 4 bytes más para la instrucción nueva
        int idx = romEntry_->getInt("NationalDexScriptOffset");
        Bytes script = scriptNarc_.files.at(idx);
        script.resize(script.size() + 4, 0);
        genericIPSPatch(script, "NationalDexAtStartTweak");
        scriptNarc_.files.at(idx) = script;
    } else if (t == RUN_WITHOUT_RUNNING_SHOES) {
        // anula el salto que comprueba la bandera 2403 (tener las Deportivas)
        int ovlNum = romEntry_->getInt("FieldOvlNumber");
        Bytes ovl = readOverlay(ovlNum);
        int offset = find(ovl, runningShoesPrefix);
        if (offset != 0) {  // (sic) -1 no se comprueba en FVX
            writeWord(ovl, offset, 0);
            writeOverlay(ovlNum, ovl);
        }
    } else if (t == FORCE_CHALLENGE_MODE) {
        int offset = find(arm9_, forceChallengeModeLocator);
        if (offset > 0) {
            // la función que devuelve la dificultad pasa a ser «mov r0, #2; bx lr»
            arm9_.at(offset) = 0x02;
            arm9_.at(offset + 1) = 0x20;
            arm9_.at(offset + 2) = 0x70;
            arm9_.at(offset + 3) = 0x47;
        }
    } else if (t == DISABLE_LOW_HP_MUSIC) {
        int ovlNum = romEntry_->getInt("LowHealthMusicOvlNumber");
        Bytes ovl = readOverlay(ovlNum);
        int offset = find(ovl, lowHealthMusicLocator);
        if (offset > 0) {
            ovl.at(offset + 1) = 0xE0;  // bne -> b: nunca «PS bajos» para la música
            writeOverlay(ovlNum, ovl);
        }
    } else if (t == FORGETTABLE_HMS) {
        int offset = romEntry_->getInt("HMMovesForgettableFunctionOffset");
        if (offset == 0) return;
        Bytes before = romfn::hexToBytes(std::string(hmsForgettableBefore));
        for (size_t i = 0; i < before.size(); i++) {
            if (arm9_.at(offset + i) != before[i]) {
                char buf[160];
                std::snprintf(buf, sizeof buf, "Expected 0x%x, was 0x%x. Likely HMMovesForgettableFunctionOffset is faulty.", before[i],
                              arm9_.at(offset + i));
                throw std::runtime_error(buf);
            }
        }
        Bytes after = romfn::hexToBytes(std::string(hmsForgettableAfter));
        std::copy(after.begin(), after.end(), arm9_.begin() + offset);
    } else if (t == FAST_EGG_HATCHING) {
        for (Species* pk : speciesInclFormes_)
            if (pk) pk->breedingInfo.eggCycles = 1;
    } else if (t == NO_EV_YIELDS) {
        for (Species* pk : speciesInclFormes_)
            if (pk) pk->evYield = EVYield{};
    }
}

// Cambia el Huevo Suerte que regala la profesora Encina por un Abono Viscoso
void Gen5RomHandler::removeFreeLuckyEgg() {
    int scriptFileGifts = romEntry_->getInt("LuckyEggScriptOffset");
    int setVarGift = hiddenItemSetVarCommand;
    Bytes& itemScripts = scriptNarc_.files.at(scriptFileGifts);
    int offset = 0;
    int lookingForEggs = romEntry_->romType() == Type_BW ? 1 : 2;
    while (lookingForEggs > 0) {
        int part1 = readWord(itemScripts, offset);
        if (part1 == scriptListTerminator) break;
        int offsetInFile = readLong(itemScripts, offset) + offset + 4;  // readRelativePointer
        offset += 4;
        if (offsetInFile > static_cast<int>(itemScripts.size())) break;
        while (true) {
            offsetInFile++;
            // los regalos no tienen por qué estar alineados: se lee byte a byte
            int b = itemScripts.at(offsetInFile);
            if (b == setVarGift) {
                int command = readWord(itemScripts, offsetInFile);
                int variable = readWord(itemScripts, offsetInFile + 2);
                int item = readWord(itemScripts, offsetInFile + 4);
                if (command == setVarGift && variable == hiddenItemVarSet && item == items::luckyEgg) {
                    writeWord(itemScripts, offsetInFile + 4, items::gooeyMulch);
                    lookingForEggs--;
                }
            }
            if (b == 0x2E) break;  // empieza otro bloque
        }
    }
}

void Gen5RomHandler::enableGuaranteedPokemonCatching() {
    int ovlNum = romEntry_->getInt("BattleOvlNumber");
    Bytes ovl = readOverlay(ovlNum);
    int offset = find(ovl, perfectOddsBranchLocator);
    if (offset > 0) {
        // se anula el «blt» que salta si la probabilidad es menor de 255: siempre se captura
        ovl.at(offset) = 0x00;
        ovl.at(offset + 1) = 0x00;
        writeOverlay(ovlNum, ovl);
    }
}

// ---------------- evoluciones ----------------

void Gen5RomHandler::removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) {
    using ET = EvolutionType;
    auto movesets = getMovesLearnt();
    for (Species* pk : pokes_) {
        if (!pk) continue;
        for (const auto& evo : pk->evolutionsFrom()) {
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
                if (evo->from()->number() == species::slowpoke) {
                    // Slowpoke ya evoluciona por nivel: Piedra Agua
                    evo->updateEvolutionMethod(ET::STONE, items::waterStone, useEstimatedLevels);
                } else {
                    evo->updateEvolutionMethod(ET::ITEM, evo->extraInfo(), useEstimatedLevels);
                }
            }
            if (evo->type() == ET::TRADE_SPECIAL) {
                // Karrablast <-> Shelmet: subir de nivel con el otro en el equipo
                markImprovedEvolutions(pk);
                evo->updateEvolutionMethod(ET::WITH_OTHER, evo->from()->number() == species::karrablast ? species::shelmet : species::karrablast,
                                           useEstimatedLevels);
            }
        }
    }
}

void Gen5RomHandler::makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) {
    // menos amistad para evolucionar (normal, de día y de noche)
    int offset = find(arm9_, friendshipValueForEvoLocator);
    if (offset > 0) {
        for (int d : {0, 20, 38})
            if (arm9_.at(offset + d) == global::vanillaHappinessToEvolve) arm9_.at(offset + d) = static_cast<uint8_t>(global::easierHappinessToEvolve);
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

void Gen5RomHandler::removeEvosForPokemonPool(const SpeciesSet& included) {
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
                                      return std::none_of(keep.begin(), keep.end(), [&](const EvolutionPtr& k) { return k->sameAs(*e); });
                                  }),
                   from.end());
    }
    // tabla de bebés (qué sale de cada huevo)
    std::string path = romEntry_->getFile("BabyPokemon");
    auto babyNARC = readNARC(path);
    for (int i = 1; i <= pokemonCount; i++) {
        const Species* baby = pokes_[i];
        while (!baby->evolutionsTo().empty()) baby = baby->evolutionsTo().front()->from();
        writeWord(babyNARC.files.at(i), 0, baby->number());
    }
    writeNARC(path, babyNARC);
}

// ---------------- introducción y música ----------------

bool Gen5RomHandler::setIntroPokemon(Species* pk) {
    int introPokemon = pk->number();
    if (!pk->isBaseForme()) return false;  // por si acaso, sin formas
    int gfxOvl = romEntry_->getInt("IntroGraphicOvlNumber");
    Bytes introGraphicOverlay = readOverlay(gfxOvl);
    int offset = find(introGraphicOverlay, introGraphicPrefix);
    if (offset > 0) {
        offset += hexLen(introGraphicPrefix);
        writeWord(introGraphicOverlay, offset, introPokemon);  // la especie que decide qué imagen se carga
        writeOverlay(gfxOvl, introGraphicOverlay);
    }
    int cryOvl = romEntry_->getInt("IntroCryOvlNumber");
    if (romEntry_->romType() == Type_BW) {
        Bytes ov = readOverlay(cryOvl);
        offset = find(ov, bw1IntroCryPrefix);
        if (offset > 0) {
            offset += hexLen(bw1IntroCryPrefix);
            // se sube 2 bytes todo lo que hay tras el desplazamiento para hacer sitio a la especie
            // al final de la función, y se carga con una instrucción relativa al PC
            for (int i = offset + 6; i < offset + 40; i++) ov.at(i - 2) = ov.at(i);
            ov.at(offset + 10)++;  // la llamada a PlayCry se ha movido
            writeFullInt(ov, offset + 38, introPokemon);
            ov.at(offset) = 0x9;
            ov.at(offset + 1) = 0x48;
            writeOverlay(cryOvl, ov);
        }
    } else {
        Bytes ov = readOverlay(cryOvl);
        offset = find(ov, bw2IntroCryLocator);
        if (offset > 0) {
            writeWord(ov, offset, introPokemon);
            writeOverlay(cryOvl, ov);
        }
    }
    return true;
}

bool Gen5RomHandler::setCatchingTutorial(Species*, Species*) {
    throw std::logic_error("UnsupportedOperationException");  // AbstractRomHandler: Gen 5 no lo tiene
}

void Gen5RomHandler::setPCPotionItem(int) { throw std::logic_error("UnsupportedOperationException"); }

bool Gen5RomHandler::hasStaticMusicFix() const { return romEntry_->hasTweakFile("NewIndexToMusicTweak"); }

void Gen5RomHandler::applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) {
    int fieldOvl = romEntry_->getInt("FieldOvlNumber");
    Bytes fieldOverlay = readOverlay(fieldOvl);
    genericIPSPatch(fieldOverlay, "NewIndexToMusicOvlTweak");
    writeOverlay(fieldOvl, fieldOverlay);
    genericIPSPatch(arm9_, "NewIndexToMusicTweak");  // la ARM9 ya se amplió en loadedROM
    const std::string& prefix = romEntry_->getString("NewIndexToMusicPrefix");
    int pool = find(arm9_, prefix) + static_cast<int>(prefix.size()) / 2;
    std::vector<int> replaced;
    auto isReplaced = [&](int i) { return std::find(replaced.begin(), replaced.end(), i) != replaced.end(); };
    int iMax = -1;
    for (const auto& [oldStatic, newMusic] : changes) {
        int i = pool;
        int index = readWord(arm9_, i);
        while (index != oldStatic || isReplaced(i)) {
            i += 4;
            index = readWord(arm9_, i);
        }
        if (romEntry_->romType() == Type_BW2 && index > pokemonCount) {
            // Kyurem Blanco/Negro
            writeWord(arm9_, i - 0xFE, 0);
            writeWord(arm9_, i - 0xFC, 0);
            writeWord(arm9_, i - 0xFA, 0);
            writeWord(arm9_, i - 0xF8, 0x4290);
        }
        writeWord(arm9_, i, newMusic);
        replaced.push_back(i);
        if (i > iMax) iMax = i;
    }
    auto specials = getSpecialMusicStatics();
    for (int i = pool; i <= iMax; i += 4) {
        if (isReplaced(i)) continue;
        // un estático con música especial que no se ha cambiado se queda; el resto ya no existe
        if (std::find(specials.begin(), specials.end(), readWord(arm9_, i)) == specials.end()) writeWord(arm9_, i, 0);
    }
}

}  // namespace pr::gen5
