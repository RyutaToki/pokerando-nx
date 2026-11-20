// Detección rápida de ROMs para el explorador (solo lee la cabecera) y apertura completa.
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "romio/rom_handler.hpp"

namespace pr {

struct RomInfo {
    bool recognized = false;  // es un juego Pokémon conocido
    bool supported = false;   // esta versión de la app puede randomizarlo
    std::string platform;     // "GBA" o "NDS"
    std::string code;         // código del juego, p. ej. "BPES"
    int generation = 0;
    std::string gameName;     // "Pokémon Esmeralda"
    std::string shortName;    // "Esmeralda" (para el nombre del archivo de salida)
    std::string region;       // "España"
    std::optional<uint32_t> expectedCRC32;  // CRC de la ROM original conocida
    uint64_t size = 0;
    std::string note;         // motivo si no es compatible
};

RomInfo detectRom(const std::string& path);
uint32_t fileCRC32(const std::string& path);
// Carga la ROM completa; lanza std::runtime_error si no es compatible.
std::unique_ptr<RomHandler> openRom(const std::string& path);

}  // namespace pr
