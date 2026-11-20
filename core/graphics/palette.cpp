#include "graphics/palette.hpp"

#include <algorithm>

namespace pr {

Color Color::fromHSV(double h, double s, double v) {
    if (h < 0.0 || h > 360.0) throw std::invalid_argument("hue argument \"h\" out of bounds (0.0 - 360.0)");
    if (s < 0.0 || s > 1.0) throw std::invalid_argument("saturation argument \"s\" out of bounds (0.0 - 1.0)");
    if (v < 0.0 || v > 1.0) throw std::invalid_argument("value argument \"v\" out of bounds (0.0 - 1.0)");
    double c = v * s;
    double x = c * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1));
    double m = v - c;
    const double bases[6][3] = {{c, x, 0}, {x, c, 0}, {0, c, x}, {0, x, c}, {x, 0, c}, {c, 0, x}};
    const double* base = bases[static_cast<int>(h / 60)];
    return Color(static_cast<int>((base[0] + m) * 255), static_cast<int>((base[1] + m) * 255),
                 static_cast<int>((base[2] + m) * 255));
}

std::array<double, 3> Color::toHSV() const {
    double rf = r / 255.0, gf = g / 255.0, bf = b / 255.0;
    double cMax = std::max(std::max(rf, gf), bf);
    double cMin = std::min(std::min(rf, gf), bf);
    double delta = cMax - cMin;
    double h;
    if (delta == 0) h = 0;
    else if (cMax == rf) h = 60 * std::fmod((gf - bf) / delta, 6);
    else if (cMax == gf) h = 60 * (((bf - rf) / delta) + 2);
    else h = 60 * (((rf - gf) / delta) + 4);
    double s = cMax == 0 ? 0 : delta / cMax;
    return {h, s, cMax};
}

int Color::getComp(int i) const {
    switch (i) {
        case 0: return r;
        case 1: return g;
        case 2: return b;
        default: throw std::out_of_range("componente RGB fuera de rango");
    }
}

void Color::setComp(int i, int value) {
    switch (i) {
        case 0: r = value; break;
        case 1: g = value; break;
        case 2: b = value; break;
        default: throw std::out_of_range("componente RGB fuera de rango");
    }
}

void Color::setComps(const std::function<int(const Color&, int)>& cc) {
    for (int i = 0; i < 3; i++) {
        int v = std::clamp(cc(*this, i), 0, 255);
        setComp(i, v);
    }
}

}  // namespace pr
