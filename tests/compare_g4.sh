#!/bin/bash
# Compara el volcado de FVX (tests/java/Dump.java) con «pokerando-cli dump» para ROMs de Gen 4
# hasta la primera sección que aún no esté portada. Uso: tests/compare_g4.sh rom.nds [rom2.nds...]
set -e
cd "$(dirname "$0")/.."
source env.sh >/dev/null 2>&1
OUT=${TMPDIR:-/tmp}/pr_compare_g4
mkdir -p "$OUT/jc"
CP=$(ls "$PWD"/reference/fvx/*/build/libs/*.jar | tr '\n' ':')
javac -d "$OUT/jc" -cp "$CP" tests/java/Dump.java
for ROM in "$@"; do
    ROM=$(realpath "$ROM")
    tag=$(basename "$ROM" | tr -c 'A-Za-z0-9\n' '_')
    if [ ! -s "$OUT/$tag.java.txt" ] || [ tests/java/Dump.java -nt "$OUT/$tag.java.txt" ]; then
        (cd "$OUT" && java -Xmx3g -cp "$OUT/jc:$CP" Dump "$ROM" "$OUT/$tag.rw.nds" > "$OUT/$tag.java.txt" 2>/dev/null) || true
    fi
    ./build/cli/pokerando-cli dump "$ROM" > "$OUT/$tag.cpp.txt" 2> "$OUT/$tag.cpp.err" || true
    n=$(grep -c "" "$OUT/$tag.cpp.txt")
    head -n "$n" "$OUT/$tag.java.txt" > "$OUT/$tag.java.head.txt"
    # SENSIBLE: las bayas por tipo salen de un HashMap<Type> de Java (hash de identidad, orden irreproducible):
    # se comparan los números ordenados
    for f in "$OUT/$tag.java.head.txt" "$OUT/$tag.cpp.txt"; do
        python3 -I tests/sort_sensible.py "$f"
    done
    d=$(diff "$OUT/$tag.java.head.txt" "$OUT/$tag.cpp.txt" | grep -c '^<' || true)
    echo "$(basename "$ROM"): $d líneas distintas de $n · $(cat "$OUT/$tag.cpp.err")"
    [ "$d" = 0 ] || diff "$OUT/$tag.java.head.txt" "$OUT/$tag.cpp.txt" | head -8
done
