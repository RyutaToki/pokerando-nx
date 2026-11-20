#include "graphics/palette_populate.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

#include "gamedata/species.hpp"
#include "util/embedded.hpp"
#include "util/java_compat.hpp"

namespace pr {

LightDarkMode randomLightDarkMode(JavaRandom& r) {
    if (r.nextDouble() < 0.2) return r.nextBoolean() ? LightDarkMode::LIGHT : LightDarkMode::DARK;
    return LightDarkMode::DEFAULT;
}

// ---------- Gen3to5TypeColors ----------
namespace {
// Color(String): primero un hexadecimal de 6 cifras; si no, tres enteros
Color colorFromString(const std::string& s) {
    for (size_t i = 0; i + 6 <= s.size(); i++) {
        bool ok = true;
        for (size_t j = i; j < i + 6; j++) ok = ok && std::isxdigit(static_cast<unsigned char>(s[j]));
        if (ok) return Color::fromHex(std::stoi(s.substr(i, 6), nullptr, 16));
    }
    std::vector<int> nums;
    for (size_t i = 0; i < s.size() && nums.size() < 3;) {
        if (std::isdigit(static_cast<unsigned char>(s[i]))) {
            size_t j = i;
            while (j < s.size() && std::isdigit(static_cast<unsigned char>(s[j]))) j++;
            nums.push_back(std::stoi(s.substr(i, j - i)));
            i = j;
        } else {
            i++;
        }
    }
    if (nums.size() < 3) throw std::invalid_argument("No color value found in \"" + s + "\".");
    return Color(nums[0], nums[1], nums[2]);
}

std::map<Type, std::vector<TypeColor>> readTypeColors() {
    std::map<Type, std::vector<TypeColor>> map;
    auto f = embeddedFile("palettes/Gen3To5TypeColors.txt");
    if (!f) throw std::runtime_error("falta palettes/Gen3To5TypeColors.txt");
    std::string s(*f);
    std::optional<Type> type;
    std::vector<TypeColor> cur;
    // tokens "\[.*?\]" o "\(.*?\)" (sin saltos de línea dentro)
    for (size_t i = 0; i < s.size(); i++) {
        char open = s[i];
        if (open != '[' && open != '(') continue;
        char close = open == '[' ? ']' : ')';
        size_t j = i + 1;
        while (j < s.size() && s[j] != close && s[j] != '\n' && s[j] != '\r') j++;
        if (j >= s.size() || s[j] != close) continue;
        std::string tok = s.substr(i + 1, j - i - 1);
        if (open == '[') {
            if (type) {
                map[*type] = cur;
                cur.clear();
            }
            auto t = typeFromName(tok);
            if (!t) throw std::runtime_error("tipo desconocido en Gen3To5TypeColors: " + tok);
            type = *t;
        } else {
            cur.push_back(TypeColor{colorFromString(tok), type});
        }
        i = j;
    }
    if (type) map[*type] = cur;
    return map;
}
const std::map<Type, std::vector<TypeColor>>& typeColors() {
    static const auto m = readTypeColors();
    return m;
}
}  // namespace

TypeColor Gen3to5TypeColors::getRandomTypeColor(Type type, JavaRandom& r) {
    auto it = typeColors().find(type);
    if (it == typeColors().end()) return TypeColor{Color::fromHex(0xC0C0C0), type};
    return it->second[r.nextInt(static_cast<int>(it->second.size()))];
}

TypeColor Gen3to5TypeColors::getRandomTypeColor(JavaRandom& r) {
    // RandomColorSelector (HSV) con peso s, x3 si 20 <= h <= 70; límites {0,0,0.6}-{360,1,1}.
    // FVX calcula el peso máximo muestreando con un Random sin semilla; aquí se usa el valor exacto, 3.0.
    const double lo[3] = {0, 0, 0.6}, hi[3] = {360, 1, 1};
    const double maxValue = 3.0;
    auto weight = [](const double* p) {
        double w = p[1];
        if (20 <= p[0] && p[0] <= 70) w *= 3;
        return w;
    };
    for (int attempt = 0; attempt <= 200; attempt++) {
        double p[3];
        for (int i = 0; i < 3; i++) p[i] = r.nextDouble() * (hi[i] - lo[i]) + lo[i];
        double value = r.nextDouble() * maxValue;
        if (attempt == 200) break;
        if (value < weight(p)) return TypeColor{Color::fromHSV(p[0], p[1], p[2]), std::nullopt};
    }
    throw std::runtime_error("Picking point took too long, exceeded 200 attempts.");
}

// ---------- BaseColorMap ----------
Color BaseColorMap::getBaseColor(const TypeColor& k) {
    if (!baseColors_.count(key(k))) add(k);
    return baseColors_.at(key(k));
}

LightDarkMode BaseColorMap::getLightDarkMode(const TypeColor& k) {
    if (!baseColors_.count(key(k))) add(k);
    return modes_.at(key(k));
}

void BaseColorMap::add(const TypeColor& k) {
    Color t;
    for (int i = 0; i < 3; i++) {
        int c = k.color.getComp(i);
        int randMin = static_cast<int>(-0.032 * c + 7.1);
        int randMax = static_cast<int>(-0.042 * c + 17.19);
        int a = random_.nextInt(randMax - randMin);
        int b = random_.nextInt(randMax);
        int change = a + b;
        double sign = random_.nextInt(2) == 0 ? 1.0 : -1.0;
        int value = c + static_cast<int>(c * change * 0.01 * sign);
        t.setComp(i, std::min(value, 255));
    }
    baseColors_[key(k)] = t;
    modes_[key(k)] = randomLightDarkMode(random_);
}

// ---------- TypeBaseColorList ----------
TypeBaseColorList::TypeBaseColorList(Species* sp, const TypeBaseColorList* prevo, bool typeSanity, JavaRandom& r)
    : random_(r), species_(sp), prevo_(prevo),
      baseColorMap_(prevo ? prevo->baseColorMap_ : std::make_shared<BaseColorMap>(r)) {
    if (!prevo) basic(typeSanity);
    else fromPrevo(typeSanity);
}

bool TypeBaseColorList::contains(const TypeColor& c) const { return std::find(colors_.begin(), colors_.end(), c) != colors_.end(); }

void TypeBaseColorList::basic(bool typeSanity) {
    if (typeSanity) {
        moveFirstColorOfType(0, species_->primaryType());
        moveFirstColorOfType(1, species_->secondaryType() ? species_->secondaryType() : species_->primaryType());
        moveFirstColorOfType(2, species_->primaryType());
        if (species_->secondaryType()) moveFirstColorOfType(3, species_->secondaryType());
    }
    for (int i = 0; i < 16 - static_cast<int>(colors_.size()); i++) colors_.push_back(randomUnused());
}

void TypeBaseColorList::fromPrevo(bool typeSanity) {
    colors_ = prevo_->colors_;
    auto swap01 = [&] { std::swap(colors_.at(0), colors_.at(1)); };
    const Species* pv = prevo_->species_;
    bool samePrimary = species_->primaryType() == pv->primaryType();
    bool sameSecondary = species_->secondaryType() == pv->secondaryType();
    if (!typeSanity) {
        if (random_.nextDouble() < 0.3) swap01();
    } else if (samePrimary && sameSecondary) {
        if (random_.nextDouble() < 0.3) swap01();
    } else {
        if (!samePrimary) moveFirstColorOfType(0, species_->primaryType());
        if (!sameSecondary)
            moveFirstColorOfType(1, species_->secondaryType() ? species_->secondaryType() : species_->primaryType());
    }
}

void TypeBaseColorList::moveFirstColorOfType(int insertIndex, std::optional<Type> type) {
    TypeColor c;
    int idx = findColorOfType(insertIndex, type);
    if (idx == -1) {
        c = randomUnused(*type);
    } else {
        c = colors_[idx];
        colors_.erase(colors_.begin() + idx);
    }
    colors_.insert(colors_.begin() + insertIndex, c);
}

int TypeBaseColorList::findColorOfType(int start, std::optional<Type> type) const {
    for (int i = start; i < static_cast<int>(colors_.size()); i++)
        if (colors_[i].type == type) return i;
    return -1;
}

TypeColor TypeBaseColorList::randomUnused() {
    for (int i = 0; i < 100; i++) {
        TypeColor c = Gen3to5TypeColors::getRandomTypeColor(random_);
        if (!contains(c)) return c;
    }
    throw std::runtime_error("Could not get an unused TypeColor in less than 100 tries.");
}

TypeColor TypeBaseColorList::randomUnused(Type type) {
    for (int i = 0; i < 100; i++) {
        TypeColor c = Gen3to5TypeColors::getRandomTypeColor(type, random_);
        if (!contains(c)) return c;
    }
    throw std::runtime_error("Could not get an unused TypeColor of a type in less than 100 tries.");
}

// ---------- PaletteDescription / PalettePartDescription ----------
PaletteDescription::PaletteDescription(const std::string& s) {
    std::string sb;
    bool hasName = false, hasBody = false;
    for (char c : s) {
        if (c == '[') {
            name = sb;
            hasName = true;
            sb.clear();
        } else if (c == ']') {
            body = sb;
            hasBody = true;
            sb.clear();
        } else if (c == ' ' || !(c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v' || (c >= 0x1C && c <= 0x1F))) {
            sb += c;
        }
    }
    note = sb;
    (void)hasName;
    (void)hasBody;
}

namespace {
enum class CT { DIGIT, LETTER, SIB, IGNORE };
CT charType(char c) {
    if (c >= '0' && c <= '9') return CT::DIGIT;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return CT::LETTER;
    if (c == ';') return CT::SIB;
    return CT::IGNORE;
}
}  // namespace

PalettePartDescription::PalettePartDescription(const std::string& u) {
    blank_ = std::all_of(u.begin(), u.end(), [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; });
    std::vector<std::string> tokens;
    std::string sb;
    char last = ' ';
    for (char cur : u) {
        CT t = charType(cur);
        if (t != CT::IGNORE) {
            if (t != charType(last)) {
                tokens.push_back(sb);
                sb.clear();
            }
            sb += cur;
        }
        last = cur;
    }
    tokens.push_back(sb);
    std::vector<int>* cur = &slots_;
    for (const auto& tok : tokens) {
        if (!tok.empty() && std::all_of(tok.begin(), tok.end(), [](char c) { return c >= '0' && c <= '9'; })) cur->push_back(std::stoi(tok) - 1);
        else if (tok == "L") suffix_ = Suffix::LIGHT;
        else if (tok == "D") suffix_ = Suffix::DARK;
        else if (tok == "LN") suffix_ = Suffix::NO_LIGHT;
        else if (tok == "DN") suffix_ = Suffix::NO_DARK;
        else if (tok == "B") suffix_ = Suffix::BASE;
        else if (tok == "E") endDarkened_ = true;
        else if (tok == "A") cur = &averageSlots_;
        else if (tok == ";") cur = &siblingSlots_;
    }
    if (cur == &siblingSlots_ && !siblingSlots_.empty()) {
        sharedSlot_ = siblingSlots_.back();
        siblingSlots_.pop_back();
    }
}

std::vector<PalettePartDescription> PalettePartDescription::allFrom(const PaletteDescription& d) {
    std::vector<PalettePartDescription> r;
    for (const auto& p : jc::split(d.body, "/")) r.emplace_back(p);
    return r;
}

const std::vector<int>& PalettePartDescription::siblingSlots() const {
    if (!hasSibling()) throw std::logic_error("Can't get sibling slots when there is none.");
    return siblingSlots_;
}

int PalettePartDescription::sharedSlot() const {
    if (sharedSlot_ == -1) throw std::logic_error("Shared color slot has not been set.");
    return sharedSlot_;
}

LightDarkMode PalettePartDescription::correctLightDarkMode(LightDarkMode in) const {
    switch (suffix_) {
        case Suffix::BASE: return LightDarkMode::DEFAULT;
        case Suffix::DARK: return LightDarkMode::DARK;
        case Suffix::LIGHT: return LightDarkMode::LIGHT;
        case Suffix::NO_DARK: return in == LightDarkMode::DARK ? LightDarkMode::LIGHT : in;
        case Suffix::NO_LIGHT: return in == LightDarkMode::LIGHT ? LightDarkMode::DARK : in;
        default: return in;
    }
}

// ---------- PalettePopulator ----------
int PalettePopulator::sum3(int bound) {
    int a = random_.nextInt(bound);
    int b = random_.nextInt(bound);
    int c = random_.nextInt(bound);
    return a + b + c;
}

double PalettePopulator::leftChange(int palLimit, int maxRGB) {
    if (palLimit == 0) return 0;
    double coeff = (sum3(5) + 153) / palLimit;  // división entera, como en Java
    return coeff - coeff / 255 * maxRGB + palLimit * palLimit;
}

double PalettePopulator::rightChange(int palLimit, int maxRGB) {
    if (palLimit == 0) return 0;
    double coeff = (sum3(5) + 153) / palLimit;
    return coeff - coeff / 255 * (255 - maxRGB) + palLimit * palLimit;
}

void PalettePopulator::populatePartFromBaseColor(Palette& palette, const PalettePartDescription& d, const Color& baseColor,
                                                 LightDarkMode mode) {
    palette_ = &palette;
    desc_ = &d;
    baseColor_ = baseColor;
    mode_ = d.correctLightDarkMode(mode);
    if (d.length() != 0) fill(makeShades());
    if (d.hasSibling() && d.siblingLength() != 0) {
        Color shared = siblingColor(baseColor);
        fill(makeSiblingShades(shared, d.siblingSlots(), d.sharedSlot()));
    }
}

Color PalettePopulator::siblingColor(const Color& baseColor) {
    Color s = palette_->get(desc_->sharedSlot());
    if (s.r <= 3 && s.g <= 3 && s.b <= 3) s.setComps([&](const Color&, int i) { return static_cast<int>(baseColor.getComp(i) * 0.15); });
    else if (s.r >= 252 && s.g >= 252 && s.b >= 252)
        s.setComps([&](const Color&, int i) { return static_cast<int>(baseColor.getComp(i) * 0.85); });
    return s;
}

void PalettePopulator::fill(const Shades& s) {
    for (size_t i = 0; i < s.size(); i++)
        if (s[i]) palette_->set(static_cast<int>(i), *s[i]);
}

PalettePopulator::Shades PalettePopulator::fitInSlots(const std::vector<Color>& sorted, const std::vector<int>& slots) {
    Shades out(palette_->size());
    for (size_t i = 0; i < slots.size(); i++)
        if (slots[i] != -1) out.at(slots[i]) = sorted.at(i);
    return out;
}

PalettePopulator::Shades PalettePopulator::makeShades() {
    std::vector<Color> sorted(desc_->length());
    initShifts();
    makeBaseColorLightOrDark();
    makeMiddle(sorted);
    int n = static_cast<int>(sorted.size());
    makeLeft(sorted, n / 2 - (evenSlots() ? 2 : 1));
    makeRight(sorted, n / 2 + 1);
    if (desc_->isEndDarkened()) sorted[n - 1].setComps([&](const Color& o, int i) { return static_cast<int>(o.getComp(i) - right_[i]); });
    return fitInSlots(sorted, desc_->slots());
}

PalettePopulator::Shades PalettePopulator::makeSiblingShades(Color shared, const std::vector<int>& slots, int sharedSlot) {
    int sharedLoc = -1;
    for (size_t i = 0; i < slots.size(); i++)
        if (slots[i] == sharedSlot) {
            sharedLoc = static_cast<int>(i);
            break;
        }
    std::vector<Color> sorted(slots.size());
    std::array<double, 3> var{};
    for (int i = 0; i < 3; i++) {
        int s = sum3(10);
        double sign = random_.nextInt(2) == 0 ? 1.0 : -1.0;
        var[i] = (s + 3) * 0.01 * sign;
    }
    int n = static_cast<int>(slots.size());
    for (int i = 0; i < 3; i++) left_[i] = leftChange(sharedLoc, shared.getComp(i));
    for (int i = 0; i < 3; i++) right_[i] = rightChange(n - sharedLoc - 1, shared.getComp(i));
    sorted.at(sharedLoc) = shared;
    if (sharedLoc - 1 >= 0)
        sorted[sharedLoc - 1].setComps([&](const Color&, int i) { return static_cast<int>(sorted[sharedLoc].getComp(i) + left_[i] * (1 + var[i])); });
    if (sharedLoc + 1 < n)
        sorted[sharedLoc + 1].setComps([&](const Color&, int i) { return static_cast<int>(sorted[sharedLoc].getComp(i) - right_[i] * (1 + var[i])); });
    makeLeft(sorted, sharedLoc - 2);
    makeRight(sorted, sharedLoc + 2);
    return fitInSlots(sorted, desc_->siblingSlots());
}

void PalettePopulator::initShifts() {
    for (int i = 0; i < 3; i++) left_[i] = leftChange(middleSlotID(), baseColor_.getComp(i));
    for (int i = 0; i < 3; i++) right_[i] = rightChange(middleSlotID(), baseColor_.getComp(i));
}

void PalettePopulator::makeBaseColorLightOrDark() {
    const double mod = 0.5;
    int half = desc_->length() / 2 + 1;
    if (mode_ == LightDarkMode::DARK) {
        baseColor_.setComps([&](const Color& o, int i) { return static_cast<int>(o.getComp(i) - right_[i] * half * mod); });
        for (int i = 0; i < 3; i++) left_[i] = rightChange(middleSlotID(), baseColor_.getComp(i));
        for (int i = 0; i < 3; i++) right_[i] = rightChange(middleSlotID(), baseColor_.getComp(i));
    } else if (mode_ == LightDarkMode::LIGHT) {
        baseColor_.setComps([&](const Color& o, int i) { return static_cast<int>(o.getComp(i) + left_[i] * half * mod); });
        for (int i = 0; i < 3; i++) left_[i] = leftChange(middleSlotID(), baseColor_.getComp(i));
        for (int i = 0; i < 3; i++) right_[i] = leftChange(middleSlotID(), baseColor_.getComp(i));
        double m = lightDarkenCoeff();
        for (int i = 0; i < 3; i++) right_[i] += m * leftChange(middleSlotID(), baseColor_.getComp(i));
    }
}

double PalettePopulator::lightDarkenCoeff() {
    std::array<int, 3> bc{baseColor_.r, baseColor_.g, baseColor_.b};
    std::sort(bc.begin(), bc.end());
    double avg = (bc[1] + bc[2]) / 2.0;
    if (baseColor_.g >= 225 && baseColor_.r < 200 && baseColor_.b < 200) avg += 75;
    if (baseColor_.g >= 225) avg += 75;
    if (avg > 255) avg = 255;
    if (avg < 192) return 0.5;
    return (0.0002 * std::pow(avg - 192, 3.5) + 50) * 0.01;
}

void PalettePopulator::makeMiddle(std::vector<Color>& sorted) {
    int n = static_cast<int>(sorted.size());
    if (evenSlots()) {
        for (int total = 0, r = 0; total < 9 && r < 5; r++) {
            total = 0;
            for (int p = 0; p < 3; p++) {
                int lv = std::min(static_cast<int>(baseColor_.getComp(p) + left_[p] * 0.5), 255);
                sorted[n / 2 - 1].setComp(p, lv);
                int rv = std::max(static_cast<int>(baseColor_.getComp(p) - right_[p] * 0.5), 0);
                sorted[n / 2].setComp(p, rv);
                total += (sorted[n / 2].getComp(p) >> 2) - (sorted[n / 2 - 1].getComp(p) >> 2);
            }
        }
    } else {
        sorted[n / 2] = baseColor_;
    }
}

void PalettePopulator::makeLeft(std::vector<Color>& sorted, int rightBound) {
    for (int p = rightBound; p >= 0; p--) {
        int right = p + 1;
        sorted.at(p).setComps([&](const Color&, int i) { return static_cast<int>(sorted.at(right).getComp(i) + left_[i]); });
    }
}

void PalettePopulator::makeRight(std::vector<Color>& sorted, int leftBound) {
    for (int p = leftBound; p < static_cast<int>(sorted.size()); p++) {
        int left = p - 1;
        sorted.at(p).setComps([&](const Color&, int i) { return static_cast<int>(sorted.at(left).getComp(i) - right_[i]); });
    }
}

void PalettePopulator::populateAverageColor(Palette& palette, const PalettePartDescription& d) {
    Color& to = palette.get(d.averageToSlot());
    auto from = d.averageFromSlots();
    to.setComps([&](const Color&, int i) {
        int sum = 0;
        for (int s : from) sum += palette.get(s).getComp(i);
        return sum / static_cast<int>(from.size());
    });
}

}  // namespace pr
