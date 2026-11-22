// §6.2 Elegir ROM (explorador de la SD)
#include "app/shiny.hpp"
#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdlib>
#include <atomic>
#include <memory>

#include "app/session.hpp"
#include "screens/screens.hpp"

namespace screens {

namespace {
using namespace ui;

struct Entry {
    std::string name, path;
    bool dir = false, up = false, favorite = false;
    uint64_t size = 0;
};

bool lessNoCase(const std::string& a, const std::string& b) {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                        [](char x, char y) { return std::tolower((unsigned char)x) < std::tolower((unsigned char)y); });
}

bool romExt(const std::string& n) {
    if (n.size() < 4) return false;
    std::string e = n.substr(n.size() - 4);
    for (char& c : e) c = static_cast<char>(std::tolower((unsigned char)c));
    return e == ".gba" || e == ".nds";
}

std::string withSlash(std::string p) {
    if (p.empty() || p.back() != '/') p += '/';
    return p;
}

// estado del panel «Juego detectado»
struct Detection {
    std::string path;
    pr::RomInfo info;
    enum class Integrity { UNKNOWN, CHECKING, OK, MODIFIED } integrity = Integrity::UNKNOWN;
};

class DetectPanel : public brls::View {
public:
    explicit DetectPanel(std::shared_ptr<Detection> d) : d_(std::move(d)) {}
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        const auto& c = colors();
        const Detection& d = *d_;
        if (d.path.empty()) {
            textBox(vg, x, y, w, "Elige una ROM .gba o .nds de la lista para ver de qué juego se trata.", font::regular, 17, c.textMuted);
            return;
        }
        text(vg, x, y + 10, "JUEGO DETECTADO", font::semibold, 15, c.accent, Align::LEFT, 15 * 0.06f);
        const pr::RomInfo& in = d.info;
        std::string name = in.recognized ? in.gameName : "Juego no compatible";
        text(vg, x, y + 52, ellipsize(vg, name, font::bold, 28, w), font::bold, 28, c.text);
        float ry = y + 100;
        auto row = [&](const std::string& k, const std::string& v, NVGcolor vc) {
            text(vg, x, ry, k, font::medium, 17, c.textMuted);
            text(vg, x + 120, ry, ellipsize(vg, v, font::medium, 17, w - 120), font::medium, 17, vc);
            ry += 34;
        };
        row("Plataforma", in.platform.empty() ? "—" : (in.platform == "GBA" ? "Game Boy Advance" : "Nintendo DS"), c.text);
        row("Región", in.code.empty() ? "—" : in.region + " (" + in.code + ")", c.text);
        row("Generación", in.generation ? std::to_string(in.generation) + ".ª" : "—", c.text);
        if (in.supported) {
            switch (d.integrity) {
                case Detection::Integrity::OK: row("Integridad", "ROM original verificada", c.accent); break;
                case Detection::Integrity::MODIFIED: row("Integridad", "ROM modificada o hack", c.warn); break;
                case Detection::Integrity::CHECKING: row("Integridad", "Comprobando…", c.textMuted); break;
                default: row("Integridad", "—", c.textMuted); break;
            }
            if (d.integrity == Detection::Integrity::MODIFIED)
                textBox(vg, x, ry + 4, w, "Puede que no funcione bien: el randomizer está pensado para ROMs originales.", font::regular, 15, c.warn);
        } else if (!in.note.empty()) {
            textBox(vg, x, ry + 4, w, in.note, font::regular, 16, c.warn);
        }
    }

private:
    std::shared_ptr<Detection> d_;
};

class BrowserScreen : public Screen {
public:
    BrowserScreen() : det_(std::make_shared<Detection>()), alive_(std::make_shared<std::atomic<bool>>(true)) {
        const auto& c = colors();
        frame->header->title = app::session().pendingPreset ? "Elegir ROM · " + app::session().pendingPreset->name : "Elegir ROM";
        frame->header->drawRight = [this](NVGcontext* vg, float x, float y, float w, float h) {
            text(vg, x + w, y + h / 2, ellipsizeLeft(vg, app::displayPath(dir_), w - 260), font::mono, 16, colors().textMuted, Align::RIGHT);
        };
        frame->footer->hints = {{"A", "Abrir"}, {"B", "Atrás"}, {"Y", "Favoritos"}};

        auto* row = new brls::Box(brls::Axis::ROW);
        row->setPadding(28, 48, 28, 48);
        row->setGrow(1);
        anchor_ = new FocusAnchor();
        row->addView(anchor_);
        scroll_ = new brls::ScrollingFrame();
        scroll_->setWidth(680);
        scroll_->setMarginRight(32);
        list_ = new brls::Box(brls::Axis::COLUMN);
        list_->setPaddingRight(6);
        scroll_->setContentView(list_);
        row->addView(scroll_);

        auto* panel = new Panel(c.surface, 14);
        panel->setAxis(brls::Axis::COLUMN);
        panel->setGrow(1);
        panel->setPadding(28, 28, 28, 28);
        auto* info = new DetectPanel(det_);
        info->setGrow(1);
        panel->addView(info);
        continue_ = new Button("Continuar a opciones", true, [this] { openSelected(); });
        continue_->setEnabled(false);
        panel->addView(continue_);
        row->addView(panel);
        frame->content->addView(row);

        action(brls::BUTTON_B, [] { brls::Application::popActivity(); });
        action(brls::BUTTON_Y, [this] { toggleFavorite(); });

        std::string start = app::config().startAtLast ? app::config().lastDir : "";
        if (start.empty() || !app::isDir(start)) start = app::defaultRomDir();
        load(withSlash(start));
#if !defined(__SWITCH__)
        // prueba en el PC: PR_AUTOOPEN=ruta abre esa ROM como si se pulsara «Continuar»
        if (const char* auto_ = std::getenv("PR_AUTOOPEN")) {
            std::string path = auto_;
            brls::delay(300, [this, path] {
                det_->path = path;
                det_->info = pr::detectRom(path);
                openSelected();
            });
        }
#endif
    }
    // al abrir, el foco va a la primera ROM (o a la carpeta pedida), no a «.. (subir)»
    void onContentAvailable() override {
        if (initialFocus_) brls::Application::giveFocus(initialFocus_);
    }
    ~BrowserScreen() override {
        *alive_ = false;
        // si se sale del explorador sin elegir ROM, el preset pendiente se descarta
        app::session().pendingPreset.reset();
    }

private:
    static std::string ellipsizeLeft(NVGcontext* vg, const std::string& s, float maxW) {
        if (textWidth(vg, s, font::mono, 16) <= maxW) return s;
        std::string r = s;
        while (r.size() > 1 && textWidth(vg, "…" + r, font::mono, 16) > maxW) {
            r.erase(0, 1);
            while (!r.empty() && (static_cast<unsigned char>(r[0]) & 0xC0) == 0x80) r.erase(0, 1);
        }
        return "…" + r;
    }

    void load(const std::string& dir, const std::string& focusName = "") {
        dir_ = dir;
        std::vector<Entry> dirs, files;
        if (DIR* d = opendir(dir.c_str())) {
            while (dirent* e = readdir(d)) {
                std::string n = e->d_name;
                if (n == "." || n == ".." || n.empty() || n[0] == '.') continue;
                Entry en;
                en.name = n;
                en.path = dir + n;
                struct stat st;
                if (::stat(en.path.c_str(), &st) != 0) continue;
                en.dir = S_ISDIR(st.st_mode);
                en.size = static_cast<uint64_t>(st.st_size);
                if (en.dir) {
                    en.path = withSlash(en.path);
                    dirs.push_back(en);
                } else if (romExt(n) || !app::config().hideUnsupported) {
                    files.push_back(en);
                }
            }
            closedir(d);
        }
        std::sort(dirs.begin(), dirs.end(), [](const Entry& a, const Entry& b) { return lessNoCase(a.name, b.name); });
        std::sort(files.begin(), files.end(), [](const Entry& a, const Entry& b) { return lessNoCase(a.name, b.name); });
        entries_.clear();
        if (dir != app::sdRoot() && dir.size() > 1) {
            Entry up;
            up.name = ".. (subir)";
            up.path = app::parentDir(dir);
            up.up = true;
            entries_.push_back(up);
        }
        // favoritas primero (las que no son la carpeta actual)
        for (const auto& f : app::config().favorites) {
            if (withSlash(f) == dir || !app::isDir(f)) continue;
            Entry fav;
            fav.name = app::fileName(f.back() == '/' ? f.substr(0, f.size() - 1) : f);
            fav.path = withSlash(f);
            fav.dir = fav.favorite = true;
            entries_.push_back(fav);
        }
        entries_.insert(entries_.end(), dirs.begin(), dirs.end());
        entries_.insert(entries_.end(), files.begin(), files.end());
        rebuild(focusName);
    }

    void rebuild(const std::string& focusName) {
        auto stack = brls::Application::getActivitiesStack();
        bool shown = !stack.empty() && stack.back() == this;
        if (shown) anchor_->park();  // que el foco no quede en una fila que se borra
        list_->clearViews();
        rows_.clear();
        brls::View* focus = nullptr;
        bool firstFile = false;
        for (size_t i = 0; i < entries_.size(); i++) {
            const Entry& e = entries_[i];
            FileRow::Kind k = e.up ? FileRow::Kind::UP : e.dir ? FileRow::Kind::FOLDER : romExt(e.name) ? FileRow::Kind::FILE : FileRow::Kind::UNSUPPORTED;
            auto* r = new FileRow(k, e.name, (e.dir || e.up) ? "" : (k == FileRow::Kind::UNSUPPORTED ? "" : app::formatSize(e.size)));
            r->setStar(e.favorite || (e.dir && isFavorite(e.path)));
            r->setMarginBottom(6);
            r->setOnClick([this, i] { activate(i); });
            r->getFocusEvent()->subscribe([this, i](brls::View*) { focused(i); });
            list_->addView(r);
            rows_.push_back(r);
            bool preferred = !focusName.empty() ? e.name == focusName : (k == FileRow::Kind::FILE && !firstFile);
            if (k == FileRow::Kind::FILE) firstFile = true;
            if (!focus || preferred) focus = r;
        }
        if (entries_.empty()) {
            auto* t = new Text("Carpeta vacía", font::regular, 17, colors().textMuted, 40);
            list_->addView(t);
            focus = continue_;
        }
        det_->path.clear();
        continue_->setEnabled(false);
        if (shown) anchor_->release(focus);
        else initialFocus_ = focus;
    }

    bool isFavorite(const std::string& p) const {
        for (const auto& f : app::config().favorites)
            if (withSlash(f) == withSlash(p)) return true;
        return false;
    }

    void focused(size_t i) {
        if (i >= entries_.size()) return;
        const Entry& e = entries_[i];
        if (e.dir || e.up) {
            det_->path.clear();
            continue_->setEnabled(false);
            return;
        }
        if (det_->path == e.path) return;
        det_->path = e.path;
        det_->info = pr::detectRom(e.path);
        det_->integrity = Detection::Integrity::UNKNOWN;
        continue_->setEnabled(det_->info.supported);
        if (det_->info.supported && det_->info.expectedCRC32) {
            det_->integrity = Detection::Integrity::CHECKING;
            std::string path = e.path;
            uint32_t expected = *det_->info.expectedCRC32;
            auto det = det_;
            auto alive = alive_;
            brls::async([path, expected, det, alive] {
                uint32_t crc = 0;
                bool ok = true;
                try {
                    crc = pr::fileCRC32(path);
                } catch (...) {
                    ok = false;
                }
                brls::sync([=] {
                    if (!*alive || det->path != path) return;
                    det->integrity = ok && crc == expected ? Detection::Integrity::OK : Detection::Integrity::MODIFIED;
                });
            });
        } else if (det_->info.supported && det_->info.platform == "NDS") {
            // DS: la comprobación de FVX (CRC de la ARM9, los overlays y los archivos clave)
            det_->integrity = Detection::Integrity::CHECKING;
            std::string path = e.path;
            auto det = det_;
            auto alive = alive_;
            brls::async([path, det, alive] {
                bool valid = false;
                try {
                    valid = pr::openRom(path)->isRomValid();
                } catch (...) {
                }
                brls::sync([=] {
                    if (!*alive || det->path != path) return;
                    det->integrity = valid ? Detection::Integrity::OK : Detection::Integrity::MODIFIED;
                });
            });
        } else if (det_->info.supported) {
            det_->integrity = Detection::Integrity::MODIFIED;
        }
    }

    void activate(size_t i) {
        if (i >= entries_.size()) return;
        const Entry e = entries_[i];
        if (e.up) {
            std::string from = dir_;
            while (from.size() > 1 && from.back() == '/') from.pop_back();
            load(withSlash(e.path), app::fileName(from));
        } else if (e.dir) {
            load(e.path);
        } else {
            focused(i);
            if (det_->info.supported) openSelected();
            else ui::dialog("ROM no compatible", det_->info.note.empty() ? "Este archivo no es una ROM compatible." : det_->info.note,
                            "Aceptar", nullptr, "", nullptr, true);
        }
    }

    void toggleFavorite() {
        auto& favs = app::config().favorites;
        auto it = std::find_if(favs.begin(), favs.end(), [&](const std::string& f) { return withSlash(f) == dir_; });
        if (it == favs.end()) {
            favs.push_back(dir_);
            ui::toast("Carpeta añadida a favoritas");
        } else {
            favs.erase(it);
            ui::toast("Carpeta quitada de favoritas");
        }
        app::saveConfig();
    }

    void openSelected() {
        if (det_->path.empty() || !det_->info.supported) return;
        std::string path = det_->path;
        app::config().lastDir = dir_;
        app::saveConfig();
        auto alive = alive_;
        ui::toast("Cargando la ROM…");
        brls::async([this, path, alive] {
            std::shared_ptr<pr::RomHandler> rh;
            std::string err;
            try {
                rh = pr::openRom(path);
                app::shinyPatchAvailable(path);  // se deja calculado ya: así Extras abre al instante
            } catch (const std::exception& e) {
                err = e.what();
            }
            brls::sync([=, this] {
                if (!*alive) return;
                if (!rh) {
                    ui::dialog("No se pudo abrir la ROM", err, "Aceptar", nullptr, "", nullptr, true);
                    return;
                }
                auto& se = app::session();
                bool sameGame = se.preview && se.info.code == pr::detectRom(path).code;
                se.romPath = path;
                se.info = pr::detectRom(path);
                se.preview = rh;
                se.caps = app::RomCaps::from(*rh);
                // si cambia de juego, se mantienen las opciones pero ajustadas al nuevo juego
                app::tweakForRom(se.opts.s, *rh);
                if (!sameGame) se.seed = app::newSeed();
                if (se.pendingPreset) {
                    app::Preset p = *se.pendingPreset;
                    se.pendingPreset.reset();
                    frame->header->title = "Elegir ROM";  // si vuelve atrás, ya es un explorador normal
                    if (app::applyPreset(p)) ui::toast("Preset «" + p.name + "» aplicado");
                    else ui::dialog("Preset no válido", "No se pudo leer la configuración de este preset.", "Aceptar", nullptr, "", nullptr, true);
                }
                pushOptions();
            });
        });
    }

    std::shared_ptr<Detection> det_;
    std::shared_ptr<std::atomic<bool>> alive_;
    std::string dir_;
    std::vector<Entry> entries_;
    std::vector<FileRow*> rows_;
    brls::ScrollingFrame* scroll_;
    brls::Box* list_;
    Button* continue_;
    FocusAnchor* anchor_;
    brls::View* initialFocus_ = nullptr;
};
}  // namespace

void pushBrowser() { brls::Application::pushActivity(new BrowserScreen(), brls::TransitionAnimation::NONE); }

}  // namespace screens
