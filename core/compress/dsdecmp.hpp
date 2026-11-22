// Port de DSDecmp de FVX: descompresión LZ10/LZ11 de Nintendo DS.
#pragma once
#include <cstdint>
#include <vector>

namespace pr::dsdecmp {
// lanza std::invalid_argument si no hay datos comprimidos, std::out_of_range si están mal
std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, int offset = 0);
}
