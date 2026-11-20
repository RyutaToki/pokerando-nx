// Entrenadores de Gen 4 (port de Gen4RomHandler.loadTrainerClasses/loadTrainers/saveTrainers...).
#include <algorithm>

#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "romio/gen4/gen4_trainer_tags_gen.hpp"
#include "romio/rom_functions.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
}  // namespace

void Gen4RomHandler::loadTrainerClasses() {
    trainerClasses_.clear();
    auto names = getStrings(romEntry_->getInt("TrainerClassesTextOffset"));
    for (size_t i = 0; i < names.size(); i++) trainerClasses_.push_back(std::make_unique<TrainerClass>(static_cast<int>(i), names[i]));
}

void Gen4RomHandler::saveTrainerClasses() {
    std::vector<std::string> names;
    for (const auto& tc : trainerClasses_) names.push_back(tc->name);
    setStrings(romEntry_->getInt("TrainerClassesTextOffset"), names);
}

std::vector<std::string> Gen4RomHandler::loadTrainerNames() const {
    auto t = getStrings(romEntry_->getInt("TrainerNamesTextOffset"));
    t.erase(t.begin());  // la primera está vacía
    for (auto& n : t)
        if (n.find("\\and") != std::string::npos) n = romfn::replaceAll(n, "\\and", "&");
    return t;
}

void Gen4RomHandler::loadTrainers() {
    trainers_.clear();
    auto trs = readNARC(romEntry_->getFile("TrainerData"));
    auto trpokes = readNARC(romEntry_->getFile("TrainerPokemon"));
    auto tnames = loadTrainerNames();
    int trainernum = static_cast<int>(trs.files.size());
    for (int i = 1; i < trainernum; i++) {
        // 20 bytes: banderas (1 = movimientos, 2 = objetos), clase, -, nº de Pokémon, objetos x4, IA, -, combate doble, -
        const Bytes& trainer = trs.files[i];
        const Bytes& trpoke = trpokes.files.at(i);
        auto tr = std::make_shared<Trainer>();
        bool readMovesets = (trainer.at(0) & 1) != 0;
        bool readItems = (trainer.at(0) & 2) != 0;
        tr->trainerClass = trainerClasses_.at(trainer.at(1)).get();
        tr->index = i;
        int numPokes = trainer.at(3);
        if (trainer.at(16) != 0) tr->currBattleStyle->style = BattleStyle::Style::DOUBLE_BATTLE;
        int pokeOffs = 0;
        tr->name = tnames.at(i - 1);
        for (int poke = 0; poke < numPokes; poke++) {
            // IV SB LV LV SP SP FRM FRM (HI HI) (M1 M1 M2 M2 M3 M3 M4 M4); SB = 0 0 Ab Ab 0 0 G G
            int difficulty = trpoke.at(pokeOffs);
            int level = trpoke.at(pokeOffs + 2);
            int sp = trpoke.at(pokeOffs + 4) + ((trpoke.at(pokeOffs + 5) & 0x01) << 8);
            int formnum = static_cast<int8_t>(trpoke.at(pokeOffs + 5)) >> 2;
            auto tpk = std::make_shared<TrainerPokemon>(pokes_.at(sp), level);
            tpk->IVs = (difficulty * 31) / 255;
            int abilitySlot = (trpoke.at(pokeOffs + 1) >> 4) & 0xF;
            if (abilitySlot == 0) abilitySlot = 1;  // la primera habilidad es la 0 en Gen 4
            tpk->abilitySlot = abilitySlot;
            tpk->holder.setAltFormeAllowed();
            tpk->holder.setFormeNumber(formnum);
            pokeOffs += 6;
            if (readItems) {
                int id = readWord(trpoke, pokeOffs);
                tpk->heldItem = (id < static_cast<int>(items_.size()) && items_[id]) ? std::optional<int>(id) : std::nullopt;
                pokeOffs += 2;
            }
            if (readMovesets) {
                for (int m = 0; m < 4; m++) tpk->moves[m] = readWord(trpoke, pokeOffs + m * 2);
                pokeOffs += 8;
            }
            if (romEntry_->romType() != Type_DP) pokeOffs += 2;
            tr->pokemon.push_back(tpk);
        }
        trainers_.push_back(tr);
    }
    const std::vector<std::pair<int, std::string_view>>* tags;
    const std::vector<std::pair<int, int>>* multi;
    switch (romEntry_->romType()) {
        case Type_DP: tags = &trtags::trainerTagsDP; multi = &trtags::multiBattleDP; break;
        case Type_Plat: tags = &trtags::trainerTagsPt; multi = &trtags::multiBattlePt; break;
        default: tags = &trtags::trainerTagsHGSS; multi = &trtags::multiBattleHGSS; break;
    }
    for (const auto& [idx, tag] : *tags) trainers_.at(idx)->tag = std::string(tag);
    for (const auto& [idx, st] : *multi)
        if (idx < static_cast<int>(trainers_.size())) trainers_[idx]->multiBattleStatus = static_cast<Trainer::MultiBattleStatus>(st);
}

void Gen4RomHandler::fixAbilitySlotValuesForHGSS() {
    // HGSS: si un Pokémon usa la 2.ª habilidad, todos los que van detrás también; se iguala al último
    for (auto& tr : trainers_) {
        if (tr->pokemon.empty()) continue;
        int last = tr->pokemon.back()->abilitySlot;
        for (auto& tp : tr->pokemon) tp->abilitySlot = last;
    }
}

void Gen4RomHandler::saveTrainers() {
    if (romEntry_->romType() == Type_HGSS) fixAbilitySlotValuesForHGSS();
    auto trs = readNARC(romEntry_->getFile("TrainerData"));
    nds::NARCArchive trpokes;
    auto movesets = getMovesLearnt();
    trpokes.files.push_back(Bytes(8, 0));
    int trainernum = static_cast<int>(trs.files.size());
    auto it = trainers_.begin();
    for (int i = 1; i < trainernum; i++) {
        Bytes& trainer = trs.files[i];
        const Trainer& tr = **it++;
        trainer.at(0) = static_cast<uint8_t>(tr.poketype());
        int numPokes = static_cast<int>(tr.pokemon.size());
        trainer.at(3) = static_cast<uint8_t>(numPokes);
        if (tr.forcedDoubleBattle) {
            // marcar a los compañeros (p. ej. Cheryl) convierte los combates dobles salvajes en combates raros
            bool excluded = romEntry_->romType() != Type_HGSS &&
                            std::find(partnerTrainerIndices.begin(), partnerTrainerIndices.end(), tr.index) != partnerTrainerIndices.end();
            if (!excluded) {
                if (tr.currBattleStyle->style == BattleStyle::Style::DOUBLE_BATTLE) {
                    if (trainer.at(16) == 0) trainer[16] |= 3;
                } else {
                    if (trainer.at(16) == 3) trainer[16] = 0;
                }
            }
        }
        int bytesNeeded = 6 * numPokes;
        if (romEntry_->romType() != Type_DP) bytesNeeded += 2 * numPokes;
        if (tr.pokemonHaveCustomMoves()) bytesNeeded += 8 * numPokes;
        if (tr.pokemonHaveItems()) bytesNeeded += 2 * numPokes;
        Bytes trpoke(bytesNeeded, 0);
        int pokeOffs = 0;
        for (const auto& tpp : tr.pokemon) {
            const TrainerPokemon& tp = *tpp;
            int ability = tp.abilitySlot == 1 ? 0 : tp.abilitySlot << 4;
            int difficulty = std::min(255, 1 + (tp.IVs * 255) / 31);
            writeWord(trpoke, pokeOffs, difficulty | ability << 8);
            writeWord(trpoke, pokeOffs + 2, tp.level);
            writeWord(trpoke, pokeOffs + 4, tp.species()->baseNumber());
            trpoke[pokeOffs + 5] |= static_cast<uint8_t>(tp.holder.formeNumber() << 2);
            pokeOffs += 6;
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
            if (romEntry_->romType() != Type_DP) pokeOffs += 2;
        }
        trpokes.files.push_back(trpoke);
    }
    writeNARC(romEntry_->getFile("TrainerData"), trs);
    writeNARC(romEntry_->getFile("TrainerPokemon"), trpokes);
    saveTrainerNames();
}

void Gen4RomHandler::saveTrainerNames() {
    int idx = romEntry_->getInt("TrainerNamesTextOffset");
    auto oldNames = getStrings(idx);
    std::vector<std::string> names;
    names.push_back(oldNames.at(0));  // se conserva la entrada 0
    for (const auto& tr : trainers_) {
        std::string n = tr->name.value_or("null");
        if (n.find('&') != std::string::npos) n = romfn::replaceAll(n, "&", "\\and");
        names.push_back(n);
    }
    setStrings(idx, names, lastStringsCompressed_);
}

std::vector<int> Gen4RomHandler::getEliteFourTrainers(bool) const { return romEntry_->getArray("EliteFourIndices"); }

std::vector<std::pair<std::string, Type>> Gen4RomHandler::getGymAndEliteTypeThemes() const {
    const auto& t = romEntry_->romType() == Type_DP     ? trtags::gymAndEliteThemesDP
                    : romEntry_->romType() == Type_Plat ? trtags::gymAndEliteThemesPt
                                                        : trtags::gymAndEliteThemesHGSS;
    std::vector<std::pair<std::string, Type>> out;
    for (const auto& [k, v] : t) out.emplace_back(std::string(k), v);
    return out;
}

}  // namespace pr::gen4
