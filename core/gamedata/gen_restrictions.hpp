// Port de GenRestrictions (qué generaciones de Pokémon se permiten).
#pragma once
#include <array>

namespace pr {

struct GenRestrictions {
    static constexpr int MAX_GENERATION = 7;
    std::array<bool, MAX_GENERATION> gensAllowed{true, true, true, true, true, true, true};
    bool allowEvolutionaryRelatives = true;

    GenRestrictions() = default;
    explicit GenRestrictions(int state) {
        allowEvolutionaryRelatives = (state & 1) > 0;
        for (int g = 1; g <= MAX_GENERATION; g++) setGenAllowed(g, (state & (1 << g)) > 0);
    }
    bool nothingSelected() const {
        for (bool b : gensAllowed)
            if (b) return false;
        return true;
    }
    int toInt() const {
        int v = allowEvolutionaryRelatives ? 1 : 0;
        for (int g = 1; g <= MAX_GENERATION; g++) v |= isGenAllowed(g) ? (1 << g) : 0;
        return v;
    }
    bool isGenAllowed(int gen) const { return gensAllowed[gen - 1]; }
    void setGenAllowed(int gen, bool a) { gensAllowed[gen - 1] = a; }
    void limitToGen(int gen) {
        for (int i = MAX_GENERATION; i > gen; i--) setGenAllowed(i, false);
    }
};

}  // namespace pr
