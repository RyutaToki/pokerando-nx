#include "random/name_randomizers.hpp"

#include <algorithm>
#include <climits>
#include <unordered_map>

#include "constants/move_ids.hpp"
#include "random/custom_names.hpp"
#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr {

namespace {
using NameLists = std::unordered_map<std::string, std::vector<std::string>>;

// readNameListFile: líneas "[CLAVE] (palabra) (palabra)..."
NameLists readNameList(const char* path) {
    NameLists r;
    auto f = embeddedFile(path);
    if (!f) return r;
    std::string data(*f);
    size_t pos = 0;
    while (pos <= data.size()) {
        size_t nl = data.find('\n', pos);
        if (nl == std::string::npos) nl = data.size();
        std::string line = jc::trim(data.substr(pos, nl - pos));
        pos = nl + 1;
        if (line.empty() || line[0] == '#') continue;
        size_t b = std::string::npos, e = 0;
        for (size_t i = 0; i < line.size(); i++) {  // \[([^]]+)]
            if (line[i] != '[') continue;
            size_t j = i + 1;
            while (j < line.size() && line[j] != ']') j++;
            if (j < line.size() && j > i + 1) {
                b = i;
                e = j;
                break;
            }
        }
        if (b == std::string::npos) continue;
        std::string key = line.substr(b + 1, e - b - 1);
        std::vector<std::string> words;
        for (size_t i = e + 1; i < line.size(); i++) {  // \(([^)]+)\)
            if (line[i] != '(') continue;
            size_t j = i + 1;
            while (j < line.size() && line[j] != ')') j++;
            if (j < line.size() && j > i + 1) {
                words.push_back(line.substr(i + 1, j - i - 1));
                i = j;
            } else if (j >= line.size()) {
                break;
            }
        }
        if (!words.empty()) r[key] = words;
    }
    return r;
}
const NameLists& typeNames() { static NameLists n = readNameList("move_names/TypeMoveNames.txt"); return n; }
const NameLists& catNames() { static NameLists n = readNameList("move_names/CatMoveNames.txt"); return n; }
const NameLists& extraNames() { static NameLists n = readNameList("move_names/SubCatMoveNames.txt"); return n; }
const std::vector<std::string>* extra(const char* k) {
    auto it = extraNames().find(k);
    return it == extraNames().end() ? nullptr : &it->second;
}
size_t javaLength(const std::string& s) {  // String.length(): unidades UTF-16
    size_t n = 0;
    for (char32_t c : jc::utf8ToU32(s)) n += c > 0xFFFF ? 2 : 1;
    return n;
}

const std::vector<int> healMoves = {moves::recover, moves::softBoiled, moves::rest, moves::milkDrink, moves::morningSun,
                                    moves::synthesis, moves::moonlight, moves::swallow, moves::wish, moves::ingrain,
                                    moves::refresh, moves::slackOff, moves::roost, moves::healingWish, moves::aquaRing,
                                    moves::healOrder, moves::lunarDance, moves::healBell, moves::aromatherapy,
                                    moves::healPulse, moves::floralHealing, moves::purify, moves::shoreUp, moves::lifeDew,
                                    moves::jungleHealing};

const char* statusKey(StatusType t) {
    switch (t) {
        case StatusType::POISON:
        case StatusType::TOXIC_POISON: return "INFLICT_POISON";
        case StatusType::BURN: return "INFLICT_BURN";
        case StatusType::FREEZE: return "INFLICT_FREEZE";
        case StatusType::PARALYZE: return "INFLICT_PARALYZE";
        case StatusType::SLEEP: return "INFLICT_SLEEP";
        case StatusType::CONFUSION: return "INFLICT_CONFUSION";
        default: return nullptr;
    }
}

const std::vector<std::string>* statusWords(const Move& mv) {
    if (std::find(healMoves.begin(), healMoves.end(), mv.number) != healMoves.end()) return extra("STATUS_HEAL");
    if (mv.statusType != StatusType::NONE) {
        if (const char* k = statusKey(mv.statusType)) {
            auto w = extra(k);
            if (w && !w->empty()) return w;
        }
    }
    if (mv.isTrapMove) return extra("STATUS_TRAP");
    using S = StatChangeMoveType;
    if (mv.statChangeMoveType != S::NONE_OR_UNKNOWN) {
        if (mv.statChangeMoveType == S::NO_DAMAGE_USER) {
            bool pos = false;
            for (const auto& sc : mv.statChanges)
                if (sc.type != StatChangeType::NONE && sc.stages > 0) {
                    pos = true;
                    break;
                }
            return extra(pos ? "STATUS_BUFF" : "STATUS_DEBUFF");
        }
        if (mv.statChangeMoveType == S::NO_DAMAGE_TARGET) return extra("STATUS_DEBUFF");
        if (mv.statChangeMoveType == S::NO_DAMAGE_ALL || mv.statChangeMoveType == S::NO_DAMAGE_ALLY) return extra("STATUS_BUFF");
    }
    return nullptr;
}

std::vector<std::string> actionWords(const Move& mv) {
    auto ok = [](const std::vector<std::string>* w) { return w && !w->empty(); };
    if (mv.isPunchMove && ok(extra("PUNCH"))) return *extra("PUNCH");
    if (mv.isSoundMove && ok(extra("SOUND"))) return *extra("SOUND");
    if (mv.absorbPercent > 0 && mv.power > 0 && ok(extra("DRAIN"))) return *extra("DRAIN");
    if (mv.category == MoveCategory::STATUS) {
        auto w = statusWords(mv);
        if (ok(w)) return *w;
    }
    if (mv.isTrapMove && ok(extra("STATUS_TRAP"))) return *extra("STATUS_TRAP");
    auto it = catNames().find(moveCategoryName(mv.category));
    if (it == catNames().end() || it->second.empty()) return {"Strike"};
    return it->second;
}
}  // namespace

std::string MoveNameRandomizer::randomMoveName(const Move& mv, int maxLen) {
    std::vector<std::string> tn{"Attack"};
    if (mv.type) {
        auto it = typeNames().find(typeName(*mv.type));
        if (it != typeNames().end() && !it->second.empty()) tn = it->second;
    }
    auto aw = actionWords(mv);
    for (int i = 0; i < 50; i++) {
        const std::string& tw = tn[random.nextInt(static_cast<int>(tn.size()))];
        const std::string& a = aw[random.nextInt(static_cast<int>(aw.size()))];
        std::string name = tw + " " + a;
        if (static_cast<int>(javaLength(name)) > maxLen) {
            std::string compact = tw + a;
            if (static_cast<int>(javaLength(compact)) <= maxLen) name = compact;
            else continue;
        }
        if (used_.count(name)) continue;
        used_.insert(name);
        return name;
    }
    return "Attack Strike";
}

void MoveNameRandomizer::randomizeMoveNames() {
    used_.clear();
    auto& moves = const_cast<std::vector<std::optional<Move>>&>(romHandler.getMoves());
    int maxLen = romHandler.getMaxMoveNameLength();
    std::vector<std::string> names;
    for (auto& m : moves)
        if (m) names.push_back(m->name);
    bool upper = detectUpperCaseNames(names);
    for (auto& m : moves) {
        if (!m || m->internalId == moves::struggle) continue;
        std::string n = randomMoveName(*m, maxLen);
        m->name = upper ? jc::toUpperCase(n) : n;
    }
    changesMade = true;
}

TrainerNameRandomizer::CustomNames TrainerNameRandomizer::readList(const std::vector<std::string>& names, int limit) {
    CustomNames c;
    for (const auto& n : names) {
        int len = romHandler.internalStringLength(n);
        if (len <= limit) {
            c.all.push_back(n);
            c.byLength[len].push_back(n);
        }
    }
    return c;
}

void TrainerNameRandomizer::randomizeTrainerNames() {
    if (!romHandler.canChangeTrainerText()) return;
    static const std::vector<std::string> repeated = {"GRUNT", "EXECUTIVE", "SHADOW", "ADMIN", "GOON", "EMPLOYEE"};
    const int maxTries = 10000, maxNameLen = 10;
    auto raw = CustomNamesSet::readEmbedded();
    auto singles = readList(raw.trainerNames, maxNameLen);
    auto doubles = readList(raw.doublesTrainerNames, maxNameLen);
    std::vector<std::string> current;
    for (const auto& t : romHandler.getTrainers())
        if (t->name) current.push_back(*t->name);
    // (Gen 3 no tiene clases de entrenador "personales")
    auto mode = romHandler.trainerNameMode();
    int maxLength = romHandler.maxTrainerNameLength();
    int totalMax = romHandler.maxSumOfTrainerNameLengths();
    auto tcLens = romHandler.getTCNameLengthsByTrainer();
    std::unordered_map<std::string, std::string> translation;
    std::vector<std::string> out;
    bool success = false;
    int tries = 0;
    using M = RomHandler::TrainerNameMode;
    while (!success && tries < maxTries) {
        success = true;
        translation.clear();
        out.clear();
        int total = 0;
        int idx = -1;
        for (const auto& name : current) {
            idx++;
            auto it = translation.find(name);
            if (it != translation.end() &&
                std::find(repeated.begin(), repeated.end(), jc::toUpperCase(name)) == repeated.end()) {
                out.push_back(it->second);
                total += romHandler.internalStringLength(it->second);
            } else {
                bool dbl = name.find('&') != std::string::npos;
                const CustomNames& cn = dbl ? doubles : singles;
                const std::vector<std::string>* pickFrom = &cn.all;
                int len = romHandler.internalStringLength(name);
                if (mode == M::SAME_LENGTH) {
                    auto b = cn.byLength.find(len);
                    pickFrom = b == cn.byLength.end() ? nullptr : &b->second;
                }
                std::string to = name;
                int ctl = len;
                if (pickFrom && !pickFrom->empty() && len > 0) {
                    int inner = 0;
                    to = (*pickFrom)[random.nextInt(static_cast<int>(pickFrom->size()))];
                    ctl = romHandler.internalStringLength(to);
                    while ((mode == M::MAX_LENGTH && ctl > maxLength) ||
                           (mode == M::MAX_LENGTH_WITH_CLASS && ctl + tcLens.at(idx) > maxLength)) {
                        inner++;
                        if (inner == 100) {
                            to = name;
                            ctl = len;
                            break;
                        }
                        to = (*pickFrom)[random.nextInt(static_cast<int>(pickFrom->size()))];
                        ctl = romHandler.internalStringLength(to);
                    }
                }
                translation[name] = to;
                out.push_back(to);
                total += ctl;
            }
            if (total > totalMax) {
                success = false;
                tries++;
                break;
            }
        }
    }
    if (!success)
        throw RandomizationException("Could not randomize trainer names within 10000 tries.\nPlease add some shorter names to your custom trainer names.");
    if (detectUpperCaseNames(current))
        for (auto& n : out) n = jc::toUpperCase(n);
    size_t i = 0;
    for (const auto& t : romHandler.getTrainers())
        if (t->name) t->name = out.at(i++);
    changesMade = true;
}

void TrainerNameRandomizer::randomizeTrainerClassNames() {
    if (!romHandler.canChangeTrainerText()) return;
    auto raw = CustomNamesSet::readEmbedded();
    auto singles = readList(raw.trainerClasses, INT_MAX);
    auto doubles = readList(raw.doublesTrainerClasses, INT_MAX);
    std::vector<std::string> current;
    for (const auto& tc : romHandler.getTrainerClasses()) current.push_back(tc->name);
    auto dblClasses = romHandler.getDoublesTrainerClasses();
    bool sameLen = romHandler.fixedTrainerClassNamesLength();
    int maxLength = romHandler.maxTrainerClassNameLength();
    std::unordered_map<std::string, std::string> translation;
    std::vector<std::string> out;
    for (size_t i = 0; i < current.size(); i++) {
        const std::string& name = current[i];
        auto it = translation.find(name);
        if (it != translation.end()) {
            out.push_back(it->second);
            continue;
        }
        bool dbl = std::find(dblClasses.begin(), dblClasses.end(), static_cast<int>(i)) != dblClasses.end();
        const CustomNames& cn = dbl ? doubles : singles;
        const std::vector<std::string>* pickFrom = &cn.all;
        if (sameLen) {
            auto b = cn.byLength.find(romHandler.internalStringLength(name));
            pickFrom = b == cn.byLength.end() ? nullptr : &b->second;
        }
        std::string to = name;
        if (pickFrom && !pickFrom->empty()) {
            to = (*pickFrom)[random.nextInt(static_cast<int>(pickFrom->size()))];
            while (static_cast<int>(javaLength(to)) > maxLength) to = (*pickFrom)[random.nextInt(static_cast<int>(pickFrom->size()))];
        }
        translation[name] = to;
        out.push_back(to);
    }
    if (detectUpperCaseNames(current))
        for (auto& n : out) n = jc::toUpperCase(n);
    size_t i = 0;
    for (const auto& tc : romHandler.getTrainerClasses()) tc->name = out.at(i++);
    changesMade = true;
}

}  // namespace pr
