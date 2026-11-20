// Textos de la 4.ª generación (Diamante/Perla/Platino/HGSS): cifrado de los archivos de msg.narc,
// compresión de 9 bits y tabla Generation4.tbl. Port de PokeTextData, TextToPoke y UnicodeParser de FVX.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace pr::text4 {

struct Decoded {
    std::vector<std::string> strings;  // UTF-8
    bool compressed = false;           // alguna cadena usaba la compresión [F100]
};

// PokeTextData(data).decrypt()
Decoded readTexts(const std::vector<uint8_t>& data);

// TextToPoke.MakeFile + PokeTextData.SetKey(0xD00E) + encrypt(): el archivo listo para el NARC
std::vector<uint8_t> makeEncryptedFile(const std::vector<std::string>& strings, bool compressed);

}  // namespace pr::text4
