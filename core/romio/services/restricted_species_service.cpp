#include "romio/services/restricted_species_service.hpp"

#include <algorithm>
#include <stdexcept>

#include "constants/species_ids.hpp"
#include "romio/rom_handler.hpp"
#include "util/java_random.hpp"

namespace pr {

void RestrictedSpeciesService::check() const {
    if (!restrictionsSet_) throw std::logic_error("Restrictions not set.");
}

SpeciesSet RestrictedSpeciesService::getSpecies(bool noLegendaries, bool allowAltFormes, bool allowCosmeticFormes) const {
    SpeciesSet allowed;
    allowed.addAll(noLegendaries ? getNonLegendaries(allowAltFormes) : getAll(allowAltFormes));
    if (allowAltFormes && !allowCosmeticFormes) allowed.removeIf([](Species* s) { return s->isEssentiallyCosmetic(); });
    return SpeciesSet::unmodifiable(allowed.toVector());
}

Species* RestrictedSpeciesService::randomSpecies(JavaRandom& random) const {
    // getRandomSpecies modifica la caché del conjunto, como en Java
    return const_cast<SpeciesSet&>(getAll(false)).getRandomSpecies(random);
}

const SpeciesSet& RestrictedSpeciesService::getAll(bool i) const { check(); return i ? allInclAltFormes_ : all_; }
const SpeciesSet& RestrictedSpeciesService::getNonLegendaries(bool i) const {
    check();
    return i ? nonLegendariesInclAltFormes_ : nonLegendaries_;
}
const SpeciesSet& RestrictedSpeciesService::getLegendaries(bool i) const { check(); return i ? legendariesInclAltFormes_ : legendaries_; }
const SpeciesSet& RestrictedSpeciesService::getUltrabeasts(bool i) const { check(); return i ? ultraBeastsInclAltFormes_ : ultraBeasts_; }
const SpeciesSet& RestrictedSpeciesService::getMegaEvolutions() const { check(); return megaEvolutions_; }

SpeciesSet RestrictedSpeciesService::getAbilityDependentFormes() const {
    SpeciesSet r;
    for (Species* sp : allInclAltFormes_.toVector()) {
        if (sp->isBaseForme()) continue;
        int b = sp->baseNumber();
        if (b == species::castform || (b == species::darmanitan && sp->formeNumber() == 1) || b == species::aegislash ||
            b == species::wishiwashi)
            r.add(sp);
    }
    return r;
}

SpeciesSet RestrictedSpeciesService::getBannedFormesForPlayerPokemon() const {
    SpeciesSet r;
    for (Species* sp : allInclAltFormes_.toVector()) {
        if (!sp->isBaseForme() && (sp->baseNumber() == species::giratina || sp->baseNumber() == species::shaymin)) r.add(sp);
    }
    return r;
}

void RestrictedSpeciesService::setRestrictions(const std::optional<GenRestrictions>& restrictions) {
    if (restrictions) {
        SpeciesSet allIncl;
        SpeciesSet allNonRestricted = rh_.getSpeciesSetInclFormes();
        for (int gen = 1; gen <= GenRestrictions::MAX_GENERATION; gen++) {
            if (restrictions->isGenAllowed(gen))
                allIncl.addAll(allNonRestricted.filter([gen](Species* s) { return s->baseForme()->generation() == gen; }));
        }
        if (restrictions->allowEvolutionaryRelatives) allIncl.addFullFamilies(false);
        allInclAltFormes_ = SpeciesSet::unmodifiable(allIncl.toVector());
    } else {
        allInclAltFormes_ = SpeciesSet::unmodifiable(rh_.getSpeciesSetInclFormes().toVector());
    }
    nonLegendariesInclAltFormes_ = SpeciesSet::unmodifiable(allInclAltFormes_.filter([](Species* s) { return !s->isLegendary(); }).toVector());
    legendariesInclAltFormes_ = SpeciesSet::unmodifiable(allInclAltFormes_.filter([](Species* s) { return s->isLegendary(); }).toVector());
    ultraBeastsInclAltFormes_ = SpeciesSet::unmodifiable(allInclAltFormes_.filter([](Species* s) { return s->isUltraBeast(); }).toVector());
    SpeciesSet alt = rh_.getAltFormes();
    auto notAlt = [&alt](Species* s) { return !alt.contains(s); };
    all_ = SpeciesSet::unmodifiable(allInclAltFormes_.filter(notAlt).toVector());
    nonLegendaries_ = SpeciesSet::unmodifiable(nonLegendariesInclAltFormes_.filter(notAlt).toVector());
    legendaries_ = SpeciesSet::unmodifiable(legendariesInclAltFormes_.filter(notAlt).toVector());
    ultraBeasts_ = SpeciesSet::unmodifiable(ultraBeastsInclAltFormes_.filter(notAlt).toVector());
    megaEvolutions_ = SpeciesSet::unmodifiable(allInclAltFormes_.filter([](Species* s) { return s->isMegaEvolution(); }).toVector());
    restrictionsSet_ = true;
    for (Species* s : allInclAltFormes_.toVector()) s->saveOriginalData();
}

std::vector<Type> TypeService::getTypes() const { return rh_.getTypeTable().types(); }
bool TypeService::typeInGame(Type t) const {
    auto ts = getTypes();
    return std::find(ts.begin(), ts.end(), t) != ts.end();
}
Type TypeService::randomType(JavaRandom& random) const {
    auto ts = getTypes();
    return ts[random.nextInt(static_cast<int>(ts.size()))];
}

}  // namespace pr
