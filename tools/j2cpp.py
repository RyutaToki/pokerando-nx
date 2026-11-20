#!/usr/bin/env python3
"""Traducción mecánica (aproximada) de fragmentos Java de FVX a C++.

El resultado SIEMPRE se revisa a mano; solo ahorra el trabajo repetitivo.
Uso: j2cpp.py Archivo.java desde hasta   (líneas, ambas incluidas)
"""
import re
import sys

ID_NAMESPACES = {"SpeciesIDs": "species", "MoveIDs": "moves", "ItemIDs": "items", "AbilityIDs": "abilities"}
CONST_CLASSES = ["Gen3Constants", "Gen4Constants", "Gen5Constants", "GlobalConstants", "GBConstants"]
ROM_ENTRY = {"getIntValue": "getInt", "getStringValue": "getString", "getArrayValue": "getArray",
             "putIntValue": "putInt", "putStringValue": "putString", "putArrayValue": "putArray"}


def convert(line):
    for java, ns in ID_NAMESPACES.items():
        line = re.sub(r"\b" + java + r"\.(\w+)\.(\w+)", ns + r"::\1::\2", line)
        line = re.sub(r"\b" + java + r"\.", ns + "::", line)
    for c in CONST_CLASSES:
        line = re.sub(r"\b" + c + r"\.", "", line)
    # Enum.VALOR -> Enum::VALOR
    line = re.sub(r"\b([A-Z][A-Za-z0-9]+)\.([A-Z][A-Z0-9_]+)\b", r"\1::\2", line)
    for j, c in ROM_ENTRY.items():
        line = re.sub(r"\bromEntry\." + j + r"\(", "romEntry_->" + c + "(", line)
    line = re.sub(r"\bromEntry\.", "romEntry_->", line)
    line = re.sub(r"\brom\[", "rom_[", line)
    line = re.sub(r"\(byte\)\s*", "(uint8_t) ", line)
    line = re.sub(r"\.length\b(?!\()", ".size()", line)
    line = re.sub(r"\bfinal\s+", "", line)
    line = re.sub(r"\bString\b", "std::string", line)
    line = re.sub(r"\bboolean\b", "bool", line)
    line = re.sub(r"\bnull\b", "nullptr", line)
    line = re.sub(r"^(\s*)@Override\s*$", "", line)
    line = re.sub(r"\bpublic\s+|\bprivate\s+|\bprotected\s+", "", line)
    line = re.sub(r"(\w+)\.equals\(([^()]*)\)", r"(\1 == \2)", line)
    return line


def main(path, start, end):
    lines = open(path, encoding="utf-8").read().split("\n")[int(start) - 1:int(end)]
    for l in lines:
        print(convert(l))


if __name__ == "__main__":
    main(*sys.argv[1:4])
