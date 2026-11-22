#include "app/shiny.hpp"

#include <cstdint>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

namespace app {

namespace {

struct Site {
    size_t cmp;     // offset de «cmp rX, #7»
    size_t branch;  // offset del bls/bhi
};

// Juegos verificados a mano (código de 3 letras, cualquier región) y nº de comparaciones esperadas.
// Esmeralda: CreateBoxMon, GetMonSpritePalFromSpeciesAndPersonality,
// GetMonSpritePalStructFromOtIdPersonality, IsShinyOtIdPersonality y una comprobación en línea más.
const std::map<std::string, size_t> kVerified = {{"BPE", 5}};

uint16_t hw(const std::vector<uint8_t>& r, size_t o) { return static_cast<uint16_t>(r[o] | (r[o + 1] << 8)); }

std::vector<Site> findSites(const std::vector<uint8_t>& rom) {
    std::vector<Site> out;
    for (size_t o = 12; o + 6 <= rom.size(); o += 2) {
        uint16_t h = hw(rom, o);
        if ((h & 0xF8FF) != 0x2807) continue;  // cmp rX, #7
        int rx = (h >> 8) & 7;
        // la instrucción anterior es «eors rX, rY» y hay al menos 3 EOR sobre rX en las 10 instrucciones anteriores
        if ((hw(rom, o - 2) & 0xFFC7) != (0x4040 | rx)) continue;
        int eors = 0;
        for (size_t k = 1; k <= 10 && o >= 2 * k; k++)
            if ((hw(rom, o - 2 * k) & 0xFFC7) == (0x4040 | rx)) eors++;
        if (eors < 3) continue;
        uint16_t b = hw(rom, o + 2);
        if ((b & 0xFE00) != 0xD800) continue;  // bhi (D8) o bls (D9)
        out.push_back({o, o + 2});
    }
    return out;
}

// de una vez (istreambuf_iterator va byte a byte y en la SD de la Switch tarda segundos)
bool readFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    std::streamsize size = f.tellg();
    if (size < 0) return false;
    out.resize(static_cast<size_t>(size));
    f.seekg(0);
    return static_cast<bool>(f.read(reinterpret_cast<char*>(out.data()), size));
}

// ¿Es un juego verificado? Solo mira la cabecera de GBA (código en 0xAC), sin leer la ROM entera.
bool isVerifiedGame(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    char head[0xB0];
    if (!f.read(head, sizeof head)) return false;
    return kVerified.count(std::string(head + 0xAC, 3)) > 0;
}

// sitios válidos o vacío si el juego no está verificado o no coincide
std::vector<Site> validSites(const std::vector<uint8_t>& rom) {
    if (rom.size() < 0xC0) return {};
    std::string code(reinterpret_cast<const char*>(&rom[0xAC]), 3);
    auto it = kVerified.find(code);
    if (it == kVerified.end()) return {};
    auto sites = findSites(rom);
    if (sites.size() != it->second) return {};
    return sites;
}

}  // namespace

bool shinyPatchAvailable(const std::string& romFile) {
    // se llama también desde el hilo que carga la ROM (para que Extras abra sin esperas)
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);
    static std::map<std::string, bool> cache;
    auto c = cache.find(romFile);
    if (c != cache.end()) return c->second;
    std::vector<uint8_t> rom;
    bool ok = isVerifiedGame(romFile) && readFile(romFile, rom) && !validSites(rom).empty();
    cache[romFile] = ok;
    return ok;
}

bool applyShinyPatch(const std::string& romFile, int threshold, std::string* error) {
    auto fail = [&](const char* m) {
        if (error) *error = m;
        return false;
    };
    int shift = 0;
    while ((1 << shift) < threshold) shift++;
    if ((1 << shift) != threshold || shift < 3 || shift > 15) return fail("Probabilidad de shiny no válida.");
    if (threshold == 8) return true;  // original
    std::vector<uint8_t> rom;
    if (!readFile(romFile, rom)) return fail("No se pudo leer la ROM para el parche de shiny.");
    auto sites = validSites(rom);
    if (sites.empty()) return fail("El parche de shiny no está disponible para este juego.");
    std::fstream f(romFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!f) return fail("No se pudo escribir el parche de shiny.");
    for (const Site& s : sites) {
        int rx = rom[s.cmp + 1] & 7;
        uint16_t lsr = static_cast<uint16_t>(0x0800 | (shift << 6) | (rx << 3) | rx);  // lsrs rX, rX, #shift
        uint8_t cond = rom[s.branch + 1] == 0xD9 ? 0xD0 : 0xD1;                       // bls -> beq, bhi -> bne
        uint8_t a[2] = {static_cast<uint8_t>(lsr & 0xFF), static_cast<uint8_t>(lsr >> 8)};
        f.seekp(static_cast<std::streamoff>(s.cmp));
        f.write(reinterpret_cast<const char*>(a), 2);
        f.seekp(static_cast<std::streamoff>(s.branch + 1));
        f.write(reinterpret_cast<const char*>(&cond), 1);
    }
    f.flush();
    if (!f) return fail("No se pudo escribir el parche de shiny.");
    return true;
}

}  // namespace app
