#!/usr/bin/env python3
"""Traduce MoveSynergy.java de FVX a core/gamedata/move_synergy.cpp (getSoftMoveSynergy se hace a mano)."""
import re, sys
sys.path.insert(0, "tools")
from j2cpp import convert

src = open("reference/fvx/romio/src/main/java/com/uprfvx/romio/gamedata/MoveSynergy.java").read()
body = src[src.index("public class MoveSynergy {") + len("public class MoveSynergy {"):src.rindex("}")]
body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
body = re.sub(r"//(?! fallthrough).*", "", body)

# getSoftMoveSynergy se escribe a mano (usa la tabla de tipos)
i = body.index("    public static List<Move> getSoftMoveSynergy")
j = body.index("    public static List<Move> getHardMoveAntiSynergy")
body = body[:i] + body[j:]

body = re.sub(r"(\w+)\.addAll\(moveList\s*\.stream\(\)\s*\.filter\(mv -> (.*?)\)\s*\.map\(mv -> mv\.number\)\s*"
              r"\.collect\(Collectors\.toList\(\)\)\);",
              lambda m: "for (const Move* mv : moveList) if (" + re.sub(r"\s+", " ", m.group(2)) + ") " + m.group(1) + ".push_back(mv->number);",
              body, flags=re.S)
body = re.sub(r"return moveList\s*\.stream\(\)\s*\.filter\(mv -> (\w+)\.contains\(mv\.number\)\)\s*\.distinct\(\)\s*"
              r"\.collect\(toCollection\(ArrayList::new\)\);", r"return pickIn(moveList, \1);", body, flags=re.S)
assert ".stream()" not in body, body[body.index(".stream()") - 300:body.index(".stream()") + 200]
body = re.sub(r"List<Integer> (\w+) = new ArrayList<>\(\);", r"std::vector<int> \1;", body)
body = re.sub(r"(\w+)\.add\(", r"\1.push_back(", body)
body = body.replace("public static List<Move> ", "std::vector<Move*> ")
body = body.replace("List<Move> moveList", "const std::vector<Move*>& moveList")
body = body.replace("Type pkType1", "std::optional<Type> pkType1").replace("Type pkType2", "std::optional<Type> pkType2")
body = body.replace("Move mv1", "const Move* mv1").replace("BaseStats bs", "const BaseStats& bs")
body = body.replace("!(bs instanceof Gen1BaseStats)", "true")
body = re.sub(r"\bbs\.getHp\(\)", "bs.getHp()", body)
body = re.sub(r"\bbs\.getBST\(\)", "bs.bst()", body)
body = re.sub(r"\bbs\.get(Attack|Defense|Spatk|Spdef|Speed)\(\)", lambda m: "bs." + m.group(1).lower(), body)
body = re.sub(r"\bmv1\.", "mv1->", body)
body = re.sub(r"\bmv\.", "mv->", body)
body = body.replace("if (mv1->type != null) {\n            switch (mv1->type) {", "if (mv1->type) {\n            switch (*mv1->type) {")
body = re.sub(r"case (FIRE|ELECTRIC):", r"case Type::\1:", body)
out = []
for line in body.split("\n"):
    out.append(convert(line))
body = "\n".join(out)
body = body.replace("// fallthrough", "[[fallthrough]];")
assert "getBST" not in body and "bs.get" not in body.replace("bs.getHp", "")
hdr = '''// Generado por tools/translate_move_synergy.py a partir de MoveSynergy.java de FVX. No editar a mano.
#include "gamedata/move_synergy.hpp"

#include <algorithm>

#include "constants/ability_ids.hpp"
#include "constants/move_ids.hpp"

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

namespace pr::MoveSynergy {

namespace {
std::vector<Move*> pickIn(const std::vector<Move*>& moveList, const std::vector<int>& nums) {
    std::vector<Move*> r;
    for (Move* mv : moveList)
        if (std::find(nums.begin(), nums.end(), mv->number) != nums.end() && std::find(r.begin(), r.end(), mv) == r.end())
            r.push_back(mv);
    return r;
}
}  // namespace

std::vector<Move*> getSoftMoveSynergy(const Move* mv1, const std::vector<Move*>& moveList, const TypeTable& typeTable) {
    std::vector<int> synergisticMoves;
    if (mv1->category != MoveCategory::STATUS) {
        // en Java, mv1.type == null lanzaría una excepción; en Gen 3 todos los ataques con daño tienen tipo
        std::vector<Type> nve = typeTable.notVeryEffectiveWhenAttacking(*mv1->type);
        auto imm = typeTable.immuneWhenAttacking(*mv1->type);
        nve.insert(nve.end(), imm.begin(), imm.end());
        for (Type nveType : nve) {
            std::vector<Type> se;
            for (auto& [t, e] : typeTable.against(nveType, std::nullopt))
                if (e == Effectiveness::DOUBLE) se.push_back(t);
            for (const Move* mv : moveList)
                if (mv->category != MoveCategory::STATUS && mv->type && std::find(se.begin(), se.end(), *mv->type) != se.end())
                    synergisticMoves.push_back(mv->number);
        }
    }
    switch (mv1->number) {
        case moves::swordsDance:
            for (const Move* mv : moveList) if (mv->category == MoveCategory::PHYSICAL) synergisticMoves.push_back(mv->number);
            break;
        case moves::nastyPlot:
        case moves::tailGlow:
            for (const Move* mv : moveList) if (mv->category == MoveCategory::SPECIAL) synergisticMoves.push_back(mv->number);
            break;
    }
    return pickIn(moveList, synergisticMoves);
}
'''
open("core/gamedata/move_synergy.cpp", "w").write(hdr + body + "\n}  // namespace pr::MoveSynergy\n")
