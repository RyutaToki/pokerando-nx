// Evoluciones, movimientos por nivel y movimientos huevo de Gen 3
// (port de Gen3RomHandler.loadEvolutions/writeEvolutions/getMovesLearnt/getEggMoves...).
#include <algorithm>

#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

namespace {
constexpr EvolutionType kEvolutionTypeTable[] = {
    EvolutionType::HAPPINESS, EvolutionType::HAPPINESS_DAY, EvolutionType::HAPPINESS_NIGHT, EvolutionType::LEVEL,
    EvolutionType::TRADE, EvolutionType::TRADE_ITEM, EvolutionType::STONE, EvolutionType::LEVEL_ATTACK_HIGHER,
    EvolutionType::LEVEL_ATK_DEF_SAME, EvolutionType::LEVEL_DEFENSE_HIGHER, EvolutionType::LEVEL_LOW_PV,
    EvolutionType::LEVEL_HIGH_PV, EvolutionType::LEVEL_CREATE_EXTRA, EvolutionType::LEVEL_IS_EXTRA,
    EvolutionType::HIGH_BEAUTY};
}  // namespace

int evolutionTypeToIndex(EvolutionType t) {
    for (size_t i = 0; i < std::size(kEvolutionTypeTable); i++) {
        if (kEvolutionTypeTable[i] == t) return static_cast<int>(i) + 1;
    }
    return -1;
}

EvolutionType evolutionTypeFromIndex(int index) {
    if (index == -1) return EvolutionType::NONE;
    return kEvolutionTypeTable[index - 1];
}

void Gen3RomHandler::loadEvolutions() {
    for (Species* pk : pokes_) {
        if (pk) {
            pk->evolutionsFrom().clear();
            pk->evolutionsTo().clear();
        }
    }
    int baseOffset = romEntry_->getInt("PokemonEvolutions");
    int numInternalPokes = romEntry_->getInt("PokemonCount");
    for (int i = 1; i <= numRealPokemon_; i++) {
        Species* pk = speciesList_[i];
        int idx = pokedexToInternal_[pk->number()];
        int evoOffset = baseOffset + idx * 0x28;
        for (int j = 0; j < 5; j++) {
            int method = readWord(evoOffset + j * 8);
            int evolvingTo = readWord(evoOffset + j * 8 + 4);
            if (method >= 1 && method <= evolutionMethodCount && evolvingTo >= 1 && evolvingTo <= numInternalPokes) {
                EvolutionType et = evolutionTypeFromIndex(method);
                int extraInfo = readWord(evoOffset + j * 8 + 2);
                if (usesItem(et)) extraInfo = itemIDToStandard(extraInfo);
                auto evo = std::make_shared<Evolution>(pk, pokesInternal_[evolvingTo], et, extraInfo);
                auto& from = pk->evolutionsFrom();
                bool exists = std::any_of(from.begin(), from.end(), [&](const EvolutionPtr& e) { return e->sameAs(*evo); });
                if (!exists) {
                    from.push_back(evo);
                    pokesInternal_[evolvingTo]->evolutionsTo().push_back(evo);
                }
            }
        }
    }
}

void Gen3RomHandler::writeEvolutions() {
    int baseOffset = romEntry_->getInt("PokemonEvolutions");
    for (int i = 1; i <= numRealPokemon_; i++) {
        Species* pk = speciesList_[i];
        int idx = pokedexToInternal_[pk->number()];
        int evoOffset = baseOffset + idx * 0x28;
        int evosWritten = 0;
        for (const auto& evo : pk->evolutionsFrom()) {
            writeWord(evoOffset, evolutionTypeToIndex(evo->type()));
            int extraInfo = evo->extraInfo();
            if (usesItem(evo->type())) extraInfo = itemIDToInternal(extraInfo);
            writeWord(evoOffset + 2, extraInfo);
            writeWord(evoOffset + 4, pokedexToInternal_[evo->to()->number()]);
            writeWord(evoOffset + 6, 0);
            evoOffset += 8;
            if (++evosWritten == 5) break;
        }
        while (evosWritten < 5) {
            writeWord(evoOffset, 0);
            writeWord(evoOffset + 2, 0);
            writeWord(evoOffset + 4, 0);
            writeWord(evoOffset + 6, 0);
            evoOffset += 8;
            evosWritten++;
        }
    }
}

std::map<int, std::vector<MoveLearnt>> Gen3RomHandler::getMovesLearnt() const {
    std::map<int, std::vector<MoveLearnt>> movesets;
    int baseOffset = romEntry_->getInt("PokemonMovesets");
    for (int i = 1; i <= numRealPokemon_; i++) {
        const Species* pk = speciesList_[i];
        int pointerOffset = baseOffset + pokedexToInternal_[pk->number()] * 4;
        movesets[pk->number()] = readMovesLearnt(readPointer(pointerOffset));
    }
    return movesets;
}

std::vector<MoveLearnt> Gen3RomHandler::readMovesLearnt(int offset) const {
    std::vector<MoveLearnt> moves;
    if (jamboMovesetHack_) {
        while (rom_.at(offset) != 0x00 || rom_.at(offset + 1) != 0x00 || rom_.at(offset + 2) != 0xFF) {
            moves.push_back({readWord(offset), rom_[offset + 2]});
            offset += 3;
        }
    } else {
        while (rom_.at(offset) != 0xFF || rom_.at(offset + 1) != 0xFF) {
            int move = rom_[offset];
            int level = (rom_[offset + 1] & 0xFE) >> 1;
            if ((rom_[offset + 1] & 0x01) == 0x01) move += 0x100;
            moves.push_back({move, level});
            offset += 2;
        }
    }
    return moves;
}

std::vector<uint8_t> Gen3RomHandler::movesLearntToBytes(const std::vector<MoveLearnt>& ml) const {
    int entrySize = jamboMovesetHack_ ? 3 : 2;
    const auto& terminator = jamboMovesetHack_ ? jamboMovesLearntTerminator : vanillaMovesLearntTerminator;
    std::vector<uint8_t> bytes(entrySize * ml.size() + terminator.size());
    for (size_t i = 0; i < ml.size(); i++) {
        int o = static_cast<int>(i) * entrySize;
        if (jamboMovesetHack_) {
            bytes[o] = static_cast<uint8_t>(ml[i].move % 0x100);
            bytes[o + 1] = static_cast<uint8_t>((ml[i].move / 0x100) % 0x100);
            bytes[o + 2] = static_cast<uint8_t>(ml[i].level);
        } else {
            bytes[o] = static_cast<uint8_t>(ml[i].move & 0xFF);
            int levelPart = (ml[i].level << 1) & 0xFE;
            if (ml[i].move > 255) levelPart++;
            bytes[o + 1] = static_cast<uint8_t>(levelPart);
        }
    }
    std::copy(terminator.begin(), terminator.end(), bytes.end() - static_cast<long>(terminator.size()));
    return bytes;
}

void Gen3RomHandler::setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) {
    int baseOffset = romEntry_->getInt("PokemonMovesets");
    for (int i = 1; i <= numRealPokemon_; i++) {
        Species* pk = speciesList_[i];
        int pointerOffset = baseOffset + pokedexToInternal_[pk->number()] * 4;
        const auto& moves = movesets.at(pk->number());
        rewriteData(pointerOffset, movesLearntToBytes(moves),
                    [this](int off) { return static_cast<int>(movesLearntToBytes(readMovesLearnt(off)).size()); });
    }
}

std::map<int, std::vector<int>> Gen3RomHandler::getEggMoves() const {
    std::map<int, std::vector<int>> eggMoves;
    int offset = romEntry_->getInt("EggMoves");
    int currentSpecies = 0;
    std::vector<int> currentMoves;
    int val = readWord(offset);
    // ver egg_moves.h en las decomps de Gen 3
    while (val != 0xFFFF) {
        if (val > 20000) {
            if (!currentMoves.empty()) eggMoves[internalToPokedex_[currentSpecies]] = currentMoves;
            currentSpecies = val - 20000;
            currentMoves.clear();
        } else {
            currentMoves.push_back(val);
        }
        offset += 2;
        val = readWord(offset);
    }
    if (!currentMoves.empty()) eggMoves[internalToPokedex_[currentSpecies]] = currentMoves;
    return eggMoves;
}

void Gen3RomHandler::setEggMoves(const std::map<int, std::vector<int>>& eggMoves) {
    int offset = romEntry_->getInt("EggMoves");
    for (const auto& [species, moves] : eggMoves) {
        writeWord(offset, pokedexToInternal_[species] + 20000);
        offset += 2;
        for (int m : moves) {
            writeWord(offset, m);
            offset += 2;
        }
    }
}

}  // namespace pr::gen3
