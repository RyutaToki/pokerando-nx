#!/usr/bin/env python3
"""Extrae de Settings.java el mapa accesor -> campo (getX/isX/setX) a tools/settings_accessors.json."""
import json
import re
import sys

t = open(sys.argv[1], encoding="utf-8").read()
acc = {}
for name, body in re.findall(r"public (?:void|Settings) (set\w+)\([^)]*\)\s*\{\s*this\.(\w+) = \w+;", t):
    acc[name] = body
for name, body in re.findall(r"public \S+ ((?:get|is)\w+)\(\)\s*\{\s*return (?:this\.)?(\w+);\s*\}", t):
    acc[name] = body
acc["setUpdateMovesToGeneration"] = "updateMovesToGeneration"
json.dump(acc, open(sys.argv[2], "w"), indent=1, sort_keys=True)
print(len(acc), "accesores")
