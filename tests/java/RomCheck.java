import com.uprfvx.romio.romio.RomOpener;
import com.uprfvx.romio.romhandlers.RomHandler;
import java.io.File;

// Abre ROMs con FVX y dice si las reconoce y si son originales.
public class RomCheck {
    public static void main(String[] args) {
        for (String path : args) {
            RomOpener.Results r = new RomOpener().openRomFile(new File(path));
            if (!r.wasOpeningSuccessful()) {
                System.out.println(path + ": FALLO " + r.getFailType());
                continue;
            }
            RomHandler h = r.getRomHandler();
            System.out.println(path + ": " + h.getROMName() + " [" + h.getROMCode() + "] gen" + h.generationOfPokemon()
                    + " valida=" + h.isRomValid(System.out));
        }
    }
}
