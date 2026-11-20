// Gen 4: tabla de tipos, ajustes disponibles y Pokémon vetados.
#include <stdexcept>

#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "romio/misc_tweak.hpp"

namespace pr::gen4 {

SpeciesSet Gen4RomHandler::getBannedForWildEncounters() const {
    // Unown: en DPPt no se consiguen todas las letras fuera de las Ruinas Sosiego; en HGSS no salen
    // hasta resolver un puzle en las Ruinas Alfa
    return SpeciesSet(pokes_.at(species::unown));
}

SpeciesSet Gen4RomHandler::getBannedFormesForTrainerPokemon() const {
    SpeciesSet banned;
    // Giratina-Origen vuelve a su forma Modificada si no lleva el Griseogranate
    if (romEntry_->romType() != Type_DP) banned.add(pokes_.at(species::Gen4Formes::giratinaO));
    return banned;
}

int Gen4RomHandler::miscTweaksAvailable() const {
    using namespace misctweak;
    int a = LOWER_CASE_POKEMON_NAMES.value;
    a |= RANDOMIZE_CATCHING_TUTORIAL.value;
    if (romEntry_->hasTweakFile("FastestTextTweak")) a |= FASTEST_TEXT.value;
    a |= BAN_LUCKY_EGG.value;
    if (romEntry_->hasTweakFile("NationalDexAtStartTweak")) a |= NATIONAL_DEX_AT_START.value;
    a |= RUN_WITHOUT_RUNNING_SHOES.value;
    a |= FASTER_HP_AND_EXP_BARS.value;
    if (romEntry_->hasTweakFile("FastDistortionWorldTweak")) a |= FAST_DISTORTION_WORLD.value;
    if (romEntry_->romType() == Type_Plat || romEntry_->romType() == Type_HGSS) a |= UPDATE_ROTOM_FORME_TYPING.value;
    if (romEntry_->getInt("TMMovesReusableFunctionOffset") != 0) a |= REUSABLE_TMS.value;
    a |= FORGETTABLE_HMS.value;
    a |= FAST_EGG_HATCHING.value;
    a |= NO_EV_YIELDS.value;
    return a;
}

TypeTable Gen4RomHandler::getTypeTable() const {
    if (typeTable_) return *typeTable_;
    TypeTable t(TypeTable::vanillaGen2To5().types());
    Bytes ovl = readOverlay(romEntry_->getInt("BattleOvlNumber"));
    int cur = romEntry_->getInt("TypeEffectivenessOffset");
    auto sb = [&](int o) { return static_cast<int>(static_cast<int8_t>(ovl.at(o))); };
    int attacking = sb(cur);
    while (attacking != static_cast<int8_t>(typeTableTerminator)) {
        if (sb(cur) != static_cast<int8_t>(typeTableForesightTerminator)) {
            int defending = sb(cur + 1);
            std::optional<Effectiveness> e;
            switch (sb(cur + 2)) {
                case 20: e = Effectiveness::DOUBLE; break;
                case 10: e = Effectiveness::NEUTRAL; break;
                case 5: e = Effectiveness::HALF; break;
                case 0: e = Effectiveness::ZERO; break;
            }
            if (e) t.setEffectiveness(*typeFromByte(attacking), *typeFromByte(defending), *e);
        }
        cur += 3;
        attacking = sb(cur);
    }
    typeTable_ = t;
    return t;
}

void Gen4RomHandler::setTypeTable(const TypeTable& t) {
    typeTable_ = t;
    if (t.nonNeutralEffectivenessCount() > nonNeutralEffectivenessCount)
        throw std::invalid_argument("Too many non-neutral Effectiveness-es. Was " + std::to_string(t.nonNeutralEffectivenessCount()) +
                                    ", has to be at most " + std::to_string(nonNeutralEffectivenessCount));
    int ovlNum = romEntry_->getInt("BattleOvlNumber");
    Bytes ovl = readOverlay(ovlNum);
    int offset = romEntry_->getInt("TypeEffectivenessOffset");
    Bytes mainPart, ghost;
    for (Type a : t.types()) {
        for (Type d : t.types()) {
            Effectiveness e = t.getEffectiveness(a, d);
            if (e == Effectiveness::NEUTRAL) continue;
            auto& part = (d == Type::GHOST && e == Effectiveness::ZERO) ? ghost : mainPart;
            uint8_t internal = e == Effectiveness::DOUBLE ? 20 : (e == Effectiveness::HALF ? 5 : 0);
            part.push_back(static_cast<uint8_t>(typeToByte(a)));
            part.push_back(static_cast<uint8_t>(typeToByte(d)));
            part.push_back(internal);
        }
    }
    auto put = [&](const Bytes& b) {
        std::copy(b.begin(), b.end(), ovl.begin() + offset);
        offset += static_cast<int>(b.size());
    };
    put(mainPart);
    put({typeTableForesightTerminator, typeTableForesightTerminator, 0x00});
    put(ghost);
    put({typeTableTerminator, typeTableTerminator, 0x00});
    writeOverlay(ovlNum, ovl);
}

}  // namespace pr::gen4
