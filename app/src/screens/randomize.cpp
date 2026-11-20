// §6.4 Randomizar (en curso) y Resultado (terminado)
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>

#include "app/session.hpp"
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;
using Phase = pr::GameRandomizer::Phase;

enum class StepState { PENDING, RUNNING, DONE };

// spinner de 18 px dibujado a mano (arco que gira)
void spinner(NVGcontext* vg, float cx, float cy, float r, NVGcolor c) {
    double t = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    float a = static_cast<float>(std::fmod(t * 6.0, 2 * M_PI));
    nvgBeginPath(vg);
    nvgArc(vg, cx, cy, r, a, a + 4.2f, NVG_CW);
    nvgStrokeColor(vg, c);
    nvgStrokeWidth(vg, 2.5f);
    nvgLineCap(vg, NVG_ROUND);
    nvgStroke(vg);
}

// fila de un paso: fondo surface, radio 8, padding 14×18
class StepRow : public brls::View {
public:
    explicit StepRow(std::string label, bool compact = false) : label_(std::move(label)) {
        setHeight(compact ? 38 : 50);
        setMarginBottom(compact ? 6 : 8);
    }
    void setState(StepState s) { state_ = s; }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        const auto& c = colors();
        fillRRect(vg, x, y, w, h, 8, c.surface);
        float cy = y + h / 2;
        text(vg, x + 18, cy, label_, font::medium, 17, c.text);
        switch (state_) {
            case StepState::PENDING: text(vg, x + w - 18, cy, "Pendiente", font::medium, 16, c.textMuted, Align::RIGHT); break;
            case StepState::RUNNING: {
                float tw = text(vg, x + w - 18, cy, "En curso…", font::medium, 16, c.text, Align::RIGHT);
                spinner(vg, x + w - 18 - tw - 20, cy, 8, c.accent);
                break;
            }
            case StepState::DONE: text(vg, x + w - 18, cy, "Hecho", font::semibold, 16, c.accent, Align::RIGHT); break;
        }
    }

private:
    std::string label_;
    StepState state_ = StepState::PENDING;
};

// «¡ROM lista!» con el círculo y el ✓
class DoneBanner : public brls::View {
public:
    DoneBanner(std::string title, std::string sub) : title_(std::move(title)), sub_(std::move(sub)) {
        setHeight(56);
        setMarginBottom(24);
    }
    void draw(NVGcontext* vg, float x, float y, float, float h, brls::Style, brls::FrameContext*) override {
        const auto& c = colors();
        nvgBeginPath(vg);
        nvgCircle(vg, x + 28, y + h / 2, 28);
        nvgFillColor(vg, c.accent);
        nvgFill(vg);
        icon(vg, Icon::CHECK, x + 14, y + h / 2 - 14, 28, c.onAccent, 3);
        text(vg, x + 76, y + 18, title_, font::bold, 26, c.text);
        text(vg, x + 76, y + 44, sub_, font::regular, 16, c.textMuted);
    }

private:
    std::string title_, sub_;
};

// ruta en monoespaciada que salta de línea en cualquier carácter
class PathText : public brls::View {
public:
    explicit PathText(std::string p) : p_(std::move(p)) { setHeight(70); }
    void draw(NVGcontext* vg, float x, float y, float w, float, brls::Style, brls::FrameContext*) override {
        // nvgTextBox solo corta en espacios: se parte a mano por anchura
        const auto& c = colors();
        std::string line;
        float ly = y + 11;
        size_t i = 0;
        while (i < p_.size()) {
            size_t len = 1;
            while (i + len < p_.size() && (static_cast<unsigned char>(p_[i + len]) & 0xC0) == 0x80) len++;
            std::string ch = p_.substr(i, len);
            if (!line.empty() && textWidth(vg, line + ch, font::mono, 17) > w) {
                text(vg, x, ly, line, font::mono, 17, c.text);
                ly += 24;
                line.clear();
            }
            line += ch;
            i += len;
        }
        if (!line.empty()) text(vg, x, ly, line, font::mono, 17, c.text);
    }

private:
    std::string p_;
};

// tarjeta de una inicial (92 px, fondo bg)
class StarterCard : public brls::View {
public:
    explicit StarterCard(std::string n) : n_(std::move(n)) {
        setHeight(92);
        setGrow(1);
    }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        fillRRect(vg, x, y, w, h, 10, colors().bg);
        text(vg, x + w / 2, y + h / 2, ellipsize(vg, n_, font::semibold, 18, w - 16), font::semibold, 18, colors().text, Align::CENTER);
    }

private:
    std::string n_;
};

brls::Box* hspace(float w) {
    auto* b = new brls::Box();
    b->setWidth(w);
    return b;
}

std::string secondsText(double s) {
    char buf[32];
    snprintf(buf, sizeof buf, "%.1f", s);
    std::string r = buf;
    for (char& ch : r)
        if (ch == '.') ch = ',';
    return r;
}

// ---------------- Resultado ----------------
class ResultScreen : public Screen {
public:
    ResultScreen(const app::HistoryEntry& e, bool fresh, std::vector<Phase> phases) : e_(e) {
        const auto& c = colors();
        frame->header->title = fresh ? "Randomizar" : "Resultado";
        frame->header->rightText = e.game + " · semilla " + std::to_string(e.seed);
        frame->footer->hints = {{"A", "Aceptar"}, {"Y", "Otra semilla"}, {"B", "Volver al inicio"}};

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setPadding(32, 48, 32, 48);
        row->setGrow(1);

        auto* left = new brls::Box(brls::Axis::COLUMN);
        left->setWidth(560);
        left->setMarginRight(40);
        left->addView(new DoneBanner("¡ROM lista!", fresh ? "Completado en " + secondsText(e.seconds) + " s"
                                                          : "Generada el " + e.date + " · preset «" + e.preset + "»"));
        if (fresh) {
            for (Phase p : phases) {
                // debajo del «¡ROM lista!» caben 7 filas normales; si hay más, se compactan
                auto* s = new StepRow(app::phaseName(p), phases.size() > 7);
                s->setState(StepState::DONE);
                left->addView(s);
            }
        } else {
            auto* info = new Text("Preset «" + e.preset + "» · semilla " + std::to_string(e.seed), font::regular, 17, c.textMuted, 28);
            left->addView(info);
            if (!app::exists(e.romPath)) {
                auto* gone = new Text("El archivo ya no está en la SD.", font::medium, 17, c.warn, 28);
                left->addView(gone);
            }
        }
        row->addView(left);

        auto* panel = new Panel(c.surface, 14);
        panel->setAxis(brls::Axis::COLUMN);
        panel->setGrow(1);
        panel->setPadding(28, 28, 28, 28);
        auto* l1 = sectionLabel("GUARDADO EN");
        l1->setMarginBottom(8);
        panel->addView(l1);
        panel->addView(new PathText(e.romPath));

        auto* spacer = new brls::Box();
        spacer->setGrow(1);
        panel->addView(spacer);

        bool emu = emulatorAvailable(e.platform);
        if (!emu) {
            auto* note = new Text("Emulador no encontrado", font::regular, 15, c.textMuted, 24);
            note->setMarginBottom(6);
            panel->addView(note);
        }
        auto* buttons = new brls::Box(brls::Axis::ROW);
        std::string emuName = e.platform == "NDS" ? (app::config().ndsEmulator == "retroarch" ? "RetroArch" : "melonDS")
                                                  : (app::config().gbaEmulator == "retroarch" ? "RetroArch" : "mGBA");
        play_ = new Button("Jugar en " + emuName, true, [this] {
            if (!launchEmulator(e_.romPath, e_.platform))
                ui::dialog("No se pudo abrir el emulador", "Comprueba en Ajustes qué emulador está elegido y que esté instalado.",
                           "Aceptar", nullptr, "", nullptr, true);
        });
        play_->setGrow(1);
        play_->setEnabled(emu && app::exists(e.romPath));
        buttons->addView(play_);
        buttons->addView(hspace(12));
        log_ = new Button("Ver log de spoilers", false, [this] { pushLog(e_.logPath); });
        log_->setGrow(1);
        log_->setEnabled(!e.logPath.empty() && app::exists(e.logPath));
        buttons->addView(log_);
        panel->addView(buttons);
        row->addView(panel);
        frame->content->addView(row);

        action(brls::BUTTON_B, [] { popToHome(); });
        if (fresh) {
            action(brls::BUTTON_Y, [] {
                app::session().seed = app::newSeed();
                brls::Application::popActivity(brls::TransitionAnimation::NONE);
                pushRandomize();
            });
        } else {
            frame->footer->hints = {{"A", "Aceptar"}, {"B", "Atrás"}};
            action(brls::BUTTON_B, [] { brls::Application::popActivity(); });
        }
    }
    void onContentAvailable() override {
        if (play_->enabled()) brls::Application::giveFocus(play_);
        else if (log_->enabled()) brls::Application::giveFocus(log_);
        else brls::Application::giveFocus(play_);
    }

private:
    app::HistoryEntry e_;
    Button* play_;
    Button* log_;
};

// ---------------- En curso ----------------
struct Progress {
    std::atomic<int> phase{-1};
    std::atomic<bool> done{false};
    app::JobResult result;
};

class RandomizeScreen : public Screen {
public:
    RandomizeScreen() : prog_(std::make_shared<Progress>()) {
        auto& se = app::session();
        frame->header->title = "Randomizar";
        frame->header->rightText = se.info.gameName + " · semilla " + std::to_string(se.seed);
        frame->footer->hints = {};

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setPadding(32, 48, 32, 48);
        row->setGrow(1);
        auto* left = new brls::Box(brls::Axis::COLUMN);
        left->setWidth(560);
        phases_ = app::activePhases(se.opts);
        for (Phase p : phases_) {
            auto* s = new StepRow(app::phaseName(p));
            left->addView(s);
            rows_.push_back(s);
        }
        row->addView(left);
        auto* note = new Text("No apagues la consola ni cierres la app hasta que termine.", font::regular, 16, colors().textMuted, 24);
        note->setMarginLeft(40);
        note->setGrow(1);
        note->setWrap(true);
        row->addView(note);
        frame->content->addView(row);

        // sin botones: no se puede salir a mitad
        action(brls::BUTTON_B, [] {});
        action(brls::BUTTON_START, [] {});
    }

    void onContentAvailable() override {
        if (started_) return;
        started_ = true;
        auto& se = app::session();
        auto prog = prog_;
        std::string rom = se.romPath;
        app::OptState opts = se.opts;
        int64_t seed = se.seed;
        auto phases = phases_;
        // se deja dibujar un fotograma antes de empezar
        brls::delay(50, [=] {
            brls::async([=] {
                prog->result = app::runRandomization(rom, opts, seed, [prog, phases](Phase p) {
                    for (size_t i = 0; i < phases.size(); i++)
                        if (phases[i] == p) prog->phase = static_cast<int>(i);
                });
                prog->done = true;
            });
        });
    }

    // se llama en cada fotograma desde el marco
    void tick() {
        int cur = prog_->phase;
        bool done = prog_->done;
        for (size_t i = 0; i < rows_.size(); i++) {
            int ii = static_cast<int>(i);
            rows_[i]->setState(done || ii < cur ? StepState::DONE : ii == cur ? StepState::RUNNING : StepState::PENDING);
        }
        if (done && !finished_) {
            finished_ = true;
            finish();
        }
    }

private:
    void finish() {
        auto& se = app::session();
        const app::JobResult& r = prog_->result;
        if (!r.ok) {
            std::string err = r.error.empty() ? "Error desconocido." : r.error;
            brls::sync([err] {
                ui::dialog("Error al randomizar", err, "Aceptar", [] { brls::Application::popActivity(); }, "", nullptr, true);
            });
            return;
        }
        app::HistoryEntry e;
        e.game = se.info.gameName;
        e.shortName = se.info.shortName;
        e.platform = se.info.platform;
        e.preset = se.presetName;
        e.date = app::nowString();
        e.romPath = r.romOut;
        e.logPath = r.logOut;
        e.seed = se.seed;
        e.starters = r.starters;
        e.seconds = r.seconds;
        auto& h = app::history();
        // las randomizadas anteriores se han podido borrar: fuera del historial
        h.erase(std::remove_if(h.begin(), h.end(), [](const app::HistoryEntry& x) { return !app::exists(x.romPath); }), h.end());
        h.insert(h.begin(), e);
        app::saveHistory();
        // la siguiente vez, otra semilla
        se.seed = app::newSeed();
        auto phases = phases_;
        brls::sync([e, phases] {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            brls::Application::pushActivity(new ResultScreen(e, true, phases), brls::TransitionAnimation::NONE);
        });
    }

    std::shared_ptr<Progress> prog_;
    std::vector<Phase> phases_;
    std::vector<StepRow*> rows_;
    bool started_ = false, finished_ = false;
};

// vista invisible que llama a tick() en cada fotograma
class Ticker : public brls::View {
public:
    explicit Ticker(std::function<void()> f) : f_(std::move(f)) {
        setWidth(0);
        setHeight(0);
    }
    void draw(NVGcontext*, float, float, float, float, brls::Style, brls::FrameContext*) override { f_(); }

private:
    std::function<void()> f_;
};

}  // namespace

void pushRandomize() {
    auto* s = new RandomizeScreen();
    s->frame->addView(new Ticker([s] { s->tick(); }));
    brls::Application::pushActivity(s, brls::TransitionAnimation::NONE);
}

void pushResult(const app::HistoryEntry& e, bool fresh) {
    brls::Application::pushActivity(new ResultScreen(e, fresh, {}), brls::TransitionAnimation::NONE);
}

}  // namespace screens
