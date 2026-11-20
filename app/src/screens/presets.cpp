// §6.5 Presets guardados e historial de ROMs generadas
#include <algorithm>
#include <cstdio>

#include "app/session.hpp"
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;

// lista con scroll a todo el ancho y un ancla para reconstruirla sin perder el foco
class ListScreen : public Screen {
public:
    ListScreen() {
        auto* row = new brls::Box(brls::Axis::COLUMN);
        row->setPadding(28, 48, 28, 48);
        row->setGrow(1);
        anchor_ = new FocusAnchor();
        row->addView(anchor_);
        scroll_ = new brls::ScrollingFrame();
        scroll_->setGrow(1);
        list_ = new brls::Box(brls::Axis::COLUMN);
        list_->setPaddingRight(6);
        scroll_->setContentView(list_);
        row->addView(scroll_);
        frame->content->addView(row);
        action(brls::BUTTON_B, [] { brls::Application::popActivity(); });
    }
    void onContentAvailable() override {
        shown_ = true;
        if (!rows_.empty()) brls::Application::giveFocus(rows_[std::min(focus_, rows_.size() - 1)]);
    }

protected:
    // índice de la fila con el foco
    size_t focused() const {
        brls::View* f = brls::Application::getCurrentFocus();
        for (size_t i = 0; i < rows_.size(); i++)
            if (rows_[i] == f) return i;
        return rows_.size();
    }
    virtual void fill() = 0;
    void rebuild(size_t focusIndex) {
        if (shown_) anchor_->park();
        list_->clearViews();
        rows_.clear();
        fill();
        focus_ = focusIndex;
        if (shown_) anchor_->release(rows_.empty() ? nullptr : rows_[std::min(focusIndex, rows_.size() - 1)]);
    }
    void addRow(Focusable* r) {
        r->setMarginBottom(6);
        list_->addView(r);
        rows_.push_back(r);
    }
    void addEmpty(const std::string& msg) {
        auto* t = new Text(msg, font::regular, 17, colors().textMuted, 40);
        t->setWrap(true);
        list_->addView(t);
    }

    FocusAnchor* anchor_;
    brls::ScrollingFrame* scroll_;
    brls::Box* list_;
    std::vector<Focusable*> rows_;
    size_t focus_ = 0;
    bool shown_ = false;
};

// ---------------- Presets ----------------
class PresetsScreen : public ListScreen {
public:
    explicit PresetsScreen(bool fromOptions) : fromOptions_(fromOptions) {
        frame->header->title = "Presets guardados";
        bool canLoad = !app::session().romPath.empty();
        frame->header->rightText = canLoad ? "Para " + app::session().info.gameName : "Abre una ROM para cargar un preset";
        frame->footer->hints = {{"A", "Cargar"}, {"X", "Renombrar"}, {"Y", "Duplicar"}, {"−", "Borrar"}, {"B", "Atrás"}};
        action(brls::BUTTON_X, [this] { rename(); });
        action(brls::BUTTON_Y, [this] { duplicate(); });
        action(brls::BUTTON_BACK, [this] { remove(); });
        rebuild(0);
    }

private:
    void fill() override {
        presets_ = app::loadPresets();
        for (size_t i = 0; i < presets_.size(); i++) {
            const auto& p = presets_[i];
            std::string sub = p.builtin ? "De serie · cualquier juego" : (p.game.empty() ? "Cualquier juego" : p.game) + " · " + p.date;
            auto* r = new ListRow(p.name, sub, p.builtin ? "" : "");
            r->setOnClick([this, i] { load(i); });
            addRow(r);
        }
    }

    const app::Preset* current() {
        size_t i = focused();
        return i < presets_.size() ? &presets_[i] : nullptr;
    }

    void load(size_t i) {
        auto& se = app::session();
        if (se.romPath.empty()) {
            ui::dialog("Primero abre una ROM", "Los presets se cargan sobre la ROM elegida. Ve a Abrir ROM y vuelve aquí desde el menú rápido (−) de Opciones.",
                       "Aceptar", nullptr);
            return;
        }
        if (!app::applyPreset(presets_[i])) {
            ui::dialog("Preset no válido", "No se pudo leer la configuración de este preset.", "Aceptar", nullptr, "", nullptr, true);
            return;
        }
        ui::toast("Preset «" + presets_[i].name + "» cargado");
        // vuelve a Opciones (recreada para que muestre los valores nuevos) o la abre si se llegó desde Inicio
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        if (fromOptions_) brls::Application::popActivity(brls::TransitionAnimation::NONE);
        pushOptions();
    }

    void rename() {
        const app::Preset* p = current();
        if (!p) return;
        if (p->builtin) {
            ui::toast("Los presets de serie no se pueden renombrar");
            return;
        }
        app::Preset copy = *p;
        size_t idx = focused();
        brls::Application::getImeManager()->openForText(
            [this, copy, idx](std::string name) mutable {
                if (name.empty() || name == copy.name) return;
                std::string oldFile = copy.file;
                copy.name = name;
                copy.file.clear();
                if (!app::savePreset(copy)) {
                    ui::dialog("Error al guardar", "No se pudo escribir el preset en la SD.", "Aceptar", nullptr, "", nullptr, true);
                    return;
                }
                if (copy.file != oldFile) {
                    app::Preset old;
                    old.file = oldFile;
                    app::deletePreset(old);
                }
                rebuild(idx);
                ui::toast("Preset renombrado");
            },
            "Renombrar preset", "Nombre nuevo", 32, copy.name);
    }

    void duplicate() {
        const app::Preset* p = current();
        if (!p) return;
        app::Preset copy = *p;
        copy.builtin = false;
        copy.file.clear();
        copy.date.clear();
        // nombre libre: «X (copia)», «X (copia 2)»...
        std::string base = p->name + " (copia";
        std::string name = base + ")";
        for (int n = 2; std::any_of(presets_.begin(), presets_.end(), [&](const app::Preset& q) { return q.name == name; }); n++)
            name = base + " " + std::to_string(n) + ")";
        copy.name = name;
        if (!app::savePreset(copy)) {
            ui::dialog("Error al guardar", "No se pudo escribir el preset en la SD.", "Aceptar", nullptr, "", nullptr, true);
            return;
        }
        rebuild(focused());
        ui::toast("Preset duplicado");
    }

    void remove() {
        const app::Preset* p = current();
        if (!p) return;
        if (p->builtin) {
            ui::toast("Los presets de serie no se pueden borrar");
            return;
        }
        app::Preset copy = *p;
        size_t idx = focused();
        ui::dialog("¿Borrar el preset «" + copy.name + "»?", "No se puede deshacer.", "Borrar",
                   [this, copy, idx] {
                       app::deletePreset(copy);
                       rebuild(idx);
                       ui::toast("Preset borrado");
                   },
                   "Cancelar");
    }

    std::vector<app::Preset> presets_;
    bool fromOptions_;
};

// ---------------- Historial ----------------
class HistoryScreen : public ListScreen {
public:
    HistoryScreen() {
        frame->header->title = "ROMs generadas";
        frame->header->rightText = std::to_string(app::history().size()) + " en total";
        frame->footer->hints = {{"A", "Abrir"}, {"−", "Borrar"}, {"B", "Atrás"}};
        action(brls::BUTTON_BACK, [this] { remove(); });
        rebuild(0);
    }

private:
    void fill() override {
        const auto& h = app::history();
        if (h.empty()) addEmpty("Aún no has randomizado nada. Empieza con Abrir ROM.");
        for (size_t i = 0; i < h.size(); i++) {
            const auto& e = h[i];
            auto* r = new ListRow(e.game + " · Random", "Preset «" + e.preset + "» · semilla " + std::to_string(e.seed) + " · " + e.date,
                                  e.platform);
            r->setOnClick([i] { pushResult(app::history()[i], false); });
            addRow(r);
        }
    }

    void remove() {
        size_t i = focused();
        auto& h = app::history();
        if (i >= h.size()) return;
        app::HistoryEntry e = h[i];
        ui::dialog("¿Borrar esta ROM generada?", "Se borrarán de la SD la ROM y su log:\n" + app::fileName(e.romPath), "Borrar",
                   [this, i] {
                       auto& h = app::history();
                       if (i >= h.size()) return;
                       std::remove(h[i].romPath.c_str());
                       if (!h[i].logPath.empty()) std::remove(h[i].logPath.c_str());
                       h.erase(h.begin() + static_cast<long>(i));
                       app::saveHistory();
                       frame->header->rightText = std::to_string(h.size()) + " en total";
                       rebuild(i);
                       ui::toast("ROM borrada");
                   },
                   "Cancelar");
    }
};

}  // namespace

void pushPresets(bool fromOptions) { brls::Application::pushActivity(new PresetsScreen(fromOptions), brls::TransitionAnimation::NONE); }
void pushHistory() { brls::Application::pushActivity(new HistoryScreen(), brls::TransitionAnimation::NONE); }

}  // namespace screens
