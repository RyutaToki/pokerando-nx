// PokéRando NX — randomizer de Pokémon GBA/DS para Nintendo Switch (port de UPR FVX, GPLv3).
#include <borealis.hpp>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "app/session.hpp"
#include "app/state.hpp"
#include "screens/screens.hpp"
#include "ui/theme.hpp"

#if !defined(__SWITCH__)
#include <glad/glad.h>
#endif

namespace {

// actividad vacía en la base de la pila (Borealis nunca saca la primera)
class RootActivity : public brls::Activity {
public:
    brls::View* createContentView() override {
        auto* b = new brls::Box();
        b->setWidth(1280);
        b->setHeight(720);
        return b;
    }
};

#if !defined(__SWITCH__)
// Modo de captura para el desarrollo: PR_SNAPSHOT=archivo.ppm guarda la ventana tras unos fotogramas y sale.
void saveSnapshot(const char* path) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int w = vp[2], h = vp[3];
    std::vector<unsigned char> px(static_cast<size_t>(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    std::ofstream f(path, std::ios::binary);
    f << "P6\n" << w << " " << h << "\n255\n";
    for (int y = h - 1; y >= 0; y--) f.write(reinterpret_cast<const char*>(&px[static_cast<size_t>(y) * w * 3]), w * 3);
}
#endif

}  // namespace

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++)
        if (std::strcmp(argv[i], "-d") == 0) brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);

    brls::Platform::APP_LOCALE_DEFAULT = brls::LOCALE_ES;
    if (!brls::Application::init()) return EXIT_FAILURE;
    brls::Application::createWindow("PokéRando NX");
    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);
    brls::Application::setGlobalQuit(false);

    ui::loadFonts();
    app::loadConfig();
    ui::setAccent(app::config().accent);
    app::loadHistory();
    app::session().seed = app::newSeed();

    brls::Application::pushActivity(new RootActivity(), brls::TransitionAnimation::NONE);

    // arranque directo en una pantalla (solo para pruebas en el PC)
    const char* start = std::getenv("PR_START");
    const char* rom = std::getenv("PR_ROM");
    if (rom) {
        auto& se = app::session();
        se.romPath = rom;
        se.info = pr::detectRom(rom);
        try {
            se.preview = pr::openRom(rom);
            se.caps = app::RomCaps::from(*se.preview);
            se.opts.s.tweakForRom(*se.preview);
        } catch (...) {
        }
    }
    std::string s = start ? start : "home";
    if (s == "home") screens::pushHome();
    else if (s == "browser") screens::pushBrowser();
    else if (s == "options") screens::pushOptions();
    else if (s == "presets") screens::pushPresets();
    else if (s == "history") screens::pushHistory();
    else if (s == "settings") screens::pushSettings();
    else if (s == "randomize") screens::pushRandomize();
    else screens::pushHome();

#if !defined(__SWITCH__)
    const char* snap = std::getenv("PR_SNAPSHOT");
    int frames = 0;
    if (snap) brls::Application::setAutomaticDeactivation(false);
#endif
    while (brls::Application::mainLoop()) {
#if !defined(__SWITCH__)
        if (snap && ++frames == (std::getenv("PR_FRAMES") ? std::atoi(std::getenv("PR_FRAMES")) : 40)) {
            saveSnapshot(snap);
            break;
        }
#endif
    }
    return EXIT_SUCCESS;
}
