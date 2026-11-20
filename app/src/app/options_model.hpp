// Traducción entre los controles de la pantalla de Opciones (§7 de la especificación) y los
// campos de Settings de FVX. La fuente de verdad siempre es un Settings, así una settings
// string pegada de FVX se ve reflejada en los controles.
#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "romio/rom_handler.hpp"
#include "settings/settings.hpp"

namespace app {

// lo que la app guarda además de Settings
struct OptState {
    pr::Settings s;
    int shiny = 0;          // índice en kShinyOdds
    bool spoilerLog = true;
};

// probabilidades de shiny (Extras): umbral de (TID^SID^PIDh^PIDl) < umbral
struct ShinyOdds {
    const char* label;
    int threshold;  // 8 = original
};
inline const ShinyOdds kShinyOdds[] = {{"1/8192", 8}, {"1/4096", 16}, {"1/1024", 64}, {"1/256", 256}, {"1/64", 1024}, {"1/8", 8192}};

// capacidades del juego cargado
struct RomCaps {
    int gen = 3;
    bool abilities = true, tutors = true, statics = true, split = false, shops = true, typeTable = true, palettes = true;
    bool fourStartingMoves = true, triangle = true, bossPokemon = true, bossItems = true, trainerText = true;
    int tweaks = 0;
    bool shinyPatch = false;
    static RomCaps from(const pr::RomHandler& rh);
};

struct ModeDef {
    std::string label;
    std::vector<std::string> names;
    std::function<int(const OptState&)> get;
    std::function<void(OptState&, int)> set;
};

struct ToggleDef {
    std::string label;
    std::function<bool(const RomCaps&)> available;
    std::function<bool(const OptState&)> enabled;  // depende del modo u otras opciones
    std::function<bool(const OptState&)> get;
    std::function<void(OptState&, bool)> set;
};

struct SliderDef {
    std::string label;
    std::function<bool(const OptState&)> enabled;
    std::function<int(const OptState&)> get;
    std::function<void(OptState&, int)> set;
};

struct TabDef {
    std::string title;
    bool exclusive = false;
    std::function<bool(const RomCaps&)> available;
    std::optional<ModeDef> mode;
    bool shinyPanel = false;
    std::vector<ToggleDef> toggles;
    std::optional<SliderDef> slider;
    std::function<void(OptState&)> reset;  // «Restablecer pestaña»
};

const std::vector<TabDef>& tabs();

// presets de serie
pr::Settings defaultSettings();

}  // namespace app
