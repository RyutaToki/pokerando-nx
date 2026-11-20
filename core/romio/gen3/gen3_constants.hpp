// Port parcial de Gen3Constants (FVX). Se completa a medida que avanza el port.
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <set>
#include <string_view>

#include "constants/item_ids.hpp"
#include "gamedata/types.hpp"

namespace pr::gen3 {

inline constexpr int RomType_Ruby = 0, RomType_Sapp = 1, RomType_Em = 2, RomType_FRLG = 3;

inline constexpr int size8M = 0x800000, size16M = 0x1000000, size32M = 0x2000000;
inline constexpr std::string_view unofficialEmeraldROMName = "YJencrypted";
inline constexpr int romNameOffset = 0xA0, romCodeOffset = 0xAC, romVersionOffset = 0xBC, headerChecksumOffset = 0xBD;
inline constexpr std::string_view rsPokemonNamesPointerSuffix = "30B50025084CC8F7";

inline constexpr int pokemonFrontImagesPointer = 0x128, pokemonBackImagesPointer = 0x12C,
                     pokemonNormalPalettesPointer = 0x130, pokemonShinyPalettesPointer = 0x134,
                     pokemonIconSpritesPointer = 0x138, pokemonIconPalettesPointer = 0x13C,
                     pokemonNamesPointer = 0x144, moveNamesPointer = 0x148, decorationNamesPointer = 0x14C;
inline constexpr int pokemonStatsPointer = 0x1BC, abilityNamesPointer = 0x1C0, abilityDescriptionsPointer = 0x1C4,
                     itemDataPointer = 0x1C8, moveDataPointer = 0x1CC, ballSpritesPointer = 0x1D0,
                     ballPalettesPointer = 0x1D4;

inline constexpr std::array<uint8_t, 28> emptyPokemonSig = {
    0x32, 0x96, 0x32, 0x96, 0x96, 0x32, 0x00, 0x00, 0x03, 0x01, 0xAA, 0x0A, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x78, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};

inline constexpr int baseStatsEntrySize = 0x1C;
inline constexpr int bsHPOffset = 0, bsAttackOffset = 1, bsDefenseOffset = 2, bsSpeedOffset = 3, bsSpAtkOffset = 4,
                     bsSpDefOffset = 5, bsPrimaryTypeOffset = 6, bsSecondaryTypeOffset = 7, bsCatchRateOffset = 8,
                     bsEVYieldOffset = 10, bsCommonHeldItemOffset = 12, bsRareHeldItemOffset = 14,
                     bsGenderRatioOffset = 16, bsEggCyclesOffset = 17, bsGrowthCurveOffset = 19,
                     bsPrimaryEggGroupOffset = 20, bsSecondaryEggGroupOffset = 21, bsAbility1Offset = 22,
                     bsAbility2Offset = 23;

inline constexpr uint8_t textTerminator = 0xFF, textVariable = 0xFD, textPadding = 0x00;

inline constexpr int unhackedMaxPokedex = 411, unhackedRealPokedex = 386, hoennPokesStart = 252;
inline constexpr int cacophonyIndex = 76, airLockIndex = 77, highestAbilityIndex = 77;
inline constexpr int tmCount = 50, hmCount = 8;

inline constexpr int rsTrainerFrontPalettesOffset = 0x298, emTrainerFrontPalettesOffset = 0x2E8,
                     frlgTrainerFrontPalettesOffset = 0x4A0, rsTrainerBackPalettesOffset = 0x18,
                     emTrainerBackPalettesOffset = 0x40, frlgTrainerBackPalettesOffset = 0x30;
inline constexpr int brendanMapIconPaletteOffset = -0x20, mayMapIconImageOffset = 0xA0,
                     mayMapIconPaletteOffset = 0x80, redMapIconPalettePointerOffset = 0xC4,
                     leafMapIconImagePointerOffset = -0x30, leafMapIconPalettePointerOffset = 0xE0;

// typeTable[byte] -> tipo (nullopt si el byte no es un tipo; 0x09 es el tipo "???")
std::optional<Type> typeFromByte(int b);
int typeToByte(std::optional<Type> t);  // nullopt -> 0x09 (tipo ???)

int itemIDToStandard(int internalId);
int itemIDToInternal(int standardId);
int getLastItemID(int romType);
const std::set<int>& bannedItems();
const std::set<int>& badItems(int romType);

}  // namespace pr::gen3
