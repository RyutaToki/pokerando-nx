// Funciones de Gen4RomHandler todavía sin portar: lanzan una excepción si se usan.
// Generado por tools/gen4_stubs.py; se regenera al portar cada función.
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <stdexcept>

#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

void Gen4RomHandler::setEncounters(bool useTimeOfDay, const std::vector<EncounterArea>& areas) { throw std::logic_error("Gen 4: pendiente: setEncounters"); }

}  // namespace pr::gen4
