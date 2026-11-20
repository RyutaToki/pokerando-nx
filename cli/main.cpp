// Programa de pruebas en Linux para el núcleo de PokéRando NX.
//   pokerando-cli info <rom>        detecta el juego
//   pokerando-cli dump <rom>        vuelca los datos leídos (para comparar con FVX)
//   pokerando-cli resave <rom> <s>  abre y guarda sin cambios
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "romio/gen3/gen3_rom_handler.hpp"
#include "util/java_compat.hpp"

namespace {

std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("No se puede abrir " + path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

std::string itemName(const pr::gen3::Gen3RomHandler& h, const std::optional<int>& id) {
    if (!id) return "null";
    return h.items().at(*id)->name;
}

void dump(const pr::gen3::Gen3RomHandler& h) {
    std::printf("ROM %s [%s] valida=%s\n", h.romName().c_str(), h.romCode().c_str(), h.isRomValid() ? "true" : "false");
    std::printf("== ITEMS\n");
    for (const auto& it : h.items()) {
        if (!it) continue;
        std::printf("%d|%s|allowed=%d|bad=%d|tm=%d\n", it->id, it->name.c_str(), it->allowed, it->bad, it->tm);
    }
    std::printf("== ABILITIES\n");
    for (int i = 0; i <= 77; i++) std::printf("%d|%s\n", i, h.abilityName(i).c_str());
    std::printf("== MOVES perfectAccuracy=%d\n", h.perfectAccuracy());
    for (const auto& m : h.moves()) {
        if (!m) continue;
        std::printf("%d|%s|pw=%d|pp=%d|acc=%.2f|%s|%s|eff=%d|tgt=%d|pri=%d|contact=%d|sound=%d|hits=%.2f|%s",
                    m->number, m->name.c_str(), m->power, m->pp, m->hitratio, m->type ? pr::typeName(*m->type) : "null",
                    pr::moveCategoryName(m->category), m->effectIndex, m->target, m->priority, m->makesContact,
                    m->isSoundMove, m->hitCount, pr::statChangeMoveTypeName(m->statChangeMoveType));
        for (const auto& sc : m->statChanges) std::printf("|%s,%d,%.2f", pr::statChangeTypeName(sc.type), sc.stages, sc.percentChance);
        std::printf("|%s|%s|%.2f|flinch=%.2f|recoil=%d|absorb=%d|crit=%s|charge=%d|recharge=%d|trap=%d\n",
                    pr::statusMoveTypeName(m->statusMoveType), pr::statusTypeName(m->statusType), m->statusPercentChance,
                    m->flinchPercentChance, m->recoilPercent, m->absorbPercent, pr::criticalChanceName(m->criticalChance),
                    m->isChargeMove, m->isRechargeMove, m->isTrapMove);
    }
    std::printf("== SPECIES\n");
    for (const auto* pk : h.species()) {
        if (!pk) continue;
        const auto& bs = pk->baseStats();
        const auto& bi = pk->breedingInfo;
        std::printf("%d|%s|gen%d|%d/%d/%d/%d/%d/%d|bst=%d|%s/%s|catch=%d|ev=%d|%s|ab=%d,%d|items=%s,%s,%s|g=%d|egg=%s/%s/%d\n",
                    pk->number(), pk->name().c_str(), pk->generation(), bs.getHp(), bs.attack, bs.defense, bs.spatk,
                    bs.spdef, bs.speed, bs.bst(), pk->primaryType() ? pr::typeName(*pk->primaryType()) : "null",
                    pk->secondaryType() ? pr::typeName(*pk->secondaryType()) : "null", pk->catchRate,
                    pk->evYield.toWord(), pk->growthCurve ? pr::expCurveName(*pk->growthCurve) : "null",
                    pk->ability1, pk->ability2, itemName(h, pk->guaranteedHeldItem).c_str(),
                    itemName(h, pk->commonHeldItem).c_str(), itemName(h, pk->rareHeldItem).c_str(), pk->genderRatio,
                    pr::eggGroupName(bi.primary), bi.secondary ? pr::eggGroupName(*bi.secondary) : "null",
                    bi.eggCycles);
    }
}

void dumpMore(const pr::gen3::Gen3RomHandler& h) {
    std::printf("== EVOLUTIONS\n");
    for (const auto* pk : h.species()) {
        if (!pk) continue;
        for (const auto& e : pk->evolutionsFrom())
            std::printf("%d>%d|%s|%d|est=%d\n", e->from()->number(), e->to()->number(), pr::evolutionTypeName(e->type()),
                        e->extraInfo(), e->estimatedEvoLvl());
        std::printf("%d to:%zu\n", pk->number(), pk->evolutionsTo().size());
    }
    std::printf("== MOVESETS\n");
    for (const auto& [n, ml] : h.getMovesLearnt()) {
        std::printf("%d:", n);
        for (const auto& m : ml) std::printf(" %d@%d", m.move, m.level);
        std::printf("\n");
    }
    std::printf("== EGGMOVES\n");
    for (const auto& [n, ml] : h.getEggMoves()) {
        std::printf("%d:", n);
        for (int m : ml) std::printf(" %d", m);
        std::printf("\n");
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "uso: %s info|dump|resave <rom> [salida]\n", argv[0]);
        return 2;
    }
    std::string cmd = argv[1], path = argv[2];
    try {
        auto bytes = readFile(path);
        std::vector<uint8_t> header(bytes.begin(), bytes.begin() + std::min<size_t>(bytes.size(), 0x100000));
        if (!pr::gen3::Gen3RomHandler::isLoadable(header, bytes.size())) {
            std::printf("No compatible\n");
            return 1;
        }
        pr::gen3::Gen3RomHandler h;
        h.loadRom(std::move(bytes), path);
        if (cmd == "info") {
            std::printf("%s [%s] gen%d valida=%s crc=%08X\n", h.romName().c_str(), h.romCode().c_str(),
                        h.generationOfPokemon(), h.isRomValid() ? "true" : "false", h.actualCRC32());
        } else if (cmd == "dump") {
            dump(h);
            dumpMore(h);
        } else if (cmd == "resave" && argc >= 4) {
            h.saveRomFileOnly(argv[3]);
        } else if (cmd == "rewrite" && argc >= 4) {
            // reescribe datos de tamaño variable sin cambiarlos y guarda como FVX
            h.setMovesLearnt(h.getMovesLearnt());
            h.setEggMoves(h.getEggMoves());
            h.saveRom(argv[3]);
        } else {
            std::fprintf(stderr, "orden desconocida\n");
            return 2;
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    return 0;
}
