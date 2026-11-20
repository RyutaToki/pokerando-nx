// Parche de probabilidad de shiny (pestaña Extras, exclusivo de esta app).
//
// En Gen 3 un Pokémon es shiny si (TID ^ SID ^ PID_alto ^ PID_bajo) < 8. El código compilado hace
// tres EOR, «cmp rX, #7» y un salto bls/bhi. Como 8192 no cabe en el inmediato de cmp (máx. 255),
// se cambia «cmp rX, #7» por «lsrs rX, rX, #n» (Z = 1 si el valor < 2^n) y bls/bhi por beq/bne.
// El registro no se vuelve a usar después en ninguno de los puntos (comprobado en Esmeralda).
#pragma once
#include <string>

namespace app {
// ¿Sabemos dónde está la comprobación de shiny en esta ROM? (lee el archivo; el resultado se guarda)
bool shinyPatchAvailable(const std::string& romFile);
// Parchea el archivo ya guardado. threshold: 8 = original, 16 = 1/4096 ... 8192 = 1/8.
bool applyShinyPatch(const std::string& romFile, int threshold, std::string* error);
}  // namespace app
