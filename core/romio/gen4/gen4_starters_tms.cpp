// Iniciales, MT/MO y tutores de Gen 4 (port de Gen4RomHandler.getStarters/setStarters,
// getTMMoves/setTMMoves, getTMHMCompatibility, getMoveTutor*...).
#include <map>
#include <stdexcept>

#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "romio/rom_functions.hpp"
#include "util/java_compat.hpp"
#include "util/java_hash.hpp"

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
void readByteIntoFlags(const Bytes& data, std::vector<bool>& flags, int offsetIntoFlags, int offsetIntoData) {
    int b = data.at(offsetIntoData);
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++)
        flags[offsetIntoFlags + i] = ((b >> i) & 1) == 1;
}
uint8_t getByteFromFlags(const std::vector<bool>& flags, int offsetIntoFlags) {
    int b = 0;
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++)
        b |= (flags[offsetIntoFlags + i] ? 1 : 0) << i;
    return static_cast<uint8_t>(b);
}
// RomFunctions.StringLengthSD: String.length() de Java
int stringLength(const std::string& s) {
    int n = 0;
    for (char32_t c : jc::utf8ToU32(s)) n += c > 0xFFFF ? 2 : 1;
    return n;
}
int textCharsPerLine(int romType) { return romType == Type_HGSS ? hgssTextCharsPerLine : dpptTextCharsPerLine; }
// AbstractDSRomHandler.typeTMPaletteNumber
int typeTMPaletteNumber(std::optional<Type> t) {
    if (!t) return 411;
    switch (*t) {
        case Type::FIGHTING: return 398;
        case Type::DRAGON: return 399;
        case Type::WATER: return 400;
        case Type::PSYCHIC: return 401;
        case Type::NORMAL: return 402;
        case Type::POISON: return 403;
        case Type::ICE: return 404;
        case Type::GRASS: return 405;
        case Type::FIRE: return 406;
        case Type::DARK: return 407;
        case Type::STEEL: return 408;
        case Type::ELECTRIC: return 409;
        case Type::GROUND: return 410;
        case Type::ROCK: return 412;
        case Type::FLYING: return 413;
        case Type::BUG: return 610;
        default: return 411;
    }
}
const std::string& tmDataPrefix(int romType) {
    static const std::string dppt(dpptTMDataPrefix), hgss(hgssTMDataPrefix);
    return (romType == Type_DP || romType == Type_Plat) ? dppt : hgss;
}
}  // namespace

// ---------------- iniciales ----------------

std::vector<Species*> Gen4RomHandler::getStarters() const {
    if (romEntry_->romType() == Type_HGSS) {
        auto tails = romfn::search(arm9_, hgssStarterCodeSuffix);
        if (tails.size() == 1) {
            int so = tails[0] - 13;
            return {pokes_.at(readWord(arm9_, so)), pokes_.at(readWord(arm9_, so + 4)), pokes_.at(readWord(arm9_, so + 8))};
        }
        return {pokes_[species::chikorita], pokes_[species::cyndaquil], pokes_[species::totodile]};
    }
    Bytes data = readOverlay(romEntry_->getInt("StarterPokemonOvlNumber"));
    int o = romEntry_->getInt("StarterPokemonOffset");
    return {pokes_.at(readWord(data, o)), pokes_.at(readWord(data, o + 4)), pokes_.at(readWord(data, o + 8))};
}

bool Gen4RomHandler::setStarters(const std::vector<Species*>& ns) {
    if (ns.size() != 3) return false;
    const int rt = romEntry_->romType();
    if (rt == Type_HGSS) {
        auto tails = romfn::search(arm9_, hgssStarterCodeSuffix);
        if (tails.size() != 1) return false;
        int so = tails[0] - 13;
        writeWord(arm9_, so, ns[0]->number());
        writeWord(arm9_, so + 4, ns[1]->number());
        writeWord(arm9_, so + 8, ns[2]->number());
        // scripts del rival: comparan el inicial del jugador con 152 y 155
        for (int fileCheck : hgssFilesWithRivalScript) {
            Bytes& file = scriptNarc_.files.at(fileCheck);
            auto rivalOffsets = romfn::search(file, hgssRivalScriptMagic);
            if (rivalOffsets.size() == 1) {
                int base = rivalOffsets[0];
                writeWord(file, base + 8, ns[0]->number());
                int jumpAmount = readLong(file, base + 13);
                int secondBase = jumpAmount + base + 17;
                if (file.at(secondBase) == 0x11 && file.at(secondBase + 4) == species::cyndaquil)
                    writeWord(file, secondBase + 4, ns[1]->number());
            }
        }
        // texto de la pantalla de elección (en inglés, como FVX)
        auto sp = getStrings(romEntry_->getInt("StarterScreenTextOffset"));
        const char* intros[] = {"So, you like", "You'll take", "Do you want"};
        for (int i = 0; i < 3; i++) {
            const Species* s = ns[i];
            int color = (i == 0) ? 3 : i;
            std::string type = romfn::camelCase(typeName(*s->primaryType(false)));
            sp.at(i + 1) = std::string("Professor Elm: ") + intros[i] + " \\vFF00\\z000" + std::to_string(color) + s->name() +
                           "\\vFF00\\z0000,\\nthe " + type + "-type Pokémon?";
            sp.at(i + 4) = "\\vFF00\\z000" + std::to_string(color) + s->name() + "\\vFF00\\z0000, the " + type +
                           "-type Pokémon, is\\nin this Poké Ball!";
        }
        setStrings(romEntry_->getInt("StarterScreenTextOffset"), sp);
        // gritos
        Bytes ovl = readOverlay(romEntry_->getInt("StarterPokemonOvlNumber"));
        std::string criesPrefix(starterCriesPrefix);
        int offset = find(ovl, criesPrefix);
        if (offset > 0) {
            offset += static_cast<int>(criesPrefix.size()) / 2;
            for (const Species* s : ns) {
                writeLong(ovl, offset, s->number());
                offset += 4;
            }
        }
        writeOverlay(romEntry_->getInt("StarterPokemonOvlNumber"), ovl);
        return true;
    }

    // DPPt
    Bytes sd = readOverlay(romEntry_->getInt("StarterPokemonOvlNumber"));
    int so = romEntry_->getInt("StarterPokemonOffset");
    writeWord(sd, so, ns[0]->number());
    writeWord(sd, so + 4, ns[1]->number());
    writeWord(sd, so + 8, ns[2]->number());
    if (rt == Type_DP || rt == Type_Plat) {
        const std::string& gfxPrefix = romEntry_->getString("StarterPokemonGraphicsPrefix");
        int offset = find(sd, gfxPrefix);
        if (offset > 0) {
            // La rutina de los gráficos usa el índice como desplazamiento de un puntero; se cambia la base
            // del puntero para meter dos add/sub que permitan cualquier número de Gen 4.
            offset += static_cast<int>(gfxPrefix.size()) / 2;
            writeWord(sd, offset + 0xC, readWord(sd, offset + 0xA));
            if (offset % 4 == 0) sd.at(offset + 0xC) = static_cast<uint8_t>(sd.at(offset + 0xC) - 1);
            writeWord(sd, offset + 0xA, readWord(sd, offset + 0x8));
            sd.at(offset + 0xA) = static_cast<uint8_t>(sd.at(offset + 0xA) - 1);
            writeWord(sd, offset + 0x8, readWord(sd, offset + 0x6));
            writeWord(sd, offset + 0x6, readWord(sd, offset + 0x4));
            writeWord(sd, offset + 0x4, readWord(sd, offset + 0x2));
            writeWord(sd, offset + 0x2, 0x6828);
            writeWord(sd, offset, 0x182D);
            offset += 0x16;
            writeWord(sd, offset, 0x6828);
            offset += 0xA;
            for (int i = 0; i < 3; i++) {
                int diff = ns[i]->number() - (4 * (i + 1));
                int instr1 = 0x3200, instr2 = 0x3200;
                if (diff < 0) {
                    instr1 |= 0x800;
                    diff = -diff;
                } else if (diff > 255) {
                    instr2 |= 0xFF;
                    diff -= 255;
                }
                instr1 |= (diff & 0xFF);
                sd.at(offset) = static_cast<uint8_t>(4 * (i + 1));
                writeWord(sd, offset + 2, readWord(sd, offset + 4));
                writeWord(sd, offset + 4, instr1);
                writeWord(sd, offset + 8, instr2);
                offset += 0xE;
            }
            sd.at(offset) = 1;
            const std::string& inner = romEntry_->getString("StarterPokemonGraphicsPrefixInner");
            offset = find(sd, inner);
            if (offset > 0) {
                offset += static_cast<int>(inner.size()) / 2;
                sd.at(offset + 1) = 0x68;
            }
        }
    }
    writeOverlay(romEntry_->getInt("StarterPokemonOvlNumber"), sd);

    // scripts del rival (IfJump con los números de los iniciales)
    const auto& rivalFiles = rt == Type_Plat ? ptFilesWithRivalScript : dpFilesWithRivalScript;
    for (int fileCheck : rivalFiles) {
        Bytes& file = scriptNarc_.files.at(fileCheck);
        for (int base : romfn::search(file, dpptRivalScriptMagic)) {
            int jumpLoc = base + static_cast<int>(dpptRivalScriptMagic.size());
            int jumpTo = readLong(file, jumpLoc) + jumpLoc + 4;
            int w = readWord(file, jumpTo);
            if (w != 0xE5 && w != 0x28F && (w != 0x125 || rt != Type_Plat)) continue;
            writeWord(file, base + 0x8, ns[0]->number());
            writeWord(file, base + 0x15, ns[1]->number());
        }
    }
    // combates dobles con el rival o el amigo
    const auto& tagFiles = rt == Type_Plat ? ptFilesWithTagScript : dpFilesWithTagScript;
    const auto& m1 = dpptTagBattleScriptMagic1;
    const auto& m2 = dpptTagBattleScriptMagic2;
    for (int fileCheck : tagFiles) {
        Bytes& file = scriptNarc_.files.at(fileCheck);
        for (int base : romfn::search(file, m1)) {
            int second = base + static_cast<int>(m1.size()) + 2;
            if (second + static_cast<int>(m2.size()) > static_cast<int>(file.size())) continue;
            bool valid = true;
            for (size_t k = 0; k < m2.size(); k++)
                if (file[second + k] != m2[k]) {
                    valid = false;
                    break;
                }
            if (!valid) continue;
            int jumpLoc = second + static_cast<int>(m2.size());
            int jumpTo = readLong(file, jumpLoc) + jumpLoc + 4;
            if (readWord(file, jumpTo) != 0x1B) continue;
            if (readWord(file, base + 0x21) == species::turtwig) writeWord(file, base + 0x21, ns[0]->number());
            else writeWord(file, base + 0x21, ns[2]->number());
            writeWord(file, base + 0xE, ns[1]->number());
        }
    }
    // textos (en inglés, como FVX)
    auto sp = getStrings(romEntry_->getInt("StarterScreenTextOffset"));
    auto dex = getStrings(romEntry_->getInt("PokedexSpeciesTextOffset"));
    for (int i = 0; i < 3; i++) {
        int color = (i == 0) ? 3 : i;
        sp.at(i + 1) = "\\vFF00\\z000" + std::to_string(color) + dex.at(ns[i]->number()) + " " + ns[i]->name() +
                       "\\vFF00\\z0000!\\nWill you take this Pokémon?";
    }
    setStrings(romEntry_->getInt("StarterScreenTextOffset"), sp);
    auto loc = getStrings(romEntry_->getInt("StarterLocationTextOffset"));
    if (rt == Type_DP) {
        loc.at(dpStarterStringIndex) =
            "\\v0103\\z0000: Fwaaah!\\nYour Pokémon totally rocked!\\pBut mine was way tougher\\nthan yours!\\p...They were "
            "other people's\\nPokémon, though...\\pBut we had to use them...\\nThey won't mind, will they?\\p";
    } else {
        loc.at(ptStarterStringIndex) = "\\v0103\\z0000\\z0000: Then, I choose you!\\nI'm picking this one!\\p";
    }
    setStrings(romEntry_->getInt("StarterLocationTextOffset"), loc);
    return true;
}

bool Gen4RomHandler::supportsStarterHeldItems() const {
    return romEntry_->romType() == Type_DP || romEntry_->romType() == Type_Plat;
}

std::vector<std::optional<int>> Gen4RomHandler::getStarterHeldItems() const {
    const Bytes& file = scriptNarc_.files.at(romEntry_->getInt("StarterPokemonScriptOffset"));
    int id = readWord(file, romEntry_->getInt("StarterPokemonHeldItemOffset"));
    if (id >= 0 && id < static_cast<int>(items_.size()) && items_[id]) return {id};
    return {std::nullopt};
}

void Gen4RomHandler::setStarterHeldItems(const std::vector<std::optional<int>>& items) {
    if (items.size() != 1) throw std::invalid_argument("Incorrect amount of items given, must be 1");
    Bytes& file = scriptNarc_.files.at(romEntry_->getInt("StarterPokemonScriptOffset"));
    writeWord(file, romEntry_->getInt("StarterPokemonHeldItemOffset"), items[0] ? *items[0] : 0);
}

// ---------------- MT / MO ----------------

std::vector<int> Gen4RomHandler::getTMMoves() const {
    const std::string& prefix = tmDataPrefix(romEntry_->romType());
    int offset = find(arm9_, prefix);
    if (offset <= 0) return {};
    offset += static_cast<int>(prefix.size()) / 2;
    std::vector<int> tms;
    for (int i = 0; i < tmCount; i++) tms.push_back(readWord(arm9_, offset + i * 2));
    return tms;
}

std::vector<int> Gen4RomHandler::getHMMoves() const {
    const std::string& prefix = tmDataPrefix(romEntry_->romType());
    int offset = find(arm9_, prefix);
    if (offset <= 0) return {};
    offset += static_cast<int>(prefix.size()) / 2 + tmCount * 2;
    std::vector<int> hms;
    for (int i = 0; i < hmCount; i++) hms.push_back(readWord(arm9_, offset + i * 2));
    return hms;
}

void Gen4RomHandler::replaceAllStringsInEntry(int entry, std::map<std::string, std::string> replacements) {
    // solo nombres de movimientos y Pokémon: que no se partan en dos líneas por un espacio
    for (auto& [k, v] : replacements) v = romfn::replaceAll(v, " ", "_");
    std::vector<std::pair<std::string, std::string>> reps(replacements.begin(), replacements.end());
    int lineLength = textCharsPerLine(romEntry_->romType());
    auto strings = getStrings(entry);
    for (auto& s : strings) {
        bool needs = false;
        for (const auto& [k, v] : reps)
            if (s.find(k) != std::string::npos) {
                needs = true;
                break;
            }
        if (needs) {
            std::string ns = romfn::formatTextWithReplacements(s, &reps, "\\n", "\\l", "\\p", lineLength, stringLength);
            s = romfn::replaceAll(ns, "_", " ");
        }
    }
    setStrings(entry, strings);
}

void Gen4RomHandler::setBottomScreenTMText(int textOffset, int stringNumber, int newMoveIndex) {
    auto strings = getStrings(textOffset);
    const std::string& orig = strings.at(stringNumber);
    size_t post = orig.find('\\');
    std::string origName = orig.substr(0, post);
    bool allCaps = origName == jc::toUpperCase(origName);
    std::string newName = moves_.at(newMoveIndex)->name;
    if (allCaps) newName = jc::toUpperCase(newName);
    strings.at(stringNumber) = newName + orig.substr(post);
    setStrings(textOffset, strings);
}

void Gen4RomHandler::setTMMoves(const std::vector<int>& moveIndexes) {
    auto oldMoveIndexes = getTMMoves();
    const int rt = romEntry_->romType();
    const std::string& prefix = tmDataPrefix(rt);
    int offset = find(arm9_, prefix);
    if (offset <= 0) return;
    offset += static_cast<int>(prefix.size()) / 2;
    for (int i = 0; i < tmCount; i++) writeWord(arm9_, offset + i * 2, moveIndexes.at(i));

    // descripciones de las MT (de 5 líneas de movimiento a 3 de objeto)
    auto itemDescs = getStrings(romEntry_->getInt("ItemDescriptionsTextOffset"));
    auto moveDescs = getStrings(romEntry_->getInt("MoveDescriptionsTextOffset"));
    int cpl = textCharsPerLine(rt);
    for (int i = 0; i < tmCount; i++)
        itemDescs.at(i + tmItemOffset) =
            romfn::rewriteDescriptionForNewLineSize(moveDescs.at(moveIndexes[i]), "\\n", cpl, stringLength);
    setStrings(romEntry_->getInt("ItemDescriptionsTextOffset"), itemDescs);

    // paletas
    std::string pals(rt == Type_DP ? dpItemPalettesPrefix : pthgssItemPalettesPrefix);
    int offsPals = find(arm9_, pals);
    if (offsPals > 0)
        for (int i = 0; i < tmCount; i++)
            writeWord(arm9_, offsPals + i * 8 + 2, typeTMPaletteNumber(moves_.at(moveIndexes[i])->type));

    // textos que nombran la MT
    for (int i = 0; i < tmCount; i++) {
        int oldIdx = oldMoveIndexes.at(i), newIdx = moveIndexes[i], tmNumber = i + 1;
        if (const auto* entries = romEntry_->getTMTexts().get(tmNumber)) {
            JavaHashSet<int> textFiles;
            for (const auto& te : *entries) textFiles.add(te.textIndex);
            std::string oldName = moves_.at(oldIdx)->name, newName = moves_.at(newIdx)->name;
            if (rt == Type_HGSS && oldIdx == moves::roar) {
                oldName = jc::toUpperCase(oldName);
                newName = jc::toUpperCase(newName);
            }
            textFiles.forEach([&](const int& tf) { replaceAllStringsInEntry(tf, {{oldName, newName}}); });
        }
        if (const auto* te = romEntry_->getTmTextsGameCorner().get(tmNumber))
            setBottomScreenTMText(te->textIndex, te->stringNumber, newIdx);
        if (const auto* so = romEntry_->getTMScriptOffsetsFrontier().get(tmNumber))
            writeWord(scriptNarc_.files.at(romEntry_->getInt("FrontierScriptNumber")), *so, newIdx);
        if (const auto* sn = romEntry_->getTMTextsFrontier().get(tmNumber))
            setBottomScreenTMText(romEntry_->getInt("MiscUITextOffset"), *sn, newIdx);
    }
}

int Gen4RomHandler::getTMCount() const { return tmCount; }
int Gen4RomHandler::getHMCount() const { return hmCount; }

std::map<int, std::vector<bool>> Gen4RomHandler::getTMHMCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    int formeCount = getFormeCount(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const Bytes& data = pokeNarc_.files.at(i > pokemonCount ? i + formeOffset : i);
        std::vector<bool> flags(tmCount + hmCount + 1);
        for (int j = 0; j < 13; j++) readByteIntoFlags(data, flags, j * 8 + 1, bsTMHMCompatOffset + j);
        compat[pokes_[i]->number()] = flags;
    }
    return compat;
}

void Gen4RomHandler::setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) {
    // FVX usa el número de la especie como índice del archivo también para las formas (sin formeOffset)
    for (const auto& [num, flags] : data) {
        Bytes& d = pokeNarc_.files.at(num);
        for (int j = 0; j < 13; j++) d.at(bsTMHMCompatOffset + j) = getByteFromFlags(flags, j * 8 + 1);
    }
}

// ---------------- tutores ----------------

bool Gen4RomHandler::hasMoveTutors() const { return romEntry_->romType() != Type_DP; }

std::vector<int> Gen4RomHandler::getMoveTutorMoves() const {
    if (!hasMoveTutors()) return {};
    int base = romEntry_->getInt("MoveTutorMovesOffset");
    int amount = romEntry_->getInt("MoveTutorCount");
    int bytesPer = romEntry_->getInt("MoveTutorBytesCount");
    Bytes f = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    std::vector<int> mts;
    for (int i = 0; i < amount; i++) mts.push_back(readWord(f, base + i * bytesPer));
    return mts;
}

void Gen4RomHandler::setMoveTutorMoves(const std::vector<int>& mv) {
    if (!hasMoveTutors()) return;
    int base = romEntry_->getInt("MoveTutorMovesOffset");
    int amount = romEntry_->getInt("MoveTutorCount");
    int bytesPer = romEntry_->getInt("MoveTutorBytesCount");
    if (static_cast<int>(mv.size()) != amount) return;
    Bytes f = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    for (int i = 0; i < amount; i++) writeWord(f, base + i * bytesPer, mv[i]);
    writeOverlay(romEntry_->getInt("FieldOvlNumber"), f);
    if (romEntry_->romType() == Type_HGSS) {
        // en HGSS el tutor de Golpe Cabeza (el último) usa un script propio
        int repl = mv.back();
        Bytes& ilex = scriptNarc_.files.at(ilexForestScriptFile);
        for (int o : headbuttTutorScriptOffsets) writeWord(ilex, o, repl);
        replaceAllStringsInEntry(ilexForestStringsFile, {{moves_.at(moves::headbutt)->name, moves_.at(repl)->name}});
    }
}

std::map<int, std::vector<bool>> Gen4RomHandler::getMoveTutorCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    if (!hasMoveTutors()) return compat;
    int amount = romEntry_->getInt("MoveTutorCount");
    int base = romEntry_->getInt("MoveTutorCompatOffset");
    int bytesPer = romEntry_->getInt("MoveTutorCompatBytesCount");
    Bytes f = romEntry_->romType() == Type_HGSS ? readFile(romEntry_->getFile("MoveTutorCompat"))
                                                : readOverlay(romEntry_->getInt("MoveTutorCompatOvlNumber"));
    int formeCount = getFormeCount(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        std::vector<bool> flags(amount + 1);
        for (int j = 0; j < bytesPer; j++) readByteIntoFlags(f, flags, j * 8 + 1, base + (i - 1) * bytesPer + j);
        compat[pokes_[i]->number()] = flags;
    }
    return compat;
}

void Gen4RomHandler::setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) {
    if (!hasMoveTutors()) return;
    int amount = romEntry_->getInt("MoveTutorCount");
    int base = romEntry_->getInt("MoveTutorCompatOffset");
    int bytesPer = romEntry_->getInt("MoveTutorCompatBytesCount");
    bool hgss = romEntry_->romType() == Type_HGSS;
    Bytes f = hgss ? readFile(romEntry_->getFile("MoveTutorCompat")) : readOverlay(romEntry_->getInt("MoveTutorCompatOvlNumber"));
    for (const auto& [num, flags] : data) {
        for (int j = 0; j < bytesPer; j++) {
            int o = base + (num - 1) * bytesPer + j;
            if (j * 8 + 8 <= amount) {
                f.at(o) = getByteFromFlags(flags, j * 8 + 1);
            } else if (j * 8 < amount) {
                int newByte = getByteFromFlags(flags, j * 8 + 1);
                int sh = 8 - amount + j * 8;
                // byte de Java con signo: (b >>> sh) << sh conserva los mismos 8 bits bajos
                int oldParts = (f.at(o) >> sh) << sh;
                f.at(o) = static_cast<uint8_t>(newByte | oldParts);
            }
        }
    }
    if (hgss) writeFile(romEntry_->getFile("MoveTutorCompat"), f);
    else writeOverlay(romEntry_->getInt("MoveTutorCompatOvlNumber"), f);
}

}  // namespace pr::gen4
