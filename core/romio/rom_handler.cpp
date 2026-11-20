#include "romio/rom_handler.hpp"

#include <stdexcept>

#include "constants/ability_ids.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "constants/global_constants.hpp"
#include "constants/item_ids.hpp"
#include "romio/services/restricted_species_service.hpp"

namespace pr {

RomHandler::RomHandler()
    : rPokeService_(std::make_unique<RestrictedSpeciesService>(*this)), typeService_(std::make_unique<TypeService>(*this)) {}
RomHandler::~RomHandler() = default;

namespace {
// Math.round de Java para double
int javaRound(double x) { return static_cast<int>(std::floor(x + 0.5)); }

int findEvolutionLevel(const std::vector<std::array<int, 3>>& samples, int targetPreBST, int targetPostBST) {
    const double p = 1, preFactor = 1.5, postFactor = 3, largeWeightForZero = 1;
    double weightedSum = 0.0, weightSum = 0.0;
    for (const auto& s : samples) {
        double dx = targetPreBST - s[0];
        double dy = targetPostBST - s[1];
        double dist = std::sqrt(dx * dx + dy * dy);
        double weight = dist == 0 ? largeWeightForZero : 1.0 / std::pow(dist, p);
        double scaledPre = std::pow(static_cast<double>(targetPreBST) / s[0], preFactor);
        double scaledPost = std::pow(static_cast<double>(targetPostBST) / s[1], postFactor);
        double adjustedLevel = s[2] * (scaledPre + scaledPost) / 2.0;
        weightedSum += adjustedLevel * weight;
        weightSum += weight;
    }
    return javaRound(weightedSum / weightSum);
}
}  // namespace

void RomHandler::estimateEvolutionLevels() {
    std::vector<EvolutionPtr> levelUpEvos, nonLevelUpEvos;
    for (Species* pk : getSpeciesSetInclFormes().toVector()) {
        for (const auto& evo : pk->evolutionsFrom()) {
            if (usesLevelThreshold(evo->type())) {
                // solo formas base, para no sobrerrepresentar líneas con formas
                if (pk->isBaseForme()) levelUpEvos.push_back(evo);
                evo->setEstimatedEvoLvl(evo->extraInfo());
                highestEvoLvl_ = std::max(highestEvoLvl_, evo->extraInfo());
            } else {
                nonLevelUpEvos.push_back(evo);
            }
        }
    }
    levelUpEstimationTriplets_.emplace();
    for (const auto& evo : levelUpEvos) {
        levelUpEstimationTriplets_->push_back({evo->from()->bst(true), evo->to()->bst(true), evo->extraInfo()});
    }
    calculateEstimatedLevels(nonLevelUpEvos);
}

void RomHandler::calculateEstimatedLevels(const std::vector<EvolutionPtr>& targetEvos) {
    for (const auto& evo : targetEvos) {
        evo->setEstimatedEvoLvl(
            findEvolutionLevel(*levelUpEstimationTriplets_, evo->from()->bst(false), evo->to()->bst(false)));
    }
    for (const auto& evo : targetEvos) {
        if (!evo->from()->evolutionsTo().empty()) {
            // al menos un 25 % más que la evolución anterior
            const auto& prev = evo->from()->evolutionsTo().front();
            evo->setEstimatedEvoLvl(
                std::max(evo->estimatedEvoLvl(), static_cast<int>(std::ceil(1.25 * prev->estimatedEvoLvl()))));
        }
        if (!evo->to()->evolutionsFrom().empty()) {
            // como mucho el 80 % de la siguiente
            for (const auto& next : evo->to()->evolutionsFrom()) {
                evo->setEstimatedEvoLvl(
                    std::min(evo->estimatedEvoLvl(), static_cast<int>(std::ceil(0.8 * next->estimatedEvoLvl()))));
            }
        }
        highestEvoLvl_ = std::max(highestEvoLvl_, evo->estimatedEvoLvl());
    }
}

void RomHandler::condenseLevelEvolutions(int maxLevel) {
    if (maxLevel >= getHighestEvoLvl()) return;
    int maxIntermediateLevel = static_cast<int>(std::ceil(0.75 * maxLevel));
    for (Species* pk : getSpeciesSetInclFormes().toVector()) {
        for (const auto& evo : pk->evolutionsFrom()) {
            bool intermediate = !evo->to()->evolutionsFrom().empty();
            if (usesLevelThreshold(evo->type())) {
                if (evo->extraInfo() > maxIntermediateLevel && intermediate) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(evo->type(), maxIntermediateLevel);
                } else if (evo->extraInfo() > maxLevel) {
                    markImprovedEvolutions(pk);
                    evo->updateEvolutionMethod(evo->type(), maxLevel);
                }
            } else {
                if (evo->estimatedEvoLvl() > maxIntermediateLevel && intermediate) {
                    markImprovedEvolutions(pk);
                    evo->setEstimatedEvoLvl(maxIntermediateLevel);
                } else if (evo->estimatedEvoLvl() > maxLevel) {
                    markImprovedEvolutions(pk);
                    evo->setEstimatedEvoLvl(maxLevel);
                }
            }
        }
    }
    highestEvoLvl_ = maxLevel;
}

void RomHandler::adjustEvolutionLevels() {
    if (!levelUpEstimationTriplets_) throw std::logic_error("levelUpEstimationTriplets has not been set");
    std::vector<EvolutionPtr> allEvos, levelUpEvos;
    for (Species* pk : getSpeciesSetInclFormes().toVector()) {
        for (const auto& evo : pk->evolutionsFrom()) {
            allEvos.push_back(evo);
            if (usesLevelThreshold(evo->type())) levelUpEvos.push_back(evo);
        }
    }
    calculateEstimatedLevels(allEvos);
    for (const auto& evo : levelUpEvos) evo->updateEvolutionMethod(evo->type(), evo->estimatedEvoLvl(), true);
}

void RomHandler::removeTimeBasedEvolutions() {
    for (Species* pk : getSpecies()) {
        if (!pk) continue;
        for (const auto& evo : pk->evolutionsFrom()) {
            EvolutionType et = evo->type();
            if (et == EvolutionType::LEVEL_DUSK) {
                markImprovedEvolutions(pk);
                evo->updateEvolutionMethod(EvolutionType::STONE, items::duskStone);
            } else if (usesTime(et)) {
                markImprovedEvolutions(pk);
                if (hadEvolutionOfType(pk, oppositeTime(et))) {
                    // pareja día/noche (p. ej. Eevee): se usan Piedra Solar / Lunar
                    int item = isDayType(et) ? items::sunStone : items::moonStone;
                    evo->updateEvolutionMethod(EvolutionType::STONE, item);
                } else {
                    evo->updateEvolutionMethod(*timeless(et), evo->extraInfo());
                }
            }
        }
    }
}

bool RomHandler::hadEvolutionOfType(Species* pk, std::optional<EvolutionType> et) const {
    auto it = preImprovedEvolutions_.find(pk->number());
    if (it == preImprovedEvolutions_.end())
        throw std::logic_error("Species should always have been added to preImprovedEvolutions.");
    return std::any_of(it->second.begin(), it->second.end(),
                       [&](const Evolution& e) { return et && e.type() == *et; });
}

void RomHandler::markImprovedEvolutions(Species* pk) {
    if (preImprovedEvolutions_.count(pk->number())) return;
    std::vector<Evolution> copy;
    for (const auto& e : pk->evolutionsFrom()) copy.push_back(*e);
    preImprovedEvolutions_[pk->number()] = std::move(copy);
}

SpeciesSet RomHandler::getMainGameWildPokemonSpecies(bool useTimeOfDay) const {
    SpeciesSet wild;
    for (const auto& area : getEncounters(useTimeOfDay)) {
        if (area.isPartiallyPostGame()) {
            for (int i = 0; i < area.partiallyPostGameCutoff; i++) wild.add(area.encounters.at(i)->species());
        } else if (!area.postGame) {
            for (const auto& e : area.encounters) wild.add(e->species());
        }
    }
    return wild;
}

ItemSet RomHandler::idsToItemSet(const std::vector<int>& ids) const {
    ItemSet s;
    const auto& items = getItems();
    for (int id : ids)
        if (id < static_cast<int>(items.size()) && items[id]) s.add(id);
    return s;
}
ItemSet RomHandler::getXItems() const { return idsToItemSet(global::xItems); }
ItemSet RomHandler::getRegularShopItems() const { return idsToItemSet(global::regularShopItems); }

ItemSet RomHandler::getAllowedItems() const {
    ItemSet s;
    for (const auto& it : getItems())
        if (it && it->allowed) s.add(it->id);
    return s;
}

ItemSet RomHandler::getNonBadItems() const {
    ItemSet s;
    getAllowedItems().forEach([&](const int& id) {
        if (!getItems()[id]->bad) s.add(id);
    });
    return s;
}

void RomHandler::setStarterTypeTriangle(const std::vector<Type>& t) {
    if (t.size() != 3) throw std::invalid_argument("Type triangle must contain three types!");
    starterTypeTriangle_ = t;
}

std::array<int, 4> RomHandler::getMovesAtLevel(const Species& pk,
                                               const std::map<int, std::vector<MoveLearnt>>& movesets,
                                               int level) const {
    std::array<int, 4> cur{};
    int count = 0;
    for (const auto& ml : movesets.at(pk.number())) {
        if (ml.level > level) break;
        bool known = false;
        for (int i = 0; i < count; i++) {
            if (cur[i] == ml.move) {
                known = true;
                break;
            }
        }
        if (!known) {
            if (count == 4) {
                cur[0] = cur[1];
                cur[1] = cur[2];
                cur[2] = cur[3];
                cur[3] = ml.move;
            } else {
                cur[count++] = ml.move;
            }
        }
    }
    return cur;
}

int RomHandler::getAbilityForTrainerPokemon(const TrainerPokemon& tp) const {
    if (abilitiesPerSpecies() == 0) throw std::logic_error("No abilities in this game.");
    if (tp.abilitySlot > abilitiesPerSpecies()) throw std::logic_error("tp.abilitySlot too high for this game.");
    // antes de randomizar, el hueco 0 significa "habilidad 1 o 2"
    if (tp.abilitySlot == 0) return abilities::undefined;
    Species* pk = !tp.species()->isBaseForme() && isTrainerPokemonUseBaseFormeAbilities() ? tp.species()->baseForme() : tp.species();
    int ab[3] = {pk->ability1, pk->ability2, pk->ability3};
    int slot = isTrainerPokemonAlwaysUseAbility1() ? 1 : tp.abilitySlot;
    return ab[slot - 1];
}

}  // namespace pr
