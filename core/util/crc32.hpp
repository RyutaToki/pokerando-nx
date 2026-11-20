#pragma once
#include <cstddef>
#include <cstdint>

namespace pr {
// CRC32 estándar (el mismo que java.util.zip.CRC32).
uint32_t crc32(const uint8_t* data, size_t len, uint32_t crc = 0);
}
