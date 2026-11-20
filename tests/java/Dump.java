import com.uprfvx.romio.gamedata.*;
import com.uprfvx.romio.romio.RomOpener;
import com.uprfvx.romio.romhandlers.RomHandler;
import java.io.File;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.util.*;

// Vuelca con FVX los mismos datos (y en el mismo formato) que "pokerando-cli dump".
public class Dump {
    static String itemName(Item it) { return it == null ? "null" : it.getName(); }
    static String s(Object o) { return o == null ? "null" : o.toString(); }

    static void pset(PrintStream out, String n, Set<Item> s) {
        out.print("== " + n);
        for (Item i : s) out.print(" " + i.getId());
        out.println();
    }

    static void printStatic(PrintStream out, StaticEncounter se, int depth) {
        out.println(" ".repeat(depth * 2) + se.getSpecies().getNumber() + "@" + se.getLevel() + " egg=" + (se.isEgg() ? 1 : 0)
                + " max=" + se.getMaxLevel());
        for (StaticEncounter l : se.getLinkedEncounters()) printStatic(out, l, depth + 1);
    }

    public static void main(String[] args) throws Exception {
        PrintStream out = new PrintStream(System.out, true, StandardCharsets.UTF_8);
        PrintStream realOut = System.out;
        System.setOut(new PrintStream(PrintStream.nullOutputStream()));  // silenciar avisos de FVX
        RomHandler h = new RomOpener().openRomFile(new File(args[0])).getRomHandler();
        boolean valid = h.isRomValid(null);
        System.setOut(realOut);
        out.printf("ROM %s [%s] valida=%s%n", h.getROMName(), h.getROMCode(), valid);
        out.println("== ITEMS");
        for (Item it : h.getItems()) {
            if (it == null) continue;
            out.printf("%d|%s|allowed=%d|bad=%d|tm=%d%n", it.getId(), it.getName(), it.isAllowed() ? 1 : 0,
                    it.isBad() ? 1 : 0, it.isTM() ? 1 : 0);
        }
        out.println("== ABILITIES");
        for (int i = 0; i <= h.highestAbilityIndex(); i++) out.printf("%d|%s%n", i, h.abilityName(i));
        out.println("== MOVES perfectAccuracy=" + h.getPerfectAccuracy());
        for (Move m : h.getMoves()) {
            if (m == null) continue;
            out.print(String.format(Locale.ROOT, "%d|%s|pw=%d|pp=%d|acc=%.2f|%s|%s|eff=%d|tgt=%d|pri=%d|contact=%d|sound=%d|hits=%.2f|%s",
                    m.number, m.name, m.power, m.pp, m.hitratio, s(m.type), m.category.name(), m.effectIndex, m.target,
                    m.priority, m.makesContact ? 1 : 0, m.isSoundMove ? 1 : 0, m.hitCount, m.statChangeMoveType.name()));
            for (Move.StatChange sc : m.statChanges)
                out.print(String.format(Locale.ROOT, "|%s,%d,%.2f", sc.type.name(), sc.stages, sc.percentChance));
            out.print(String.format(Locale.ROOT, "|%s|%s|%.2f|flinch=%.2f|recoil=%d|absorb=%d|crit=%s|charge=%d|recharge=%d|trap=%d%n",
                    m.statusMoveType.name(), m.statusType.name(), m.statusPercentChance, m.flinchPercentChance,
                    m.recoilPercent, m.absorbPercent, m.criticalChance.name(), m.isChargeMove ? 1 : 0,
                    m.isRechargeMove ? 1 : 0, m.isTrapMove ? 1 : 0));
        }
        out.println("== SPECIES");
        for (Species pk : h.getSpecies()) {
            if (pk == null) continue;
            var bs = pk.getBaseStats();
            var bi = pk.getBreedingInfo();
            out.printf("%d|%s|gen%d|%d/%d/%d/%d/%d/%d|bst=%d|%s/%s|catch=%d|ev=%d|%s|ab=%d,%d|items=%s,%s,%s|g=%d|egg=%s/%s/%d%n",
                    pk.getNumber(), pk.getName(), pk.getGeneration(), bs.getHp(), bs.getAttack(), bs.getDefense(),
                    bs.getSpatk(), bs.getSpdef(), bs.getSpeed(), bs.getBST(), s(pk.getPrimaryType(false)),
                    s(pk.getSecondaryType(false)), pk.getCatchRate(), pk.getEVYield().toWord(), (pk.getGrowthCurve() == null ? "null" : pk.getGrowthCurve().name()),
                    pk.getAbility1(), pk.getAbility2(), itemName(pk.getGuaranteedHeldItem()),
                    itemName(pk.getCommonHeldItem()), itemName(pk.getRareHeldItem()), pk.getGenderRatio(),
                    s(bi.getPrimaryEggGroup()), s(bi.getSecondaryEggGroup()), bi.getEggCycles());
        }
        out.println("== PALETTES");
        for (Species pk : h.getSpecies()) {
            if (pk == null) continue;
            out.println(pk.getNumber() + " " + java.util.HexFormat.of().formatHex(pk.getNormalPalette().toBytes()) + " "
                    + java.util.HexFormat.of().formatHex(pk.getShinyPalette().toBytes()));
        }
        out.println("== EVOLUTIONS");
        for (Species pk : h.getSpecies()) {
            if (pk == null) continue;
            for (Evolution e : pk.getEvolutionsFrom())
                out.printf("%d>%d|%s|%d|est=%d%n", e.getFrom().getNumber(), e.getTo().getNumber(), e.getType().name(),
                        e.getExtraInfo(), e.getEstimatedEvoLvl());
            out.printf("%d to:%d%n", pk.getNumber(), pk.getEvolutionsTo().size());
        }
        out.println("== MOVESETS");
        for (var en : new java.util.TreeMap<>(h.getMovesLearnt()).entrySet()) {
            out.print(en.getKey() + ":");
            for (MoveLearnt m : en.getValue()) out.print(" " + m.move + "@" + m.level);
            out.println();
        }
        out.println("== ENCOUNTERS");
        List<EncounterArea> areas = h.getEncounters(false);
        for (int i = 0; i < areas.size(); i++) {
            EncounterArea a = areas.get(i);
            out.printf("%d|%s|rate=%d|%s|%s|pg=%d|cut=%d|ban=", i, a.getDisplayName(), a.getRate(),
                    s(a.getEncounterType()), s(a.getLocationTag()), a.isPostGame() ? 1 : 0, a.getPartiallyPostGameCutoff());
            List<Integer> nums = new ArrayList<>();
            for (Species b : a.getBannedSpecies()) nums.add(b.getNumber());
            Collections.sort(nums);
            for (int n : nums) out.print(n + ",");
            out.print("|");
            for (Encounter e : a) out.print(" " + e.getSpecies().getNumber() + "@" + e.getLevel() + "-" + e.getMaxLevel());
            out.println();
        }
        out.println("== SORTED");
        for (EncounterArea a : h.getSortedEncounters(false)) out.println(a.getDisplayName());
        out.println("== WILDBANNED");
        for (Species b : h.getBannedForWildEncounters()) out.println(b.getNumber());
        out.println("== TWEAKS " + h.miscTweaksAvailable());
        out.println("== TYPETABLE");
        TypeTable tt = h.getTypeTable();
        for (Type a : tt.getTypes()) {
            for (Type d : tt.getTypes()) out.print(tt.getEffectiveness(a, d).ordinal());
            out.println();
        }
        out.println("== TRAINERCLASSES");
        for (TrainerClass tc : h.getTrainerClasses()) out.println(tc.getID() + "|" + tc.getName());
        out.println("== TRAINERS");
        for (Trainer t : h.getTrainers()) {
            out.printf("%d|%x|%d|%s|%s|multi=%d|style=%d|pt=%d|", t.getIndex(), t.getOffset(),
                    t.getTrainerclass() == null ? -1 : t.getTrainerclass().getID(), s(t.getName()), s(t.getTag()),
                    t.getMultiBattleStatus().ordinal(), t.getCurrBattleStyle().getStyle().ordinal(), t.getPoketype());
            for (TrainerPokemon tp : t.getPokemon()) {
                int[] m = tp.getMoves();
                out.printf(" %d@%d/iv%d/it%d/ab%d/m%d,%d,%d,%d", tp.getSpecies().getNumber(), tp.getLevel(), tp.getIVs(),
                        tp.getHeldItem() == null ? -1 : tp.getHeldItem().getId(), tp.getAbilitySlot(), m[0], m[1], m[2], m[3]);
            }
            out.println();
        }
        out.print("== E4 ");
        for (int i : h.getEliteFourTrainers(false)) out.print(i + ",");
        out.println();
        out.print("== STARTERS");
        for (Species st : h.getStarters()) out.print(" " + st.getNumber());
        for (Item it : h.getStarterHeldItems()) out.print(" item=" + (it == null ? -1 : it.getId()));
        out.print("\n== TMS");
        for (int m : h.getTMMoves()) out.print(" " + m);
        out.print("\n== HMS");
        for (int m : h.getHMMoves()) out.print(" " + m);
        out.print("\n== TUTORS");
        for (int m : h.getMoveTutorMoves()) out.print(" " + m);
        out.println("\n== TMCOMPAT");
        for (var en : h.getTMHMCompatibility().entrySet()) {
            out.print(en.getKey().getNumber() + ":");
            for (boolean b : en.getValue()) out.print(b ? 1 : 0);
            out.println();
        }
        out.println("== TUTORCOMPAT");
        for (var en : h.getMoveTutorCompatibility().entrySet()) {
            out.print(en.getKey().getNumber() + ":");
            for (boolean b : en.getValue()) out.print(b ? 1 : 0);
            out.println();
        }
        out.println();
        pset(out, "EVOITEMS", h.getEvolutionItems());
        pset(out, "OPSHOP", h.getOPShopItems());
        pset(out, "CONSUMABLE", h.getAllConsumableHeldItems());
        pset(out, "REQTMS", h.getRequiredFieldTMs());
        out.print("== FIELD");
        for (Item i : h.getFieldItems()) out.print(" " + i.getId());
        out.println("\n== SHOPS");
        for (Shop sh : h.getShops()) {
            out.print(sh.getName() + "|main=" + (sh.isMainGame() ? 1 : 0) + "|special=" + (sh.isSpecialShop() ? 1 : 0) + "|");
            for (Item i : sh.getItems()) out.print(" " + i.getId());
            out.println();
        }
        out.print("== PRICES");
        for (int p : h.getShopPrices()) out.print(" " + p);
        out.println("\n== TRADES");
        for (InGameTrade t : h.getInGameTrades()) {
            out.print(t.getRequestedSpecies().getNumber() + ">" + t.getGivenSpecies().getNumber() + "|" + t.getNickname()
                    + "|" + s(t.getOtName()) + "|ot=" + t.getOtId() + "|item="
                    + (t.getHeldItem() == null ? -1 : t.getHeldItem().getId()) + "|iv=");
            for (int v : t.getIVs()) out.print(v + ",");
            out.println();
        }
        out.println("== SENSIBLE");
        for (int ti = 0; ti < h.getTrainers().size(); ti += 37) {
            for (TrainerPokemon tp : h.getTrainers().get(ti).getPokemon()) {
                for (int co = 0; co < 2; co++) {
                    out.print(ti + ":" + tp.getSpecies().getNumber() + ":" + co);
                    for (Item i : h.getSensibleHeldItemsFor(tp, co == 1, h.getMoves(), tp.getMoves()))
                        out.print(" " + (i == null ? -1 : i.getId()));
                    out.println();
                }
            }
        }
        out.println();
        out.println("== EGGMOVES");
        for (var en : new java.util.TreeMap<>(h.getEggMoves()).entrySet()) {
            out.print(en.getKey() + ":");
            for (int m : en.getValue()) out.print(" " + m);
            out.println();
        }
        out.println("== STATICS");
        for (StaticEncounter se : h.getStaticPokemon()) printStatic(out, se, 0);
        out.println("== PICKUP");
        for (PickupItem p : h.getPickupItems()) {
            out.print(p.getItem().getId() + ":");
            for (int pr : p.getProbabilities()) out.print(" " + pr);
            out.println();
        }
        if (args.length > 1) {
            System.setOut(new PrintStream(PrintStream.nullOutputStream()));
            h.setMovesLearnt(h.getMovesLearnt());
            h.setEggMoves(h.getEggMoves());
            h.setStarters(h.getStarters());
            h.setStarterHeldItems(h.getStarterHeldItems());
            h.setStaticPokemon(h.getStaticPokemon());
            h.setTMMoves(h.getTMMoves());
            h.setTMHMCompatibility(h.getTMHMCompatibility());
            h.setMoveTutorMoves(h.getMoveTutorMoves());
            h.setMoveTutorCompatibility(h.getMoveTutorCompatibility());
            h.setFieldItems(h.getFieldItems());
            h.setShops(h.getShops());
            h.setShopPrices(h.getShopPrices());
            h.setPickupItems(h.getPickupItems());
            h.setInGameTrades(h.getInGameTrades());
            h.setTypeTable(h.getTypeTable());
            h.removeImpossibleEvolutions(true, false);
            h.makeEvolutionsEasier(true, false);
            int avail = h.miscTweaksAvailable();
            for (com.uprfvx.romio.MiscTweak t : com.uprfvx.romio.MiscTweak.allTweaks) {
                if ((avail & t.getValue()) != 0 && t != com.uprfvx.romio.MiscTweak.BAN_LUCKY_EGG) h.applyMiscTweak(t);
            }
            h.setIntroPokemon(h.getSpecies().get(25));
            h.setCatchingTutorial(h.getSpecies().get(263), h.getSpecies().get(258));
            h.setPCPotionItem(h.getItems().get(com.uprfvx.romio.constants.ItemIDs.superPotion));
            h.enableGuaranteedPokemonCatching();
            h.saveRom(args[1], 0, false);  // guardado completo de FVX
            System.setOut(realOut);
        }
    }
}
