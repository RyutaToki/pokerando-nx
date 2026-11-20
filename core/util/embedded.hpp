// Acceso a los archivos de core/data, incrustados en el ejecutable al compilar
// (así la app no depende de rutas en la SD para sus tablas).
#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace pr {
// ruta relativa a core/data, por ejemplo "romentries/gen3_offsets.ini"
std::optional<std::string_view> embeddedFile(std::string_view path);
}
