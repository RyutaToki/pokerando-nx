// Textos de la 5.ª generación (Negro/Blanco y Negro 2/Blanco 2): cifrado de los archivos de msg.narc,
// compresión de 9 bits y tabla Generation5.tbl. Port de PPTxtHandler de FVX.
//
// Las cadenas de Java son UTF-16; aquí se guardan en UTF-8 (WTF-8: un surrogate suelto se codifica
// solo, en 3 bytes) para poder ida y vuelta sin perder nada.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace pr::text5 {

// PPTxtHandler.readTexts
std::vector<std::string> readTexts(const std::vector<uint8_t>& data);

// PPTxtHandler.saveEntry: el archivo nuevo (o el original si hay menos cadenas que entradas)
std::vector<uint8_t> saveEntry(const std::vector<uint8_t>& originalData, const std::vector<std::string>& text);

// longitud como String.length() de Java (unidades UTF-16)
int javaLength(const std::string& s);

std::u16string toU16(const std::string& s);
std::string fromU16(const std::u16string& s);

}  // namespace pr::text5
