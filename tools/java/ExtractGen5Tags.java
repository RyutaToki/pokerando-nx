import com.uprfvx.romio.constants.Gen5Constants;
import com.uprfvx.romio.constants.enctaggers.Gen5EncounterAreaTagger;
import java.lang.reflect.Field;
import java.util.List;

// Genera core/romio/gen4/gen5_enc_tags_gen.hpp con las listas que Gen5EncounterAreaTagger y
// Gen5Constants construyen por código (addCopies...). Uso: java ExtractGen5Tags > salida.hpp
public class ExtractGen5Tags {
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
        Class<?> t = Gen5EncounterAreaTagger.class;
        System.out.println("// Generado por tools/java/ExtractGen5Tags.java desde Gen5EncounterAreaTagger y Gen5Constants (FVX, GPLv3).");
        System.out.println("#pragma once\n#include <string_view>\n#include <vector>\n\nnamespace pr::gen5::tags {\n");
        for (String n : new String[]{"bwPostGameEncounterAreasTOD", "bwPostGameEncounterAreasNoTOD",
                "b2w2PostGameEncounterAreasTOD", "b2w2PostGameEncounterAreasNoTOD"})
            ints(n, (int[]) field(t, n));
        for (String n : new String[]{"locationTagsNoTimeBW", "locationTagsUseTimeBW", "locationTagsNoTimeBW2", "locationTagsUseTimeBW2"})
            strings(n, field(t, n));
        System.out.println("\n}  // namespace pr::gen5::tags");
    }
}
