// Iniciales, MT/MO y tutores de Gen 5 (port de Gen5RomHandler.getStarters/setStarters,
// getTMMoves/setTMMoves, getTMHMCompatibility, getMoveTutor*...).
#include <map>
#include <stdexcept>

#include "compress/dsdecmp.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"
#include "romio/rom_functions.hpp"
#include "util/java_compat.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
void writeLong(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
void writeRelativePointer(Bytes& d, int offset, int pointer) { writeLong(d, offset, pointer - offset - 4); }
void readByteIntoFlags(const Bytes& data, std::vector<bool>& flags, int offsetIntoFlags, int offsetIntoData) {
    int b = data.at(offsetIntoData);
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++) flags[offsetIntoFlags + i] = ((b >> i) & 1) == 1;
}
uint8_t getByteFromFlags(const std::vector<bool>& flags, int offsetIntoFlags) {
    int b = 0;
    for (int i = 0; i < 8 && (i + offsetIntoFlags) < static_cast<int>(flags.size()); i++) b |= (flags[offsetIntoFlags + i] ? 1 : 0) << i;
    return static_cast<uint8_t>(b);
}
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
// String.format(texto, tipo, nombre) con los marcadores %1$s y %2$s de la tabla de direcciones
std::string formatStoryText(const std::string& fmt, const std::string& a, const std::string& b) {
    std::string out;
    for (size_t i = 0; i < fmt.size(); i++) {
        if (fmt.compare(i, 4, "%1$s") == 0) {
            out += a;
            i += 3;
        } else if (fmt.compare(i, 4, "%2$s") == 0) {
            out += b;
            i += 3;
        } else if (fmt.compare(i, 2, "%%") == 0) {
            out += '%';
            i++;
        } else {
            out += fmt[i];
        }
    }
    return out;
}
// tablas de los tutores de N2/B2: los bits del archivo de Pokémon no siguen el orden de los movimientos
const int countsPersonalOrder[] = {15, 17, 13, 15};
const int countsMoveOrder[] = {13, 15, 15, 17};
const int personalToMoveOrder[] = {1, 3, 0, 2};
int tutorOffsetOfData(int mt) {
    int off = 0;
    for (int c = 0; c < personalToMoveOrder[mt]; c++) off += countsMoveOrder[c];
    return off;
}
}  // namespace

// ---------------- iniciales ----------------

std::vector<Species*> Gen5RomHandler::getStarters() const {
    std::vector<Species*> starters;
    for (int i = 0; i < 3; i++) {
        const auto& entry = romEntry_->getOffsetArrayEntry("StarterOffsets" + std::to_string(i + 1))->at(0);
        starters.push_back(pokes_.at(readWord(scriptNarc_.files.at(entry.file), entry.offset)));
    }
    return starters;
}

void Gen5RomHandler::replaceStarterFiles(nds::NARCArchive& starterNARC, const nds::NARCArchive& pokespritesNARC, int starterIndex,
                                         int pokeNumber) {
    starterNARC.files.at(starterIndex * 2) = pokespritesNARC.files.at(pokeNumber * 20 + 18);
    starterNARC.files.at(12 + starterIndex) = dsdecmp::decompress(pokespritesNARC.files.at(pokeNumber * 20));
}

bool Gen5RomHandler::setStarters(const std::vector<Species*>& newStarters) {
    if (newStarters.size() != 3) return false;
    for (int i = 0; i < 3; i++) {
        int starter = newStarters[i]->number();
        for (const auto& entry : *romEntry_->getOffsetArrayEntry("StarterOffsets" + std::to_string(i + 1)))
            writeWord(scriptNarc_.files.at(entry.file), entry.offset, starter);
    }
    // «¡devuélveme a mi Purrloin!»: el script que da la Pokédex salta a uno nuevo añadido al final
    bool bw2 = romEntry_->romType() == Type_BW2;
    const auto& newScript = bw2 ? bw2NewStarterScript : bw1NewStarterScript;
    int pdIdx = romEntry_->getInt("PokedexGivenFileOffset");
    const Bytes& oldFile = scriptNarc_.files.at(pdIdx);
    int offset = find(oldFile, bw2 ? bw2StarterScriptMagic : bw1StarterScriptMagic);
    if (offset > 0) {
        Bytes newFile(oldFile);
        newFile.insert(newFile.end(), newScript.begin(), newScript.end());
        int oldLen = static_cast<int>(oldFile.size());
        bool jp = romEntry_->romCode().at(3) == 'J';
        if (bw2) {
            if (jp) newFile.at(oldLen + 0x6) -= 4;
            newFile.at(offset++) = 0x1E;
        } else {
            if (jp) {
                newFile.at(oldLen + 0x4) -= 4;
                newFile.at(oldLen + 0x8) -= 4;
            }
            newFile.at(offset++) = 0x04;
        }
        newFile.at(offset++) = 0x0;
        writeRelativePointer(newFile, offset, oldLen);
        scriptNarc_.files.at(pdIdx) = std::move(newFile);
    }
    // imágenes de los iniciales
    std::string starterGfx = romEntry_->getFile("StarterGraphics");
    auto starterNARC = readNARC(starterGfx);
    auto pokespritesNARC = readNARC(romEntry_->getFile("PokemonGraphics"));
    for (int i = 0; i < 3; i++) replaceStarterFiles(starterNARC, pokespritesNARC, i, newStarters[i]->number());
    writeNARC(starterGfx, starterNARC);
    // gritos de los iniciales
    int cryOvl = romEntry_->getInt("StarterCryOvlNumber");
    Bytes starterCryOverlay = readOverlay(cryOvl);
    const std::string& cryPrefix = romEntry_->getString("StarterCryTablePrefix");
    offset = find(starterCryOverlay, cryPrefix);
    if (offset > 0) {
        offset += static_cast<int>(cryPrefix.size()) / 2;
        for (Species* s : newStarters) {
            writeWord(starterCryOverlay, offset, s->number());
            offset += 2;
        }
        writeOverlay(cryOvl, starterCryOverlay);
    }
    // textos según la versión
    std::string starterText = romEntry_->getStoryText("Starter");
    int locIdx = romEntry_->getInt("StarterLocationTextOffset");
    auto strings = getStrings(true, locIdx);
    if (!starterText.empty()) {
        int base = bw2 ? bw2StarterTextOffset : bw1StarterTextOffset;
        for (int i = 0; i < 3; i++) {
            Species* s = newStarters[i];
            strings.at(base - i) = formatStoryText(starterText, romfn::camelCase(typeName(*s->primaryType(false))), s->name());
        }
    }
    if (bw2) {
        std::string rivalText = romEntry_->getStoryText("Rival");
        if (!rivalText.empty()) strings.at(bw2RivalTextOffset) = rivalText;
    } else {
        std::string cheren1 = romEntry_->getStoryText("Cheren1"), cheren2 = romEntry_->getStoryText("Cheren2");
        if (!cheren1.empty()) strings.at(bw1CherenText1Offset) = cheren1;
        if (!cheren2.empty()) strings.at(bw1CherenText2Offset) = cheren2;
    }
    setStrings(true, locIdx, strings);
    return true;
}

// ---------------- MT / MO ----------------

std::vector<int> Gen5RomHandler::getTMMoves() const {
    int offset = find(arm9_, tmDataPrefix);
    if (offset <= 0) return {};  // null en Java
    offset += static_cast<int>(tmDataPrefix.size()) / 2;
    std::vector<int> tms;
    for (int i = 0; i < tmBlockOneCount; i++) tms.push_back(readWord(arm9_, offset + i * 2));
    offset += (tmBlockOneCount + hmCount) * 2;  // tras las 92 MT y las 6 MO
    for (int i = 0; i < tmCount - tmBlockOneCount; i++) tms.push_back(readWord(arm9_, offset + i * 2));
    return tms;
}

std::vector<int> Gen5RomHandler::getHMMoves() const {
    int offset = find(arm9_, tmDataPrefix);
    if (offset <= 0) return {};
    offset += static_cast<int>(tmDataPrefix.size()) / 2 + tmBlockOneCount * 2;
    std::vector<int> hms;
    for (int i = 0; i < hmCount; i++) hms.push_back(readWord(arm9_, offset + i * 2));
    return hms;
}

void Gen5RomHandler::setTMMoves(const std::vector<int>& moveIndexes) {
    int offset = find(arm9_, tmDataPrefix);
    if (offset <= 0) return;
    offset += static_cast<int>(tmDataPrefix.size()) / 2;
    for (int i = 0; i < tmBlockOneCount; i++) writeWord(arm9_, offset + i * 2, moveIndexes.at(i));
    offset += (tmBlockOneCount + hmCount) * 2;
    for (int i = 0; i < tmCount - tmBlockOneCount; i++) writeWord(arm9_, offset + i * 2, moveIndexes.at(i + tmBlockOneCount));
    // descripciones de las MT (MT01 es el objeto 328; MT93-95 son 618-620)
    int itemDescIdx = romEntry_->getInt("ItemDescriptionsTextOffset");
    auto itemDescriptions = getStrings(false, itemDescIdx);
    auto moveDescriptions = getStrings(false, romEntry_->getInt("MoveDescriptionsTextOffset"));
    for (int i = 0; i < tmBlockOneCount; i++) itemDescriptions.at(i + tmBlockOneOffset) = moveDescriptions.at(moveIndexes[i]);
    for (int i = 0; i < tmCount - tmBlockOneCount; i++)
        itemDescriptions.at(i + tmBlockTwoOffset) = moveDescriptions.at(moveIndexes[i + tmBlockOneCount]);
    setStrings(false, itemDescIdx, itemDescriptions);
    // paletas (color de la MT según el tipo)
    int offsPals = find(arm9_, romEntry_->romType() == Type_BW ? bw1ItemPalettesPrefix : bw2ItemPalettesPrefix);
    if (offsPals > 0) {
        for (int i = 0; i < tmBlockOneCount; i++) {
            int itmNum = tmBlockOneOffset + i;
            writeWord(arm9_, offsPals + itmNum * 4 + 2, typeTMPaletteNumber(moves_.at(moveIndexes[i])->type));
        }
        for (int i = 0; i < tmCount - tmBlockOneCount; i++) {
            int itmNum = tmBlockTwoOffset + i;
            writeWord(arm9_, offsPals + itmNum * 4 + 2, typeTMPaletteNumber(moves_.at(moveIndexes[i + tmBlockOneCount])->type));
        }
    }
}

int Gen5RomHandler::getTMCount() const { return tmCount; }
int Gen5RomHandler::getHMCount() const { return hmCount; }

std::map<int, std::vector<bool>> Gen5RomHandler::getTMHMCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    int formeCount = getFormeCount(romEntry_->romType());
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const Bytes& data = pokeNarc_.files.at(i > pokemonCount ? i + formeOffset : i);
        std::vector<bool> flags(tmCount + hmCount + 1);
        for (int j = 0; j < 13; j++) readByteIntoFlags(data, flags, j * 8 + 1, bsTMHMCompatOffset + j);
        compat[pokes_[i]->number()] = flags;
    }
    return compat;
}

void Gen5RomHandler::setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) {
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (const auto& [num, flags] : data) {
        Bytes& d = pokeNarc_.files.at(num > pokemonCount ? num + formeOffset : num);
        for (int j = 0; j < 13; j++) d.at(bsTMHMCompatOffset + j) = getByteFromFlags(flags, j * 8 + 1);
    }
}

// ---------------- tutores (solo N2/B2) ----------------

bool Gen5RomHandler::hasMoveTutors() const { return romEntry_->romType() == Type_BW2; }

std::vector<int> Gen5RomHandler::getMoveTutorMoves() const {
    if (!hasMoveTutors()) return {};
    int base = romEntry_->getInt("MoveTutorDataOffset");
    Bytes mtFile = readOverlay(romEntry_->getInt("MoveTutorOvlNumber"));
    std::vector<int> moves;
    for (int i = 0; i < bw2MoveTutorCount; i++) moves.push_back(readWord(mtFile, base + i * bw2MoveTutorBytesPerEntry));
    return moves;
}

void Gen5RomHandler::setMoveTutorMoves(const std::vector<int>& moves) {
    if (!hasMoveTutors()) return;
    if (static_cast<int>(moves.size()) != bw2MoveTutorCount) return;
    int base = romEntry_->getInt("MoveTutorDataOffset");
    int ovl = romEntry_->getInt("MoveTutorOvlNumber");
    Bytes mtFile = readOverlay(ovl);
    for (int i = 0; i < bw2MoveTutorCount; i++) writeWord(mtFile, base + i * bw2MoveTutorBytesPerEntry, moves[i]);
    writeOverlay(ovl, mtFile);
}

std::map<int, std::vector<bool>> Gen5RomHandler::getMoveTutorCompatibility() const {
    std::map<int, std::vector<bool>> compat;
    if (!hasMoveTutors()) return compat;
    int formeCount = getFormeCount(romEntry_->romType());
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const Bytes& data = pokeNarc_.files.at(i > pokemonCount ? i + formeOffset : i);
        std::vector<bool> flags(bw2MoveTutorCount + 1);
        for (int mt = 0; mt < 4; mt++) {
            std::vector<bool> mtflags(countsPersonalOrder[mt] + 1);
            for (int j = 0; j < 4; j++) readByteIntoFlags(data, mtflags, j * 8 + 1, bsMTCompatOffset + mt * 4 + j);
            int off = tutorOffsetOfData(mt);
            for (int k = 0; k < countsPersonalOrder[mt]; k++) flags.at(off + 1 + k) = mtflags[1 + k];
        }
        compat[pokes_[i]->number()] = flags;
    }
    return compat;
}

void Gen5RomHandler::setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) {
    if (!hasMoveTutors()) return;
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (const auto& [num, flags] : data) {
        Bytes& d = pokeNarc_.files.at(num > pokemonCount ? num + formeOffset : num);
        for (int mt = 0; mt < 4; mt++) {
            int off = tutorOffsetOfData(mt);
            std::vector<bool> mtflags(countsPersonalOrder[mt] + 1);
            for (int k = 0; k < countsPersonalOrder[mt]; k++) mtflags[1 + k] = flags.at(off + 1 + k);
            for (int j = 0; j < 4; j++) d.at(bsMTCompatOffset + mt * 4 + j) = getByteFromFlags(mtflags, j * 8 + 1);
        }
    }
}

}  // namespace pr::gen5
