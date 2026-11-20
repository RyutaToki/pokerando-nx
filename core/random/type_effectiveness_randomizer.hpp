// Port de TypeEffectivenessRandomizer.
#pragma once
#include <array>

#include "random/randomizer.hpp"

namespace pr {

class TypeEffectivenessRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeTypeEffectiveness(bool balanced);
    void randomizeTypeEffectivenessKeepIdentities();
    void invertTypeEffectiveness(bool randomImmunities);

private:
    bool balanced_ = false;
    std::optional<TypeTable> old_, table_;
    std::array<int, 6> effCounts_{};
    std::array<int, 6> maxAtk_{}, maxDef_{};
    int tries_ = 0;
    void placeEffectiveness(Effectiveness e);
    bool isValidPlacement(Type a, Type d, Effectiveness e);
};

}  // namespace pr
