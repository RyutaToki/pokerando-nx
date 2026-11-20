#include "nds/blz.hpp"

#include <stdexcept>

namespace pr::nds {

namespace {
constexpr int BLZ_SHIFT = 1;
constexpr int BLZ_MASK = 0x80;
constexpr int BLZ_THRESHOLD = 2;
constexpr int BLZ_N = 0x1002;
constexpr int BLZ_F = 0x12;
constexpr int RAW_MAXIM = 0x00FFFFFF;

std::vector<int> prepareData(const std::vector<uint8_t>& data) {
    std::vector<int> fb(data.size() + 3, 0);
    for (size_t i = 0; i < data.size(); i++) fb[i] = data[i];
    return fb;
}

int readUnsigned(const std::vector<int>& b, int o) { return b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | ((b[o + 3] & 0x7F) << 24); }

void writeUnsigned(std::vector<int>& b, int o, int v) {
    b[o] = v & 0xFF;
    b[o + 1] = (v >> 8) & 0xFF;
    b[o + 2] = (v >> 16) & 0xFF;
    b[o + 3] = (v >> 24) & 0x7F;
}

void invert(std::vector<int>& buffer, int offset, int length) {
    int bottom = offset + length - 1;
    while (offset < bottom) {
        int ch = buffer[offset];
        buffer[offset++] = buffer[bottom];
        buffer[bottom--] = ch;
    }
}

struct SearchPair {
    int l, p;
};

SearchPair search(int p, const std::vector<int>& raw_buffer, int raw, int raw_end) {
    int l = BLZ_THRESHOLD;
    int max = (raw >= BLZ_N) ? BLZ_N : raw;
    for (int pos = 3; pos <= max; pos++) {
        int len;
        for (len = 0; len < BLZ_F; len++) {
            if (raw + len == raw_end) break;
            if (len >= pos) break;
            if (raw_buffer[raw + len] != raw_buffer[raw + len - pos]) break;
        }
        if (len > l) {
            p = pos;
            if ((l = len) == BLZ_F) break;
        }
    }
    return {l, p};
}
}  // namespace

std::vector<uint8_t> blzDecode(const std::vector<uint8_t>& data) {
    std::vector<int> pak_buffer = prepareData(data);
    int pak_len = static_cast<int>(pak_buffer.size()) - 3;
    int inc_len = readUnsigned(pak_buffer, pak_len - 4);
    int enc_len, dec_len, raw_len;
    if (inc_len < 1) {
        enc_len = 0;
        dec_len = pak_len;
        pak_len = 0;
        raw_len = dec_len;
    } else {
        if (pak_len < 8) throw std::runtime_error("BLZ: cabecera incorrecta");
        int hdr_len = pak_buffer[pak_len - 5];
        if (hdr_len < 8 || hdr_len > 0xB) throw std::runtime_error("BLZ: longitud de cabecera incorrecta");
        if (pak_len <= hdr_len) throw std::runtime_error("BLZ: longitud incorrecta");
        enc_len = readUnsigned(pak_buffer, pak_len - 8) & 0x00FFFFFF;
        dec_len = pak_len - enc_len;
        pak_len = enc_len - hdr_len;
        raw_len = dec_len + enc_len + inc_len;
        if (raw_len > RAW_MAXIM) throw std::runtime_error("BLZ: longitud descomprimida incorrecta");
    }
    std::vector<int> raw_buffer(raw_len, 0);
    int pak = 0, raw = 0;
    int pak_end = dec_len + pak_len;
    int raw_end = raw_len;
    for (int len = 0; len < dec_len; len++) raw_buffer[raw++] = pak_buffer[pak++];
    invert(pak_buffer, dec_len, pak_len);
    int mask = 0, flags = 0;
    while (raw < raw_end) {
        if ((mask = (static_cast<unsigned>(mask) >> BLZ_SHIFT)) == 0) {
            if (pak == pak_end) break;
            flags = pak_buffer[pak++];
            mask = BLZ_MASK;
        }
        if ((flags & mask) == 0) {
            if (pak == pak_end) break;
            raw_buffer[raw++] = pak_buffer[pak++];
        } else {
            if ((pak + 1) >= pak_end) break;
            int pos = pak_buffer[pak++] << 8;
            pos |= pak_buffer[pak++];
            int len = (static_cast<unsigned>(pos) >> 12) + BLZ_THRESHOLD + 1;
            if (raw + len > raw_end) len = raw_end - raw;
            pos = (pos & 0xFFF) + 3;
            while ((len--) > 0) {
                int c = raw_buffer[raw - pos];
                raw_buffer[raw++] = c;
            }
        }
    }
    invert(raw_buffer, dec_len, raw_len - dec_len);
    std::vector<uint8_t> out(raw);
    for (int i = 0; i < raw; i++) out[i] = static_cast<uint8_t>(raw_buffer[i]);
    return out;
}

std::vector<uint8_t> blzEncode(const std::vector<uint8_t>& data, bool arm9, bool best) {
    std::vector<int> raw_buffer = prepareData(data);
    int raw_len = static_cast<int>(raw_buffer.size()) - 3;
    int pak_tmp = 0, raw_tmp = raw_len;
    int pak_len = raw_len + ((raw_len + 7) / 8) + 11;
    std::vector<int> pak_buffer(pak_len, 0);
    int raw_new = raw_len;
    if (arm9) raw_new -= 0x4000;
    invert(raw_buffer, 0, raw_len);
    int pak = 0, raw = 0, raw_end = raw_new, mask = 0, flg = 0;
    int pos_best = 0, pos_next = 0, pos_post = 0;
    while (raw < raw_end) {
        if ((mask = (static_cast<unsigned>(mask) >> BLZ_SHIFT)) == 0) {
            pak_buffer[(flg = pak++)] = 0;
            mask = BLZ_MASK;
        }
        SearchPair s1 = search(pos_best, raw_buffer, raw, raw_end);
        int len_best = s1.l;
        pos_best = s1.p;
        if (best) {
            if (len_best > BLZ_THRESHOLD) {
                if (raw + len_best < raw_end) {
                    raw += len_best;
                    SearchPair s2 = search(pos_next, raw_buffer, raw, raw_end);
                    int len_next = s2.l;
                    pos_next = s2.p;
                    raw -= (len_best - 1);
                    SearchPair s3 = search(pos_post, raw_buffer, raw, raw_end);
                    int len_post = s3.l;
                    pos_post = s3.p;
                    raw--;
                    if (len_next <= BLZ_THRESHOLD) len_next = 1;
                    if (len_post <= BLZ_THRESHOLD) len_post = 1;
                    if ((len_best + len_next) <= (1 + len_post)) len_best = 1;
                }
            }
        }
        pak_buffer[flg] = (pak_buffer[flg] << 1);
        if (len_best > BLZ_THRESHOLD) {
            raw += len_best;
            pak_buffer[flg] |= 1;
            pak_buffer[pak++] = ((len_best - (BLZ_THRESHOLD + 1)) << 4) | (static_cast<unsigned>(pos_best - 3) >> 8);
            pak_buffer[pak++] = (pos_best - 3) & 0xFF;
        } else {
            pak_buffer[pak++] = raw_buffer[raw++];
        }
        if (pak + raw_len - raw < pak_tmp + raw_tmp) {
            pak_tmp = pak;
            raw_tmp = raw_len - raw;
        }
    }
    while ((mask > 0) && (mask != 1)) {
        mask = static_cast<unsigned>(mask) >> BLZ_SHIFT;
        pak_buffer[flg] = pak_buffer[flg] << 1;
    }
    pak_len = pak;
    invert(raw_buffer, 0, raw_len);
    invert(pak_buffer, 0, pak_len);
    std::vector<int> outBuf;
    if (pak_tmp == 0 || (raw_len + 4 < ((pak_tmp + raw_tmp + 3) & static_cast<int>(0xFFFFFFFC)) + 8)) {
        // sin compresión (en FVX este caso se queda en un bucle infinito; aquí se copia tal cual)
        outBuf.assign(raw_len + 8, 0);
        pak = 0;
        for (raw = 0; raw < raw_len; raw++) outBuf[pak++] = raw_buffer[raw];
        while ((pak & 3) > 0) outBuf[pak++] = 0;
        for (int k = 0; k < 4; k++) outBuf[pak++] = 0;
    } else {
        outBuf.assign(raw_tmp + pak_tmp + 11, 0);
        for (int len = 0; len < raw_tmp; len++) outBuf[len] = raw_buffer[len];
        for (int len = 0; len < pak_tmp; len++) outBuf[raw_tmp + len] = pak_buffer[len + pak_len - pak_tmp];
        pak = raw_tmp + pak_tmp;
        int enc_len = pak_tmp;
        int hdr_len = 8;
        int inc_len = raw_len - pak_tmp - raw_tmp;
        while ((pak & 3) > 0) {
            outBuf[pak++] = 0xFF;
            hdr_len++;
        }
        writeUnsigned(outBuf, pak, enc_len + hdr_len);
        pak += 3;
        outBuf[pak++] = hdr_len;
        writeUnsigned(outBuf, pak, inc_len - hdr_len);
        pak += 4;
    }
    std::vector<uint8_t> out(pak);
    for (int i = 0; i < pak; i++) out[i] = static_cast<uint8_t>(outBuf[i]);
    return out;
}

}  // namespace pr::nds
