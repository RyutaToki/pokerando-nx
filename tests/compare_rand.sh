#!/usr/bin/env bash
# Randomiza con FVX y con el núcleo C++ usando las mismas opciones y semilla, y compara las ROMs.
# Uso: tests/compare_rand.sh <rom> <semilla> opcion=valor [opcion=valor ...]
set -euo pipefail
cd "$(dirname "$0")/.."
source env.sh
ROM="$1"; SEED="$2"; shift 2
OUT=build/compare
mkdir -p "$OUT/jc"
CP=$(ls reference/fvx/*/build/libs/*.jar | tr '\n' ':')
for j in MakeSettings Randomize; do
    [ "$OUT/jc/$j.class" -nt "tests/java/$j.java" ] || javac -d "$OUT/jc" -cp "$CP" "tests/java/$j.java"
done
cmake --build build >/dev/null
SETTINGS=$(java -cp "$OUT/jc:$CP" MakeSettings "$@")
java -cp "$OUT/jc:$CP" Randomize "$ROM" "$SETTINGS" "$SEED" "$OUT/rand_java.bin" "$OUT/rand_java.log" > /dev/null
./build/cli/pokerando-cli randomize "$ROM" "$SETTINGS" "$SEED" "$OUT/rand_cpp.bin"
if cmp -s "$OUT/rand_java.bin" "$OUT/rand_cpp.bin"; then
    echo "OK [$*] semilla $SEED: ROM idéntica a FVX"
else
    echo "DIFERENTE [$*] semilla $SEED: $(cmp -l "$OUT/rand_java.bin" "$OUT/rand_cpp.bin" | wc -l) bytes distintos"
    exit 1
fi
