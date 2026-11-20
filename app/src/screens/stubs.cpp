// Pantallas pendientes (temporal)
#include "screens/screens.hpp"
namespace screens {
#define STUB(fn, ...) void fn(__VA_ARGS__) { ui::toast("Pendiente"); }
STUB(pushBrowser)
STUB(pushOptions)
STUB(pushRandomize)
void pushResult(const app::HistoryEntry&, bool) { ui::toast("Pendiente"); }
STUB(pushPresets)
STUB(pushHistory)
STUB(pushSettings)
void pushLog(const std::string&) {}
void pushStarterPicker(std::function<void()>) {}
}  // namespace screens
