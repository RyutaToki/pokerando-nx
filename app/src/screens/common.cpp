#include "screens/screens.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace screens {

Screen::Screen() { frame = new ui::Frame(); }

void Screen::action(brls::ControllerButton b, std::function<void()> f, bool repeat) {
    frame->registerAction("", b, [f](brls::View*) {
        f();
        return true;
    }, true, repeat);
}

void popToHome() {
    // la pila empieza con una actividad vacía (Borealis nunca saca la primera); Inicio se crea de nuevo
    // para que muestre el historial actualizado
    while (brls::Application::getActivitiesStack().size() > 1) brls::Application::popActivity(brls::TransitionAnimation::NONE);
    pushHome();
}

bool emulatorAvailable(const std::string& platform, std::string* nroOut) {
    const auto& c = app::config();
    std::vector<std::string> candidates;
    bool gba = platform != "NDS";
    std::string emu = gba ? c.gbaEmulator : c.ndsEmulator;
    if (emu == "retroarch") {
        candidates = gba ? std::vector<std::string>{"sdmc:/retroarch/cores/mgba_libretro_libnx.nro"}
                         : std::vector<std::string>{"sdmc:/retroarch/cores/melonds_libretro_libnx.nro", "sdmc:/retroarch/cores/desmume_libretro_libnx.nro"};
    } else if (gba) {
        candidates = {"sdmc:/switch/mgba/mgba.nro", "sdmc:/switch/mgba.nro"};
    } else {
        candidates = {"sdmc:/switch/melonds/melonDS.nro", "sdmc:/switch/melonDS.nro", "sdmc:/switch/melonds.nro"};
    }
    for (const auto& p : candidates) {
        if (app::exists(p)) {
            if (nroOut) *nroOut = p;
            return true;
        }
    }
    return false;
}

bool launchEmulator(const std::string& romPath, const std::string& platform) {
    std::string nro;
    if (!emulatorAvailable(platform, &nro)) return false;
#ifdef __SWITCH__
    std::string args = "\"" + nro + "\" \"" + romPath + "\"";
    if (R_FAILED(envSetNextLoad(nro.c_str(), args.c_str()))) return false;
    brls::Application::quit();
    return true;
#else
    (void)romPath;
    return false;
#endif
}

}  // namespace screens
