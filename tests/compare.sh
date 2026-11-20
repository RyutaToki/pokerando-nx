#!/usr/bin/env bash
# Compara lo que leen FVX (Java) y el núcleo C++ de una ROM, y comprueba
# que abrir y guardar sin cambios da un archivo idéntico.
# Uso: tests/compare.sh roms/archivo.gba
set -euo pipefail
cd "$(dirname "$0")/.."
source env.sh
ROM="$1"
OUT="build/compare"
mkdir -p "$OUT/jc"
cmake --build build >/dev/null
CP=$(ls reference/fvx/*/build/libs/*.jar | tr '\n' ':')
javac -d "$OUT/jc" -cp "$CP" tests/java/Dump.java
java -cp "$OUT/jc:$CP" Dump "$ROM" > "$OUT/java.txt"
./build/cli/pokerando-cli dump "$ROM" > "$OUT/cpp.txt"
if diff -u "$OUT/java.txt" "$OUT/cpp.txt" > "$OUT/diff.txt"; then
    echo "OK: volcados idénticos ($(wc -l < "$OUT/cpp.txt") líneas)"
else
    echo "DIFERENCIAS: $(grep -c '^[-+][^-+]' "$OUT/diff.txt") líneas, ver $OUT/diff.txt"
    exit 1
fi
./build/cli/pokerando-cli resave "$ROM" "$OUT/resave.bin"
cmp -s "$ROM" "$OUT/resave.bin" && echo "OK: guardar sin cambios da una ROM idéntica" || { echo "FALLO: la ROM guardada difiere"; exit 1; }
