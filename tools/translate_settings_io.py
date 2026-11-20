#!/usr/bin/env python3
"""Traduce Settings.toStringWithoutVersion/fromStringWithoutVersion de FVX a C++ (revisar a mano)."""
import json
import re
import sys

src, acc_path, dst = sys.argv[1:4]
lines = open(src, encoding="utf-8").read().split("\n")
acc = json.load(open(acc_path))
start = next(i for i, l in enumerate(lines) if "private String toStringWithoutVersion()" in l)
end = next(i for i, l in enumerate(lines) if "public static class TweakForROMFeedback" in l)
code = "\n".join(lines[start:end])
code = re.sub(r"//[^\n]*", "", code)

# unir sentencias en una sola línea
stmts, buf, depth = [], "", 0
for ch in code:
    buf += ch
    if ch == "(":
        depth += 1
    elif ch == ")":
        depth -= 1
    elif ch in ";{}" and depth == 0:
        stmts.append(" ".join(buf.split()))
        buf = ""
out = []
for s in stmts:
    if not s:
        continue
    s = re.sub(r"restoreEnum\((\w+(?:\.\w+)?)\.class,\s*(data\[\d+\]),\s*([^)]*)\)",
               lambda m: "restoreEnum<%s>(%s, {%s})" % (m.group(1).replace(".", "::"), m.group(2), m.group(3)), s)
    s = re.sub(r"makeByteSelected\(([^()]*(?:\([^()]*\)[^()]*)*)\)", r"makeByteSelected({\1})", s)
    s = re.sub(r"\bsettings\.(set\w+)\((.*)\);$", lambda m: "settings.%s = %s;" % (acc.get(m.group(1), m.group(1)[3].lower() + m.group(1)[4:]), m.group(2)), s)
    s = re.sub(r"\bsettings\.settingBattleStyle\.setModification\((.*)\);", r"settings.settingBattleStyle.mod = \1;", s)
    s = re.sub(r"\bsettings\.settingBattleStyle\.setStyle\((.*)\);", r"settings.settingBattleStyle.style = \1;", s)
    s = s.replace("settingBattleStyle.getModification()", "settingBattleStyle.mod")
    s = s.replace("settingBattleStyle.getStyle()", "settingBattleStyle.style")
    s = re.sub(r"\b([A-Z]\w+)\.([A-Z]\w+)\.([A-Z_]+)\b", r"\1::\2::\3", s)
    s = re.sub(r"\b([A-Z]\w+)\.([A-Z][A-Z_0-9]+)\b", r"\1::\2", s)
    s = re.sub(r"^out\.write\((.*)\);$", r"out.push_back(static_cast<uint8_t>(\1));", s)
    s = s.replace("IOFunctions.read2ByteInt(data, ", "read2ByteInt(data, ")
    s = s.replace("IOFunctions.readFullIntBigEndian(data, ", "readFullIntBigEndian(data, ")
    s = s.replace("IOFunctions.readFullInt(data, ", "readFullInt(data, ")
    s = s.replace("Byte.toUnsignedInt(", "static_cast<uint8_t>(")
    s = s.replace("(byte)", "(int8_t)")
    s = s.replace("ExpCurve.fromByte(", "expCurveFromByte(")
    out.append(s)
open(dst, "w", encoding="utf-8").write("\n".join(out) + "\n")
print(len(out), "sentencias")
