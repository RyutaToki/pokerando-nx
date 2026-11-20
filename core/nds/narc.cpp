#include "nds/narc.hpp"

#include <map>
#include <stdexcept>

namespace pr::nds {

namespace {
int readWord(const std::vector<uint8_t>& d, size_t o) { return d.at(o) | (d.at(o + 1) << 8); }
int readLong(const std::vector<uint8_t>& d, size_t o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeWord(std::vector<uint8_t>& d, size_t o, int v) {
    d[o] = static_cast<uint8_t>(v & 0xFF);
    d[o + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}
void writeLong(std::vector<uint8_t>& d, size_t o, int v) {
    for (int i = 0; i < 4; i++) d[o + i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFF);
}

std::map<std::string, std::vector<uint8_t>> readNitroFrames(const std::vector<uint8_t>& data) {
    int frameCount = readWord(data, 0x0E);
    int offset = 0x10;
    std::map<std::string, std::vector<uint8_t>> frames;
    for (int i = 0; i < frameCount; i++) {
        std::string magic{static_cast<char>(data.at(offset + 3)), static_cast<char>(data.at(offset + 2)),
                          static_cast<char>(data.at(offset + 1)), static_cast<char>(data.at(offset))};
        int frame_size = readLong(data, offset + 4);
        // hacks que no actualizan el tamaño de sus NARC ampliados
        if (i == frameCount - 1 && offset + frame_size < static_cast<int>(data.size())) frame_size = static_cast<int>(data.size()) - offset;
        if (frame_size < 8 || offset + frame_size > static_cast<int>(data.size())) throw std::runtime_error("NARC: marco incorrecto");
        frames[magic] = std::vector<uint8_t>(data.begin() + offset + 8, data.begin() + offset + frame_size);
        offset += frame_size;
    }
    return frames;
}
}  // namespace

NARCArchive::NARCArchive(const std::vector<uint8_t>& data) {
    auto frames = readNitroFrames(data);
    if (!frames.count("FATB") || !frames.count("FNTB") || !frames.count("FIMG")) throw std::runtime_error("Not a valid narc file");
    const auto& fatb = frames["FATB"];
    const auto& fimg = frames["FIMG"];
    int fileCount = readLong(fatb, 0);
    for (int i = 0; i < fileCount; i++) {
        int start = readLong(fatb, 4 + i * 8);
        int end = readLong(fatb, 8 + i * 8);
        if (start < 0 || end < start || end > static_cast<int>(fimg.size())) throw std::runtime_error("NARC: archivo fuera de rango");
        files.emplace_back(fimg.begin() + start, fimg.begin() + end);
    }
    const auto& fntb = frames["FNTB"];
    int unk1 = readLong(fntb, 0);
    if (unk1 == 8) {
        hasFilenames_ = true;
        // (como en FVX: el desplazamiento no avanza tras cada nombre)
        int offset = 8;
        for (int i = 0; i < fileCount; i++) {
            int fnLength = fntb.at(offset);
            offset++;
            filenames_.emplace_back(reinterpret_cast<const char*>(&fntb.at(offset)), fnLength);
        }
    } else {
        hasFilenames_ = false;
        filenames_.assign(fileCount, std::string());
    }
}

std::vector<uint8_t> NARCArchive::getBytes() const {
    int bytesRequired = 0;
    for (const auto& f : files) bytesRequired += static_cast<int>((f.size() + 3) / 4 * 4);
    std::vector<uint8_t> fatb(4 + files.size() * 8 + 8, 0);
    std::vector<uint8_t> fimg(bytesRequired + 8, 0);
    fatb[0] = 'B'; fatb[1] = 'T'; fatb[2] = 'A'; fatb[3] = 'F';
    writeLong(fatb, 4, static_cast<int>(fatb.size()));
    fimg[0] = 'G'; fimg[1] = 'M'; fimg[2] = 'I'; fimg[3] = 'F';
    writeLong(fimg, 4, static_cast<int>(fimg.size()));
    int offset = 0;
    writeLong(fatb, 8, static_cast<int>(files.size()));
    for (size_t i = 0; i < files.size(); i++) {
        const auto& file = files[i];
        int req = static_cast<int>((file.size() + 3) / 4 * 4);
        std::copy(file.begin(), file.end(), fimg.begin() + offset + 8);
        for (int k = static_cast<int>(file.size()); k < req; k++) fimg[offset + 8 + k] = 0xFF;
        writeLong(fatb, 12 + i * 8, offset);
        writeLong(fatb, 16 + i * 8, offset + static_cast<int>(file.size()));
        offset += req;
    }
    size_t fntbSize = 16;
    if (hasFilenames_)
        for (const auto& n : filenames_) fntbSize += n.size() + 1;
    std::vector<uint8_t> fntb(fntbSize, 0);
    fntb[0] = 'B'; fntb[1] = 'T'; fntb[2] = 'N'; fntb[3] = 'F';
    writeLong(fntb, 4, static_cast<int>(fntb.size()));
    if (hasFilenames_) {
        writeLong(fntb, 8, 8);
        writeLong(fntb, 12, 0x10000);
        size_t o = 16;
        for (const auto& n : filenames_) {
            fntb[o] = static_cast<uint8_t>(n.size());
            std::copy(n.begin(), n.end(), fntb.begin() + o + 1);
            o += 1 + n.size();
        }
    } else {
        writeLong(fntb, 8, 4);
        writeLong(fntb, 12, 0x10000);
    }
    size_t total = 16 + fatb.size() + fntb.size() + fimg.size();
    std::vector<uint8_t> out(total, 0);
    out[0] = 'N'; out[1] = 'A'; out[2] = 'R'; out[3] = 'C';
    writeWord(out, 4, 0xFFFE);
    writeWord(out, 6, 0x0100);
    writeLong(out, 8, static_cast<int>(total));
    writeWord(out, 12, 0x10);
    writeWord(out, 14, 3);
    std::copy(fatb.begin(), fatb.end(), out.begin() + 16);
    std::copy(fntb.begin(), fntb.end(), out.begin() + 16 + fatb.size());
    std::copy(fimg.begin(), fimg.end(), out.begin() + 16 + fatb.size() + fntb.size());
    return out;
}

}  // namespace pr::nds
