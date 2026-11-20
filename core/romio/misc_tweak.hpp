// Port de MiscTweak (ajustes varios). value es la máscara de bits usada en las opciones.
#pragma once
#include <string_view>
#include <vector>

namespace pr {

struct MiscTweak {
    int value;
    std::string_view id;
    int priority;
    bool operator==(const MiscTweak& o) const { return value == o.value; }
};

namespace misctweak {
inline constexpr MiscTweak BW_EXP_PATCH{1, "bwPatch", 0};
inline constexpr MiscTweak NERF_X_ACCURACY{1 << 1, "nerfXAcc", 0};
inline constexpr MiscTweak FIX_CRIT_RATE{1 << 2, "critRateFix", 0};
inline constexpr MiscTweak FASTEST_TEXT{1 << 3, "fastestText", 0};
inline constexpr MiscTweak RUNNING_SHOES_INDOORS{1 << 4, "runningShoes", 0};
inline constexpr MiscTweak RANDOMIZE_PC_POTION{1 << 5, "pcPotion", 0};
inline constexpr MiscTweak ALLOW_PIKACHU_EVOLUTION{1 << 6, "pikachuEvo", 0};
inline constexpr MiscTweak NATIONAL_DEX_AT_START{1 << 7, "nationalDex", 0};
inline constexpr MiscTweak FAST_EGG_HATCHING{1 << 8, "fastEggHatching", 0};
inline constexpr MiscTweak FORCE_CHALLENGE_MODE{1 << 9, "forceChallengeMode", 0};
inline constexpr MiscTweak LOWER_CASE_POKEMON_NAMES{1 << 10, "lowerCaseNames", 0};
inline constexpr MiscTweak RANDOMIZE_CATCHING_TUTORIAL{1 << 11, "catchingTutorial", 0};
inline constexpr MiscTweak BAN_LUCKY_EGG{1 << 12, "luckyEgg", 1};
inline constexpr MiscTweak NO_FREE_LUCKY_EGG{1 << 13, "freeLuckyEgg", 0};
inline constexpr MiscTweak BAN_BIG_MANIAC_ITEMS{1 << 14, "maniacItems", 1};
inline constexpr MiscTweak SOS_BATTLES_FOR_ALL{1 << 15, "sosBattles", 0};
inline constexpr MiscTweak BALANCE_STATIC_LEVELS{1 << 16, "balanceStaticLevels", 0};
inline constexpr MiscTweak RETAIN_ALT_FORMES{1 << 17, "retainAltFormes", 0};
inline constexpr MiscTweak RUN_WITHOUT_RUNNING_SHOES{1 << 18, "runWithoutRunningShoes", 0};
inline constexpr MiscTweak FASTER_HP_AND_EXP_BARS{1 << 19, "fasterHpAndExpBars", 0};
inline constexpr MiscTweak FAST_DISTORTION_WORLD{1 << 20, "fastDistortionWorld", 0};
inline constexpr MiscTweak UPDATE_ROTOM_FORME_TYPING{1 << 21, "updateRotomFormeTyping", 0};
inline constexpr MiscTweak DISABLE_LOW_HP_MUSIC{1 << 22, "disableLowHpMusic", 0};
inline constexpr MiscTweak REUSABLE_TMS{1 << 23, "reusableTMs", 0};
inline constexpr MiscTweak FORGETTABLE_HMS{1 << 24, "forgettableHMs", 0};
inline constexpr MiscTweak NO_EV_YIELDS{1 << 25, "noEVYields", 0};

// MiscTweak.allTweaks, en orden de declaración
inline const std::vector<MiscTweak> allTweaks = {
    BW_EXP_PATCH, NERF_X_ACCURACY, FIX_CRIT_RATE, FASTEST_TEXT, RUNNING_SHOES_INDOORS, RANDOMIZE_PC_POTION,
    ALLOW_PIKACHU_EVOLUTION, NATIONAL_DEX_AT_START, FAST_EGG_HATCHING, FORCE_CHALLENGE_MODE, LOWER_CASE_POKEMON_NAMES,
    RANDOMIZE_CATCHING_TUTORIAL, BAN_LUCKY_EGG, NO_FREE_LUCKY_EGG, BAN_BIG_MANIAC_ITEMS, SOS_BATTLES_FOR_ALL,
    BALANCE_STATIC_LEVELS, RETAIN_ALT_FORMES, RUN_WITHOUT_RUNNING_SHOES, FASTER_HP_AND_EXP_BARS,
    FAST_DISTORTION_WORLD, UPDATE_ROTOM_FORME_TYPING, DISABLE_LOW_HP_MUSIC, REUSABLE_TMS, FORGETTABLE_HMS,
    NO_EV_YIELDS};

// Nombre en español (para la interfaz y el registro)
inline const char* nameEs(const MiscTweak& t) {
    switch (t.value) {
        case 1: return "Parche de experiencia de B/N";
        case 1 << 1: return "Debilitar Ataque X";
        case 1 << 2: return "\"Arreglar\" probabilidad de crítico";
        case 1 << 3: return "Texto lo más rápido posible";
        case 1 << 4: return "Deportivas en interiores";
        case 1 << 5: return "Objeto aleatorio en el PC";
        case 1 << 6: return "Permitir evolucionar a Pikachu";
        case 1 << 7: return "Pokédex Nacional desde el principio";
        case 1 << 8: return "Huevos que eclosionan rápido";
        case 1 << 9: return "Forzar modo desafío";
        case 1 << 10: return "Nombres de Pokémon en minúsculas";
        case 1 << 11: return "Tutorial de captura aleatorio";
        case 1 << 12: return "Prohibir Huevo Suerte";
        case 1 << 13: return "Sin Huevo Suerte gratis";
        case 1 << 14: return "Prohibir objetos para vender caros";
        case 1 << 15: return "Todos los salvajes piden ayuda (SOS)";
        case 1 << 16: return "Equilibrar niveles de estáticos";
        case 1 << 17: return "No revertir formas temporales";
        case 1 << 18: return "Correr sin deportivas";
        case 1 << 19: return "Barras de PS y EXP más rápidas";
        case 1 << 20: return "Mundo Distorsión rápido";
        case 1 << 21: return "Actualizar tipos de las formas de Rotom";
        case 1 << 22: return "Quitar música de PS bajos";
        case 1 << 23: return "MT reutilizables";
        case 1 << 24: return "MO olvidables";
        case 1 << 25: return "Los Pokémon no dan EV";
        default: return "?";
    }
}
}  // namespace misctweak

}  // namespace pr
