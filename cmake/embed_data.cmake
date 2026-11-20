# Genera un .cpp con el contenido de todos los archivos de DATA_DIR incrustados.
# Uso: cmake -DDATA_DIR=... -DOUTPUT=... -P embed_data.cmake
file(GLOB_RECURSE files RELATIVE "${DATA_DIR}" "${DATA_DIR}/*")
list(SORT files)
set(body "")
set(table "")
set(i 0)
foreach(f IN LISTS files)
    file(READ "${DATA_DIR}/${f}" hex HEX)
    string(LENGTH "${hex}" hexlen)
    math(EXPR len "${hexlen} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    string(APPEND body "static const unsigned char d${i}[] = {${bytes}0};\n")
    string(APPEND table "    {\"${f}\", {reinterpret_cast<const char*>(d${i}), ${len}}},\n")
    math(EXPR i "${i} + 1")
endforeach()
file(WRITE "${OUTPUT}.tmp" "// Generado por cmake/embed_data.cmake. No editar.
#include <map>
#include \"util/embedded.hpp\"
namespace {
${body}
const std::map<std::string_view, std::string_view>& files() {
    static const std::map<std::string_view, std::string_view> m = {
${table}    };
    return m;
}
}  // namespace
namespace pr {
std::optional<std::string_view> embeddedFile(std::string_view path) {
    auto it = files().find(path);
    if (it == files().end()) return std::nullopt;
    return it->second;
}
}  // namespace pr
")
file(COPY_FILE "${OUTPUT}.tmp" "${OUTPUT}" ONLY_IF_DIFFERENT)
file(REMOVE "${OUTPUT}.tmp")
