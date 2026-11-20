// Paletas de los Pokémon en Gen 3 (comprimidas en LZ10).
#include "compress/lz.hpp"
#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

void Gen3RomHandler::loadPokemonPalettes() {
    int normal = romEntry_->getInt("PokemonNormalPalettes");
    int shiny = romEntry_->getInt("PokemonShinyPalettes");
    for (Species* pk : getSpeciesSet().toVector()) {
        int n = pokedexToInternal_[pk->number()];
        pk->normalPalette = Palette::fromBytes(lz::decompress(rom_, readPointer(normal + n * 8)));
        pk->shinyPalette = Palette::fromBytes(lz::decompress(rom_, readPointer(shiny + n * 8)));
    }
}

void Gen3RomHandler::savePokemonPalettes() {
    int normal = romEntry_->getInt("PokemonNormalPalettes");
    int shiny = romEntry_->getInt("PokemonShinyPalettes");
    for (Species* pk : getSpeciesSet().toVector()) {
        int n = pokedexToInternal_[pk->number()];
        int normalPtr = normal + n * 8;
        int shinyPtr = shiny + n * 8;
        if (pk->number() == species::unown) {
            // todas las formas de Unown comparten paleta
            std::vector<int> altNormal, altShiny;
            for (int i = 0; i < unownFormeCount - 1; i++) {
                altNormal.push_back(normal + (unownBIndex + i) * 8);
                altShiny.push_back(shiny + (unownBIndex + i) * 8);
            }
            rewriteCompressedData(normalPtr, pk->normalPalette->toBytes(), altNormal);
            rewriteCompressedData(shinyPtr, pk->shinyPalette->toBytes(), altShiny);
        } else {
            rewriteCompressedData(normalPtr, pk->normalPalette->toBytes());
            rewriteCompressedData(shinyPtr, pk->shinyPalette->toBytes());
        }
    }
}

int Gen3RomHandler::lengthOfCompressedDataAt(int offset) const {
    // FVX lo mide descomprimiendo y volviendo a comprimir
    return static_cast<int>(lz::compressLZ10(lz::decompress(rom_, offset)).size());
}

void Gen3RomHandler::rewriteCompressedData(int pointerOffset, const std::vector<uint8_t>& uncompressed,
                                           const std::vector<int>& secondary) {
    rewriteData(pointerOffset, lz::compressLZ10(uncompressed), [this](int off) { return lengthOfCompressedDataAt(off); },
                secondary);
}

}  // namespace pr::gen3
