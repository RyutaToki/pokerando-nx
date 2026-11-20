#include "compress/lz.hpp"

#include <algorithm>
#include <stdexcept>

namespace pr::lz {

namespace {

size_t readHeaderLength(const std::vector<uint8_t>& d, size_t& off) {
    off++;
    size_t length = d.at(off) | (d.at(off + 1) << 8) | (d.at(off + 2) << 16);
    off += 3;
    if (length == 0) {
        length = (static_cast<size_t>(d.at(off)) << 24) | (d.at(off + 1) << 16) | (d.at(off + 2) << 8) | d.at(off + 3);
        off += 4;
    }
    return length;
}

std::vector<uint8_t> decompress10(const std::vector<uint8_t>& d, size_t off) {
    size_t length = readHeaderLength(d, off);
    std::vector<uint8_t> out(length);
    size_t cur = 0;
    while (cur < out.size()) {
        int flags = d.at(off++);
        for (int i = 0; i < 8; i++) {
            if (flags & (0x80 >> i)) {
                int b = d.at(off++);
                int n = (b >> 4) + 3;
                size_t disp = ((b & 0x0F) << 8) | d.at(off++);
                if (disp > cur) throw std::out_of_range("Cannot go back more than already written");
                size_t cdest = cur;
                for (int j = 0; j < n; j++) out.at(cur++) = out.at(cdest - disp - 1 + j);  // Java lanzaría igual
                if (cur > out.size()) break;
            } else {
                int b = d.at(off++);
                if (cur >= out.size()) {  // Java: ArrayIndexOutOfBounds capturada
                    cur++;
                    break;
                }
                out[cur++] = static_cast<uint8_t>(b);
            }
        }
    }
    return out;
}

std::vector<uint8_t> decompress11(const std::vector<uint8_t>& d, size_t off) {
    size_t length = readHeaderLength(d, off);
    std::vector<uint8_t> out(length);
    size_t cur = 0;
    while (cur < out.size()) {
        int flags = d.at(off++);
        for (int i = 0; i < 8 && cur < out.size(); i++) {
            if (flags & (0x80 >> i)) {
                int b1 = d.at(off++);
                size_t len, disp;
                switch (b1 >> 4) {
                    case 0: {
                        len = b1 << 4;
                        int bt = d.at(off++);
                        len |= bt >> 4;
                        len += 0x11;
                        disp = ((bt & 0x0F) << 8) | d.at(off++);
                        break;
                    }
                    case 1: {
                        int bt = d.at(off++), b2 = d.at(off++), b3 = d.at(off++);
                        len = ((b1 & 0xF) << 12) | (bt << 4) | (b2 >> 4);
                        len += 0x111;
                        disp = ((b2 & 0x0F) << 8) | b3;
                        break;
                    }
                    default:
                        len = (b1 >> 4) + 1;
                        disp = ((b1 & 0x0F) << 8) | d.at(off++);
                }
                if (disp > cur) throw std::out_of_range("Cannot go back more than already written");
                size_t cdest = cur;
                for (size_t j = 0; j < len && cur < out.size(); j++) out[cur++] = out[cdest - disp - 1 + j];
            } else {
                out[cur++] = d.at(off++);
            }
        }
    }
    return out;
}

// getOccurenceInfo de DSCmp
std::pair<int, int> occurrence(const std::vector<uint8_t>& d, int current, int pending, int finished) {
    if (pending == 0) return {0, 0};
    int disp = 0, length = 0;
    for (int i = 0; i < finished - 1; i++) {
        int po = current - finished + i;
        int pl = 0;
        for (int j = 0; j < pending; j++) {
            if (d[po + j] != d[current + j]) break;
            pl++;
        }
        if (pl > length) {
            length = pl;
            disp = finished - i;
            if (length == pending) break;
        }
    }
    return {length, disp};
}

void writeHeader(std::vector<uint8_t>& out, int type, size_t len) {
    out.push_back(static_cast<uint8_t>(type));
    if (len > 0xFFFFFF) {
        out.insert(out.end(), {0, 0, 0});
        for (int i = 0; i < 4; i++) out.push_back(static_cast<uint8_t>((len >> (8 * i)) & 0xFF));
    } else {
        for (int i = 0; i < 3; i++) out.push_back(static_cast<uint8_t>((len >> (8 * i)) & 0xFF));
    }
}

}  // namespace

std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t offset) {
    switch (data.at(offset)) {
        case 0x10: return decompress10(data, offset);
        case 0x11: return decompress11(data, offset);
        default: throw std::invalid_argument("no compressed data found at offset");
    }
}

std::vector<uint8_t> compressLZ10(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    writeHeader(out, 0x10, in.size());
    int cur = 0, n = static_cast<int>(in.size());
    uint8_t buf[17] = {0};
    int lenBuf = 1, blocks = 0;
    while (cur < n) {
        if (blocks == 8) {
            out.insert(out.end(), buf, buf + lenBuf);
            buf[0] = 0;
            lenBuf = 1;
            blocks = 0;
        }
        auto [len, disp] = occurrence(in, cur, std::min(n - cur, 0x12), std::min(cur, 0x1000));
        if (len < 3) {
            buf[lenBuf++] = in[cur++];
        } else {
            buf[0] |= static_cast<uint8_t>(1 << (7 - blocks));
            buf[lenBuf] = static_cast<uint8_t>(((len - 3) & 0xF) << 4);
            buf[lenBuf++] |= static_cast<uint8_t>(((disp - 1) >> 8) & 0xF);
            buf[lenBuf++] = static_cast<uint8_t>((disp - 1) & 0xFF);
            cur += len;
        }
        blocks++;
    }
    if (blocks > 0) out.insert(out.end(), buf, buf + lenBuf);
    return out;
}

std::vector<uint8_t> compressLZ11(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    writeHeader(out, 0x11, in.size());
    int cur = 0, n = static_cast<int>(in.size());
    uint8_t buf[33] = {0};
    int lenBuf = 1, blocks = 0;
    while (cur < n) {
        if (blocks == 8) {
            out.insert(out.end(), buf, buf + lenBuf);
            buf[0] = 0;
            lenBuf = 1;
            blocks = 0;
        }
        auto [len, disp] = occurrence(in, cur, std::min(n - cur, 0x10110), std::min(cur, 0x1000));
        if (len < 3) {
            buf[lenBuf++] = in[cur++];
        } else {
            buf[0] |= static_cast<uint8_t>(1 << (7 - blocks));
            if (len > 0x110) {
                buf[lenBuf] = 0x10;
                buf[lenBuf++] |= static_cast<uint8_t>(((len - 0x111) >> 12) & 0xF);
                buf[lenBuf++] = static_cast<uint8_t>(((len - 0x111) >> 4) & 0xFF);
                buf[lenBuf] = static_cast<uint8_t>(((len - 0x111) & 0xF) << 4);
            } else if (len > 0x10) {
                buf[lenBuf] = 0x00;
                buf[lenBuf++] |= static_cast<uint8_t>(((len - 0x111) >> 4) & 0xF);
                buf[lenBuf] = static_cast<uint8_t>(((len - 0x111) & 0xF) << 4);
            } else {
                buf[lenBuf] = static_cast<uint8_t>(((len - 1) & 0xF) << 4);
            }
            buf[lenBuf++] |= static_cast<uint8_t>(((disp - 1) >> 8) & 0xF);
            buf[lenBuf++] = static_cast<uint8_t>((disp - 1) & 0xFF);
            cur += len;
        }
        blocks++;
    }
    if (blocks > 0) out.insert(out.end(), buf, buf + lenBuf);
    return out;
}

}  // namespace pr::lz
