#include "app/shiny.hpp"

namespace app {
// TODO: localizar la comparación en cada juego/región (ver especificación §7.10).
bool shinyPatchAvailable(const pr::RomHandler&) { return false; }
bool applyShinyPatch(const std::string&, const pr::RomHandler&, int, std::string* error) {
    if (error) *error = "El parche de shiny no está disponible para este juego.";
    return false;
}
}  // namespace app
