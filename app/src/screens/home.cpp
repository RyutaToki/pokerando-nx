// §6.1 Inicio
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;

// tarjeta de una ROM generada (fondo bg dentro del panel)
class HistoryCard : public Focusable {
public:
    explicit HistoryCard(app::HistoryEntry e) : e_(std::move(e)) {
        setHeight(86);
        setMarginBottom(12);
        setOnClick([this] { pushResult(e_, false); });
    }
    void drawContent(NVGcontext* vg, float x, float y, float w, float h) override {
        const auto& c = colors();
        fillRRect(vg, x, y, w, h, 10, c.bg);
        if (isFocused()) strokeRRect(vg, x, y, w, h, 10, 3, c.accent);
        float bw = textWidth(vg, e_.platform, font::bold, 14) + 20;
        badge(vg, x + w - 20 - bw, y + h / 2, e_.platform, e_.platform == "NDS" ? c.warn : c.accent);
        float maxW = w - 40 - bw - 16;
        text(vg, x + 20, y + 30, ellipsize(vg, e_.game + " · Random", font::semibold, 19, maxW), font::semibold, 19, c.text);
        std::string sub = "Preset «" + e_.preset + "» · semilla " + std::to_string(e_.seed);
        text(vg, x + 20, y + 58, ellipsize(vg, sub, font::regular, 15, maxW), font::regular, 15, c.textMuted);
    }

private:
    app::HistoryEntry e_;
};

class HomeScreen : public Screen {
public:
    HomeScreen() {
        auto& c = colors();
        frame->header->drawLeft = [](NVGcontext* vg, float x, float y, float, float h) { logo(vg, x, y + h / 2); };
        uint64_t free = app::freeSpace(app::sdRoot());
        frame->header->rightText = free ? "SD: " + app::formatSize(free) + " libres" : "";
        frame->footer->hints = {{"A", "Seleccionar"}, {"+", "Salir"}};

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setPadding(40, 48, 40, 48);
        row->setGrow(1);
        auto* left = new brls::Box(brls::Axis::COLUMN);
        left->setWidth(520);
        left->setMarginRight(40);
        auto add = [&](Icon i, const char* label, std::function<void()> f) {
            auto* b = new MenuButton(i, label);
            b->setMarginBottom(12);
            b->setOnClick(std::move(f));
            left->addView(b);
            return b;
        };
        first_ = add(Icon::FOLDER, "Abrir ROM", [] { pushBrowser(); });
        add(Icon::LIST, "Presets guardados", [] { pushPresets(); });
        add(Icon::CLOCK, "ROMs generadas", [] { pushHistory(); });
        add(Icon::GEAR, "Ajustes", [] { pushSettings(); });
        row->addView(left);

        auto* panel = new Panel(c.surface, 14);
        panel->setAxis(brls::Axis::COLUMN);
        panel->setGrow(1);
        panel->setPadding(32, 32, 32, 32);
        auto* title = new Text("Últimas generadas", font::semibold, 20, c.textMuted, 28);
        title->setMarginBottom(18);
        panel->addView(title);
        const auto& h = app::history();
        if (h.empty()) {
            auto* t = new Text("Aún no has randomizado nada. Empieza con Abrir ROM.", font::regular, 17, c.text, 60);
            t->setWrap(true);
            panel->addView(t);
        } else {
            for (size_t i = 0; i < h.size() && i < 3; i++) panel->addView(new HistoryCard(h[i]));
        }
        auto* spacer = new brls::Box();
        spacer->setGrow(1);
        panel->addView(spacer);
        auto* note = new Text("Las ROMs nuevas se guardan junto a la original. La original nunca se modifica.", font::regular, 15,
                              c.textMuted, 42);
        note->setWrap(true);
        panel->addView(note);
        row->addView(panel);
        frame->content->addView(row);

        action(brls::BUTTON_START, [] { brls::Application::quit(); });
        // en Inicio, B no hace nada (para no salir sin querer)
        action(brls::BUTTON_B, [] {});
    }
    void onContentAvailable() override { brls::Application::giveFocus(first_); }

private:
    brls::View* first_ = nullptr;
};
}  // namespace

void pushHome() { brls::Application::pushActivity(new HomeScreen(), brls::TransitionAnimation::NONE); }

}  // namespace screens
