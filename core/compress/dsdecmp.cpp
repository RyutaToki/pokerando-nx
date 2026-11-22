#include "compress/dsdecmp.hpp"

#include <cstdio>
#include <stdexcept>

namespace pr::dsdecmp {

namespace {
using Bytes = std::vector<uint8_t>;

int readLength(const Bytes& data, int& offset) {
    offset++;
    int length = data.at(offset) | (data.at(offset + 1) << 8) | (data.at(offset + 2) << 16);
    offset += 3;
    if (length == 0) {
        // IOFunctions.readFullIntBigEndian
        length = static_cast<int>((static_cast<uint32_t>(data.at(offset)) << 24) | (static_cast<uint32_t>(data.at(offset + 1)) << 16) |
                                  (static_cast<uint32_t>(data.at(offset + 2)) << 8) | data.at(offset + 3));
        offset += 4;
    }
    if (length < 0) throw std::length_error("NegativeArraySizeException");
    return length;
}

Bytes decompress10LZ(const Bytes& data, int offset) {
    int length = readLength(data, offset);
    Bytes out(length, 0);
    int curr = 0;
    while (curr < length) {
        int flags = data.at(offset++);
        for (int i = 0; i < 8; i++) {
            bool flag = (flags & (0x80 >> i)) > 0;
            if (flag) {
                int b = data.at(offset++);
                int n = (b >> 4) + 3;
                int disp = ((b & 0x0F) << 8) | data.at(offset++);
                int cdest = curr;
                if (disp > curr) throw std::out_of_range("Cannot go back more than already written");
                for (int j = 0; j < n; j++) {
                    uint8_t v = out.at(cdest - disp - 1 + j);
                    out.at(curr++) = v;
                }
                if (curr > length) break;
            } else {
                int b = data.at(offset++);
                // en Java el índice ya se ha incrementado cuando salta la excepción
                int idx = curr++;
                if (idx < length) {
                    out[idx] = static_cast<uint8_t>(b);
                } else if (b == 0) {
                    break;
                }
                if (curr > length) break;
            }
        }
    }
    return out;
}

Bytes decompress11LZ(const Bytes& data, int offset) {
    int length = readLength(data, offset);
    Bytes out(length, 0);
    int curr = 0;
    while (curr < length) {
        int flags = data.at(offset++);
        for (int i = 0; i < 8 && curr < length; i++) {
            bool flag = (flags & (0x80 >> i)) > 0;
            if (flag) {
                int b1 = data.at(offset++);
                int len, disp;
                switch (b1 >> 4) {
                    case 0: {
                        len = b1 << 4;
                        int bt = data.at(offset++);
                        len |= bt >> 4;
                        len += 0x11;
                        disp = ((bt & 0x0F) << 8) | data.at(offset++);
                        break;
                    }
                    case 1: {
                        int bt = data.at(offset++);
                        int b2 = data.at(offset++);
                        int b3 = data.at(offset++);
                        len = ((b1 & 0xF) << 12) | (bt << 4) | (b2 >> 4);
                        len += 0x111;
                        disp = ((b2 & 0x0F) << 8) | b3;
                        break;
                    }
                    default:
                        len = (b1 >> 4) + 1;
                        disp = ((b1 & 0x0F) << 8) | data.at(offset++);
                        break;
                }
                if (disp > curr) throw std::out_of_range("Cannot go back more than already written");
                int cdest = curr;
                for (int j = 0; j < len && curr < length; j++) {
                    uint8_t v = out.at(cdest - disp - 1 + j);
                    out[curr++] = v;
                }
                if (curr > length) break;
            } else {
                out[curr++] = data.at(offset++);
                if (curr > length) break;
            }
        }
    }
    return out;
}
}  // namespace

Bytes decompress(const Bytes& data, int offset) {
    switch (data.at(offset)) {
        case 0x10: return decompress10LZ(data, offset);
        case 0x11: return decompress11LZ(data, offset);
        default: {
            char buf[64];
            std::snprintf(buf, sizeof buf, "no compressed data found at offset 0x%x", offset);
            throw std::invalid_argument(buf);
        }
    }
}

}  // namespace pr::dsdecmp
