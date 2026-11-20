#!/usr/bin/env python3
"""Genera las fuentes estáticas y recortadas de app/romfs/fonts a partir de las variables (licencia OFL).
Uso: make_fonts.py <InterVariable.ttf> <SpaceGrotesk[wght].ttf> <JetBrainsMono-Regular.ttf> <salida>"""
import sys
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer
from fontTools import subset

inter, grotesk, mono, out = sys.argv[1:5]
# Latín básico, Latín-1, Latín extendido A, puntuación general, flechas, ✓, ₽, ½, triángulos
UNICODES = list(range(0x20, 0x7F)) + list(range(0xA0, 0x180)) + list(range(0x2010, 0x2070)) + \
    [0x20AC, 0x20BD, 0x2190, 0x2191, 0x2192, 0x2193, 0x25B2, 0x25B6, 0x25BC, 0x25C0, 0x2713, 0x2022, 0x00B7, 0x2212]

def make(src, wght, name):
    f = TTFont(src)
    if wght is not None and "fvar" in f:
        f = instancer.instantiateVariableFont(f, {"wght": wght, **({"opsz": 14} if any(a.axisTag == "opsz" for a in f["fvar"].axes) else {})})
    opts = subset.Options()
    opts.layout_features = ["kern", "liga", "calt", "tnum"]
    opts.name_IDs = ["*"]
    opts.notdef_outline = True
    s = subset.Subsetter(opts)
    s.populate(unicodes=UNICODES)
    s.subset(f)
    f.save(f"{out}/{name}.ttf")

for w, n in [(400, "Inter-Regular"), (500, "Inter-Medium"), (600, "Inter-SemiBold"), (700, "Inter-Bold")]:
    make(inter, w, n)
make(grotesk, 700, "SpaceGrotesk-Bold")
make(mono, None, "JetBrainsMono-Regular")
