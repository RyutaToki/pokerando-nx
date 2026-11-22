// Igual que tests/java/Gen5TextCheck.java, con el port en C++.
#include <cstdio>
#include <iostream>

#include "nds/narc.hpp"
#include "nds/nds_rom.hpp"
#include "text/gen5_text.hpp"
#include "util/crc32.hpp"

int main(int argc, char** argv) {
    if (argc < 3) return 1;
    pr::nds::NDSRom rom(argv[1]);
    pr::nds::NARCArchive narc(*rom.getFile(argv[2]));
    for (size_t f = 0; f < narc.files.size(); f++) {
        std::vector<std::string> strs;
        try {
            strs = pr::text5::readTexts(narc.files[f]);
        } catch (const std::exception&) {
            std::cout << "file " << f << " error\n";
            continue;
        }
        std::cout << "file " << f << " n=" << strs.size() << "\n";
        for (const auto& s : strs) std::cout << " " << s << "\n";
        std::vector<uint8_t> enc;
        try {
            enc = pr::text5::saveEntry(narc.files[f], strs);
        } catch (const std::exception&) {
            std::cout << "enc error\n";
            continue;
        }
        char buf[16];
        snprintf(buf, sizeof buf, "%08X", pr::crc32(enc.data(), enc.size()));
        std::cout << "enc " << enc.size() << " " << buf << "\n";
    }
}
