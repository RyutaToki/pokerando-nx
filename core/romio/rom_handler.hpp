// Base común de los manejadores de ROM: port de la interfaz RomHandler y de
// AbstractRomHandler de FVX. Cada generación implementa las partes virtuales.
#pragma once
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/evolution.hpp"
#include "gamedata/move.hpp"
#include "gamedata/species.hpp"
#include "gamedata/trainer.hpp"
#include "gamedata/types.hpp"

namespace pr {

class RomHandler {
public:
    virtual ~RomHandler() = default;

    // --- información de la ROM ---
    virtual std::string romName() const = 0;
    virtual const std::string& romCode() const = 0;
    virtual int romType() const = 0;
    virtual int generationOfPokemon() const = 0;
    virtual bool isRomValid() const = 0;

    // --- Pokémon ---
    // lista con posición 0 vacía (nullptr), como getSpecies() en Java
    virtual const std::vector<Species*>& getSpecies() const = 0;
    virtual const std::vector<Species*>& getSpeciesInclFormes() const = 0;
    // SpeciesSet.unmodifiable(getSpecies()): una copia nueva en cada llamada, como en Java
    SpeciesSet getSpeciesSet() const { return SpeciesSet::unmodifiable(getSpecies()); }
    SpeciesSet getSpeciesSetInclFormes() const { return SpeciesSet::unmodifiable(getSpeciesInclFormes()); }

    // --- movimientos ---
    virtual const std::vector<std::optional<Move>>& getMoves() const = 0;
    int getPerfectAccuracy() const { return perfectAccuracy_; }
    virtual std::map<int, std::vector<MoveLearnt>> getMovesLearnt() const = 0;
    virtual void setMovesLearnt(const std::map<int, std::vector<MoveLearnt>>& movesets) = 0;
    std::array<int, 4> getMovesAtLevel(const Species& pk, const std::map<int, std::vector<MoveLearnt>>& movesets,
                                       int level) const;

    // --- entrenadores ---
    const std::vector<std::unique_ptr<TrainerClass>>& getTrainerClasses() const { return trainerClasses_; }
    const std::vector<TrainerPtr>& getTrainers() const { return trainers_; }
    std::vector<TrainerPtr>& getTrainers() { return trainers_; }

    // --- evoluciones (AbstractRomHandler) ---
    int getHighestEvoLvl() const { return highestEvoLvl_; }
    void condenseLevelEvolutions(int maxLevel);
    void adjustEvolutionLevels();
    void removeTimeBasedEvolutions();
    // TreeMap<Species, List<Evolution>> ordenado por número
    const std::map<int, std::vector<Evolution>>& getPreImprovedEvolutions() const { return preImprovedEvolutions_; }

protected:
    void estimateEvolutionLevels();
    void markImprovedEvolutions(Species* pk);
    bool hadEvolutionOfType(Species* pk, std::optional<EvolutionType> et) const;

    int perfectAccuracy_ = 100;
    std::vector<std::unique_ptr<TrainerClass>> trainerClasses_;
    std::vector<TrainerPtr> trainers_;

private:
    void calculateEstimatedLevels(const std::vector<EvolutionPtr>& targetEvos);

    int highestEvoLvl_ = 0;
    std::optional<std::vector<std::array<int, 3>>> levelUpEstimationTriplets_;
    std::map<int, std::vector<Evolution>> preImprovedEvolutions_;
};

}  // namespace pr
