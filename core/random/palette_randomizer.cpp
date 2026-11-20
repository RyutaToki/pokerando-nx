#include "random/palette_randomizer.hpp"

#include "util/embedded.hpp"
#include "util/java_hash.hpp"

namespace pr {

namespace {
std::vector<PaletteDescription> readDescriptions(const std::string& id) {
    std::string path = "palettes/pokePalettes" + id + ".txt";
    auto f = embeddedFile(path);
    if (!f) throw std::runtime_error("Could not find resource " + path);
    std::vector<PaletteDescription> r;
    std::string_view s = *f;
    size_t pos = 0;
    while (pos < s.size()) {  // BufferedReader.readLine
        size_t nl = s.find_first_of("\r\n", pos);
        if (nl == std::string_view::npos) nl = s.size();
        r.emplace_back(std::string(s.substr(pos, nl - pos)));
        pos = nl;
        if (pos < s.size() && s[pos] == '\r') pos++;
        if (pos < s.size() && s[pos] == '\n') pos++;
    }
    return r;
}
}  // namespace

void Gen3to5PaletteRandomizer::randomizePokemonPalettes() {
    bool typeSanity = settings.pokemonPalettesFollowTypes;
    bool shinyFromNormal = settings.pokemonPalettesShinyFromNormal;
    std::string id = romHandler.getPaletteFilesID();
    if (id.empty()) throw RandomizationException("Could not randomize palettes, unrecognized romtype.");
    using Map = JavaHashMap<Species*, std::shared_ptr<TypeBaseColorList>, JavaHashCode<Species*>, SpeciesPtrEq>;
    Map lists;
    CopyUpEvolutionsHelper::Options o(
        [&](Species* pk) {
            if (shinyFromNormal) pk->shinyPalette = pk->normalPalette;
            lists.put(pk, std::make_shared<TypeBaseColorList>(pk, nullptr, typeSanity, random));
        },
        [&](Species* from, Species* to, bool) {
            if (shinyFromNormal) to->shinyPalette = to->normalPalette;
            std::shared_ptr<TypeBaseColorList> prevo = *lists.get(from);
            lists.put(to, std::make_shared<TypeBaseColorList>(to, prevo.get(), typeSanity, random));
        });
    o.withEvolutionSanity(settings.pokemonPalettesFollowEvolutions).withCopySplitEvos(true);
    copyUpEvolutionsHelper.apply(o);

    auto descs = readDescriptions(id);
    PalettePopulator pp(random);
    lists.forEach([&](Species* const& pk, const std::shared_ptr<TypeBaseColorList>& tbcl) {
        Palette& pal = *pk->normalPalette;
        int idx = pk->number() - 1;
        PaletteDescription blank("");
        auto parts = PalettePartDescription::allFrom(idx <= static_cast<int>(descs.size()) ? descs.at(idx) : blank);
        for (size_t i = 0; i < parts.size(); i++) {
            if (parts[i].isAverageDescription()) {
                pp.populateAverageColor(pal, parts[i]);
            } else if (!parts[i].isBlank()) {
                Color base = tbcl->getBaseColor(static_cast<int>(i));
                LightDarkMode m = tbcl->getLightDarkMode(static_cast<int>(i));
                pp.populatePartFromBaseColor(pal, parts[i], base, m);
            }
        }
    });
    changesMade = true;
}

}  // namespace pr
