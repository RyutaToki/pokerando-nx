// Componentes de la interfaz (docs/especificacion-UI.md, §4-§5). Todos se dibujan a mano con NanoVG
// para seguir la maqueta; Borealis se encarga del foco, los mandos y el táctil.
#pragma once
#include <borealis.hpp>
#include <functional>
#include <string>
#include <vector>

#include "ui/theme.hpp"

namespace ui {

// Base de los elementos enfocables: oculta el resaltado de Borealis y añade tap + acción A.
class Focusable : public brls::Box {
public:
    Focusable();
    void setOnClick(std::function<void()> f) { onClick_ = std::move(f); }
    void setEnabled(bool e);
    bool enabled() const { return enabled_; }
    void click() {
        if (enabled_ && onClick_) onClick_();
    }
    // dibuja el contenido atenuado al 40 % si está desactivado
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) final;
    virtual void drawContent(NVGcontext* vg, float x, float y, float w, float h) = 0;

protected:
    std::function<void()> onClick_;
    bool enabled_ = true;
};

// Ancla invisible: aparca el foco mientras se reconstruye una lista (para que Borealis no
// se quede apuntando a una vista borrada). Solo es enfocable mientras se usa.
class FocusAnchor : public brls::View {
public:
    FocusAnchor() {
        setWidth(0);
        setHeight(0);
        setFocusable(false);
        setHideHighlight(true);
    }
    void park() {
        setFocusable(true);
        brls::Application::giveFocus(this);
    }
    void release(brls::View* next) {
        if (next) brls::Application::giveFocus(next);
        setFocusable(false);
    }
    void draw(NVGcontext*, float, float, float, float, brls::Style, brls::FrameContext*) override {}
};

// Texto simple de una línea (o varias si wrap), con fuente, tamaño y color propios.
class Text : public brls::View {
public:
    Text(std::string s, const char* font, float size, NVGcolor color, float height = 0);
    void setText(const std::string& s) { text_ = s; }
    void setColor(NVGcolor c) { color_ = c; }
    void setWrap(bool w) { wrap_ = w; }
    void setAlign(Align a) { align_ = a; }
    void setLetterSpacing(float s) { spacing_ = s; }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override;

private:
    std::string text_;
    const char* font_;
    float size_;
    NVGcolor color_;
    bool wrap_ = false;
    Align align_ = Align::LEFT;
    float spacing_ = 0;
};

// Etiqueta de sección en mayúsculas (§3)
Text* sectionLabel(const std::string& s, NVGcolor color = colors().textMuted);

// Panel con fondo y radio
class Panel : public brls::Box {
public:
    Panel(NVGcolor bg, float radius);
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) override;
    void setBg(NVGcolor bg) { bg_ = bg; }

private:
    NVGcolor bg_;
    float radius_;
};

// §5.1 Botón de menú grande
class MenuButton : public Focusable {
public:
    MenuButton(Icon icon, std::string label);
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;

private:
    Icon icon_;
    std::string label_;
};

// §5.2 Fila de archivo o carpeta
class FileRow : public Focusable {
public:
    enum class Kind { UP, FOLDER, FILE, UNSUPPORTED };
    FileRow(Kind kind, std::string name, std::string right = "");
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;
    void setStar(bool s) { star_ = s; }
    void setName(const std::string& n) { name_ = n; }

private:
    Kind kind_;
    std::string name_, right_;
    bool star_ = false;
};

// Fila de lista genérica (presets, historial): título, subtítulo e insignia opcional
class ListRow : public Focusable {
public:
    ListRow(std::string title, std::string subtitle, std::string badge = "");
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;
    void setTitle(const std::string& t) { title_ = t; }

private:
    std::string title_, subtitle_, badge_;
};

// §5.3 Pestaña lateral
class TabItem : public Focusable {
public:
    explicit TabItem(std::string label);
    void setActive(bool a) { active_ = a; }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;

private:
    std::string label_;
    bool active_ = false;
};

// §5.4 Selector de modo (segmentado). Cada botón es enfocable.
class Segmented : public brls::Box {
public:
    Segmented(const std::string& label, std::vector<std::string> options, int selected, std::function<void(int)> onChange);
    void setSelected(int i);
    int selected() const { return selected_; }

private:
    class Seg;
    std::vector<Seg*> segs_;
    int selected_;
    std::function<void(int)> onChange_;
};

// §5.5 Interruptor
class Toggle : public Focusable {
public:
    Toggle(std::string label, bool on, std::function<void(bool)> onChange);
    void setOn(bool on) { on_ = on; }
    bool on() const { return on_; }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;

private:
    std::string label_;
    bool on_;
    float knob_ = 0;  // 0..1 animado
    std::function<void(bool)> onChange_;
};

// §5.6 Slider de porcentaje (−50..+50 en pasos de 5)
class Slider : public Focusable {
public:
    Slider(std::string label, int value, std::function<void(int)> onChange, int min = -50, int max = 50, int step = 5);
    void setValue(int v);
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;

private:
    std::string label_;
    int value_, min_, max_, step_;
    std::function<void(int)> onChange_;
    float trackX_ = 0, trackW_ = 1;
};

// §5.7 Fichas de elección
class Chips : public brls::Box {
public:
    Chips(std::vector<std::string> options, int selected, std::function<void(int)> onChange, std::vector<NVGcolor> swatches = {});
    void setSelected(int i) { selected_ = i; }
    int selected() const { return selected_; }
    void setEnabled(bool e);

private:
    class Chip;
    std::vector<Chip*> chips_;
    int selected_;
    std::function<void(int)> onChange_;
};

// §5.8 Botón principal / secundario
class Button : public Focusable {
public:
    Button(std::string label, bool primary, std::function<void()> onClick);
    void setLabel(const std::string& l) { label_ = l; }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override;

private:
    std::string label_;
    bool primary_;
};

// §5.9 insignia GBA/NDS; devuelve el ancho
float badge(NVGcontext* vg, float x, float cy, const std::string& text, NVGcolor bg);
// §5.10 «Exclusivo de esta app»
float exclusiveTag(NVGcontext* vg, float x, float cy);
// §5.11 logotipo; devuelve el ancho
float logo(NVGcontext* vg, float x, float cy, bool withName = true);

// ---- marco común: cabecera 88, contenido 568, pie 64 (§4) ----
struct Hint {
    std::string key;   // "A", "B", "+", "L / R"...
    std::string text;
    bool accent = false;
};

class Header : public brls::View {
public:
    Header();
    std::function<void(NVGcontext*, float x, float y, float w, float h)> drawLeft, drawRight;
    std::string title;
    std::string rightText;
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override;
};

class Footer : public brls::View {
public:
    Footer();
    std::vector<Hint> hints;
    std::vector<Hint> leftHints;
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override;
};

class Frame : public brls::Box {
public:
    Frame();
    Header* header;
    brls::Box* content;
    Footer* footer;
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) override;
};

// Toast (§8): abajo en el centro, 2,5 s
void toast(const std::string& text);
void drawToast(NVGcontext* vg);

// Diálogo modal (§8). Botones: el primero confirma (A), el segundo cancela (B).
void dialog(const std::string& title, const std::string& body, const std::string& okText, std::function<void()> onOk,
            const std::string& cancelText = "", std::function<void()> onCancel = nullptr, bool error = false);

// Menú rápido modal con una lista de opciones
void menu(const std::string& title, std::vector<std::pair<std::string, std::function<void()>>> items);

}  // namespace ui
