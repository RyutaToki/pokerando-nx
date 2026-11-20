// Contrapartida en C++ de tests/java/RngHashCheck.java. La salida debe ser idéntica.
#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#include "util/java_hash.hpp"
#include "util/java_random.hpp"

struct Key {
    int n;
    bool operator==(const Key& o) const { return n == o.n; }
};
template <>
struct pr::JavaHashCode<Key> {
    int operator()(const Key& k) const { return 31 + k.n; }
};

int main() {
    pr::JavaRandom r(123456789LL);
    for (int i = 0; i < 2000; i++) {
        int a = r.nextInt(1000), b = r.nextInt(7), c = r.nextInt(64), d = r.nextInt(-5, 37);
        bool e = r.nextBoolean();
        double f = r.nextDouble(), g = r.nextDouble(0.8, 1.2);
        int64_t h = r.nextLong();
        uint64_t fb, gb;
        std::memcpy(&fb, &f, 8);
        std::memcpy(&gb, &g, 8);
        std::printf("%d %d %d %d %s %" PRIx64 " %" PRIx64 " %" PRId64 "\n", a, b, c, d, e ? "true" : "false", fb, gb, h);
    }
    std::vector<int> l;
    for (int i = 0; i < 50; i++) l.push_back(i);
    r.shuffle(l);
    std::string out = "[";
    for (size_t i = 0; i < l.size(); i++) out += (i ? ", " : "") + std::to_string(l[i]);
    std::printf("%s]\n", out.c_str());
    pr::JavaRandom ops(42);
    for (int round = 0; round < 200; round++) {
        pr::JavaHashSet<Key> s;
        if (round % 3 != 0) s = pr::JavaHashSet<Key>::fromCollection(std::vector<Key>{{round}, {round * 7 + 1}});
        int n = ops.nextInt(900);
        for (int i = 0; i < n; i++) {
            int op = ops.nextInt(10);
            Key k{ops.nextInt(round % 2 == 0 ? 700 : 5000)};
            if (op < 7) s.add(k);
            else if (op < 9) s.remove(k);
            else if (ops.nextInt(50) == 0) s.clear();
        }
        if (round % 5 == 0) s = pr::JavaHashSet<Key>::fromCollection(s.toVector());
        if (round % 7 == 0) s.removeIf([](const Key& k) { return k.n % 3 == 0; });
        s.forEach([](const Key& k) { std::printf("%d,", k.n); });
        std::printf("\n");
    }
}
