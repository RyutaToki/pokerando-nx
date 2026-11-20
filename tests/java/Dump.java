import com.uprfvx.romio.gamedata.*;
import com.uprfvx.romio.romio.RomOpener;
import com.uprfvx.romio.romhandlers.RomHandler;
import java.io.File;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

// Vuelca con FVX los mismos datos (y en el mismo formato) que "pokerando-cli dump".
public class Dump {
    static String itemName(Item it) { return it == null ? "null" : it.getName(); }
    static String s(Object o) { return o == null ? "null" : o.toString(); }

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
        for (int i = 0; i <= 77; i++) out.printf("%d|%s%n", i, h.abilityName(i));
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
    }
}
