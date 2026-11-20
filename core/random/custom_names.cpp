#include "random/custom_names.hpp"

#include <stdexcept>

#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr {
namespace {
std::vector<std::string> readLines(const std::string& name) {
    auto f = embeddedFile("names/" + name);
    if (!f) throw std::runtime_error("Could not read custom names from file.");
    std::vector<std::string> out;
    size_t pos = 0;
    std::string_view t = *f;
    // Files.readAllLines: separa por \n, \r o \r\n; no añade una línea final vacía
    while (pos < t.size()) {
        size_t e = t.find_first_of("\r\n", pos);
        std::string line(t.substr(pos, e == std::string_view::npos ? std::string_view::npos : e - pos));
        out.push_back(jc::trim(line));
        if (e == std::string_view::npos) break;
        pos = e + 1;
        if (t[e] == '\r' && pos < t.size() && t[pos] == '\n') pos++;
    }
    return out;
}
}  // namespace

CustomNamesSet CustomNamesSet::readEmbedded() {
    return {readLines("TrainerNames.txt"), readLines("TrainerClasses.txt"), readLines("DoublesTrainerNames.txt"),
            readLines("DoublesTrainerClasses.txt"), readLines("PokemonNicknames.txt")};
}
}  // namespace pr
