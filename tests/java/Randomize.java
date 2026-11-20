import com.uprfvx.random.GameRandomizer;
import com.uprfvx.random.Settings;
import com.uprfvx.romio.romio.RomOpener;
import com.uprfvx.romio.romhandlers.RomHandler;
import java.io.*;
import java.util.ResourceBundle;

// Randomiza con FVX: Randomize <rom> <settingsString> <semilla> <salida.gba> [log.txt]
public class Randomize {
    public static void main(String[] a) throws Exception {
        PrintStream realOut = System.out;
        System.setOut(new PrintStream(OutputStream.nullOutputStream()));
        // FVX calibra su selector de colores con un Random sin semilla (no reproducible);
        // aquí se fija el valor teórico 3.0, el mismo que usa PokéRando NX
        java.lang.reflect.Field fSel = Class.forName("com.uprfvx.romio.graphics.palettes.Gen3to5TypeColors")
                .getDeclaredField("randomColorSelector");
        fSel.setAccessible(true);
        Object sel = fSel.get(null);
        java.lang.reflect.Field fPts = sel.getClass().getDeclaredField("randomPointSelector");
        fPts.setAccessible(true);
        Object pts = fPts.get(sel);
        java.lang.reflect.Field fMax = pts.getClass().getDeclaredField("maxValue");
        fMax.setAccessible(true);
        fMax.setDouble(pts, 3.0);
        RomHandler h = new RomOpener().openRomFile(new File(a[0])).getRomHandler();
        Settings s = Settings.fromString(a[1]);
        s.tweakForRom(h);
        ResourceBundle bundle = ResourceBundle.getBundle("com.uprfvx.random.gui.Bundle");
        PrintStream log = a.length > 4 ? new PrintStream(new FileOutputStream(a[4]), true, "UTF-8")
                : new PrintStream(OutputStream.nullOutputStream());
        GameRandomizer.Results r = new GameRandomizer(s, null, h, bundle, false).randomize(a[3], log, Long.parseLong(a[2]));
        System.setOut(realOut);
        if (!r.wasSaveSuccessful()) {
            r.getException().printStackTrace();
            System.exit(1);
        }
        System.out.println("checkValue=" + r.getCheckValue());
    }
}
