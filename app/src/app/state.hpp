// Configuración, presets e historial de la app (sdmc:/switch/pokerandonx/, §9 de la especificación).
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace app {

// carpetas
std::string dataDir();       // sdmc:/switch/pokerandonx/
std::string defaultRomDir();  // sdmc:/roms/ o sdmc:/
std::string sdRoot();
// ruta para mostrar: en el PC, la carpeta de usuario se muestra como «sdmc:/» (igual que en la consola)
std::string displayPath(const std::string& path);
bool exists(const std::string& path);
bool isDir(const std::string& path);
bool makeDirs(const std::string& path);
std::string parentDir(const std::string& path);
std::string fileName(const std::string& path);
// espacio libre de la SD en bytes (0 si no se sabe)
uint64_t freeSpace(const std::string& path);
std::string formatSize(uint64_t bytes);
std::string nowString();  // "2026-11-20 14:05"

struct Config {
    int accent = 0;
    std::string lastDir;
    bool startAtLast = true;  // carpeta inicial del explorador: la última usada o sdmc:/roms/
    std::vector<std::string> favorites;
    bool hideUnsupported = false;
    std::string gbaEmulator = "mgba";       // mgba | retroarch
    std::string ndsEmulator = "melonds";    // melonds | retroarch
    bool keepRandomized = false;  // conservar las ROMs randomizadas anteriores (por defecto se borran)
    bool spoilerLog = true;
    std::string language = "es";
};
Config& config();
void loadConfig();
void saveConfig();

struct Preset {
    std::string name;
    std::string game;  // nombre del juego para el que se creó ("" = cualquiera)
    std::string date;
    std::string settings;  // settings string de FVX
    int shiny = 0;         // índice en las fichas de shiny (0 = 1/8192)
    bool builtin = false;
    std::string file;  // ruta del .json (vacío en los de serie)
};
std::vector<Preset> loadPresets();  // primero los de serie
bool savePreset(Preset& p);          // escribe presets/<nombre>.json
bool deletePreset(const Preset& p);

struct HistoryEntry {
    std::string game, shortName, platform, preset, date, romPath, logPath;
    int64_t seed = 0;
    std::vector<std::string> starters;
    double seconds = 0;
};
std::vector<HistoryEntry>& history();  // la más reciente primero
void loadHistory();
void saveHistory();

}  // namespace app
