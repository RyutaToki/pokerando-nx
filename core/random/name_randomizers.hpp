// Port de MoveNameRandomizer y TrainerNameRandomizer.
#pragma once
#include <map>
#include <set>

#include "random/randomizer.hpp"

namespace pr {

class MoveNameRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeMoveNames();

private:
    std::set<std::string> used_;  // solo se consulta si contiene
    std::string randomMoveName(const Move& mv, int maxLen);
};

class TrainerNameRandomizer : public Randomizer {
public:
    using Randomizer::Randomizer;
    void randomizeTrainerNames();
    void randomizeTrainerClassNames();

private:
    struct CustomNames {
        std::vector<std::string> all;
        std::map<int, std::vector<std::string>> byLength;
    };
    CustomNames readList(const std::vector<std::string>& names, int limit);
};

}  // namespace pr
