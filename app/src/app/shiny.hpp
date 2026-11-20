// Parche de probabilidad de shiny (pestaña Extras, exclusivo de esta app).
#pragma once
#include <string>

#include "romio/rom_handler.hpp"

namespace app {
// ¿Sabemos dónde está la comprobación de shiny en este juego y región?
bool shinyPatchAvailable(const pr::RomHandler& rh);
// Parchea el archivo ya guardado. threshold: 8 = original, 16 = 1/4096 ... 8192 = 1/8.
bool applyShinyPatch(const std::string& romFile, const pr::RomHandler& rh, int threshold, std::string* error);
}  // namespace app
