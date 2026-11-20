import com.uprfvx.romio.constants.Gen4Constants;
import com.uprfvx.romio.gamedata.Trainer;
import com.uprfvx.romio.gamedata.Type;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

// Genera core/romio/gen4/gen4_trainer_tags_gen.hpp aplicando las funciones de Gen4Constants
// (tagTrainers*, setMultiBattleStatus*) a una lista ficticia de entrenadores.
public class ExtractGen4Trainers {
    static void emit(String suffix, int type) {
        List<Trainer> trs = new ArrayList<>();
        for (int i = 0; i < 1500; i++) trs.add(new Trainer());
        if (type == Gen4Constants.Type_DP) { Gen4Constants.tagTrainersDP(trs); Gen4Constants.setMultiBattleStatusDP(trs); }
        else if (type == Gen4Constants.Type_Plat) { Gen4Constants.tagTrainersPt(trs); Gen4Constants.setMultiBattleStatusPt(trs); }
        else { Gen4Constants.tagTrainersHGSS(trs); Gen4Constants.setMultiBattleStatusHGSS(trs); }
        StringBuilder tags = new StringBuilder("inline const std::vector<std::pair<int, std::string_view>> trainerTags" + suffix + " = {");
        StringBuilder multi = new StringBuilder("inline const std::vector<std::pair<int, int>> multiBattle" + suffix + " = {");
        int maxTag = 0;
        for (int i = 0; i < trs.size(); i++) {
            Trainer t = trs.get(i);
            if (t.getTag() != null) { tags.append("{").append(i).append(", \"").append(t.getTag()).append("\"}, "); maxTag = i; }
            if (t.getMultiBattleStatus() != Trainer.MultiBattleStatus.NEVER)
                multi.append("{").append(i).append(", ").append(t.getMultiBattleStatus().ordinal()).append("}, ");
        }
        System.out.println(tags.append("};"));
        System.out.println(multi.append("};"));
    }

    static void themes(String suffix, Map<String, Type> m) {
        StringBuilder sb = new StringBuilder("inline const std::vector<std::pair<std::string_view, Type>> gymAndEliteThemes" + suffix + " = {");
        for (Map.Entry<String, Type> e : m.entrySet()) sb.append("{\"").append(e.getKey()).append("\", Type::").append(e.getValue().name()).append("}, ");
        System.out.println(sb.append("};"));
    }

    public static void main(String[] args) {
        System.out.println("// Generado por tools/java/ExtractGen4Trainers.java desde Gen4Constants (FVX, GPLv3).");
        System.out.println("// trainerTags: (posición en la lista de entrenadores, etiqueta); multiBattle: (posición, MultiBattleStatus).");
        System.out.println("#pragma once\n#include <string_view>\n#include <utility>\n#include <vector>\n\n#include \"gamedata/types.hpp\"\n\nnamespace pr::gen4::trtags {\n");
        emit("DP", Gen4Constants.Type_DP);
        emit("Pt", Gen4Constants.Type_Plat);
        emit("HGSS", Gen4Constants.Type_HGSS);
        themes("DP", Gen4Constants.gymAndEliteThemesDP);
        themes("Pt", Gen4Constants.gymAndEliteThemesPt);
        themes("HGSS", Gen4Constants.gymAndEliteThemesHGSS);
        System.out.println("\n}  // namespace pr::gen4::trtags");
    }
}
