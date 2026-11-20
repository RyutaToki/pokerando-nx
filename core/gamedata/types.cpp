#include "gamedata/types.hpp"

#include <cmath>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace pr {

std::optional<Type> typeFromName(std::string_view name) {
    for (int i = 0; i <= static_cast<int>(Type::FAIRY); i++)
        if (name == typeName(static_cast<Type>(i))) return static_cast<Type>(i);
    return std::nullopt;
}

const char* typeName(Type t) {
    static const char* names[] = {"NORMAL", "FIGHTING", "FLYING", "GRASS", "WATER", "FIRE", "ROCK", "GROUND",
                                  "PSYCHIC", "BUG", "DRAGON", "ELECTRIC", "GHOST", "POISON", "ICE", "STEEL",
                                  "DARK", "FAIRY"};
    return names[static_cast<int>(t)];
}

std::optional<ExpCurve> expCurveFromByte(int b) {
    switch (static_cast<int8_t>(b)) {
        case 0: return ExpCurve::MEDIUM_FAST;
        case 1: return ExpCurve::ERRATIC;
        case 2: return ExpCurve::FLUCTUATING;
        case 3: return ExpCurve::MEDIUM_SLOW;
        case 4: return ExpCurve::FAST;
        case 5: return ExpCurve::SLOW;
        default: return std::nullopt;
    }
}

int expCurveToByte(ExpCurve c) {
    switch (c) {
        case ExpCurve::MEDIUM_FAST: return 0;
        case ExpCurve::ERRATIC: return 1;
        case ExpCurve::FLUCTUATING: return 2;
        case ExpCurve::MEDIUM_SLOW: return 3;
        case ExpCurve::FAST: return 4;
        case ExpCurve::SLOW: return 5;
    }
    return 0;
}

const char* expCurveName(ExpCurve c) {
    static const char* names[] = {"SLOW", "MEDIUM_SLOW", "MEDIUM_FAST", "FAST", "ERRATIC", "FLUCTUATING"};
    return names[static_cast<int>(c)];
}

namespace {
constexpr EggGroup kEggInOrder[] = {
    EggGroup::MONSTER, EggGroup::WATER_1, EggGroup::BUG, EggGroup::FLYING, EggGroup::FIELD,
    EggGroup::FAIRY, EggGroup::GRASS, EggGroup::HUMAN_LIKE, EggGroup::WATER_3, EggGroup::MINERAL,
    EggGroup::AMORPHOUS, EggGroup::WATER_2, EggGroup::DITTO, EggGroup::DRAGON, EggGroup::UNDISCOVERED};
}

EggGroup eggGroupFromID(int id) {
    if (id < 1 || id > 15) throw std::out_of_range("id de grupo huevo fuera de rango");
    return kEggInOrder[id - 1];
}

int eggGroupToID(EggGroup g) {
    for (int i = 0; i < 15; i++) {
        if (kEggInOrder[i] == g) return i + 1;
    }
    return 0;
}

const char* eggGroupName(EggGroup g) {
    static const char* names[] = {"MONSTER", "BUG", "FLYING", "FIELD", "FAIRY", "GRASS", "HUMAN_LIKE",
                                  "MINERAL", "AMORPHOUS", "DRAGON", "WATER_1", "WATER_2", "WATER_3",
                                  "DITTO", "UNDISCOVERED"};
    return names[static_cast<int>(g)];
}

EVYield EVYield::fromWord(int w) {
    EVYield e;
    e.hp = w & 3;
    e.attack = (w >> 2) & 3;
    e.defense = (w >> 4) & 3;
    e.speed = (w >> 6) & 3;
    e.spatk = (w >> 8) & 3;
    e.spdef = (w >> 10) & 3;
    return e;
}

int EVYield::toWord() const {
    return hp | (attack << 2) | (defense << 4) | (speed << 6) | (spatk << 8) | (spdef << 10);
}

double javaDoubleSum(const double* v, size_t n) {
    double s0 = 0, s1 = 0, simple = 0;
    for (size_t i = 0; i < n; i++) {
        double tmp = v[i] - s1;
        double velvel = s0 + tmp;
        s1 = (velvel - s0) - tmp;
        s0 = velvel;
        simple += v[i];
    }
    double tmp = s0 - s1;
    if (std::isnan(tmp) && std::isinf(simple)) return simple;
    return tmp;
}

namespace {
void rangeCheck(int v, const char* name) {
    if (v < 0 || v > BaseStats::STAT_MAX) throw std::invalid_argument(std::string(name) + " must be between 0-255");
}
// Double.compare de Java
int javaDoubleCompare(double a, double b) {
    if (a < b) return -1;
    if (a > b) return 1;
    int64_t ab, bb;
    std::memcpy(&ab, &a, 8);
    std::memcpy(&bb, &b, 8);
    if (std::isnan(a)) ab = 0x7ff8000000000000LL;
    if (std::isnan(b)) bb = 0x7ff8000000000000LL;
    return ab == bb ? 0 : (ab < bb ? -1 : 1);
}

// java.util.PriorityQueue (montículo binario) con un comparador
template <class Cmp>
struct JavaPriorityQueue {
    std::vector<int> q;
    Cmp cmp;
    explicit JavaPriorityQueue(Cmp c) : cmp(c) {}
    void add(int x) {
        int k = static_cast<int>(q.size());
        q.push_back(x);
        while (k > 0) {
            int parent = (k - 1) >> 1;
            int e = q[parent];
            if (cmp(x, e) >= 0) break;
            q[k] = e;
            k = parent;
        }
        q[k] = x;
    }
    std::optional<int> poll() {
        if (q.empty()) return std::nullopt;
        int result = q[0];
        int n = static_cast<int>(q.size()) - 1;
        int x = q[n];
        q.pop_back();
        if (n > 0) {
            int k = 0, half = n >> 1;
            while (k < half) {
                int child = 2 * k + 1;
                int c = q[child];
                int right = child + 1;
                if (right < n && cmp(c, q[right]) > 0) c = q[child = right];
                if (cmp(x, c) <= 0) break;
                q[k] = c;
                k = child;
            }
            q[k] = x;
        }
        return result;
    }
};
}  // namespace

BaseStats::BaseStats(int h, int a, int d, int sa, int sd, int s) {
    rangeCheck(h, "hp");
    rangeCheck(a, "attack");
    rangeCheck(d, "defense");
    rangeCheck(sa, "spatk");
    rangeCheck(sd, "spdef");
    rangeCheck(s, "speed");
    bst_ = h + a + d + sa + sd + s;
    hpRatio = h;
    attackRatio = a;
    defenseRatio = d;
    spatkRatio = sa;
    spdefRatio = sd;
    speedRatio = s;
    calculateStats();
}

BaseStats BaseStats::makeShedinja(int a, int d, int sa, int sd, int s) {
    // super(0, ...) ya llama a calculateStats con las proporciones de Shedinja
    BaseStats b;
    b.shedinja = true;
    rangeCheck(a, "attack");
    rangeCheck(d, "defense");
    rangeCheck(sa, "spatk");
    rangeCheck(sd, "spdef");
    rangeCheck(s, "speed");
    b.bst_ = a + d + sa + sd + s;
    b.attackRatio = a;
    b.defenseRatio = d;
    b.spatkRatio = sa;
    b.spdefRatio = sd;
    b.speedRatio = s;
    b.calculateStats();
    b.bst_ = a + d + sa + sd + s;
    b.bstForPowerLevels_ = b.bst_ * 6 / 5;
    b.calculateStats();
    return b;
}

int BaseStats::bst() const {
    if (shedinja) return bstForPowerLevels_;
    return hp + attack + defense + spatk + spdef + speed;
}

void BaseStats::setBST(int v) {
    if (shedinja) {
        bstForPowerLevels_ = v;
        bst_ = v * 5 / 6;
    } else {
        if (v < 0) throw std::invalid_argument("bst must be positive.");
        bst_ = v;
    }
    calculateStats();
}

void BaseStats::setStatRatios(double h, double a, double d, double sa, double sd, double s) {
    if (shedinja) throw std::logic_error("Use the ShedinjaBaseStats-specific setStatRatios instead.");
    for (double v : {h, a, d, sa, sd, s})
        if (v < 0) throw std::invalid_argument("stat ratio must be positive.");
    hpRatio = h;
    attackRatio = a;
    defenseRatio = d;
    spatkRatio = sa;
    spdefRatio = sd;
    speedRatio = s;
    calculateStats();
}

void BaseStats::setStatRatiosShedinja(double a, double d, double sa, double sd, double s) {
    for (double v : {a, d, sa, sd, s})
        if (v < 0) throw std::invalid_argument("stat ratio must be positive.");
    attackRatio = a;
    defenseRatio = d;
    spatkRatio = sa;
    spdefRatio = sd;
    speedRatio = s;
    calculateStats();
}

void BaseStats::calculateStats() {
    std::vector<double> ratios = shedinja ? std::vector<double>{attackRatio, defenseRatio, spatkRatio, spdefRatio, speedRatio}
                                          : std::vector<double>{hpRatio, attackRatio, defenseRatio, spatkRatio, spdefRatio, speedRatio};
    double total = javaDoubleSum(ratios.data(), ratios.size());
    std::vector<double> raw(ratios.size(), 0.0);
    if (total == 0) {
        if (bst_ != 0) throw std::logic_error("All stat ratios are 0, but BST is not 0.");
    } else {
        for (size_t i = 0; i < ratios.size(); i++) raw[i] = bst_ * (ratios[i] / total);
    }
    std::vector<int> stats(raw.size(), 0);
    auto cmp = [&](int a, int b) {
        return javaDoubleCompare((stats[a] - raw[a] + 0.000001) / raw[a], (stats[b] - raw[b] + 0.000001) / raw[b]);
    };
    JavaPriorityQueue<decltype(cmp)> pq(cmp);
    for (int i = 0; i < static_cast<int>(raw.size()); i++)
        if (raw[i] != 0) pq.add(i);  // las proporciones 0 no reciben puntos
    for (int i = 0; i < bst_; i++) {
        auto off = pq.poll();
        if (!off) throw std::logic_error("Could not distribute all stat points.");
        stats[*off]++;
        if (stats[*off] < STAT_MAX) pq.add(*off);
    }
    if (shedinja) {
        attack = stats[0];
        defense = stats[1];
        spatk = stats[2];
        spdef = stats[3];
        speed = stats[4];
    } else {
        hp = stats[0];
        attack = stats[1];
        defense = stats[2];
        spatk = stats[3];
        spdef = stats[4];
        speed = stats[5];
    }
}

}  // namespace pr
