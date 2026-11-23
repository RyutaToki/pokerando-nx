// Registro de la randomización ("spoiler log"), en español. Sigue la estructura de
// RandomizationLogger de FVX: mismas secciones, mismos atajos {XXXX} y mismo orden.
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <map>
#include <ostream>
#include <set>

#include "log/text_es.hpp"
#include "random/game_randomizer.hpp"
#include "romio/misc_tweak.hpp"
#include "util/java_compat.hpp"

namespace pr {

namespace {
using S = Settings;

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return buf;
}
// longitud en caracteres (no en bytes), para alinear columnas con tildes
size_t len(const std::string& s) { return jc::utf8ToU32(s).size(); }
std::string padR(const std::string& s, size_t w) { return s + std::string(w > len(s) ? w - len(s) : 0, ' '); }
std::string padL(const std::string& s, size_t w) { return std::string(w > len(s) ? w - len(s) : 0, ' ') + s; }

// TextTable de FVX
class TextTable {
public:
    explicit TextTable(int cols) : cols_(cols) {}
    void addRow(const std::vector<std::string>& r) {
        evenOut();
        std::vector<std::optional<std::string>> row(r.begin(), r.end());
        data_.push_back(row);
    }
    void addCell(int col, const std::string& c) {
        size_t row = 0;
        while (row < data_.size() && data_[row][col].has_value()) row++;
        if (row == data_.size()) data_.emplace_back(cols_);
        data_[row][col] = c;
    }
    void evenOut() {
        for (auto& r : data_)
            for (auto& c : r)
                if (!c) c = "";
    }
    std::string str() {
        evenOut();
        std::vector<size_t> w(cols_, 0);
        for (auto& r : data_)
            for (int c = 0; c < cols_; c++) w[c] = std::max(w[c], len(*r[c]));
        std::string out;
        for (auto& r : data_) {
            for (int c = 0; c < cols_; c++) {
                out += padR(*r[c], w[c]);
                if (c != cols_ - 1) out += "|";
            }
            out += "\n";
        }
        return out;
    }

private:
    int cols_;
    std::vector<std::vector<std::optional<std::string>>> data_;
};

struct Section {
    const char* id;
    const char* title;
    const char* shortcut;
};
const Section kSections[] = {
    {"toc", "Índice", "TABL"},
    {"overview", "Resumen de la randomización", "OVRD"},
    {"stat", "Estadísticas", "STAT"},
    {"dg", "Diagnóstico", "DIAG"},
    {"bsu", "Actualizaciones de stats base", "PBSU"},
    {"psta", "Stats base / Tipos / Habilidades", "PKST"},
    {"pe", "Evoluciones", "PKEV"},
    {"pei", "Mejoras de evoluciones", "PKEI"},
    {"sp", "Pokémon iniciales", "SRPK"},
    {"stp", "Pokémon estáticos", "STPK"},
    {"igt", "Intercambios", "IGTR"},
    {"mu", "Actualizaciones de movimientos", "MVEU"},
    {"md", "Datos de movimientos", "MVDT"},
    {"pms", "Movimientos de cada Pokémon", "PKMV"},
    {"lvlc", "Niveles máximos (level caps)", "LVLC"},
    {"tp", "Pokémon de los entrenadores", "TRPK"},
    {"wp", "Pokémon salvajes", "WDPK"},
    {"tm", "Movimientos de las MT", "TMMV"},
    {"tmc", "Compatibilidad MT/MO", "TMCB"},
    {"mt", "Movimientos de los tutores", "MTMV"},
    {"mtc", "Compatibilidad de los tutores", "MTCB"},
    {"sh", "Objetos de las tiendas", "SHMS"},
    {"pu", "Objetos de Recogida", "PUMS"},
    {"teu", "Actualizaciones de la tabla de tipos", "TPEU"},
    {"te", "Tabla de tipos", "TPEF"},
};
const Section& section(const std::string& id) {
    for (const auto& s : kSections)
        if (id == s.id) return s;
    throw std::logic_error("sección desconocida: " + id);
}

const char* effName(Effectiveness e) {
    switch (e) {
        case Effectiveness::ZERO: return "Sin efecto/Inmune";
        case Effectiveness::HALF: return "Poco eficaz";
        case Effectiveness::DOUBLE: return "Súper eficaz";
        default: return "Neutro";
    }
}
const char* effSymbol(Effectiveness e) {
    switch (e) {
        case Effectiveness::ZERO: return " 0 ";
        case Effectiveness::HALF: return " ½ ";
        case Effectiveness::DOUBLE: return " 2 ";
        default: return "---";
    }
}
}  // namespace

void GameRandomizer::captureOriginals() {
    // como en el constructor de RandomizationLogger (leer los estáticos puede aplicar parches)
    origMTMoves_ = rh_.getMoveTutorMoves();
    origTrainerNames_ = {""};
    for (const auto& t : rh_.getTrainers()) origTrainerNames_.push_back(t->fullDisplayName());
    if (rh_.canChangeStaticPokemon()) {
        for (auto& se : rh_.getStaticPokemon()) origStatics_.push_back(se.deepCopy());
    }
    origTrades_ = rh_.getInGameTrades();
    for (const auto& m : rh_.getMoves()) origMoveNames_.push_back(m ? std::optional<std::string>(m->name) : std::nullopt);
}

void GameRandomizer::writeLog(std::ostream& log, long long elapsedMs, int64_t seed) {
    const auto& items = rh_.getItems();
    const auto& moves = rh_.getMoves();
    auto itemName = [&](int id) -> std::string {
        return id >= 0 && id < static_cast<int>(items.size()) && items[id] ? items[id]->name : "???";
    };
    auto moveName = [&](int id) -> std::string {
        return id >= 0 && id < static_cast<int>(moves.size()) && moves[id] ? moves[id]->name : "???";
    };
    auto title = [&](const char* id) {
        const auto& s = section(id);
        log << " ( " << s.title << " {" << s.shortcut << "} )\n\n";
    };
    auto sep = [&] { log << "\n==========================================================\n\n"; };
    auto row = [&](const char* id) {
        const auto& s = section(id);
        std::string t = s.title;
        log << " " << t << std::string(49 > len(t) ? 49 - len(t) : 0, '-') << "{" << s.shortcut << "}\n";
    };
    auto tpString = [&](const TrainerPokemon& tp) {
        std::string s = tp.species()->fullName();
        if (tp.heldItem) s += "@" + itemName(*tp.heldItem);
        return s + " Nv" + std::to_string(tp.level);
    };
    auto staticString = [&](const StaticEncounter& se, bool printLevel) {
        std::string s = se.species()->fullName();
        if (se.isEgg) return s + " (huevo)";
        if (!printLevel) return s;
        std::string lv = "Nv" + std::to_string(se.level);
        if (se.maxLevel > 0) lv += "-" + std::to_string(se.maxLevel);
        bool linked = std::any_of(se.linkedEncounters.begin(), se.linkedEncounters.end(), [&](const StaticEncounter& l) { return l.level != se.level; });
        if (linked)
            for (const auto& l : se.linkedEncounters) lv += " / Nv" + std::to_string(l.level);
        return s + " " + lv;
    };
    auto evoMethod = [&](const Evolution& evo) {
        std::string m = es::evolutionMethod(evo.type());
        std::string arg;
        if (usesItem(evo.type())) arg = itemName(evo.extraInfo());
        else if (usesMove(evo.type())) arg = moveName(evo.extraInfo());
        else if (usesSpecies(evo.type())) arg = rh_.getSpecies().at(evo.extraInfo())->fullName();
        else if (usesLocation(evo.type())) arg = "ningún sitio";
        size_t p = m.find("%s");
        if (p != std::string::npos) m.replace(p, 2, arg);
        if (usesLevelThreshold(evo.type())) m += fmt(", nv. %d+", evo.extraInfo());
        else m += fmt(" (nivel estimado %d)", evo.estimatedEvoLvl());
        return m;
    };

    const bool metronome = settings_.movesetsMod == S::MovesetsMod::METRONOME_ONLY;
    const bool logBSU = bsUpdater_.isUpdated();
    const bool logTraits = bsUpdater_.isUpdated() || bs_.isChangesMade() || types_.isChangesMade() ||
                           abilities_.isChangesMade() || encItems_.isChangesMade();
    const bool logEvos = evo_.isChangesMade();
    const bool logEvoImpr = !rh_.getPreImprovedEvolutions().empty();
    const bool logStarters = starter_.isChangesMade();
    const bool logStatics = rh_.canChangeStaticPokemon() && statics_.isChangesMade();
    const bool logTrades = trades_.isChangesMade();
    const bool logMU = moveUpdater_.isUpdated();
    const bool logMD = moveData_.isChangesMade() || moveUpdater_.isUpdated() || moveNames_.isChangesMade();
    const bool logMovesets = movesets_.isChangesMade() || metronome;
    const bool logTrainers = trainerPoke_.isChangesMade() || trainerMoves_.isChangesMade() || trainerNames_.isChangesMade();
    const bool logWild = wild_.isChangesMade();
    const bool logTM = tmt_.tmChangesMade || metronome;
    const bool logTMC = comp_.tmhmChangesMade;
    const bool logMT = rh_.hasMoveTutors() && (tmt_.tutorChangesMade || metronome);
    const bool logMTC = rh_.hasMoveTutors() && comp_.tutorChangesMade;
    const bool logShops = items_.shopChangesMade;
    const bool logPickup = items_.pickupChangesMade;
    const bool logTEU = typeEffUpdater_.isUpdated();
    const bool logTE = typeEffUpdater_.isUpdated() || typeEff_.isChangesMade();

    // ---------- cabecera ----------
    log << " [ Registro de la partida randomizada ]\n\n";
    log << "Juego base: " << rh_.romName() << "\n";
    log << "Programa: POKERM NX by RyutaToki (port de Universal Pokemon Randomizer FVX)\n";
    log << "Semilla: " << seed << "\n";
    log << "Cadena de opciones: " << settings_.toString() << "\n\n";
    log << "La misma ROM, semilla y cadena de opciones dan el mismo resultado que FVX en el PC.\n";
    sep();

    // ---------- índice ----------
    title("toc");
    log << "Busca (Ctrl+F) las palabras entre llaves {} para saltar\na cada sección.\n\n";
    row("toc");
    row("overview");
    log << "\n";
    if (logBSU) row("bsu");
    if (logTraits) row("psta");
    if (logEvos) row("pe");
    if (logEvoImpr) row("pei");
    if (logBSU || logTraits || logEvos || logEvoImpr) log << "\n";
    if (logStarters) row("sp");
    if (logStatics) row("stp");
    if (logTrades) row("igt");
    if (logStarters || logStatics || logTrades) log << "\n";
    if (logMU) row("mu");
    if (logMD) row("md");
    if (logMovesets) row("pms");
    if (logMU || logMD || logMovesets) log << "\n";
    if (logTrainers) {
        row("lvlc");
        row("tp");
        log << "\n";
    }
    if (logWild) {
        row("wp");
        log << "\n";
    }
    if (logTM) row("tm");
    if (logTMC) row("tmc");
    if (logMT) row("mt");
    if (logMTC) row("mtc");
    if (logTM || logTMC || logMT || logMTC) log << "\n";
    if (logShops) row("sh");
    if (logPickup) row("pu");
    if (logShops || logPickup) log << "\n";
    if (logTEU) row("teu");
    if (logTE) row("te");
    if (logTEU || logTE) log << "\n";
    row("stat");
    row("dg");
    sep();

    // ---------- resumen ----------
    title("overview");
    auto ov = [&](const char* what, bool changed, bool relevant) {
        if (relevant) log << what << ": " << (changed ? "Randomizado/cambiado" : "Sin cambios") << "\n";
    };
    ov("Stats base", bs_.isChangesMade() || bsUpdater_.isUpdated(), true);
    ov("Tipos", types_.isChangesMade(), true);
    ov("Habilidades", abilities_.isChangesMade(), rh_.abilitiesPerSpecies() != 0);
    ov("Evoluciones", evo_.isChangesMade() || logEvoImpr, true);
    ov("Pokémon iniciales", starter_.isChangesMade(), true);
    ov("Pokémon estáticos", statics_.isChangesMade(), rh_.canChangeStaticPokemon());
    ov("Intercambios", trades_.isChangesMade(), true);
    ov("Datos de movimientos", logMD, true);
    ov("Movimientos de los Pokémon", movesets_.isChangesMade(), true);
    ov("Pokémon de los entrenadores", trainerPoke_.isChangesMade(), true);
    ov("Movimientos de los entrenadores", trainerMoves_.isChangesMade(), true);
    ov("Nombres de los entrenadores", trainerNames_.isChangesMade(), true);
    ov("Pokémon salvajes", wild_.isChangesMade(), true);
    ov("Movimientos de las MT", tmt_.tmChangesMade, true);
    ov("Compatibilidad MT/MO", comp_.tmhmChangesMade, true);
    ov("Movimientos de los tutores", tmt_.tutorChangesMade, rh_.hasMoveTutors());
    ov("Compatibilidad de los tutores", comp_.tutorChangesMade, rh_.hasMoveTutors());
    ov("Objetos del suelo", items_.fieldChangesMade, true);
    ov("Objetos de las tiendas", items_.shopChangesMade, rh_.hasShopSupport());
    ov("Objetos de Recogida", items_.pickupChangesMade, rh_.abilitiesPerSpecies() != 0);
    ov("Tabla de tipos", typeEff_.isChangesMade() || typeEffUpdater_.isUpdated(), rh_.hasTypeEffectivenessSupport());
    ov("Paletas de colores", palettes_.isChangesMade(), rh_.hasPokemonPaletteSupport());
    log << "\n";
    if (misc_.isChangesMade()) {
        log << "Se aplicaron estos ajustes varios:\n";
        for (const auto& mt : misctweak::allTweaks)
            if (settings_.currentMiscTweaks & mt.value) log << misctweak::nameEs(mt) << "\n";
    } else {
        log << "No se aplicó ningún ajuste vario.\n";
    }
    log << "\n";
    if (intro_.isChangesMade() && intro_.introSpecies) log << "Pokémon de la introducción: " << intro_.introSpecies->fullName() << "\n";
    sep();

    // ---------- actualizaciones de stats base ----------
    if (logBSU) {
        title("bsu");
        log << "Stats base actualizados a los de la " << settings_.updateBaseStatsToGeneration << ".ª generación:\n\n";
        static const char* names[] = {"PS", "ATQ", "DEF", "ATESP", "DEFESP", "VEL", "ESP"};
        for (const auto& [num, ups] : bsUpdater_.updates) {
            log << rh_.getSpeciesInclFormes().at(num)->fullName() << ":\n";
            for (const auto& [t, v] : ups) log << "\t" << padR(names[int(t)], 7) << fmt(": %3d -> %3d\n", v.first, v.second);
        }
        sep();
    }

    // ---------- stats / tipos / habilidades ----------
    if (logTraits) {
        title("psta");
        std::vector<Species*> all;
        for (Species* b : rh_.getSpecies()) {
            if (!b) continue;
            all.push_back(b);
            auto alts = b->altFormes().toVector();
            std::sort(alts.begin(), alts.end(), [](Species* a, Species* c) { return a->number() < c->number(); });
            all.insert(all.end(), alts.begin(), alts.end());
        }
        size_t numLen = std::to_string(all.size()).size(), nameLen = 0, abLen = 0;
        for (Species* s : all) nameLen = std::max(nameLen, len(s->fullName()));
        for (int i = 0; i < rh_.highestAbilityIndex(); i++) abLen = std::max(abLen, len(rh_.abilityName(i)));
        const size_t typeLen = 21;
        log << padL("NÚM", numLen) << "|" << padR("NOMBRE", nameLen) << "|" << padR("TIPO", typeLen) << "|" << padL("TOT", 4);
        for (const char* h : {"PS", "ATQ", "DEF", "AESP", "DESP", "VEL"}) log << "|" << padL(h, 4);
        static const char* abH[] = {"HABILIDAD 1", "HABILIDAD 2", "HAB. OCULTA"};
        for (int i = 0; i < rh_.abilitiesPerSpecies(); i++) log << "|" << padR(abH[i], abLen);
        log << "|OBJETO\n";
        for (Species* pk : all) {
            if (pk->isEssentiallyCosmetic()) continue;
            log << (pk->isBaseForme() ? padL(std::to_string(pk->baseNumber()), numLen) : std::string(numLen, ' '));
            log << "|" << padR(pk->fullName(), nameLen);
            std::string ty = es::tipo(pk->primaryType());
            if (pk->secondaryType()) ty += "/" + es::tipo(pk->secondaryType());
            log << "|" << padR(ty, typeLen) << fmt("|%4d", pk->bst(false));
            const auto& b = pk->baseStats();
            log << fmt("|%4d|%4d|%4d|%4d|%4d|%4d", b.getHp(), b.attack, b.defense, b.spatk, b.spdef, b.speed);
            int ab[3] = {pk->ability1, pk->ability2, pk->ability3};
            for (int i = 0; i < rh_.abilitiesPerSpecies(); i++)
                log << "|" << padR(i > 0 && ab[i] == ab[0] ? "--" : rh_.abilityName(ab[i]), abLen);
            log << "|";
            std::vector<std::string> its;
            if (pk->guaranteedHeldItem) {
                its.push_back(itemName(*pk->guaranteedHeldItem) + " (100%)");
            } else {
                if (pk->commonHeldItem) its.push_back(itemName(*pk->commonHeldItem) + " (común)");
                if (pk->rareHeldItem) its.push_back(itemName(*pk->rareHeldItem) + " (raro)");
                if (pk->darkGrassHeldItem) its.push_back(itemName(*pk->darkGrassHeldItem) + " (solo hierba oscura)");
            }
            for (size_t i = 0; i < its.size(); i++) log << (i ? ", " : "") << its[i];
            log << "\n";
        }
        sep();
    }

    // ---------- evoluciones ----------
    if (logEvos) {
        title("pe");
        size_t nameLen = 0;
        for (Species* s : rh_.getSpeciesInclFormes())
            if (s) nameLen = std::max(nameLen, len(s->fullName()));
        log << padR("DE", nameLen) << "|" << padR("A", nameLen) << "|MÉTODO\n";
        for (Species* pk : rh_.getSpeciesInclFormes()) {
            if (!pk || pk->isEssentiallyCosmetic() || pk->evolutionsFrom().empty()) continue;
            for (size_t i = 0; i < pk->evolutionsFrom().size(); i++) {
                const Evolution& e = *pk->evolutionsFrom()[i];
                if (e.type() == EvolutionType::NONE) continue;
                log << padR(i == 0 ? pk->fullName() : "", nameLen) << "|" << padR(e.to()->fullName(), nameLen) << "|" << evoMethod(e) << "\n";
            }
        }
        sep();
    }

    // ---------- mejoras de evoluciones ----------
    if (logEvoImpr) {
        title("pei");
        log << "Algunas evoluciones se han cambiado con estas opciones:\n";
        if (settings_.changeImpossibleEvolutions) log << "- Cambiar evoluciones imposibles\n";
        if (settings_.makeEvolutionsEasier) log << "- Facilitar las evoluciones\n";
        if (settings_.removeTimeBasedEvolutions) log << "- Quitar evoluciones que dependen de la hora\n";
        log << "\n";
        if (settings_.makeEvolutionsEasier && rh_.generationOfPokemon() != 1)
            log << "Las evoluciones por amistad ahora piden 160 de amistad (antes 220).\n";
        log << "\n";
        TextTable t(4);
        t.addRow({"DE", "A", "MÉTODO ANTIGUO", "MÉTODO NUEVO"});
        for (const auto& [num, oldEvos] : rh_.getPreImprovedEvolutions()) {
            Species* pk = rh_.getSpeciesInclFormes().at(num);
            t.addCell(0, pk->fullName());
            std::vector<Species*> order;
            std::map<Species*, std::vector<const Evolution*>> oldBy, newBy;
            for (const auto& e : oldEvos) oldBy[e.to()].push_back(&e);
            for (const auto& e : pk->evolutionsFrom()) {
                if (!newBy.count(e->to())) order.push_back(e->to());
                newBy[e->to()].push_back(e.get());
            }
            for (Species* to : order) {
                t.addCell(1, to->fullName());
                auto& olds = oldBy[to];
                if (olds.empty()) t.addCell(2, "Ninguno");
                auto& news = newBy[to];
                for (size_t i = 0; i < news.size(); i++) {
                    t.addCell(3, evoMethod(*news[i]));
                    if (i < olds.size()) t.addCell(2, evoMethod(*olds[i]));
                }
                t.evenOut();
            }
            t.evenOut();
        }
        log << t.str();
        sep();
    }

    // ---------- iniciales ----------
    if (logStarters) {
        title("sp");
        const char* mode = "Sin cambios";
        switch (settings_.startersMod) {
            case S::StartersMod::CUSTOM: mode = "Elegidos a mano"; break;
            case S::StartersMod::COMPLETELY_RANDOM: mode = "Aleatorios (cualquiera)"; break;
            case S::StartersMod::RANDOM_BASIC: mode = "Aleatorios (cualquier Pokémon básico)"; break;
            case S::StartersMod::RANDOM_WITH_TWO_EVOLUTIONS: mode = "Aleatorios (básicos con dos evoluciones)"; break;
            default: break;
        }
        log << "Modo: " << mode << "\n";
        auto starters = rh_.getStarters();
        auto held = rh_.getStarterHeldItems();
        for (size_t i = 0; i < starters.size(); i++) {
            std::optional<int> it;
            if (held.size() == 1) it = held[0];
            else if (held.size() == starters.size()) it = held[i];
            log << "Inicial " << i + 1 << ": " << starters[i]->fullName();
            if (it) log << ", con " << itemName(*it);
            log << "\n";
        }
        sep();
    }

    // ---------- estáticos ----------
    if (logStatics) {
        title("stp");
        auto now = rh_.getStaticPokemon();
        std::map<std::string, int> seen;
        for (size_t i = 0; i < origStatics_.size() && i < now.size(); i++) {
            std::string o = staticString(origStatics_[i], settings_.staticLevelModified);
            log << o;
            if (seen.count(o)) log << "(" << ++seen[o] << ")";
            else seen[o] = 1;
            log << " => " << staticString(now[i], settings_.staticLevelModified) << "\n";
        }
        sep();
    }

    // ---------- intercambios ----------
    if (logTrades) {
        title("igt");
        auto now = rh_.getInGameTrades();
        TextTable t(6);
        t.addRow({"PEDÍA", "DABA", "MOTE", "PIDE AHORA", "DA AHORA", "MOTE NUEVO"});
        for (size_t i = 0; i < origTrades_.size() && i < now.size(); i++) {
            const auto& o = origTrades_[i];
            const auto& n = now[i];
            t.addRow({o.requestedSpecies ? o.requestedSpecies->fullName() : "Cualquiera", o.givenSpecies()->fullName(), o.nickname,
                      n.requestedSpecies ? n.requestedSpecies->fullName() : "Cualquiera", n.givenSpecies()->fullName(), n.nickname});
        }
        log << t.str();
        sep();
    }

    // ---------- actualizaciones de movimientos ----------
    if (logMU) {
        title("mu");
        log << "Movimientos actualizados a los datos de la " << settings_.updateMovesToGeneration << ".ª generación:\n\n";
        static const char* names[] = {"POTENCIA", "PP", "PRECISIÓN", "TIPO", "CATEGORÍA"};
        for (const auto& [num, ups] : moveUpdater_.updates) {
            log << moveName(num) << ":\n";
            for (const auto& [t, v] : ups) {
                log << "\t" << padR(names[int(t)], 9) << ": ";
                switch (t) {
                    case MoveUpdater::UpdateType::POWER:
                    case MoveUpdater::UpdateType::PP: log << fmt("%4d -> %4d\n", int(v.before), int(v.after)); break;
                    case MoveUpdater::UpdateType::ACCURACY: log << fmt("%3.0f%% -> %3.0f%%\n", v.before, v.after); break;
                    case MoveUpdater::UpdateType::TYPE:
                        log << (v.before < 0 ? "???" : es::tipo(Type(int(v.before)))) << " -> " << es::tipo(Type(int(v.after))) << "\n";
                        break;
                    case MoveUpdater::UpdateType::CATEGORY:
                        log << es::categoryName(MoveCategory(int(v.before))) << " -> " << es::categoryName(MoveCategory(int(v.after))) << "\n";
                        break;
                }
            }
        }
        sep();
    }

    // ---------- datos de movimientos ----------
    if (logMD) {
        title("md");
        bool names = moveNames_.isChangesMade();
        bool split = rh_.hasPhysicalSpecialSplit();
        std::vector<std::string> head = {"NÚM"};
        if (names) head.push_back("NOMBRE ANTIGUO");
        for (const char* h : {"NOMBRE", "TIPO", "POTENCIA", "PRECISIÓN", "PP"}) head.push_back(h);
        if (split) head.push_back("CATEGORÍA");
        TextTable t(static_cast<int>(head.size()));
        t.addRow(head);
        for (size_t i = 0; i < moves.size(); i++) {
            if (!moves[i]) continue;
            const Move& m = *moves[i];
            std::vector<std::string> r = {std::to_string(m.internalId)};
            if (names) r.push_back(i < origMoveNames_.size() && origMoveNames_[i] ? *origMoveNames_[i] : "???");
            r.push_back(m.name);
            r.push_back(es::tipo(m.type));
            r.push_back(std::to_string(m.power));
            r.push_back(std::to_string(static_cast<int>(m.hitratio)) + "%");
            r.push_back(std::to_string(m.pp));
            if (split) r.push_back(es::categoryName(m.category));
            t.addRow(r);
        }
        log << t.str();
        sep();
    }

    // ---------- movimientos de cada Pokémon ----------
    if (logMovesets) {
        title("pms");
        if (metronome) {
            log << "Modo «solo Metrónomo»: todos los Pokémon aprenden únicamente Metrónomo.\n";
        } else {
            auto learnt = rh_.getMovesLearnt();
            auto egg = rh_.getEggMoves();
            auto fmtMove = [&](int id, Species* learner) {
                if (id < 0 || id >= static_cast<int>(moves.size()) || !moves[id]) return std::string("movimiento no válido");
                const Move& m = *moves[id];
                std::string s = padR(m.name, rh_.getMaxMoveNameLength()) + "| " + padR(es::tipo(m.type), 9) + " | " +
                                padR(es::categoryName(m.category), 8) + " | POT=" +
                                (m.category == MoveCategory::STATUS ? std::string(" --") : fmt("%3d", m.power)) + fmt(" | PP=%2d | PREC=%3.0f%%", m.pp, m.hitratio);
                if (m.category != MoveCategory::STATUS && learner->hasType(m.type, false)) s += " (mismo tipo)";
                return s;
            };
            for (Species* pk : rh_.getSpeciesInclFormes()) {
                if (!pk || pk->isEssentiallyCosmetic()) continue;
                log << fmt("%03d ", pk->baseNumber()) << pk->fullName() << " -> ";
                auto evos = pk->evolvedSpecies(false).toVector();
                std::sort(evos.begin(), evos.end(), [](Species* a, Species* b) { return a->number() < b->number(); });
                if (evos.empty()) log << " (no evoluciona)\n";
                for (size_t i = 0; i < evos.size(); i++) log << (i ? ", " : "") << evos[i]->fullName() << (i + 1 == evos.size() ? "\n" : "");
                const auto& b = pk->baseStats();
                log << "  PS| ATQ| DEF|AESP|DESP| VEL\n" << fmt("%4d|%4d|%4d|%4d|%4d|%4d\n", b.getHp(), b.attack, b.defense, b.spatk, b.spdef, b.speed);
                auto it = learnt.find(pk->number());
                if (it != learnt.end())
                    for (const auto& ml : it->second) {
                        if (ml.level == 0) log << "Al evolucionar: ";
                        else log << fmt("Nivel %-2d: ", ml.level);
                        log << fmtMove(ml.move, pk) << "\n";
                    }
                auto e = egg.find(pk->number());
                if (e != egg.end() && !e->second.empty()) {
                    log << "Movimientos huevo:\n";
                    for (int m : e->second) log << " - " << fmtMove(m, pk) << "\n";
                }
                log << "\n";
            }
        }
        sep();
    }

    // ---------- entrenadores ----------
    if (logTrainers) {
        title("lvlc");
        bool frlg = rh_.generationOfPokemon() == 3 && rh_.romType() == 3;
        auto maxLevel = [](const Trainer& t) {
            int m = 0;
            for (const auto& p : t.pokemon) m = std::max(m, p->level);
            return m;
        };
        std::vector<std::string> order;
        std::map<std::string, const Trainer*> bosses;
        for (const auto& t : rh_.getTrainers()) {
            const std::string tag = t->tag.value_or("");
            bool leader = t->tag && ((jc::startsWith(tag, "GYM") && jc::endsWith(tag, "-LEADER")) || jc::startsWith(tag, "ELITE") ||
                                     jc::startsWith(tag, "CHAMPION"));
            if (!(leader || (frlg && tag == "RIVAL8-0"))) continue;
            std::string n = t->fullDisplayName();
            auto f = bosses.find(n);
            if (f == bosses.end()) {
                bosses[n] = t.get();
                order.push_back(n);
            } else if (maxLevel(*f->second) > maxLevel(*t)) {
                f->second = t.get();
            }
        }
        std::vector<const Trainer*> sorted;
        for (const auto& n : order) sorted.push_back(bosses[n]);
        std::stable_sort(sorted.begin(), sorted.end(), [&](const Trainer* a, const Trainer* b) { return maxLevel(*a) < maxLevel(*b); });
        for (const Trainer* t : sorted) log << t->fullDisplayName() << " - nivel máximo: " << maxLevel(*t) << "\n";
        sep();

        title("tp");
        static const char* styles[] = {"Individual", "Doble", "Triple", "Rotatorio"};
        bool prevCustom = false;
        for (const auto& t : rh_.getTrainers()) {
            bool custom = t->pokemonHaveCustomMoves() &&
                          std::any_of(t->pokemon.begin(), t->pokemon.end(), [](const TrainerPokemonPtr& p) { return !p->resetMoves; });
            if (custom && !prevCustom) log << "\n";
            prevCustom = custom;
            log << "#" << t->index << " ";
            std::string now = t->fullDisplayName();
            if (!now.empty()) {
                std::string orig = t->index < static_cast<int>(origTrainerNames_.size()) ? origTrainerNames_[t->index] : "";
                if (trainerNames_.isChangesMade()) log << "(" << orig << " => " << now << ")";
                else log << "(" << now << ")";
            }
            if (t->offset != 0) log << fmt("@%X", t->offset);
            if (custom) {
                log << "\n";
                for (const auto& p : t->pokemon) {
                    log << tpString(*p);
                    if (rh_.abilitiesPerSpecies() != 0) log << ", habilidad: " << rh_.abilityName(rh_.getAbilityForTrainerPokemon(*p));
                    log << " - ";
                    bool first = true;
                    for (int m : p->moves) {
                        if (!m) continue;
                        log << (first ? "" : ", ") << moveName(m);
                        first = false;
                    }
                    log << "\n";
                }
            } else {
                log << " - ";
                for (size_t i = 0; i < t->pokemon.size(); i++) log << (i ? ", " : "") << tpString(*t->pokemon[i]);
            }
            if (settings_.settingBattleStyle.isBattleStyleChanged())
                log << " (estilo de combate: " << styles[int(t->currBattleStyle->style)] << ")";
            log << "\n";
        }
        sep();
    }

    // ---------- salvajes ----------
    if (logWild) {
        title("wp");
        bool timeBased = settings_.useTimeBasedEncounters || (!settings_.randomizeWildPokemon && settings_.wildLevelsModified);
        int i = 0;
        for (const auto& area : rh_.getSortedEncounters(timeBased)) {
            if (area.encounterType == EncounterType::UNUSED) continue;
            i++;
            if (area.displayName.empty()) log << fmt("Zona n.º %d (frecuencia=%d)\n", i, area.rate);
            else log << fmt("Zona n.º %d - ", i) << area.displayName << fmt(" (frecuencia=%d)\n", area.rate);
            for (const auto& e : area.encounters) {
                if (e->maxLevel > 0 && e->maxLevel != e->level) log << e->species()->fullName() << fmt(" Nv%d-%d\n", e->level, e->maxLevel);
                else log << e->species()->fullName() << fmt(" Nv%d\n", e->level);
            }
            log << "\n";
        }
        sep();
    }

    // ---------- MT ----------
    if (logTM) {
        title("tm");
        if (metronome) {
            log << "Modo «solo Metrónomo»: todas las MT contienen Metrónomo.\n";
        } else {
            auto tms = rh_.getTMMoves();
            for (size_t i = 0; i < tms.size(); i++) log << fmt("MT%02d ", int(i + 1)) << moveName(tms[i]) << "\n";
        }
        sep();
    }
    auto logCompat = [&](const std::map<int, std::vector<bool>>& compat, const std::vector<int>& mvs, const char* byMove,
                         const std::function<std::string(size_t)>& mvName) {
        auto spName = [&](int n) {
            Species* s = rh_.getSpeciesInclFormes().at(n);
            return fmt("#%03d ", s->baseNumber()) + s->fullName();
        };
        log << "--Por Pokémon:--\n";
        for (const auto& [n, f] : compat) {
            log << spName(n);
            int j = 0;
            for (size_t i = 0; i < mvs.size(); i++) {
                if (!f[i + 1]) continue;
                if (j != 0) log << ", ";
                if (j % 5 == 0) log << "\n\t";
                log << mvName(i);
                j++;
            }
            log << "\n";
        }
        log << "\n" << byMove;
        for (size_t i = 0; i < mvs.size(); i++) {
            log << mvName(i);
            int j = 0;
            for (const auto& [n, f] : compat) {
                if (!f[i + 1]) continue;
                if (j != 0) log << ", ";
                if (j % 5 == 0) log << "\n\t";
                log << spName(n);
                j++;
            }
            log << "\n";
        }
    };
    if (logTMC) {
        title("tmc");
        if (settings_.fullHMCompat) log << "Todos los Pokémon pueden aprender todas las MO.\n";
        if (settings_.tmsHmsCompatibilityMod == S::TMsHMsCompatibilityMod::FULL) {
            log << "Todos los Pokémon pueden aprender todas las MT y MO.\n";
        } else if (settings_.tmsHmsCompatibilityMod != S::TMsHMsCompatibilityMod::UNCHANGED) {
            auto list = rh_.getTMMoves();
            int tmCount = rh_.getTMCount();
            if (!settings_.fullHMCompat) {
                auto hms = rh_.getHMMoves();
                list.insert(list.end(), hms.begin(), hms.end());
            }
            logCompat(rh_.getTMHMCompatibility(), list, "--Por MT/MO:--\n", [&](size_t i) {
                return static_cast<int>(i) < tmCount ? fmt("MT%02d ", int(i + 1)) + moveName(list[i])
                                                     : fmt("MO%02d ", int(i + 1) - tmCount) + moveName(list[i]);
            });
        }
        sep();
    }
    if (logMT) {
        title("mt");
        if (metronome) {
            log << "Modo «solo Metrónomo»: todos los tutores enseñan Metrónomo.\n";
        } else {
            auto now = rh_.getMoveTutorMoves();
            for (size_t i = 0; i < now.size() && i < origMTMoves_.size(); i++)
                log << padR(moveName(origMTMoves_[i]), 12) << " -> " << moveName(now[i]) << "\n";
        }
        sep();
    }
    if (logMTC) {
        title("mtc");
        if (settings_.moveTutorsCompatibilityMod == S::MoveTutorsCompatibilityMod::FULL) {
            log << "Todos los Pokémon pueden aprender todos los movimientos de los tutores.\n";
        } else {
            auto list = rh_.getMoveTutorMoves();
            logCompat(rh_.getMoveTutorCompatibility(), list, "--Por movimiento de tutor:--\n", [&](size_t i) { return moveName(list[i]); });
        }
        sep();
    }

    // ---------- tiendas y Recogida ----------
    if (logShops) {
        title("sh");
        if (settings_.addCheapRareCandiesToShops) log << "Se han añadido Caramelos Raros baratos a casi todas las tiendas.\n\n";
        if (settings_.shopItemsMod != S::ShopItemsMod::UNCHANGED) {
            log << "--Tiendas especiales:--\n\n";
            for (const auto& shop : rh_.getShops()) {
                if (!shop.isSpecialShop) continue;
                log << shop.name << "\n";
                for (int it : shop.items) log << "- " << itemName(it) << "\n";
                log << "\n";
            }
        }
        sep();
    }
    if (logPickup) {
        title("pu");
        auto pickup = rh_.getPickupItems();
        for (int r = 0; r < 10; r++) {
            log << fmt("Nivel %d-%d\n", r * 10 + 1, (r + 1) * 10);
            std::map<int, std::vector<std::string>> byProb;
            for (const auto& p : pickup) {
                int pr = p.probabilities[r];
                if (byProb.count(pr)) byProb[pr].push_back(itemName(p.item));
                else if (pr > 0) byProb[pr] = {itemName(p.item)};
            }
            for (auto it = byProb.rbegin(); it != byProb.rend(); ++it) {
                log << it->first << "%: ";
                for (size_t i = 0; i < it->second.size(); i++) log << (i ? ", " : "") << it->second[i];
                log << "\n";
            }
            log << "\n";
        }
        sep();
    }

    // ---------- tabla de tipos ----------
    if (logTEU) {
        title("teu");
        log << "Tabla de tipos actualizada a la de la " << (rh_.generationOfPokemon() == 1 ? 2 : 6) << ".ª generación:\n\n";
        for (const auto& [att, m] : typeEffUpdater_.updates) {
            log << es::tipo(att) << " usado contra...\n";
            for (const auto& [def, v] : m)
                log << "\t" << padR(es::tipo(def), 9) << ":   " << padR(effName(v.first), 18) << " -> " << effName(v.second) << "\n";
        }
        sep();
    }
    if (logTE) {
        title("te");
        TypeTable tt = rh_.getTypeTable();
        size_t maxLen = 0;
        std::vector<std::u32string> up;
        for (Type t : tt.types()) {
            up.push_back(jc::utf8ToU32(jc::toUpperCase(es::tipo(t))));
            maxLen = std::max(maxLen, up.back().size());
        }
        log << std::string(maxLen + 1, ' ');
        for (const auto& n : up) log << jc::u32ToUtf8(n.substr(0, 3)) << std::string(3 - std::min<size_t>(3, n.size()), ' ') << " ";
        log << "\n";
        for (size_t a = 0; a < tt.types().size(); a++) {
            log << jc::u32ToUtf8(up[a]) << std::string(maxLen - up[a].size(), ' ');
            for (Type d : tt.types()) log << "|" << effSymbol(tt.getEffectiveness(tt.types()[a], d));
            log << "|\n";
        }
        log << "Atacante (izquierda), defensor (arriba)\n";
        for (Effectiveness e : {Effectiveness::NEUTRAL, Effectiveness::DOUBLE, Effectiveness::HALF, Effectiveness::ZERO})
            log << effSymbol(e) << " = " << effName(e) << "\n";
        sep();
    }

    // ---------- estadísticas y diagnóstico ----------
    title("stat");
    log << "Tiempo: " << elapsedMs << " ms\n";
    log << "Llamadas al generador (no cosméticas): " << nonCosmetic_.calls() << "\n";
    log << "Llamadas al generador (cosméticas)   : " << cosmetic_.calls() << "\n";
    log << "Llamadas al generador (total)        : " << nonCosmetic_.calls() + cosmetic_.calls() << "\n";
    sep();
    title("dg");
    if (!rh_.isRomValid())
        log << "La ROM cargada no es una ROM oficial sin modificar.\nRandomizar hacks o volcados defectuosos no está soportado y puede dar problemas.\n";
    log << rh_.romDiagnostics();
}

}  // namespace pr
