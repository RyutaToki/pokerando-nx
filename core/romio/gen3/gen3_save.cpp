// Guardado de los datos cargados (port de Gen3RomHandler.saveSpeciesStats, saveBasicPokeStats,
// saveMoves y writeFixedLengthString).
#include <algorithm>
#include <cmath>

#include "constants/species_ids.hpp"
#include "romio/gen3/gen3_constants.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"

namespace pr::gen3 {

void Gen3RomHandler::writeFixedLengthString(const std::string& str, int offset, int length) {
    auto translated = translateString(str);
    int len = std::min(static_cast<int>(translated.size()), length);
    std::copy(translated.begin(), translated.begin() + len, rom_.begin() + offset);
    if (len < length) {
        writeByte(offset + len, textTerminator);
        len++;
    }
    while (len < length) {
        writeByte(offset + len, textPadding);
        len++;
    }
}

void Gen3RomHandler::prepareSaveRom() {
    saveSpeciesStats();
    saveMoves();
    saveTrainerClasses();
    saveTrainers();
    savePokemonPalettes();
}

void Gen3RomHandler::saveSpeciesStats() {
    auto& e = *romEntry_;
    int offs = e.getInt("PokemonNames");
    int nameLen = e.getInt("PokemonNameLength");
    int offs2 = e.getInt("PokemonStats");
    int n = e.getInt("PokemonCount");
    for (int i = 1; i <= n; i++) {
        Species* pk = pokesInternal_[i];
        writeFixedLengthString(pk->name(), offs + i * nameLen, nameLen);
        saveBasicPokeStats(*pk, offs2 + i * baseStatsEntrySize);
    }
    // Stats fijos de Deoxys en el código: si no se escriben, Deoxys tendría siempre los originales
    if (e.romType() == RomType_FRLG || e.romType() == RomType_Em) {
        std::string prefix = e.getString("DeoxysStatPrefix");
        int offset = find(prefix);
        if (offset > 0) {
            offset += static_cast<int>(prefix.size() / 2);
            const BaseStats& bs = pokes_.at(species::deoxys)->baseStats();
            writeWord(offset, bs.getHp());
            writeWord(offset + 2, bs.attack);
            writeWord(offset + 4, bs.defense);
            writeWord(offset + 6, bs.speed);
            writeWord(offset + 8, bs.spatk);
            writeWord(offset + 10, bs.spdef);
        }
    }
    writeEvolutions();
}

void Gen3RomHandler::saveBasicPokeStats(const Species& pk, int o) {
    const BaseStats& bs = pk.baseStats();
    writeByte(o + bsHPOffset, static_cast<uint8_t>(bs.getHp()));
    writeByte(o + bsAttackOffset, static_cast<uint8_t>(bs.attack));
    writeByte(o + bsDefenseOffset, static_cast<uint8_t>(bs.defense));
    writeByte(o + bsSpeedOffset, static_cast<uint8_t>(bs.speed));
    writeByte(o + bsSpAtkOffset, static_cast<uint8_t>(bs.spatk));
    writeByte(o + bsSpDefOffset, static_cast<uint8_t>(bs.spdef));
    writeByte(o + bsPrimaryTypeOffset, static_cast<uint8_t>(typeToByte(pk.primaryType())));
    writeByte(o + bsSecondaryTypeOffset,
              static_cast<uint8_t>(typeToByte(pk.secondaryType() ? pk.secondaryType() : pk.primaryType())));
    writeByte(o + bsCatchRateOffset, static_cast<uint8_t>(pk.catchRate));
    writeWord(o + bsEVYieldOffset, pk.evYield.toWord());
    writeByte(o + bsGrowthCurveOffset, static_cast<uint8_t>(expCurveToByte(pk.growthCurve.value())));
    writeByte(o + bsAbility1Offset, static_cast<uint8_t>(pk.ability1));
    // necesario para no romper evoluciones con habilidades aleatorias
    writeByte(o + bsAbility2Offset, static_cast<uint8_t>(pk.ability2 == 0 ? pk.ability1 : pk.ability2));
    if (pk.guaranteedHeldItem) {
        int id = itemIDToInternal(*pk.guaranteedHeldItem);
        writeWord(o + bsCommonHeldItemOffset, id);
        writeWord(o + bsRareHeldItemOffset, id);
    } else {
        writeWord(o + bsCommonHeldItemOffset, pk.commonHeldItem ? itemIDToInternal(*pk.commonHeldItem) : 0);
        writeWord(o + bsRareHeldItemOffset, pk.rareHeldItem ? itemIDToInternal(*pk.rareHeldItem) : 0);
    }
    writeByte(o + bsGenderRatioOffset, static_cast<uint8_t>(pk.genderRatio));
    const BreedingInfo& bi = pk.breedingInfo;
    writeByte(o + bsPrimaryEggGroupOffset, static_cast<uint8_t>(eggGroupToID(bi.primary)));
    writeByte(o + bsSecondaryEggGroupOffset,
              static_cast<uint8_t>(bi.secondary ? eggGroupToID(*bi.secondary) : eggGroupToID(bi.primary)));
    writeByte(o + bsEggCyclesOffset, static_cast<uint8_t>(bi.eggCycles));
}

void Gen3RomHandler::saveMoves() {
    auto& e = *romEntry_;
    int moveCount = e.getInt("MoveCount");
    int offs = e.getInt("MoveData");
    int nameoffs = e.getInt("MoveNames");
    int namelen = e.getInt("MoveNameLength");
    for (int i = 1; i <= moveCount; i++) {
        const Move& m = moves_[i].value();
        int hitratio = static_cast<int>(std::floor(m.hitratio + 0.5));  // Math.round de Java
        hitratio = std::clamp(hitratio, 0, 100);
        writeFixedLengthString(m.name, nameoffs + i * namelen, namelen);
        writeBytes(offs + i * 0xC, {static_cast<uint8_t>(m.effectIndex), static_cast<uint8_t>(m.power),
                                    static_cast<uint8_t>(typeToByte(m.type)), static_cast<uint8_t>(hitratio),
                                    static_cast<uint8_t>(m.pp)});
    }
}

}  // namespace pr::gen3
