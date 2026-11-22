import com.uprfvx.romio.constants.Gen5Constants;
import com.uprfvx.romio.gamedata.Trainer;
import com.uprfvx.romio.gamedata.Type;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

// Genera core/romio/gen5/gen5_trainer_tags_gen.hpp aplicando las funciones de Gen5Constants
// (tagTrainers*, setMultiBattleStatus*, setForcedRivalStarterPositions*) a una lista ficticia de entrenadores.
// Las funciones de FVX solo marcan índices menores que el tamaño de la lista: al aplicarlas se comprueba igual.
public class ExtractGen5Trainers {
    static void emit(String suffix, int which) {
        List<Trainer> trs = new ArrayList<>();
        for (int i = 0; i < 1500; i++) trs.add(new Trainer());
        if (which == 0) { Gen5Constants.tagTrainersBW(trs); Gen5Constants.setMultiBattleStatusBW(trs); Gen5Constants.setForcedRivalStarterPositionsBW(trs); }
        else { Gen5Constants.tagTrainersBW2(trs); Gen5Constants.setMultiBattleStatusBW2(trs, which == 1); Gen5Constants.setForcedRivalStarterPositionsBW2(trs); }
        StringBuilder tags = new StringBuilder("inline const std::vector<std::pair<int, std::string_view>> trainerTags" + suffix + " = {");
        StringBuilder multi = new StringBuilder("inline const std::vector<std::pair<int, int>> multiBattle" + suffix + " = {");
        StringBuilder starter = new StringBuilder("inline const std::vector<std::pair<int, int>> forcedStarter" + suffix + " = {");
        for (int i = 0; i < trs.size(); i++) {
            Trainer t = trs.get(i);
            if (t.getTag() != null) tags.append("{").append(i).append(", \"").append(t.getTag()).append("\"}, ");
            if (t.getMultiBattleStatus() != Trainer.MultiBattleStatus.NEVER)
                multi.append("{").append(i).append(", ").append(t.getMultiBattleStatus().ordinal()).append("}, ");
            if (t.getForceStarterPosition() >= 0) starter.append("{").append(i).append(", ").append(t.getForceStarterPosition()).append("}, ");
        }
        System.out.println(tags.append("};"));
        System.out.println(multi.append("};"));
        System.out.println(starter.append("};"));
    }

    static void themes(String suffix, Map<String, Type> m) {
        StringBuilder sb = new StringBuilder("inline const std::vector<std::pair<std::string_view, Type>> gymAndEliteThemes" + suffix + " = {");
        for (Map.Entry<String, Type> e : m.entrySet()) sb.append("{\"").append(e.getKey()).append("\", Type::").append(e.getValue().name()).append("}, ");
        System.out.println(sb.append("};"));
    }

    public static void main(String[] args) {
        System.out.println("// Generado por tools/java/ExtractGen5Trainers.java desde Gen5Constants (FVX, GPLv3).");
        System.out.println("// trainerTags: (posición, etiqueta); multiBattle: (posición, MultiBattleStatus); forcedStarter: (posición, hueco).");
        System.out.println("// Los temas siguen el orden de iteración del HashMap de Java.");
        System.out.println("#pragma once\n#include <string_view>\n#include <utility>\n#include <vector>\n\n#include \"gamedata/types.hpp\"\n\nnamespace pr::gen5::trtags {\n");
        emit("BW", 0);
        emit("B2", 1);
        emit("W2", 2);
        themes("BW", Gen5Constants.gymAndEliteThemesBW);
        themes("BW2", Gen5Constants.gymAndEliteThemesBW2);
        System.out.println("\n}  // namespace pr::gen5::trtags");
    }
}
