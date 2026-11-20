// BLZ (Bottom LZ, CUE 2011): compresión de la ARM9 y de los overlays de DS.
// Port de cuecompressors/BLZCoder.java de FVX; el compresor da exactamente los mismos bytes.
#pragma once
#include <cstdint>
#include <vector>

namespace pr::nds {
std::vector<uint8_t> blzDecode(const std::vector<uint8_t>& data);
// arm9: deja sin comprimir los primeros 0x4000 bytes (como -en9); best: modo óptimo (LZ-CUE)
std::vector<uint8_t> blzEncode(const std::vector<uint8_t>& data, bool arm9, bool best = false);
}  // namespace pr::nds
