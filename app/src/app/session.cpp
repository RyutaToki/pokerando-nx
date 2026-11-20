#include "app/session.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <random>

#include "app/shiny.hpp"
#include "util/java_compat.hpp"

namespace app {

using S = pr::Settings;
using Phase = pr::GameRandomizer::Phase;

Session& session() {
    static Session s;
    return s;
}

int64_t newSeed() {
    static std::mt19937_64 rng(static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
    // semillas positivas de hasta 12 cifras, fáciles de apuntar
    return static_cast<int64_t>(rng() % 1000000000000ull);
}

bool applySettingsString(const std::string& str, std::string* error) {
    try {
        S s = S::fromString(pr::jc::trim(str));
        Session& se = session();
        if (se.preview) tweakForRom(s, *se.preview);
        se.opts.s = s;
        return true;
    } catch (const std::exception& e) {
        if (error) *error = e.what();
        return false;
    }
}

bool applyPreset(const Preset& p) {
    if (!applySettingsString(p.settings)) return false;
    Session& se = session();
    se.opts.shiny = p.shiny;
    se.presetName = p.name;
    se.presetSettings = se.opts.s.toString();
    se.presetShiny = p.shiny;
    se.dirty = false;
    return true;
}

const char* phaseName(Phase p) {
    switch (p) {
        case Phase::BASE: return "Pokémon base y tipos";
        case Phase::STARTERS: return "Iniciales";
        case Phase::MOVES: return "Movimientos y MT";
        case Phase::WILD: return "Salvajes";
        case Phase::TRAINERS: return "Entrenadores y líderes";
        case Phase::STATICS_TRADES: return "Regalos e intercambios";
        case Phase::ITEMS: return "Objetos";
        case Phase::EXTRAS: return "Extras";
        case Phase::SAVE: return "Guardar ROM y log";
    }
    return "";
}

std::vector<Phase> activePhases(const OptState& o) {
    const S& s = o.s;
    std::vector<Phase> v;
    if (s.baseStatisticsMod != S::BaseStatisticsMod::UNCHANGED || s.updateBaseStats || s.speciesTypesMod != S::SpeciesTypesMod::UNCHANGED ||
        s.abilitiesMod != S::AbilitiesMod::UNCHANGED || s.evolutionsMod != S::EvolutionsMod::UNCHANGED || s.standardizeEXPCurves ||
        s.typeEffectivenessMod != S::TypeEffectivenessMod::UNCHANGED || s.updateTypeEffectiveness || s.randomizeMovePowers ||
        s.randomizeMoveTypes || s.randomizeMoveNames || s.bstMod != S::BSTMod::UNCHANGED)
        v.push_back(Phase::BASE);
    if (s.startersMod != S::StartersMod::UNCHANGED || s.randomizeStartersHeldItems) v.push_back(Phase::STARTERS);
    if (s.movesetsMod != S::MovesetsMod::UNCHANGED || s.tmsMod != S::TMsMod::UNCHANGED || s.reorderDamagingMoves ||
        s.tmsHmsCompatibilityMod != S::TMsHMsCompatibilityMod::UNCHANGED || s.moveTutorMovesMod != S::MoveTutorMovesMod::UNCHANGED ||
        s.moveTutorsCompatibilityMod != S::MoveTutorsCompatibilityMod::UNCHANGED || s.fullHMCompat)
        v.push_back(Phase::MOVES);
    if (s.randomizeWildPokemon || s.wildLevelsModified || s.useMinimumCatchRate || s.randomizeWildPokemonHeldItems) v.push_back(Phase::WILD);
    if (s.trainersMod != S::TrainersMod::UNCHANGED || s.trainersLevelModified || s.randomizeTrainerNames || s.betterBossTrainerMovesets ||
        s.additionalBossTrainerPokemon > 0 || s.randomizeHeldItemsForBossTrainerPokemon || s.settingBattleStyle.isBattleStyleChanged() ||
        s.trainersEvolveTheirPokemon)
        v.push_back(Phase::TRAINERS);
    if (s.staticPokemonMod != S::StaticPokemonMod::UNCHANGED || s.staticLevelModified || s.inGameTradesMod != S::InGameTradesMod::UNCHANGED)
        v.push_back(Phase::STATICS_TRADES);
    if (s.fieldItemsMod != S::FieldItemsMod::UNCHANGED || s.shopItemsMod != S::ShopItemsMod::UNCHANGED || s.pickupItemsMod != S::PickupItemsMod::UNCHANGED ||
        s.addCheapRareCandiesToShops)
        v.push_back(Phase::ITEMS);
    if (s.pokemonPalettesMod != S::PokemonPalettesMod::UNCHANGED || s.currentMiscTweaks != 0 || s.randomizeIntroMon || o.shiny != 0 ||
        s.changeImpossibleEvolutions)
        v.push_back(Phase::EXTRAS);
    v.push_back(Phase::SAVE);
    return v;
}

static std::string asciiName(const std::string& s) {
    std::string out;
    std::u32string u = pr::jc::utf8ToU32(s);
    for (char32_t c : u) {
        if (c == U' ') continue;
        if (c < 0x80) out += static_cast<char>(c);
        else if (c == U'á' || c == U'Á') out += c == U'á' ? 'a' : 'A';
        else if (c == U'é') out += 'e';
        else if (c == U'í') out += 'i';
        else if (c == U'ó') out += 'o';
        else if (c == U'ú') out += 'u';
        else if (c == U'ñ') out += 'n';
    }
    return out.empty() ? "Pokemon" : out;
}

std::string outputPath(const std::string& romPath, const pr::RomInfo& info, int64_t seed) {
    std::string ext = info.platform == "NDS" ? ".nds" : ".gba";
    return parentDir(romPath) + asciiName(info.shortName) + "_" + std::to_string(seed) + ext;
}

std::vector<std::string> removeOldRandomized(const std::string& romPath, const pr::RomInfo& info, const std::vector<std::string>& keep) {
    std::vector<std::string> removed;
    std::string dir = parentDir(romPath);
    std::string prefix = asciiName(info.shortName) + "_";
    std::error_code ec;
    for (const auto& e : std::filesystem::directory_iterator(dir, ec)) {
        if (!e.is_regular_file(ec)) continue;
        std::string name = e.path().filename().string();
        if (name.compare(0, prefix.size(), prefix) != 0) continue;
        // {Juego}_{solo dígitos}{.gba|.nds|.log.txt}
        size_t i = prefix.size(), j = i;
        while (j < name.size() && std::isdigit(static_cast<unsigned char>(name[j]))) j++;
        if (j == i) continue;
        std::string rest = name.substr(j);
        if (rest != ".gba" && rest != ".nds" && rest != ".log.txt") continue;
        std::string full = dir + name;
        if (full == romPath || std::find(keep.begin(), keep.end(), full) != keep.end()) continue;
        if (std::filesystem::remove(e.path(), ec)) removed.push_back(full);
    }
    return removed;
}

JobResult runRandomization(const std::string& romPath, const OptState& opts, int64_t seed, const std::function<void(Phase)>& onPhase) {
    JobResult r;
    auto t0 = std::chrono::steady_clock::now();
    std::string out, logPath;
    try {
        pr::RomInfo info = pr::detectRom(romPath);
        out = outputPath(romPath, info, seed);
        if (!makeDirs(parentDir(out))) throw std::runtime_error("No se pudo crear la carpeta " + parentDir(out));
        uint64_t need = info.size + (1u << 20);
        uint64_t free = freeSpace(parentDir(out));
        if (free != 0 && free < need)
            throw std::runtime_error("No hay espacio suficiente en la SD: hacen falta " + formatSize(need) + " y quedan " + formatSize(free) + ".");
        auto rh = pr::openRom(romPath);
        S s = opts.s;
        tweakForRom(s, *rh);
        pr::GameRandomizer gr(s, *rh);
        gr.onPhase = onPhase;
        logPath = out.substr(0, out.size() - 4) + ".log.txt";
        if (opts.spoilerLog) {
            std::ofstream log(logPath, std::ios::binary);
            gr.randomize(out, seed, &log);
            if (!log) throw std::runtime_error("Error al escribir el log.");
            r.logOut = logPath;
        } else {
            gr.randomize(out, seed);
        }
        // (un preset con shiny cargado en un juego sin soporte se ignora: las fichas salen desactivadas)
        if (opts.shiny != 0 && shinyPatchAvailable(romPath)) {
            std::string err;
            if (!applyShinyPatch(out, kShinyOdds[opts.shiny].threshold, &err)) throw std::runtime_error(err);
        }
        for (pr::Species* sp : rh->getStarters()) r.starters.push_back(sp ? sp->fullName() : "?");
        r.romOut = out;
        r.ok = true;
        // por defecto, solo se queda la última randomizada de cada juego en la carpeta
        if (!config().keepRandomized) removeOldRandomized(romPath, info, {out, logPath});
    } catch (const std::exception& e) {
        r.error = e.what();
        // no dejar archivos a medias
        if (!out.empty()) std::remove(out.c_str());
        if (!logPath.empty()) std::remove(logPath.c_str());
    }
    r.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    return r;
}

}  // namespace app
