// Movimientos por nivel y movimientos huevo de Gen 4.
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

namespace {
int read2(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void write2(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}

std::map<int, std::vector<int>> readEggMoves(const Bytes& data, int start) {
    std::map<int, std::vector<int>> eggMoves;
    int off = start, currentSpecies = 0;
    std::vector<int> currentMoves;
    int val = read2(data, off);
    while (val != 0xFFFF) {
        if (val > 20000) {
            if (!currentMoves.empty()) eggMoves[currentSpecies] = currentMoves;
            currentSpecies = val - 20000;
            currentMoves.clear();
        } else {
            currentMoves.push_back(val);
        }
        off += 2;
        val = read2(data, off);
    }
    if (!currentMoves.empty()) eggMoves[currentSpecies] = currentMoves;
    return eggMoves;
}

void writeEggMoves(const std::map<int, std::vector<int>>& eggMoves, Bytes& data, int start) {
    int off = start;
    for (const auto& [sp, moves] : eggMoves) {
        write2(data, off, sp + 20000);
        off += 2;
        for (int m : moves) {
            write2(data, off, m);
            off += 2;
        }
    }
}
}  // namespace

std::map<int, std::vector<MoveLearnt>> Gen4RomHandler::getMovesLearnt() const {
    std::map<int, std::vector<MoveLearnt>> movesets;
    auto narc = readNARC(romEntry_->getFile("PokemonMovesets"));
    int formeCount = getFormeCount(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const Bytes& rom = narc.files.at(i > pokemonCount ? i + formeOffset : i);
        int loc = 0;
        std::vector<MoveLearnt> learnt;
        while (rom.at(loc) != 0xFF || rom.at(loc + 1) != 0xFF) {
            int move = rom[loc];
            int level = (rom[loc + 1] & 0xFE) >> 1;
            if (rom[loc + 1] & 0x01) move += 256;
            learnt.push_back({move, level});
            loc += 2;
        }
        movesets[pokes_[i]->number()] = learnt;
    }
    return movesets;
}

void Gen4RomHandler::setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) {
    nds::NARCArchive narc;
    const Bytes blank = {0xFF, 0xFF, 0, 0};
    narc.files.push_back(blank);
    int formeCount = getFormeCount(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        if (i == pokemonCount + 1)
            for (int j = 0; j < formeOffset; j++) narc.files.push_back(blank);
        const auto& learnt = movesets.at(pokes_[i]->number());
        size_t sizeNeeded = learnt.size() * 2 + 2;
        if (sizeNeeded % 4 != 0) sizeNeeded += 2;
        Bytes moveset(sizeNeeded, 0);
        size_t j = 0;
        for (; j < learnt.size(); j++) {
            moveset[j * 2] = static_cast<uint8_t>(learnt[j].move & 0xFF);
            int levelPart = (learnt[j].level << 1) & 0xFE;
            if (learnt[j].move > 255) levelPart++;
            moveset[j * 2 + 1] = static_cast<uint8_t>(levelPart);
        }
        moveset[j * 2] = 0xFF;
        moveset[j * 2 + 1] = 0xFF;
        narc.files.push_back(moveset);
    }
    writeNARC(romEntry_->getFile("PokemonMovesets"), narc);
}

std::map<int, std::vector<int>> Gen4RomHandler::getEggMoves() const {
    if (romEntry_->romType() == Type_HGSS) {
        auto narc = readNARC(romEntry_->getFile("EggMoves"));
        return readEggMoves(narc.files.at(0), 0);
    }
    Bytes fieldOvl = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    int offset = find(fieldOvl, std::string(dpptEggMoveTablePrefix));
    if (offset > 0) return readEggMoves(fieldOvl, offset + static_cast<int>(dpptEggMoveTablePrefix.size()) / 2);
    return {};
}

void Gen4RomHandler::setEggMoves(const std::map<int, std::vector<int>>& eggMoves) {
    if (romEntry_->romType() == Type_HGSS) {
        auto narc = readNARC(romEntry_->getFile("EggMoves"));
        writeEggMoves(eggMoves, narc.files.at(0), 0);
        writeNARC(romEntry_->getFile("EggMoves"), narc);
        return;
    }
    int ovl = romEntry_->getInt("FieldOvlNumber");
    Bytes fieldOvl = readOverlay(ovl);
    int offset = find(fieldOvl, std::string(dpptEggMoveTablePrefix));
    if (offset > 0) {
        writeEggMoves(eggMoves, fieldOvl, offset + static_cast<int>(dpptEggMoveTablePrefix.size()) / 2);
        writeOverlay(ovl, fieldOvl);
    }
}

}  // namespace pr::gen4
