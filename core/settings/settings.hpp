// Port de Settings (opciones del randomizer). El formato de texto (Base64 + CRC32) es el mismo
// que el de FVX 1.6.3, así que las "settings strings" y los presets son compatibles.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/gen_restrictions.hpp"
#include "gamedata/trainer.hpp"
#include "gamedata/types.hpp"

namespace pr {

class RomHandler;

struct Settings {
    static constexpr int VERSION = 429;  // FVX 1.6.3
    static constexpr int LENGTH_OF_SETTINGS_DATA = 69;
    static constexpr int LENGTH_OF_NAME_LENGTH = 1;
    static constexpr int LENGTH_OF_CHECKSUM = 4;
    static constexpr int LENGTH_OF_END_PADDING = 4;
    static constexpr int MAKE_EVOLUTIONS_EASIER_DEFAULT_LVL = 40;

#include "settings/settings_fields.inc"

    // "429" + Base64
    std::string toString() const;
    // Lanza std::invalid_argument si el texto no es válido o es de otra versión
    static Settings fromString(const std::string& withVersion);

    struct TweakForROMFeedback {
        bool changedStarter = false;
        bool removedCodeTweaks = false;
    };
    // Ajusta las opciones a lo que permite el juego cargado (Settings.tweakForRom)
    TweakForROMFeedback tweakForRom(const RomHandler& rh);

    bool useEstimatedLevelsForEvolutionImprovements() const { return estimateLevelForEvolutionImprovements; }

private:
    std::string toStringWithoutVersion() const;
    static Settings fromStringWithoutVersion(const std::string& s);
};

}  // namespace pr
