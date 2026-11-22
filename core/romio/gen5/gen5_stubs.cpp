// Funciones de Gen5RomHandler todavía sin portar: lanzan una excepción si se usan.
// Generado por tools/gen5_stubs.py; se regenera al portar cada función.
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <stdexcept>

#include "romio/gen5/gen5_rom_handler.hpp"

namespace pr::gen5 {

std::vector<StaticEncounter> Gen5RomHandler::getStaticPokemon() { throw std::logic_error("Gen 5: pendiente: getStaticPokemon"); }
bool Gen5RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& statics) { throw std::logic_error("Gen 5: pendiente: setStaticPokemon"); }

}  // namespace pr::gen5
