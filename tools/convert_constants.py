#!/usr/bin/env python3
"""Convierte las declaraciones simples de una clase *Constants.java de FVX a C++.

Soporta: int (varias por declaración), byte, String, List<Integer> = Arrays.asList(...),
Set<Integer> = new HashSet<>(Arrays.asList(...)), byte[]/int[] literales.
Lo demás (mapas, funciones, tablas construidas) se lista al final como pendiente
y se porta a mano.

Uso: convert_constants.py Entrada.java salida.hpp namespace
"""
import re
import sys

sys.path.insert(0, __import__("os").path.dirname(__file__))
from j2cpp import convert  # noqa: E402

DECL_RE = re.compile(r"(?:public|private|protected)?\s*static\s+final\s+([\w<>\[\], ]+?)\s+(\w+)\s*=", re.S)


def statements(text):
    """Devuelve las declaraciones 'public static final ... ;' completas (pueden ocupar varias líneas)."""
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    out = []
    for m in DECL_RE.finditer(text):
        depth, i = 0, m.end()
        in_str = False
        while i < len(text):
            ch = text[i]
            if ch == '"' and text[i - 1] != "\\":
                in_str = not in_str
            elif not in_str:
                if ch in "({[":
                    depth += 1
                elif ch in ")}]":
                    depth -= 1
                elif ch == ";" and depth == 0:
                    break
            i += 1
        out.append((m.group(1).strip(), m.group(2), text[m.start():i]))
    return out


def split_top(s):
    parts, depth, cur, in_str = [], 0, "", False
    for i, ch in enumerate(s):
        if ch == '"' and (i == 0 or s[i - 1] != "\\"):
            in_str = not in_str
        if not in_str:
            if ch in "({[":
                depth += 1
            elif ch in ")}]":
                depth -= 1
            elif ch == "," and depth == 0:
                parts.append(cur)
                cur = ""
                continue
        cur += ch
    if cur.strip():
        parts.append(cur)
    return parts


def main(src, dst, ns):
    text = open(src, encoding="utf-8").read()
    lines, todo = [], []
    for jtype, name, stmt in statements(text):
        body = stmt[stmt.index("=", stmt.index(name)) + 1:].strip()
        flat = " ".join(convert(body).split())
        if jtype in ("int", "byte", "double", "boolean"):
            ctype = {"int": "int", "byte": "uint8_t", "double": "double", "boolean": "bool"}[jtype]
            # varias declaraciones: "a = 1, b = 2"
            decl = name + " = " + " ".join(convert(body).split())
            for part in split_top(decl):
                n, e = part.split("=", 1)
                e = e.strip()
                if jtype == "byte":
                    e = re.sub(r"^\(uint8_t\)\s*", "", e)
                lines.append("inline constexpr %s %s = %s;" % (ctype, n.strip(), e))
        elif jtype == "String":
            lines.append("inline constexpr std::string_view %s = %s;" % (name, flat))
        elif jtype in ("List<Integer>", "Set<Integer>"):
            m = re.match(r"^(?:Collections\.unmodifiable\w+\()?(?:new HashSet<>\()?Arrays\.asList\((.*)\)\)*$", flat)
            if m:
                ctype = "std::vector<int>" if jtype.startswith("List") else "std::set<int>"
                lines.append("inline const %s %s = {%s};" % (ctype, name, m.group(1)))
            else:
                todo.append("%s %s" % (jtype, name))
        elif jtype == "List<String>":
            m = (re.match(r"^Collections\.unmodifiableList\((?:Arrays\.asList|List\.of)\((.*)\)\)$", flat)
                 or re.match(r"^(?:Arrays\.asList|List\.of)\((.*)\)$", flat))
            if m:
                lines.append("inline const std::vector<std::string_view> %s = {%s};" % (name, m.group(1)))
            else:
                todo.append("%s %s" % (jtype, name))
        elif jtype in ("byte[]", "int[]"):
            ctype = "uint8_t" if jtype == "byte[]" else "int"
            for part in split_top(name + " = " + flat):
                n, e = part.split("=", 1)
                e = e.strip()
                m = re.match(r"^new \w+\[\]\s*\{(.*)\}$", e) or re.match(r"^\{(.*)\}$", e)
                if m:
                    vals = re.sub(r"\(uint8_t\)\s*", "", m.group(1))
                    lines.append("inline const std::vector<%s> %s = {%s};" % (ctype, n.strip(), vals))
                else:
                    todo.append("%s %s" % (jtype, n.strip()))
        else:
            todo.append("%s %s" % (jtype, name))
    with open(dst, "w", encoding="utf-8") as f:
        f.write("// Generado por tools/convert_constants.py desde %s (FVX, GPLv3). No editar.\n" % src.split("/")[-1])
        f.write("#pragma once\n#include <cstdint>\n#include <set>\n#include <string_view>\n#include <vector>\n\n")
        f.write('#include "constants/ability_ids.hpp"\n#include "constants/item_ids.hpp"\n')
        f.write('#include "constants/move_ids.hpp"\n#include "constants/species_ids.hpp"\n\n')
        f.write("namespace pr::%s {\n\n" % ns)
        f.write("\n".join(lines))
        f.write("\n\n// Pendiente de portar a mano:\n")
        for t in todo:
            f.write("//   %s\n" % t)
        f.write("\n}  // namespace pr::%s\n" % ns)
    print("%d convertidas, %d pendientes" % (len(lines), len(todo)))


if __name__ == "__main__":
    main(*sys.argv[1:4])
