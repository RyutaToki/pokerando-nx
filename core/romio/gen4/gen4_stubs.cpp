// Funciones de Gen4RomHandler todavía sin portar: lanzan una excepción si se usan.
// Generado por tools/gen4_stubs.py; se regenera al portar cada función.
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <stdexcept>

#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

void Gen4RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) { throw std::logic_error("Gen 4: pendiente: setEncounters"); }
std::vector<int> Gen4RomHandler::getDoublesTrainerClasses() const { throw std::logic_error("Gen 4: pendiente: getDoublesTrainerClasses"); }
void Gen4RomHandler::removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) { throw std::logic_error("Gen 4: pendiente: removeImpossibleEvolutions"); }
void Gen4RomHandler::makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) { throw std::logic_error("Gen 4: pendiente: makeEvolutionsEasier"); }
void Gen4RomHandler::removeEvosForPokemonPool(const SpeciesSet& included) { throw std::logic_error("Gen 4: pendiente: removeEvosForPokemonPool"); }
void Gen4RomHandler::applyMiscTweak(const MiscTweak& t) { throw std::logic_error("Gen 4: pendiente: applyMiscTweak"); }
bool Gen4RomHandler::setIntroPokemon(Species* pk) { throw std::logic_error("Gen 4: pendiente: setIntroPokemon"); }
bool Gen4RomHandler::setCatchingTutorial(Species* opponent, Species* player) { throw std::logic_error("Gen 4: pendiente: setCatchingTutorial"); }
void Gen4RomHandler::setPCPotionItem(int item) { throw std::logic_error("Gen 4: pendiente: setPCPotionItem"); }
void Gen4RomHandler::enableGuaranteedPokemonCatching() { throw std::logic_error("Gen 4: pendiente: enableGuaranteedPokemonCatching"); }
void Gen4RomHandler::applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) { throw std::logic_error("Gen 4: pendiente: applyCorrectStaticMusic"); }
std::vector<int> Gen4RomHandler::getFieldMoves() const { throw std::logic_error("Gen 4: pendiente: getFieldMoves"); }
std::vector<int> Gen4RomHandler::getEarlyRequiredHMMoves() const { throw std::logic_error("Gen 4: pendiente: getEarlyRequiredHMMoves"); }
std::string Gen4RomHandler::getPaletteFilesID() const { throw std::logic_error("Gen 4: pendiente: getPaletteFilesID"); }

}  // namespace pr::gen4
