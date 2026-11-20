#include "random/species_moveset_randomizer.hpp"

#include <algorithm>
#include <cmath>

#include "constants/global_extra.hpp"
#include "constants/move_ids.hpp"

namespace pr {

namespace {
bool has(const std::vector<int>& v, int x) { return std::find(v.begin(), v.end(), x) != v.end(); }
bool unusedIn(const std::vector<Move*>& list, const std::vector<int>& used) {
    for (Move* m : list)
        if (!has(used, m->number)) return true;
    return false;
}
Species* findInPool(const SpeciesSet& pool, int id) {
    for (Species* s : pool.toVector())
        if (s->number() == id) return s;
    return nullptr;
}
}  // namespace

SpeciesMovesetRandomizer::MoveSets SpeciesMovesetRandomizer::createSetsOfMoves(bool noBroken) {
    MoveSets s;
    auto& all = const_cast<std::vector<std::optional<Move>>&>(romHandler.getMoves());
    std::vector<int> banned = noBroken ? romHandler.getGameBreakingMoves() : std::vector<int>{};
    auto add = [&](const std::vector<int>& v) { banned.insert(banned.end(), v.begin(), v.end()); };
    add(romHandler.getHMMoves());
    add(romHandler.getMovesBannedFromLevelup());
    add(global::zMoves());
    add(romHandler.getIllegalMoves());
    for (auto& om : all) {
        if (!om) continue;
        Move* mv = &*om;
        if (global::bannedRandomMove(mv->number) || has(banned, mv->number)) continue;
        s.valid.push_back(mv);
        if (mv->type) s.validType[*mv->type].push_back(mv);
        if (!global::bannedForDamagingMove(mv->number) && mv->isGoodDamaging(romHandler.getPerfectAccuracy())) {
            s.validDamaging.push_back(mv);
            if (mv->type) s.validTypeDamaging[*mv->type].push_back(mv);
        }
    }
    // equilibrar la potencia media de cada tipo añadiendo movimientos repetidos
    std::map<Type, double> avg;  // TreeMap
    double total = 0;
    // AVISO: FVX suma en el orden de un HashMap<Type> (no reproducible); aquí, orden del enum
    for (auto& [t, list] : s.validType) {
        int sum = 0;
        for (Move* m : list)
            if (m->power > 0) sum = static_cast<int>(sum + m->power * m->hitCount);  // += de Java: int += double
        double a = static_cast<double>(sum) / static_cast<double>(list.size());
        avg[t] = a;
        total += a;
    }
    total /= static_cast<double>(s.validType.size());
    double minAvg = total * 0.75, maxAvg = total * 1.25;
    for (auto& [t, a0] : avg) {
        double a = a0;
        auto& typeMoves = s.validType.at(t);
        std::vector<Move*> already;
        auto containsAll = [&](const std::vector<Move*>& v) {
            for (Move* m : v)
                if (std::find(already.begin(), already.end(), m) == already.end()) return false;
            return true;
        };
        auto step = [&](bool stronger) {
            std::vector<Move*> cand;
            for (Move* m : typeMoves)
                if (stronger ? m->power * m->hitCount > a : m->power * m->hitCount < a) cand.push_back(m);
            if (cand.empty()) return false;
            if (containsAll(cand)) {
                already.clear();
            } else {
                cand.erase(std::remove_if(cand.begin(), cand.end(),
                                          [&](Move* m) { return std::find(already.begin(), already.end(), m) != already.end(); }),
                           cand.end());
            }
            Move* extra = cand[random.nextInt(static_cast<int>(cand.size()))];
            a = (a * typeMoves.size() + extra->power * extra->hitCount) / (typeMoves.size() + 1);
            typeMoves.push_back(extra);
            already.push_back(extra);
            return true;
        };
        for (int it = 0; a < minAvg && it < 10000; it++)
            if (!step(true)) break;
        for (int it = 0; a > maxAvg && it < 10000; it++)
            if (!step(false)) break;
    }
    return s;
}

std::optional<Type> SpeciesMovesetRandomizer::pickTypeOfMove(Species* pk) {
    double picked = random.nextDouble();
    auto p = pk->primaryType(), s = pk->secondaryType();
    if ((p == Type::NORMAL && s) || s == Type::NORMAL) {
        auto other = p == Type::NORMAL ? s : p;
        if (picked < 0.1) return Type::NORMAL;
        if (picked < 0.4) return other;
    } else if (s) {
        if (picked < 0.2) return p;
        if (picked < 0.4) return s;
    } else if (picked < 0.4) {
        return p;
    }
    return std::nullopt;
}

Move* SpeciesMovesetRandomizer::pickMove(MoveSets& sets, bool attemptDamaging, std::optional<Type> typeOfMove, double ratio,
                                         const std::vector<int>& learnt) {
    std::vector<Move*>* pickList = &sets.valid;
    std::vector<Move*> filtered;
    if (attemptDamaging) {
        if (typeOfMove) {
            auto it = sets.validTypeDamaging.find(*typeOfMove);
            if (it != sets.validTypeDamaging.end() && unusedIn(it->second, learnt)) pickList = &it->second;
            else if (unusedIn(sets.validDamaging, learnt)) pickList = &sets.validDamaging;
        } else if (unusedIn(sets.validDamaging, learnt)) {
            pickList = &sets.validDamaging;
        }
        MoveCategory forced = random.nextDouble() < ratio ? MoveCategory::PHYSICAL : MoveCategory::SPECIAL;
        for (Move* m : *pickList)
            if (m->category == forced) filtered.push_back(m);
        if (!filtered.empty() && unusedIn(filtered, learnt)) pickList = &filtered;
    } else if (typeOfMove) {
        auto it = sets.validType.find(*typeOfMove);
        if (it != sets.validType.end() && unusedIn(it->second, learnt)) pickList = &it->second;
    }
    Move* mv = (*pickList)[random.nextInt(static_cast<int>(pickList->size()))];
    while (has(learnt, mv->number)) mv = (*pickList)[random.nextInt(static_cast<int>(pickList->size()))];
    return mv;
}

void SpeciesMovesetRandomizer::randomizeMovesLearnt() {
    bool typeThemed = settings.movesetsMod == Settings::MovesetsMod::RANDOM_PREFER_SAME_TYPE;
    bool forceStart = romHandler.supportsFourStartingMoves() && settings.startWithGuaranteedMoves;
    int forceCount = settings.guaranteedMoveCount;
    double goodPct = settings.movesetsForceGoodDamaging ? settings.movesetsGoodDamagingPercent / 100.0 : 0;
    auto movesets = romHandler.getMovesLearnt();
    auto sets = createSetsOfMoves(settings.blockBrokenMovesetMoves);
    for (auto& [num, moves] : movesets) {
        std::vector<int> learnt;
        int lv1Attacking = 0;
        Species* pk = findInPool(rSpecService.getAll(true), num);
        if (!pk) continue;
        double ratio = pk->baseStats().getAttackSpecialAttackRatio();
        if (forceStart) {
            int lv1 = 0;
            for (auto& ml : moves) lv1 += ml.level == 1;
            for (int i = 0; i < forceCount - lv1; i++) moves.insert(moves.begin(), MoveLearnt{0, 1});
        }
        if (settings.evolutionMovesForAll && moves.at(0).level != 0) moves.insert(moves.begin(), MoveLearnt{0, 0});
        if (pk->isEssentiallyCosmetic()) {
            for (size_t i = 0; i < moves.size(); i++) moves[i].move = movesets.at(pk->conceptualBaseForme()->number()).at(i).move;
            continue;
        }
        size_t lv1index = moves.at(0).level == 1 ? 0 : 1;  // el nivel 0 es el movimiento al evolucionar
        while (lv1index < moves.size() && moves[lv1index].level == 1) lv1index++;
        if (lv1index != 0) lv1index--;
        int goodLeft = static_cast<int>(std::floor(goodPct * moves.size() + 0.5));
        for (size_t i = 0; i < moves.size(); i++) {
            bool attemptDamaging = i == lv1index || goodLeft > 0;
            std::optional<Type> t;
            if (typeThemed) t = pickTypeOfMove(pk);
            Move* mv = pickMove(sets, attemptDamaging, t, ratio, learnt);
            if (i == lv1index) lv1Attacking = mv->number;
            else goodLeft--;
            learnt.push_back(mv->number);
        }
        random.shuffle(learnt);
        if (learnt.at(lv1index) != lv1Attacking) {
            for (size_t i = 0; i < learnt.size(); i++) {
                if (learnt[i] == lv1Attacking) {
                    learnt[i] = learnt[lv1index];
                    learnt[lv1index] = lv1Attacking;
                    break;
                }
            }
        }
        for (size_t i = 0; i < learnt.size(); i++) {
            moves[i].move = learnt[i];
            if (i == lv1index) moves[i].level = 1;
        }
    }
    romHandler.setMovesLearnt(movesets);
    changesMade = true;
}

void SpeciesMovesetRandomizer::randomizeEggMoves() {
    bool typeThemed = settings.movesetsMod == Settings::MovesetsMod::RANDOM_PREFER_SAME_TYPE;
    double goodPct = settings.movesetsForceGoodDamaging ? settings.movesetsGoodDamagingPercent / 100.0 : 0;
    auto movesets = romHandler.getEggMoves();
    auto sets = createSetsOfMoves(settings.blockBrokenMovesetMoves);
    for (auto& [num, moves] : movesets) {
        std::vector<int> learnt;
        Species* pk = findInPool(rSpecService.getAll(true), num);
        if (!pk) continue;
        double ratio = pk->baseStats().getAttackSpecialAttackRatio();
        if (pk->isEssentiallyCosmetic()) {
            for (size_t i = 0; i < moves.size(); i++) moves[i] = movesets.at(pk->conceptualBaseForme()->number()).at(i);
            continue;
        }
        int goodLeft = static_cast<int>(std::floor(goodPct * moves.size() + 0.5));
        for (size_t i = 0; i < moves.size(); i++) {
            bool attemptDamaging = goodLeft > 0;
            std::optional<Type> t;
            if (typeThemed) t = pickTypeOfMove(pk);
            Move* mv = pickMove(sets, attemptDamaging, t, ratio, learnt);
            goodLeft--;
            learnt.push_back(mv->number);
        }
        random.shuffle(learnt);
        for (size_t i = 0; i < learnt.size(); i++) moves[i] = learnt[i];
    }
    romHandler.setEggMoves(movesets);
    changesMade = true;
}

void SpeciesMovesetRandomizer::orderDamagingMovesByDamage() {
    auto movesets = romHandler.getMovesLearnt();
    const auto& all = romHandler.getMoves();
    for (auto& [num, moves] : movesets) {
        std::vector<size_t> idx;
        std::vector<const Move*> dmg;
        for (size_t i = 0; i < moves.size(); i++) {
            if (moves[i].level == 0) continue;  // no reordenar el movimiento al evolucionar
            const Move& mv = *all.at(moves[i].move);
            if (mv.power > 1) {
                idx.push_back(i);
                dmg.push_back(&mv);
            }
        }
        random.shuffle(dmg);
        std::stable_sort(dmg.begin(), dmg.end(), [](const Move* a, const Move* b) { return a->power * a->hitCount < b->power * b->hitCount; });
        for (size_t i = 0; i < dmg.size(); i++) moves[idx[i]].move = dmg[i]->number;
    }
    romHandler.setMovesLearnt(movesets);
    changesMade = true;
}

void SpeciesMovesetRandomizer::metronomeOnlyMode() {
    auto movesets = romHandler.getMovesLearnt();
    for (auto& [n, ms] : movesets) {
        if (!ms.empty()) ms = {MoveLearnt{moves::metronome, 1}};
    }
    romHandler.setMovesLearnt(movesets);
    for (const auto& t : romHandler.getTrainers())
        for (auto& tp : t->pokemon) tp->resetMoves = true;
    auto tms = romHandler.getTMMoves();
    std::fill(tms.begin(), tms.end(), moves::metronome);
    romHandler.setTMMoves(tms);
    if (romHandler.hasMoveTutors()) {
        auto mts = romHandler.getMoveTutorMoves();
        std::fill(mts.begin(), mts.end(), moves::metronome);
        romHandler.setMoveTutorMoves(mts);
    }
    auto& all = const_cast<std::vector<std::optional<Move>>&>(romHandler.getMoves());
    all.at(moves::metronome)->pp = 40;
    for (int hm : romHandler.getHMMoves()) all.at(hm)->pp = 0;
}

}  // namespace pr
