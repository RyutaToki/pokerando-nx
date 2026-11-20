// Utilidades que imitan el comportamiento de funciones de Java usadas por FVX,
// para que el port lea los mismos datos exactamente igual.
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace pr::jc {

// String.trim(): quita los caracteres <= ' ' de ambos extremos.
std::string trim(std::string_view s);

// String.split(regexLiteral, limit) para un separador literal de un carácter o más.
// limit == 0 se comporta como Java: descarta las cadenas vacías del final.
std::vector<std::string> split(std::string_view s, std::string_view sep, int limit = 0);

// String.toLowerCase() para ASCII (los nombres de Pokémon de Gen 3 son ASCII o Latin-1).
std::string toLower(std::string_view s);

bool startsWith(std::string_view s, std::string_view prefix);
bool endsWith(std::string_view s, std::string_view suffix);
bool contains(std::string_view s, std::string_view needle);
bool equalsIgnoreCase(std::string_view a, std::string_view b);

// Integer.parseInt(s, radix): devuelve false si no cabe en int32 o no es un número válido.
bool parseIntRadix(std::string_view s, int radix, int32_t& out);
bool parseLongRadix(std::string_view s, int radix, int64_t& out);

// UTF-8 <-> UTF-32 (los textos de las tablas .tbl contienen caracteres no ASCII).
std::u32string utf8ToU32(std::string_view s);
std::string u32ToUtf8(std::u32string_view s);

}  // namespace pr::jc
