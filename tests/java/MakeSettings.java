import com.uprfvx.random.Settings;
import com.uprfvx.romio.gamedata.ExpCurve;
import java.lang.reflect.*;

// Construye una settings string a partir de opciones campo=valor: MakeSettings startersMod=COMPLETELY_RANDOM ...
public class MakeSettings {
    public static void main(String[] args) throws Exception {
        Settings s = new Settings();
        s.setSelectedEXPCurve(ExpCurve.MEDIUM_FAST);
        s.setRomName("Pokemon Emerald (S)");
        for (String arg : args) {
            String[] kv = arg.split("=", 2);
            if (kv[0].equals("battleStyleMod")) {
                s.getBattleStyle().setModification(com.uprfvx.romio.gamedata.BattleStyle.Modification.valueOf(kv[1]));
                continue;
            }
            if (kv[0].equals("battleStyle")) {
                s.getBattleStyle().setStyle(com.uprfvx.romio.gamedata.BattleStyle.Style.valueOf(kv[1]));
                continue;
            }
            Field f = Settings.class.getDeclaredField(kv[0]);
            f.setAccessible(true);
            Class<?> t = f.getType();
            if (t == boolean.class) f.setBoolean(s, kv[1].equals("1") || kv[1].equals("true"));
            else if (t == int.class) f.setInt(s, Integer.parseInt(kv[1]));
            else if (t.isEnum()) f.set(s, Enum.valueOf((Class) t, kv[1]));
            else throw new IllegalArgumentException("tipo no soportado: " + kv[0]);
        }
        System.out.println(s.toString());
    }
}
