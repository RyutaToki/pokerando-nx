// Réplica exacta de java.util.Random (JDK 25) tal como lo usa FVX (RandomSource.RandomWithCounter).
// Con la misma semilla produce exactamente la misma secuencia que Java.
#pragma once
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pr {

class JavaRandom {
public:
    JavaRandom() = default;
    explicit JavaRandom(int64_t seed) { setSeed(seed); }

    void setSeed(int64_t seed) {
        seed_ = (static_cast<uint64_t>(seed) ^ kMultiplier) & kMask;
        haveNextNextGaussian_ = false;
        calls_ = 0;
    }
    int calls() const { return calls_; }

    int nextInt() { calls_++; return next(32); }
    int nextInt(int bound) {
        calls_++;
        if (bound <= 0) throw std::invalid_argument("bound must be positive");
        int r = next(31);
        int m = bound - 1;
        if ((bound & m) == 0) {
            r = static_cast<int>((static_cast<int64_t>(bound) * r) >> 31);
        } else {
            for (int u = r; static_cast<int>(static_cast<uint32_t>(u - (r = u % bound)) + static_cast<uint32_t>(m)) < 0; u = next(31)) {
            }
        }
        return r;
    }
    // RandomGenerator.nextInt(origin, bound) -> RandomSupport.boundedNextInt (llama a nextInt())
    int nextInt(int origin, int bound) {
        if (origin >= bound) throw std::invalid_argument("bound must be greater than origin");
        int r = nextInt();
        const uint32_t n = static_cast<uint32_t>(bound) - static_cast<uint32_t>(origin);
        const uint32_t m = n - 1;
        if ((n & m) == 0) {
            r = static_cast<int>((static_cast<uint32_t>(r) & m) + static_cast<uint32_t>(origin));
        } else if (static_cast<int32_t>(n) > 0) {
            for (int u = static_cast<int>(static_cast<uint32_t>(r) >> 1);
                 static_cast<int32_t>(static_cast<uint32_t>(u) + m - static_cast<uint32_t>(r = u % static_cast<int>(n))) < 0;
                 u = static_cast<int>(static_cast<uint32_t>(nextInt()) >> 1)) {
            }
            r += origin;
        } else {
            while (r < origin || r >= bound) r = nextInt();
        }
        return r;
    }
    int64_t nextLong() {
        calls_++;
        int64_t hi = next(32);
        int64_t lo = next(32);
        return static_cast<int64_t>(static_cast<uint64_t>(hi) << 32) + lo;
    }
    bool nextBoolean() { calls_++; return next(1) != 0; }
    float nextFloat() { calls_++; return next(24) * (1.0f / (1 << 24)); }
    double nextDouble() {
        calls_++;
        return static_cast<double>((static_cast<int64_t>(next(26)) << 27) + next(27)) * 0x1.0p-53;
    }
    // RandomSupport.boundedNextDouble (llama a nextDouble())
    double nextDouble(double origin, double bound) {
        if (!(origin < bound)) throw std::invalid_argument("bound must be greater than origin");
        double r = nextDouble();
        r = r * (bound - origin) + origin;
        if (r >= bound) r = std::nextafter(bound, -INFINITY);
        return r;
    }
    double nextGaussian() {
        calls_++;
        if (haveNextNextGaussian_) {
            haveNextNextGaussian_ = false;
            return nextNextGaussian_;
        }
        double v1, v2, s;
        do {
            // Random.nextGaussian llama al nextDouble() sobrescrito, que también cuenta
            v1 = 2 * nextDouble() - 1;
            v2 = 2 * nextDouble() - 1;
            s = v1 * v1 + v2 * v2;
        } while (s >= 1 || s == 0);
        double multiplier = std::sqrt(-2 * std::log(s) / s);  // StrictMath en Java
        nextNextGaussian_ = v2 * multiplier;
        haveNextNextGaussian_ = true;
        return v1 * multiplier;
    }

    // Collections.shuffle(list, rnd)
    template <class T>
    void shuffle(std::vector<T>& list) {
        for (size_t i = list.size(); i > 1; i--) std::swap(list[i - 1], list[nextInt(static_cast<int>(i))]);
    }

private:
    static constexpr uint64_t kMultiplier = 0x5DEECE66DULL, kAddend = 0xBULL, kMask = (1ULL << 48) - 1;

    int next(int bits) {
        seed_ = (seed_ * kMultiplier + kAddend) & kMask;
        return static_cast<int>(static_cast<int64_t>(seed_) >> (48 - bits));
    }

    uint64_t seed_ = 0;
    double nextNextGaussian_ = 0;
    bool haveNextNextGaussian_ = false;
    int calls_ = 0;
};

}  // namespace pr
