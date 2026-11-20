// §6.5 Ajustes, visor del log de spoilers y selector de iniciales
#include <algorithm>
#include <cmath>
#include <fstream>

#include "app/session.hpp"
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;

// ---------------- Ajustes ----------------
class SettingsScreen : public Screen {
public:
    SettingsScreen() {
        auto& cfg = app::config();
        frame->header->title = "Ajustes";
        frame->header->rightText = "PokéRando NX v" PR_VERSION;
        frame->footer->hints = {{"A", "Cambiar"}, {"B", "Atrás"}};

        auto* scroll = new brls::ScrollingFrame();
        scroll->setGrow(1);
        auto* body = new brls::Box(brls::Axis::COLUMN);
        body->setPadding(28, 48, 40, 48);
        scroll->setContentView(body);
        frame->content->addView(scroll);

        auto gap = [&](float h) {
            auto* b = new brls::Box();
            b->setHeight(h);
            body->addView(b);
        };

        auto* l = sectionLabel("COLOR DE ACENTO");
        l->setMarginBottom(10);
        body->addView(l);
        std::vector<NVGcolor> sw;
        for (int i = 0; i < 4; i++) sw.push_back(accentChoice(i));
        auto* chips = new Chips({"", "", "", ""}, cfg.accent, [](int i) {
            app::config().accent = i;
            setAccent(i);
            app::saveConfig();
        }, sw);
        first_ = chips;
        body->addView(chips);
        gap(24);

        body->addView(new Segmented("CARPETA INICIAL DEL EXPLORADOR", {"La última que usé", "sdmc:/roms/"}, cfg.startAtLast ? 0 : 1, [](int i) {
            app::config().startAtLast = i == 0;
            app::saveConfig();
        }));
        gap(18);
        body->addView(new Segmented("EMULADOR PARA GBA", {"mGBA", "RetroArch (núcleo mGBA)"}, cfg.gbaEmulator == "retroarch" ? 1 : 0, [](int i) {
            app::config().gbaEmulator = i ? "retroarch" : "mgba";
            app::saveConfig();
        }));
        gap(18);
        body->addView(new Segmented("EMULADOR PARA DS", {"melonDS", "RetroArch (núcleo melonDS)"}, cfg.ndsEmulator == "retroarch" ? 1 : 0, [](int i) {
            app::config().ndsEmulator = i ? "retroarch" : "melonds";
            app::saveConfig();
        }));
        gap(18);
        body->addView(new Segmented("IDIOMA", {"Español"}, 0, [](int) {}));
        gap(18);

        auto* grid = new brls::Box(brls::Axis::ROW);
        auto* t1 = new Toggle("Ocultar archivos no compatibles", cfg.hideUnsupported, [](bool b) {
            app::config().hideUnsupported = b;
            app::saveConfig();
        });
        t1->setGrow(1);
        t1->setWidth(10);
        t1->setMarginRight(8);
        grid->addView(t1);
        auto* t2 = new Toggle("Conservar ROMs randomizadas", cfg.keepRandomized, [](bool b) {
            app::config().keepRandomized = b;
            app::saveConfig();
        });
        t2->setGrow(1);
        t2->setWidth(10);
        grid->addView(t2);
        body->addView(grid);
        gap(24);

        auto* about = new FileRow(FileRow::Kind::FILE, "Acerca de PokéRando NX");
        about->setOnClick([] {
            ui::dialog("Acerca de PokéRando NX",
                       "Versión " PR_VERSION ". Randomizer de Pokémon GBA y DS para Nintendo Switch.\n\n"
                       "La lógica es un port de Universal Pokemon Randomizer (Dabomstew), "
                       "Universal Pokemon Randomizer ZX (Ajarmar) y Universal Pokemon Randomizer FVX (foxoftheasterisk, voliol y colaboradores).\n\n"
                       "Licencia GPLv3. Interfaz hecha con Borealis. No incluye ROMs ni material de Nintendo, Game Freak o The Pokémon Company.",
                       "Aceptar", nullptr);
        });
        body->addView(about);

        action(brls::BUTTON_B, [] { brls::Application::popActivity(); });
    }
    void onContentAvailable() override { brls::Application::giveFocus(first_); }

private:
    brls::View* first_ = nullptr;
};

// ---------------- Log ----------------
class LogView : public brls::View {
public:
    explicit LogView(const std::string& path) {
        setFocusable(true);
        setHideHighlight(true);
        std::ifstream f(path, std::ios::binary);
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // tabuladores a espacios
            std::string out;
            for (char c : line) {
                if (c == '\t') out.append(4 - out.size() % 4, ' ');
                else out += c;
            }
            lines_.push_back(out);
        }
        if (!f.eof() && lines_.empty()) lines_.push_back("No se pudo leer el log.");
        addGestureRecognizer(new brls::PanGestureRecognizer(
            [this](brls::PanGestureStatus st, brls::Sound*) {
                if (st.state == brls::GestureState::START) dragAcc_ = 0;
                dragAcc_ -= st.delta.y;
                int n = static_cast<int>(dragAcc_ / kLine);
                if (n) {
                    scrollBy(n, 0);
                    dragAcc_ -= n * kLine;
                }
            },
            brls::PanAxis::ANY));
    }
    static constexpr float kLine = 22;
    int visible() { return std::max(1, static_cast<int>((getHeight() - 8) / kLine)); }
    void scrollBy(int dy, int dx) {
        int maxTop = std::max(0, static_cast<int>(lines_.size()) - visible());
        top_ = std::clamp(top_ + dy, 0, maxTop);
        left_ = std::clamp(left_ + dx, 0, 400);
    }
    size_t lineCount() const { return lines_.size(); }
    int top() const { return top_; }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        const auto& c = colors();
        fillRRect(vg, x, y, w, h, 12, c.surface);
        nvgSave(vg);
        nvgScissor(vg, x + 16, y + 4, w - 40, h - 8);
        int n = visible();
        for (int i = 0; i < n && top_ + i < static_cast<int>(lines_.size()); i++) {
            const std::string& s = lines_[top_ + i];
            // avanza left_ caracteres (UTF-8)
            size_t pos = 0;
            for (int k = 0; k < left_ && pos < s.size(); k++) {
                pos++;
                while (pos < s.size() && (static_cast<unsigned char>(s[pos]) & 0xC0) == 0x80) pos++;
            }
            if (pos < s.size()) text(vg, x + 16, y + 4 + kLine * i + kLine / 2, s.substr(pos), font::mono, 15, c.text);
        }
        nvgRestore(vg);
        // barra de scroll
        if (lines_.size() > static_cast<size_t>(n)) {
            float th = std::max(30.0f, h * n / lines_.size());
            float ty = y + (h - th) * top_ / std::max<size_t>(1, lines_.size() - n);
            fillRRect(vg, x + w - 14, ty + 4, 6, th - 8, 3, c.trackOff);
        }
    }

private:
    std::vector<std::string> lines_;
    int top_ = 0, left_ = 0;
    float dragAcc_ = 0;
};

class LogScreen : public Screen {
public:
    explicit LogScreen(const std::string& path) {
        frame->header->title = "Log de spoilers";
        frame->footer->leftHints = {{"ZL / ZR", "Página"}};
        frame->footer->hints = {{"L / R", "Principio / final"}, {"B", "Atrás"}};
        auto* box = new brls::Box(brls::Axis::COLUMN);
        box->setPadding(24, 48, 24, 48);
        box->setGrow(1);
        view_ = new LogView(path);
        view_->setGrow(1);
        box->addView(view_);
        frame->content->addView(box);
        std::string name = app::fileName(path);
        frame->header->drawRight = [this, name](NVGcontext* vg, float x, float y, float w, float h) {
            std::string pos = std::to_string(std::min<size_t>(view_->top() + 1, view_->lineCount())) + " / " + std::to_string(view_->lineCount());
            float pw = text(vg, x + w, y + h / 2, pos, font::mono, 16, colors().textMuted, Align::RIGHT);
            text(vg, x + w - pw - 24, y + h / 2, ellipsize(vg, name, font::mono, 16, w - pw - 300), font::mono, 16, colors().text, Align::RIGHT);
        };
        action(brls::BUTTON_UP, [this] { view_->scrollBy(-1, 0); }, true);
        action(brls::BUTTON_DOWN, [this] { view_->scrollBy(1, 0); }, true);
        action(brls::BUTTON_LEFT, [this] { view_->scrollBy(0, -8); }, true);
        action(brls::BUTTON_RIGHT, [this] { view_->scrollBy(0, 8); }, true);
        action(brls::BUTTON_LT, [this] { view_->scrollBy(-view_->visible(), 0); }, true);
        action(brls::BUTTON_RT, [this] { view_->scrollBy(view_->visible(), 0); }, true);
        action(brls::BUTTON_LB, [this] { view_->scrollBy(-1000000, 0); });
        action(brls::BUTTON_RB, [this] { view_->scrollBy(1000000, 0); });
        action(brls::BUTTON_B, [] { brls::Application::popActivity(); });
    }
    void onContentAvailable() override { brls::Application::giveFocus(view_); }

private:
    LogView* view_;
};

// ---------------- Selector de iniciales ----------------
// Fila de especie (más baja que las de archivo para que quepan más)
class SpeciesRow : public Focusable {
public:
    SpeciesRow(std::string label, std::string right) : label_(std::move(label)), right_(std::move(right)) {
        setHeight(48);
        setMarginBottom(4);
    }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override {
        const auto& c = colors();
        bool f = isFocused();
        if (f) {
            fillRRect(vg, x, y, w, h, 8, c.surface);
            strokeRRect(vg, x, y, w, h, 8, 3, c.accent);
        }
        text(vg, x + 18, y + h / 2, label_, f ? font::semibold : font::medium, 18, c.text);
        if (!right_.empty()) text(vg, x + w - 18, y + h / 2, right_, font::medium, 15, c.accent, Align::RIGHT);
    }
    void setRight(const std::string& r) { right_ = r; }

private:
    std::string label_, right_;
};

// hueco de inicial: «Inicial 1» y el nombre elegido
class SlotRow : public Focusable {
public:
    explicit SlotRow(int n) : n_(n) {
        setHeight(76);
        setMarginBottom(10);
    }
    void set(const std::string& name, bool active) {
        name_ = name;
        active_ = active;
    }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override {
        const auto& c = colors();
        fillRRect(vg, x, y, w, h, 10, c.surface);
        if (active_) fillRRect(vg, x, y, 5, h, 2, c.accent);
        if (isFocused()) strokeRRect(vg, x, y, w, h, 10, 3, c.accent);
        text(vg, x + 22, y + 24, "INICIAL " + std::to_string(n_), font::semibold, 14, c.textMuted, Align::LEFT, 14 * 0.06f);
        text(vg, x + 22, y + 52, ellipsize(vg, name_, font::semibold, 19, w - 40), font::semibold, 19, active_ ? c.accent : c.text);
    }

private:
    int n_;
    std::string name_;
    bool active_ = false;
};

std::string lower(const std::string& s) {
    std::string r = s;
    for (char& ch : r) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return r;
}

class StarterPickerScreen : public Screen {
public:
    explicit StarterPickerScreen(std::function<void()> onDone) : onDone_(std::move(onDone)) {
        auto& se = app::session();
        frame->header->title = "Elegir iniciales";
        frame->header->drawRight = [this](NVGcontext* vg, float x, float y, float w, float h) {
            text(vg, x + w, y + h / 2, filter_.empty() ? "Todas las especies" : "Buscar: «" + filter_ + "»", font::regular, 16,
                 colors().textMuted, Align::RIGHT);
        };
        frame->footer->hints = {{"A", "Elegir"}, {"Y", "Buscar"}, {"X", "Aleatoria"}, {"B", "Listo"}};

        // misma lista que usa el randomizer (StarterRandomizer::getCustomStarters)
        if (se.preview)
            for (pr::Species* s : se.preview->getSpeciesInclFormes())
                if (!s || !s->isEssentiallyCosmetic()) species_.push_back(s);

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setPadding(28, 48, 28, 48);
        row->setGrow(1);
        auto* left = new brls::Box(brls::Axis::COLUMN);
        left->setWidth(360);
        left->setMarginRight(32);
        for (int i = 0; i < 3; i++) {
            auto* s = new SlotRow(i + 1);
            s->setOnClick([this, i] {
                slot_ = i;
                refreshSlots();
                if (!rows_.empty()) brls::Application::giveFocus(rows_.front());
            });
            s->getFocusEvent()->subscribe([this, i](brls::View*) {
                slot_ = i;
                refreshSlots();
            });
            left->addView(s);
            slots_.push_back(s);
        }
        auto* note = new Text("Elige un hueco y después la especie. Con X el hueco queda aleatorio.", font::regular, 15, colors().textMuted, 64);
        note->setWrap(true);
        note->setMarginTop(8);
        left->addView(note);
        row->addView(left);

        anchor_ = new FocusAnchor();
        row->addView(anchor_);
        auto* scroll = new brls::ScrollingFrame();
        scroll->setGrow(1);
        list_ = new brls::Box(brls::Axis::COLUMN);
        list_->setPaddingRight(6);
        scroll->setContentView(list_);
        row->addView(scroll);
        frame->content->addView(row);

        action(brls::BUTTON_B, [this] { done(); });
        action(brls::BUTTON_Y, [this] { search(); });
        action(brls::BUTTON_X, [this] { pick(0); });
        refreshSlots();
        buildList();
    }
    void onContentAvailable() override {
        shown_ = true;
        brls::Application::giveFocus(slots_[0]);
    }

private:
    std::string nameOf(int idx) const {
        if (idx <= 0 || idx >= static_cast<int>(species_.size()) || !species_[idx]) return "Aleatoria";
        return species_[idx]->fullName();
    }
    void refreshSlots() {
        const auto& cs = app::session().opts.s.customStarters;
        for (int i = 0; i < 3; i++) slots_[i]->set(nameOf(cs[i]), i == slot_);
        for (auto& [idx, r] : rowIdx_) {
            std::string tag;
            for (int i = 0; i < 3; i++)
                if (cs[i] == idx) tag = "Inicial " + std::to_string(i + 1);
            r->setRight(tag);
        }
    }
    void buildList() {
        if (shown_) anchor_->park();
        list_->clearViews();
        rows_.clear();
        rowIdx_.clear();
        std::string f = lower(filter_);
        for (size_t i = 1; i < species_.size(); i++) {
            if (!species_[i]) continue;
            std::string name = species_[i]->fullName();
            std::string label = "#" + std::to_string(species_[i]->number()) + "  " + name;
            if (!f.empty() && lower(label).find(f) == std::string::npos) continue;
            auto* r = new SpeciesRow(label, "");
            int idx = static_cast<int>(i);
            r->setOnClick([this, idx] { pick(idx); });
            list_->addView(r);
            rows_.push_back(r);
            rowIdx_.push_back({idx, r});
        }
        if (rows_.empty()) list_->addView(new Text("Ninguna especie coincide.", font::regular, 17, colors().textMuted, 40));
        refreshSlots();
        if (shown_) anchor_->release(rows_.empty() ? static_cast<brls::View*>(slots_[slot_]) : rows_.front());
    }
    void pick(int idx) {
        auto& se = app::session();
        se.opts.s.customStarters[slot_] = idx;
        se.dirty = true;
        se.presetName = "Personalizado";
        int next = slot_ + 1;
        if (next < 3) {
            slot_ = next;
            refreshSlots();
        } else {
            refreshSlots();
            brls::Application::giveFocus(slots_[2]);
        }
    }
    void search() {
        brls::Application::getImeManager()->openForText(
            [this](std::string s) {
                filter_ = s;
                buildList();
            },
            "Buscar especie", "Nombre o número (vacío = todas)", 20, filter_);
    }
    void done() {
        brls::Application::popActivity();
        if (onDone_) onDone_();
    }

    std::function<void()> onDone_;
    std::vector<pr::Species*> species_;
    std::vector<SlotRow*> slots_;
    std::vector<SpeciesRow*> rows_;
    std::vector<std::pair<int, SpeciesRow*>> rowIdx_;
    FocusAnchor* anchor_;
    brls::Box* list_;
    std::string filter_;
    int slot_ = 0;
    bool shown_ = false;
};

}  // namespace

void pushSettings() { brls::Application::pushActivity(new SettingsScreen(), brls::TransitionAnimation::NONE); }
void pushLog(const std::string& path) { brls::Application::pushActivity(new LogScreen(path), brls::TransitionAnimation::NONE); }
void pushStarterPicker(std::function<void()> onDone) {
    brls::Application::pushActivity(new StarterPickerScreen(std::move(onDone)), brls::TransitionAnimation::NONE);
}

}  // namespace screens
