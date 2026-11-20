// §6.3 Opciones del randomizer
#include "app/session.hpp"
#include "app/shiny.hpp"
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;

class OptionsScreen : public Screen {
public:
    OptionsScreen() {
        auto& se = app::session();
        const auto& c = colors();
        frame->header->title = "Opciones · " + se.info.gameName;
        frame->header->drawRight = [](NVGcontext* vg, float x, float y, float w, float h) {
            auto& se = app::session();
            const auto& c = colors();
            float cy = y + h / 2;
            std::string seed = std::to_string(se.seed);
            float rx = x + w;
            rx -= text(vg, rx, cy, seed, font::mono, 16, c.text, Align::RIGHT);
            rx -= text(vg, rx - 6, cy, "Semilla:", font::regular, 16, c.textMuted, Align::RIGHT) + 6 + 28;
            rx -= text(vg, rx, cy, se.presetName, font::medium, 16, c.text, Align::RIGHT);
            text(vg, rx - 6, cy, "Preset:", font::regular, 16, c.textMuted, Align::RIGHT);
        };
        frame->header->addGestureRecognizer(new brls::TapGestureRecognizer([this](brls::TapGestureStatus st, brls::Sound*) {
            if (st.state == brls::GestureState::END) editSeed();
        }));
        frame->footer->leftHints = {{"L / R", "Cambiar pestaña"}};
        frame->footer->hints = {{"A", "Cambiar"}, {"Y", "Semilla"}, {"X", "Guardar preset"}, {"+", "Randomizar", true}};

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setGrow(1);
        anchor_ = new FocusAnchor();
        row->addView(anchor_);

        sidebar_ = new Panel(c.sidebar, 0);
        sidebar_->setAxis(brls::Axis::COLUMN);
        sidebar_->setWidth(300);
        sidebar_->setPadding(16, 0, 16, 0);
        for (size_t i = 0; i < app::tabs().size(); i++) {
            const auto& t = app::tabs()[i];
            if (t.available && !t.available(se.caps)) continue;
            auto* item = new TabItem(t.title);
            item->getFocusEvent()->subscribe([this, i](brls::View*) { selectTab(i, false); });
            item->setOnClick([this] { focusContent(); });
            tabItems_.push_back({i, item});
            sidebar_->addView(item);
        }
        row->addView(sidebar_);

        scroll_ = new brls::ScrollingFrame();
        scroll_->setGrow(1);
        body_ = new brls::Box(brls::Axis::COLUMN);
        body_->setPadding(24, 44, 32, 44);
        scroll_->setContentView(body_);
        row->addView(scroll_);
        frame->content->addView(row);

        action(brls::BUTTON_LB, [this] { cycleTab(-1); });
        action(brls::BUTTON_RB, [this] { cycleTab(1); });
        action(brls::BUTTON_X, [this] { savePreset(); });
        action(brls::BUTTON_Y, [this] { editSeed(); });
        action(brls::BUTTON_START, [this] { randomize(); });
        action(brls::BUTTON_BACK, [this] { quickMenu(); });
        action(brls::BUTTON_B, [this] { back(); });

        current_ = tabItems_.empty() ? 0 : tabItems_[0].first;
        if (const char* t = std::getenv("PR_TAB")) current_ = std::min<size_t>(std::atoi(t), app::tabs().size() - 1);
        buildTab();
    }

    void onContentAvailable() override {
        for (auto& [i, item] : tabItems_)
            if (i == current_) brls::Application::giveFocus(item);
    }

private:
    bool shown() {
        auto st = brls::Application::getActivitiesStack();
        return !st.empty() && st.back() == this;
    }

    void selectTab(size_t i, bool focusFirst) {
        if (i == current_ && built_) {
            if (focusFirst) focusContent();
            return;
        }
        current_ = i;
        bool focusInContent = focusInBody();
        if (shown() && focusInContent) anchor_->park();
        buildTab();
        if (shown() && focusInContent) anchor_->release(firstControl_ ? firstControl_ : tabItem(i));
        else if (focusFirst) focusContent();
    }

    bool focusInBody() {
        brls::View* f = brls::Application::getCurrentFocus();
        for (brls::View* v = f; v; v = v->hasParent() ? v->getParent() : nullptr)
            if (v == body_) return true;
        return false;
    }

    TabItem* tabItem(size_t i) {
        for (auto& [k, item] : tabItems_)
            if (k == i) return item;
        return nullptr;
    }

    void cycleTab(int d) {
        if (tabItems_.empty()) return;
        size_t pos = 0;
        for (size_t k = 0; k < tabItems_.size(); k++)
            if (tabItems_[k].first == current_) pos = k;
        pos = (pos + tabItems_.size() + d) % tabItems_.size();
        bool inSidebar = !focusInBody();
        selectTab(tabItems_[pos].first, false);
        if (inSidebar) brls::Application::giveFocus(tabItems_[pos].second);
    }

    void focusContent() {
        if (firstControl_) brls::Application::giveFocus(firstControl_);
    }

    void buildTab() {
        built_ = true;
        auto& se = app::session();
        const auto& c = colors();
        const app::TabDef& tab = app::tabs()[current_];
        for (auto& [k, item] : tabItems_) item->setActive(k == current_);
        body_->clearViews();
        toggles_.clear();
        firstControl_ = nullptr;
        modeWidget_ = nullptr;

        // título (H2) y etiqueta «Exclusivo de esta app»
        auto* title = new Heading(tab.title, tab.exclusive);
        title->setMarginBottom(18);
        body_->addView(title);

        if (tab.mode) {
            const auto& m = *tab.mode;
            auto* seg = new Segmented(m.label, m.names, m.get(se.opts), [this, &m](int i) {
                auto& se = app::session();
                m.set(se.opts, i);
                changed();
                if (&m == &*app::tabs()[1].mode && i == 1) pushStarterPicker([this] { refresh(); });
            });
            seg->setMarginBottom(18);
            modeWidget_ = seg;
            body_->addView(seg);
            setFirst(seg);
        }

        // iniciales elegidas a mano
        if (current_ == 1) {
            customStarters_ = new FileRow(FileRow::Kind::FILE, "");
            customStarters_->setMarginBottom(18);
            customStarters_->setOnClick([this] { pushStarterPicker([this] { refresh(); }); });
            body_->addView(customStarters_);
        } else {
            customStarters_ = nullptr;
        }

        if (tab.shinyPanel) {
            bool avail = se.preview && app::shinyPatchAvailable(*se.preview);
            auto* p = new Panel(c.surface, 12);
            p->setAxis(brls::Axis::COLUMN);
            p->setPadding(18, 20, 18, 20);
            p->setMarginBottom(18);
            p->addView(new ShinyTitle(avail));
            std::vector<std::string> labels;
            for (const auto& o : app::kShinyOdds) labels.push_back(o.label);
            auto* chips = new Chips(labels, avail ? se.opts.shiny : 0, [this](int i) {
                app::session().opts.shiny = i;
                changed();
            });
            chips->setMarginTop(14);
            chips->setEnabled(avail);
            p->addView(chips);
            auto* note = new Text(avail ? "Afecta a salvajes, huevos y regalos. Los entrenadores rivales no cambian."
                                        : "No disponible para este juego.",
                                  font::regular, 15, c.textMuted, 30);
            note->setMarginTop(10);
            p->addView(note);
            body_->addView(p);
            if (avail) setFirst(chips);
        }

        // rejilla de interruptores en 2 columnas
        std::vector<const app::ToggleDef*> defs;
        for (const auto& t : tab.toggles)
            if (!t.available || t.available(se.caps)) defs.push_back(&t);
        for (size_t i = 0; i < defs.size(); i += 2) {
            auto* r = new brls::Box(brls::Axis::ROW);
            r->setMarginBottom(8);
            for (size_t k = i; k < i + 2; k++) {
                if (k >= defs.size()) {
                    auto* spacer = new brls::Box();
                    spacer->setGrow(1);
                    spacer->setWidth(10);
                    r->addView(spacer);
                    continue;
                }
                const app::ToggleDef* d = defs[k];
                auto* t = new Toggle(d->label, d->get(se.opts), [this, d](bool on) {
                    d->set(app::session().opts, on);
                    changed();
                });
                t->setGrow(1);
                t->setWidth(10);
                if (k == i) t->setMarginRight(8);
                toggles_.push_back({d, t});
                r->addView(t);
            }
            body_->addView(r);
        }

        if (tab.slider) {
            const auto& sd = *tab.slider;
            auto* sl = new Slider(sd.label, sd.get(se.opts), [this, &sd](int v) {
                sd.set(app::session().opts, v);
                changed();
            });
            sl->setMarginTop(10);
            slider_ = {&sd, sl};
            body_->addView(sl);
        } else {
            slider_ = {nullptr, nullptr};
        }
        refresh();
        for (auto& [d, t] : toggles_) setFirst(t);
        if (slider_.second) setFirst(slider_.second);
    }

    void setFirst(brls::View* v) {
        if (firstControl_) return;
        if (auto* b = dynamic_cast<brls::Box*>(v)) {
            // primer hijo enfocable
            brls::View* d = b->getDefaultFocus();
            if (d) firstControl_ = d;
            return;
        }
        if (v->isFocusable()) firstControl_ = v;
    }

    // vuelve a leer el estado de todas las opciones (dependencias entre ellas)
    void refresh() {
        auto& se = app::session();
        const app::TabDef& tab = app::tabs()[current_];
        if (modeWidget_ && tab.mode) modeWidget_->setSelected(tab.mode->get(se.opts));
        for (auto& [d, t] : toggles_) {
            t->setOn(d->get(se.opts));
            t->setEnabled(!d->enabled || d->enabled(se.opts));
        }
        if (slider_.second) slider_.second->setEnabled(!slider_.first->enabled || slider_.first->enabled(se.opts));
        if (customStarters_) {
            bool custom = se.opts.s.startersMod == pr::Settings::StartersMod::CUSTOM;
            customStarters_->setVisibility(custom ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
            std::string names;
            if (se.preview) {
                std::vector<pr::Species*> list;
                for (pr::Species* s : se.preview->getSpeciesInclFormes())
                    if (!s || !s->isEssentiallyCosmetic()) list.push_back(s);
                for (int idx : se.opts.s.customStarters) {
                    if (idx <= 0 || idx >= static_cast<int>(list.size()) || !list[idx]) continue;
                    names += (names.empty() ? "" : ", ") + list[idx]->fullName();
                }
            }
            customStarters_->setName("Iniciales elegidas: " + (names.empty() ? std::string("ninguna (pulsa para elegir)") : names));
        }
    }

    void changed() {
        auto& se = app::session();
        se.dirty = true;
        std::string now = se.opts.s.toString();
        if (now != se.presetSettings || se.opts.shiny != se.presetShiny) se.presetName = "Personalizado";
        refresh();
    }

    void editSeed() {
        brls::Application::getImeManager()->openForNumber(
            [](long v) {
                if (v >= 0) app::session().seed = v;
            },
            "Semilla", "Escribe la semilla (un número)", 18, std::to_string(app::session().seed));
    }

    void savePreset() {
        auto& se = app::session();
        std::string initial = se.presetName == "Personalizado" ? "" : se.presetName;
        brls::Application::getImeManager()->openForText(
            [](std::string name) {
                if (name.empty()) return;
                auto& se = app::session();
                app::Preset p;
                p.name = name;
                p.game = se.info.gameName;
                p.settings = se.opts.s.toString();
                p.shiny = se.opts.shiny;
                for (const auto& old : app::loadPresets())
                    if (!old.builtin && old.name == name) p.file = old.file;
                if (app::savePreset(p)) {
                    se.presetName = name;
                    se.presetSettings = p.settings;
                    se.presetShiny = p.shiny;
                    se.dirty = false;
                    ui::toast("Preset guardado");
                } else {
                    ui::dialog("Error al guardar", "No se pudo escribir el preset en la SD.", "Aceptar", nullptr, "", nullptr, true);
                }
            },
            "Guardar preset", "Nombre del preset", 32, initial);
    }

    void quickMenu() {
        ui::menu("Menú rápido",
                 {{"Cargar preset", [] { pushPresets(true); }},
                  {"Restablecer pestaña",
                   [this] {
                       app::tabs()[current_].reset(app::session().opts);
                       changed();
                       rebuildKeepingFocus();
                   }},
                  {"Restablecer todo",
                   [this] {
                       auto& se = app::session();
                       se.opts.s = app::defaultSettings();
                       if (se.preview) app::tweakForRom(se.opts.s, *se.preview);
                       se.opts.shiny = 0;
                       changed();
                       rebuildKeepingFocus();
                   }},
                  {"Pegar settings string",
                   [this] {
                       brls::Application::getImeManager()->openForText(
                           [this](std::string str) {
                               if (str.empty()) return;
                               std::string err;
                               if (app::applySettingsString(str, &err)) {
                                   changed();
                                   rebuildKeepingFocus();
                                   ui::toast("Settings string pegada");
                               } else {
                                   ui::dialog("Settings string no válida", err, "Aceptar", nullptr, "", nullptr, true);
                               }
                           },
                           "Pegar settings string", "Settings string de UPR FVX", 400, "");
                   }}});
    }

    void rebuildKeepingFocus() {
        bool in = focusInBody();
        if (in) anchor_->park();
        buildTab();
        if (in) anchor_->release(firstControl_);
    }

    void back() {
        if (!app::session().dirty) {
            brls::Application::popActivity();
            return;
        }
        ui::dialog("¿Salir sin guardar el preset?", "Las opciones se mantienen mientras la app esté abierta.", "Salir",
                   [] { brls::Application::popActivity(); }, "Cancelar");
    }

    void randomize() {
        auto& se = app::session();
        if (se.opts.s.startersMod == pr::Settings::StartersMod::CUSTOM) {
            int n = 0;
            for (int v : se.opts.s.customStarters) n += v > 0;
            if (n == 0) {
                ui::dialog("Faltan las iniciales", "Has elegido «Elegir a mano» pero no hay ninguna inicial elegida.", "Elegir",
                           [this] { pushStarterPicker([this] { refresh(); }); }, "Cancelar", nullptr, true);
                return;
            }
        }
        pushRandomize();
    }

    // H2 + etiqueta de exclusivo
    class Heading : public brls::View {
    public:
        Heading(std::string t, bool excl) : t_(std::move(t)), excl_(excl) { setHeight(36); }
        void draw(NVGcontext* vg, float x, float y, float, float h, brls::Style, brls::FrameContext*) override {
            float w = text(vg, x, y + h / 2, t_, font::bold, 26, colors().text);
            if (excl_) exclusiveTag(vg, x + w + 16, y + h / 2);
        }

    private:
        std::string t_;
        bool excl_;
    };

    class ShinyTitle : public brls::View {
    public:
        explicit ShinyTitle(bool avail) : avail_(avail) { setHeight(26); }
        void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
            text(vg, x, y + h / 2, "Probabilidad de shiny", font::semibold, 19, colors().text);
            text(vg, x + w, y + h / 2, avail_ ? "Original: 1 entre 8192" : "No disponible para este juego", font::regular, 16,
                 avail_ ? colors().textMuted : colors().warn, Align::RIGHT);
        }

    private:
        bool avail_;
    };

    size_t current_ = 0;
    bool built_ = false;
    std::vector<std::pair<size_t, TabItem*>> tabItems_;
    Panel* sidebar_;
    brls::ScrollingFrame* scroll_;
    brls::Box* body_;
    FocusAnchor* anchor_;
    brls::View* firstControl_ = nullptr;
    Segmented* modeWidget_ = nullptr;
    FileRow* customStarters_ = nullptr;
    std::vector<std::pair<const app::ToggleDef*, Toggle*>> toggles_;
    std::pair<const app::SliderDef*, Slider*> slider_{nullptr, nullptr};
};
}  // namespace

void pushOptions() { brls::Application::pushActivity(new OptionsScreen(), brls::TransitionAnimation::NONE); }

}  // namespace screens
