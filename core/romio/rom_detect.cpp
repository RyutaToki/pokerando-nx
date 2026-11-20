#include "romio/rom_detect.hpp"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "romio/gen3/gen3_rom_entry.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"
#include "util/crc32.hpp"

namespace pr {

namespace {
std::string regionName(char c) {
    switch (c) {
        case 'S': return "España";
        case 'E': return "América";
        case 'P': return "Europa";
        case 'U': return "Australia";
        case 'D': return "Alemania";
        case 'F': return "Francia";
        case 'I': return "Italia";
        case 'J': return "Japón";
        case 'K': return "Corea";
        default: return std::string("Desconocida (") + c + ")";
    }
}

struct Known {
    const char* prefix;  // 3 primeras letras del código
    const char* shortName;
    int gen;
};
const Known kGba[] = {{"AXV", "Rubí", 3}, {"AXP", "Zafiro", 3}, {"BPE", "Esmeralda", 3}, {"BPR", "Rojo Fuego", 3}, {"BPG", "Verde Hoja", 3}};
const Known kNds[] = {{"ADA", "Diamante", 4},  {"APA", "Perla", 4},      {"CPU", "Platino", 4},     {"IPK", "Oro HeartGold", 4},
                      {"IPG", "Plata SoulSilver", 4}, {"IRB", "Negro", 5}, {"IRA", "Blanco", 5}, {"IRE", "Negro 2", 5},
                      {"IRD", "Blanco 2", 5}};

bool endsWithIgnoreCase(const std::string& s, const char* ext) {
    std::string e(ext);
    if (s.size() < e.size()) return false;
    for (size_t i = 0; i < e.size(); i++)
        if (std::tolower(static_cast<unsigned char>(s[s.size() - e.size() + i])) != e[i]) return false;
    return true;
}
}  // namespace

RomInfo detectRom(const std::string& path) {
    RomInfo info;
    bool gba = endsWithIgnoreCase(path, ".gba"), nds = endsWithIgnoreCase(path, ".nds");
    info.platform = gba ? "GBA" : nds ? "NDS" : "";
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        info.note = "No se puede leer el archivo.";
        return info;
    }
    f.seekg(0, std::ios::end);
    info.size = static_cast<uint64_t>(f.tellg());
    f.seekg(0);
    std::vector<uint8_t> head(0x200, 0);
    f.read(reinterpret_cast<char*>(head.data()), static_cast<std::streamsize>(head.size()));
    if (gba) {
        info.code.assign(reinterpret_cast<const char*>(&head[0xAC]), 4);
    } else if (nds) {
        info.code.assign(reinterpret_cast<const char*>(&head[0x0C]), 4);
    } else {
        info.note = "Solo se admiten archivos .gba y .nds.";
        return info;
    }
    for (char& c : info.code)
        if (c < 0x20 || c > 0x7E) c = '?';
    info.region = regionName(info.code[3]);
    const Known* table = gba ? kGba : kNds;
    size_t n = gba ? std::size(kGba) : std::size(kNds);
    for (size_t i = 0; i < n; i++) {
        if (info.code.compare(0, 3, table[i].prefix) == 0) {
            info.recognized = true;
            info.shortName = table[i].shortName;
            info.gameName = std::string("Pokémon ") + table[i].shortName;
            info.generation = table[i].gen;
        }
    }
    if (!info.recognized) {
        info.note = "No es un juego Pokémon reconocido.";
        return info;
    }
    if (gba) {
        info.supported = gen3::Gen3RomHandler::isLoadable(head, info.size);
        for (const auto& e : gen3::Gen3RomEntry::all()) {
            if (e->romCode() == info.code && head[0xBC] == e->version()) {
                if (e->expectedCRC32() >= 0) info.expectedCRC32 = static_cast<uint32_t>(e->expectedCRC32());
                break;
            }
        }
        if (!info.supported) info.note = "Versión o región no compatible.";
    } else {
        info.note = info.generation == 4 ? "Los juegos de 4.ª generación llegarán en la fase 2."
                                         : "Los juegos de 5.ª generación llegarán en la fase 3.";
    }
    return info;
}

uint32_t fileCRC32(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("No se puede leer " + path);
    std::vector<uint8_t> buf(1 << 20);
    uint32_t crc = 0;
    while (f) {
        f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
        std::streamsize got = f.gcount();
        if (got <= 0) break;
        crc = crc32(buf.data(), static_cast<size_t>(got), crc);
    }
    return crc;
}

std::unique_ptr<RomHandler> openRom(const std::string& path) {
    RomInfo info = detectRom(path);
    if (!info.supported) throw std::runtime_error(info.note.empty() ? "ROM no compatible." : info.note);
    std::ifstream f(path, std::ios::binary);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    auto h = std::make_unique<gen3::Gen3RomHandler>();
    h->loadRom(std::move(bytes), path);
    return h;
}

}  // namespace pr
