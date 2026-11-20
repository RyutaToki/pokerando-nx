#include "random/tmhm_compat_randomizer.hpp"

#include <algorithm>

namespace pr {

double TMHMTutorCompatibilityRandomizer::probability(Species* pk, const Move& mv, bool requiredEarly, bool preferSameType) {
    double p = 0.5;
    if (preferSameType) {
        if (pk->primaryType() == mv.type || (pk->secondaryType() && pk->secondaryType() == mv.type)) p = 0.9;
        else if (mv.type && *mv.type == Type::NORMAL) p = 0.5;
        else p = 0.25;
    }
    if (requiredEarly) p = std::min(1.0, p * 1.8);
    return p;
}

void TMHMTutorCompatibilityRandomizer::randomizeCompat(Species* pk, std::vector<bool>& flags, const std::vector<int>& ids,
                                                       const std::vector<int>& prio, bool pst) {
    const auto& moves = romHandler.getMoves();
    for (size_t i = 1; i <= ids.size(); i++) {
        int m = ids[i - 1];
        double p = probability(pk, *moves.at(m), std::find(prio.begin(), prio.end(), m) != prio.end(), pst);
        flags[i] = random.nextDouble() < p;
    }
}

void TMHMTutorCompatibilityRandomizer::copyUp(Species* from, Species* to, const std::vector<bool>& prev, std::vector<bool>& tf,
                                              const std::vector<int>& ids, bool pst) {
    const auto& moves = romHandler.getMoves();
    for (size_t i = 1; i <= ids.size(); i++) {
        if (!prev[i]) {
            const Move& mv = *moves.at(ids[i - 1]);
            double p = 0.25;
            if (pst) {
                p = 0.1;
                auto tp = to->primaryType(), ts = to->secondaryType(), fp = from->primaryType(), fs = from->secondaryType();
                if ((tp == mv.type && tp != fp && tp != fs) || (ts && ts == mv.type && ts != fs && ts != fp)) p = 0.9;
            }
            tf[i] = random.nextDouble() < p;
        } else {
            tf[i] = prev[i];
        }
    }
}

void TMHMTutorCompatibilityRandomizer::randomizeGeneric(Compat& compat, const std::vector<int>& moves, const std::vector<int>& prio,
                                                        bool pst, bool follow, bool copySplit) {
    if (follow) {
        CopyUpEvolutionsHelper::Options o(
            [&](Species* pk) { randomizeCompat(pk, compat.at(pk->number()), moves, prio, pst); },
            [&](Species* f, Species* t, bool) { copyUp(f, t, compat.at(f->number()), compat.at(t->number()), moves, pst); });
        o.withCopySplitEvos(copySplit);
        copyUpEvolutionsHelper.apply(o);
    } else {
        for (auto& [num, flags] : compat) randomizeCompat(romHandler.getSpeciesInclFormes().at(num), flags, moves, prio, pst);
    }
}

void TMHMTutorCompatibilityRandomizer::randomizeTMHMCompatibility() {
    bool pst = settings.tmsHmsCompatibilityMod == Settings::TMsHMsCompatibilityMod::RANDOM_PREFER_TYPE;
    auto compat = romHandler.getTMHMCompatibility();
    auto tmhms = romHandler.getTMMoves();
    auto hms = romHandler.getHMMoves();
    tmhms.insert(tmhms.end(), hms.begin(), hms.end());
    randomizeGeneric(compat, tmhms, romHandler.getEarlyRequiredHMMoves(), pst, settings.tmsFollowEvolutions, false);
    romHandler.setTMHMCompatibility(compat);
    tmhmChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::fullTMHMCompatibility() {
    auto compat = romHandler.getTMHMCompatibility();
    for (auto& [n, f] : compat)
        for (size_t i = 1; i < f.size(); i++) f[i] = true;
    romHandler.setTMHMCompatibility(compat);
}

void TMHMTutorCompatibilityRandomizer::sanity(Compat& compat, const std::vector<int>& moves) {
    auto movesets = romHandler.getMovesLearnt();
    for (auto& [n, f] : compat) {
        for (const auto& ml : movesets.at(n)) {
            auto it = std::find(moves.begin(), moves.end(), ml.move);
            if (it != moves.end()) f[(it - moves.begin()) + 1] = true;
        }
    }
}

void TMHMTutorCompatibilityRandomizer::evoSanity(Compat& compat) {
    CopyUpEvolutionsHelper::Options o(nullptr, [&](Species* f, Species* t, bool) {
        auto& from = compat.at(f->number());
        auto& to = compat.at(t->number());
        for (size_t i = 1; i < to.size(); i++) to[i] = to[i] || from[i];
    });
    o.withCopySplitEvos(true);
    copyUpEvolutionsHelper.apply(o);
}

void TMHMTutorCompatibilityRandomizer::cosmetic(Compat& compat) {
    for (auto& [n, f] : compat) {
        Species* pk = romHandler.getSpeciesInclFormes().at(n);
        if (!pk->isEssentiallyCosmetic()) continue;
        const auto& base = compat.at(pk->conceptualBaseForme()->number());
        for (size_t i = 1; i < f.size(); i++) f[i] = base[i];
    }
}

void TMHMTutorCompatibilityRandomizer::ensureTMCompatSanity() {
    auto compat = romHandler.getTMHMCompatibility();
    sanity(compat, romHandler.getTMMoves());
    romHandler.setTMHMCompatibility(compat);
    tmhmChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::ensureTMEvolutionSanity() {
    auto compat = romHandler.getTMHMCompatibility();
    evoSanity(compat);
    romHandler.setTMHMCompatibility(compat);
    tmhmChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::fullHMCompatibility() {
    auto compat = romHandler.getTMHMCompatibility();
    int tmCount = romHandler.getTMCount();
    for (auto& [n, f] : compat)
        for (size_t i = tmCount + 1; i < f.size(); i++) f[i] = true;
    romHandler.setTMHMCompatibility(compat);
    tmhmChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::copyTMCompatibilityToCosmeticFormes() {
    auto compat = romHandler.getTMHMCompatibility();
    cosmetic(compat);
    romHandler.setTMHMCompatibility(compat);
    tmhmChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::randomizeMoveTutorCompatibility() {
    if (!romHandler.hasMoveTutors()) return;
    bool pst = settings.moveTutorsCompatibilityMod == Settings::MoveTutorsCompatibilityMod::RANDOM_PREFER_TYPE;
    auto compat = romHandler.getMoveTutorCompatibility();
    randomizeGeneric(compat, romHandler.getMoveTutorMoves(), {}, pst, settings.tutorFollowEvolutions, true);
    romHandler.setMoveTutorCompatibility(compat);
    tutorChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::fullMoveTutorCompatibility() {
    if (!romHandler.hasMoveTutors()) return;
    auto compat = romHandler.getMoveTutorCompatibility();
    for (auto& [n, f] : compat)
        for (size_t i = 1; i < f.size(); i++) f[i] = true;
    romHandler.setMoveTutorCompatibility(compat);
    tutorChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::ensureMoveTutorCompatSanity() {
    if (!romHandler.hasMoveTutors()) return;
    auto compat = romHandler.getMoveTutorCompatibility();
    sanity(compat, romHandler.getMoveTutorMoves());
    romHandler.setMoveTutorCompatibility(compat);
    tutorChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::ensureMoveTutorEvolutionSanity() {
    if (!romHandler.hasMoveTutors()) return;
    auto compat = romHandler.getMoveTutorCompatibility();
    evoSanity(compat);
    romHandler.setMoveTutorCompatibility(compat);
    tutorChangesMade = true;
}

void TMHMTutorCompatibilityRandomizer::copyMoveTutorCompatibilityToCosmeticFormes() {
    auto compat = romHandler.getMoveTutorCompatibility();
    cosmetic(compat);
    romHandler.setMoveTutorCompatibility(compat);
    tutorChangesMade = true;
}

}  // namespace pr
