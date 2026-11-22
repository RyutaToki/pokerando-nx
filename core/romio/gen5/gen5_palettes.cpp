// Paletas de los Pokémon en Gen 5 (port de Gen5RomHandler.load/savePokemonPalettes y getGraphicsIndex).
#include "romio/gen5/gen5_rom_handler.hpp"

namespace pr::gen5 {

namespace {
// 40 bytes de cabecera NCLR que preceden a cada paleta (AbstractDSRomHandler.PALETTE_PREFIX_BYTES)
const uint8_t kPalettePrefix[] = {0x52, 0x4C, 0x43, 0x4E, 0xFF, 0xFE, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x10, 0x00,
                                  0x01, 0x00, 0x54, 0x54, 0x4C, 0x50, 0x38, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0A, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};

Palette readPalette(const nds::NARCArchive& narc, int index) {
    const Bytes& f = narc.files.at(index);
    return Palette::fromBytes(Bytes(f.begin() + sizeof kPalettePrefix, f.end()));
}

void writePalette(nds::NARCArchive& narc, int index, const Palette& p) {
    Bytes b(std::begin(kPalettePrefix), std::end(kPalettePrefix));
    auto pb = p.toBytes();
    b.insert(b.end(), pb.begin(), pb.end());
    narc.files.at(index) = std::move(b);
}
}  // namespace

int Gen5RomHandler::graphicsIndex(const Species& pk) const {
    if (pk.isBaseForme()) return pk.number();
    return formeGraphicsIndices_.at(pk.baseForme()->number()) + pk.formeNumber() - 1;
}

void Gen5RomHandler::loadPokemonPalettes() {
    auto narc = readNARC(romEntry_->getFile("PokemonGraphics"));
    for (Species* pk : speciesInclFormes_) {
        if (!pk) continue;
        int gfx = graphicsIndex(*pk);
        pk->normalPalette = readPalette(narc, gfx * 20 + 18);
        pk->shinyPalette = readPalette(narc, gfx * 20 + 19);
    }
}

void Gen5RomHandler::savePokemonPalettes() {
    std::string path = romEntry_->getFile("PokemonGraphics");
    auto narc = readNARC(path);
    for (Species* pk : speciesInclFormes_) {
        if (!pk) continue;
        int gfx = graphicsIndex(*pk);
        writePalette(narc, gfx * 20 + 18, *pk->normalPalette);
        writePalette(narc, gfx * 20 + 19, *pk->shinyPalette);
    }
    writeNARC(path, narc);
}

}  // namespace pr::gen5
