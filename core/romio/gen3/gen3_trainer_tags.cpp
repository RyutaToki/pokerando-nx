// Etiquetas de entrenadores de Gen 3 (líderes, Alto Mando, rivales...) y combates múltiples.
// Traducido de Gen3Constants.trainerTagsRS/E/FRLG y setMultiBattleStatusEm (FVX).
#include "romio/gen3/gen3_trainer_tags.hpp"

#include "romio/gen3/gen3_constants.hpp"

namespace pr::gen3 {

namespace {
using TrainerList = std::vector<TrainerPtr>;

void tag(TrainerList& trs, int trainerNum, const char* t) { trs.at(trainerNum - 1)->tag = t; }
template <class... I>
void tag(TrainerList& trs, const char* t, I... nums) {
    for (int n : {static_cast<int>(nums)...}) trs.at(n - 1)->tag = t;
}
template <class... I>
void setMultiBattleStatus(TrainerList& trs, Trainer::MultiBattleStatus st, I... nums) {
    for (int n : {static_cast<int>(nums)...}) {
        if (static_cast<int>(trs.size()) > n - 1) trs[n - 1]->multiBattleStatus = st;
    }
}
}  // namespace

void trainerTagsRS(std::vector<TrainerPtr>& trs, int romType) {
        // Gym Trainers
        tag(trs, "GYM1", 0x140, 0x141);
        tag(trs, "GYM2", 0x1AA, 0x1A9, 0xB3);
        tag(trs, "GYM3", 0xBF, 0x143, 0xC2, 0x289);
        tag(trs, "GYM4", 0xC9, 0x288, 0xCB, 0x28A, 0xCD);
        tag(trs, "GYM5", 0x47, 0x59, 0x49, 0x5A, 0x48, 0x5B, 0x4A);
        tag(trs, "GYM6", 0x191, 0x28F, 0x28E, 0x194);
        tag(trs, "GYM7", 0xE9, 0xEA, 0xEB, 0xF4, 0xF5, 0xF6);
        tag(trs, "GYM8", 0x82, 0x266, 0x83, 0x12D, 0x81, 0x74, 0x80, 0x265);

        // Gym Leaders
        tag(trs, 0x109, "GYM1-LEADER");
        tag(trs, 0x10A, "GYM2-LEADER");
        tag(trs, 0x10B, "GYM3-LEADER");
        tag(trs, 0x10C, "GYM4-LEADER");
        tag(trs, 0x10D, "GYM5-LEADER");
        tag(trs, 0x10E, "GYM6-LEADER");
        tag(trs, 0x10F, "GYM7-LEADER");
        tag(trs, 0x110, "GYM8-LEADER");
        // Elite 4
        tag(trs, 0x105, "ELITE1");
        tag(trs, 0x106, "ELITE2");
        tag(trs, 0x107, "ELITE3");
        tag(trs, 0x108, "ELITE4");
        tag(trs, 0x14F, "CHAMPION");
        // Brendan
        tag(trs, 0x208, "RIVAL1-2");
        tag(trs, 0x20B, "RIVAL1-0");
        tag(trs, 0x20E, "RIVAL1-1");

        tag(trs, 0x209, "RIVAL2-2");
        tag(trs, 0x20C, "RIVAL2-0");
        tag(trs, 0x20F, "RIVAL2-1");

        tag(trs, 0x20A, "RIVAL3-2");
        tag(trs, 0x20D, "RIVAL3-0");
        tag(trs, 0x210, "RIVAL3-1");

        tag(trs, 0x295, "RIVAL4-2");
        tag(trs, 0x296, "RIVAL4-0");
        tag(trs, 0x297, "RIVAL4-1");

        // May
        tag(trs, 0x211, "RIVAL1-2");
        tag(trs, 0x214, "RIVAL1-0");
        tag(trs, 0x217, "RIVAL1-1");

        tag(trs, 0x212, "RIVAL2-2");
        tag(trs, 0x215, "RIVAL2-0");
        tag(trs, 0x218, "RIVAL2-1");

        tag(trs, 0x213, "RIVAL3-2");
        tag(trs, 0x216, "RIVAL3-0");
        tag(trs, 0x219, "RIVAL3-1");

        tag(trs, 0x298, "RIVAL4-2");
        tag(trs, 0x299, "RIVAL4-0");
        tag(trs, 0x29A, "RIVAL4-1");

        // Wally
        tag(trs, "THEMED:WALLY-STRONG", 0x207, 0x290, 0x291, 0x292, 0x293, 0x294);

        if (romType == RomType_Ruby) {
            tag(trs, "THEMED:MAXIE-LEADER", 0x259, 0x25A);
            tag(trs, "THEMED:COURTNEY-STRONG", 0x257, 0x258);
            tag(trs, "THEMED:TABITHA-STRONG", 0x254, 0x255);
        } else {
            tag(trs, "THEMED:ARCHIE-LEADER", 0x23, 0x22);
            tag(trs, "THEMED:MATT-STRONG", 0x1E, 0x1F);
            tag(trs, "THEMED:SHELLY-STRONG", 0x20, 0x21);
        }

    }

void trainerTagsE(std::vector<TrainerPtr>& trs) {
        // Gym Trainers
        tag(trs, "GYM1", 0x140, 0x141, 0x23B);
        tag(trs, "GYM2", 0x1AA, 0x1A9, 0xB3, 0x23C, 0x23D, 0x23E);
        tag(trs, "GYM3", 0xBF, 0x143, 0xC2, 0x289, 0x322);
        tag(trs, "GYM4", 0x288, 0xC9, 0xCB, 0x28A, 0xCA, 0xCC, 0x1F5, 0xCD);
        tag(trs, "GYM5", 0x47, 0x59, 0x49, 0x5A, 0x48, 0x5B, 0x4A);
        tag(trs, "GYM6", 0x192, 0x28F, 0x191, 0x28E, 0x194, 0x323);
        tag(trs, "GYM7", 0xE9, 0xEA, 0xEB, 0xF4, 0xF5, 0xF6, 0x24F, 0x248, 0x247, 0x249, 0x246, 0x23F);
        tag(trs, "GYM8", 0x265, 0x80, 0x1F6, 0x73, 0x81, 0x76, 0x82, 0x12D, 0x83, 0x266);

        // Gym Leaders + Emerald Rematches!
        tag(trs, "GYM1-LEADER", 0x109, 0x302, 0x303, 0x304, 0x305);
        tag(trs, "GYM2-LEADER", 0x10A, 0x306, 0x307, 0x308, 0x309);
        tag(trs, "GYM3-LEADER", 0x10B, 0x30A, 0x30B, 0x30C, 0x30D);
        tag(trs, "GYM4-LEADER", 0x10C, 0x30E, 0x30F, 0x310, 0x311);
        tag(trs, "GYM5-LEADER", 0x10D, 0x312, 0x313, 0x314, 0x315);
        tag(trs, "GYM6-LEADER", 0x10E, 0x316, 0x317, 0x318, 0x319);
        tag(trs, "GYM7-LEADER", 0x10F, 0x31A, 0x31B, 0x31C, 0x31D);
        tag(trs, "GYM8-LEADER", 0x110, 0x31E, 0x31F, 0x320, 0x321);

        // Elite 4
        tag(trs, 0x105, "ELITE1");
        tag(trs, 0x106, "ELITE2");
        tag(trs, 0x107, "ELITE3");
        tag(trs, 0x108, "ELITE4");
        tag(trs, 0x14F, "CHAMPION");

        // Brendan
        tag(trs, 0x208, "RIVAL1-2");
        tag(trs, 0x20B, "RIVAL1-0");
        tag(trs, 0x20E, "RIVAL1-1");

        tag(trs, 0x251, "RIVAL2-2");
        tag(trs, 0x250, "RIVAL2-0");
        tag(trs, 0x257, "RIVAL2-1");

        tag(trs, 0x209, "RIVAL3-2");
        tag(trs, 0x20C, "RIVAL3-0");
        tag(trs, 0x20F, "RIVAL3-1");

        tag(trs, 0x20A, "RIVAL4-2");
        tag(trs, 0x20D, "RIVAL4-0");
        tag(trs, 0x210, "RIVAL4-1");

        tag(trs, 0x295, "RIVAL5-2");
        tag(trs, 0x296, "RIVAL5-0");
        tag(trs, 0x297, "RIVAL5-1");

        // May
        tag(trs, 0x211, "RIVAL1-2");
        tag(trs, 0x214, "RIVAL1-0");
        tag(trs, 0x217, "RIVAL1-1");

        tag(trs, 0x258, "RIVAL2-2");
        tag(trs, 0x300, "RIVAL2-0");
        tag(trs, 0x301, "RIVAL2-1");

        tag(trs, 0x212, "RIVAL3-2");
        tag(trs, 0x215, "RIVAL3-0");
        tag(trs, 0x218, "RIVAL3-1");

        tag(trs, 0x213, "RIVAL4-2");
        tag(trs, 0x216, "RIVAL4-0");
        tag(trs, 0x219, "RIVAL4-1");

        tag(trs, 0x298, "RIVAL5-2");
        tag(trs, 0x299, "RIVAL5-0");
        tag(trs, 0x29A, "RIVAL5-1");

        // Themed
        tag(trs, "THEMED:MAXIE-LEADER", 0x259, 0x25A, 0x2DE);
        tag(trs, "THEMED:TABITHA-STRONG", 0x202, 0x255, 0x2DC);
        tag(trs, "THEMED:ARCHIE-LEADER", 0x22);
        tag(trs, "THEMED:MATT-STRONG", 0x1E);
        tag(trs, "THEMED:SHELLY-STRONG", 0x20, 0x21);
        tag(trs, "THEMED:WALLY-STRONG", 0x207, 0x290, 0x291, 0x292, 0x293, 0x294);

        // Steven
        // TODO: make these share a theme
        tag(trs, 855, "NOTSTRONG"); // Mossdeep tag battle
        tag(trs, emMeteorFallsStevenIndex, "UBER");

    }

void trainerTagsFRLG(std::vector<TrainerPtr>& trs) {

        // Gym Trainers
        tag(trs, "GYM1", 0x8E);
        tag(trs, "GYM2", 0xEA, 0x96);
        tag(trs, "GYM3", 0xDC, 0x8D, 0x1A7);
        tag(trs, "GYM4", 0x10A, 0x84, 0x109, 0xA0, 0x192, 0x10B, 0x85);
        tag(trs, "GYM5", 0x125, 0x124, 0x120, 0x127, 0x126, 0x121);
        tag(trs, "GYM6", 0x11A, 0x119, 0x1CF, 0x11B, 0x1CE, 0x1D0, 0x118);
        tag(trs, "GYM7", 0xD5, 0xB1, 0xB2, 0xD6, 0xB3, 0xD7, 0xB4);
        tag(trs, "GYM8", 0x129, 0x143, 0x188, 0x190, 0x142, 0x128, 0x191, 0x144);

        // Gym Leaders
        tag(trs, 0x19E, "GYM1-LEADER");
        tag(trs, 0x19F, "GYM2-LEADER");
        tag(trs, 0x1A0, "GYM3-LEADER");
        tag(trs, 0x1A1, "GYM4-LEADER");
        tag(trs, 0x1A2, "GYM5-LEADER");
        tag(trs, 0x1A4, "GYM6-LEADER");
        tag(trs, 0x1A3, "GYM7-LEADER");
        tag(trs, 0x15E, "GYM8-LEADER");

        // Giovanni
        tag(trs, 0x15C, "GIO1-LEADER");
        tag(trs, 0x15D, "GIO2-LEADER");

        // E4 Round 1
        tag(trs, 0x19A, "ELITE1-1");
        tag(trs, 0x19B, "ELITE2-1");
        tag(trs, 0x19C, "ELITE3-1");
        tag(trs, 0x19D, "ELITE4-1");

        // E4 Round 2
        tag(trs, 0x2DF, "ELITE1-2");
        tag(trs, 0x2E0, "ELITE2-2");
        tag(trs, 0x2E1, "ELITE3-2");
        tag(trs, 0x2E2, "ELITE4-2");

        // Rival Battles

        // Initial Rival
        tag(trs, 0x148, "RIVAL1-0");
        tag(trs, 0x146, "RIVAL1-1");
        tag(trs, 0x147, "RIVAL1-2");

        // Route 22 (weak)
        tag(trs, 0x14B, "RIVAL2-0");
        tag(trs, 0x149, "RIVAL2-1");
        tag(trs, 0x14A, "RIVAL2-2");

        // Cerulean
        tag(trs, 0x14E, "RIVAL3-0");
        tag(trs, 0x14C, "RIVAL3-1");
        tag(trs, 0x14D, "RIVAL3-2");

        // SS Anne
        tag(trs, 0x1AC, "RIVAL4-0");
        tag(trs, 0x1AA, "RIVAL4-1");
        tag(trs, 0x1AB, "RIVAL4-2");

        // Pokemon Tower
        tag(trs, 0x1AF, "RIVAL5-0");
        tag(trs, 0x1AD, "RIVAL5-1");
        tag(trs, 0x1AE, "RIVAL5-2");

        // Silph Co
        tag(trs, 0x1B2, "RIVAL6-0");
        tag(trs, 0x1B0, "RIVAL6-1");
        tag(trs, 0x1B1, "RIVAL6-2");

        // Route 22 (strong)
        tag(trs, 0x1B5, "RIVAL7-0");
        tag(trs, 0x1B3, "RIVAL7-1");
        tag(trs, 0x1B4, "RIVAL7-2");

        // E4 Round 1
        tag(trs, 0x1B8, "RIVAL8-0");
        tag(trs, 0x1B6, "RIVAL8-1");
        tag(trs, 0x1B7, "RIVAL8-2");

        // E4 Round 2
        tag(trs, 0x2E5, "RIVAL9-0");
        tag(trs, 0x2E3, "RIVAL9-1");
        tag(trs, 0x2E4, "RIVAL9-2");

    }

void setMultiBattleStatusEm(std::vector<TrainerPtr>& trs) {
        // 25 + 569: Double Battle with Team Aqua Grunts on Mt. Pyre
        // 105 + 237: Double Battle with Hex Maniac Patricia and Psychic Joshua
        // 397 + 508: Double Battle with Dragon Tamer Aaron and Cooltrainer Marley
        // 404 + 654: Double Battle with Bird Keeper Edwardo and Camper Flint
        // 504 + 505: Double Battle with Ninja Boy Jonas and Parasol Lady Kayley
        // 514 + 734: Double Battle with Tabitha and Maxie in Mossdeep Space Center
        // 572 + 573: Double Battle with Sailor Brenden and Battle Girl Lilith
        // 721 + 730: Double Battle with Team Magma Grunts in Team Magma Hideout
        // 848 + 850: Double Battle with Psychic Mariela and Gentleman Everett
        // 855: Steven from the Double Battle in Mossdeep Space Center
        setMultiBattleStatus(trs, Trainer::MultiBattleStatus::ALWAYS, 25, 105, 237, 397, 404, 504, 505, 508, 514,
                569, 572, 573, 654, 721, 730, 734, 848, 850, 855
        );

        // 1 + 124: Potential Double Battle with Hiker Sawyer and Beauty Melissa
        // 3 + 192: Potential Double Battle with Team Aqua Grunts in Team Aqua Hideout
        // 8 + 14: Potential Double Battle with Team Aqua Grunts in Seafloor Cavern
        // 9 + 236 + 247: Potential Double Battle with Pokemon Breeder Gabrielle, Psychic William, and Psychic Kayla
        // 11 + 767: Potential Double Battle with Cooltrainer Marcel and Cooltrainer Cristin
        // 12 + 195: Potential Double Battle with Bird Keeper Alberto and Guitarist Fernando
        // 13 + 106: Potential Double Battle with Collector Ed and Hex Maniac Kindra
        // 15 + 450: Potential Double Battle with Swimmer Declan and Swimmer Grace
        // 18 + 596: Potential Double Battle with Team Aqua Grunts in Weather Institute
        // 28 + 193: Potential Double Battle with Team Aqua Grunts in Team Aqua Hideout
        // 29 + 249: Potential Double Battle with Expert Fredrick and Psychic Jacki
        // 31 + 35 + 145: Potential Double Battles with Black Belt Zander, Hex Maniac Leah, and PokéManiac Mark
        // 33 + 567: Potential Double Battle with Shelly and Team Aqua Grunt in Seafloor Cavern
        // 37 + 715: Potential Double Battle with Aroma Lady Rose and Youngster Deandre
        // 38 + 417: Potential Double Battle with Cooltrainer Felix and Cooltrainer Dianne
        // 57 + 698: Potential Double Battle with Tuber Lola and Tuber Chandler
        // 64 + 491 + 697: Potential Double Battles with Tuber Ricky, Sailor Edmond, and Tuber Hailey
        // 107 + 764: Potential Double Battle with Hex Maniac Tammy and Bug Maniac Cale
        // 108 + 475: Potential Double Battle with Hex Maniac Valerie and Psychic Cedric
        // 115 + 502: Potential Double Battle with Lady Daphne and Pokéfan Annika
        // 118 + 129: Potential Double Battle with Lady Brianna and Beauty Bridget
        // 130 + 301: Potential Double Battle with Beauty Olivia and Pokéfan Bethany
        // 131 + 614: Potential Double Battle with Beauty Tiffany and Lass Crissy
        // 137 + 511: Potential Double Battle with Expert Mollie and Expert Conor
        // 144 + 375: Potential Double Battle with Beauty Thalia and Youngster Demetrius
        // 146 + 579: Potential Double Battle with Team Magma Grunts on Mt. Chimney
        // 160 + 595: Potential Double Battle with Swimmer Roland and Triathlete Isabella
        // 168 + 455: Potential Double Battle with Swimmer Santiago and Swimmer Katie
        // 170 + 460: Potential Double Battle with Swimmer Franklin and Swimmer Debra
        // 171 + 385: Potential Double Battle with Swimmer Kevin and Triathlete Taila
        // 180 + 509: Potential Double Battle with Black Belt Hitoshi and Battle Girl Reyna
        // 182 + 307 + 748 + 749: Potential Double Battles with Black Belt Koichi, Expert Timothy, Triathlete Kyra, and Ninja Boy Jaiden
        // 191 + 649: Potential Double Battle with Guitarist Kirk and Battle Girl Vivian
        // 194 + 802: Potential Double Battle with Guitarist Shawn and Bug Maniac Angelo
        // 201 + 648: Potential Double Battle with Kindler Cole and Cooltrainer Gerald
        // 204 + 501: Potential Double Battle with Kindler Jace and Hiker Eli
        // 217 + 566: Potential Double Battle with Picnicker Autumn and Triathlete Julio
        // 232 + 701: Potential Double Battle with Psychic Edward and Triathlete Alyssa
        // 233 + 246: Potential Double Battle with Psychic Preston and Psychic Maura
        // 234 + 244 + 575 + 582: Potential Double Battles with Psychic Virgil, Psychic Hannah, Hex Maniac Sylvia, and Gentleman Nate
        // 235 + 245: Potential Double Battle with Psychic Blake and Psychic Samantha
        // 248 + 849: Potential Double Battle with Psychic Alexis and Psychic Alvaro
        // 273 + 605: Potential Double Battle with School Kid Jerry and Lass Janice
        // 302 + 699: Potential Double Battle with Pokéfan Isabel and Pokéfan Kaleb
        // 321 + 571: Potential Double Battle with Youngster Tommy and Hiker Marc
        // 324 + 325: Potential Double Battle with Cooltrainer Quincy and Cooltrainer Katelynn
        // 345 + 742: Potential Double Battle with Fisherman Carter and Bird Keeper Elijah
        // 377 + 459: Potential Double Battle with Triathlete Pablo and Swimmer Sienna
        // 383 + 576: Potential Double Battle with Triathlete Isobel and Swimmer Leonardo
        // 400 + 761: Potential Double Battle with Bird Keeper Phil and Parasol Lady Rachel
        // 401 + 655: Potential Double Battle with Bird Keeper Jared and Picnicker Ashley
        // 403 + 506: Potential Double Battle with Bird Keeper Presley and Expert Auron
        // 413 + 507: Potential Double Battle with Bird Keeper Alex and Sailor Kelvin
        // 415 + 759: Potential Double Battle with Ninja Boy Yasu and Guitarist Fabian
        // 416 + 760: Potential Double Battle with Ninja Boy Takashi and Kindler Dayton
        // 418 + 547: Potential Double Battle with Tuber Jani and Ruin Maniac Garrison
        // 420 + 710 + 711: Potential Double Battles with Ninja Boy Lung, Camper Lawrence, and PokéManiac Wyatt
        // 436 + 762: Potential Double Battle with Parasol Lady Angelica and Cooltrainer Leonel
        // 445 + 739: Potential Double Battle with Swimmer Beth and Triathlete Camron
        // 464 + 578: Potential Double Battle with Swimmer Carlee and Swimmer Harrison
        // 494 + 495: Potential Double Battle with Sailor Phillip and Sailor Leonard (S.S. Tidal)
        // 503 + 539: Potential Double Battle with Cooltrainer Jazmyn and Bug Catcher Davis
        // 512 + 700: Potential Double Battle with Collector Edwin and Guitarist Joseph
        // 513 + 752: Potential Double Battle with Collector Hector and Psychic Marlene
        // 540 + 546: Potential Double Battle with Cooltrainer Mitchell and Cooltrainer Halle
        // 577 + 674: Potential Double Battle with Cooltrainer Athena and Bird Keeper Aidan
        // 580 + 676: Potential Double Battle with Swimmer Clarence and Swimmer Tisha
        // 583 + 584 + 585 + 591: Potential Double Battles with Hex Maniac Kathleen, Gentleman Clifford, Psychic Nicholas, and Psychic Macey
        // 594 + 733: Potential Double Battle with Expert Paxton and Cooltrainer Darcy
        // 598 + 758: Potential Double Battle with Cooltrainer Jonathan and Expert Makayla
        // 629 + 712: Potential Double Battle with Hiker Lucas and Picnicker Angelina
        // 631 + 753 + 754: Potential Double Battles with Hiker Clark, Hiker Devan, and Youngster Johnson
        // 653 + 763: Potential Double Battle with Ninja Boy Riley and Battle Girl Callie
        // 694 + 695: Potential Double Battle with Rich Boy Dawson and Lady Sarah
        // 702 + 703: Potential Double Battle with Guitarist Marcos and Black Belt Rhett
        // 704 + 705: Potential Double Battle with Camper Tyron and Aroma Lady Celina
        // 706 + 707: Potential Double Battle with Picnicker Bianca and Kindler Hayden
        // 708 + 709: Potential Double Battle with Picnicker Sophie and Bird Keeper Coby
        // 713 + 714: Potential Double Battle with Fisherman Kai and Picnicker Charlotte
        // 719 + 720: Potential Double Battle with Team Magma Grunts in Team Magma Hideout
        // 727 + 728: Potential Double Battle with Team Magma Grunts in Team Magma Hideout
        // 735 + 736: Potential Double Battle with Swimmer Pete and Swimmer Isabelle
        // 737 + 738: Potential Double Battle with Ruin Maniac Andres and Bird Keeper Josue
        // 740 + 741: Potential Double Battle with Sailor Cory and Cooltrainer Carolina
        // 743 + 744 + 745: Potential Double Battles with Picnicker Celia, Ruin Maniac Bryan, and Camper Branden
        // 746 + 747: Potential Double Battle with Kindler Bryant and Aroma Lady Shayla
        // 750 + 751: Potential Double Battle with Psychic Alix and Battle Girl Helene
        // 755 + 756 + 757: Potential Double Battles with Triathlete Melina, Psychic Brandi, and Battle Girl Aisha
        // 765 + 766: Potential Double Battle with Pokémon Breeder Myles and Pokémon Breeder Pat
        setMultiBattleStatus(trs, Trainer::MultiBattleStatus::POTENTIAL, 1, 3, 8, 9, 11, 12, 13, 14, 15, 18, 28,
                29, 31, 33, 35, 37, 38, 57, 64, 106, 107, 108, 115, 118, 124, 129, 130, 131, 137, 144, 145, 146, 160,
                168, 170, 171, 180, 182, 191, 192, 193, 194, 195, 201, 204, 217, 232, 233, 234, 235, 236, 244, 245, 246,
                247, 248, 249, 273, 301, 302, 307, 321, 324, 325, 345, 375, 377, 383, 385, 400, 401, 403, 413, 415, 416,
                417, 418, 420, 436, 445, 450, 455, 459, 460, 464, 475, 491, 494, 495, 501, 502, 503, 506, 507, 509, 511,
                512, 513, 539, 540, 546, 547, 566, 567, 571, 575, 576, 577, 578, 579, 580, 582, 583, 584, 585, 591, 594,
                595, 596, 598, 605, 614, 629, 631, 648, 649, 653, 655, 674, 676, 694, 695, 697, 698, 699, 700, 701, 702,
                703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 719, 720, 727, 728, 733, 735, 736, 737,
                738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758,
                759, 760, 761, 762, 763, 764, 765, 766, 767, 802, 849
        );
    }


}  // namespace pr::gen3
