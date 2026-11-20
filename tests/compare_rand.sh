#!/usr/bin/env bash
# Randomiza con FVX y con el núcleo C++ usando las mismas opciones y semilla, y compara las ROMs.
# Uso: tests/compare_rand.sh <rom> <semilla> opcion=valor [opcion=valor ...]
set -euo pipefail
cd "$(dirname "$0")/.."
source env.sh
ROM="$1"; SEED="$2"; shift 2
OUT="${OUT_DIR:-build/compare}"
mkdir -p "$OUT/jc"
CP=$(ls reference/fvx/*/build/libs/*.jar | tr '\n' ':')
for j in MakeSettings Randomize; do
    [ "$OUT/jc/$j.class" -nt "tests/java/$j.java" ] || javac -d "$OUT/jc" -cp "$CP" "tests/java/$j.java"
done
# CLI=<ejecutable> usa una copia fija del programa (para pruebas largas mientras se sigue programando)
CLI="${CLI:-./build/cli/pokerando-cli}"
[ "$CLI" = ./build/cli/pokerando-cli ] && cmake --build build >/dev/null
SETTINGS=$(java -cp "$OUT/jc:$CP" MakeSettings "$@")
# FVX lee las listas de nombres de "data/..." relativo al directorio de trabajo
ABS=$(pwd)
ROMABS=$(realpath "$ROM")
rm -f "$OUT/rand_java.bin" "$OUT/rand_cpp.bin"
JERR=0; CERR=0
(cd reference/fvx/random/src/main/resources && java -cp "$ABS/$OUT/jc:$(echo $CP | sed "s#reference#$ABS/reference#g")" Randomize "$ROMABS" "$SETTINGS" "$SEED" "$ABS/$OUT/rand_java.bin" "$ABS/$OUT/rand_java.log" > /dev/null 2> "$ABS/$OUT/java_err.txt") || JERR=1
"$CLI" randomize "$ROM" "$SETTINGS" "$SEED" "$OUT/rand_cpp.bin" > /dev/null 2> "$OUT/cpp_err.txt" || CERR=1
if [ $JERR = 1 ] || [ $CERR = 1 ]; then
    J=$(grep -m1 -E "Exception|Error" "$OUT/java_err.txt" | cut -c1-160 || true)
    C=$(tail -n1 "$OUT/cpp_err.txt" | cut -c1-160)
    if [ $JERR = 1 ] && [ $CERR = 1 ]; then echo "AMBOS FALLAN [$*] semilla $SEED: FVX: $J | C++: $C"; exit 2; fi
    echo "SOLO FALLA $([ $JERR = 1 ] && echo FVX || echo C++) [$*] semilla $SEED: FVX: $J | C++: $C"; exit 1
fi
if cmp -s "$OUT/rand_java.bin" "$OUT/rand_cpp.bin"; then
    echo "OK [$*] semilla $SEED: ROM idéntica a FVX"
else
    echo "DIFERENTE [$*] semilla $SEED: $(cmp -l "$OUT/rand_java.bin" "$OUT/rand_cpp.bin" | wc -l) bytes distintos"
    exit 1
fi
