// Funciones de Gen5RomHandler todavía sin portar: lanzan una excepción si se usan.
// Generado por tools/gen5_stubs.py; se regenera al portar cada función.
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <stdexcept>

#include "romio/gen5/gen5_rom_handler.hpp"

namespace pr::gen5 {

std::vector<EncounterArea> Gen5RomHandler::getEncounters(bool useTimeOfDay) const { throw std::logic_error("Gen 5: pendiente: getEncounters"); }
std::vector<EncounterArea> Gen5RomHandler::getSortedEncounters(bool useTimeOfDay) const { throw std::logic_error("Gen 5: pendiente: getSortedEncounters"); }
void Gen5RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) { throw std::logic_error("Gen 5: pendiente: setEncounters"); }
SpeciesSet Gen5RomHandler::getBannedForWildEncounters() const { throw std::logic_error("Gen 5: pendiente: getBannedForWildEncounters"); }
SpeciesSet Gen5RomHandler::getBannedFormesForTrainerPokemon() const { throw std::logic_error("Gen 5: pendiente: getBannedFormesForTrainerPokemon"); }
std::vector<Species*> Gen5RomHandler::getStarters() const { throw std::logic_error("Gen 5: pendiente: getStarters"); }
bool Gen5RomHandler::setStarters(const std::vector<Species*>& starters) { throw std::logic_error("Gen 5: pendiente: setStarters"); }
std::vector<StaticEncounter> Gen5RomHandler::getStaticPokemon() { throw std::logic_error("Gen 5: pendiente: getStaticPokemon"); }
bool Gen5RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& statics) { throw std::logic_error("Gen 5: pendiente: setStaticPokemon"); }
std::vector<int> Gen5RomHandler::getTMMoves() const { throw std::logic_error("Gen 5: pendiente: getTMMoves"); }
std::vector<int> Gen5RomHandler::getHMMoves() const { throw std::logic_error("Gen 5: pendiente: getHMMoves"); }
void Gen5RomHandler::setTMMoves(const std::vector<int>& moveIndexes) { throw std::logic_error("Gen 5: pendiente: setTMMoves"); }
int Gen5RomHandler::getTMCount() const { throw std::logic_error("Gen 5: pendiente: getTMCount"); }
int Gen5RomHandler::getHMCount() const { throw std::logic_error("Gen 5: pendiente: getHMCount"); }
std::map<int, std::vector<bool>> Gen5RomHandler::getTMHMCompatibility() const { throw std::logic_error("Gen 5: pendiente: getTMHMCompatibility"); }
void Gen5RomHandler::setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) { throw std::logic_error("Gen 5: pendiente: setTMHMCompatibility"); }
bool Gen5RomHandler::hasMoveTutors() const { throw std::logic_error("Gen 5: pendiente: hasMoveTutors"); }
std::vector<int> Gen5RomHandler::getMoveTutorMoves() const { throw std::logic_error("Gen 5: pendiente: getMoveTutorMoves"); }
void Gen5RomHandler::setMoveTutorMoves(const std::vector<int>& moveIndexes) { throw std::logic_error("Gen 5: pendiente: setMoveTutorMoves"); }
std::map<int, std::vector<bool>> Gen5RomHandler::getMoveTutorCompatibility() const { throw std::logic_error("Gen 5: pendiente: getMoveTutorCompatibility"); }
void Gen5RomHandler::setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) { throw std::logic_error("Gen 5: pendiente: setMoveTutorCompatibility"); }
ItemSet Gen5RomHandler::getEvolutionItems() const { throw std::logic_error("Gen 5: pendiente: getEvolutionItems"); }
ItemSet Gen5RomHandler::getAllConsumableHeldItems() const { throw std::logic_error("Gen 5: pendiente: getAllConsumableHeldItems"); }
ItemSet Gen5RomHandler::getRequiredFieldTMs() const { throw std::logic_error("Gen 5: pendiente: getRequiredFieldTMs"); }
std::vector<int> Gen5RomHandler::getFieldItems() const { throw std::logic_error("Gen 5: pendiente: getFieldItems"); }
void Gen5RomHandler::setFieldItems(const std::vector<int>& fieldItems) { throw std::logic_error("Gen 5: pendiente: setFieldItems"); }
std::vector<Shop> Gen5RomHandler::getShops() const { throw std::logic_error("Gen 5: pendiente: getShops"); }
void Gen5RomHandler::setShops(const std::vector<Shop>& shops) { throw std::logic_error("Gen 5: pendiente: setShops"); }
std::vector<int> Gen5RomHandler::getShopPrices() const { throw std::logic_error("Gen 5: pendiente: getShopPrices"); }
void Gen5RomHandler::setShopPrices(const std::vector<int>& prices) { throw std::logic_error("Gen 5: pendiente: setShopPrices"); }
void Gen5RomHandler::setBalancedShopPrices() { throw std::logic_error("Gen 5: pendiente: setBalancedShopPrices"); }
std::vector<PickupItem> Gen5RomHandler::getPickupItems() { throw std::logic_error("Gen 5: pendiente: getPickupItems"); }
void Gen5RomHandler::setPickupItems(const std::vector<PickupItem>& items) { throw std::logic_error("Gen 5: pendiente: setPickupItems"); }
std::vector<InGameTrade> Gen5RomHandler::getInGameTrades() const { throw std::logic_error("Gen 5: pendiente: getInGameTrades"); }
void Gen5RomHandler::setInGameTrades(const std::vector<InGameTrade>& trades) { throw std::logic_error("Gen 5: pendiente: setInGameTrades"); }
std::vector<std::optional<int>> Gen5RomHandler::getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                            const std::array<int, 4>& pokeMoves) const { throw std::logic_error("Gen 5: pendiente: getSensibleHeldItemsFor"); }
ItemSet Gen5RomHandler::getAllHeldItems() const { throw std::logic_error("Gen 5: pendiente: getAllHeldItems"); }
bool Gen5RomHandler::hasStaticMusicFix() const { throw std::logic_error("Gen 5: pendiente: hasStaticMusicFix"); }
TypeTable Gen5RomHandler::getTypeTable() const { throw std::logic_error("Gen 5: pendiente: getTypeTable"); }
void Gen5RomHandler::setTypeTable(const TypeTable& t) { throw std::logic_error("Gen 5: pendiente: setTypeTable"); }
void Gen5RomHandler::removeImpossibleEvolutions(bool changeMoveEvos, bool useEstimatedLevels) { throw std::logic_error("Gen 5: pendiente: removeImpossibleEvolutions"); }
void Gen5RomHandler::makeEvolutionsEasier(bool changeWithOtherEvos, bool useEstimatedLevels) { throw std::logic_error("Gen 5: pendiente: makeEvolutionsEasier"); }
void Gen5RomHandler::removeEvosForPokemonPool(const SpeciesSet& included) { throw std::logic_error("Gen 5: pendiente: removeEvosForPokemonPool"); }
int Gen5RomHandler::miscTweaksAvailable() const { throw std::logic_error("Gen 5: pendiente: miscTweaksAvailable"); }
void Gen5RomHandler::applyMiscTweak(const MiscTweak& t) { throw std::logic_error("Gen 5: pendiente: applyMiscTweak"); }
bool Gen5RomHandler::setIntroPokemon(Species* pk) { throw std::logic_error("Gen 5: pendiente: setIntroPokemon"); }
bool Gen5RomHandler::setCatchingTutorial(Species* opponent, Species* player) { throw std::logic_error("Gen 5: pendiente: setCatchingTutorial"); }
void Gen5RomHandler::setPCPotionItem(int item) { throw std::logic_error("Gen 5: pendiente: setPCPotionItem"); }
void Gen5RomHandler::enableGuaranteedPokemonCatching() { throw std::logic_error("Gen 5: pendiente: enableGuaranteedPokemonCatching"); }
void Gen5RomHandler::applyCorrectStaticMusic(const std::vector<std::pair<int, int>>& changes) { throw std::logic_error("Gen 5: pendiente: applyCorrectStaticMusic"); }

}  // namespace pr::gen5
