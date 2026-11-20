// Port de RestrictedSpeciesService: los Pokémon disponibles según las restricciones de generación.
#pragma once
#include <optional>

#include "gamedata/gen_restrictions.hpp"
#include "gamedata/species.hpp"

namespace pr {

class RomHandler;
class JavaRandom;

class RestrictedSpeciesService {
public:
    explicit RestrictedSpeciesService(RomHandler& rh) : rh_(rh) {}

    SpeciesSet getSpecies(bool noLegendaries, bool allowAltFormes, bool allowCosmeticFormes) const;
    Species* randomSpecies(JavaRandom& random) const;
    const SpeciesSet& getAll(bool includeAltFormes) const;
    const SpeciesSet& getNonLegendaries(bool includeAltFormes) const;
    const SpeciesSet& getLegendaries(bool includeAltFormes) const;
    const SpeciesSet& getUltrabeasts(bool includeAltFormes) const;
    const SpeciesSet& getMegaEvolutions() const;
    SpeciesSet getAbilityDependentFormes() const;
    SpeciesSet getBannedFormesForPlayerPokemon() const;
    void setRestrictions(const std::optional<GenRestrictions>& restrictions);

private:
    void check() const;
    RomHandler& rh_;
    bool restrictionsSet_ = false;
    SpeciesSet all_, allInclAltFormes_, nonLegendaries_, nonLegendariesInclAltFormes_, legendaries_,
        legendariesInclAltFormes_, ultraBeasts_, ultraBeastsInclAltFormes_, megaEvolutions_;
};

class TypeService {
public:
    explicit TypeService(RomHandler& rh) : rh_(rh) {}
    std::vector<Type> getTypes() const;
    bool typeInGame(Type t) const;
    Type randomType(JavaRandom& random) const;

private:
    RomHandler& rh_;
};

}  // namespace pr
