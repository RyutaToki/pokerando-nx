// Port del paquete de paletas de FVX usado al randomizar Gen 3-5: TypeColor, BaseColorMap, LightDarkMode,
// RandomColorSelector/RandomPointSelector, Gen3to5TypeColors, TypeBaseColorList, PaletteDescription,
// PalettePartDescription y PalettePopulator.
#pragma once
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "gamedata/types.hpp"
#include "graphics/palette.hpp"
#include "util/java_random.hpp"

namespace pr {

class Species;

enum class LightDarkMode { DEFAULT, LIGHT, DARK };
LightDarkMode randomLightDarkMode(JavaRandom& r);

struct TypeColor {
    Color color;
    std::optional<Type> type;
    bool operator==(const TypeColor& o) const { return color == o.color; }  // Color.equals: solo RGB
};

namespace Gen3to5TypeColors {
TypeColor getRandomTypeColor(Type type, JavaRandom& r);
TypeColor getRandomTypeColor(JavaRandom& r);
}  // namespace Gen3to5TypeColors

class BaseColorMap {
public:
    explicit BaseColorMap(JavaRandom& r) : random_(r) {}
    Color getBaseColor(const TypeColor& key);
    LightDarkMode getLightDarkMode(const TypeColor& key);

private:
    using Key = std::array<int, 3>;
    static Key key(const TypeColor& c) { return {c.color.r, c.color.g, c.color.b}; }
    void add(const TypeColor& k);
    JavaRandom& random_;
    std::map<Key, Color> baseColors_;
    std::map<Key, LightDarkMode> modes_;
};

class TypeBaseColorList {
public:
    TypeBaseColorList(Species* sp, const TypeBaseColorList* prevo, bool typeSanity, JavaRandom& r);
    Color getBaseColor(int i) { return baseColorMap_->getBaseColor(colors_.at(i)); }
    LightDarkMode getLightDarkMode(int i) { return baseColorMap_->getLightDarkMode(colors_.at(i)); }

private:
    JavaRandom& random_;
    Species* species_;
    std::vector<TypeColor> colors_;  // LinkedList en Java
    const TypeBaseColorList* prevo_;
    std::shared_ptr<BaseColorMap> baseColorMap_;  // compartido con la preevolución
    void basic(bool typeSanity);
    void fromPrevo(bool typeSanity);
    void moveFirstColorOfType(int insertIndex, std::optional<Type> type);
    int findColorOfType(int start, std::optional<Type> type) const;
    TypeColor randomUnused();
    TypeColor randomUnused(Type type);
    bool contains(const TypeColor& c) const;
};

struct PaletteDescription {
    std::string name, body, note;
    explicit PaletteDescription(const std::string& line);
};

class PalettePartDescription {
public:
    explicit PalettePartDescription(const std::string& unparsed);
    static std::vector<PalettePartDescription> allFrom(const PaletteDescription& d);
    bool isBlank() const { return blank_; }
    bool isEndDarkened() const { return endDarkened_; }
    bool hasSibling() const { return !siblingSlots_.empty(); }
    bool isAverageDescription() const { return !averageSlots_.empty(); }
    int length() const { return static_cast<int>(slots_.size()); }
    int siblingLength() const { return static_cast<int>(siblingSlots_.size()); }
    const std::vector<int>& slots() const { return slots_; }
    const std::vector<int>& siblingSlots() const;
    int sharedSlot() const;
    int averageToSlot() const { return averageSlots_.at(0); }
    std::vector<int> averageFromSlots() const { return {averageSlots_.begin() + 1, averageSlots_.end()}; }
    LightDarkMode correctLightDarkMode(LightDarkMode in) const;

private:
    enum class Suffix { ANY, LIGHT, DARK, BASE, NO_LIGHT, NO_DARK };
    bool blank_ = false, endDarkened_ = false;
    std::vector<int> slots_, siblingSlots_, averageSlots_;
    int sharedSlot_ = -1;
    Suffix suffix_ = Suffix::ANY;
};

class PalettePopulator {
public:
    explicit PalettePopulator(JavaRandom& r) : random_(r) {}
    void populatePartFromBaseColor(Palette& palette, const PalettePartDescription& d, const Color& baseColor, LightDarkMode mode);
    void populateAverageColor(Palette& palette, const PalettePartDescription& d);

private:
    JavaRandom& random_;
    Palette* palette_ = nullptr;
    const PalettePartDescription* desc_ = nullptr;
    Color baseColor_;
    LightDarkMode mode_ = LightDarkMode::DEFAULT;
    std::array<double, 3> left_{}, right_{};

    using Shades = std::vector<std::optional<Color>>;
    Color siblingColor(const Color& baseColor);
    void fill(const Shades& s);
    Shades makeShades();
    Shades makeSiblingShades(Color shared, const std::vector<int>& slots, int sharedSlot);
    void makeBaseColorLightOrDark();
    double lightDarkenCoeff();
    void initShifts();
    int middleSlotID() const { return (desc_->length() - (evenSlots() ? 0 : 1)) / 2; }
    bool evenSlots() const { return desc_->length() % 2 == 0; }
    Shades fitInSlots(const std::vector<Color>& sorted, const std::vector<int>& slots);
    void makeMiddle(std::vector<Color>& sorted);
    void makeLeft(std::vector<Color>& sorted, int rightBound);
    void makeRight(std::vector<Color>& sorted, int leftBound);
    double leftChange(int palLimit, int maxRGB);
    double rightChange(int palLimit, int maxRGB);
    int sum3(int bound);
};

}  // namespace pr
