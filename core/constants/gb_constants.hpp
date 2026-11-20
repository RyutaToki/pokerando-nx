// Generado por tools/convert_constants.py desde GBConstants.java (FVX, GPLv3). No editar.
#pragma once
#include <cstdint>
#include <set>
#include <string_view>
#include <vector>

#include "constants/ability_ids.hpp"
#include "constants/item_ids.hpp"
#include "constants/move_ids.hpp"
#include "constants/species_ids.hpp"

namespace pr::gb {

inline constexpr int minRomSize = 0x80000;
inline constexpr int maxRomSize = 0x200000;
inline constexpr int longSize = 4;
inline constexpr int jpFlagOffset = 0x14A;
inline constexpr int versionOffset = 0x14C;
inline constexpr int crcOffset = 0x14E;
inline constexpr int romSigOffset = 0x134;
inline constexpr int isGBCOffset = 0x143;
inline constexpr int romCodeOffset = 0x13F;
inline constexpr uint8_t stringTerminator = 0x50;
inline constexpr uint8_t stringPrintedTextEnd = 0x57;
inline constexpr uint8_t stringPrintedTextPromptEnd = 0x58;
inline constexpr int farTextLength = 5;
inline constexpr int bankSize = 0x4000;
inline constexpr uint8_t gbZ80Jump = 0xC3;
inline constexpr uint8_t gbZ80Nop = 0x00;
inline constexpr uint8_t gbZ80XorA = 0xAF;
inline constexpr uint8_t gbZ80LdA = 0x3E;
inline constexpr uint8_t gbZ80LdAToFar = 0xEA;
inline constexpr uint8_t gbZ80Ret = 0xC9;
inline constexpr uint8_t gbZ80JumpRelative = 0x18;
inline constexpr uint8_t gbZ80JumpRelativeNZ = 0x20;
inline constexpr uint8_t gbZ80JumpRelativeC = 0x38;
inline constexpr uint8_t gbZ80Call = 0xCD;
inline constexpr uint8_t gbZ80RetC = 0xD8;
inline constexpr int gbZ80CallSize = 3;
inline constexpr uint8_t typeTableTerminator = 0xFF;
inline constexpr uint8_t typeTableForesightTerminator = 0xFE;
inline constexpr uint8_t evosAndMovesTerminator = 0x00;

// Pendiente de portar a mano:
//   Set<Type> physicalTypes

}  // namespace pr::gb
