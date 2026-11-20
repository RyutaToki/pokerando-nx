#!/usr/bin/env python3
"""Regenera core/romio/gen4/gen4_stubs.cpp: una versión «pendiente» de cada función declarada en
gen4_rom_handler.hpp que todavía no está definida en ningún otro .cpp de core/romio/gen4."""
import glob
import os
import re

D = os.path.join(os.path.dirname(__file__), "..", "core", "romio", "gen4")
h = open(os.path.join(D, "gen4_rom_handler.hpp")).read()
cpp = "".join(open(f).read() for f in glob.glob(os.path.join(D, "*.cpp")) if not f.endswith("gen4_stubs.cpp"))
decls = re.findall(r'^\s+((?:static\s+)?[\w:<>,\s\*&]+?\s+(\w+)\(([^;{]*)\)\s*(const)?\s*(override)?;)', h, re.M)
out = []
for full, name, args, const, ov in decls:
    if f"Gen4RomHandler::{name}(" in cpp:
        continue
    ret = full.strip()
    ret = ret[:ret.index(name + "(")].strip()
    if ret.startswith("static"):
        continue
    args = re.sub(r"\s*=\s*[^,)]+", "", args)
    sig = f"{ret} Gen4RomHandler::{name}({args}){' const' if const else ''}"
    out.append(sig + ' { throw std::logic_error("Gen 4: pendiente: ' + name + '"); }')
src = ("// Funciones de Gen4RomHandler todavía sin portar: lanzan una excepción si se usan.\n"
       "// Generado por tools/gen4_stubs.py; se regenera al portar cada función.\n"
       '#pragma GCC diagnostic ignored "-Wunused-parameter"\n#include <stdexcept>\n\n'
       '#include "romio/gen4/gen4_rom_handler.hpp"\n\nnamespace pr::gen4 {\n\n' + "\n".join(out) + "\n\n}  // namespace pr::gen4\n")
open(os.path.join(D, "gen4_stubs.cpp"), "w").write(src)
print(f"{len(out)} pendientes")
