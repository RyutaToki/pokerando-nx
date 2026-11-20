import com.uprfvx.romio.gamedata.*;
import com.uprfvx.romio.romio.RomOpener;
import com.uprfvx.romio.romhandlers.RomHandler;
import java.io.File;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;

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
