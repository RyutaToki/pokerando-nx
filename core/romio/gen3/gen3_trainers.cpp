// Entrenadores de Gen 3 (port de Gen3RomHandler.loadTrainerClasses/loadTrainers/saveTrainers...).
#include <algorithm>

#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"
#include "romio/gen3/gen3_trainer_tags.hpp"

namespace pr::gen3 {

void Gen3RomHandler::loadTrainerClasses() {
    trainerClasses_.clear();
    int base = romEntry_->getInt("TrainerClassNames");
    int amount = romEntry_->getInt("TrainerClassCount");
    int length = romEntry_->getInt("TrainerClassNameLength");
    for (int i = 0; i < amount; i++) {
        trainerClasses_.push_back(std::make_unique<TrainerClass>(i, readVariableLengthString(base + i * length)));
    }
}

void Gen3RomHandler::saveTrainerClasses() {
    int base = romEntry_->getInt("TrainerClassNames");
    int length = romEntry_->getInt("TrainerClassNameLength");
    for (size_t i = 0; i < trainerClasses_.size(); i++) {
        writeFixedLengthString(trainerClasses_[i]->name, base + static_cast<int>(i) * length, length);
    }
}

std::optional<int> Gen3RomHandler::itemAt(int internalId) const {
    int id = itemIDToStandard(internalId);
    const auto& it = items_.at(id);
    return it ? std::optional<int>(it->id) : std::nullopt;
}

void Gen3RomHandler::loadTrainers() {
    trainers_.clear();
    int base = romEntry_->getInt("TrainerData");
    int amount = romEntry_->getInt("TrainerCount");
    int entryLen = romEntry_->getInt("TrainerEntrySize");
    for (int i = 1; i < amount; i++) {
        // Estructura de 40 bytes: ver include/battle.h en pokefirered
        int trOffset = base + i * entryLen;
        auto tr = std::make_shared<Trainer>();
        tr->offset = trOffset;
        tr->index = i;
        tr->trainerClass = trainerClasses_.at(rom_.at(trOffset + 1)).get();
        int pokeDataType = rom_[trOffset];
        if (rom_.at(trOffset + (entryLen - 16)) == 0x01) tr->currBattleStyle->style = BattleStyle::Style::DOUBLE_BATTLE;
        int numPokes = rom_.at(trOffset + (entryLen - 8));
        int pointerToPokes = readPointer(trOffset + (entryLen - 4));
        tr->name = readVariableLengthString(trOffset + 4);

        // IV IV LV SP SP (HI HI) (M1 M1 M2 M2 M3 M3 M4 M4); IV es 0-255 para 0-31
        bool customMoves = pokeDataType == 1 || pokeDataType == 3;
        bool hasItems = pokeDataType == 2 || pokeDataType == 3;
        if (pokeDataType >= 0 && pokeDataType <= 3) {
            int size = customMoves ? 16 : 8;
            for (int poke = 0; poke < numPokes; poke++) {
                int o = pointerToPokes + poke * size;
                auto tp = std::make_shared<TrainerPokemon>(pokesInternal_.at(readWord(o + 4)), readWord(o + 2));
                tp->IVs = ((readWord(o) & 0xFF) * 31) / 255;
                if (hasItems) tp->heldItem = itemAt(readWord(o + 6));
                if (customMoves) {
                    int movesStart = hasItems ? 8 : 6;
                    for (int m = 0; m < 4; m++) tp->moves[m] = readWord(o + movesStart + m * 2);
                }
                tp->abilitySlot = 1;  // en Gen 3 los Pokémon de entrenador usan siempre la primera habilidad
                tr->pokemon.push_back(tp);
            }
        }
        trainers_.push_back(tr);
    }
    if (romEntry_->romType() == RomType_Em) readMossdeepStevenTrainer();
    if (romEntry_->romType() == RomType_Ruby || romEntry_->romType() == RomType_Sapp) {
        trainerTagsRS(trainers_, romEntry_->romType());
    } else if (romEntry_->romType() == RomType_Em) {
        trainerTagsE(trainers_);
        setMultiBattleStatusEm(trainers_);
    } else {
        trainerTagsFRLG(trainers_);
    }
}

// Steven en Algaria no es un entrenador para el juego, solo datos de Pokémon; se añade al final.
void Gen3RomHandler::readMossdeepStevenTrainer() {
    int off = romEntry_->getInt("MossdeepStevenTeamOffset");
    auto steven = std::make_shared<Trainer>();
    steven->offset = off;
    steven->index = static_cast<int>(trainers_.size()) + 1;
    const auto& meteorFalls = trainers_.at(emMeteorFallsStevenIndex - 1);
    steven->trainerClass = meteorFalls->trainerClass;
    steven->name = meteorFalls->name;
    for (int i = 0; i < 3; i++) {
        int o = off + i * 20;
        auto tp = std::make_shared<TrainerPokemon>(pokesInternal_.at(readWord(o)), static_cast<int8_t>(rom_.at(o + 3)));
        tp->IVs = static_cast<int8_t>(rom_.at(o + 2));
        for (int m = 0; m < 4; m++) tp->moves[m] = readWord(o + 12 + m * 2);
        tp->abilitySlot = 1;
        steven->pokemon.push_back(tp);
    }
    trainers_.push_back(steven);
}

std::vector<uint8_t> Gen3RomHandler::trainerPokemonToBytes(const Trainer& t) const {
    bool custom = t.pokemonHaveCustomMoves();
    bool items = t.pokemonHaveItems();
    std::vector<uint8_t> data(t.pokemon.size() * (custom ? 16 : 8));
    auto w = [&](int o, int v) {
        data[o] = static_cast<uint8_t>(v % 0x100);
        data[o + 1] = static_cast<uint8_t>((v / 0x100) % 0x100);
    };
    auto movesets = getMovesLearnt();  // por si hay que reiniciar movimientos
    for (size_t i = 0; i < t.pokemon.size(); i++) {
        const TrainerPokemon& tp = *t.pokemon[i];
        int b = static_cast<int>(i) * (custom ? 16 : 8);
        // se suma 1 para compensar el truncado de la división
        w(b, std::min(255, 1 + (tp.IVs * 255) / 31));
        w(b + 2, tp.level);
        w(b + 4, pokedexToInternal_[tp.species()->number()]);
        if (custom) {
            int movesStart;
            if (items) {
                w(b + 6, tp.heldItem ? itemIDToInternal(*tp.heldItem) : 0);
                movesStart = 8;
            } else {
                movesStart = 6;
                w(b + 14, 0);
            }
            if (tp.resetMoves) {
                auto pm = getMovesAtLevel(*tp.species(), movesets, tp.level);
                for (int m = 0; m < 4; m++) w(b + movesStart + m * 2, pm[m]);
            } else {
                for (int m = 0; m < 4; m++) w(b + movesStart + m * 2, tp.moves[m]);
            }
        } else {
            w(b + 6, !items || !tp.heldItem ? 0 : itemIDToInternal(*tp.heldItem));
        }
    }
    return data;
}

int Gen3RomHandler::readTrainerPokemonDataLength(int trainerOffset) const {
    int entryLen = romEntry_->getInt("TrainerEntrySize");
    int pokeType = rom_.at(trainerOffset);
    int pokeCount = rom_.at(trainerOffset + (entryLen - 8));
    return pokeCount * ((pokeType & 1) == 1 ? 16 : 8);
}

void Gen3RomHandler::saveTrainers() {
    int base = romEntry_->getInt("TrainerData");
    int amount = romEntry_->getInt("TrainerCount");
    int entryLen = romEntry_->getInt("TrainerEntrySize");
    int nameLen = romEntry_->getInt("TrainerNameLength");
    auto it = trainers_.begin();
    for (int i = 1; i < amount; i++) {
        int trOffset = base + i * entryLen;
        const Trainer& tr = **it++;
        // primero se reescriben los Pokémon (usa los datos viejos del entrenador para medir)
        rewriteData(trOffset + (entryLen - 4), trainerPokemonToBytes(tr),
                    [this, trOffset](int) { return readTrainerPokemonDataLength(trOffset); });
        writeByte(trOffset, static_cast<uint8_t>(tr.poketype()));
        writeFixedLengthString(tr.name.value_or(""), trOffset + 4, nameLen);
        writeByte(trOffset + (entryLen - 8), static_cast<uint8_t>(tr.pokemon.size()));
        if (tr.forcedDoubleBattle) {
            writeByte(trOffset + (entryLen - 16), tr.currBattleStyle->style == BattleStyle::Style::DOUBLE_BATTLE ? 0x01 : 0x00);
        }
    }
    if (romEntry_->romType() == RomType_Em) writeMossdeepStevenTrainer();
}

void Gen3RomHandler::writeMossdeepStevenTrainer() {
    int off = romEntry_->getInt("MossdeepStevenTeamOffset");
    const Trainer& steven = *trainers_.back();
    for (int i = 0; i < 3; i++) {
        int o = off + i * 20;
        const TrainerPokemon& tp = *steven.pokemon.at(i);
        writeWord(o, pokedexToInternal_[tp.species()->number()]);
        writeByte(o + 2, static_cast<uint8_t>(tp.IVs));
        writeByte(o + 3, static_cast<uint8_t>(tp.level));
        for (int m = 0; m < 4; m++) writeWord(o + 12 + m * 2, tp.moves[m]);
    }
}

std::vector<int> Gen3RomHandler::getEliteFourTrainers(bool /*isChallengeMode*/) const {
    return romEntry_->getArray("EliteFourIndices");
}

std::vector<std::pair<std::string, Type>> Gen3RomHandler::getGymAndEliteTypeThemes() const {
    using T = Type;
    switch (romEntry_->romType()) {
        case RomType_FRLG:
            return {{"ELITE1", T::ICE}, {"ELITE2", T::FIGHTING}, {"ELITE3", T::GHOST}, {"ELITE4", T::DRAGON},
                    {"GYM1", T::ROCK}, {"GYM2", T::WATER}, {"GYM3", T::ELECTRIC}, {"GYM4", T::GRASS},
                    {"GYM5", T::POISON}, {"GYM6", T::PSYCHIC}, {"GYM7", T::FIRE}, {"GYM8", T::GROUND}};
        case RomType_Em:
            return {{"CHAMPION", T::WATER}, {"ELITE1", T::DARK}, {"ELITE2", T::GHOST}, {"ELITE3", T::ICE},
                    {"ELITE4", T::DRAGON}, {"GYM1", T::ROCK}, {"GYM2", T::FIGHTING}, {"GYM3", T::ELECTRIC},
                    {"GYM4", T::FIRE}, {"GYM5", T::NORMAL}, {"GYM6", T::FLYING}, {"GYM7", T::PSYCHIC},
                    {"GYM8", T::WATER}};
        default:
            return {{"CHAMPION", T::STEEL}, {"ELITE1", T::DARK}, {"ELITE2", T::GHOST}, {"ELITE3", T::ICE},
                    {"ELITE4", T::DRAGON}, {"GYM1", T::ROCK}, {"GYM2", T::FIGHTING}, {"GYM3", T::ELECTRIC},
                    {"GYM4", T::FIRE}, {"GYM5", T::NORMAL}, {"GYM6", T::FLYING}, {"GYM7", T::PSYCHIC},
                    {"GYM8", T::WATER}};
    }
}

}  // namespace pr::gen3
