#!/usr/bin/env python3
"""Convierte las clases *IDs.java de FVX (constantes int) en cabeceras C++.

Uso: convert_ids.py Entrada.java salida.hpp NombreNamespace
"""
import re
import sys

CPP_ONLY_KEYWORDS = {
    "and", "and_eq", "asm", "auto", "bitand", "bitor", "compl", "concept", "consteval",
    "constexpr", "constinit", "co_await", "co_return", "co_yield", "decltype", "delete",
    "explicit", "export", "extern", "friend", "inline", "mutable", "namespace", "not",
    "not_eq", "operator", "or", "or_eq", "register", "requires", "signed", "sizeof",
    "struct", "template", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "wchar_t", "xor", "xor_eq", "alignas", "alignof", "char8_t", "char16_t",
    "char32_t", "nullptr", "static_assert", "thread_local", "NULL", "EOF",
}

CONST_RE = re.compile(r"public\s+static\s+final\s+int\s+(\w+)\s*=\s*([^;]+);")
CLASS_RE = re.compile(r"public\s+(?:static\s+)?(?:final\s+)?class\s+(\w+)\s*\{")


def fix_name(name):
    return name + "_" if name in CPP_ONLY_KEYWORDS else name


def fix_expr(expr):
    expr = re.sub(r"\b[A-Z]\w*IDs\.", "", expr)  # quitar prefijos tipo ItemIDs.
    return re.sub(r"\b([A-Za-z_]\w*)\b", lambda m: fix_name(m.group(1)), expr)


def main(src, dst, ns):
    out = [
        "// Generado automáticamente por tools/convert_ids.py a partir de FVX. No editar.",
        "// Origen: " + src.split("/")[-1] + " (Universal Pokemon Randomizer FVX, GPLv3)",
        "#pragma once",
        "",
        "namespace pr::" + ns + " {",
    ]
    depth = 0
    for line in open(src, encoding="utf-8"):
        stripped = line.strip()
        cls = CLASS_RE.search(stripped)
        if cls:
            if depth >= 1:
                out.append("    " * depth + "namespace " + cls.group(1) + " {")
            depth += 1
            continue
        for m in CONST_RE.finditer(stripped):
            # admite varias en una línea: "int a = 1, b = a + 2;"
            decls = (m.group(1) + " = " + m.group(2)).split(",")
            for decl in decls:
                name, expr = decl.split("=", 1)
                out.append("    " * depth + "inline constexpr int " + fix_name(name.strip()) + " = "
                           + fix_expr(expr.strip()) + ";")
        if depth > 1 and stripped == "}":
            depth -= 1
            out.append("    " * depth + "}")
    out.append("}  // namespace pr::" + ns)
    open(dst, "w", encoding="utf-8").write("\n".join(out) + "\n")


if __name__ == "__main__":
    main(*sys.argv[1:4])
