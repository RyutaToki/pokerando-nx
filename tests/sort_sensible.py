"""Ordena los números de cada línea de la sección SENSIBLE de un volcado (modifica el archivo)."""
import sys

p = sys.argv[1]
out, inside = [], False
for line in open(p, encoding="utf-8", errors="surrogateescape").read().split("\n"):
    if line.startswith("== "):
        inside = line.startswith("== SENSIBLE")
    elif inside and line:
        head, _, rest = line.partition(" ")
        line = head + " " + " ".join(sorted(rest.split(), key=int)) if rest else line
    out.append(line)
open(p, "w", encoding="utf-8", errors="surrogateescape").write("\n".join(out))
