// Igual que tests/java/Gen4TextCheck.java, con el port en C++.
#include <cstdio>
#include <iostream>

#include "nds/narc.hpp"
#include "nds/nds_rom.hpp"
#include "text/gen4_text.hpp"
#include "util/crc32.hpp"

int main(int argc, char** argv) {
    if (argc < 3) return 1;
    pr::nds::NDSRom rom(argv[1]);
    pr::nds::NARCArchive narc(*rom.getFile(argv[2]));
    for (size_t f = 0; f < narc.files.size(); f++) {
        auto d = pr::text4::readTexts(narc.files[f]);
        std::cout << "file " << f << " n=" << d.strings.size() << " comp=" << (d.compressed ? "true" : "false") << "\n";
        for (const auto& s : d.strings) std::cout << " " << s << "\n";
        auto enc = pr::text4::makeEncryptedFile(d.strings, d.compressed);
        char buf[16];
        snprintf(buf, sizeof buf, "%08X", pr::crc32(enc.data(), enc.size()));
        std::cout << "enc " << enc.size() << " " << buf << "\n";
    }
}
