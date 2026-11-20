import java.util.*;

// Genera una secuencia de referencia de Random y del orden de HashSet/HashMap
// para comparar con tests/cpp/rng_hash_check.cpp.
public class RngHashCheck {
    static final class Key {  // hashCode como Species: 31 + número
        final int n;
        Key(int n) { this.n = n; }
        public int hashCode() { return 31 + n; }
        public boolean equals(Object o) { return o instanceof Key k && k.n == n; }
    }

    public static void main(String[] a) {
        StringBuilder sb = new StringBuilder();
        Random r = new Random(123456789L);
        for (int i = 0; i < 2000; i++) {
            sb.append(r.nextInt(1000)).append(' ').append(r.nextInt(7)).append(' ').append(r.nextInt(64)).append(' ');
            sb.append(r.nextInt(-5, 37)).append(' ').append(r.nextBoolean()).append(' ');
            sb.append(Long.toHexString(Double.doubleToRawLongBits(r.nextDouble()))).append(" ").append(Long.toHexString(Double.doubleToRawLongBits(r.nextDouble(0.8, 1.2)))).append(" ");
            sb.append(r.nextLong()).append('\n');
        }
        List<Integer> l = new ArrayList<>();
        for (int i = 0; i < 50; i++) l.add(i);
        Collections.shuffle(l, r);
        sb.append(l).append('\n');
        // HashSet con operaciones aleatorias
        Random ops = new Random(42);
        for (int round = 0; round < 200; round++) {
            Set<Key> s = round % 3 == 0 ? new HashSet<>() : new HashSet<>(List.of(new Key(round), new Key(round * 7 + 1)));
            int n = ops.nextInt(900);
            for (int i = 0; i < n; i++) {
                int op = ops.nextInt(10);
                Key k = new Key(ops.nextInt(round % 2 == 0 ? 700 : 5000));
                if (op < 7) s.add(k);
                else if (op < 9) s.remove(k);
                else if (ops.nextInt(50) == 0) s.clear();
            }
            if (round % 5 == 0) s = new HashSet<>(s);
            if (round % 7 == 0) s.removeIf(k -> k.n % 3 == 0);
            for (Key k : s) sb.append(k.n).append(',');
            sb.append('\n');
        }
        System.out.print(sb);
    }
}
