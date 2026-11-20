// Funciones de Gen4RomHandler todavía sin portar: lanzan una excepción si se usan.
// Generado por tools/gen4_stubs.py; se regenera al portar cada función.
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <stdexcept>

#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

void Gen4RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) { throw std::logic_error("Gen 4: pendiente: setEncounters"); }
std::vector<Species*> Gen4RomHandler::getStarters() const { throw std::logic_error("Gen 4: pendiente: getStarters"); }
bool Gen4RomHandler::setStarters(const std::vector<Species*>& starters) { throw std::logic_error("Gen 4: pendiente: setStarters"); }
bool Gen4RomHandler::supportsStarterHeldItems() const { throw std::logic_error("Gen 4: pendiente: supportsStarterHeldItems"); }
std::vector<std::optional<int>> Gen4RomHandler::getStarterHeldItems() const { throw std::logic_error("Gen 4: pendiente: getStarterHeldItems"); }
void Gen4RomHandler::setStarterHeldItems(const std::vector<std::optional<int>>& items) { throw std::logic_error("Gen 4: pendiente: setStarterHeldItems"); }
std::vector<StaticEncounter> Gen4RomHandler::getStaticPokemon() { throw std::logic_error("Gen 4: pendiente: getStaticPokemon"); }
bool Gen4RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& statics) { throw std::logic_error("Gen 4: pendiente: setStaticPokemon"); }
std::vector<int> Gen4RomHandler::getTMMoves() const { throw std::logic_error("Gen 4: pendiente: getTMMoves"); }
std::vector<int> Gen4RomHandler::getHMMoves() const { throw std::logic_error("Gen 4: pendiente: getHMMoves"); }
void Gen4RomHandler::setTMMoves(const std::vector<int>& moveIndexes) { throw std::logic_error("Gen 4: pendiente: setTMMoves"); }
int Gen4RomHandler::getTMCount() const { throw std::logic_error("Gen 4: pendiente: getTMCount"); }
int Gen4RomHandler::getHMCount() const { throw std::logic_error("Gen 4: pendiente: getHMCount"); }
std::map<int, std::vector<bool>> Gen4RomHandler::getTMHMCompatibility() const { throw std::logic_error("Gen 4: pendiente: getTMHMCompatibility"); }
void Gen4RomHandler::setTMHMCompatibility(const std::map<int, std::vector<bool>>& data) { throw std::logic_error("Gen 4: pendiente: setTMHMCompatibility"); }
bool Gen4RomHandler::hasMoveTutors() const { throw std::logic_error("Gen 4: pendiente: hasMoveTutors"); }
std::vector<int> Gen4RomHandler::getMoveTutorMoves() const { throw std::logic_error("Gen 4: pendiente: getMoveTutorMoves"); }
void Gen4RomHandler::setMoveTutorMoves(const std::vector<int>& moveIndexes) { throw std::logic_error("Gen 4: pendiente: setMoveTutorMoves"); }
std::map<int, std::vector<bool>> Gen4RomHandler::getMoveTutorCompatibility() const { throw std::logic_error("Gen 4: pendiente: getMoveTutorCompatibility"); }
void Gen4RomHandler::setMoveTutorCompatibility(const std::map<int, std::vector<bool>>& data) { throw std::logic_error("Gen 4: pendiente: setMoveTutorCompatibility"); }
ItemSet Gen4RomHandler::getEvolutionItems() const { throw std::logic_error("Gen 4: pendiente: getEvolutionItems"); }
ItemSet Gen4RomHandler::getOPShopItems() const { throw std::logic_error("Gen 4: pendiente: getOPShopItems"); }
ItemSet Gen4RomHandler::getAllConsumableHeldItems() const { throw std::logic_error("Gen 4: pendiente: getAllConsumableHeldItems"); }
ItemSet Gen4RomHandler::getRequiredFieldTMs() const { throw std::logic_error("Gen 4: pendiente: getRequiredFieldTMs"); }
std::vector<int> Gen4RomHandler::getFieldItems() const { throw std::logic_error("Gen 4: pendiente: getFieldItems"); }
void Gen4RomHandler::setFieldItems(const std::vector<int>& fieldItems) { throw std::logic_error("Gen 4: pendiente: setFieldItems"); }
bool Gen4RomHandler::hasShopSupport() const { throw std::logic_error("Gen 4: pendiente: hasShopSupport"); }
bool Gen4RomHandler::canChangeShopSizes() const { throw std::logic_error("Gen 4: pendiente: canChangeShopSizes"); }
std::vector<Shop> Gen4RomHandler::getShops() const { throw std::logic_error("Gen 4: pendiente: getShops"); }
void Gen4RomHandler::setShops(const std::vector<Shop>& shops) { throw std::logic_error("Gen 4: pendiente: setShops"); }
std::vector<int> Gen4RomHandler::getShopPrices() const { throw std::logic_error("Gen 4: pendiente: getShopPrices"); }
void Gen4RomHandler::setShopPrices(const std::vector<int>& prices) { throw std::logic_error("Gen 4: pendiente: setShopPrices"); }
void Gen4RomHandler::setBalancedShopPrices() { throw std::logic_error("Gen 4: pendiente: setBalancedShopPrices"); }
std::vector<PickupItem> Gen4RomHandler::getPickupItems() { throw std::logic_error("Gen 4: pendiente: getPickupItems"); }
void Gen4RomHandler::setPickupItems(const std::vector<PickupItem>& items) { throw std::logic_error("Gen 4: pendiente: setPickupItems"); }
std::vector<InGameTrade> Gen4RomHandler::getInGameTrades() const { throw std::logic_error("Gen 4: pendiente: getInGameTrades"); }
void Gen4RomHandler::setInGameTrades(const std::vector<InGameTrade>& trades) { throw std::logic_error("Gen 4: pendiente: setInGameTrades"); }
std::vector<std::optional<int>> Gen4RomHandler::getSensibleHeldItemsFor(const TrainerPokemon& tp, bool consumableOnly,
                                                            const std::array<int, 4>& pokeMoves) const { throw std::logic_error("Gen 4: pendiente: getSensibleHeldItemsFor"); }
ItemSet Gen4RomHandler::getAllHeldItems() const { throw std::logic_error("Gen 4: pendiente: getAllHeldItems"); }
std::vector<int> Gen4RomHandler::getDoublesTrainerClasses() const { throw std::logic_error("Gen 4: pendiente: getDoublesTrainerClasses"); }
std::vector<int> Gen4RomHandler::getMainGameLegendaries() const { throw std::logic_error("Gen 4: pendiente: getMainGameLegendaries"); }
std::vector<int> Gen4RomHandler::getSpecialMusicStatics() const { throw std::logic_error("Gen 4: pendiente: getSpecialMusicStatics"); }
bool Gen4RomHandler::hasStaticMusicFix() const { throw std::logic_error("Gen 4: pendiente: hasStaticMusicFix"); }
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
