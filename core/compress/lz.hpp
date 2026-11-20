// Port de DSDecmp/DSCmp (LZ10 y LZ11 de GBA/DS). La salida del compresor es idéntica a la de FVX.
#pragma once
#include <cstdint>
#include <vector>

namespace pr::lz {
std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t offset);
std::vector<uint8_t> compressLZ10(const std::vector<uint8_t>& decompressed);
std::vector<uint8_t> compressLZ11(const std::vector<uint8_t>& decompressed);
}  // namespace pr::lz
