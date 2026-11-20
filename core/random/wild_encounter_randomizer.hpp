// Port de WildEncounterRandomizer.
#pragma once
#include <map>
#include <memory>

#include "random/randomizer.hpp"

namespace pr {

class WildEncounterRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeEncounters();
    void changeCatchRates();

private:
    using SetPtr = std::shared_ptr<SpeciesSet>;
    using Zone = std::vector<EncounterArea*>;

    struct AreaInfo {
        explicit AreaInfo(Species* s) : species(s) {}
        std::map<Type, int> possibleThemes;  // EnumMap
        SpeciesSet bannedForReplacement;
        SpeciesSet family;
        Species* species;
        int lowestLevel = 100;
        void addTypeTheme(std::optional<Type> theme, int count);
        std::optional<Type> getTheme(bool defaultToPrimary) const;
        SpeciesSet getBannedForReplacement() const { return SpeciesSet::unmodifiable(bannedForReplacement.toVector()); }
        SpeciesSet getFamily() const { return SpeciesSet::unmodifiable(family.toVector()); }
    };
    using InfoMap = JavaHashMap<Species*, std::shared_ptr<AreaInfo>, JavaHashCode<Species*>, SpeciesPtrEq>;

    struct Inner;
    std::vector<EncounterArea*> prepEncounterAreas(std::vector<EncounterArea>& areas);
    SpeciesSet getBannedForWildEncounters(bool banIrregularAltFormes, bool abilitiesAreRandomized);
    void applyLevelModifier(int mod, std::vector<EncounterArea>& areas);
    void randomizeCosmeticForme(Encounter& enc);
    void minimumCatchRate(int rateNonLegendary, int rateLegendary);
};

}  // namespace pr
