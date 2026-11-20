#!/usr/bin/env python3
"""Genera core/random/updaters_gen.inc con las listas de cambios de MoveUpdater y SpeciesBaseStatUpdater de FVX."""
import re, sys
sys.path.insert(0, "tools")
from j2cpp import convert

D = "reference/fvx/random/src/main/java/com/uprfvx/random/updaters/"

def methods(path, cls, param_java, param_cpp):
    src = open(D + path).read()
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    src = re.sub(r"//.*", "", src)
    out = []
    for m in re.finditer(r"private void (gen\dUpdates)\(List<\w+> " + param_java + r"\) \{", src):
        start = m.end()
        depth, i = 1, start
        while depth:
            depth += {"{": 1, "}": -1}.get(src[i], 0)
            i += 1
        body = src[start:i - 1]
        if "isUSUM" in body:  # solo Gen 6-7
            body = "\n        // (Aegislash: solo Gen 6-7, no portado)\n"
        body = body.replace("(" + param_java + ", ", "(")
        body = "\n".join(convert(l) for l in body.split("\n"))
        out.append(f"void {cls}::{m.group(1)}() {{{body}}}\n")
    return "\n".join(out)

code = "// Generado por tools/translate_updaters.py a partir de FVX. No editar a mano.\n\n"
code += methods("MoveUpdater.java", "MoveUpdater", "moves", "mvs")
code += "\n" + methods("SpeciesBaseStatUpdater.java", "SpeciesBaseStatUpdater", "pokes", "pokes")
code = re.sub(r"romHandler\.generationOfPokemon\(\)", "gen()", code)
code = re.sub(r"romHandler\.getPerfectAccuracy\(\)", "romHandler.getPerfectAccuracy()", code)
assert "MoveIDs" not in code and "SpeciesIDs" not in code and "Type." not in code, code[:2000]
open("core/random/updaters_gen.inc", "w").write(code)
