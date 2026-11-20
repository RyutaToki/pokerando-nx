// Port de Color y Palette (com.uprfvx.romio.graphics.palettes).
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

namespace pr {

struct Color {
    int r = 255, g = 255, b = 255;  // DEFAULT_COLOR = WHITE
    Color() = default;
    Color(int r_, int g_, int b_) : r(r_), g(g_), b(b_) {
        if (r < 0 || r > 255) throw std::invalid_argument("red value out of bounds (0-255)");
        if (g < 0 || g > 255) throw std::invalid_argument("green value out of bounds (0-255)");
        if (b < 0 || b > 255) throw std::invalid_argument("blue value out of bounds (0-255)");
    }
    static Color fromHex(int hex) { return Color((hex & 0xFF0000) >> 16, (hex & 0xFF00) >> 8, hex & 0xFF); }
    static Color fromHSV(double h, double s, double v);
    static int convHighColorWordToARGB(int word) {
        int red = static_cast<int>((word & 0x1F) * 8.25);
        int green = static_cast<int>(((word & 0x3E0) >> 5) * 8.25);
        int blue = static_cast<int>(((word & 0x7C00) >> 10) * 8.25);
        return static_cast<int>(0xFF000000u | (red << 16) | (green << 8) | blue);
    }
    std::array<uint8_t, 2> toBytes() const {
        int sum = (b / 8) * 1024 + (r / 8) + (g / 8) * 32;
        return {static_cast<uint8_t>(sum), static_cast<uint8_t>(sum / 256)};
    }
    int toARGB() const { return static_cast<int>(0xFF000000u | (r << 16) | (g << 8) | b); }
    std::array<double, 3> toHSV() const;
    int getComp(int i) const;
    void setComp(int i, int value);
    void setComps(const std::function<int(const Color&, int)>& cc);
    bool operator==(const Color&) const = default;
};

class Palette {
public:
    Palette() : colors_(16) {}
    explicit Palette(int size) : colors_(size) {}
    Palette(int size, const Color& c) : colors_(size, c) {}
    explicit Palette(std::vector<Color> colors) : colors_(std::move(colors)) {}
    static Palette fromBytes(const std::vector<uint8_t>& bytes) {
        std::vector<Color> c(bytes.size() / 2);
        for (size_t i = 0; i < c.size(); i++) c[i] = Color::fromHex(Color::convHighColorWordToARGB(bytes[2 * i] | (bytes[2 * i + 1] << 8)));
        return Palette(std::move(c));
    }
    Palette(const Palette& original, int size) : colors_(size) {
        for (int i = 0; i < std::min(size, original.size()); i++) colors_[i] = original.get(i);
    }
    Palette(const Palette&) = default;
    Palette& operator=(const Palette&) = default;

    const Color& get(int i) const { return colors_.at(i); }
    Color& get(int i) { return colors_.at(i); }
    void set(int i, const Color& c) { colors_.at(i) = c; }
    int size() const { return static_cast<int>(colors_.size()); }
    std::vector<uint8_t> toBytes() const {
        std::vector<uint8_t> out;
        for (const auto& c : colors_) {
            auto b = c.toBytes();
            out.push_back(b[0]);
            out.push_back(b[1]);
        }
        return out;
    }

private:
    std::vector<Color> colors_;
};

}  // namespace pr
