// Generado por tools/translate_updaters.py a partir de FVX. No editar a mano.

void MoveUpdater::gen2Updates() {
        updateMoveType(moves::karateChop, Type::FIGHTING);
        updateMoveType(moves::gust, Type::FLYING);
        updateMovePower(moves::wingAttack, 60);
        updateMoveAccuracy(moves::whirlwind, 100);
        updateMoveType(moves::sandAttack, Type::GROUND);
        updateMovePower(moves::doubleEdge, 120);
        updateMoveAccuracy(moves::blizzard, 70);
        updateMoveAccuracy(moves::rockThrow, 90);
        updateMoveAccuracy(moves::hypnosis, 60);
        updateMovePower(moves::selfDestruct, 200);
        updateMovePower(moves::explosion, 250);
        updateMovePower(moves::dig, 60);
        
    }

void MoveUpdater::gen3Updates() {
        updateMoveAccuracy(moves::razorWind, 100);
        
        
        
        updateMoveAccuracy(moves::lowKick, 100);
    }

void MoveUpdater::gen4Updates() {
        updateMovePower(moves::fly, 90);
        updateMovePP(moves::vineWhip, 15);
        updateMovePP(moves::absorb, 25);
        updateMovePP(moves::megaDrain, 15);
        updateMovePower(moves::dig, 80);
        updateMovePP(moves::recover, 10);
        updateMoveAccuracy(moves::flash, 100);
        updateMovePower(moves::petalDance, 90);
        updateMoveAccuracy(moves::disable, 80);
        updateMovePower(moves::jumpKick, 85);
        updateMovePower(moves::highJumpKick, 100);

        if (gen() >= 2) {
            updateMovePower(moves::zapCannon, 120);
            updateMovePower(moves::outrage, 120);
            updateMovePP(moves::outrage, 10);
            updateMovePP(moves::gigaDrain, 10);
            updateMovePower(moves::rockSmash, 40);
        }

        if (gen() == 3) {
            updateMovePP(moves::stockpile, 20);
            updateMovePower(moves::dive, 80);
            updateMovePower(moves::leafBlade, 90);
        }
    }

void MoveUpdater::gen5Updates() {
        updateMoveAccuracy(moves::bind, 85);
        updateMovePP(moves::jumpKick, 10);
        updateMovePower(moves::jumpKick, 100);
        updateMovePower(moves::tackle, 50);
        updateMoveAccuracy(moves::tackle, 100);
        updateMoveAccuracy(moves::wrap, 90);
        updateMovePP(moves::thrash, 10);
        updateMovePower(moves::thrash, 120);
        updateMoveAccuracy(moves::disable, 100);
        updateMovePP(moves::petalDance, 10);
        updateMovePower(moves::petalDance, 120);
        updateMoveAccuracy(moves::fireSpin, 85);
        updateMovePower(moves::fireSpin, 35);
        updateMoveAccuracy(moves::toxic, 90);
        updateMoveAccuracy(moves::clamp, 85);
        updateMovePP(moves::clamp, 15);
        updateMovePP(moves::highJumpKick, 10);
        updateMovePower(moves::highJumpKick, 130);
        updateMoveAccuracy(moves::glare, 90);
        updateMoveAccuracy(moves::poisonGas, 80);
        updateMoveAccuracy(moves::crabhammer, 90);

        if (gen() >= 2) {
            updateMoveType(moves::curse, Type::GHOST);
            updateMoveAccuracy(moves::cottonSpore, 100);
            updateMoveAccuracy(moves::scaryFace, 100);
            updateMoveAccuracy(moves::boneRush, 90);
            updateMovePower(moves::gigaDrain, 75);
            updateMovePower(moves::furyCutter, 20);
            updateMovePP(moves::futureSight, 10);
            updateMovePower(moves::futureSight, 100);
            updateMoveAccuracy(moves::futureSight, 100);
            updateMovePower(moves::whirlpool, 35);
            updateMoveAccuracy(moves::whirlpool, 85);
        }

        if (gen() >= 3) {
            updateMovePower(moves::uproar, 90);
            updateMovePower(moves::sandTomb, 35);
            updateMoveAccuracy(moves::sandTomb, 85);
            updateMovePower(moves::bulletSeed, 25);
            updateMovePower(moves::icicleSpear, 25);
            updateMovePower(moves::covet, 60);
            updateMoveAccuracy(moves::rockBlast, 90);
            updateMovePower(moves::doomDesire, 140);
            updateMoveAccuracy(moves::doomDesire, 100);
        }

        if (gen() == 4) {
            updateMovePower(moves::feint, 30);
            updateMovePower(moves::lastResort, 140);
            updateMovePP(moves::drainPunch, 10);
            updateMovePower(moves::drainPunch, 75);
            updateMoveAccuracy(moves::magmaStorm, 75);
        }
    }

void MoveUpdater::gen6Updates() {
        updateMovePP(moves::swordsDance, 20);
        updateMoveAccuracy(moves::whirlwind, romHandler.getPerfectAccuracy());
        updateMovePP(moves::vineWhip, 25);
        updateMovePower(moves::vineWhip, 45);
        updateMovePower(moves::pinMissile, 25);
        updateMoveAccuracy(moves::pinMissile, 95);
        updateMovePower(moves::flamethrower, 90);
        updateMovePower(moves::hydroPump, 110);
        updateMovePower(moves::surf, 90);
        updateMovePower(moves::iceBeam, 90);
        updateMovePower(moves::blizzard, 110);
        updateMovePP(moves::growth, 20);
        updateMovePower(moves::thunderbolt, 90);
        updateMovePower(moves::thunder, 110);
        updateMovePP(moves::minimize, 10);
        updateMovePP(moves::barrier, 20);
        updateMovePower(moves::lick, 30);
        updateMovePower(moves::smog, 30);
        updateMovePower(moves::fireBlast, 110);
        updateMovePP(moves::skullBash, 10);
        updateMovePower(moves::skullBash, 130);
        updateMoveAccuracy(moves::glare, 100);
        updateMoveAccuracy(moves::poisonGas, 90);
        updateMovePower(moves::bubble, 40);
        updateMoveAccuracy(moves::psywave, 100);
        updateMovePP(moves::acidArmor, 20);
        updateMovePower(moves::crabhammer, 100);

        if (gen() >= 2) {
            updateMovePP(moves::thief, 25);
            updateMovePower(moves::thief, 60);
            updateMovePower(moves::snore, 50);
            updateMovePower(moves::furyCutter, 40);
            updateMovePower(moves::futureSight, 120);
        }

        if (gen() >= 3) {
            updateMovePower(moves::heatWave, 95);
            updateMoveAccuracy(moves::willOWisp, 85);
            updateMovePower(moves::smellingSalts, 70);
            updateMovePower(moves::knockOff, 65);
            updateMovePower(moves::meteorMash, 90);
            updateMoveAccuracy(moves::meteorMash, 90);
            updateMovePower(moves::airCutter, 60);
            updateMovePower(moves::overheat, 130);
            updateMovePP(moves::rockTomb, 15);
            updateMovePower(moves::rockTomb, 60);
            updateMoveAccuracy(moves::rockTomb, 95);
            updateMovePP(moves::extrasensory, 20);
            updateMovePower(moves::muddyWater, 90);
            updateMovePP(moves::covet, 25);
        }

        if (gen() >= 4) {
            updateMovePower(moves::wakeUpSlap, 70);
            updateMovePP(moves::tailwind, 15);
            updateMovePower(moves::assurance, 60);
            updateMoveAccuracy(moves::psychoShift, 100);
            updateMovePower(moves::auraSphere, 80);
            updateMovePP(moves::airSlash, 15);
            updateMovePower(moves::dragonPulse, 85);
            updateMovePower(moves::powerGem, 80);
            updateMovePower(moves::energyBall, 90);
            updateMovePower(moves::dracoMeteor, 130);
            updateMovePower(moves::leafStorm, 130);
            updateMoveAccuracy(moves::gunkShot, 80);
            updateMovePower(moves::chatter, 65);
            updateMovePower(moves::magmaStorm, 100);
        }

        if (gen() == 5) {
            updateMovePower(moves::synchronoise, 120);
            updateMovePower(moves::lowSweep, 65);
            updateMovePower(moves::hex, 65);
            updateMovePower(moves::incinerate, 60);
            updateMovePower(moves::waterPledge, 80);
            updateMovePower(moves::firePledge, 80);
            updateMovePower(moves::grassPledge, 80);
            updateMovePower(moves::struggleBug, 50);
            
            
            
            updateMovePower(moves::frostBreath, 45);
            updateMovePower(moves::stormThrow, 45);
            updateMovePP(moves::sacredSword, 15);
            updateMovePower(moves::hurricane, 110);
            updateMovePower(moves::technoBlast, 120);
        }
    }

void MoveUpdater::gen7Updates() {
        updateMovePower(moves::leechLife, 80);
        updateMovePP(moves::leechLife, 10);
        updateMovePP(moves::submission, 20);
        updateMovePower(moves::tackle, 40);
        updateMoveAccuracy(moves::thunderWave, 90);

        if (gen() >= 2) {
            updateMoveAccuracy(moves::swagger, 85);
        }

        if (gen() >= 3) {
            updateMovePP(moves::knockOff, 20);
        }

        if (gen() >= 4) {
            updateMoveAccuracy(moves::darkVoid, 50);
            updateMovePower(moves::suckerPunch, 70);
        }

        if (gen() == 6) {
            updateMoveAccuracy(moves::aromaticMist, romHandler.getPerfectAccuracy());
            updateMovePower(moves::fellStinger, 50);
            updateMovePower(moves::flyingPress, 100);
            updateMovePP(moves::matBlock, 10);
            updateMovePower(moves::mysticalFire, 75);
            updateMovePower(moves::parabolicCharge, 65);
            updateMoveAccuracy(moves::topsyTurvy, romHandler.getPerfectAccuracy());
            updateMoveCategory(moves::waterShuriken, MoveCategory::SPECIAL);
        }
    }

void MoveUpdater::gen8Updates() {
        if (gen() >= 2) {
            updateMovePower(moves::rapidSpin, 50);
        }

        if (gen() == 7) {
            updateMovePower(moves::multiAttack, 120);
        }
    }

void MoveUpdater::gen9Updates() {
        updateMovePP(moves::recover, 5);
        updateMovePP(moves::softBoiled, 5);
        updateMovePP(moves::rest, 5);

        if (gen() >= 2) {
            updateMovePP(moves::milkDrink, 5);
        }

        if (gen() >= 3) {
            updateMovePower(moves::lusterPurge, 95);
            updateMovePower(moves::mistBall, 95);
            updateMovePP(moves::slackOff, 5);
        }

        if (gen() >= 4) {
            updateMovePP(moves::roost, 5);
        }

        if (gen() >= 7) {
            updateMovePP(moves::shoreUp, 5);
        }

        if (gen() >= 8) {
            updateMovePower(moves::grassyGlide, 55);
            updateMovePower(moves::wickedBlow, 75);
            updateMovePower(moves::glacialLance, 120);
        }
    }

void SpeciesBaseStatUpdater::gen6Updates() {
        if (gen() == 1) {
            
            updateSpecial(species::butterfree, 90);
            updateSpecial(species::clefable, 95);
            updateSpecial(species::wigglytuff, 85);
            updateSpecial(species::vileplume, 110);
        }

        updateSpAtk(species::butterfree, 90);
        updateAtk(species::beedrill, 90);
        updateSpeed(species::pidgeot, 101);
        updateDef(species::pikachu, 40);
        updateSpDef(species::pikachu, 50);
        updateSpeed(species::raichu, 110);
        updateAtk(species::nidoqueen, 92);
        updateAtk(species::nidoking, 102);
        updateSpAtk(species::clefable, 95);
        updateSpAtk(species::wigglytuff, 85);
        updateSpAtk(species::vileplume, 110);
        updateAtk(species::poliwrath, 95);
        updateSpDef(species::alakazam, 95);
        updateSpDef(species::victreebel, 70);
        updateAtk(species::golem, 120);

        if (gen() >= 2) {
            updateDef(species::ampharos, 85);
            updateDef(species::bellossom, 95);
            updateSpAtk(species::azumarill, 60);
            updateSpDef(species::jumpluff, 95);
        }
        if (gen() >= 3) {
            updateSpAtk(species::beautifly, 100);
            updateSpDef(species::exploud, 73);
        }
        if (gen() >= 4) {
            updateSpDef(species::staraptor, 60);
            updateDef(species::roserade, 65);
        }
        if (gen() >= 5) {
            updateAtk(species::stoutland, 110);
            updateAtk(species::unfezant, 115);
            updateSpDef(species::gigalith, 80);
            updateAtk(species::seismitoad, 95);
            updateSpDef(species::leavanny, 80);
            updateAtk(species::scolipede, 100);
            updateDef(species::krookodile, 80);
        }
    }

void SpeciesBaseStatUpdater::gen7Updates() {
        updateAtk(species::arbok, 95);
        updateAtk(species::dugtrio, 100);
        updateAtk(species::farfetchd, 90);
        updateSpeed(species::dodrio, 110);
        updateSpeed(species::electrode, 150);
        updateSpDef(species::exeggutor, 75);

        if (gen() >= 2) {
            updateSpAtk(species::noctowl, 86);
            updateSpDef(species::ariados, 70);
            updateDef(species::qwilfish, 85);
            updateHP(species::magcargo, 60);
            updateSpAtk(species::magcargo, 90);
            updateHP(species::corsola, 65);
            updateDef(species::corsola, 95);
            updateSpDef(species::corsola, 95);
            updateHP(species::mantine, 85);
        }
        if (gen() >= 3) {
            updateSpAtk(species::swellow, 75);
            updateSpAtk(species::pelipper, 95);
            updateSpAtk(species::masquerain, 100);
            updateSpeed(species::masquerain, 80);
            updateSpeed(species::delcatty, 90);
            updateDef(species::volbeat, 75);
            updateSpDef(species::volbeat, 85);
            updateDef(species::illumise, 75);
            updateSpDef(species::illumise, 85);
            updateHP(species::lunatone, 90);
            updateHP(species::solrock, 90);
            updateHP(species::chimecho, 75);
            updateDef(species::chimecho, 80);
            updateSpDef(species::chimecho, 90);
        }
        if (gen() >= 5) {
            updateHP(species::woobat, 65);
            updateAtk(species::crustle, 105);
            updateAtk(species::beartic, 130);
            updateHP(species::cryogonal, 80);
            updateDef(species::cryogonal, 50);
        }
        if (gen() == 6) {
            updateSpDef(species::Gen6Formes::alakazamMega, 105);
        }
    }

void SpeciesBaseStatUpdater::gen8Updates() {
        // (Aegislash: solo Gen 6-7, no portado)
}

void SpeciesBaseStatUpdater::gen9Updates() {
        if (gen() >= 4) {
            updateDef(species::cresselia, 110);
            updateSpDef(species::cresselia, 120);
        }
        
        
        
        
        
    }
