// Pantallas de la app (§6 de la especificación).
#pragma once
#include <borealis.hpp>
#include <functional>
#include <string>

#include "app/state.hpp"
#include "ui/widgets.hpp"

namespace screens {

// Actividad con el marco común (cabecera, contenido y pie)
class Screen : public brls::Activity {
public:
    Screen();
    brls::View* createContentView() override { return frame; }
    // registra una acción en toda la pantalla
    void action(brls::ControllerButton b, std::function<void()> f, bool repeat = false);
    ui::Frame* frame;
};

void pushHome();
void pushBrowser();
void pushOptions();
void pushRandomize();
void pushResult(const app::HistoryEntry& e, bool fresh);
void pushPresets(bool fromOptions = false);  // fromOptions: al cargar, recrea Opciones
void pushHistory();
void pushSettings();
void pushLog(const std::string& path);
void pushStarterPicker(std::function<void()> onDone);

// vuelve a la pantalla de Inicio cerrando las demás
void popToHome();

// abre la ROM en el emulador configurado; devuelve false si no se encuentra
bool emulatorAvailable(const std::string& platform, std::string* nroOut = nullptr);
bool launchEmulator(const std::string& romPath, const std::string& platform);

}  // namespace screens
