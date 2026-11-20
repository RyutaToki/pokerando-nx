import com.uprfvx.romio.newnds.NARCArchive;
import com.uprfvx.romio.newnds.NDSRom;
import java.util.zip.CRC32;

// Prueba de la base de DS: lee ARM9 y overlays, reescribe ARM9 y un NARC y guarda la ROM.
// Uso: NdsCheck rom.nds salida.nds ruta/del/narc
public class NdsCheck {
    static String crc(byte[] b) {
        CRC32 c = new CRC32();
        c.update(b);
        return String.format("%08X", c.getValue());
    }

    public static void main(String[] args) throws Exception {
        NDSRom rom = new NDSRom(args[0]);
        System.out.println("code " + rom.getCode() + " version " + rom.getVersion());
        byte[] arm9 = rom.getARM9();
        System.out.println("arm9 " + arm9.length + " " + crc(arm9));
        int n = 0;
        for (;; n++) {
            byte[] o = rom.getOverlay(n);
            if (o == null) break;
            System.out.println("overlay " + n + " " + o.length + " " + crc(o) + " @" + Integer.toHexString(rom.getOverlayAddress(n)));
        }
        rom.writeARM9(arm9);
        byte[] narcBytes = rom.getFile(args[2]);
        NARCArchive narc = new NARCArchive(narcBytes);
        System.out.println("narc " + narc.files.size() + " " + crc(narcBytes));
        byte[] rebuilt = narc.getBytes();
        System.out.println("narc rebuilt " + rebuilt.length + " " + crc(rebuilt));
        rom.writeFile(args[2], rebuilt);
        rom.saveTo(args[1]);
    }
}
