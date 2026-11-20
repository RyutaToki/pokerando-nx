// Port de los "updaters" de FVX: actualizan movimientos, stats base y tabla de tipos a una generación posterior.
#pragma once
#include <map>

#include "romio/rom_handler.hpp"

namespace pr {

// Update<Object>: antes/después guardados como números (potencia, PP, precisión) o como enum (tipo, categoría)
struct UpdateValue {
    double before = 0, after = 0;
};

class TypeEffectivenessUpdater {
public:
    explicit TypeEffectivenessUpdater(RomHandler& rh) : romHandler(rh) {}
    void updateTypeEffectiveness();
    bool isUpdated() const { return !updates.empty(); }
    // TreeMap<Type atacante, TreeMap<Type defensor, Update<Effectiveness>>>
    std::map<Type, std::map<Type, std::pair<Effectiveness, Effectiveness>>> updates;

private:
    RomHandler& romHandler;
};

class MoveUpdater {
public:
    enum class UpdateType { POWER, PP, ACCURACY, TYPE, CATEGORY };
    explicit MoveUpdater(RomHandler& rh) : romHandler(rh) {}
    void updateMoves(int updateToGen);
    bool isUpdated() const { return !updates.empty(); }
    std::map<int, std::map<UpdateType, UpdateValue>> updates;  // clave: número del movimiento

private:
    RomHandler& romHandler;
    int gen() const { return romHandler.generationOfPokemon(); }
    Move& mv(int n);
    void updateMovePower(int n, int power);
    void updateMovePP(int n, int pp);
    void updateMoveAccuracy(int n, double acc);
    void updateMoveType(int n, Type t);
    void updateMoveCategory(int n, MoveCategory c);
    void gen2Updates();
    void gen3Updates();
    void gen4Updates();
    void gen5Updates();
    void gen6Updates();
    void gen7Updates();
    void gen8Updates();
    void gen9Updates();
};

class SpeciesBaseStatUpdater {
public:
    enum class UpdateType { HP, ATK, DEF, SPATK, SPDEF, SPEED, SPECIAL };
    explicit SpeciesBaseStatUpdater(RomHandler& rh) : romHandler(rh) {}
    void updateSpeciesStats(int updateToGen);
    bool isUpdated() const { return !updates.empty(); }
    std::map<int, std::map<UpdateType, std::pair<int, int>>> updates;  // clave: número de especie

private:
    RomHandler& romHandler;
    int gen() const { return romHandler.generationOfPokemon(); }
    void set(int species, int stat, int value, UpdateType t);
    void updateHP(int s, int v) { set(s, 0, v, UpdateType::HP); }
    void updateAtk(int s, int v) { set(s, 1, v, UpdateType::ATK); }
    void updateDef(int s, int v) { set(s, 2, v, UpdateType::DEF); }
    void updateSpAtk(int s, int v) { if (gen() != 1) set(s, 3, v, UpdateType::SPATK); }
    void updateSpDef(int s, int v) { if (gen() != 1) set(s, 4, v, UpdateType::SPDEF); }
    void updateSpeed(int s, int v) { set(s, 5, v, UpdateType::SPEED); }
    void updateSpecial(int, int) {}  // solo Gen 1
    void gen6Updates();
    void gen7Updates();
    void gen8Updates();
    void gen9Updates();
};

}  // namespace pr
