// Pokémon estáticos de Gen 5 (port de Gen5RomHandler.getStaticPokemon/setStaticPokemon): encuentros
// fijos, los Foongus/Amoonguss que se hacen pasar por objetos, los Claros Ocultos de N2/B2 y los
// Pokémon errantes; y los arreglos del legendario de la caja (Reshiram/Zekrom) y de los errantes de N/B.
#include <algorithm>
#include <stdexcept>

#include "constants/species_ids.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
void writeFullInt(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
int sbyte(const Bytes& d, int o) { return static_cast<int8_t>(d.at(o)); }

// DSStaticPokemon.getLevel / setLevel / getForme / setForme
int staticLevel(const DSStaticPokemon& s, const nds::NARCArchive& narc, int i) {
    if (static_cast<int>(s.levelEntries.size()) <= i) return 1;
    const auto& e = s.levelEntries[i];
    return sbyte(narc.files.at(e.file), e.offset);
}
void staticSetLevel(const DSStaticPokemon& s, nds::NARCArchive& narc, int level, int i) {
    if (static_cast<int>(s.levelEntries.size()) > i) {  // puede no tenerlo (p. ej. un huevo)
        const auto& e = s.levelEntries[i];
        narc.files.at(e.file).at(e.offset) = static_cast<uint8_t>(level);
    }
}
int staticForme(const DSStaticPokemon& s, const nds::NARCArchive& narc) {
    if (s.formeEntries.empty()) return 0;
    return sbyte(narc.files.at(s.formeEntries[0].file), s.formeEntries[0].offset);
}
void staticSetForme(const DSStaticPokemon& s, nds::NARCArchive& narc, int forme) {
    for (const auto& e : s.formeEntries) narc.files.at(e.file).at(e.offset) = static_cast<uint8_t>(forme);
}
void staticSetPokemon(const DSStaticPokemon& s, nds::NARCArchive& narc, const Species* pk) {
    for (const auto& e : s.speciesEntries) writeWord(narc.files.at(e.file), e.offset, pk->baseNumber());
}
}  // namespace

Species* Gen5RomHandler::staticGetPokemon(const DSStaticPokemon& s) const {
    const auto& e = s.speciesEntries.at(0);
    return species_.at(readWord(scriptNarc_.files.at(e.file), e.offset));  // getSpecies(): sin formas
}

std::vector<StaticEncounter> Gen5RomHandler::getStaticPokemon() {
    std::vector<StaticEncounter> sp;
    if (!romEntry_->hasStaticPokemonSupport()) return sp;
    const auto& eggOffsets = romEntry_->getArray("StaticEggPokemonOffsets");
    // encuentros fijos
    const auto& statics = romEntry_->getStaticPokemon();
    for (size_t i = 0; i < statics.size(); i++) {
        const DSStaticPokemon& s = statics[i];
        Species* base = staticGetPokemon(s);
        int forme = staticForme(s, scriptNarc_);
        StaticEncounter se(base);
        se.holder.setAltFormeAllowed();
        se.holder.setFormeNumber(forme);
        se.level = staticLevel(s, scriptNarc_, 0);
        se.isEgg = std::find(eggOffsets.begin(), eggOffsets.end(), static_cast<int>(i)) != eggOffsets.end();
        for (size_t le = 1; le < s.levelEntries.size(); le++) {
            StaticEncounter linked(base);
            linked.holder.setAltFormeAllowed();
            linked.holder.setFormeNumber(forme);
            linked.level = staticLevel(s, scriptNarc_, static_cast<int>(le));
            se.linkedEncounters.push_back(linked);
        }
        sp.push_back(se);
    }
    // Foongus/Amoonguss disfrazados de objeto
    auto mapNARC = readNARC(romEntry_->getFile("MapFiles"));
    for (const auto& s : romEntry_->getStaticPokemonFakeBall()) {
        Species* pk = staticGetPokemon(s);
        StaticEncounter se(pk);
        se.level = staticLevel(s, mapNARC, 0);
        for (size_t le = 1; le < s.levelEntries.size(); le++) {
            StaticEncounter linked(pk);
            linked.level = staticLevel(s, mapNARC, static_cast<int>(le));
            se.linkedEncounters.push_back(linked);
        }
        sp.push_back(se);
    }
    // Claros Ocultos (N2/B2)
    if (romEntry_->romType() == Type_BW2) {
        std::vector<Species*> allowed(pokes_.begin() + 1, pokes_.begin() + 494);
        for (int i : bw2HiddenHollowUnovaPokemon) allowed.push_back(pokes_.at(i));
        auto hhNARC = readNARC(romEntry_->getFile("HiddenHollows"));
        int hhIndex = romEntry_->getInt("HiddenHollowIndex");
        for (const Bytes& hh : hhNARC.files) {
            for (int version = 0; version < 2; version++) {
                if (version != hhIndex) continue;
                for (int rarity = 0; rarity < 3; rarity++) {
                    std::vector<size_t> group;  // índices en sp (en Java, referencias a los mismos objetos)
                    for (int g = 0; g < 4; g++) {
                        int base = version * 78 + rarity * 26;
                        StaticEncounter se(pokes_.at(readWord(hh, base + g * 2)));
                        se.holder.setAltFormeAllowed();
                        se.holder.setFormeNumber(sbyte(hh, base + 20 + g));
                        se.level = sbyte(hh, base + 12 + g);
                        se.maxLevel = sbyte(hh, base + 8 + g);
                        se.isEgg = false;
                        se.restrictedPool = true;
                        se.restrictedList = allowed;
                        bool original = true;
                        for (size_t idx : group) {
                            if (sp[idx].species() == se.species()) {
                                sp[idx].linkedEncounters.push_back(se);
                                original = false;
                                break;
                            }
                        }
                        if (original) {
                            group.push_back(sp.size());
                            sp.push_back(se);
                            if (!hiddenHollowCounted_) hiddenHollowCount_++;
                        }
                    }
                }
            }
        }
    }
    hiddenHollowCounted_ = true;
    // errantes
    const auto& roamers = romEntry_->getRoamingPokemon();
    if (!roamers.empty()) {
        int roamerOvl = romEntry_->getInt("RoamerOvlNumber");
        int firstSpeciesOffset = roamers.front().speciesOverlayOffsets.at(0);
        // en el código original es «mov r0, #0x2» (0x2002 leído como palabra)
        if (readWord(readOverlay(roamerOvl), firstSpeciesOffset) > static_cast<int>(pokes_.size())) applyBlackWhiteRoamerPatch();
        for (const auto& r : roamers) {
            Bytes ovl = readOverlay(roamerOvl);
            StaticEncounter se(pokes_.at(readWord(ovl, r.speciesOverlayOffsets.at(0))));
            se.level = r.levelOverlayOffsets.empty() ? 1 : sbyte(ovl, r.levelOverlayOffsets[0]);
            sp.push_back(se);
        }
    }
    return sp;
}

bool Gen5RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& staticPokemon) {
    if (!romEntry_->hasStaticPokemonSupport()) return false;
    size_t expected = romEntry_->getStaticPokemon().size() + romEntry_->getStaticPokemonFakeBall().size() +
                      static_cast<size_t>(hiddenHollowCount_) + romEntry_->getRoamingPokemon().size();
    if (staticPokemon.size() != expected) return false;
    auto it = staticPokemon.begin();
    auto next = [&]() -> const StaticEncounter& {
        if (it == staticPokemon.end()) throw std::runtime_error("NoSuchElementException");
        return *it++;
    };
    for (const auto& s : romEntry_->getStaticPokemon()) {
        const StaticEncounter& se = next();
        staticSetPokemon(s, scriptNarc_, se.species()->baseForme());
        staticSetForme(s, scriptNarc_, se.holder.formeNumber());
        staticSetLevel(s, scriptNarc_, se.level, 0);
        for (size_t i = 0; i < se.linkedEncounters.size(); i++) staticSetLevel(s, scriptNarc_, se.linkedEncounters[i].level, static_cast<int>(i + 1));
    }
    std::string mapPath = romEntry_->getFile("MapFiles");
    auto mapNARC = readNARC(mapPath);
    for (const auto& s : romEntry_->getStaticPokemonFakeBall()) {
        const StaticEncounter& se = next();
        staticSetPokemon(s, scriptNarc_, se.species()->baseForme());
        staticSetLevel(s, mapNARC, se.level, 0);
        for (size_t i = 0; i < se.linkedEncounters.size(); i++) staticSetLevel(s, mapNARC, se.linkedEncounters[i].level, static_cast<int>(i + 1));
    }
    writeNARC(mapPath, mapNARC);
    if (romEntry_->romType() == Type_BW2) {
        std::string hhPath = romEntry_->getFile("HiddenHollows");
        auto hhNARC = readNARC(hhPath);
        int hhIndex = romEntry_->getInt("HiddenHollowIndex");
        for (Bytes& hh : hhNARC.files) {
            for (int version = 0; version < 2; version++) {
                if (version != hhIndex) continue;
                for (int rarity = 0; rarity < 3; rarity++) {
                    int base = version * 78 + rarity * 26;
                    for (int g = 0; g < 4; g++) {
                        const StaticEncounter& se = next();
                        // el porcentaje de hembras sustituye al de la especie: 30 % salvo especies de un solo sexo
                        int genderRatio;
                        switch (se.species()->baseForme()->genderRatio) {
                            case 0xFE: genderRatio = 100; break;
                            case 0x00:
                            case 0xFF: genderRatio = 0; break;
                            default: genderRatio = 30; break;
                        }
                        auto put = [&](int grp, const StaticEncounter& e) {
                            writeWord(hh, base + grp * 2, e.species()->baseNumber());
                            hh.at(base + 16 + grp) = static_cast<uint8_t>(genderRatio);
                            hh.at(base + 20 + grp) = static_cast<uint8_t>(e.holder.formeNumber());
                            hh.at(base + 12 + grp) = static_cast<uint8_t>(e.level);
                            hh.at(base + 8 + grp) = static_cast<uint8_t>(e.maxLevel);
                        };
                        put(g, se);
                        for (const auto& linked : se.linkedEncounters) put(++g, linked);
                    }
                }
            }
        }
        writeNARC(hhPath, hhNARC);
    }
    int roamerOvl = romEntry_->getInt("RoamerOvlNumber");
    for (const auto& r : romEntry_->getRoamingPokemon()) {
        const StaticEncounter& se = next();
        int value = se.species()->baseForme()->number();
        Bytes ovl = readOverlay(roamerOvl);
        for (int o : r.speciesOverlayOffsets) writeWord(ovl, o, value);
        writeOverlay(roamerOvl, ovl);
        for (const auto& e : r.speciesScriptOffsets) writeWord(scriptNarc_.files.at(e.file), e.offset, value);
        ovl = readOverlay(roamerOvl);
        for (int o : r.levelOverlayOffsets) ovl.at(o) = static_cast<uint8_t>(se.level);
        writeOverlay(roamerOvl, ovl);
    }
    // En N/B el juego comprueba en varios sitios la especie de Reshiram/Zekrom para sacarlo de la caja
    // y ponerlo el primero del equipo: hay que cambiarla por la del nuevo estático
    if (romEntry_->romType() == Type_BW)
        fixBoxLegendaryBW1(staticPokemon.at(romEntry_->getInt("BoxLegendaryOffset")).species()->baseNumber());
    return true;
}

void Gen5RomHandler::fixBoxLegendaryBW1(int species) {
    int ovlNum = romEntry_->getInt("FieldOvlNumber");
    Bytes ov = readOverlay(ovlNum);
    if (romEntry_->isBlack()) {
        // en Negro la especie de Reshiram se carga de constantes relativas al PC: basta con cambiarlas
        int first = find(ov, blackBoxLegendaryCheckPrefix1);
        if (first > 0) writeFullInt(ov, first + static_cast<int>(blackBoxLegendaryCheckPrefix1.size()) / 2, species);
        int second = find(ov, blackBoxLegendaryCheckPrefix2);
        if (second > 0) writeFullInt(ov, second + static_cast<int>(blackBoxLegendaryCheckPrefix2.size()) / 2, species);
    } else {
        // en Blanco la especie de Zekrom se calcula (161 << 2): se cambia el código para cargarla de una constante
        int first = find(ov, whiteBoxLegendaryCheckPrefix1);
        if (first > 0) {
            first += static_cast<int>(whiteBoxLegendaryCheckPrefix1.size()) / 2;
            ov.at(first + 66) = 0x00;  // quita la carga de la cadena de depuración «scrcmd_pokemon_fld.c»
            ov.at(first + 67) = 0x00;
            writeFullInt(ov, first + 320, species);  // y en su hueco va la especie
            ov.at(first + 18) = 0x00;
            ov.at(first + 19) = 0x00;
            ov.at(first + 26) = 0x49;
            ov.at(first + 27) = 0x49;
        }
        int second = find(ov, whiteBoxLegendaryCheckPrefix2);
        if (second > 0) {
            second += static_cast<int>(whiteBoxLegendaryCheckPrefix2.size()) / 2;
            // «mov r4, #0» antes de «add r6, r0, #0» para no tocar la bandera Z
            ov.at(second + 502) = 0x00;
            ov.at(second + 503) = 0x24;
            ov.at(second + 504) = 0x06;
            ov.at(second + 505) = 0x1C;
            writeFullInt(ov, second + 556, species);
            ov.at(second + 78) = 0x77;
            ov.at(second + 79) = 0x49;
            ov.at(second + 80) = 0x00;
            ov.at(second + 81) = 0x00;
        }
    }
    writeOverlay(ovlNum, ov);
}

void Gen5RomHandler::applyBlackWhiteRoamerPatch() {
    int offset = romEntry_->getInt("GetRoamerFlagOffsetStartOffset");
    int ovlNum = romEntry_->getInt("RoamerOvlNumber");
    Bytes ov = readOverlay(ovlNum);
    // la función devuelve 0 para Thundurus, 1 para Tornadus y 2 para otros (nunca se usa): ese hueco
    // guarda la especie de Thundurus, que se carga relativa al PC en vez de calcular Tornadus + 1
    ov.at(offset + 10) = 0x00;
    writeFullInt(ov, offset + 20, species::thundurus);
    ov.at(offset + 6) = 0x03;
    ov.at(offset + 7) = 0x49;
    writeOverlay(ovlNum, ov);
}

}  // namespace pr::gen5
