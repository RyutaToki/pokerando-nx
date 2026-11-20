// Base de los randomizadores (port de Randomizer.java) y excepción de randomización.
#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "random/copy_up_evolutions_helper.hpp"
#include "romio/rom_handler.hpp"
#include "romio/services/restricted_species_service.hpp"
#include "settings/settings.hpp"
#include "util/java_random.hpp"

namespace pr {

struct RandomizationException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Randomizer {
public:
    Randomizer(RomHandler& rh, Settings& settings, JavaRandom& random)
        : romHandler(rh), rSpecService(rh.getRestrictedSpeciesService()), typeService(rh.getTypeService()),
          copyUpEvolutionsHelper([&rh] { return rh.getSpeciesSetInclFormes(); }), settings(settings), random(random) {}
    virtual ~Randomizer() = default;
    bool isChangesMade() const { return changesMade; }

protected:
    int applyPercentageLevelModifier(int level, int pct) const {
        int modified = static_cast<int>(std::floor(level * (1 + pct / 100.0) + 0.5));  // Math.round
        return std::clamp(modified, 1, 100);
    }
    static bool detectUpperCaseNames(const std::vector<std::string>& names);

    RomHandler& romHandler;
    RestrictedSpeciesService& rSpecService;
    TypeService& typeService;
    CopyUpEvolutionsHelper copyUpEvolutionsHelper;
    Settings& settings;
    JavaRandom& random;
    bool changesMade = false;
};

}  // namespace pr
