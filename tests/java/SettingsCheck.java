import com.uprfvx.random.Settings;
import com.uprfvx.romio.gamedata.ExpCurve;
import java.lang.reflect.*;
import java.util.*;

// Genera settings strings aleatorias con FVX y vuelca sus campos, para comparar con C++.
public class SettingsCheck {
    public static void main(String[] a) throws Exception {
        Random r = new Random(7);
        StringBuilder sb = new StringBuilder();
        for (int n = 0; n < 200; n++) {
            Settings s = new Settings();
            for (Field f : Settings.class.getDeclaredFields()) {
                if (Modifier.isStatic(f.getModifiers())) continue;
                f.setAccessible(true);
                Class<?> t = f.getType();
                if (t == boolean.class) f.setBoolean(s, r.nextBoolean());
                else if (t.isEnum()) {
                    Object[] v = t.getEnumConstants();
                    f.set(s, v[r.nextInt(v.length)]);
                }
            }
            s.setSelectedEXPCurve(ExpCurve.values()[r.nextInt(ExpCurve.values().length)]);
            if (s.getSelectedEXPCurve() == ExpCurve.ERRATIC) s.setSelectedEXPCurve(ExpCurve.SLOW);
            s.setRomName("Pokemon Emerald (S)");
            s.setCurrentMiscTweaks(r.nextInt(1 << 26));
            s.setTrainersLevelModifier(r.nextInt(100) - 50);
            s.setWildLevelModifier(r.nextInt(100) - 50);
            s.setGuaranteedMoveCount(2 + r.nextInt(3));
            s.setMovesetsGoodDamagingPercent(r.nextInt(100));
            s.setCustomStarters(new int[]{r.nextInt(386) + 1, r.nextInt(386) + 1, r.nextInt(386) + 1});
            s.setStartersBSTMinimum(r.nextInt(400));
            s.setStartersBSTMaximum(400 + r.nextInt(300));
            s.setMakeEvolutionsEasierLvl(r.nextInt(100));
            s.setEliteFourUniquePokemonNumber(r.nextInt(7));
            s.setMinimumCatchRateLevel(1 + r.nextInt(5));
            String str = s.toString();
            sb.append(str).append('\n');
            Settings back = Settings.fromString(str);
            List<String> lines = new ArrayList<>();
            for (Field f : Settings.class.getDeclaredFields()) {
                if (Modifier.isStatic(f.getModifiers())) continue;
                f.setAccessible(true);
                Object v = f.get(back);
                Class<?> t = f.getType();
                String val;
                if (t == boolean.class) val = ((Boolean) v) ? "1" : "0";
                else if (t == int.class) val = v.toString();
                else if (t.isEnum()) val = v == null ? "null" : ((Enum<?>) v).name();
                else continue;
                lines.add(f.getName() + "=" + val);
            }
            Collections.sort(lines);
            for (String l : lines) sb.append(l).append('\n');
        }
        System.out.print(sb);
    }
}
