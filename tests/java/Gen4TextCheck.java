import com.uprfvx.romio.newnds.NARCArchive;
import com.uprfvx.romio.newnds.NDSRom;
import java.util.List;
import java.util.zip.CRC32;
import text.PokeTextData;
import text.TextToPoke;

// Lee todos los textos de un msg.narc de Gen 4 y los vuelve a cifrar. Uso: Gen4TextCheck rom.nds ruta/msg.narc
public class Gen4TextCheck {
    public static void main(String[] args) throws Exception {
        NDSRom rom = new NDSRom(args[0]);
        NARCArchive narc = new NARCArchive(rom.getFile(args[1]));
        for (int f = 0; f < narc.files.size(); f++) {
            PokeTextData pt = new PokeTextData(narc.files.get(f));
            pt.decrypt();
            System.out.println("file " + f + " n=" + pt.strlist.size() + " comp=" + pt.compressFlag);
            for (String s : pt.strlist) System.out.println(" " + s);
            byte[] raw = TextToPoke.MakeFile(pt.strlist, pt.compressFlag);
            PokeTextData enc = new PokeTextData(raw);
            enc.SetKey(0xD00E);
            enc.encrypt();
            CRC32 c = new CRC32();
            c.update(enc.get());
            System.out.println("enc " + enc.get().length + " " + String.format("%08X", c.getValue()));
        }
    }
}
