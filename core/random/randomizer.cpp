#include "random/randomizer.hpp"

#include "util/java_compat.hpp"

namespace pr {

bool Randomizer::detectUpperCaseNames(const std::vector<std::string>& names) {
    int upper = 0, lower = 0;
    for (const auto& n : names) {
        for (char32_t c : jc::utf8ToU32(n)) {
            bool up = (c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDE && c != 0xD7);
            bool lo = (c >= 'a' && c <= 'z') || (c >= 0xDF && c <= 0xFF && c != 0xF7);
            if (up) upper++;
            else if (lo) lower++;
        }
    }
    return lower == 0 || (upper > 0 && lower * 10 < upper);
}

}  // namespace pr
