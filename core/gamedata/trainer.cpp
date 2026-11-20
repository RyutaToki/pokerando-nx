#include "gamedata/trainer.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "util/java_compat.hpp"

namespace pr {

TrainerPokemon::TrainerPokemon(Species* s, int lvl) : holder(s), level(lvl) {
    if (lvl < 0) throw std::invalid_argument("level must be non-negative");
}

Trainer Trainer::deepCopy() const {
    Trainer t = *this;
    t.pokemon.clear();
    for (const auto& tp : pokemon) t.pokemon.push_back(std::make_shared<TrainerPokemon>(*tp));
    return t;
}

std::string Trainer::fullDisplayName() const {
    if (!trainerClass && !name) return "NAMELESS";
    std::string s;
    if (trainerClass) s += trainerClass->name;
    if (trainerClass && name) s += " ";
    if (name) s += *name;
    return s;
}

bool Trainer::isBoss() const {
    return tag && (jc::startsWith(*tag, "ELITE") || jc::startsWith(*tag, "CHAMPION") || jc::startsWith(*tag, "UBER") ||
                   jc::endsWith(*tag, "LEADER"));
}
bool Trainer::isImportant() const {
    return tag && (jc::startsWith(*tag, "RIVAL") || jc::startsWith(*tag, "FRIEND") || jc::endsWith(*tag, "STRONG"));
}
bool Trainer::isFirstRivalOrFriend() const {
    return tag && (jc::startsWith(*tag, "RIVAL1-") || jc::startsWith(*tag, "FRIEND1-"));
}
bool Trainer::shouldNotGetBuffs() const { return isFirstRivalOrFriend() || (tag && jc::endsWith(*tag, "NOTSTRONG")); }

bool Trainer::pokemonHaveItems() const {
    return std::any_of(pokemon.begin(), pokemon.end(), [](const TrainerPokemonPtr& tp) { return tp->heldItem.has_value(); });
}

bool Trainer::pokemonHaveCustomMoves() const {
    for (const auto& tp : pokemon) {
        if (tp->resetMoves) continue;
        for (int m : tp->moves)
            if (m != 0) return true;
    }
    return false;
}

bool Trainer::pokemonHaveDifferentLevels() const {
    std::set<int> levels;
    for (const auto& tp : pokemon) levels.insert(tp->level);
    return levels.size() > 1;
}

bool Trainer::pokemonHaveUniqueHeldItems() const {
    std::set<int> seen;
    for (const auto& tp : pokemon) {
        if (tp->heldItem && !seen.insert(*tp->heldItem).second) return false;
    }
    return true;
}

}  // namespace pr
