// Port de CustomNamesSet: listas de nombres para entrenadores y motes (incrustadas desde FVX).
#pragma once
#include <string>
#include <vector>

namespace pr {
struct CustomNamesSet {
    std::vector<std::string> trainerNames, trainerClasses, doublesTrainerNames, doublesTrainerClasses, pokemonNicknames;
    static CustomNamesSet readEmbedded();
};
}  // namespace pr
