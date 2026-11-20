// Textos en español compartidos por el registro y la interfaz.
#pragma once
#include <string>

#include "gamedata/evolution.hpp"
#include "gamedata/move.hpp"
#include "gamedata/types.hpp"

namespace pr::es {

inline const char* tipo(Type t) {
    static const char* n[] = {"Normal", "Lucha",    "Volador",  "Planta", "Agua",  "Fuego",
                              "Roca",   "Tierra",   "Psíquico", "Bicho",  "Dragón", "Eléctrico",
                              "Fantasma", "Veneno", "Hielo",    "Acero",  "Siniestro", "Hada"};
    return n[static_cast<int>(t)];
}
inline std::string tipo(const std::optional<Type>& t) { return t ? std::string(tipo(*t)) : "???"; }

inline const char* categoryName(MoveCategory c) {
    switch (c) {
        case MoveCategory::PHYSICAL: return "Físico";
        case MoveCategory::SPECIAL: return "Especial";
        default: return "Estado";
    }
}

// "%s" se sustituye por el objeto, movimiento, Pokémon o lugar
inline const char* evolutionMethod(EvolutionType t) {
    using E = EvolutionType;
    switch (t) {
        case E::LEVEL: case E::LEVEL_CREATE_EXTRA: case E::LEVEL_GAME_THIS: return "Subir de nivel";
        case E::STONE: return "Usar %s";
        case E::TRADE: return "Intercambio";
        case E::TRADE_ITEM: return "Intercambio llevando %s";
        case E::HAPPINESS: return "Subir de nivel con mucha amistad";
        case E::HAPPINESS_DAY: return "Subir de nivel con mucha amistad de día";
        case E::HAPPINESS_NIGHT: return "Subir de nivel con mucha amistad de noche";
        case E::LEVEL_ATTACK_HIGHER: return "Subir de nivel con más Ataque que Defensa";
        case E::LEVEL_DEFENSE_HIGHER: return "Subir de nivel con más Defensa que Ataque";
        case E::LEVEL_ATK_DEF_SAME: return "Subir de nivel con Ataque igual a Defensa";
        case E::LEVEL_LOW_PV: return "Subir de nivel (valor de personalidad bajo)";
        case E::LEVEL_HIGH_PV: return "Subir de nivel (valor de personalidad alto)";
        case E::LEVEL_IS_EXTRA: return "Subir de nivel con un hueco libre en el equipo y una Poké Ball";
        case E::HIGH_BEAUTY: return "Subir de nivel con mucha belleza";
        case E::STONE_MALE_ONLY: return "Usar %s (solo machos)";
        case E::STONE_FEMALE_ONLY: return "Usar %s (solo hembras)";
        case E::ITEM_DAY: return "Subir de nivel de día llevando %s";
        case E::ITEM_NIGHT: return "Subir de nivel de noche llevando %s";
        case E::WITH_MOVE: return "Subir de nivel sabiendo %s";
        case E::WITH_OTHER: return "Subir de nivel con %s en el equipo";
        case E::LEVEL_MALE_ONLY: return "Subir de nivel (solo machos)";
        case E::LEVEL_FEMALE_ONLY: return "Subir de nivel (solo hembras)";
        case E::MAGNETIC_FIELD: case E::SNOWY: return "Subir de nivel en %s";
        case E::MOSS_ROCK: return "Subir de nivel junto a la roca musgo de %s";
        case E::ICE_ROCK: return "Subir de nivel junto a la roca hielo de %s";
        case E::TRADE_SPECIAL: return "Intercambio por Karrablast/Shelmet";
        case E::FAIRY_AFFECTION: return "Subir de nivel sabiendo un movimiento Hada y con mucho afecto";
        case E::LEVEL_WITH_DARK: return "Subir de nivel con un Pokémon Siniestro en el equipo";
        case E::LEVEL_UPSIDE_DOWN: return "Subir de nivel con la consola boca abajo";
        case E::LEVEL_RAIN: return "Subir de nivel mientras llueve";
        case E::LEVEL_DAY: case E::LEVEL_GAME_THIS_DAY: return "Subir de nivel de día";
        case E::LEVEL_NIGHT: case E::LEVEL_GAME_THIS_NIGHT: return "Subir de nivel de noche";
        case E::LEVEL_DUSK: return "Subir de nivel al atardecer";
        case E::LEVEL_ULTRA: return "Subir de nivel en el Ultraespacio";
        case E::STONE_ULTRA: return "Usar %s en el Ultraespacio";
        case E::ITEM: return "Subir de nivel llevando %s";
        default: return "Ninguno";
    }
}

}  // namespace pr::es
