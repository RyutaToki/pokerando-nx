#include "app/state.hpp"

#include <sys/stat.h>
#include <sys/statvfs.h>

#include <borealis/extern/nlohmann/json.hpp>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>

namespace app {

namespace fs = std::filesystem;
using json = nlohmann::json;

std::string sdRoot() {
#ifdef __SWITCH__
    return "sdmc:/";
#else
    const char* home = std::getenv("HOME");
    return std::string(home ? home : "/tmp") + "/";
#endif
}

std::string displayPath(const std::string& path) {
#ifdef __SWITCH__
    return path;
#else
    std::string root = sdRoot();
    if (path.compare(0, root.size(), root) == 0) return "sdmc:/" + path.substr(root.size());
    return path;
#endif
}

std::string dataDir() {
#ifdef __SWITCH__
    return "sdmc:/switch/pokerandonx/";
#else
    return sdRoot() + ".local/share/pokerandonx/";
#endif
}

std::string defaultRomDir() {
    std::string r = sdRoot() + "roms/";
    return isDir(r) ? r : sdRoot();
}

bool exists(const std::string& p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0;
}
bool isDir(const std::string& p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}
bool makeDirs(const std::string& p) {
    std::error_code ec;
    fs::create_directories(p, ec);
    return isDir(p);
}
std::string parentDir(const std::string& p) {
    std::string s = p;
    while (s.size() > 1 && s.back() == '/') s.pop_back();
    size_t i = s.find_last_of('/');
    if (i == std::string::npos) return s;
    return s.substr(0, i + 1);
}
std::string fileName(const std::string& p) {
    size_t i = p.find_last_of('/');
    return i == std::string::npos ? p : p.substr(i + 1);
}

uint64_t freeSpace(const std::string& path) {
    struct statvfs st;
    if (::statvfs(path.c_str(), &st) != 0) return 0;
    return static_cast<uint64_t>(st.f_bavail) * st.f_frsize;
}

std::string formatSize(uint64_t b) {
    char buf[32];
    if (b >= (1ull << 30)) std::snprintf(buf, sizeof buf, "%.1f GB", b / double(1ull << 30));
    else if (b >= (1ull << 20)) std::snprintf(buf, sizeof buf, "%.1f MB", b / double(1ull << 20));
    else if (b >= 1024) std::snprintf(buf, sizeof buf, "%.0f KB", b / 1024.0);
    else std::snprintf(buf, sizeof buf, "%llu B", static_cast<unsigned long long>(b));
    std::string s = buf;
    for (char& c : s)
        if (c == '.') c = ',';
    return s;
}

std::string nowString() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M", std::localtime(&t));
    return buf;
}

static json parseFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return json();
    try {
        return json::parse(f);
    } catch (...) {
        return json();
    }
}

// Si quedó un .tmp válido (escritura que no llegó a renombrarse: en la Switch rename() no puede
// sustituir un archivo que ya existe), es la versión más reciente: se usa esa.
static json readJson(const std::string& path) {
    json t = parseFile(path + ".tmp");
    if (!t.is_null()) return t;
    return parseFile(path);
}

static bool writeJson(const std::string& path, const json& j) {
    makeDirs(parentDir(path));
    std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::trunc);
        if (!f) return false;
        f << j.dump(2);
        if (!f) return false;
    }
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        // en la Switch no se puede renombrar encima de un archivo existente: se borra antes
        ec.clear();
        fs::remove(path, ec);
        ec.clear();
        fs::rename(tmp, path, ec);
    }
    return !ec;
}

// ---------- configuración ----------
Config& config() {
    static Config c;
    return c;
}

void loadConfig() {
    json j = readJson(dataDir() + "config.json");
    if (!j.is_object()) return;
    Config& c = config();
    c.accent = j.value("accent", c.accent);
    c.lastDir = j.value("lastDir", c.lastDir);
    c.startAtLast = j.value("startAtLast", c.startAtLast);
    c.favorites = j.value("favorites", c.favorites);
    c.hideUnsupported = j.value("hideUnsupported", c.hideUnsupported);
    c.gbaEmulator = j.value("gbaEmulator", c.gbaEmulator);
    c.ndsEmulator = j.value("ndsEmulator", c.ndsEmulator);
    c.keepRandomized = j.value("keepRandomized", c.keepRandomized);
    c.spoilerLog = j.value("spoilerLog", c.spoilerLog);
    c.language = j.value("language", c.language);
}

void saveConfig() {
    const Config& c = config();
    json j = {{"accent", c.accent},           {"lastDir", c.lastDir}, {"startAtLast", c.startAtLast},         {"favorites", c.favorites},
              {"hideUnsupported", c.hideUnsupported}, {"gbaEmulator", c.gbaEmulator}, {"ndsEmulator", c.ndsEmulator},
              {"keepRandomized", c.keepRandomized}, {"spoilerLog", c.spoilerLog}, {"language", c.language}};
    writeJson(dataDir() + "config.json", j);
}

// ---------- presets ----------
std::vector<Preset> builtinPresets();  // presets_builtin.cpp

static std::string safeFileName(const std::string& name) {
    std::string r;
    for (char c : name) r += (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') ? '_' : c;
    return r.empty() ? "preset" : r;
}

std::vector<Preset> loadPresets() {
    std::vector<Preset> out = builtinPresets();
    std::string dir = dataDir() + "presets/";
    std::vector<Preset> user;
    std::error_code ec;
    if (isDir(dir)) {
        for (const auto& e : fs::directory_iterator(dir, ec)) {
            if (e.path().extension() != ".json") continue;
            json j = readJson(e.path().string());
            if (!j.is_object()) continue;
            Preset p;
            p.name = j.value("name", e.path().stem().string());
            p.game = j.value("game", "");
            p.date = j.value("date", "");
            p.settings = j.value("settings", "");
            p.shiny = j.value("shiny", 0);
            p.file = e.path().string();
            user.push_back(p);
        }
    }
    std::sort(user.begin(), user.end(), [](const Preset& a, const Preset& b) { return a.date > b.date; });
    out.insert(out.end(), user.begin(), user.end());
    return out;
}

bool savePreset(Preset& p) {
    if (p.date.empty()) p.date = nowString();
    if (p.file.empty()) p.file = dataDir() + "presets/" + safeFileName(p.name) + ".json";
    json j = {{"name", p.name}, {"game", p.game}, {"date", p.date}, {"settings", p.settings}, {"shiny", p.shiny}};
    return writeJson(p.file, j);
}

bool deletePreset(const Preset& p) {
    if (p.builtin || p.file.empty()) return false;
    std::error_code ec;
    return fs::remove(p.file, ec);
}

// ---------- historial ----------
std::vector<HistoryEntry>& history() {
    static std::vector<HistoryEntry> h;
    return h;
}

void loadHistory() {
    json j = readJson(dataDir() + "history.json");
    auto& h = history();
    h.clear();
    if (!j.is_array()) return;
    for (const auto& e : j) {
        HistoryEntry x;
        x.game = e.value("game", "");
        x.shortName = e.value("shortName", "");
        x.platform = e.value("platform", "GBA");
        x.preset = e.value("preset", "");
        x.date = e.value("date", "");
        x.romPath = e.value("romPath", "");
        x.logPath = e.value("logPath", "");
        x.seed = e.value("seed", int64_t(0));
        x.starters = e.value("starters", std::vector<std::string>{});
        x.seconds = e.value("seconds", 0.0);
        h.push_back(x);
    }
}

void saveHistory() {
    json j = json::array();
    for (const auto& x : history())
        j.push_back({{"game", x.game}, {"shortName", x.shortName}, {"platform", x.platform}, {"preset", x.preset},
                     {"date", x.date}, {"romPath", x.romPath}, {"logPath", x.logPath}, {"seed", x.seed},
                     {"starters", x.starters}, {"seconds", x.seconds}});
    writeJson(dataDir() + "history.json", j);
}

}  // namespace app
