// Port de partes de RomFunctions y PatchFunctions de FVX.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace pr::romfn {

using StringSizeDeterminer = std::function<int(const std::string&)>;

// String.replace(CharSequence, CharSequence) de Java (reemplaza todas, de izquierda a derecha)
std::string replaceAll(std::string s, const std::string& from, const std::string& to);

std::string rewriteDescriptionForNewLineSize(std::string moveDesc, const std::string& newline, int lineSize,
                                             const StringSizeDeterminer& ssd);
// replacements: en el orden de iteración del HashMap<String,String> de Java
std::string formatTextWithReplacements(std::string text, const std::vector<std::pair<std::string, std::string>>* replacements,
                                       const std::string& newline, const std::string& extraline,
                                       const std::string& newpara, int maxLineLength, const StringSizeDeterminer& ssd);
std::string camelCase(const std::string& original);

// Aplica un parche IPS incrustado (ruta relativa a core/data/patches, sin ".ips")
void applyPatch(std::vector<uint8_t>& rom, const std::string& patchName);

}  // namespace pr::romfn
