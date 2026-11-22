import com.uprfvx.romio.newnds.NARCArchive;
import com.uprfvx.romio.newnds.NDSRom;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.CRC32;
import text.PPTxtHandler;

// Lee todos los textos de un NARC de Gen 5 y los vuelve a guardar. Uso: Gen5TextCheck rom.nds ruta/a.narc
public class Gen5TextCheck {
    public static void main(String[] args) throws Exception {
        PrintStream out = new PrintStream(System.out, true, StandardCharsets.UTF_8);
        NDSRom rom = new NDSRom(args[0]);
        NARCArchive narc = new NARCArchive(rom.getFile(args[1]));
        for (int f = 0; f < narc.files.size(); f++) {
            List<String> strs;
            try {
                strs = new ArrayList<>(PPTxtHandler.readTexts(narc.files.get(f)));
            } catch (Exception e) {
                out.println("file " + f + " error");
                continue;
            }
            out.println("file " + f + " n=" + strs.size());
            for (String s : strs) out.println(" " + s);
            byte[] enc;
            try {
                enc = PPTxtHandler.saveEntry(narc.files.get(f), new ArrayList<>(strs));
            } catch (Exception e) {
                out.println("enc error");
                continue;
            }
            CRC32 c = new CRC32();
            c.update(enc);
            out.println("enc " + enc.length + " " + String.format("%08X", c.getValue()));
        }
    }
}
