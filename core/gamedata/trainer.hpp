// Port de TrainerClass, BattleStyle, TrainerPokemon y Trainer.
#pragma once
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/encounter.hpp"

namespace pr {

struct TrainerClass {
    TrainerClass(int id, std::string name) : id(id), name(std::move(name)) {}
    int id;
    std::string name;
};

struct BattleStyle {
    enum class Modification { UNCHANGED, RANDOM, SINGLE_STYLE };
    enum class Style { SINGLE_BATTLE, DOUBLE_BATTLE, TRIPLE_BATTLE, ROTATION_BATTLE };
    Modification mod = Modification::UNCHANGED;
    Style style = Style::SINGLE_BATTLE;
    bool isBattleStyleChanged() const { return mod != Modification::UNCHANGED; }
    bool isOnlyMultiBattles() const { return mod == Modification::SINGLE_STYLE && style != Style::SINGLE_BATTLE; }
    int requiredPokemonCount() const {
        if (!isBattleStyleChanged()) return -1;
        switch (style) {
            case Style::DOUBLE_BATTLE: return 2;
            case Style::TRIPLE_BATTLE:
            case Style::ROTATION_BATTLE: return 3;
            default: return 1;
        }
    }
};

struct TrainerPokemon {
    TrainerPokemon(Species* species, int level);
    TrainerPokemon(const TrainerPokemon&) = default;

    SpeciesHolder holder;
    int level = 0;
    std::array<int, 4> moves{0, 0, 0, 0};
    std::optional<int> heldItem;  // id estándar
    bool hasMegaStone = false;
    bool hasZCrystal = false;
    int abilitySlot = 0;
    int forcedGenderFlag = 0;
    int8_t nature = 0;
    int8_t hpEVs = 0, atkEVs = 0, defEVs = 0, spatkEVs = 0, spdefEVs = 0, speedEVs = 0;
    int IVs = 0;
    int strength = 0;
    bool resetMoves = false;
    bool isAddedTeamMember = false;

    Species* species() const { return holder.species(); }
};
using TrainerPokemonPtr = std::shared_ptr<TrainerPokemon>;

struct Trainer {
    enum class MultiBattleStatus { NEVER, POTENTIAL, ALWAYS };

    int offset = 0;
    int index = 0;
    std::vector<TrainerPokemonPtr> pokemon;
    std::optional<std::string> tag;
    std::optional<std::string> name;
    TrainerClass* trainerClass = nullptr;
    MultiBattleStatus multiBattleStatus = MultiBattleStatus::NEVER;
    bool forcedDoubleBattle = false;
    int forceStarterPosition = -1;
    bool requiresUniqueHeldItems = false;
    // compartido con las copias, como en Java
    std::shared_ptr<BattleStyle> currBattleStyle = std::make_shared<BattleStyle>();

    Trainer() = default;
    Trainer deepCopy() const;  // new Trainer(original): copia los TrainerPokemon

    int poketype() const { return (pokemonHaveCustomMoves() ? 1 : 0) + (pokemonHaveItems() ? 2 : 0); }
    std::string fullDisplayName() const;
    bool isBoss() const;
    bool isImportant() const;
    bool isRegular() const { return !isBoss() && !isImportant(); }
    bool isFirstRivalOrFriend() const;
    bool shouldNotGetBuffs() const;
    bool pokemonHaveItems() const;
    bool pokemonHaveCustomMoves() const;
    bool pokemonHaveDifferentLevels() const;
    bool pokemonHaveUniqueHeldItems() const;
};
using TrainerPtr = std::shared_ptr<Trainer>;

}  // namespace pr
