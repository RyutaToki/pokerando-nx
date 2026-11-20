// Estado de la sesión actual: ROM elegida, opciones, preset y semilla.
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "app/options_model.hpp"
#include "app/state.hpp"
#include "random/game_randomizer.hpp"
#include "romio/rom_detect.hpp"

namespace app {

struct Session {
    std::string romPath;
    pr::RomInfo info;
    std::shared_ptr<pr::RomHandler> preview;  // ROM cargada para consultar capacidades y especies
    RomCaps caps;
    OptState opts;
    std::string presetName = "Personalizado";
    std::string presetSettings;  // para saber si se ha cambiado algo desde que se cargó
    int presetShiny = 0;
    int64_t seed = 0;
    bool dirty = false;  // cambios sin guardar como preset
};
Session& session();
int64_t newSeed();

// aplica un preset a la sesión (settings string de FVX); devuelve false si no se pudo leer
bool applyPreset(const Preset& p);
bool applySettingsString(const std::string& s, std::string* error = nullptr);

struct JobResult {
    bool ok = false;
    std::string error;
    std::string romOut, logOut;
    double seconds = 0;
    std::vector<std::string> starters;
};

// Ejecuta la randomización (llamar desde un hilo aparte). onPhase se llama desde ese hilo.
JobResult runRandomization(const std::string& romPath, const OptState& opts, int64_t seed,
                           const std::function<void(pr::GameRandomizer::Phase)>& onPhase);

// fases que se mostrarán según las opciones activas
std::vector<pr::GameRandomizer::Phase> activePhases(const OptState& o);
const char* phaseName(pr::GameRandomizer::Phase p);

// nombre de archivo de salida: {carpeta}/randomizadas/{Juego}_{semilla}.gba
std::string outputPath(const std::string& romPath, const pr::RomInfo& info, int64_t seed);

}  // namespace app
