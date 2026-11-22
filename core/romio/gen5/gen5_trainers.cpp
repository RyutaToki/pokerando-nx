// Entrenadores de Gen 5 (port de Gen5RomHandler.loadTrainerClasses/loadTrainers/saveTrainers,
// saveTrainerNames, makeDoubleBattleModePossible y temas de gimnasio).
#include <algorithm>
#include <stdexcept>

#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"
#include "romio/gen5/gen5_trainer_tags_gen.hpp"
#include "romio/rom_functions.hpp"
#include "util/java_compat.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}

// getMugshotTrainerMap: (cadena del retrato, nombre de entrenador). Map.ofEntries: el orden no importa
const std::vector<std::pair<int, int>>& mugshotTrainerMap(int romType) {
    static const std::vector<std::pair<int, int>> bw = {{1, 0x35},  {2, 0x3B},  {3, 0x0B},  {4, 0x0D},  {5, 0x0C},
                                                        {6, 0x15},  {7, 0x16},  {8, 0x17},  {9, 0x18},  {10, 0x19},
                                                        {11, 0x83}, {12, 0x84}, {13, 0x85}, {14, 0xE4}, {15, 0xE6},
                                                        {16, 0xE5}, {17, 0xE7}, {18, 0x197}, {19, 0x24A}, {20, 0xE8}};
    static const std::vector<std::pair<int, int>> bw2 = {{1, 0x9C},  {2, 0x9D},  {3, 0x9A},   {4, 0x99},  {5, 0x9E},  {6, 0x9B},
                                                         {7, 0x9F},  {8, 0xA0},  {9, 0x26},   {10, 0x28}, {11, 0x27}, {12, 0x29},
                                                         {13, 0x155}, {15, 0x5}, {16, 0x166}, {17, 0x159}, {18, 0xCA}};
    if (romType == Type_BW) return bw;
    if (romType == Type_BW2) return bw2;
    throw std::invalid_argument("invalid ROM Type: " + std::to_string(romType));
}

std::optional<int> itemOrNull(const std::vector<std::optional<Item>>& items, int id) {
    if (!items.at(id)) return std::nullopt;  // items.get(id) lanza fuera de rango
    return id;
}
}  // namespace

void Gen5RomHandler::loadTrainerClasses() {
    trainerClasses_.clear();
    auto names = getStrings(false, romEntry_->getInt("TrainerClassesTextOffset"));
    if (romEntry_->romType() == Type_BW2) {
        auto pwt = getStrings(false, romEntry_->getInt("PWTTrainerClassesTextOffset"));
        names.insert(names.end(), pwt.begin(), pwt.end());
    }
    for (size_t i = 0; i < names.size(); i++) trainerClasses_.push_back(std::make_unique<TrainerClass>(static_cast<int>(i), names[i]));
}

void Gen5RomHandler::saveTrainerClasses() {
    std::vector<std::string> names;
    for (const auto& tc : trainerClasses_) names.push_back(tc->name);
    int idx = romEntry_->getInt("TrainerClassesTextOffset");
    if (romEntry_->romType() == Type_BW2) {
        size_t normal = getStrings(false, idx).size();
        setStrings(false, idx, std::vector<std::string>(names.begin(), names.begin() + normal));
        setStrings(false, romEntry_->getInt("PWTTrainerClassesTextOffset"), std::vector<std::string>(names.begin() + normal, names.end()));
    } else {
        setStrings(false, idx, names);
    }
}

std::vector<std::string> Gen5RomHandler::loadTrainerNames() const {
    auto t = getStrings(false, romEntry_->getInt("TrainerNamesTextOffset"));
    t.erase(t.begin());  // la primera está vacía
    if (romEntry_->romType() == Type_BW2) {
        auto pwt = getStrings(false, romEntry_->getInt("PWTTrainerNamesTextOffset"));
        t.insert(t.end(), pwt.begin(), pwt.end());
    }
    return t;
}

void Gen5RomHandler::loadTrainers() {
    trainers_.clear();
    auto trs = readNARC(romEntry_->getFile("TrainerData"));
    auto trpokes = readNARC(romEntry_->getFile("TrainerPokemon"));
    int trainernum = static_cast<int>(trs.files.size());
    auto tnames = loadTrainerNames();
    for (int i = 1; i < trainernum; i++) {
        // 20 bytes: banderas (1 = movimientos, 2 = objetos), clase, modo de combate, nº de Pokémon, objetos x4, IA, ...
        const Bytes& trainer = trs.files[i];
        const Bytes& trpoke = trpokes.files.at(i);
        auto tr = std::make_shared<Trainer>();
        tr->index = i;
        bool readMovesets = (trainer.at(0) & 1) != 0;
        bool readItems = (trainer.at(0) & 2) != 0;
        tr->trainerClass = trainerClasses_.at(trainer.at(1)).get();
        int numPokes = trainer.at(3);
        int pokeOffs = 0;
        tr->name = tnames.at(i - 1);
        switch (trainer.at(2)) {
            case 0: tr->currBattleStyle->style = BattleStyle::Style::SINGLE_BATTLE; break;
            case 1:
                tr->currBattleStyle->style = BattleStyle::Style::DOUBLE_BATTLE;
                originalDoubleTrainers_.push_back(i);
                break;
            case 2: tr->currBattleStyle->style = BattleStyle::Style::TRIPLE_BATTLE; break;
            case 3: tr->currBattleStyle->style = BattleStyle::Style::ROTATION_BATTLE; break;
            default: break;
        }
        for (int poke = 0; poke < numPokes; poke++) {
            // IV SB LV LV SP SP FRM FRM (HI HI) (M1 M1 M2 M2 M3 M3 M4 M4); SB = 0 0 Ab Ab 0 0 Fm Ml
            int difficulty = trpoke.at(pokeOffs);
            int level = readWord(trpoke, pokeOffs + 2);
            int sp = readWord(trpoke, pokeOffs + 4);
            int formnum = readWord(trpoke, pokeOffs + 6);
            auto tpk = std::make_shared<TrainerPokemon>(pokes_.at(sp), level);
            tpk->IVs = difficulty * 31 / 255;
            int abilityAndFlag = trpoke.at(pokeOffs + 1);
            tpk->abilitySlot = (abilityAndFlag >> 4) & 0xF;
            tpk->forcedGenderFlag = abilityAndFlag & 0xF;
            tpk->holder.setAltFormeAllowed();
            tpk->holder.setFormeNumber(formnum);
            pokeOffs += 8;
            if (readItems) {
                tpk->heldItem = itemOrNull(items_, readWord(trpoke, pokeOffs));
                pokeOffs += 2;
            }
            if (readMovesets) {
                for (int m = 0; m < 4; m++) tpk->moves[m] = readWord(trpoke, pokeOffs + m * 2);
                pokeOffs += 8;
            }
            tr->pokemon.push_back(tpk);
        }
        trainers_.push_back(tr);
    }
    const std::vector<std::pair<int, std::string_view>>* tags;
    const std::vector<std::pair<int, int>>* multi;
    const std::vector<std::pair<int, int>>* starter;
    if (romEntry_->romType() == Type_BW) {
        tags = &trtags::trainerTagsBW;
        multi = &trtags::multiBattleBW;
        starter = &trtags::forcedStarterBW;
    } else {
        if (!romEntry_->getFile("DriftveilPokemon").empty()) {
            // los entrenadores del Torneo Mundial Pokémon (PWT) de Driftveil
            auto driftveil = readNARC(romEntry_->getFile("DriftveilPokemon"));
            int currentFile = 1;
            for (int trno = 0; trno < 17; trno++) {
                auto tr = std::make_shared<Trainer>();
                tr->index = static_cast<int>(trainers_.size()) + 1;
                int nameAndClassIndex = bw2DriftveilTrainerOffsets.at(trno);
                tr->trainerClass = trainerClasses_.at(normalTrainerClassLength + nameAndClassIndex).get();
                tr->name = tnames.at(normalTrainerNameLength + nameAndClassIndex);
                tr->requiresUniqueHeldItems = true;
                int pokemonNum = trno < 2 ? 3 : 6;
                for (int poke = 0; poke < pokemonNum; poke++) {
                    const Bytes& d = driftveil.files.at(currentFile);
                    auto tpk = std::make_shared<TrainerPokemon>(pokes_.at(readWord(d, 0)), 25);
                    tpk->IVs = 31;
                    tpk->heldItem = itemOrNull(items_, readWord(d, 12));
                    for (int m = 0; m < 4; m++) tpk->moves[m] = readWord(d, 2 + m * 2);
                    tr->pokemon.push_back(tpk);
                    currentFile++;
                }
                trainers_.push_back(tr);
            }
        }
        bool isBlack2 = jc::startsWith(romEntry_->romCode(), "IRE");
        tags = isBlack2 ? &trtags::trainerTagsB2 : &trtags::trainerTagsW2;
        multi = isBlack2 ? &trtags::multiBattleB2 : &trtags::multiBattleW2;
        starter = &trtags::forcedStarterB2;
    }
    int n = static_cast<int>(trainers_.size());
    for (const auto& [idx, tag] : *tags)
        if (idx < n) trainers_[idx]->tag = std::string(tag);
    for (const auto& [idx, st] : *multi)
        if (idx < n) trainers_[idx]->multiBattleStatus = static_cast<Trainer::MultiBattleStatus>(st);
    for (const auto& [idx, pos] : *starter) trainers_.at(idx)->forceStarterPosition = pos;
}

void Gen5RomHandler::saveTrainers() {
    auto it = trainers_.begin();
    auto trs = readNARC(romEntry_->getFile("TrainerData"));
    nds::NARCArchive trpokes;
    auto movesets = getMovesLearnt();  // por si hay que reponer movimientos
    trpokes.files.push_back(Bytes(8, 0));
    int trainernum = static_cast<int>(trs.files.size());
    for (int i = 1; i < trainernum; i++) {
        Bytes& trainer = trs.files[i];
        const Trainer& tr = **it++;
        trainer.at(0) = static_cast<uint8_t>(tr.poketype());
        int numPokes = static_cast<int>(tr.pokemon.size());
        trainer.at(3) = static_cast<uint8_t>(numPokes);
        if (tr.forcedDoubleBattle) {
            switch (tr.currBattleStyle->style) {
                case BattleStyle::Style::SINGLE_BATTLE:
                    if (trainer.at(2) != 0) {
                        trainer[2] = 0;
                        trainer.at(12) &= 0x7F;  // IA de combate individual
                    }
                    break;
                case BattleStyle::Style::DOUBLE_BATTLE:
                    if (trainer.at(2) != 1) {
                        trainer[2] = 1;
                        trainer.at(12) |= 0x80;  // para que no ataquen a sus propios Pokémon
                    }
                    break;
                case BattleStyle::Style::TRIPLE_BATTLE:
                    if (trainer.at(2) != 2) {
                        trainer[2] = 2;
                        trainer.at(12) |= 0x80;
                    }
                    break;
                case BattleStyle::Style::ROTATION_BATTLE:
                    if (trainer.at(2) != 3) {
                        trainer[2] = 3;
                        trainer.at(12) |= 0x7F;  // (sic) en FVX
                    }
                    break;
            }
        }
        int bytesNeeded = 8 * numPokes;
        if (tr.pokemonHaveCustomMoves()) bytesNeeded += 8 * numPokes;
        if (tr.pokemonHaveItems()) bytesNeeded += 2 * numPokes;
        Bytes trpoke(bytesNeeded, 0);
        int pokeOffs = 0;
        for (const auto& tpp : tr.pokemon) {
            const TrainerPokemon& tp = *tpp;
            int difficulty = std::min(255, 1 + (tp.IVs * 255) / 31);  // +1 por el redondeo de la división
            int abilityAndFlag = static_cast<int8_t>((tp.abilitySlot << 4) | tp.forcedGenderFlag);
            writeWord(trpoke, pokeOffs, difficulty | abilityAndFlag << 8);
            writeWord(trpoke, pokeOffs + 2, tp.level);
            writeWord(trpoke, pokeOffs + 4, tp.species()->baseNumber());
            writeWord(trpoke, pokeOffs + 6, tp.holder.formeNumber());
            pokeOffs += 8;
            if (tr.pokemonHaveItems()) {
                writeWord(trpoke, pokeOffs, tp.heldItem.value_or(0));
                pokeOffs += 2;
            }
            if (tr.pokemonHaveCustomMoves()) {
                if (tp.resetMoves) {
                    auto pm = getMovesAtLevel(*tp.species(), movesets, tp.level);
                    for (int m = 0; m < 4; m++) writeWord(trpoke, pokeOffs + m * 2, pm[m]);
                } else {
                    for (int m = 0; m < 4; m++) writeWord(trpoke, pokeOffs + m * 2, tp.moves[m]);
                }
                pokeOffs += 8;
            }
        }
        trpokes.files.push_back(trpoke);
    }
    writeNARC(romEntry_->getFile("TrainerData"), trs);
    writeNARC(romEntry_->getFile("TrainerPokemon"), trpokes);

    // PWT
    if (romEntry_->romType() == Type_BW2 && !romEntry_->getFile("DriftveilPokemon").empty()) {
        auto driftveil = readNARC(romEntry_->getFile("DriftveilPokemon"));
        int currentFile = 1;
        for (int trno = 0; trno < 17; trno++) {
            const Trainer& tr = **it++;
            auto tpks = tr.pokemon.begin();
            int pokemonNum = trno < 2 ? 3 : 6;
            for (int poke = 0; poke < pokemonNum; poke++) {
                Bytes& d = driftveil.files.at(currentFile);
                if (tpks == tr.pokemon.end()) throw std::runtime_error("NoSuchElementException");
                const TrainerPokemon& tp = **tpks++;
                writeWord(d, 0, tp.species()->baseForme()->number());
                writeWord(d, 12, tp.heldItem.value_or(0));
                if (tp.resetMoves) {
                    auto pm = getMovesAtLevel(*tp.species()->baseForme(), movesets, tp.level);
                    for (int m = 0; m < 4; m++) writeWord(d, 2 + m * 2, pm[m]);
                } else {
                    for (int m = 0; m < 4; m++) writeWord(d, 2 + m * 2, tp.moves[m]);
                }
                currentFile++;
            }
        }
        writeNARC(romEntry_->getFile("DriftveilPokemon"), driftveil);
    }
    saveTrainerNames();
}

void Gen5RomHandler::saveTrainerNames() {
    int idx = romEntry_->getInt("TrainerNamesTextOffset");
    std::vector<std::string> tNames;
    auto oldTNames = getStrings(false, idx);
    tNames.push_back(oldTNames.at(0));  // se conserva la entrada 0
    for (const auto& tr : trainers_) tNames.push_back(tr->name.value_or("null"));
    if (romEntry_->romType() == Type_BW2) {
        size_t normal = getStrings(false, idx).size();
        setStrings(false, idx, std::vector<std::string>(tNames.begin(), tNames.begin() + normal));
        // solo algunos nombres del PWT se usan en entrenadores, y no son los primeros
        int pwtIdx = romEntry_->getInt("PWTTrainerNamesTextOffset");
        auto internalPWTNames = getStrings(false, pwtIdx);
        for (size_t i = 0; normal + i < tNames.size(); i++) internalPWTNames.at(bw2DriftveilTrainerOffsets.at(i)) = tNames[normal + i];
        setStrings(false, pwtIdx, internalPWTNames);
    } else {
        setStrings(false, idx, tNames);
    }
    // los nombres de los retratos van aparte
    int mugIdx = romEntry_->getInt("TrainerMugshotsTextOffset");
    auto mnames = getStrings(false, mugIdx);
    for (const auto& [k, v] : mugshotTrainerMap(romEntry_->romType())) mnames.at(k) = tNames.at(v);
    setStrings(false, mugIdx, mnames);
}

void Gen5RomHandler::makeDoubleBattleModePossible() {
    auto notSupported = [] {
        return std::runtime_error("javax.naming.OperationNotSupportedException: Double Battle Mode not supported for this game");
    };
    auto trainerTextBoxes = readNARC(romEntry_->getFile("TrainerTextBoxes"));
    Bytes& data = trainerTextBoxes.files.at(0);
    for (size_t i = 0; i < data.size(); i += 4) {
        int trainerIndex = readWord(data, static_cast<int>(i));
        if (std::find(originalDoubleTrainers_.begin(), originalDoubleTrainers_.end(), trainerIndex) != originalDoubleTrainers_.end()) {
            int textBoxIndex = readWord(data, static_cast<int>(i + 2));
            if (textBoxIndex == 3) writeWord(data, static_cast<int>(i + 2), 0);
            else if (textBoxIndex == 5) writeWord(data, static_cast<int>(i + 2), 2);
            else if (textBoxIndex == 6) writeWord(data, static_cast<int>(i + 2), 0x18);
        }
    }
    writeNARC(romEntry_->getFile("TrainerTextBoxes"), trainerTextBoxes);

    int fieldOvl = romEntry_->getInt("FieldOvlNumber");
    Bytes fieldOverlay = readOverlay(fieldOvl);
    std::string trainerOverworldTextBoxPrefix = romEntry_->getString("TrainerOverworldTextBoxPrefix");
    int prefixLen = static_cast<int>(trainerOverworldTextBoxPrefix.size()) / 2;
    int offset = find(fieldOverlay, trainerOverworldTextBoxPrefix);
    if (offset <= 0) throw notSupported();
    offset += prefixLen;
    // el primer entrenador de una pareja usa los mismos cuadros de texto que uno solo
    fieldOverlay.at(offset - 2) = 0;
    fieldOverlay.at(offset) = 2;
    fieldOverlay.at(offset + 2) = 0x18;

    offset = find(fieldOverlay, romEntry_->getString("DoubleBattleLimitPrefix"));
    if (offset <= 0) throw notSupported();
    offset += prefixLen;  // (sic) FVX usa la longitud del primer prefijo
    // sin límite: te ven aunque solo lleves un Pokémon
    writeWord(fieldOverlay, offset, 0x46C0);  // nop
    writeWord(fieldOverlay, offset + 2, 0x46C0);

    int beqToSingleTrainer = romEntry_->getInt("BeqToSingleTrainerNumber");
    offset = find(fieldOverlay, romEntry_->getString("DoubleBattleGetPointerPrefix"));
    if (offset <= 0) throw notSupported();
    offset += prefixLen;
    writeWord(fieldOverlay, offset + 0x10, readWord(fieldOverlay, offset + 0xE));
    writeWord(fieldOverlay, offset + 0xE, readWord(fieldOverlay, offset + 0xC));
    writeWord(fieldOverlay, offset + 0xC, readWord(fieldOverlay, offset + 0xA));
    writeWord(fieldOverlay, offset + 0xA, beqToSingleTrainer);
    writeWord(fieldOverlay, offset + 8, 0x2800);
    writeOverlay(fieldOvl, fieldOverlay);

    std::string textBoxChoicePrefix = romEntry_->getString("TextBoxChoicePrefix");
    offset = find(arm9_, textBoxChoicePrefix);
    if (offset <= 0) throw notSupported();
    // solo comprueba el entrenador que toca (evita que el juego se congele con parejas de combate doble)
    offset += static_cast<int>(textBoxChoicePrefix.size()) / 2;
    arm9_.at(offset - 4) = 2;
}

std::vector<int> Gen5RomHandler::getMainPlaythroughTrainers() const {
    return romEntry_->romType() == Type_BW ? bw1MainPlaythroughTrainers : bw2MainPlaythroughTrainers;
}

std::vector<int> Gen5RomHandler::getEliteFourTrainers(bool isChallengeMode) const {
    return romEntry_->getArray(isChallengeMode ? "ChallengeModeEliteFourIndices" : "EliteFourIndices");
}

std::vector<std::pair<std::string, Type>> Gen5RomHandler::getGymAndEliteTypeThemes() const {
    std::vector<std::pair<std::string, Type>> out;
    if (romEntry_->romType() == Type_BW) {
        for (const auto& [k, v] : trtags::gymAndEliteThemesBW) out.emplace_back(std::string(k), v);
        if (isTypeTriangleChanged()) {
            // put sobre claves ya existentes: el orden del HashMap no cambia
            auto triangle = getStarterTypeTriangle();
            for (auto& [k, v] : out) {
                if (k == "GYM1") v = triangle.at(0);
                else if (k == "GYM9") v = triangle.at(1);
                else if (k == "GYM10") v = triangle.at(2);
            }
        }
    } else {
        for (const auto& [k, v] : trtags::gymAndEliteThemesBW2) out.emplace_back(std::string(k), v);
    }
    return out;
}

}  // namespace pr::gen5
