// Movimientos de Gen 5: datos de los movimientos, movimientos por nivel y movimientos huevo
// (port de Gen5RomHandler.loadMoves/saveMoves, get/setMovesLearnt y get/setEggMoves).
#include <algorithm>
#include <cmath>

#include "constants/global_constants.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/gen5/gen5_rom_handler.hpp"
#include "romio/rom_functions.hpp"

namespace pr::gen5 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int readFullInt(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
int sbyte(const Bytes& d, int o) { return static_cast<int8_t>(d.at(o)); }
template <class L>
bool has(const L& list, int v) { return std::find(list.begin(), list.end(), v) != list.end(); }
}  // namespace

void Gen5RomHandler::loadMoves() {
    moveNarc_ = readNARC(romEntry_->getFile("MoveData"));
    moves_.assign(moveCount + 1, std::nullopt);
    auto moveNames = getStrings(false, romEntry_->getInt("MoveNamesTextOffset"));
    for (int i = 1; i <= moveCount; i++) {
        const Bytes& d = moveNarc_.files.at(i);
        moves_[i].emplace();
        Move& m = *moves_[i];
        m.name = moveNames.at(i);
        m.number = i;
        m.internalId = i;
        m.effectIndex = readWord(d, 16);
        m.hitratio = d.at(4);
        m.power = d.at(3);
        m.pp = d.at(5);
        m.type = typeFromByte(d.at(0));
        m.flinchPercentChance = d.at(15);
        m.target = d.at(20);
        m.category = moveCategoryFromByte(d.at(2));
        m.priority = sbyte(d, 6);
        int critStages = d.at(14);
        if (critStages == 6) m.criticalChance = CriticalChance::GUARANTEED;
        else if (critStages > 0) m.criticalChance = CriticalChance::INCREASED;

        int internalStatusType = readWord(d, 8);
        int flags = readFullInt(d, 32);
        m.makesContact = (flags & 0x001) != 0;
        m.isChargeMove = (flags & 0x002) != 0;
        m.isRechargeMove = (flags & 0x004) != 0;
        m.isPunchMove = (flags & 0x080) != 0;
        m.isSoundMove = (flags & 0x100) != 0;
        m.isTrapMove = m.effectIndex == trappingEffect || internalStatusType == 8;
        int qualities = sbyte(d, 1);
        int recoilOrAbsorbPercent = sbyte(d, 18);
        if (qualities == damageAbsorbQuality) m.absorbPercent = recoilOrAbsorbPercent;
        else m.recoilPercent = -recoilOrAbsorbPercent;

        if (i == moves::swift) perfectAccuracy_ = static_cast<int>(m.hitratio);
        if (has(global::normalMultihitMoves, i)) m.hitCount = 19 / 6.0;
        else if (has(global::doubleHitMoves, i)) m.hitCount = 2;
        else if (i == moves::tripleKick) m.hitCount = 2.71;  // suponiendo que el primer golpe acierta

        switch (qualities) {
            case noDamageStatChangeQuality:
            case noDamageStatusAndStatChangeQuality:
                // todos los aliados o uno mismo
                m.statChangeMoveType = (m.target == 6 || m.target == 7) ? StatChangeMoveType::NO_DAMAGE_USER
                                                                        : StatChangeMoveType::NO_DAMAGE_TARGET;
                break;
            case damageTargetDebuffQuality: m.statChangeMoveType = StatChangeMoveType::DAMAGE_TARGET; break;
            case damageUserBuffQuality: m.statChangeMoveType = StatChangeMoveType::DAMAGE_USER; break;
            default: m.statChangeMoveType = StatChangeMoveType::NONE_OR_UNKNOWN; break;
        }
        for (int sc = 0; sc < 3; sc++) {
            int t = sbyte(d, 21 + sc);
            if (t < 0 || t > static_cast<int>(StatChangeType::SPECIAL)) throw std::out_of_range("StatChangeType.values()");
            m.statChanges[sc].type = static_cast<StatChangeType>(t);
            m.statChanges[sc].stages = sbyte(d, 24 + sc);
            m.statChanges[sc].percentChance = sbyte(d, 27 + sc);
        }
        // solo los estados que existen en StatusType
        if (internalStatusType < 7) {
            m.statusType = static_cast<StatusType>(internalStatusType);
            if (m.statusType == StatusType::POISON && (i == moves::toxic || i == moves::poisonFang)) m.statusType = StatusType::TOXIC_POISON;
            m.statusPercentChance = d.at(10);
            if (m.number == moves::chatter) m.statusPercentChance = 1.0;
            switch (qualities) {
                case noDamageStatusQuality:
                case noDamageStatusAndStatChangeQuality: m.statusMoveType = StatusMoveType::NO_DAMAGE; break;
                case damageStatusQuality: m.statusMoveType = StatusMoveType::DAMAGE; break;
                default: break;
            }
        }
    }
}

void Gen5RomHandler::saveMoves() {
    int namesIdx = romEntry_->getInt("MoveNamesTextOffset");
    auto moveNames = getStrings(false, namesIdx);
    for (int i = 1; i <= moveCount; i++) {
        Bytes& data = moveNarc_.files.at(i);
        const Move& m = *moves_[i];
        moveNames.at(i) = m.name;
        data[2] = static_cast<uint8_t>(moveCategoryToByte(m.category));
        data[3] = static_cast<uint8_t>(m.power);
        data[0] = static_cast<uint8_t>(typeToByte(m.type));
        int hitratio = static_cast<int>(std::floor(m.hitratio + 0.5));  // Math.round
        if (hitratio < 0) hitratio = 0;
        if (hitratio > 101) hitratio = 100;  // (sic) en FVX
        data[4] = static_cast<uint8_t>(hitratio);
        data[5] = static_cast<uint8_t>(m.pp);
    }
    // mensajes de combate ya compuestos con el nombre del movimiento
    int battleIdx = romEntry_->getInt("BattleMoveNamesTextOffset");
    auto battleMsgs = getStrings(false, battleIdx);
    auto oldMoveNames = getStrings(false, namesIdx);
    for (int i = 1; i <= moveCount; i++) {
        const std::string& oldName = oldMoveNames.at(i);
        const std::string& newName = moves_[i]->name;
        int base = i * 3;
        for (int j = 0; j < 3; j++)
            if (base + j < static_cast<int>(battleMsgs.size())) battleMsgs[base + j] = romfn::replaceAll(battleMsgs[base + j], oldName, newName);
    }
    setStrings(false, namesIdx, moveNames);
    setStrings(false, battleIdx, battleMsgs);
    writeNARC(romEntry_->getFile("MoveData"), moveNarc_);
}

std::map<int, std::vector<MoveLearnt>> Gen5RomHandler::getMovesLearnt() const {
    std::map<int, std::vector<MoveLearnt>> movesets;
    auto narc = readNARC(romEntry_->getFile("PokemonMovesets"));
    int formeCount = getFormeCount(romEntry_->romType());
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const Bytes& data = narc.files.at(i > pokemonCount ? i + formeOffset : i);
        int loc = 0;
        std::vector<MoveLearnt> learnt;
        while (readWord(data, loc) != 0xFFFF || readWord(data, loc + 2) != 0xFFFF) {
            learnt.push_back({readWord(data, loc), readWord(data, loc + 2)});
            loc += 4;
        }
        movesets[pokes_[i]->number()] = learnt;
    }
    return movesets;
}

void Gen5RomHandler::setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) {
    auto narc = readNARC(romEntry_->getFile("PokemonMovesets"));
    int formeCount = getFormeCount(romEntry_->romType());
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        const auto& learnt = movesets.at(pokes_[i]->number());
        Bytes moveset(learnt.size() * 4 + 4, 0);
        size_t j = 0;
        for (; j < learnt.size(); j++) {
            writeWord(moveset, static_cast<int>(j * 4), learnt[j].move);
            writeWord(moveset, static_cast<int>(j * 4 + 2), learnt[j].level);
        }
        writeWord(moveset, static_cast<int>(j * 4), 0xFFFF);
        writeWord(moveset, static_cast<int>(j * 4 + 2), 0xFFFF);
        narc.files.at(i > pokemonCount ? i + formeOffset : i) = std::move(moveset);
    }
    writeNARC(romEntry_->getFile("PokemonMovesets"), narc);
}

std::map<int, std::vector<int>> Gen5RomHandler::getEggMoves() const {
    std::map<int, std::vector<int>> eggMoves;
    auto narc = readNARC(romEntry_->getFile("EggMoves"));
    for (int i = 1; i <= pokemonCount; i++) {
        const Bytes& data = narc.files.at(i);
        int n = readWord(data, 0);
        std::vector<int> moves;
        for (int j = 0; j < n; j++) moves.push_back(readWord(data, 2 + j * 2));
        eggMoves[pokes_[i]->number()] = moves;
    }
    return eggMoves;
}

void Gen5RomHandler::setEggMoves(const std::map<int, std::vector<int>>& eggMoves) {
    auto narc = readNARC(romEntry_->getFile("EggMoves"));
    for (int i = 1; i <= pokemonCount; i++) {
        Bytes& data = narc.files.at(i);
        const auto& moves = eggMoves.at(pokes_[i]->number());
        for (size_t j = 0; j < moves.size(); j++) writeWord(data, static_cast<int>(2 + j * 2), moves[j]);
    }
    writeNARC(romEntry_->getFile("EggMoves"), narc);
}

}  // namespace pr::gen5
