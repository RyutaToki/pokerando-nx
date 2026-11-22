#!/usr/bin/env python3
"""Reduce una combinación de opciones que da una ROM distinta de FVX a la mínima que lo sigue haciendo.
Uso: tests/bisect_rand.py <rom> <semilla> <archivo con las opciones> (OUT_DIR y CLI se pasan a compare_rand.sh)"""
import subprocess, sys

rom, seed, argfile = sys.argv[1], sys.argv[2], sys.argv[3]
args = open(argfile).read().split()


def differs(a):
    r = subprocess.run(["tests/compare_rand.sh", rom, seed] + a, capture_output=True, text=True)
    last = (r.stdout.strip().splitlines() or [""])[-1]
    return r.returncode == 1 and last.startswith("DIFERENTE")


assert differs(args), "no se reproduce"
i = 0
while i < len(args):
    trial = args[:i] + args[i + 1:]
    if differs(trial):
        print("sobra", args[i], flush=True)
        args = trial
    else:
        print("necesaria", args[i], flush=True)
        i += 1
print("MÍNIMA:", " ".join(args), flush=True)
