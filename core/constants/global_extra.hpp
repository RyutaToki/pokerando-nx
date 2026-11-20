// Partes de GlobalConstants que no son simples listas (se construyen en un bloque static en Java).
#pragma once
#include <algorithm>
#include <vector>

#include "constants/global_constants.hpp"
#include "constants/move_ids.hpp"

namespace pr::global {

inline const std::vector<int>& zMoves() {
    static const std::vector<int> v = [] {
        std::vector<int> r(fixedPowerZMoves.begin(), fixedPowerZMoves.end());
        r.insert(r.end(), varyingPowerZMoves.begin(), varyingPowerZMoves.end());
        return r;
    }();
    return v;
}

inline bool bannedRandomMove(int m) { return m == moves::struggle; }

inline bool bannedForDamagingMove(int m) {
    using namespace moves;
    static const std::vector<int> v = {selfDestruct, dreamEater, explosion, snore, falseSwipe, futureSight, fakeOut,
                                       focusPunch, doomDesire, feint, lastResort, suckerPunch, constrict, rage, rollout,
                                       iceBall, synchronoise, shellTrap, foulPlay, spitUp, sonicBoom, dragonRage,
                                       hornDrill, guillotine, fissure, sheerCold};
    return std::find(v.begin(), v.end(), m) != v.end();
}

}  // namespace pr::global
