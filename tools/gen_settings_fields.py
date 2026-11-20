#!/usr/bin/env python3
"""Genera core/settings/settings_fields.inc (enums y campos de Settings.java de FVX)."""
import re
import sys

src, dst = sys.argv[1], sys.argv[2]
text = open(src, encoding="utf-8").read()
start = text.index("public static final int MAKE_EVOLUTIONS_EASIER_DEFAULT_LVL")
end = text.index("public void writeToFileFormat")
body = re.sub(r"//[^\n]*", "", text[start:end])

TYPE_MAP = {"boolean": "bool", "int": "int", "String": "std::string", "int[]": "std::vector<int>",
            "ExpCurve": "std::optional<ExpCurve>", "Type": "std::optional<Type>",
            "GenRestrictions": "std::optional<GenRestrictions>", "BattleStyle": "BattleStyle"}
out = ["// Generado por tools/gen_settings_fields.py desde Settings.java (FVX). No editar.", ""]
enums = []
for m in re.finditer(r"public enum (\w+)\s*\{([^}]*)\}|private ([\w\[\]<>]+) ([^;=]+?)(?:\s*=\s*([^;]+))?;", body):
    if m.group(1):
        name = m.group(1)
        vals = [v.strip() for v in m.group(2).split(",") if v.strip()]
        enums.append(name)
        out.append("enum class %s { %s };" % (name, ", ".join(vals)))
        out.append("static constexpr const char* %sNames[] = {%s};" % (name, ", ".join('"%s"' % v for v in vals)))
        continue
    jtype, names, default = m.group(3), m.group(4), m.group(5)
    ctype = TYPE_MAP.get(jtype, jtype)
    for n in [x.strip() for x in names.split(",")]:
        d = ""
        if default is not None:
            dv = default.strip()
            if dv.startswith("new int["):
                d = " = std::vector<int>(%s)" % re.search(r"\[(\d+)\]", dv).group(1)
            elif dv.startswith("new GenRestrictions"):
                d = " = GenRestrictions()"
            elif dv.startswith("new "):
                d = ""
            elif dv == "null":
                d = ""
            else:
                dv = re.sub(r"\b(\w+)\.(\w+)\b", r"\1::\2", dv)
                d = " = " + dv
        elif ctype in ("bool",):
            d = " = false"
        elif ctype == "int":
            d = " = 0"
        out.append("%s %s%s;" % (ctype, n, d))
open(dst, "w", encoding="utf-8").write("\n".join(out) + "\n")
print(len(enums), "enums")
