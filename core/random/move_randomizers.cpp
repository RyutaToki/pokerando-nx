#include "random/move_randomizers.hpp"

#include <algorithm>
#include <cmath>

#include "constants/global_extra.hpp"
#include "constants/move_ids.hpp"

namespace pr {

namespace {
std::vector<Move*> movesList(RomHandler& rh) {
    std::vector<Move*> v;
    for (auto& m : const_cast<std::vector<std::optional<Move>>&>(rh.getMoves()))
        if (m) v.push_back(&*m);
    return v;
}
bool has(const std::vector<int>& v, int x) { return std::find(v.begin(), v.end(), x) != v.end(); }
}  // namespace

void MoveDataRandomizer::randomizeMovePowers() {
    for (Move* mv : movesList(romHandler)) {
        if (mv->internalId == moves::struggle || mv->power < 10) continue;
        if (random.nextInt(3) != 2) mv->power = random.nextInt(11) * 5 + 50;
        else mv->power = random.nextInt(27) * 5 + 20;
        for (int i = 0; i < 2; i++)
            if (random.nextInt(100) == 0) mv->power += 50;
        if (mv->hitCount != 1) {
            mv->power = static_cast<int>(std::floor(mv->power / mv->hitCount / 5 + 0.5) * 5);
            if (mv->power == 0) mv->power = 5;
        }
    }
    changesMade = true;
}

void MoveDataRandomizer::randomizeMovePPs() {
    for (Move* mv : movesList(romHandler)) {
        if (mv->internalId == moves::struggle) continue;
        if (random.nextInt(3) != 2) mv->pp = random.nextInt(3) * 5 + 15;
        else mv->pp = random.nextInt(8) * 5 + 5;
    }
    changesMade = true;
}

void MoveDataRandomizer::randomizeMoveAccuracies() {
    for (Move* mv : movesList(romHandler)) {
        if (mv->internalId == moves::struggle || mv->hitratio < 5) continue;
        if (mv->hitratio <= 50) {
            mv->hitratio = random.nextInt(7) * 5 + 20;
            if (random.nextInt(10) == 0) mv->hitratio = (mv->hitratio * 3 / 2) / 5 * 5;  // aritmética de double, como en Java
        } else if (mv->hitratio < 90) {
            mv->hitratio = 100;
            while (mv->hitratio > 20) {
                if (random.nextInt(10) < 2) break;
                mv->hitratio -= 5;
            }
        } else {
            mv->hitratio = 100;
            while (mv->hitratio > 20) {
                if (random.nextInt(10) < 4) break;
                mv->hitratio -= 5;
            }
        }
    }
    changesMade = true;
}

void MoveDataRandomizer::randomizeMoveTypes() {
    for (Move* mv : movesList(romHandler))
        if (mv->internalId != moves::struggle && mv->type) mv->type = typeService.randomType(random);
    changesMade = true;
}

void MoveDataRandomizer::randomizeMoveCategory() {
    if (!romHandler.hasPhysicalSpecialSplit()) return;
    for (Move* mv : movesList(romHandler)) {
        if (mv->internalId == moves::struggle || mv->category == MoveCategory::STATUS) continue;
        if (random.nextInt(2) == 0) mv->category = mv->category == MoveCategory::PHYSICAL ? MoveCategory::SPECIAL : MoveCategory::PHYSICAL;
    }
    changesMade = true;
}

std::vector<int> TMTutorMoveRandomizer::pickMoves(int count, const std::vector<int>& oldMoves, bool noBroken, bool preserveField,
                                                  double goodPct, const std::vector<int>& extraBanned) {
    auto hms = romHandler.getHMMoves();
    std::vector<int> banned = noBroken ? romHandler.getGameBreakingMoves() : std::vector<int>{};
    auto add = [&](const std::vector<int>& v) { banned.insert(banned.end(), v.begin(), v.end()); };
    add(romHandler.getMovesBannedFromLevelup());
    add(romHandler.getIllegalMoves());
    auto fieldMoves = romHandler.getFieldMoves();
    int preserved = 0;
    if (preserveField) {
        std::vector<int> existing;
        for (int m : oldMoves)
            if (has(fieldMoves, m)) existing.push_back(m);
        preserved = static_cast<int>(existing.size());
        add(existing);
    }
    std::vector<Move*> usable = movesList(romHandler);
    std::vector<Move*> unusable, unusableDamaging;
    for (Move* mv : usable) {
        if (global::bannedRandomMove(mv->number) || has(global::zMoves(), mv->number) || has(extraBanned, mv->number) ||
            has(hms, mv->number) || has(banned, mv->number)) {
            unusable.push_back(mv);
        } else if (global::bannedForDamagingMove(mv->number) || !mv->isGoodDamaging(romHandler.getPerfectAccuracy())) {
            unusableDamaging.push_back(mv);
        }
    }
    auto removeAllOf = [](std::vector<Move*>& v, const std::vector<Move*>& r) {
        v.erase(std::remove_if(v.begin(), v.end(), [&](Move* m) { return std::find(r.begin(), r.end(), m) != r.end(); }), v.end());
    };
    removeAllOf(usable, unusable);
    std::vector<Move*> usableDamaging = usable;
    removeAllOf(usableDamaging, unusableDamaging);
    std::vector<int> picked;
    int goodLeft = static_cast<int>(std::floor(goodPct * (count - preserved) + 0.5));
    for (int i = 0; i < count - preserved; i++) {
        Move* chosen;
        if (goodLeft > 0 && !usableDamaging.empty()) chosen = usableDamaging[random.nextInt(static_cast<int>(usableDamaging.size()))];
        else chosen = usable[random.nextInt(static_cast<int>(usable.size()))];
        picked.push_back(chosen->number);
        if (auto it = std::find(usable.begin(), usable.end(), chosen); it != usable.end()) usable.erase(it);
        if (auto it = std::find(usableDamaging.begin(), usableDamaging.end(), chosen); it != usableDamaging.end()) usableDamaging.erase(it);
        goodLeft--;
    }
    random.shuffle(picked);
    std::vector<int> out;
    size_t idx = 0;
    for (int old : oldMoves) out.push_back(preserveField && has(fieldMoves, old) ? old : picked.at(idx++));
    return out;
}

void TMTutorMoveRandomizer::randomizeTMMoves() {
    double pct = settings.tmsForceGoodDamaging ? settings.tmsGoodDamagingPercent / 100.0 : 0;
    auto oldTMs = romHandler.getTMMoves();
    oldTMs.resize(romHandler.getTMCount());
    romHandler.setTMMoves(pickMoves(romHandler.getTMCount(), oldTMs, settings.blockBrokenTMMoves, settings.keepFieldMoveTMs, pct, {}));
    tmChangesMade = true;
}

void TMTutorMoveRandomizer::randomizeMoveTutorMoves() {
    if (!romHandler.hasMoveTutors()) return;
    double pct = settings.tutorsForceGoodDamaging ? settings.tutorsGoodDamagingPercent / 100.0 : 0;
    auto oldMTs = romHandler.getMoveTutorMoves();
    romHandler.setMoveTutorMoves(pickMoves(static_cast<int>(oldMTs.size()), oldMTs, settings.blockBrokenTutorMoves,
                                           settings.keepFieldMoveTutors, pct, romHandler.getTMMoves()));
    tutorChangesMade = true;
}

}  // namespace pr
