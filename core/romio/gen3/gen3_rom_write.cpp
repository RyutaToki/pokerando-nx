// Escritura en la ROM y gestión del espacio libre (port de AbstractGBRomHandler:
// writeX, DataRewriter, freeSpace, findAndUnfreeSpace; y Gen3RomHandler.freeAllUnusedSpace).
#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

void Gen3RomHandler::writeBytes(int offset, const std::vector<uint8_t>& values) {
    if (offset == 0) throw std::invalid_argument("Not allowed to write at offset 0 of the ROM.");
    if (offset < 0 || offset + values.size() > rom_.size()) throw std::out_of_range("escritura fuera de la ROM");
    std::copy(values.begin(), values.end(), rom_.begin() + offset);
}

void Gen3RomHandler::writeWord(int offset, int value) {
    // Java: (byte)(value % 0x100), (byte)((value / 0x100) % 0x100)
    rom_.at(offset) = static_cast<uint8_t>(value % 0x100);
    rom_.at(offset + 1) = static_cast<uint8_t>((value / 0x100) % 0x100);
}

void Gen3RomHandler::writeLong(int offset, int value) {
    writeBytes(offset, {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF),
                        static_cast<uint8_t>((value >> 16) & 0xFF), static_cast<uint8_t>((value >> 24) & 0xFF)});
}

void Gen3RomHandler::writePointer(int offset, int pointer) { writeLong(offset, pointer + 0x8000000); }

int Gen3RomHandler::searchForFirst(int beginOffset, const std::vector<uint8_t>& needle) const {
    auto it = std::search(rom_.cbegin() + beginOffset, rom_.cend(), needle.cbegin(), needle.cend());
    return it == rom_.cend() ? -1 : static_cast<int>(it - rom_.cbegin());
}

void Gen3RomHandler::freeSpace(int offset, int length) {
    if (length < 1) throw std::invalid_argument("length must be at least 1.");
    for (int i = 0; i < length; i++) rom_.at(offset + i) = freeSpaceByte;
    freedSpace_.free(offset, length);
}

bool Gen3RomHandler::isRomSpaceUsed(int offset, int length) const {
    if (offset < 0) return false;
    for (int i = 0; i < length; i++) {
        if (rom_.at(offset + i) != freeSpaceByte) return true;
    }
    return false;
}

int Gen3RomHandler::findAndUnfreeSpace(int length, bool longAligned) {
    int foundOffset;
    length += longAligned ? gb::longSize : 0;
    do {
        foundOffset = freedSpace_.findAndUnfree(length);
    } while (isRomSpaceUsed(foundOffset, length));
    if (foundOffset == -1) throw std::runtime_error("ROM llena: no hay " + std::to_string(length) + " bytes libres.");
    if (longAligned) {
        int shift = gb::longSize - (foundOffset % gb::longSize);
        shift = shift == gb::longSize ? 0 : shift;
        freeSpace(foundOffset + length - (4 - shift), 4 - shift);
        foundOffset += shift;
    }
    return foundOffset;
}

int Gen3RomHandler::rewriteData(int pointerOffset, const std::vector<uint8_t>& newData,
                                const std::function<int(int)>& lengthOfOld, const std::vector<int>& secondaryPointerOffsets,
                                bool longAlign) {
    int oldDataOffset = readPointer(pointerOffset);
    int oldLength = lengthOfOld(oldDataOffset);
    freeSpace(oldDataOffset, oldLength);
    int newOffset = findAndUnfreeSpace(static_cast<int>(newData.size()), longAlign);
    writePointer(pointerOffset, newOffset);
    writeBytes(newOffset, newData);
    for (int spo : secondaryPointerOffsets) {
        int offset = readPointer(spo);
        if (spo != pointerOffset && offset != oldDataOffset) {
            char buf[128];
            std::snprintf(buf, sizeof buf, "Invalid secondary pointer spo=0x%x. Points to 0x%x instead of 0x%x.", spo,
                          offset, oldDataOffset);
            throw std::runtime_error(buf);
        }
        writePointer(spo, newOffset);
    }
    return newOffset;
}

void Gen3RomHandler::freeAllUnusedSpace() {
    int startOffset = romEntry_->getInt("FreeSpace");
    int unusedSpaceOffset = startOffset;
    const int chunkLength = unusedSpaceChunkLength;
    const int frontMargin = unusedSpaceFrontMargin;
    std::vector<uint8_t> needle(chunkLength, freeSpaceByte);
    while (true) {
        int found = searchForFirst(unusedSpaceOffset, needle);
        if (found < startOffset) break;
        if (found > unusedSpaceOffset + chunkLength || found == startOffset) {
            freeSpace(found + frontMargin, chunkLength - frontMargin);
        } else {
            freeSpace(found, chunkLength);
        }
        unusedSpaceOffset = found + chunkLength;
    }
}

}  // namespace pr::gen3
