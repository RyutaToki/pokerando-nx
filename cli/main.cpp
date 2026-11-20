// Programa de pruebas en Linux para el núcleo de PokéRando NX.
//   pokerando-cli info <rom>        detecta el juego
//   pokerando-cli dump <rom>        vuelca los datos leídos (para comparar con FVX)
//   pokerando-cli resave <rom> <s>  abre y guarda sin cambios
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include "constants/item_ids.hpp"
#include "random/game_randomizer.hpp"
#include "romio/gen3/gen3_rom_handler.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "settings/settings.hpp"
#include "util/java_compat.hpp"

namespace {

std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("No se puede abrir " + path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

std::string itemName(const pr::RomHandler& h, const std::optional<int>& id) {
    if (!id) return "null";
    return h.getItems().at(*id)->name;
}

void dump(const pr::RomHandler& h) {
    std::printf("ROM %s [%s] valida=%s\n", h.romName().c_str(), h.romCode().c_str(), h.isRomValid() ? "true" : "false");
    std::printf("== ITEMS\n");
    for (const auto& it : h.getItems()) {
        if (!it) continue;
        std::printf("%d|%s|allowed=%d|bad=%d|tm=%d\n", it->id, it->name.c_str(), it->allowed, it->bad, it->tm);
    }
    std::printf("== ABILITIES\n");
    for (int i = 0; i <= h.highestAbilityIndex(); i++) std::printf("%d|%s\n", i, h.abilityName(i).c_str());
    std::printf("== MOVES perfectAccuracy=%d\n", h.getPerfectAccuracy());
    for (const auto& m : h.getMoves()) {
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
    for (const auto* pk : h.getSpecies()) {
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

void dumpPickup(pr::RomHandler& h) {
    std::printf("== PICKUP\n");
    for (const auto& p : h.getPickupItems()) {
        std::printf("%d:", p.item);
        for (int pr : p.probabilities) std::printf(" %d", pr);
        std::printf("\n");
    }
}

void dumpStatics(pr::RomHandler& h) {
    std::printf("== STATICS\n");
    std::function<void(const pr::StaticEncounter&, int)> pr1 = [&](const pr::StaticEncounter& se, int depth) {
        std::printf("%*s%d@%d egg=%d max=%d\n", depth * 2, "", se.species()->number(), se.level, se.isEgg, se.maxLevel);
        for (const auto& l : se.linkedEncounters) pr1(l, depth + 1);
    };
    for (const auto& se : h.getStaticPokemon()) pr1(se, 0);
}

void dumpMore(const pr::RomHandler& h) {
    std::printf("== PALETTES\n");
    for (const auto* pk : h.getSpecies()) {
        if (!pk) continue;
        std::printf("%d ", pk->number());
        for (auto b : pk->normalPalette->toBytes()) std::printf("%02x", b);
        std::printf(" ");
        for (auto b : pk->shinyPalette->toBytes()) std::printf("%02x", b);
        std::printf("\n");
    }
    std::printf("== EVOLUTIONS\n");
    for (const auto* pk : h.getSpecies()) {
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
    std::printf("== ENCOUNTERS\n");
    auto dumpAreas = [](const std::vector<pr::EncounterArea>& areas) {
        for (size_t i = 0; i < areas.size(); i++) {
            const auto& a = areas[i];
            std::printf("%zu|%s|rate=%d|%s|%s|pg=%d|cut=%d|ban=", i, a.displayName.c_str(), a.rate,
                        a.encounterType ? pr::encounterTypeName(*a.encounterType) : "null",
                        a.locationTag ? a.locationTag->c_str() : "null", a.postGame, a.partiallyPostGameCutoff);
            auto banned = a.getBannedSpecies().toVector();
            std::vector<int> nums;
            for (auto* b : banned) nums.push_back(b->number());
            std::sort(nums.begin(), nums.end());
            for (int n : nums) std::printf("%d,", n);
            std::printf("|");
            for (const auto& e : a.encounters) std::printf(" %d@%d-%d", e->species()->number(), e->level, e->maxLevel);
            std::printf("\n");
        }
    };
    dumpAreas(h.getEncounters(false));
    std::printf("== SORTED\n");
    for (const auto& a : h.getSortedEncounters(false)) std::printf("%s\n", a.displayName.c_str());
    std::printf("== WILDBANNED\n");
    for (auto* b : h.getBannedForWildEncounters().toVector()) std::printf("%d\n", b->number());
    std::printf("== TWEAKS %d\n", h.miscTweaksAvailable());
    std::printf("== TYPETABLE\n");
    auto tt = h.getTypeTable();
    for (auto a : tt.types()) {
        for (auto d : tt.types()) std::printf("%d", static_cast<int>(tt.getEffectiveness(a, d)));
        std::printf("\n");
    }
    std::printf("== TRAINERCLASSES\n");
    for (const auto& tc : h.getTrainerClasses()) std::printf("%d|%s\n", tc->id, tc->name.c_str());
    std::printf("== TRAINERS\n");
    for (const auto& t : h.getTrainers()) {
        std::printf("%d|%x|%d|%s|%s|multi=%d|style=%d|pt=%d|", t->index, t->offset, t->trainerClass ? t->trainerClass->id : -1,
                    t->name ? t->name->c_str() : "null", t->tag ? t->tag->c_str() : "null",
                    static_cast<int>(t->multiBattleStatus), static_cast<int>(t->currBattleStyle->style), t->poketype());
        for (const auto& tp : t->pokemon) {
            std::printf(" %d@%d/iv%d/it%d/ab%d/m%d,%d,%d,%d", tp->species()->number(), tp->level, tp->IVs,
                        tp->heldItem ? *tp->heldItem : -1, tp->abilitySlot, tp->moves[0], tp->moves[1], tp->moves[2], tp->moves[3]);
        }
        std::printf("\n");
    }
    std::printf("== E4 ");
    for (int i : h.getEliteFourTrainers(false)) std::printf("%d,", i);
    std::printf("\n== STARTERS");
    for (auto* st : h.getStarters()) std::printf(" %d", st->number());
    for (auto& it : h.getStarterHeldItems()) std::printf(" item=%d", it ? *it : -1);
    std::printf("\n== TMS");
    for (int m : h.getTMMoves()) std::printf(" %d", m);
    std::printf("\n== HMS");
    for (int m : h.getHMMoves()) std::printf(" %d", m);
    std::printf("\n== TUTORS");
    for (int m : h.getMoveTutorMoves()) std::printf(" %d", m);
    std::printf("\n== TMCOMPAT\n");
    for (auto& [n, f] : h.getTMHMCompatibility()) {
        std::printf("%d:", n);
        for (bool b : f) std::printf("%d", b ? 1 : 0);
        std::printf("\n");
    }
    std::printf("== TUTORCOMPAT\n");
    for (auto& [n, f] : h.getMoveTutorCompatibility()) {
        std::printf("%d:", n);
        for (bool b : f) std::printf("%d", b ? 1 : 0);
        std::printf("\n");
    }
    auto pset = [](const char* n, const pr::ItemSet& s) {
        std::printf("== %s", n);
        s.forEach([](const int& i) { std::printf(" %d", i); });
        std::printf("\n");
    };
    std::printf("\n");
    pset("EVOITEMS", h.getEvolutionItems());
    pset("OPSHOP", h.getOPShopItems());
    pset("CONSUMABLE", h.getAllConsumableHeldItems());
    pset("REQTMS", h.getRequiredFieldTMs());
    std::printf("== FIELD");
    for (int i : h.getFieldItems()) std::printf(" %d", i);
    std::printf("\n== SHOPS\n");
    for (const auto& sh : h.getShops()) {
        std::printf("%s|main=%d|special=%d|", sh.name.c_str(), sh.isMainGame, sh.isSpecialShop);
        for (int i : sh.items) std::printf(" %d", i);
        std::printf("\n");
    }
    std::printf("== PRICES");
    for (int p : h.getShopPrices()) std::printf(" %d", p);
    std::printf("\n== TRADES\n");
    for (const auto& t : h.getInGameTrades()) {
        std::printf("%d>%d|%s|%s|ot=%d|item=%d|iv=", t.requestedSpecies->number(), t.givenSpecies()->number(),
                    t.nickname.c_str(), t.otName ? t.otName->c_str() : "null", t.otId, t.heldItem ? *t.heldItem : -1);
        for (int v : t.ivs) std::printf("%d,", v);
        std::printf("\n");
    }
    std::printf("== SENSIBLE\n");
    for (size_t ti = 0; ti < h.getTrainers().size(); ti += 37) {
        for (const auto& tp : h.getTrainers()[ti]->pokemon) {
            for (int co = 0; co < 2; co++) {
                std::printf("%zu:%d:%d", ti, tp->species()->number(), co);
                for (auto& i : h.getSensibleHeldItemsFor(*tp, co == 1, tp->moves)) std::printf(" %d", i ? *i : -1);
                std::printf("\n");
            }
        }
    }
    std::printf("\n== EGGMOVES\n");
    for (const auto& [n, ml] : h.getEggMoves()) {
        std::printf("%d:", n);
        for (int m : ml) std::printf(" %d", m);
        std::printf("\n");
    }
}

// reescribe datos de tamaño variable sin cambiarlos (mismas llamadas y en el mismo orden que tests/java/Dump.java)
void rewriteAll(pr::RomHandler& h) {
    h.setMovesLearnt(h.getMovesLearnt());
    h.setEggMoves(h.getEggMoves());
    h.setStarters(h.getStarters());
    h.setStarterHeldItems(h.getStarterHeldItems());
    h.setStaticPokemon(h.getStaticPokemon());
    h.setTMMoves(h.getTMMoves());
    h.setTMHMCompatibility(h.getTMHMCompatibility());
    h.setMoveTutorMoves(h.getMoveTutorMoves());
    h.setMoveTutorCompatibility(h.getMoveTutorCompatibility());
    h.setFieldItems(h.getFieldItems());
    h.setShops(h.getShops());
    h.setShopPrices(h.getShopPrices());
    h.setPickupItems(h.getPickupItems());
    h.setInGameTrades(h.getInGameTrades());
    // ajustes: mismas llamadas y en el mismo orden que tests/java/Dump.java
    h.setTypeTable(h.getTypeTable());
    h.removeImpossibleEvolutions(true, false);
    h.makeEvolutionsEasier(true, false);
    int avail = h.miscTweaksAvailable();
    for (const auto& t : pr::misctweak::allTweaks) {
        if ((avail & t.value) && t.value != pr::misctweak::BAN_LUCKY_EGG.value) h.applyMiscTweak(t);
    }
    h.setIntroPokemon(h.getSpecies()[25]);
    h.setCatchingTutorial(h.getSpecies()[263], h.getSpecies()[258]);
    if (h.generationOfPokemon() != 4) h.setPCPotionItem(pr::items::superPotion);  // Gen 4 no lo tiene
    h.enableGuaranteedPokemonCatching();
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "uso: %s info|dump|resave <rom> [salida]\n", argv[0]);
        return 2;
    }
    std::string cmd = argv[1], path = argv[2];
    try {
        // ROM de DS (4.ª generación)
        {
            std::ifstream f(path, std::ios::binary);
            char hdr[0x20] = {};
            f.read(hdr, sizeof hdr);
            std::string code(hdr + 0x0C, 4);
            if (pr::gen4::Gen4RomHandler::isLoadable(code, static_cast<int8_t>(hdr[0x1E]))) {
                pr::gen4::Gen4RomHandler h;
                h.loadRom(path);
                if (cmd == "info") {
                    std::printf("%s [%s] gen%d valida=%s\n", h.romName().c_str(), h.romCode().c_str(), h.generationOfPokemon(),
                                h.isRomValid() ? "true" : "false");
                } else if (cmd == "dump") {
                    dump(h);
                    // se vuelca hasta la primera sección que aún no esté portada
                    try {
                        dumpMore(h);
                        dumpStatics(h);
                        dumpPickup(h);
                    } catch (const std::logic_error& e) {
                        std::fflush(stdout);
                        std::fprintf(stderr, "%s\n", e.what());
                    }
                } else if (cmd == "resave" && argc >= 4) {
                    h.saveRomTo(argv[3]);
                } else if (cmd == "rewrite" && argc >= 4) {
                    rewriteAll(h);
                    h.saveRomTo(argv[3]);
                } else {
                    std::fprintf(stderr, "orden no disponible todavía para Gen 4\n");
                    return 2;
                }
                return 0;
            }
        }
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
            dumpStatics(h);
            dumpPickup(h);
        } else if (cmd == "resave" && argc >= 4) {
            h.saveRomFileOnly(argv[3]);
        } else if (cmd == "randomize" && argc >= 6) {
            // randomize <rom> <settings string> <semilla> <salida> [registro.txt]
            pr::Settings settings = pr::Settings::fromString(argv[3]);
            settings.tweakForRom(h);
            pr::GameRandomizer gr(settings, h);
            if (argc >= 7) {
                std::ofstream log(argv[6], std::ios::binary);
                gr.randomize(argv[5], std::stoll(argv[4]), &log);
            } else {
                gr.randomize(argv[5], std::stoll(argv[4]));
            }
        } else if (cmd == "rewrite" && argc >= 4) {
            rewriteAll(h);
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
