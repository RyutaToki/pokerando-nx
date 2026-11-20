import com.uprfvx.romio.constants.Gen4Constants;
import com.uprfvx.romio.constants.enctaggers.Gen4EncounterAreaTagger;
import java.lang.reflect.Field;
import java.util.List;

// Genera core/romio/gen4/gen4_enc_tags_gen.hpp con las listas que Gen4EncounterAreaTagger y
// Gen4Constants construyen por código (addCopies...). Uso: java ExtractGen4Tags > salida.hpp
public class ExtractGen4Tags {
    static Object field(Class<?> c, String name) throws Exception {
        Field f = c.getDeclaredField(name);
        f.setAccessible(true);
        return f.get(null);
    }

    static void ints(String name, int[] v) {
        StringBuilder sb = new StringBuilder("inline const std::vector<int> " + name + " = {");
        for (int i = 0; i < v.length; i++) sb.append(i == 0 ? "" : ", ").append(v[i]);
        System.out.println(sb.append("};"));
    }

    @SuppressWarnings("unchecked")
    static void strings(String name, Object o) {
        List<String> v = (List<String>) o;
        StringBuilder sb = new StringBuilder("inline const std::vector<std::string_view> " + name + " = {");
        for (int i = 0; i < v.size(); i++) sb.append(i == 0 ? "" : ", ").append('"').append(v.get(i).replace("\"", "\\\"")).append('"');
        System.out.println(sb.append("};"));
    }

    public static void main(String[] args) throws Exception {
        Class<?> t = Gen4EncounterAreaTagger.class;
        System.out.println("// Generado por tools/java/ExtractGen4Tags.java desde Gen4EncounterAreaTagger y Gen4Constants (FVX, GPLv3).");
        System.out.println("#pragma once\n#include <string_view>\n#include <vector>\n\nnamespace pr::gen4::tags {\n");
        for (String n : new String[]{"dpPostGameEncounterAreas", "platPostGameEncounterAreas", "hgssPostGameEncounterAreasTOD",
                "hgssPartialPostGameTOD", "hgssPostGameEncounterAreasNoTOD", "hgssPartialPostGameNoTOD"})
            ints(n, (int[]) field(t, n));
        System.out.println("inline constexpr int headbuttPartialPostgameCutoff = " + field(t, "headbuttPartialPostgameCutoff") + ";");
        for (String n : new String[]{"locationTagsDP", "locationTagsPt", "locationTagsNoTimeHGSS", "locationTagsUseTimeHGSS"})
            strings(n, field(t, n));
        strings("locationTagsTraverseOrderDPPt", Gen4Constants.getLocationTagsTraverseOrder(Gen4Constants.Type_DP));
        strings("locationTagsTraverseOrderHGSS", Gen4Constants.getLocationTagsTraverseOrder(Gen4Constants.Type_HGSS));
        System.out.println("\n}  // namespace pr::gen4::tags");
    }
}
