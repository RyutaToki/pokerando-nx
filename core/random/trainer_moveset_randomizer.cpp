#include "random/trainer_moveset_randomizer.hpp"

#include <algorithm>

#include "constants/ability_ids.hpp"
#include "constants/global_constants.hpp"
#include "gamedata/move_synergy.hpp"

namespace pr {

namespace {
using MoveList = std::vector<Move*>;
bool has(const std::vector<int>& v, int x) { return std::find(v.begin(), v.end(), x) != v.end(); }
bool has(const MoveList& v, const Move* x) { return std::find(v.begin(), v.end(), x) != v.end(); }
MoveList distinct(const MoveList& v) {
    MoveList r;
    for (Move* m : v)
        if (!has(r, m)) r.push_back(m);
    return r;
}
template <class F>
MoveList filter(const MoveList& v, F f) {
    MoveList r;
    for (Move* m : v)
        if (f(m)) r.push_back(m);
    return r;
}
void removeAllOf(MoveList& v, const MoveList& r) {  // List.removeAll: quita todas las apariciones
    v.erase(std::remove_if(v.begin(), v.end(), [&](Move* m) { return has(r, m); }), v.end());
}
void removeFirst(MoveList& v, const Move* m) {  // List.remove(Object)
    auto it = std::find(v.begin(), v.end(), m);
    if (it != v.end()) v.erase(it);
}
void setMoves(TrainerPokemon& tp, const MoveList& l) {
    for (size_t i = 0; i < 4; i++) tp.moves[i] = i < l.size() ? l[i]->number : 0;
}
// añade elementos de "extra" (barajado antes) hasta bias*tamaño veces, en ciclo
void addBiased(MoveList& to, const MoveList& extra, double bias) {
    for (int i = 0; i < bias * static_cast<double>(extra.size()); i++) to.push_back(extra[i % extra.size()]);
}
}  // namespace

void TrainerMovesetRandomizer::randomizeTrainerMovesets() {
    bool cyclic = settings.evolutionsMod == Settings::EvolutionsMod::RANDOM_EVERY_LEVEL;
    bool multiOnly = settings.settingBattleStyle.isOnlyMultiBattles();
    std::vector<TrainerPtr> trainers;
    for (const auto& t : romHandler.getTrainers()) {
        if (!((t->isBoss() && settings.betterBossTrainerMovesets) || (t->isImportant() && settings.betterImportantTrainerMovesets) ||
              (t->isRegular() && settings.betterRegularTrainerMovesets)))
            continue;
        if (t->shouldNotGetBuffs()) continue;
        trainers.push_back(t);
    }
    const int perfectAcc = romHandler.getPerfectAccuracy();
    const TypeTable typeTable = romHandler.getTypeTable();
    for (const auto& t : trainers) {
        for (auto& tpp : t->pokemon) {
            TrainerPokemon& tp = *tpp;
            tp.resetMoves = false;
            MoveList movesAtLevel = poolAtLevel(tp, cyclic);
            movesAtLevel = trimMoveList(tp, movesAtLevel, multiOnly);
            if (movesAtLevel.empty()) continue;

            double trainerTypeModifier = 1;
            if (t->isImportant()) trainerTypeModifier = 1.5;
            else if (t->isBoss()) trainerTypeModifier = 2;
            double movePoolSizeModifier = movesAtLevel.size() / 10.0;
            double bonus = trainerTypeModifier * movePoolSizeModifier;
            double atkSpatkRatioModifier = 0.75;
            double stabBias = 0.25 * bonus, hardAbilityBias = 1 * bonus, softAbilityBias = 0.5 * bonus, statBias = 0.5 * bonus;
            double softMoveBias = 0.25 * bonus, hardMoveBias = 1 * bonus, softMoveAntiBias = 0.5;
            Species* pk = tp.species();

            MoveList stab = filter(movesAtLevel, [&](Move* mv) { return mv->type == pk->primaryType() && mv->category != MoveCategory::STATUS; });
            random.shuffle(stab);
            addBiased(movesAtLevel, stab, stabBias);
            if (pk->secondaryType()) {
                stab = filter(movesAtLevel, [&](Move* mv) { return mv->type == pk->secondaryType() && mv->category != MoveCategory::STATUS; });
                random.shuffle(stab);
                addBiased(movesAtLevel, stab, stabBias);
            }
            if (hasAbilities()) movesAtLevel = abilitySynergies(tp, pk, movesAtLevel, hardAbilityBias, softAbilityBias);

            MoveList dl = distinct(movesAtLevel);
            if (dl.size() <= 4) {
                setMoves(tp, dl);
                continue;
            }
            movesAtLevel = statSynergies(pk, movesAtLevel, statBias);
            dl = distinct(movesAtLevel);
            if (dl.size() <= 4) {
                setMoves(tp, dl);
                continue;
            }

            double ratio = atkSpatkRatio(tp, pk);
            MoveList phys = filter(movesAtLevel, [](Move* mv) { return mv->category == MoveCategory::PHYSICAL; });
            MoveList spec = filter(movesAtLevel, [](Move* mv) { return mv->category == MoveCategory::SPECIAL; });
            if (ratio < 1 && !spec.empty()) {
                ratio = 1 / ratio;
                double accepted = atkSpatkRatioModifier * ratio;
                int additional = static_cast<int>(phys.size() * accepted) - static_cast<int>(spec.size());
                for (int i = 0; i < additional; i++) movesAtLevel.push_back(spec[random.nextInt(static_cast<int>(spec.size()))]);
            } else if (!phys.empty()) {
                double accepted = atkSpatkRatioModifier * ratio;
                int additional = static_cast<int>(spec.size() * accepted) - static_cast<int>(phys.size());
                for (int i = 0; i < additional; i++) movesAtLevel.push_back(phys[random.nextInt(static_cast<int>(phys.size()))]);
            }

            MoveList picked;
            for (int i = 1; i <= 4; i++) {
                MoveList goodDamaging;
                MoveList* pickFrom = &movesAtLevel;  // en Java es la misma lista (salvo i == 1 con ataques buenos)
                if (i == 1) {
                    goodDamaging = filter(movesAtLevel, [&](Move* mv) { return mv->isGoodDamaging(perfectAcc); });
                    if (!goodDamaging.empty()) pickFrom = &goodDamaging;
                }
                if (i == 4) {
                    MoveList needOther = distinct(filter(movesAtLevel, [](Move* mv) { return has(global::requiresOtherMove, mv->number); }));
                    for (Move* dep : needOther) {
                        bool hasRequired = false;
                        for (Move* req : MoveSynergy::requiresOtherMove(dep, movesAtLevel)) {
                            if (has(picked, req)) {
                                hasRequired = true;
                                break;
                            }
                        }
                        if (!hasRequired) removeAllOf(movesAtLevel, {dep});
                    }
                }
                Move* move = (*pickFrom)[random.nextInt(static_cast<int>(pickFrom->size()))];
                picked.push_back(move);
                if (i == 4) break;
                removeAllOf(movesAtLevel, {move});
                removeAllOf(movesAtLevel, MoveSynergy::getHardMoveAntiSynergy(move, movesAtLevel));
                dl = distinct(movesAtLevel);
                if (static_cast<int>(dl.size()) <= 4 - i) {
                    picked.insert(picked.end(), dl.begin(), dl.end());
                    break;
                }
                MoveList hard = MoveSynergy::getMoveSynergy(move, movesAtLevel, romHandler.generationOfPokemon());
                random.shuffle(hard);
                addBiased(movesAtLevel, hard, hardMoveBias);
                MoveList soft = MoveSynergy::getSoftMoveSynergy(move, movesAtLevel, typeTable);
                random.shuffle(soft);
                addBiased(movesAtLevel, soft, softMoveBias);
                MoveList softAnti = MoveSynergy::getSoftMoveAntiSynergy(move, movesAtLevel);
                random.shuffle(softAnti);
                for (int j = 0; j < softMoveAntiBias * static_cast<double>(softAnti.size()); j++) {
                    if (static_cast<int>(distinct(movesAtLevel).size()) <= 4 - i) break;
                    removeFirst(movesAtLevel, softAnti[j % softAnti.size()]);
                }
                dl = distinct(movesAtLevel);
                if (static_cast<int>(dl.size()) <= 4 - i) {
                    picked.insert(picked.end(), dl.begin(), dl.end());
                    break;
                }
            }
            setMoves(tp, picked);
        }
    }
    changesMade = true;
}

TrainerMovesetRandomizer::MoveList TrainerMovesetRandomizer::abilitySynergies(const TrainerPokemon& tp, Species* pk, MoveList moves,
                                                                              double hardBias, double softBias) {
    int ability = romHandler.getAbilityForTrainerPokemon(tp);
    MoveList hard = MoveSynergy::getHardAbilityMoveSynergy(ability, pk->primaryType(), pk->secondaryType(), moves,
                                                           romHandler.generationOfPokemon(), romHandler.getPerfectAccuracy());
    random.shuffle(hard);
    addBiased(moves, hard, hardBias);
    MoveList soft = MoveSynergy::getSoftAbilityMoveSynergy(ability, moves, pk->primaryType(), pk->secondaryType());
    random.shuffle(soft);
    addBiased(moves, soft, softBias);
    MoveList anti = MoveSynergy::getSoftAbilityMoveAntiSynergy(ability, moves);
    MoveList without = moves;
    for (Move* mv : anti) removeFirst(without, mv);
    if (!without.empty()) moves = without;
    return moves;
}

TrainerMovesetRandomizer::MoveList TrainerMovesetRandomizer::statSynergies(Species* pk, MoveList moves, double bias) {
    MoveList syn = MoveSynergy::getStatMoveSynergy(pk->baseStats(), moves);
    random.shuffle(syn);
    addBiased(moves, syn, bias);
    MoveList anti = MoveSynergy::getStatMoveAntiSynergy(pk->baseStats(), moves);
    MoveList without = moves;
    for (Move* mv : anti) removeFirst(without, mv);
    if (!without.empty()) moves = without;
    return moves;
}

double TrainerMovesetRandomizer::atkSpatkRatio(const TrainerPokemon& tp, Species* pk) {
    const auto& bs = pk->baseStats();
    double r = static_cast<double>(bs.attack) / static_cast<double>(bs.spatk);
    if (hasAbilities()) {
        switch (romHandler.getAbilityForTrainerPokemon(tp)) {
            case abilities::hugePower:
            case abilities::purePower: r *= 2; break;
            case abilities::hustle:
            case abilities::gorillaTactics: r *= 1.5; break;
            case abilities::moxie: r *= 1.1; break;
            case abilities::soulHeart: r *= 0.9; break;
        }
    }
    return r;
}

TrainerMovesetRandomizer::MoveList TrainerMovesetRandomizer::trimMoveList(TrainerPokemon& tp, MoveList moves, bool multiOnly) {
    auto done = [&]() {
        if (moves.size() <= 4) {
            setMoves(tp, moves);
            return true;
        }
        return false;
    };
    if (done()) return {};
    moves = filter(moves, [&](Move* mv) {
        return !has(global::uselessMoves, mv->number) && (multiOnly || !has(global::doubleBattleMoves, mv->number));
    });
    if (done()) return {};
    removeAllOf(moves, obsoleteMoves(moves));
    if (done()) return {};
    MoveList needOther = filter(moves, [](Move* mv) { return has(global::requiresOtherMove, mv->number); });
    for (Move* dep : needOther)
        if (MoveSynergy::requiresOtherMove(dep, moves).empty()) removeFirst(moves, dep);
    if (done()) return {};
    if (hasAbilities()) {
        MoveList without = moves;
        removeAllOf(without, MoveSynergy::getHardAbilityMoveAntiSynergy(romHandler.getAbilityForTrainerPokemon(tp), moves));
        if (!without.empty()) moves = without;
    }
    if (done()) return {};
    return moves;
}

TrainerMovesetRandomizer::MoveList TrainerMovesetRandomizer::obsoleteMoves(const MoveList& moves) {
    using SCMT = StatChangeMoveType;
    MoveList obs;
    for (Move* mv : moves) {
        if (has(global::cannotObsoleteMoves, mv->number)) continue;
        if (mv->power > 0) {
            for (Move* mv2 : moves) {
                bool ok = !has(global::cannotBeObsoletedMoves, mv2->number) && mv->type == mv2->type &&
                          ((((mv->statChangeMoveType == mv2->statChangeMoveType && mv->statChanges[0] == mv2->statChanges[0]) ||
                             (mv2->statChangeMoveType == SCMT::NONE_OR_UNKNOWN && mv->hasBeneficialStatChange())) &&
                            mv->absorbPercent >= mv2->absorbPercent && !mv->isChargeMove && !mv->isRechargeMove) ||
                           mv2->power * mv2->hitCount <= 30) &&
                          mv->hitratio >= mv2->hitratio && mv->category == mv2->category && mv->priority >= mv2->priority &&
                          mv2->power > 0 && mv->power * mv->hitCount > mv2->power * mv2->hitCount;
                if (ok) obs.push_back(mv2);
            }
        } else if (mv->statChangeMoveType == SCMT::NO_DAMAGE_USER || mv->statChangeMoveType == SCMT::NO_DAMAGE_TARGET) {
            std::vector<StatChange> sc1;
            for (const auto& sc : mv->statChanges)
                if (sc.type != StatChangeType::NONE) sc1.push_back(sc);
            auto contains = [](const std::vector<StatChange>& v, const StatChange& s) { return std::find(v.begin(), v.end(), s) != v.end(); };
            for (Move* mv2 : moves) {
                if (!(mv2 != mv && mv2->power <= 0 && mv2->statChangeMoveType == mv->statChangeMoveType &&
                      (mv2->statusType == mv->statusType || mv2->statusType == StatusType::NONE)))
                    continue;
                std::vector<StatChange> sc2;
                for (const auto& sc : mv2->statChanges)
                    if (sc.type != StatChangeType::NONE) sc2.push_back(sc);
                if (sc2.size() > sc1.size()) continue;
                std::vector<StatChange> sc1f;
                for (const auto& s : sc1)
                    if (!contains(sc2, s)) sc1f.push_back(s);
                sc2.erase(std::remove_if(sc2.begin(), sc2.end(), [&](const StatChange& s) { return contains(sc1, s); }), sc2.end());
                if (!sc1f.empty() && sc2.empty()) {
                    if (!has(global::cannotBeObsoletedMoves, mv2->number)) obs.push_back(mv2);
                    continue;
                }
                if (sc1f.empty() && sc2.empty()) continue;
                bool maybeBetter = false;
                for (const auto& s1 : sc1f) {
                    bool canStill = false;
                    for (const auto& s2 : sc2) {
                        if (s1.type == s2.type) {
                            canStill = true;
                            if ((mv->statChangeMoveType == SCMT::NO_DAMAGE_USER && s1.stages > s2.stages) ||
                                (mv->statChangeMoveType == SCMT::NO_DAMAGE_TARGET && s1.stages < s2.stages))
                                maybeBetter = true;
                            else
                                canStill = false;
                        }
                    }
                    if (!canStill) {
                        maybeBetter = false;
                        break;
                    }
                }
                if (maybeBetter && !has(global::cannotBeObsoletedMoves, mv2->number)) obs.push_back(mv2);
            }
        }
    }
    return distinct(obs);
}

TrainerMovesetRandomizer::MoveList TrainerMovesetRandomizer::poolAtLevel(const TrainerPokemon& tp, bool cyclic) {
    auto& moves = const_cast<std::vector<std::optional<Move>>&>(romHandler.getMoves());
    auto mv = [&](int n) { return &*moves.at(n); };
    const double eggP = 0.1, preEvoP = 0.5, tmP = 0.6, tutorP = 0.6;
    if (!allLevelUp_) allLevelUp_ = romHandler.getMovesLearnt();
    if (!allEgg_) allEgg_ = romHandler.getEggMoves();
    if (!allTMCompat_) allTMCompat_ = romHandler.getTMHMCompatibility();
    if (!allTMMoves_) allTMMoves_ = romHandler.getTMMoves();
    if (!allTutorCompat_ && romHandler.hasMoveTutors()) allTutorCompat_ = romHandler.getMoveTutorCompatibility();
    if (!allTutorMoves_) allTutorMoves_ = romHandler.getMoveTutorMoves();
    Species* sp = tp.species();
    const int lvl = tp.level;

    MoveList pool;
    for (const auto& ml : allLevelUp_->at(sp->number()))
        if ((ml.level <= lvl && ml.level != 0) || (ml.level == 0 && lvl >= 30)) pool.push_back(mv(ml.move));
    pool = distinct(pool);

    if (!cyclic) {
        Species* pre = romHandler.altFormesCanHaveDifferentEvolutions() ? sp : sp->baseForme();
        while (!pre->evolutionsTo().empty()) {
            pre = pre->evolutionsTo()[0]->from();
            MoveList add;
            for (const auto& ml : allLevelUp_->at(pre->number()))
                if (ml.level <= lvl && random.nextDouble() < preEvoP) add.push_back(mv(ml.move));
            add = distinct(add);
            pool.insert(pool.end(), add.begin(), add.end());
        }
    }
    auto compatMoves = [&](const std::vector<bool>& compat, const std::vector<int>& list, double p) {
        for (int m : list) {
            size_t idx = std::find(list.begin(), list.end(), m) - list.begin();  // indexOf: primera aparición
            if (!compat[idx + 1]) continue;
            Move* tm = mv(m);
            if (tm->power > 1 && lvl * 3 > tm->power * tm->hitCount && random.nextDouble() < p) pool.push_back(tm);
            else if ((tm->power <= 1 && random.nextInt(100) < lvl) || random.nextInt(200) < lvl) pool.push_back(tm);
        }
    };
    compatMoves(allTMCompat_->at(sp->number()), *allTMMoves_, tmP);
    if (romHandler.hasMoveTutors()) compatMoves(allTutorCompat_->at(sp->number()), *allTutorMoves_, tutorP);

    if (!cyclic) {
        Species* first = romHandler.altFormesCanHaveDifferentEvolutions() ? sp : sp->baseForme();
        while (!first->evolutionsTo().empty()) first = first->evolutionsTo()[0]->from();
        auto it = allEgg_->find(first->number());
        if (it != allEgg_->end())
            for (int egm : it->second)
                if (random.nextDouble() < eggP) pool.push_back(mv(egm));
    }
    return distinct(pool);
}

}  // namespace pr
