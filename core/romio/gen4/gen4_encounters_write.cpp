// Escritura de Pokémon salvajes de Gen 4 y de las zonas de la Pokédex (port de Gen4RomHandler.setEncounters,
// write*EncountersDPPt/HGSS y updatePokedexAreaData*).
#include <array>
#include <set>
#include <stdexcept>

#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

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
void writeLong(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}

using AreaIt = std::vector<EncounterArea>::const_iterator;
const EncounterArea& next(AreaIt& it) { return *it++; }
int baseNum(const EncounterPtr& e) { return e->species()->baseNumber(); }

void writeEncountersDPPt(Bytes& d, int offset, const EncounterArea& a, int len) {
    for (int i = 0; i < len; i++) {
        const auto& e = a.encounters.at(i);
        writeLong(d, offset + i * 8, e->level);
        writeLong(d, offset + i * 8 + 4, baseNum(e));
    }
}
void writeSeaEncountersDPPt(Bytes& d, int offset, const EncounterArea& a) {
    for (size_t i = 0; i < a.encounters.size(); i++) {
        const auto& e = a.encounters[i];
        writeLong(d, offset + static_cast<int>(i) * 8, (e->level << 8) + e->maxLevel);
        writeLong(d, offset + static_cast<int>(i) * 8 + 4, baseNum(e));
    }
}
void writeExtraSlotsDPPt(Bytes& d, int offset, const EncounterArea& a) {
    for (size_t i = 0; i < a.encounters.size(); i++) writeLong(d, offset + static_cast<int>(i) * 4, baseNum(a.encounters[i]));
}
void writeOptionalEncountersHGSS(Bytes& d, int offset, int amount, AreaIt& it) {
    const EncounterArea* area = nullptr;
    size_t k = 0;
    for (int i = 0; i < amount; i++) {
        if (readWord(d, offset + i * 2) != 0) {
            if (!area) area = &next(it);
            writeWord(d, offset + i * 2, baseNum(area->encounters.at(k++)));
        }
    }
}
void writePokemonHGSS(Bytes& d, int offset, const EncounterArea& a) {
    for (size_t i = 0; i < a.encounters.size(); i++) writeWord(d, offset + static_cast<int>(i) * 2, baseNum(a.encounters[i]));
}
void writeSeaEncountersHGSS(Bytes& d, int offset, const EncounterArea& a) {
    for (size_t i = 0; i < a.encounters.size(); i++) {
        const auto& e = a.encounters[i];
        int o = offset + static_cast<int>(i) * 4;
        d.at(o) = static_cast<uint8_t>(e->level);
        d.at(o + 1) = static_cast<uint8_t>(e->maxLevel);
        writeWord(d, o + 2, baseNum(e));
    }
}

// PokedexAreaData / PokedexSpecialData (TreeSet<Integer> por especie)
struct DexArea {
    std::vector<std::array<std::set<int>, 3>> data = std::vector<std::array<std::set<int>, 3>>(pokemonCount);
    void add(int sp, int time, int map) { data.at(sp - 1)[time].insert(map); }
};
struct DexSpecial {
    std::vector<std::set<int>> data = std::vector<std::set<int>>(pokemonCount);
    void add(int sp, int map) { data.at(sp - 1).insert(map); }
};
Bytes makePokedexAreaDataFile(const std::set<int>& s) {
    Bytes out(s.size() * 4 + 4, 0);
    int idx = 0;
    for (int v : s) {
        writeLong(out, idx, v);
        idx += 4;
    }
    return out;
}
bool contains(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }
}  // namespace

void Gen4RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) {
    AreaIt it = areas.cbegin();
    if (romEntry_->romType() == Type_HGSS) {
        writeMainEncountersHGSS(useTimeOfDay, it);
        writeHeadbuttEncounters(it);
        writeBugCatchContestEncounters(it);
        updatePokedexAreaDataHGSS(areas);
    } else {
        writeMainEncountersDPPt(it, useTimeOfDay);
        writeExtraEncountersDPPt(it);
        updatePokedexAreaDataDPPt(areas);
    }
}

// ---------------- DPPt ----------------

void Gen4RomHandler::writeMainEncountersDPPt(AreaIt& it, bool useTimeOfDay) {
    const std::string& file = romEntry_->getFile("WildPokemon");
    auto data = readNARC(file);
    for (Bytes& b : data.files) {
        if (readLong(b, 0) != 0) {
            const EncounterArea& walking = next(it);
            writeEncountersDPPt(b, 4, walking, 12);
            // por horario
            int todSlot = 12;
            for (int i = 0; i < 4; i++) {
                int pknum = readLong(b, 108 + 4 * i);
                if (pknum >= 1 && pknum <= pokemonCount) {
                    const auto& e = useTimeOfDay ? walking.encounters.at(todSlot++) : walking.encounters.at(dpptAlternateSlots[i + 2]);
                    writeLong(b, 108 + 4 * i, e->species()->baseForme()->number());
                }
            }
            // enjambres, Pokéradar y cartucho de GBA
            const EncounterArea* cond = nullptr;
            size_t k = 0;
            for (int i = 0; i < 20; i++) {
                if (i >= 2 && i <= 5) continue;
                int offs = 100 + i * 4 + (i >= 10 ? 24 : 0);
                int pknum = readLong(b, offs);
                if (pknum >= 1 && pknum <= pokemonCount) {
                    if (!cond) cond = &next(it);
                    writeLong(b, offs, cond->encounters.at(k++)->species()->baseForme()->number());
                }
            }
        }
        // surf, relleno, caña vieja, buena y súper (el relleno se salta)
        int offset = 204;
        for (int i = 0; i < 5; i++) {
            int rate = readLong(b, offset);
            offset += 4;
            if (rate == 0 || i == 1) {
                offset += 40;
                continue;
            }
            writeSeaEncountersDPPt(b, offset, next(it));
            offset += 40;
        }
    }
    writeNARC(file, data);
}

void Gen4RomHandler::writeExtraEncountersDPPt(AreaIt& it) {
    const std::string& file = romEntry_->getFile("ExtraEncounters");
    auto extra = readNARC(file);
    Bytes ovl = readOverlay(romEntry_->getInt("EncounterOvlNumber"));
    // Feebas
    {
        const EncounterArea& feebas = next(it);
        std::string prefix(feebasLevelPrefixDPPt);
        int offset = find(ovl, prefix);
        if (offset > 0) {
            offset += static_cast<int>(prefix.size()) / 2;
            ovl.at(offset) = static_cast<uint8_t>(feebas.encounters.front()->maxLevel);
            ovl.at(offset + 4) = static_cast<uint8_t>(feebas.encounters.front()->level);
        }
        writeExtraSlotsDPPt(extra.files.at(0), 0, feebas);
    }
    // árboles de miel
    for (int hto : romEntry_->getArray("HoneyTreeOffsets")) {
        const EncounterArea& honey = next(it);
        std::string prefix(honeyTreeLevelPrefixDPPt);
        int offset = find(ovl, prefix);
        if (offset > 0) {
            offset += static_cast<int>(prefix.size()) / 2;
            int level = honey.encounters.front()->level;
            int maxLevel = honey.encounters.front()->maxLevel;
            // «mov r0, level», se anula la comprobación del rango original y «mov r0, r6, #5» -> «add r0, r0, r6»
            ovl.at(offset + 46) = static_cast<uint8_t>(level);
            ovl.at(offset + 47) = 0x20;
            for (int k = 48; k <= 53; k++) ovl.at(offset + k) = 0x00;
            ovl.at(offset + 54) = 0x80;
            ovl.at(offset + 55) = 0x19;
            ovl.at(offset + 102) = static_cast<uint8_t>(maxLevel);
            // divisor del número aleatorio (0..0xFFFF) para el nuevo rango de niveles
            int divisor = (0xFFFF / (maxLevel - level + 1)) + 1;
            writeLong(ovl, offset + 148, divisor);
        }
        writeExtraSlotsDPPt(extra.files.at(hto), 0, honey);
    }
    // Jardín Trofeo (Mr. Backlot) y Gran Pantano
    writeExtraSlotsDPPt(extra.files.at(8), 0, next(it));
    for (int gm : {9, 10}) writeExtraSlotsDPPt(extra.files.at(gm), 0, next(it));
    writeOverlay(romEntry_->getInt("EncounterOvlNumber"), ovl);
    writeNARC(file, extra);
}

void Gen4RomHandler::updatePokedexAreaDataDPPt(const std::vector<EncounterArea>& areas) {
    auto data = readNARC(romEntry_->getFile("WildPokemon"));
    DexArea dunArea, owArea;
    DexSpecial dunPre, dunPost, owPre, owPost;
    for (size_t c = 0; c < data.files.size(); c++) {
        DexArea* target;
        DexSpecial* special;
        int index;
        if (dpptOverworldDexMaps.at(c) != -1) {
            target = &owArea;
            special = &owPost;
            index = dpptOverworldDexMaps[c];
        } else if (dpptDungeonDexMaps.at(c) != -1) {
            target = &dunArea;
            special = &dunPost;
            index = dpptDungeonDexMaps[c];
        } else {
            continue;
        }
        const Bytes& b = data.files[c];
        if (readLong(b, 0) != 0) {
            for (int i = 0; i < 12; i++) {
                int sp = pokes_.at(readLong(b, 4 + i * 8 + 4))->baseNumber();
                target->add(sp, 0, index);
                if (i != 2 && i != 3) {  // 2 y 3 solo por la mañana
                    target->add(sp, 1, index);
                    target->add(sp, 2, index);
                }
            }
            for (int i = 0; i < 4; i++) {
                int pknum = readLong(b, 108 + 4 * i);
                if (pknum >= 1 && pknum <= pokemonCount) target->add(pknum, i > 1 ? 2 : 1, index);
            }
            // de enjambres/Pokéradar/GBA solo el Pokéradar sale en la Pokédex
            for (int i = 6; i < 10; i++) {
                int pknum = readLong(b, 100 + i * 4);
                if (pknum >= 1 && pknum <= pokemonCount) special->add(pknum, index);
            }
        }
        for (int i = 0; i < 5; i++) {
            if (i == 1) continue;
            int offset = 204 + i * dpptWaterAreaSize;
            if (readLong(b, offset) == 0) continue;
            for (int k = 0; k < 5; k++) {
                int sp = pokes_.at(readLong(b, offset + 4 + k * 8 + 4))->baseNumber();
                for (int t = 0; t < 3; t++) target->add(sp, t, index);
            }
        }
    }
    // encuentros especiales que no están en el NARC
    for (const auto& area : areas) {
        const std::string& n = area.displayName;
        for (const auto& e : area.encounters) {
            int sp = baseNum(e);
            if (contains(n, "Mt. Coronet Feebas Tiles")) {
                dunPre.add(sp, dpptMtCoronetDexIndex);
                dunPost.add(sp, dpptMtCoronetDexIndex);
            } else if (contains(n, "Honey Tree Group 1") || contains(n, "Honey Tree Group 2")) {
                dunPre.add(sp, dpptFloaromaMeadowDexIndex);
                dunPost.add(sp, dpptFloaromaMeadowDexIndex);
                for (int m : dpptOverworldHoneyTreeDexIndicies) {
                    owPre.add(sp, m);
                    owPost.add(sp, m);
                }
            } else if (contains(n, "Trophy Garden Rotating Pokemon")) {
                dunPost.add(sp, dpptTrophyGardenDexIndex);
            } else if (contains(n, "Great Marsh Rotating Pokemon (Post-National Dex)")) {
                dunPost.add(sp, dpptGreatMarshDexIndex);
            } else if (contains(n, "Great Marsh Rotating Pokemon (Pre-National Dex)")) {
                dunPre.add(sp, dpptGreatMarshDexIndex);
            }
        }
    }
    const std::string& file = romEntry_->getFile("PokedexAreaData");
    auto dex = readNARC(file);
    int dunIdx = romEntry_->getInt("PokedexAreaDataDungeonIndex");
    int dunPreIdx = romEntry_->getInt("PokedexAreaDataDungeonSpecialPreNationalIndex");
    int dunPostIdx = romEntry_->getInt("PokedexAreaDataDungeonSpecialPostNationalIndex");
    int owIdx = romEntry_->getInt("PokedexAreaDataOverworldIndex");
    int owPreIdx = romEntry_->getInt("PokedexAreaDataOverworldSpecialPreNationalIndex");
    int owPostIdx = romEntry_->getInt("PokedexAreaDataOverworldSpecialPostNationalIndex");
    for (int pk = 1; pk <= pokemonCount; pk++) {
        for (int t = 0; t < 3; t++) {
            dex.files.at(dunIdx + pk + t * pokedexAreaDataSize) = makePokedexAreaDataFile(dunArea.data[pk - 1][t]);
            dex.files.at(owIdx + pk + t * pokedexAreaDataSize) = makePokedexAreaDataFile(owArea.data[pk - 1][t]);
        }
        dex.files.at(dunPreIdx + pk) = makePokedexAreaDataFile(dunPre.data[pk - 1]);
        dex.files.at(dunPostIdx + pk) = makePokedexAreaDataFile(dunPost.data[pk - 1]);
        dex.files.at(owPreIdx + pk) = makePokedexAreaDataFile(owPre.data[pk - 1]);
        dex.files.at(owPostIdx + pk) = makePokedexAreaDataFile(owPost.data[pk - 1]);
    }
    writeNARC(file, dex);
}

// ---------------- HGSS ----------------

void Gen4RomHandler::writeMainEncountersHGSS(bool useTimeOfDay, AreaIt& it) {
    const std::string& file = romEntry_->getFile("WildPokemon");
    auto data = readNARC(file);
    const int amounts[] = {0, 5, 2, 5, 5, 5};
    for (Bytes& b : data.files) {
        int rates[6];
        for (int i = 0; i < 6; i++) rates[i] = b.at(i);
        if (rates[0] != 0) {
            const EncounterArea* walking = &next(it);
            for (size_t i = 0; i < walking->encounters.size(); i++) b.at(8 + i) = static_cast<uint8_t>(walking->encounters[i]->level);
            writePokemonHGSS(b, 20, *walking);
            if (!useTimeOfDay) {
                writePokemonHGSS(b, 44, *walking);
                writePokemonHGSS(b, 68, *walking);
            } else {
                for (int i = 1; i < 3; i++) {
                    walking = &next(it);
                    writePokemonHGSS(b, 20 + i * 24, *walking);
                }
            }
        }
        writeOptionalEncountersHGSS(b, 92, 4, it);  // radio
        int offset = 100;
        for (int i = 1; i < 6; i++) {
            if (rates[i] != 0) writeSeaEncountersHGSS(b, offset, next(it));
            offset += 4 * amounts[i];
        }
        writeOptionalEncountersHGSS(b, offset, 2, it);      // enjambres
        writeOptionalEncountersHGSS(b, offset + 4, 1, it);  // pesca nocturna
        writeOptionalEncountersHGSS(b, offset + 6, 1, it);  // enjambre de pesca
    }
    writeNARC(file, data);
}

void Gen4RomHandler::writeHeadbuttEncounters(AreaIt& it) {
    const std::string& file = romEntry_->getFile("HeadbuttPokemon");
    auto data = readNARC(file);
    int c = -1;
    for (Bytes& b : data.files) {
        c++;
        // como al leer: sin datos o el mapa 24 (sin usar)
        if (b.size() == 4 || c == 24) continue;
        const EncounterArea& a = next(it);
        for (size_t i = 0; i < a.encounters.size(); i++) {
            const auto& e = a.encounters[i];
            int o = 4 + static_cast<int>(i) * 4;
            writeWord(b, o, baseNum(e));
            b.at(o + 2) = static_cast<uint8_t>(e->level);
            b.at(o + 3) = static_cast<uint8_t>(e->maxLevel);
        }
    }
    writeNARC(file, data);
}

void Gen4RomHandler::writeBugCatchContestEncounters(AreaIt& it) {
    const std::string& file = romEntry_->getFile("BCCWilds");
    Bytes data = readFile(file);
    for (int base : {0, 80, 160, 240}) {
        const EncounterArea& a = next(it);
        for (size_t i = 0; i < a.encounters.size(); i++) {
            const auto& e = a.encounters[i];
            int o = base + static_cast<int>(i) * 8;
            writeWord(data, o, baseNum(e));
            data.at(o + 2) = static_cast<uint8_t>(e->level);
            data.at(o + 3) = static_cast<uint8_t>(e->maxLevel);
        }
    }
    writeFile(file, data);
}

void Gen4RomHandler::updatePokedexAreaDataHGSS(const std::vector<EncounterArea>& areas) {
    auto data = readNARC(romEntry_->getFile("WildPokemon"));
    DexArea dunArea, owArea;
    DexSpecial dunSpecial, owSpecial;
    const int amounts[] = {0, 5, 2, 5, 5, 5};
    for (size_t c = 0; c < data.files.size(); c++) {
        DexArea* target;
        DexSpecial* special;
        int index;
        if (hgssOverworldDexMaps.at(c) != -1) {
            target = &owArea;
            special = &owSpecial;
            index = hgssOverworldDexMaps[c];
        } else if (hgssDungeonDexMaps.at(c) != -1) {
            target = &dunArea;
            special = &dunSpecial;
            index = hgssDungeonDexMaps[c];
        } else {
            continue;
        }
        const Bytes& b = data.files[c];
        int rates[6];
        for (int i = 0; i < 6; i++) rates[i] = b.at(i);
        if (rates[0] != 0)
            for (int t = 0; t < 3; t++)
                for (int i = 0; i < 12; i++) target->add(pokes_.at(readWord(b, 20 + t * 24 + i * 2))->number(), t, index);
        // radio de Hoenn/Sinnoh
        for (int i = 0; i < 4; i++) {
            int pk = readWord(b, 92 + i * 2);
            if (pk != 0) special->add(pokes_.at(pk)->baseNumber(), index);
        }
        // surf, Golpe Roca y caña vieja
        int offset = 100;
        auto seaSpecies = [&](int off, int amount) {
            std::vector<int> v;
            for (int k = 0; k < amount; k++) v.push_back(pokes_.at(readWord(b, off + 2 + k * 4))->baseNumber());
            return v;
        };
        for (int i = 1; i < 4; i++) {
            if (rates[i] != 0)
                for (int sp : seaSpecies(offset, amounts[i]))
                    for (int t = 0; t < 3; t++) target->add(sp, t, index);
            offset += 4 * amounts[i];
        }
        // caña buena y súper: de noche un hueco cambia por el Pokémon de pesca nocturna
        // (en Java pokes[0] es null y solo se usa si hay caña: se lee aquí dentro)
        auto rod = [&](int off, int replacementIndex) {
            int nightReplacement = pokes_.at(readWord(b, 192))->baseNumber();
            auto dayMorning = seaSpecies(off, 5);
            for (int sp : dayMorning) {
                target->add(sp, 0, index);
                target->add(sp, 1, index);
            }
            auto night = dayMorning;
            night.at(replacementIndex) = nightReplacement;
            for (int sp : night) target->add(sp, 2, index);
        };
        if (rates[4] != 0) rod(offset, hgssGoodRodReplacementIndex);
        if (rates[5] != 0) rod(offset + 20, hgssSuperRodReplacementIndex);
    }
    // Golpe Cabeza: desde la primera área «Route 1 Headbutt»
    int start = -1;
    for (size_t i = 0; i < areas.size(); i++)
        if (contains(areas[i].displayName, "Route 1 Headbutt")) {
            start = static_cast<int>(i);
            break;
        }
    if (start != -1) {
        for (size_t i = 0; i < hgssHeadbuttOverworldDexMaps.size(); i++) {
            for (const auto& e : areas.at(start + i).encounters) {
                if (hgssHeadbuttOverworldDexMaps[i] != -1) owSpecial.add(baseNum(e), hgssHeadbuttOverworldDexMaps[i]);
                else if (hgssHeadbuttDungeonDexMaps.at(i) != -1) dunSpecial.add(baseNum(e), hgssHeadbuttDungeonDexMaps[i]);
            }
        }
    }
    const std::string& file = romEntry_->getFile("PokedexAreaData");
    auto dex = readNARC(file);
    int dunIdx = romEntry_->getInt("PokedexAreaDataDungeonIndex");
    int owIdx = romEntry_->getInt("PokedexAreaDataOverworldIndex");
    int dunSpIdx = romEntry_->getInt("PokedexAreaDataDungeonSpecialIndex");
    int owSpIdx = romEntry_->getInt("PokedexAreaDataOverworldSpecialIndex");
    for (int pk = 1; pk <= pokemonCount; pk++) {
        for (int t = 0; t < 3; t++) {
            dex.files.at(dunIdx + pk + t * pokedexAreaDataSize) = makePokedexAreaDataFile(dunArea.data[pk - 1][t]);
            dex.files.at(owIdx + pk + t * pokedexAreaDataSize) = makePokedexAreaDataFile(owArea.data[pk - 1][t]);
        }
        dex.files.at(dunSpIdx + pk) = makePokedexAreaDataFile(dunSpecial.data[pk - 1]);
        dex.files.at(owSpIdx + pk) = makePokedexAreaDataFile(owSpecial.data[pk - 1]);
    }
    writeNARC(file, dex);
}

}  // namespace pr::gen4
