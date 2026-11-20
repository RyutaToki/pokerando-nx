// Igual que tests/java/NdsCheck.java, con el port en C++.
#include <cstdio>
#include <iostream>

#include "nds/narc.hpp"
#include "nds/nds_rom.hpp"
#include "util/crc32.hpp"

static std::string crc(const std::vector<uint8_t>& b) {
    char buf[16];
    snprintf(buf, sizeof buf, "%08X", pr::crc32(b.data(), b.size()));
    return buf;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "uso: nds-check rom.nds salida.nds ruta/del/narc\n";
        return 1;
    }
    pr::nds::NDSRom rom(argv[1]);
    std::cout << "code " << rom.getCode() << " version " << static_cast<int>(static_cast<int8_t>(rom.getVersion())) << "\n";
    auto arm9 = rom.getARM9();
    std::cout << "arm9 " << arm9.size() << " " << crc(arm9) << "\n";
    for (int n = 0;; n++) {
        auto o = rom.getOverlay(n);
        if (!o) break;
        char addr[16];
        snprintf(addr, sizeof addr, "%x", static_cast<unsigned>(rom.getOverlayAddress(n)));
        std::cout << "overlay " << n << " " << o->size() << " " << crc(*o) << " @" << addr << "\n";
    }
    rom.writeARM9(arm9);
    auto narcBytes = *rom.getFile(argv[3]);
    pr::nds::NARCArchive narc(narcBytes);
    std::cout << "narc " << narc.files.size() << " " << crc(narcBytes) << "\n";
    auto rebuilt = narc.getBytes();
    std::cout << "narc rebuilt " << rebuilt.size() << " " << crc(rebuilt) << "\n";
    rom.writeFile(argv[3], rebuilt);
    rom.saveTo(argv[2]);
    return 0;
}
