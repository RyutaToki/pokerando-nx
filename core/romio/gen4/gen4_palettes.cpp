// Paletas de los Pokémon en Gen 4 (port de Gen4RomHandler.load/savePokemonPalettes y de
// Gen4Constants.getOtherPokemonGraphicsPalettes).
#include <map>

#include "constants/species_ids.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"

namespace pr::gen4 {

namespace {
// 40 bytes de cabecera NCLR que preceden a cada paleta en los NARC
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

using Table = std::map<int, std::pair<std::vector<int>, std::vector<int>>>;  // especie -> (normal, shiny) por número de forma

Table shifted(const Table& t, int shift) {
    Table out;
    for (const auto& [k, v] : t) {
        auto c = v;
        for (int& x : c.first) x += shift;
        for (int& x : c.second) x += shift;
        out[k] = c;
    }
    return out;
}

std::vector<int> repeat(int v, int n) { return std::vector<int>(n, v); }

// Las paletas de OtherPokemonGraphics están en el mismo orden en los tres juegos, pero se desplazan
// cuando Pt añade imágenes de 3 formas (20 archivos) y HGSS las de Pichu (4 archivos).
const Table& otherPalettes(int romType) {
    using namespace species;
    static const Table dp = {
        {unown, {repeat(136, 28), repeat(137, 28)}},
        {castform, {{138, 139, 140, 141}, {142, 143, 144, 145}}},
        {deoxys, {{134, 134, 134, 134}, {135, 135, 135, 135}}},
        {burmy, {{146, 148, 150}, {147, 149, 151}}},
        {wormadam, {{152, 154, 156}, {153, 155, 157}}},
        {cherrim, {{166, 167}, {168, 169}}},
        {shellos, {{158, 160}, {159, 161}}},
        {gastrodon, {{162, 164}, {163, 165}}},
        {arceus,
         {{170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204},
          {171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203, 205}}}};
    static const Table pt = [] {
        Table t = shifted(dp, 20);
        t[rotom] = {std::vector<int>{232, 234, 236, 238, 240, 242}, std::vector<int>{233, 235, 237, 239, 241, 243}};
        t[giratina] = {std::vector<int>{244, 246}, std::vector<int>{245, 247}};
        t[shaymin] = {std::vector<int>{228, 230}, std::vector<int>{229, 231}};
        return t;
    }();
    static const Table hgss = [] {
        Table t = shifted(pt, 4);
        t[pichu] = {std::vector<int>{252, 254}, std::vector<int>{253, 255}};
        return t;
    }();
    if (romType == Type_DP) return dp;
    if (romType == Type_Plat) return pt;
    return hgss;
}
}  // namespace

void Gen4RomHandler::loadPokemonPalettes() {
    auto pokeGraphics = readNARC(romEntry_->getFile("PokemonGraphics"));
    auto formeGraphics = readNARC(romEntry_->getFile("OtherPokemonGraphics"));
    const Table& other = otherPalettes(romEntry_->romType());
    for (Species* pk : speciesInclFormes_) {
        if (!pk) continue;
        auto it = other.find(pk->baseForme()->number());
        if (it != other.end()) {
            pk->normalPalette = readPalette(formeGraphics, it->second.first.at(pk->formeNumber()));
            pk->shinyPalette = readPalette(formeGraphics, it->second.second.at(pk->formeNumber()));
        } else {
            pk->normalPalette = readPalette(pokeGraphics, pk->number() * 6 + 4);
            pk->shinyPalette = readPalette(pokeGraphics, pk->number() * 6 + 5);
        }
    }
}

void Gen4RomHandler::savePokemonPalettes() {
    std::string pokePath = romEntry_->getFile("PokemonGraphics"), formePath = romEntry_->getFile("OtherPokemonGraphics");
    auto pokeGraphics = readNARC(pokePath);
    auto formeGraphics = readNARC(formePath);
    const Table& other = otherPalettes(romEntry_->romType());
    for (Species* pk : speciesInclFormes_) {
        if (!pk) continue;
        auto it = other.find(pk->baseForme()->number());
        if (it != other.end()) {
            writePalette(formeGraphics, it->second.first.at(pk->formeNumber()), *pk->normalPalette);
            writePalette(formeGraphics, it->second.second.at(pk->formeNumber()), *pk->shinyPalette);
        } else {
            writePalette(pokeGraphics, pk->number() * 6 + 4, *pk->normalPalette);
            writePalette(pokeGraphics, pk->number() * 6 + 5, *pk->shinyPalette);
        }
    }
    writeNARC(pokePath, pokeGraphics);
    writeNARC(formePath, formeGraphics);
}

}  // namespace pr::gen4
