// Generado por tools/translate_move_synergy.py a partir de MoveSynergy.java de FVX. No editar a mano.
#include "gamedata/move_synergy.hpp"

#include <algorithm>

#include "constants/ability_ids.hpp"
#include "constants/move_ids.hpp"

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

namespace pr::MoveSynergy {

namespace {
std::vector<Move*> pickIn(const std::vector<Move*>& moveList, const std::vector<int>& nums) {
    std::vector<Move*> r;
    for (Move* mv : moveList)
        if (std::find(nums.begin(), nums.end(), mv->number) != nums.end() && std::find(r.begin(), r.end(), mv) == r.end())
            r.push_back(mv);
    return r;
}
}  // namespace

std::vector<Move*> getSoftMoveSynergy(const Move* mv1, const std::vector<Move*>& moveList, const TypeTable& typeTable) {
    std::vector<int> synergisticMoves;
    if (mv1->category != MoveCategory::STATUS) {
        // en Java, mv1.type == null lanzaría una excepción; en Gen 3 todos los ataques con daño tienen tipo
        std::vector<Type> nve = typeTable.notVeryEffectiveWhenAttacking(*mv1->type);
        auto imm = typeTable.immuneWhenAttacking(*mv1->type);
        nve.insert(nve.end(), imm.begin(), imm.end());
        for (Type nveType : nve) {
            std::vector<Type> se;
            for (auto& [t, e] : typeTable.against(nveType, std::nullopt))
                if (e == Effectiveness::DOUBLE) se.push_back(t);
            for (const Move* mv : moveList)
                if (mv->category != MoveCategory::STATUS && mv->type && std::find(se.begin(), se.end(), *mv->type) != se.end())
                    synergisticMoves.push_back(mv->number);
        }
    }
    switch (mv1->number) {
        case moves::swordsDance:
            for (const Move* mv : moveList) if (mv->category == MoveCategory::PHYSICAL) synergisticMoves.push_back(mv->number);
            break;
        case moves::nastyPlot:
        case moves::tailGlow:
            for (const Move* mv : moveList) if (mv->category == MoveCategory::SPECIAL) synergisticMoves.push_back(mv->number);
            break;
    }
    return pickIn(moveList, synergisticMoves);
}


    std::vector<Move*> getSoftAbilityMoveSynergy(int ability, const std::vector<Move*>& moveList, std::optional<Type> pkType1, std::optional<Type> pkType2) {
        std::vector<int> synergisticMoves;

        switch(ability) {
            case abilities::drizzle:
            case abilities::primordialSea:
                for (const Move* mv : moveList) if (mv->type == Type::WATER && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::drought:
            case abilities::desolateLand:
                for (const Move* mv : moveList) if (mv->type == Type::FIRE && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::refrigerate:
                if (pkType1 == Type::ICE || pkType2 == Type::ICE) {
                    for (const Move* mv : moveList) if (mv->type == Type::NORMAL && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                }
                break;
            case abilities::galeWings:
                for (const Move* mv : moveList) if (mv->type == Type::FLYING) synergisticMoves.push_back(mv->number);
                break;
            case abilities::pixilate:
                if (pkType1 == Type::FAIRY || pkType2 == Type::FAIRY) {
                    for (const Move* mv : moveList) if (mv->type == Type::NORMAL && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                }
                break;
            case abilities::aerilate:
                if (pkType1 == Type::FLYING || pkType2 == Type::FLYING) {
                    for (const Move* mv : moveList) if (mv->type == Type::NORMAL && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                }
                break;
            case abilities::darkAura:
                for (const Move* mv : moveList) if (mv->type == Type::DARK && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::fairyAura:
                for (const Move* mv : moveList) if (mv->type == Type::FAIRY && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::steelworker:
            case abilities::steelySpirit:
                for (const Move* mv : moveList) if (mv->type == Type::STEEL && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::galvanize:
                if (pkType1 == Type::ELECTRIC || pkType2 == Type::ELECTRIC) {
                    for (const Move* mv : moveList) if (mv->type == Type::NORMAL && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                }
                break;
            case abilities::electricSurge:
            case abilities::transistor:
                for (const Move* mv : moveList) if (mv->type == Type::ELECTRIC && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::psychicSurge:
                for (const Move* mv : moveList) if (mv->type == Type::PSYCHIC && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::grassySurge:
                for (const Move* mv : moveList) if (mv->type == Type::GRASS && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
            case abilities::dragonsMaw:
                for (const Move* mv : moveList) if (mv->type == Type::DRAGON && mv->category != MoveCategory::STATUS) synergisticMoves.push_back(mv->number);
                break;
        }

        return pickIn(moveList, synergisticMoves);
    }

    std::vector<Move*> getSoftAbilityMoveAntiSynergy(int ability, const std::vector<Move*>& moveList) {
        std::vector<int> antiSynergisticMoves;

        switch (ability) {
            case abilities::drizzle:
                for (const Move* mv : moveList) if (mv->type == Type::FIRE && mv->category != MoveCategory::STATUS) antiSynergisticMoves.push_back(mv->number);
                break;
            case abilities::drought:
                for (const Move* mv : moveList) if (mv->type == Type::WATER && mv->category != MoveCategory::STATUS) antiSynergisticMoves.push_back(mv->number);
                break;
            case abilities::mistySurge:
                for (const Move* mv : moveList) if (mv->type == Type::DRAGON && mv->category != MoveCategory::STATUS) antiSynergisticMoves.push_back(mv->number);
                break;
        }

        return pickIn(moveList, antiSynergisticMoves);
    }

    std::vector<Move*> getHardAbilityMoveSynergy(int ability, std::optional<Type> pkType1, std::optional<Type> pkType2, const std::vector<Move*>& moveList,
                                                       int generation, int perfectAccuracy) {
        std::vector<int> synergisticMoves;

        switch(ability) {
            case abilities::drizzle:
            case abilities::primordialSea:
                synergisticMoves.push_back(moves::thunder);
                synergisticMoves.push_back(moves::hurricane);
                if (pkType1 == Type::WATER || pkType2 == Type::WATER) {
                    synergisticMoves.push_back(moves::weatherBall);
                }
                break;
            case abilities::speedBoost:
                synergisticMoves.push_back(moves::batonPass);
                synergisticMoves.push_back(moves::storedPower);
                synergisticMoves.push_back(moves::powerTrip);
                break;
            case abilities::sturdy:
                if (generation >= 5) {
                    synergisticMoves.push_back(moves::endeavor);
                    synergisticMoves.push_back(moves::counter);
                    synergisticMoves.push_back(moves::mirrorCoat);
                    synergisticMoves.push_back(moves::flail);
                    synergisticMoves.push_back(moves::reversal);
                }
                break;
            case abilities::sandVeil:
            case abilities::sandRush:
                synergisticMoves.push_back(moves::sandstorm);
                break;
            case abilities::staticTheAbilityNotTheKeyword:
                synergisticMoves.push_back(moves::smellingSalts);
                synergisticMoves.push_back(moves::hex);
                break;
            case abilities::compoundEyes:
                for (const Move* mv : moveList) if (mv->hitratio > 0 && mv->hitratio <= 80) synergisticMoves.push_back(mv->number);
                break;
            case abilities::ownTempo:
            case abilities::tangledFeet:
                synergisticMoves.push_back(moves::petalDance);
                synergisticMoves.push_back(moves::thrash);
                synergisticMoves.push_back(moves::outrage);
                break;
            case abilities::shadowTag:
            case abilities::arenaTrap:
                synergisticMoves.push_back(moves::perishSong);
                break;
            case abilities::poisonPoint:
                synergisticMoves.push_back(moves::venoshock);
                [[fallthrough]];
            case abilities::effectSpore:
            case abilities::flameBody:
                synergisticMoves.push_back(moves::hex);
                break;
            case abilities::sereneGrace:
                for (const Move* mv : moveList) if (((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET || mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER) && mv->statChanges[0].percentChance < 100) || (mv->statusMoveType == StatusMoveType::DAMAGE && mv->statusPercentChance < 100) || mv->flinchPercentChance > 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::swiftSwim:
            case abilities::rainDish:
            case abilities::drySkin:
            case abilities::hydration:
                synergisticMoves.push_back(moves::rainDance);
                break;
            case abilities::chlorophyll:
            case abilities::harvest:
            case abilities::leafGuard:
                synergisticMoves.push_back(moves::sunnyDay);
                break;
            case abilities::soundproof:
                synergisticMoves.push_back(moves::perishSong);
                break;
            case abilities::sandStream:
                if (pkType1 == Type::ROCK || pkType2 == Type::ROCK) {
                    synergisticMoves.push_back(moves::weatherBall);
                }
                break;
            case abilities::earlyBird:
            case abilities::shedSkin:
                synergisticMoves.push_back(moves::rest);
                break;
            case abilities::truant:
                synergisticMoves.push_back(moves::transform);
                break;
            case abilities::hustle:
                for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER || mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->hasSpecificStatChange(StatChangeType::ACCURACY, true)) synergisticMoves.push_back(mv->number);
                for (const Move* mv : moveList) if (mv->category == MoveCategory::PHYSICAL && mv->hitratio == perfectAccuracy) synergisticMoves.push_back(mv->number);
                break;
            case abilities::guts:
                synergisticMoves.push_back(moves::facade);
                break;
            case abilities::rockHead:
                for (const Move* mv : moveList) if (mv->recoilPercent > 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::drought:
            case abilities::desolateLand:
                synergisticMoves.push_back(moves::solarBeam);
                synergisticMoves.push_back(moves::solarBlade);
                synergisticMoves.push_back(moves::morningSun);
                synergisticMoves.push_back(moves::synthesis);
                synergisticMoves.push_back(moves::moonlight);
                if (generation >= 5) {
                    synergisticMoves.push_back(moves::growth);
                }
                if (pkType1 == Type::FIRE || pkType2 == Type::FIRE) {
                    synergisticMoves.push_back(moves::weatherBall);
                }
                break;
            case abilities::ironFist:
                for (const Move* mv : moveList) if (mv->isPunchMove) synergisticMoves.push_back(mv->number);
                break;
            case abilities::snowCloak:
            case abilities::iceBody:
            case abilities::slushRush:
                synergisticMoves.push_back(moves::hail);
                break;
            case abilities::unburden:
                synergisticMoves.push_back(moves::fling);
                synergisticMoves.push_back(moves::acrobatics);
                break;
            case abilities::simple:
                for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER || mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->statChanges[0].stages > 0) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::acupressure);
                break;
            case abilities::adaptability:
                for (const Move* mv : moveList) if (mv->category != MoveCategory::STATUS && (mv->type == pkType1 || mv->type == pkType2)) synergisticMoves.push_back(mv->number);
                break;
            case abilities::skillLink:
                for (const Move* mv : moveList) if (mv->hitCount >= 3) synergisticMoves.push_back(mv->number);
                break;
            case abilities::sniper:
                for (const Move* mv : moveList) if (mv->criticalChance == CriticalChance::INCREASED || mv->criticalChance == CriticalChance::GUARANTEED) synergisticMoves.push_back(mv->number);
                break;
            case abilities::magicGuard:
                for (const Move* mv : moveList) if (mv->recoilPercent > 0) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::mindBlown);
                break;
            case abilities::stall:
                synergisticMoves.push_back(moves::metalBurst);
                synergisticMoves.push_back(moves::payback);
                break;
            case abilities::superLuck:
                for (const Move* mv : moveList) if (mv->criticalChance == CriticalChance::INCREASED) synergisticMoves.push_back(mv->number);
                break;
            case abilities::analytic:
                for (const Move* mv : moveList) if (mv->power > 0 && mv->priority < 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::noGuard:
                for (const Move* mv : moveList) if (mv->hitratio > 0 && mv->hitratio <= 70) synergisticMoves.push_back(mv->number);
                break;
            case abilities::technician:
                for (const Move* mv : moveList) if ((mv->power >= 40 && mv->power <= 60) || mv->hitCount > 1) synergisticMoves.push_back(mv->number);
                break;
            case abilities::slowStart:
                synergisticMoves.push_back(moves::transform);
                synergisticMoves.push_back(moves::protect);
                synergisticMoves.push_back(moves::detect);
                synergisticMoves.push_back(moves::kingsShield);
                synergisticMoves.push_back(moves::banefulBunker);
                synergisticMoves.push_back(moves::fly);
                synergisticMoves.push_back(moves::dig);
                synergisticMoves.push_back(moves::bounce);
                synergisticMoves.push_back(moves::dive);
                break;
            case abilities::snowWarning:
                synergisticMoves.push_back(moves::auroraVeil);
                synergisticMoves.push_back(moves::blizzard);
                if (pkType1 == Type::ICE || pkType2 == Type::ICE) {
                    synergisticMoves.push_back(moves::weatherBall);
                }
                break;
            case abilities::reckless:
                for (const Move* mv : moveList) if (mv->recoilPercent > 0) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::jumpKick);
                synergisticMoves.push_back(moves::highJumpKick);
                break;
            case abilities::badDreams:
                for (const Move* mv : moveList) if (mv->statusType == StatusType::SLEEP) synergisticMoves.push_back(mv->number);
                break;
            case abilities::sheerForce:
                for (const Move* mv : moveList) if (mv->statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET || (mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER && mv->statChanges[0].stages > 0) || mv->statusMoveType == StatusMoveType::DAMAGE || mv->flinchPercentChance > 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::contrary:
                for (const Move* mv : moveList) if (mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER && mv->statChanges[0].stages < 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::heavyMetal:
                synergisticMoves.push_back(moves::heatCrash);
                synergisticMoves.push_back(moves::heavySlam);
                break;
            case abilities::moody:
                synergisticMoves.push_back(moves::storedPower);
                synergisticMoves.push_back(moves::powerTrip);
                break;
            case abilities::poisonTouch:
            case abilities::toughClaws:
                for (const Move* mv : moveList) if (mv->makesContact) synergisticMoves.push_back(mv->number);
                break;
            case abilities::regenerator:
                synergisticMoves.push_back(moves::uTurn);
                synergisticMoves.push_back(moves::voltSwitch);
                synergisticMoves.push_back(moves::partingShot);
                break;
            case abilities::prankster:
                for (const Move* mv : moveList) if (mv->statusMoveType == StatusMoveType::NO_DAMAGE) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::destinyBond);
                synergisticMoves.push_back(moves::encore);
                synergisticMoves.push_back(moves::reflect);
                synergisticMoves.push_back(moves::lightScreen);
                synergisticMoves.push_back(moves::grudge);
                synergisticMoves.push_back(moves::painSplit);
                synergisticMoves.push_back(moves::substitute);
                break;
            case abilities::strongJaw:
                synergisticMoves.push_back(moves::bite);
                synergisticMoves.push_back(moves::crunch);
                synergisticMoves.push_back(moves::fireFang);
                synergisticMoves.push_back(moves::fishiousRend);
                synergisticMoves.push_back(moves::hyperFang);
                synergisticMoves.push_back(moves::iceFang);
                synergisticMoves.push_back(moves::jawLock);
                synergisticMoves.push_back(moves::poisonFang);
                synergisticMoves.push_back(moves::psychicFangs);
                synergisticMoves.push_back(moves::thunderFang);
                break;
            case abilities::megaLauncher:
                synergisticMoves.push_back(moves::auraSphere);
                synergisticMoves.push_back(moves::darkPulse);
                synergisticMoves.push_back(moves::dragonPulse);
                synergisticMoves.push_back(moves::originPulse);
                synergisticMoves.push_back(moves::terrainPulse);
                synergisticMoves.push_back(moves::waterPulse);
                break;
            case abilities::wimpOut:
            case abilities::emergencyExit:
                synergisticMoves.push_back(moves::fakeOut);
                synergisticMoves.push_back(moves::firstImpression);
                break;
            case abilities::merciless:
                for (const Move* mv : moveList) if (mv->statusType == StatusType::POISON || mv->statusType == StatusType::TOXIC_POISON) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::banefulBunker);
                break;
            case abilities::liquidVoice:
                if (pkType1 == Type::WATER || pkType2 == Type::WATER) {
                    for (const Move* mv : moveList) if (mv->isSoundMove && mv->power > 0) synergisticMoves.push_back(mv->number);
                }
                break;
            case abilities::triage:
                for (const Move* mv : moveList) if (mv->absorbPercent > 0) synergisticMoves.push_back(mv->number);
                break;
            case abilities::surgeSurfer:
                synergisticMoves.push_back(moves::electricTerrain);
                break;
            case abilities::corrosion:
                for (const Move* mv : moveList) if (mv->category == MoveCategory::STATUS && (mv->statusType == StatusType::POISON || mv->statusType == StatusType::TOXIC_POISON)) synergisticMoves.push_back(mv->number);
                break;
        }

        return pickIn(moveList, synergisticMoves);
    }

    std::vector<Move*> getHardAbilityMoveAntiSynergy(int ability, const std::vector<Move*>& moveList) {
        std::vector<int> antiSynergisticMoves;

        switch(ability) {
            case abilities::primordialSea:
                for (const Move* mv : moveList) if (mv->type == Type::FIRE && mv->category != MoveCategory::STATUS) antiSynergisticMoves.push_back(mv->number);
                [[fallthrough]];
            case abilities::drizzle:
            case abilities::sandStream:
            case abilities::snowWarning:
                antiSynergisticMoves.push_back(moves::solarBeam);
                antiSynergisticMoves.push_back(moves::solarBlade);
                antiSynergisticMoves.push_back(moves::morningSun);
                antiSynergisticMoves.push_back(moves::synthesis);
                antiSynergisticMoves.push_back(moves::moonlight);
                antiSynergisticMoves.push_back(moves::rainDance);
                antiSynergisticMoves.push_back(moves::sunnyDay);
                antiSynergisticMoves.push_back(moves::hail);
                antiSynergisticMoves.push_back(moves::sandstorm);
                break;
            case abilities::speedBoost:
                for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->statChanges[0].type == StatChangeType::SPEED && mv->statChanges[0].stages > 0) antiSynergisticMoves.push_back(mv->number);
                antiSynergisticMoves.push_back(moves::psychUp);
                antiSynergisticMoves.push_back(moves::haze);
                break;
            case abilities::desolateLand:
                for (const Move* mv : moveList) if (mv->type == Type::WATER && mv->category != MoveCategory::STATUS) antiSynergisticMoves.push_back(mv->number);
                [[fallthrough]];
            case abilities::drought:
                antiSynergisticMoves.push_back(moves::thunder);
                antiSynergisticMoves.push_back(moves::hurricane);
                antiSynergisticMoves.push_back(moves::rainDance);
                antiSynergisticMoves.push_back(moves::sunnyDay);
                antiSynergisticMoves.push_back(moves::hail);
                antiSynergisticMoves.push_back(moves::sandstorm);
                break;
            case abilities::noGuard:
                antiSynergisticMoves.push_back(moves::lockOn);
                antiSynergisticMoves.push_back(moves::mindReader);
                for (const Move* mv : moveList) if (mv->hasSpecificStatChange(StatChangeType::ACCURACY, true) || mv->hasSpecificStatChange(StatChangeType::EVASION, true) || mv->hasSpecificStatChange(StatChangeType::EVASION, false)) antiSynergisticMoves.push_back(mv->number);
                break;
            case abilities::damp:
                antiSynergisticMoves.push_back(moves::selfDestruct);
                antiSynergisticMoves.push_back(moves::explosion);
                antiSynergisticMoves.push_back(moves::mindBlown);
                antiSynergisticMoves.push_back(moves::mistyExplosion);
                break;
            case abilities::insomnia:
            case abilities::vitalSpirit:
            case abilities::comatose:
            case abilities::sweetVeil:
                antiSynergisticMoves.push_back(moves::rest);
                break;
            case abilities::airLock:
            case abilities::cloudNine:
            case abilities::deltaStream:
                antiSynergisticMoves.push_back(moves::rainDance);
                antiSynergisticMoves.push_back(moves::sunnyDay);
                antiSynergisticMoves.push_back(moves::sandstorm);
                antiSynergisticMoves.push_back(moves::hail);
                break;
            case abilities::simple:
                for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER || mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->statChanges[0].stages < 0) antiSynergisticMoves.push_back(mv->number);
                break;
            case abilities::contrary:
                for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER || mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->statChanges[0].stages > 0) antiSynergisticMoves.push_back(mv->number);
                antiSynergisticMoves.push_back(moves::shellSmash);
                break;
            case abilities::lightMetal:
                antiSynergisticMoves.push_back(moves::heatCrash);
                antiSynergisticMoves.push_back(moves::heavySlam);
                break;
            case abilities::electricSurge:
                for (const Move* mv : moveList) if ((mv->category == MoveCategory::STATUS && mv->statusType == StatusType::SLEEP)) antiSynergisticMoves.push_back(mv->number);
                antiSynergisticMoves.push_back(moves::rest);
                break;
            case abilities::psychicSurge:
                for (const Move* mv : moveList) if ((mv->priority > 0)) antiSynergisticMoves.push_back(mv->number);
                break;
            case abilities::mistySurge:
                for (const Move* mv : moveList) if ((mv->category == MoveCategory::STATUS && (mv->statusType == StatusType::BURN || mv->statusType == StatusType::FREEZE || mv->statusType == StatusType::PARALYZE || mv->statusType == StatusType::SLEEP || mv->statusType == StatusType::POISON || mv->statusType == StatusType::TOXIC_POISON))) antiSynergisticMoves.push_back(mv->number);
                antiSynergisticMoves.push_back(moves::rest);
                break;
            case abilities::grassySurge:
                antiSynergisticMoves.push_back(moves::earthquake);
                antiSynergisticMoves.push_back(moves::magnitude);
                antiSynergisticMoves.push_back(moves::bulldoze);
                break;


        }

        return pickIn(moveList, antiSynergisticMoves);
    }

    std::vector<Move*> getStatMoveSynergy(const BaseStats& bs, const std::vector<Move*>& moveList) {
        std::vector<int> synergisticMoves;

        if ((double) bs.getHp() / (double)bs.bst() < 1.0/8) {
            synergisticMoves.push_back(moves::painSplit);
            synergisticMoves.push_back(moves::endeavor);
        }

        if ((double) bs.getHp() / (double)bs.bst() >= 1.0/4) {
            synergisticMoves.push_back(moves::waterSpout);
            synergisticMoves.push_back(moves::eruption);
            synergisticMoves.push_back(moves::counter);
            synergisticMoves.push_back(moves::mirrorCoat);
        }

        if (bs.attack * 2 < bs.defense) {
            synergisticMoves.push_back(moves::powerTrick);
        }

        if ((double) bs.speed / (double)bs.bst() < 1.0/8) {
            synergisticMoves.push_back(moves::gyroBall);
        }

        if ((double) bs.speed / (double)bs.bst() >= 1.0/4) {
            synergisticMoves.push_back(moves::electroBall);
        }

        if (true) {
            if ((double) (bs.attack + bs.spatk) / (double) bs.bst() < 1.0 / 4) {
                synergisticMoves.push_back(moves::powerSplit);
            }

            if ((double) (bs.defense + bs.spdef) / (double) bs.bst() < 1.0 / 4) {
                synergisticMoves.push_back(moves::guardSplit);
            }
        }

        return pickIn(moveList, synergisticMoves);
    }

    std::vector<Move*> getStatMoveAntiSynergy(const BaseStats& bs, const std::vector<Move*>& moveList) {
        std::vector<int> antiSynergisticMoves;

        if ((double) bs.getHp() / (double)bs.bst() >= 1.0/4) {
            antiSynergisticMoves.push_back(moves::painSplit);
            antiSynergisticMoves.push_back(moves::endeavor);
        }

        if (bs.defense * 2 < bs.attack) {
            antiSynergisticMoves.push_back(moves::powerTrick);
        }

        if ((double) bs.speed / (double)bs.bst() >= 1.0/4) {
            antiSynergisticMoves.push_back(moves::gyroBall);
        }

        if ((double) bs.speed / (double)bs.bst() < 1.0/8) {
            antiSynergisticMoves.push_back(moves::electroBall);
        }

        if (true) {
            if ((double)(bs.attack + bs.spatk) / (double)bs.bst() >= 1.0/3) {
                antiSynergisticMoves.push_back(moves::powerSplit);
            }

            if ((double)(bs.defense + bs.spdef) / (double)bs.bst() >= 1.0/3) {
                antiSynergisticMoves.push_back(moves::guardSplit);
            }
        }

        return pickIn(moveList, antiSynergisticMoves);
    }

    std::vector<Move*> getMoveSynergy(const Move* mv1, const std::vector<Move*>& moveList, int generation) {
        std::vector<int> synergisticMoves;

        if ((mv1->statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET &&
                mv1->hasSpecificStatChange(StatChangeType::SPEED, false)) ||
                ((mv1->statChangeMoveType == StatChangeMoveType::DAMAGE_USER ||
                        mv1->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) &&
                        mv1->hasSpecificStatChange(StatChangeType::SPEED, true))) {
            for (const Move* mv : moveList) if (mv->flinchPercentChance > 0 && mv->priority == 0) synergisticMoves.push_back(mv->number);
        }

        if (mv1->flinchPercentChance > 0 && mv1->priority == 0) {
            for (const Move* mv : moveList) if ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_TARGET && mv->hasSpecificStatChange(StatChangeType::SPEED, false)) || ((mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER || mv->statChangeMoveType == StatChangeMoveType::NO_DAMAGE_USER) && mv->hasSpecificStatChange(StatChangeType::SPEED, true))) synergisticMoves.push_back(mv->number);
        }

        if (mv1->statChanges[0].stages >= 2 || mv1->statChanges[1].type != StatChangeType::NONE) {
            synergisticMoves.push_back(moves::batonPass);
            synergisticMoves.push_back(moves::storedPower);
            synergisticMoves.push_back(moves::powerTrip);
        }

        if (mv1->statusType == StatusType::SLEEP) {
            synergisticMoves.push_back(moves::dreamEater);
            synergisticMoves.push_back(moves::nightmare);
            synergisticMoves.push_back(moves::hex);
        }

        switch(mv1->number) {
            case moves::toxic:
                synergisticMoves.push_back(moves::protect);
                synergisticMoves.push_back(moves::detect);
                synergisticMoves.push_back(moves::kingsShield);
                synergisticMoves.push_back(moves::dig);
                synergisticMoves.push_back(moves::fly);
                synergisticMoves.push_back(moves::bounce);
                synergisticMoves.push_back(moves::dive);
                for (const Move* mv : moveList) if ((mv->isTrapMove)) synergisticMoves.push_back(mv->number);
                [[fallthrough]];
            case moves::poisonPowder:
            case moves::poisonGas:
            case moves::banefulBunker:
            case moves::toxicThread:
                synergisticMoves.push_back(moves::venoshock);
                synergisticMoves.push_back(moves::hex);
                break;
            case moves::venoshock:
                for (const Move* mv : moveList) if (mv->category == MoveCategory::STATUS && (mv->statusType == StatusType::POISON || mv->statusType == StatusType::TOXIC_POISON)) synergisticMoves.push_back(mv->number);
                break;
            case moves::protect:
            case moves::detect:
            case moves::kingsShield:
                synergisticMoves.push_back(moves::toxic);
                synergisticMoves.push_back(moves::leechSeed);
                synergisticMoves.push_back(moves::willOWisp);
                break;
            case moves::batonPass:
                for (const Move* mv : moveList) if (mv->statChanges[0].stages >= 2 || mv->statChanges[1].type != StatChangeType::NONE) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::shellSmash);
                break;
            case moves::willOWisp:
                synergisticMoves.push_back(moves::hex);
                break;
            case moves::lockOn:
            case moves::mindReader:
                for (const Move* mv : moveList) if ((mv->hitratio <= 50)) synergisticMoves.push_back(mv->number);
                break;
            case moves::sunnyDay:
                synergisticMoves.push_back(moves::solarBlade);
                synergisticMoves.push_back(moves::solarBeam);
                break;
            case moves::rainDance:
                synergisticMoves.push_back(moves::thunder);
                synergisticMoves.push_back(moves::hurricane);
                break;
            case moves::powerSwap:
                for (const Move* mv : moveList) if (mv->statChangeMoveType == StatChangeMoveType::DAMAGE_USER && (mv->hasSpecificStatChange(StatChangeType::ATTACK, false) || mv->hasSpecificStatChange(StatChangeType::SPECIAL_ATTACK, false))) synergisticMoves.push_back(mv->number);
                break;
            case moves::endure:
                synergisticMoves.push_back(moves::reversal);
                synergisticMoves.push_back(moves::flail);
                synergisticMoves.push_back(moves::endeavor);
                break;
            case moves::endeavor:
                for (const Move* mv : moveList) if (mv->category != MoveCategory::STATUS && mv->priority > 0) synergisticMoves.push_back(mv->number);
                break;
            case moves::thunderWave:
            case moves::glare:
            case moves::stunSpore:
                for (const Move* mv : moveList) if (mv->flinchPercentChance > 0) synergisticMoves.push_back(mv->number);
                for (const Move* mv : moveList) if (mv->category == MoveCategory::STATUS && mv->statusType == StatusType::CONFUSION) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::hex);
                break;
            case moves::hail:
                synergisticMoves.push_back(moves::blizzard);
                synergisticMoves.push_back(moves::auroraVeil);
                break;
            case moves::stockpile:
                synergisticMoves.push_back(moves::spitUp);
                synergisticMoves.push_back(moves::swallow);
                break;
            case moves::spitUp:
            case moves::swallow:
                synergisticMoves.push_back(moves::stockpile);
                break;
            case moves::leechSeed:
            case moves::perishSong:
                for (const Move* mv : moveList) if ((mv->isTrapMove)) synergisticMoves.push_back(mv->number);
                break;
            case moves::spikes:
            case moves::stealthRock:
            case moves::toxicSpikes:
                synergisticMoves.push_back(moves::roar);
                synergisticMoves.push_back(moves::whirlwind);
                synergisticMoves.push_back(moves::dragonTail);
                synergisticMoves.push_back(moves::circleThrow);
                break;
            case moves::rest:
                synergisticMoves.push_back(moves::sleepTalk);
                break;
            case moves::focusEnergy:
                for (const Move* mv : moveList) if (mv->criticalChance == CriticalChance::INCREASED) synergisticMoves.push_back(mv->number);
                break;
            case moves::focusPunch:
            case moves::dreamEater:
            case moves::nightmare:
                for (const Move* mv : moveList) if (mv->statusMoveType == StatusMoveType::NO_DAMAGE && mv->statusType == StatusType::SLEEP) synergisticMoves.push_back(mv->number);
                break;
            case moves::torment:
                synergisticMoves.push_back(moves::encore);
                break;
            case moves::encore:
                synergisticMoves.push_back(moves::torment);
                break;
            case moves::hex:
                for (const Move* mv : moveList) if (mv->statusMoveType == StatusMoveType::NO_DAMAGE && mv->statusType != StatusType::CONFUSION) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::banefulBunker);
                break;
            case moves::storedPower:
                for (const Move* mv : moveList) if (mv->statChanges[0].stages > 1 || mv->statChanges[1].type != StatChangeType::NONE) synergisticMoves.push_back(mv->number);
                synergisticMoves.push_back(moves::acupressure);
                synergisticMoves.push_back(moves::shellSmash);
                break;
            case moves::swagger:
                synergisticMoves.push_back(moves::punishment);
                break;
            case moves::punishment:
                synergisticMoves.push_back(moves::swagger);
                break;
            case moves::shellSmash:
                synergisticMoves.push_back(moves::storedPower);
                break;
        }

        return pickIn(moveList, synergisticMoves);
    }

    std::vector<Move*> getHardMoveAntiSynergy(const Move* mv1, const std::vector<Move*>& moveList) {
        std::vector<int> antiSynergisticMoves;


        if (mv1->category == MoveCategory::STATUS && mv1->statusType != StatusType::NONE) {
            for (const Move* mv : moveList) if ((mv->category == MoveCategory::STATUS && mv->statusType != StatusType::NONE && (mv->statusType == mv1->statusType || (mv1->statusType != StatusType::CONFUSION && mv->statusType != StatusType::CONFUSION)))) antiSynergisticMoves.push_back(mv->number);
        }

        switch (mv1->number) {
            case moves::protect: {
                antiSynergisticMoves.push_back(moves::detect);
                antiSynergisticMoves.push_back(moves::banefulBunker);
                antiSynergisticMoves.push_back(moves::kingsShield);
                break;
            }
            case moves::detect: {
                antiSynergisticMoves.push_back(moves::protect);
                antiSynergisticMoves.push_back(moves::banefulBunker);
                antiSynergisticMoves.push_back(moves::kingsShield);
                break;
            }
            case moves::kingsShield: {
                antiSynergisticMoves.push_back(moves::protect);
                antiSynergisticMoves.push_back(moves::detect);
                antiSynergisticMoves.push_back(moves::banefulBunker);
                break;
            }
            case moves::banefulBunker: {
                antiSynergisticMoves.push_back(moves::protect);
                antiSynergisticMoves.push_back(moves::detect);
                antiSynergisticMoves.push_back(moves::kingsShield);
                break;
            }
            case moves::returnTheMoveNotTheKeyword:
                antiSynergisticMoves.push_back(moves::frustration);
                break;
            case moves::frustration:
                antiSynergisticMoves.push_back(moves::returnTheMoveNotTheKeyword);
                break;
            case moves::leechSeed:
            case moves::perishSong: {
                antiSynergisticMoves.push_back(moves::whirlwind);
                antiSynergisticMoves.push_back(moves::roar);
                antiSynergisticMoves.push_back(moves::circleThrow);
                antiSynergisticMoves.push_back(moves::dragonTail);
                break;
            }
        }

        if (mv1->type) {
            switch (*mv1->type) {
                case Type::FIRE:
                    if (mv1->category != MoveCategory::STATUS) {
                        antiSynergisticMoves.push_back(moves::waterSport);
                    }
                    break;
                case Type::ELECTRIC:
                    if (mv1->category != MoveCategory::STATUS) {
                        antiSynergisticMoves.push_back(moves::mudSport);
                    }
                    break;
            }
        }

        return pickIn(moveList, antiSynergisticMoves);
    }

    std::vector<Move*> getSoftMoveAntiSynergy(const Move* mv1, const std::vector<Move*>& moveList) {
        std::vector<int> antiSynergisticMoves;


        if (mv1->category != MoveCategory::STATUS) {
            for (const Move* mv : moveList) if ((mv->category != MoveCategory::STATUS && mv->type == mv1->type)) antiSynergisticMoves.push_back(mv->number);
        }

        switch (mv1->number) {
            case moves::waterSport:
                for (const Move* mv : moveList) if ((mv->category != MoveCategory::STATUS && mv->type == Type::FIRE)) antiSynergisticMoves.push_back(mv->number);
                break;
            case moves::mudSport:
                for (const Move* mv : moveList) if ((mv->category != MoveCategory::STATUS && mv->type == Type::ELECTRIC)) antiSynergisticMoves.push_back(mv->number);
                break;
        }

        return pickIn(moveList, antiSynergisticMoves);
    }

    std::vector<Move*> requiresOtherMove(const Move* mv1, const std::vector<Move*>& moveList) {
        std::vector<int> requiresMove;
        switch (mv1->number) {
            case moves::spitUp:
            case moves::swallow:
                requiresMove.push_back(moves::stockpile);
                break;
            case moves::dreamEater:
            case moves::nightmare:
                for (const Move* mv : moveList) if ((mv->category == MoveCategory::STATUS && mv->statusType == StatusType::SLEEP)) requiresMove.push_back(mv->number);
                break;
            case moves::snore:
            case moves::sleepTalk:
                requiresMove.push_back(moves::rest);
                break;
        }
        return pickIn(moveList, requiresMove);
    }

}  // namespace pr::MoveSynergy
