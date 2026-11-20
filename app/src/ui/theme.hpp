// Tokens de color, fuentes y primitivas de dibujo (docs/especificacion-UI.md, §2-§5).
#pragma once
#include <nanovg.h>

#include <string>

namespace ui {

struct Colors {
    NVGcolor bg, surface, sidebar, border, trackOff, text, textMuted, accent, accentHover, warn, onAccent, error;
};
Colors& colors();
// acentos posibles en Ajustes: turquesa (por defecto), rojo, azul, ámbar
void setAccent(int index);
int accentIndex();
NVGcolor accentChoice(int index);

inline NVGcolor withAlpha(NVGcolor c, float a) {
    c.a *= a;
    return c;
}

// nombres de fuente registrados en Borealis
namespace font {
inline const char* regular = "inter-400";
inline const char* medium = "inter-500";
inline const char* semibold = "inter-600";
inline const char* bold = "inter-700";
inline const char* title = "grotesk-700";
inline const char* mono = "mono";
}  // namespace font
void loadFonts();
int fontId(const char* name);

// ---- primitivas ----
void fillRRect(NVGcontext* vg, float x, float y, float w, float h, float r, NVGcolor c);
// borde interior (no sobresale del rectángulo)
void strokeRRect(NVGcontext* vg, float x, float y, float w, float h, float r, float width, NVGcolor c);

enum class Align { LEFT, CENTER, RIGHT };
// dibuja texto centrado en vertical en la línea y; devuelve el ancho
float text(NVGcontext* vg, float x, float y, const std::string& s, const char* fontName, float size, NVGcolor c,
           Align a = Align::LEFT, float letterSpacing = 0);
float textWidth(NVGcontext* vg, const std::string& s, const char* fontName, float size, float letterSpacing = 0);
// texto en varias líneas dentro de un ancho (desde arriba); devuelve la altura usada
float textBox(NVGcontext* vg, float x, float y, float w, const std::string& s, const char* fontName, float size, NVGcolor c,
              float lineHeight = 1.3f);
// recorta con "…" si no cabe
std::string ellipsize(NVGcontext* vg, const std::string& s, const char* fontName, float size, float maxWidth);

// ---- iconos de línea (trazo 2 px, cuadro de size x size) ----
enum class Icon { FOLDER, LIST, CLOCK, GEAR, FILE, ARROW_LEFT, CHECK, WARNING, STAR, PLAY, DOC };
void icon(NVGcontext* vg, Icon i, float x, float y, float size, NVGcolor c, float stroke = 2);

}  // namespace ui
