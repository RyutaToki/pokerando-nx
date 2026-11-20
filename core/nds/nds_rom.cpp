#include "nds/nds_rom.hpp"

#include <algorithm>
#include <stdexcept>

#include "nds/blz.hpp"

namespace pr::nds {

namespace {
constexpr int arm9_align = 0x1FF, arm7_align = 0x1FF, fnt_align = 0x1FF, fat_align = 0x1FF, banner_align = 0x1FF, file_align = 0x1FF;

int readArr(const Bytes& d, size_t o, int size) {
    uint32_t r = 0;
    for (int i = 0; i < size; i++) r |= static_cast<uint32_t>(d.at(o + i)) << (8 * i);
    return static_cast<int>(r);
}
void writeArr(Bytes& d, size_t o, int size, int value) {
    for (int i = 0; i < size; i++) d[o + i] = static_cast<uint8_t>((static_cast<uint32_t>(value) >> (8 * i)) & 0xFF);
}

const uint16_t kCrc16Table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};

uint16_t crc16(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) crc = (crc >> 8) ^ kCrc16Table[(crc ^ data[i]) & 0xFF];
    return static_cast<uint16_t>(crc);
}

// búsqueda sin solapamientos (como RomFunctions.search de FVX)
std::vector<int> searchAll(const Bytes& hay, const Bytes& needle) {
    std::vector<int> out;
    if (needle.empty()) return out;
    size_t i = 0;
    while (i + needle.size() <= hay.size()) {
        if (std::equal(needle.begin(), needle.end(), hay.begin() + i)) {
            out.push_back(static_cast<int>(i));
            i += needle.size();
        } else {
            i++;
        }
    }
    return out;
}

// escritura secuencial con huecos rellenos de ceros (seek más allá del final)
class Out {
public:
    explicit Out(const std::string& path) : f_(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc) {
        if (!f_) throw std::runtime_error("No se pudo crear " + path);
    }
    int64_t pos() const { return pos_; }
    void seek(int64_t p) {
        if (p > size_) {
            f_.seekp(size_);
            static const char zeros[4096] = {};
            int64_t n = p - size_;
            while (n > 0) {
                int64_t k = std::min<int64_t>(n, sizeof zeros);
                f_.write(zeros, k);
                n -= k;
            }
            size_ = p;
        }
        f_.seekp(p);
        pos_ = p;
    }
    void write(const uint8_t* d, size_t n) {
        f_.write(reinterpret_cast<const char*>(d), static_cast<std::streamsize>(n));
        pos_ += static_cast<int64_t>(n);
        size_ = std::max(size_, pos_);
    }
    void write(const Bytes& b) { write(b.data(), b.size()); }
    void writeInt(int64_t offset, int size, int value) {
        uint8_t buf[4];
        for (int i = 0; i < size; i++) buf[i] = static_cast<uint8_t>((static_cast<uint32_t>(value) >> (8 * i)) & 0xFF);
        seek(offset);
        write(buf, size);
    }
    Bytes read(int64_t offset, size_t n) {
        f_.flush();
        f_.seekg(offset);
        Bytes b(n);
        f_.read(reinterpret_cast<char*>(b.data()), static_cast<std::streamsize>(n));
        return b;
    }
    void close() {
        f_.flush();
        if (!f_) throw std::runtime_error("Error al escribir la ROM");
        f_.close();
    }

private:
    std::fstream f_;
    int64_t pos_ = 0, size_ = 0;
};
}  // namespace

NDSRom::NDSRom(const std::string& filename) : romFilename_(filename), base_(filename, std::ios::binary) {
    if (!base_) throw std::runtime_error("No se puede abrir " + filename);
    readFileSystem();
}

Bytes NDSRom::readBlock(int offset, int size) {
    if (size < 0) throw std::runtime_error("NDS: tamaño negativo");
    Bytes b(size);
    base_.clear();
    base_.seekg(static_cast<uint32_t>(offset));
    base_.read(reinterpret_cast<char*>(b.data()), size);
    if (base_.gcount() != size) throw std::runtime_error("NDS: lectura fuera de la ROM");
    return b;
}

int NDSRom::readInt(int offset, int size) { return readArr(readBlock(offset, size), 0, size); }

void NDSRom::readFileSystem() {
    Bytes hdr = readBlock(0, 0x60);
    romCode_.assign(reinterpret_cast<const char*>(&hdr[0x0C]), 4);
    version_ = hdr[0x1E];
    arm9_ramoffset_ = readArr(hdr, 0x28, 4);
    int fntOffset = readArr(hdr, 0x40, 4);
    int fatOffset = readArr(hdr, 0x48, 4);
    int fatSize = readArr(hdr, 0x4C, 4);
    fat_ = readBlock(fatOffset, fatSize);

    std::map<int, std::string> directoryPaths;
    directoryPaths[0xF000] = "";
    int dircount = readInt(fntOffset + 0x6, 2);
    int n = std::min(dircount, 0x1000);
    Bytes fntHead = readBlock(fntOffset, n * 8);
    std::vector<int> subTableOffsets(dircount), firstFileIDs(dircount), parentDirIDs(dircount);
    for (int i = 0; i < n; i++) {
        subTableOffsets[i] = readArr(fntHead, i * 8, 4) + fntOffset;
        firstFileIDs[i] = readArr(fntHead, i * 8 + 4, 2);
        parentDirIDs[i] = readArr(fntHead, i * 8 + 6, 2);
    }
    std::vector<std::string> directoryNames(dircount);
    std::vector<bool> hasName(dircount, false);
    std::map<int, std::string> filenames;
    std::map<int, int> fileDirectories;
    for (int i = 0; i < n; i++) firstPassDirectory(i, subTableOffsets[i], firstFileIDs[i], directoryNames, filenames, fileDirectories);
    // (en Java un nombre de carpeta nulo y uno vacío se tratan igual aquí)
    for (int i = 1; i < n; i++) {
        std::string dirname = directoryNames[i];
        if (!dirname.empty()) {
            std::string full;
            int curDir = i;
            while (!dirname.empty()) {
                if (!full.empty()) full.insert(0, "/");
                full.insert(0, dirname);
                int parentDir = parentDirIDs[curDir];
                if (parentDir >= 0xF001 && parentDir <= 0xFFFF) {
                    curDir = parentDir - 0xF000;
                    dirname = directoryNames[curDir];
                } else {
                    break;
                }
            }
            directoryPaths[i + 0xF000] = full;
        } else {
            directoryPaths[i + 0xF000] = "";
        }
    }
    for (const auto& [fileID, name] : filenames) {
        int directory = fileDirectories[fileID];
        const std::string& dirPath = directoryPaths[directory + 0xF000];
        File nf;
        nf.offset = readArr(fat_, fileID * 8, 4);
        nf.size = readArr(fat_, fileID * 8 + 4, 4) - nf.offset;
        nf.fullPath = dirPath.empty() ? name : dirPath + "/" + name;
        nf.fileID = fileID;
        fileList_.push_back(nf);
        files_[nf.fullPath] = static_cast<int>(fileList_.size()) - 1;
        filesByID_[fileID] = static_cast<int>(fileList_.size()) - 1;
    }

    int ovlTableOffset = readArr(hdr, 0x50, 4);
    int ovlTableSize = readArr(hdr, 0x54, 4);
    int ovlCount = ovlTableSize / 32;
    Bytes y9 = readBlock(ovlTableOffset, ovlTableSize);
    for (int i = 0; i < ovlCount; i++) {
        Overlay o;
        int fileID = readArr(y9, i * 32 + 24, 4);
        int start = readArr(fat_, fileID * 8, 4);
        int end = readArr(fat_, fileID * 8 + 4, 4);
        o.offset = start;
        o.size = o.original_size = end - start;
        o.fileID = fileID;
        o.overlay_id = i;
        o.ram_address = readArr(y9, i * 32 + 4, 4);
        o.ram_size = readArr(y9, i * 32 + 8, 4);
        o.bss_size = readArr(y9, i * 32 + 12, 4);
        o.static_start = readArr(y9, i * 32 + 16, 4);
        o.static_end = readArr(y9, i * 32 + 20, 4);
        o.compressed_size = readArr(y9, i * 32 + 28, 3);
        o.compress_flag = y9[i * 32 + 31];
        overlays_.push_back(o);
        overlaysByFileID_[fileID] = i;
    }
}

void NDSRom::firstPassDirectory(int dir, int subTableOffset, int firstFileID, std::vector<std::string>& dirNames,
                                std::map<int, std::string>& filenames, std::map<int, int>& fileDirs) {
    base_.clear();
    base_.seekg(subTableOffset);
    while (true) {
        int control = base_.get();
        if (control == std::char_traits<char>::eof()) throw std::runtime_error("NDS: tabla de nombres incorrecta");
        if (control == 0x00) break;
        int namelen = control & 0x7F;
        std::string name(namelen, '\0');
        base_.read(name.data(), namelen);
        if (control & 0x80) {
            uint8_t b[2];
            base_.read(reinterpret_cast<char*>(b), 2);
            int sub = (b[0] | (b[1] << 8)) - 0xF000;
            if (sub >= 0 && sub < static_cast<int>(dirNames.size())) dirNames[sub] = name;
        } else {
            int fileID = firstFileID++;
            filenames[fileID] = name;
            fileDirs[fileID] = dir;
        }
    }
}

NDSRom::File& NDSRom::extract(File& f) {
    if (!f.extracted) {
        f.data = readBlock(f.offset, f.size);
        f.extracted = true;
    }
    return f;
}

NDSRom::Overlay& NDSRom::extract(Overlay& o) {
    if (!o.extracted) {
        o.data = readBlock(o.offset, o.original_size);
        if (o.compress_flag != 0 && o.original_size == o.compressed_size && o.compressed_size != 0) {
            o.data = blzDecode(o.data);
            o.decompressed = true;
        }
        o.extracted = true;
    }
    return o;
}

std::optional<Bytes> NDSRom::getFile(const std::string& filename) {
    auto it = files_.find(filename);
    if (it == files_.end()) return std::nullopt;
    return extract(fileList_[it->second]).data;
}

std::optional<Bytes> NDSRom::getOverlay(int number) {
    if (number < 0 || number >= static_cast<int>(overlays_.size())) return std::nullopt;
    return extract(overlays_[number]).data;
}

int NDSRom::getOverlayAddress(int number) const {
    if (number < 0 || number >= static_cast<int>(overlays_.size())) return -1;
    return overlays_[number].ram_address;
}

void NDSRom::writeFile(const std::string& filename, const Bytes& data) {
    auto it = files_.find(filename);
    if (it == files_.end()) return;
    File& f = extract(fileList_[it->second]);
    f.data = data;
}

void NDSRom::writeOverlay(int number, const Bytes& data) {
    // (en FVX la comprobación es «<=», que con number == tamaño fallaría; aquí se ignora)
    if (number < 0 || number >= static_cast<int>(overlays_.size())) return;
    Overlay& o = extract(overlays_[number]);
    o.size = static_cast<int>(data.size());
    o.data = data;
}

Bytes NDSRom::getARM9() {
    if (arm9_open_) return arm9_;
    arm9_open_ = true;
    int arm9_offset = readInt(0x20, 4);
    int arm9_size = readInt(0x2C, 4);
    Bytes arm9 = readBlock(arm9_offset, arm9_size);
    // pie de la ARM9 (nitrocode)
    Bytes after = readBlock(arm9_offset + arm9_size, 12);
    if (static_cast<uint32_t>(readArr(after, 0, 4)) == 0xDEC00621u) {
        arm9_footer_ = after;
        arm9_has_footer_ = true;
    } else {
        arm9_has_footer_ = false;
    }
    auto tailIs = [&](size_t back, uint32_t v) { return static_cast<uint32_t>(readArr(arm9, arm9.size() - back, 4)) == v; };
    while (arm9.size() >= 12 && (tailIs(12, 0xDEC00621u) || (tailIs(12, 0) && tailIs(8, 0) && tailIs(4, 0)))) {
        if (!arm9_has_footer_) {
            arm9_has_footer_ = true;
            arm9_footer_.clear();
        }
        Bytes nf(arm9.end() - 12, arm9.end());
        nf.insert(nf.end(), arm9_footer_.begin(), arm9_footer_.end());
        arm9_footer_ = nf;
        arm9.resize(arm9.size() - 12);
    }
    arm9_compressed_ = false;
    arm9_szoffset_ = 0;
    int8_t c = static_cast<int8_t>(arm9[arm9.size() - 5]);
    if (c >= 0x08 && c <= 0x0B) {
        int compSize = readArr(arm9, arm9.size() - 8, 3);
        int len = static_cast<int>(arm9.size());
        if (compSize > (len * 9 / 10) && compSize < (len * 11 / 10)) {
            arm9_compressed_ = true;
            Bytes compLength(4);
            writeArr(compLength, 0, 4, len + arm9_ramoffset_);
            auto found = searchAll(arm9, compLength);
            if (found.size() == 1) arm9_szoffset_ = found[0];
            else throw std::runtime_error("Could not read ARM9 size offset. May be a bad ROM.");
        }
    }
    if (arm9_compressed_) arm9 = blzDecode(arm9);
    arm9_ = arm9;
    return arm9_;
}

void NDSRom::writeARM9(const Bytes& arm9) {
    if (!arm9_open_) getARM9();
    arm9_changed_ = true;
    arm9_ = arm9;
}

void NDSRom::saveTo(const std::string& filename) {
    Out out(filename);
    int headersize = readInt(0x84, 4);
    out.write(readBlock(0, headersize));

    auto copy = [&](int from, int bytes) {
        const int chunk = 256 * 1024;
        while (bytes > 0) {
            int n = std::min(chunk, bytes);
            out.write(readBlock(from, n));
            from += n;
            bytes -= n;
        }
    };

    // arm9
    int arm9_offset = static_cast<int>((out.pos() + arm9_align) & ~arm9_align);
    int old_arm9_offset = readInt(0x20, 4);
    int arm9_size = readInt(0x2C, 4);
    if (arm9_open_ && arm9_changed_) {
        Bytes newARM9 = arm9_;
        if (arm9_compressed_) {
            newARM9 = blzEncode(newARM9, true, false);
            if (arm9_szoffset_ > 0) writeArr(newARM9, arm9_szoffset_, 4, static_cast<int>(newARM9.size()) + arm9_ramoffset_);
        }
        arm9_size = static_cast<int>(newARM9.size());
        out.seek(arm9_offset);
        out.write(newARM9);
        if (arm9_has_footer_) out.write(arm9_footer_);
    } else {
        out.seek(arm9_offset);
        copy(old_arm9_offset, arm9_size + 12);
    }

    // tabla de overlays de la arm9 (se escribe al final)
    int arm9_ovl_offset = static_cast<int>(out.pos());
    int arm9_ovl_size = static_cast<int>(overlays_.size()) * 32;

    // arm7
    int arm7_offset = (arm9_ovl_offset + arm9_ovl_size + arm7_align) & ~arm7_align;
    int old_arm7_offset = readInt(0x30, 4);
    int arm7_size = readInt(0x3C, 4);
    out.seek(arm7_offset);
    copy(old_arm7_offset, arm7_size);

    // overlays de la arm7
    int arm7_ovl_offset = static_cast<int>(out.pos());
    int old_arm7_ovl_offset = readInt(0x58, 4);
    int arm7_ovl_size = readInt(0x5C, 4);
    out.seek(arm7_ovl_offset);
    copy(old_arm7_ovl_offset, arm7_ovl_size);

    // banner
    int banner_offset = static_cast<int>((out.pos() + banner_align) & ~banner_align);
    int old_banner_offset = readInt(0x68, 4);
    out.seek(banner_offset);
    copy(old_banner_offset, 0x840);

    // tabla de nombres (no cambia)
    int fnt_offset = static_cast<int>((out.pos() + fnt_align) & ~fnt_align);
    int old_fnt_offset = readInt(0x40, 4);
    int fnt_size = readInt(0x44, 4);
    out.seek(fnt_offset);
    copy(old_fnt_offset, fnt_size);

    // FAT
    int fat_offset = static_cast<int>((out.pos() + fat_align) & ~fat_align);
    int fat_size = static_cast<int>(fat_.size());

    Bytes newfat(fat_.size(), 0);
    Bytes y9table(overlays_.size() * 32, 0);
    int base_offset = fat_offset + fat_size;
    int filecount = fat_size / 8;
    for (int fid = 0; fid < filecount; fid++) {
        int offset_of_file = (base_offset + file_align) & ~file_align;
        int file_len = 0;
        bool copiedCustom = false;
        auto fit = filesByID_.find(fid);
        if (fit != filesByID_.end()) {
            File& f = fileList_[fit->second];
            if (f.extracted) {
                out.seek(offset_of_file);
                out.write(f.data);
                copiedCustom = true;
                file_len = static_cast<int>(f.data.size());
            }
        }
        auto oit = overlaysByFileID_.find(fid);
        if (oit != overlaysByFileID_.end()) {
            Overlay& o = overlays_[oit->second];
            if (o.extracted) {
                Bytes buf = o.data;
                if (o.decompressed) {
                    buf = blzEncode(buf, false, false);
                    o.compressed_size = static_cast<int>(buf.size());
                }
                out.seek(offset_of_file);
                out.write(buf);
                copiedCustom = true;
                file_len = static_cast<int>(buf.size());
            }
            int id = o.overlay_id;
            writeArr(y9table, id * 32, 4, id);
            writeArr(y9table, id * 32 + 4, 4, o.ram_address);
            writeArr(y9table, id * 32 + 8, 4, o.ram_size);
            writeArr(y9table, id * 32 + 12, 4, o.bss_size);
            writeArr(y9table, id * 32 + 16, 4, o.static_start);
            writeArr(y9table, id * 32 + 20, 4, o.static_end);
            writeArr(y9table, id * 32 + 24, 4, fid);
            writeArr(y9table, id * 32 + 28, 3, o.compressed_size);
            writeArr(y9table, id * 32 + 31, 1, o.compress_flag);
        }
        if (!copiedCustom) {
            int file_starts = readArr(fat_, fid * 8, 4);
            int file_ends = readArr(fat_, fid * 8 + 4, 4);
            file_len = file_ends - file_starts;
            out.seek(offset_of_file);
            copy(file_starts, file_len);
        }
        writeArr(newfat, fid * 8, 4, offset_of_file);
        writeArr(newfat, fid * 8 + 4, 4, offset_of_file + file_len);
        base_offset = offset_of_file + file_len;
    }

    out.seek(fat_offset);
    out.write(newfat);
    out.seek(arm9_ovl_offset);
    out.write(y9table);

    int newfilesize = base_offset;
    newfilesize = (newfilesize + 3) & ~3;
    int application_end_offset = newfilesize;
    if (newfilesize != base_offset) {
        out.seek(newfilesize - 1);
        uint8_t z = 0;
        out.write(&z, 1);
    }
    newfilesize |= newfilesize >> 16;
    newfilesize |= newfilesize >> 8;
    newfilesize |= newfilesize >> 4;
    newfilesize |= newfilesize >> 2;
    newfilesize |= newfilesize >> 1;
    newfilesize++;
    if (newfilesize <= 128 * 1024) newfilesize = 128 * 1024;
    int devcap = -18;
    for (int x = newfilesize; x != 0; x >>= 1) devcap++;
    int devicecap = devcap < 0 ? 0 : devcap;

    out.writeInt(0x20, 4, arm9_offset);
    out.writeInt(0x2C, 4, arm9_size);
    out.writeInt(0x30, 4, arm7_offset);
    out.writeInt(0x3C, 4, arm7_size);
    out.writeInt(0x40, 4, fnt_offset);
    out.writeInt(0x48, 4, fat_offset);
    out.writeInt(0x50, 4, arm9_ovl_offset);
    out.writeInt(0x58, 4, arm7_ovl_offset);
    out.writeInt(0x68, 4, banner_offset);
    out.writeInt(0x80, 4, application_end_offset);
    out.writeInt(0x14, 1, devicecap);

    Bytes h = out.read(0, 0x15E);
    out.writeInt(0x15E, 2, crc16(h.data(), h.size()));
    out.close();
}

}  // namespace pr::nds
