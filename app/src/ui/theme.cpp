#include "ui/theme.hpp"

#include <borealis.hpp>
#include <cmath>

namespace ui {

namespace {
NVGcolor hex(unsigned v) { return nvgRGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF); }
const unsigned kAccents[4][2] = {{0x3DDBC4, 0x7FEBDA}, {0xFF5A5F, 0xFF9496}, {0x5B8CFF, 0x96B5FF}, {0xF2B35B, 0xF7CE91}};
int gAccent = 0;
Colors gColors = {hex(0x1F2023), hex(0x2A2B2F), hex(0x18191B), hex(0x3A3B40), hex(0x4A4B52), hex(0xEDEDED),
                  hex(0xA3A6AD), hex(0x3DDBC4), hex(0x7FEBDA), hex(0xF2B35B), hex(0x1F2023), hex(0xFF5A5F)};
}  // namespace

Colors& colors() { return gColors; }
NVGcolor accentChoice(int i) { return hex(kAccents[i & 3][0]); }
int accentIndex() { return gAccent; }
void setAccent(int i) {
    gAccent = i & 3;
    gColors.accent = hex(kAccents[gAccent][0]);
    gColors.accentHover = hex(kAccents[gAccent][1]);
}

void loadFonts() {
    const std::string base = BRLS_ASSET("fonts/");
    struct F {
        const char* name;
        const char* file;
    } fonts[] = {{font::regular, "Inter-Regular.ttf"},  {font::medium, "Inter-Medium.ttf"},
                 {font::semibold, "Inter-SemiBold.ttf"}, {font::bold, "Inter-Bold.ttf"},
                 {font::title, "SpaceGrotesk-Bold.ttf"}, {font::mono, "JetBrainsMono-Regular.ttf"}};
    NVGcontext* vg = brls::Application::getNVGContext();
    int fallback = brls::Application::getDefaultFont();
    int icons = brls::Application::getFont(brls::FONT_SWITCH_ICONS);
    for (const auto& f : fonts) {
        if (!brls::Application::loadFontFromFile(f.name, base + f.file)) {
            brls::Logger::error("No se pudo cargar la fuente {}", f.file);
            continue;
        }
        int id = brls::Application::getFont(f.name);
        // glifos que falten (p. ej. ½ en la mono) salen de la fuente del sistema
        if (fallback >= 0) nvgAddFallbackFontId(vg, id, fallback);
        if (icons >= 0) nvgAddFallbackFontId(vg, id, icons);
    }
}

int fontId(const char* name) {
    int id = brls::Application::getFont(name);
    return id >= 0 ? id : brls::Application::getDefaultFont();
}

void fillRRect(NVGcontext* vg, float x, float y, float w, float h, float r, NVGcolor c) {
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillColor(vg, c);
    nvgFill(vg);
}

void strokeRRect(NVGcontext* vg, float x, float y, float w, float h, float r, float width, NVGcolor c) {
    float hw = width / 2;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + hw, y + hw, w - width, h - width, std::max(0.f, r - hw));
    nvgStrokeColor(vg, c);
    nvgStrokeWidth(vg, width);
    nvgStroke(vg);
}

static void setFont(NVGcontext* vg, const char* fontName, float size, float letterSpacing) {
    nvgFontFaceId(vg, fontId(fontName));
    nvgFontSize(vg, size);
    nvgTextLetterSpacing(vg, letterSpacing);
}

float text(NVGcontext* vg, float x, float y, const std::string& s, const char* fontName, float size, NVGcolor c, Align a,
           float letterSpacing) {
    setFont(vg, fontName, size, letterSpacing);
    int h = a == Align::LEFT ? NVG_ALIGN_LEFT : a == Align::CENTER ? NVG_ALIGN_CENTER : NVG_ALIGN_RIGHT;
    nvgTextAlign(vg, h | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, c);
    float end = nvgText(vg, x, y, s.c_str(), nullptr);
    nvgTextLetterSpacing(vg, 0);
    return end - x;
}

float textWidth(NVGcontext* vg, const std::string& s, const char* fontName, float size, float letterSpacing) {
    setFont(vg, fontName, size, letterSpacing);
    float b[4];
    float w = nvgTextBounds(vg, 0, 0, s.c_str(), nullptr, b);
    nvgTextLetterSpacing(vg, 0);
    return w;
}

float textBox(NVGcontext* vg, float x, float y, float w, const std::string& s, const char* fontName, float size, NVGcolor c,
              float lineHeight) {
    setFont(vg, fontName, size, 0);
    nvgTextLineHeight(vg, lineHeight);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFillColor(vg, c);
    float b[4];
    nvgTextBoxBounds(vg, x, y, w, s.c_str(), nullptr, b);
    nvgTextBox(vg, x, y, w, s.c_str(), nullptr);
    nvgTextLineHeight(vg, 1);
    return b[3] - b[1];
}

std::string ellipsize(NVGcontext* vg, const std::string& s, const char* fontName, float size, float maxWidth) {
    if (textWidth(vg, s, fontName, size) <= maxWidth) return s;
    std::string r = s;
    while (!r.empty()) {
        // quitar un carácter UTF-8 entero
        size_t i = r.size() - 1;
        while (i > 0 && (static_cast<unsigned char>(r[i]) & 0xC0) == 0x80) i--;
        r.erase(i);
        if (textWidth(vg, r + "…", fontName, size) <= maxWidth) return r + "…";
    }
    return "…";
}

void icon(NVGcontext* vg, Icon i, float x, float y, float s, NVGcolor c, float stroke) {
    auto P = [&](float fx, float fy) { return std::pair<float, float>(x + fx * s, y + fy * s); };
    nvgSave(vg);
    nvgStrokeColor(vg, c);
    nvgStrokeWidth(vg, stroke);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    nvgBeginPath(vg);
    auto M = [&](float fx, float fy) { auto p = P(fx, fy); nvgMoveTo(vg, p.first, p.second); };
    auto L = [&](float fx, float fy) { auto p = P(fx, fy); nvgLineTo(vg, p.first, p.second); };
    switch (i) {
        case Icon::FOLDER:
            M(0.08f, 0.25f); L(0.38f, 0.25f); L(0.48f, 0.36f); L(0.92f, 0.36f); L(0.92f, 0.82f); L(0.08f, 0.82f);
            nvgClosePath(vg);
            break;
        case Icon::LIST:
            for (float fy : {0.28f, 0.5f, 0.72f}) {
                M(0.32f, fy); L(0.9f, fy);
                nvgCircle(vg, x + 0.14f * s, y + fy * s, stroke * 0.6f);
            }
            break;
        case Icon::CLOCK:
            nvgCircle(vg, x + s / 2, y + s / 2, s * 0.4f);
            M(0.5f, 0.28f); L(0.5f, 0.5f); L(0.66f, 0.62f);
            break;
        case Icon::GEAR: {
            nvgCircle(vg, x + s / 2, y + s / 2, s * 0.14f);
            nvgStroke(vg);
            nvgBeginPath(vg);
            const int teeth = 8;
            for (int k = 0; k < teeth * 2; k++) {
                float a = k * NVG_PI / teeth;
                float r = (k % 2 == 0) ? s * 0.42f : s * 0.32f;
                float a2 = a + NVG_PI / teeth * 0.5f;
                float px = x + s / 2 + std::cos(a) * r, py = y + s / 2 + std::sin(a) * r;
                if (k == 0) nvgMoveTo(vg, px, py);
                else nvgLineTo(vg, px, py);
                nvgLineTo(vg, x + s / 2 + std::cos(a2) * r, y + s / 2 + std::sin(a2) * r);
            }
            nvgClosePath(vg);
            break;
        }
        case Icon::FILE:
            M(0.22f, 0.1f); L(0.6f, 0.1f); L(0.8f, 0.3f); L(0.8f, 0.9f); L(0.22f, 0.9f); nvgClosePath(vg);
            M(0.6f, 0.1f); L(0.6f, 0.3f); L(0.8f, 0.3f);
            break;
        case Icon::DOC:
            M(0.22f, 0.1f); L(0.78f, 0.1f); L(0.78f, 0.9f); L(0.22f, 0.9f); nvgClosePath(vg);
            M(0.34f, 0.35f); L(0.66f, 0.35f); M(0.34f, 0.52f); L(0.66f, 0.52f); M(0.34f, 0.69f); L(0.56f, 0.69f);
            break;
        case Icon::ARROW_LEFT:
            M(0.85f, 0.5f); L(0.15f, 0.5f); M(0.4f, 0.25f); L(0.15f, 0.5f); L(0.4f, 0.75f);
            break;
        case Icon::CHECK:
            M(0.2f, 0.52f); L(0.42f, 0.74f); L(0.82f, 0.3f);
            break;
        case Icon::WARNING:
            M(0.5f, 0.12f); L(0.92f, 0.86f); L(0.08f, 0.86f); nvgClosePath(vg);
            M(0.5f, 0.4f); L(0.5f, 0.6f);
            nvgCircle(vg, x + 0.5f * s, y + 0.73f * s, stroke * 0.4f);
            break;
        case Icon::STAR:
            for (int k = 0; k < 10; k++) {
                float a = -NVG_PI / 2 + k * NVG_PI / 5;
                float r = k % 2 == 0 ? s * 0.45f : s * 0.2f;
                float px = x + s / 2 + std::cos(a) * r, py = y + s * 0.53f + std::sin(a) * r;
                if (k == 0) nvgMoveTo(vg, px, py);
                else nvgLineTo(vg, px, py);
            }
            nvgClosePath(vg);
            break;
        case Icon::PLAY:
            M(0.3f, 0.18f); L(0.82f, 0.5f); L(0.3f, 0.82f); nvgClosePath(vg);
            break;
    }
    nvgStroke(vg);
    nvgRestore(vg);
}

}  // namespace ui
