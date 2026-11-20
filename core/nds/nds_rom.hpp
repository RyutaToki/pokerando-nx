// ROM de Nintendo DS: sistema de archivos (FNT/FAT), ARM9 y overlays. Port de newnds/NDSRom.java de FVX
// (basado en ndstool de devkitPro). Lee los archivos bajo demanda y al guardar copia el resto de la ROM
// original sin cargarla entera en memoria (las ROMs de 5.ª generación ocupan hasta 512 MB).
#pragma once
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace pr::nds {

using Bytes = std::vector<uint8_t>;

class NDSRom {
public:
    explicit NDSRom(const std::string& filename);

    const std::string& getCode() const { return romCode_; }
    uint8_t getVersion() const { return version_; }

    // nullopt si el archivo no existe
    std::optional<Bytes> getFile(const std::string& filename);
    std::optional<Bytes> getOverlay(int number);
    int getOverlayAddress(int number) const;
    Bytes getARM9();

    void writeFile(const std::string& filename, const Bytes& data);
    void writeOverlay(int number, const Bytes& data);
    void writeARM9(const Bytes& arm9);

    void saveTo(const std::string& filename);

    bool hasFile(const std::string& filename) const { return files_.count(filename) > 0; }

private:
    struct File {
        int offset = 0, size = 0, fileID = 0;
        std::string fullPath;
        bool extracted = false;
        Bytes data;
    };
    struct Overlay {
        int offset = 0, size = 0, original_size = 0, fileID = 0, overlay_id = 0;
        int ram_address = 0, ram_size = 0, bss_size = 0, static_start = 0, static_end = 0;
        int compressed_size = 0, compress_flag = 0;
        bool extracted = false, decompressed = false;
        Bytes data;
    };

    void readFileSystem();
    void firstPassDirectory(int dir, int subTableOffset, int firstFileID, std::vector<std::string>& dirNames,
                            std::map<int, std::string>& filenames, std::map<int, int>& fileDirs);
    Bytes readBlock(int offset, int size);
    int readInt(int offset, int size);
    File& extract(File& f);
    Overlay& extract(Overlay& o);

    std::string romFilename_;
    std::ifstream base_;
    std::string romCode_;
    uint8_t version_ = 0;
    std::map<std::string, int> files_;     // ruta -> índice en fileList_
    std::map<int, int> filesByID_;         // id -> índice en fileList_
    std::vector<File> fileList_;
    std::vector<Overlay> overlays_;
    std::map<int, int> overlaysByFileID_;  // id de archivo -> nº de overlay
    Bytes fat_;
    bool arm9_open_ = false, arm9_changed_ = false, arm9_has_footer_ = false, arm9_compressed_ = false;
    int arm9_ramoffset_ = 0, arm9_szoffset_ = 0;
    Bytes arm9_footer_, arm9_;
};

}  // namespace pr::nds
