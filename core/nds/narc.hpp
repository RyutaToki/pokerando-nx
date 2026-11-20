// Archivo NARC (contenedor de archivos de las ROMs de DS). Port de NARCArchive.java de FVX.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace pr::nds {
class NARCArchive {
public:
    NARCArchive() = default;  // vacío y sin nombres de archivo
    explicit NARCArchive(const std::vector<uint8_t>& data);
    std::vector<uint8_t> getBytes() const;

    std::vector<std::vector<uint8_t>> files;

private:
    std::vector<std::string> filenames_;
    bool hasFilenames_ = false;
};
}  // namespace pr::nds
