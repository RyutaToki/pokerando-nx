#include "random/updaters.hpp"

#include <cmath>
#include <stdexcept>

#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"

namespace pr {

void TypeEffectivenessUpdater::updateTypeEffectiveness() {
    TypeTable tt = romHandler.getTypeTable();
    auto upd = [&](Type a, Type d, Effectiveness e) {
        Effectiveness before = tt.getEffectiveness(a, d);
        tt.setEffectiveness(a, d, e);
        updates[a][d] = {before, e};
    };
    if (romHandler.generationOfPokemon() == 1) {
        upd(Type::POISON, Type::BUG, Effectiveness::NEUTRAL);
        upd(Type::BUG, Type::POISON, Effectiveness::HALF);
        upd(Type::GHOST, Type::PSYCHIC, Effectiveness::DOUBLE);
        upd(Type::ICE, Type::FIRE, Effectiveness::HALF);
    } else {
        upd(Type::GHOST, Type::STEEL, Effectiveness::NEUTRAL);
        upd(Type::DARK, Type::STEEL, Effectiveness::NEUTRAL);
    }
    romHandler.setTypeTable(tt);
}

Move& MoveUpdater::mv(int n) {
    auto& moves = const_cast<std::vector<std::optional<Move>>&>(romHandler.getMoves());
    return *moves.at(n);
}

void MoveUpdater::updateMoves(int toGen) {
    if (toGen > 9) throw std::invalid_argument("updateToGen too high");
    void (MoveUpdater::*fns[])() = {&MoveUpdater::gen2Updates, &MoveUpdater::gen3Updates, &MoveUpdater::gen4Updates,
                                    &MoveUpdater::gen5Updates, &MoveUpdater::gen6Updates, &MoveUpdater::gen7Updates,
                                    &MoveUpdater::gen8Updates, &MoveUpdater::gen9Updates};
    for (int i = 2; i <= 9; i++)
        if (toGen >= i && gen() < i) (this->*fns[i - 2])();
}

void MoveUpdater::updateMovePower(int n, int power) {
    Move& m = mv(n);
    if (m.power != power) {
        updates[m.number][UpdateType::POWER] = {double(m.power), double(power)};
        m.power = power;
    }
}
void MoveUpdater::updateMovePP(int n, int pp) {
    Move& m = mv(n);
    if (m.pp != pp) {
        updates[m.number][UpdateType::PP] = {double(m.pp), double(pp)};
        m.pp = pp;
    }
}
void MoveUpdater::updateMoveAccuracy(int n, double acc) {
    Move& m = mv(n);
    if (std::abs(m.hitratio - acc) >= 1) {
        updates[m.number][UpdateType::ACCURACY] = {m.hitratio, acc};
        m.hitratio = acc;
    }
}
void MoveUpdater::updateMoveType(int n, Type t) {
    Move& m = mv(n);
    if (m.type != t) {
        updates[m.number][UpdateType::TYPE] = {m.type ? double(int(*m.type)) : -1.0, double(int(t))};
        m.type = t;
    }
}
void MoveUpdater::updateMoveCategory(int n, MoveCategory c) {
    Move& m = mv(n);
    if (m.category != c) {
        updates[m.number][UpdateType::CATEGORY] = {double(int(m.category)), double(int(c))};
        m.category = c;
    }
}

void SpeciesBaseStatUpdater::updateSpeciesStats(int toGen) {
    if (toGen > 9) throw std::invalid_argument("updateToGen too high");
    void (SpeciesBaseStatUpdater::*fns[])() = {&SpeciesBaseStatUpdater::gen6Updates, &SpeciesBaseStatUpdater::gen7Updates,
                                               &SpeciesBaseStatUpdater::gen8Updates, &SpeciesBaseStatUpdater::gen9Updates};
    for (int i = 6; i <= 9; i++)
        if (toGen >= i && gen() < i) (this->*fns[i - 6])();
}

void SpeciesBaseStatUpdater::set(int species, int stat, int value, UpdateType t) {
    Species* sp = romHandler.getSpeciesInclFormes().at(species);
    const BaseStats& bs = sp->baseStats();
    int v[6] = {bs.getHp(), bs.attack, bs.defense, bs.spatk, bs.spdef, bs.speed};
    int before = v[stat];
    v[stat] = value;
    sp->setBaseStats(BaseStats(v[0], v[1], v[2], v[3], v[4], v[5]));  // Java crea un BaseStats nuevo (no Shedinja)
    updates[sp->number()][t] = {before, value};
}

#include "random/updaters_gen.inc"

}  // namespace pr
