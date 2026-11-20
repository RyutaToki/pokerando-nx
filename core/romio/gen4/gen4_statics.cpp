// Pokémon estáticos de Gen 4: legendarios y regalos (scripts), intercambios del juego, Huevo Misterioso
// (HGSS), fósiles y errantes (port de Gen4RomHandler.getStaticPokemon/setStaticPokemon, DSStaticPokemon,
// StaticPokemonGameCorner, RoamingPokemon y ARMThumbCode).
#include <algorithm>
#include <stdexcept>

#include "constants/species_ids.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/gen4/gen4_rom_handler.hpp"
#include "util/java_compat.hpp"
#include "util/java_hash.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int readLong(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeLong(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
int signedByte(const Bytes& d, int o) { return static_cast<int8_t>(d.at(o)); }

// Port de ARMThumbCode: código Thumb como lista de instrucciones de 2 bytes, que corrige los BL
// relativos al insertar o quitar instrucciones.
class ARMThumbCode {
public:
    static constexpr uint8_t LSL_imm_0 = 0x00, LDRPC_r1 = 0x49, ADD_i8r4 = 0x34, ADDSP_imm7 = 0xB0, POP_pc = 0xBD,
                             BGE = 0xDA;

    explicit ARMThumbCode(const Bytes& bytes) {
        if (bytes.empty()) throw std::invalid_argument("bytes can't be empty");
        if (bytes.size() % 2 != 0) throw std::invalid_argument("bytes.length must be even");
        for (size_t i = 0; i < bytes.size(); i += 2) inst_.push_back({bytes[i], bytes[i + 1]});
    }
    Bytes toBytes() const {
        Bytes b;
        for (const auto& in : inst_) {
            b.push_back(in.lower);
            b.push_back(in.higher);
        }
        return b;
    }
    void insertInstructions(int offset, std::initializer_list<uint8_t> instBytes) {
        checkOffset(offset);
        if (offset > static_cast<int>(inst_.size()) * 2) throw std::invalid_argument("offset too high");
        if (instBytes.size() % 2 != 0) throw std::invalid_argument("instBytes must have even length.");
        std::vector<Inst> add;
        for (auto it = instBytes.begin(); it != instBytes.end(); it += 2) add.push_back({*it, *(it + 1)});
        inst_.insert(inst_.begin() + offset / 2, add.begin(), add.end());
        int n = static_cast<int>(instBytes.size());
        shiftBranchInstructions(offset + n, n / 2);
    }
    void removeInstructions(int offset, int length) {
        checkOffset(offset);
        if (static_cast<int>(inst_.size()) < offset / 2 + length)
            throw std::invalid_argument("offset and length imply removing instructions beyond the code's end");
        inst_.erase(inst_.begin() + offset / 2, inst_.begin() + offset / 2 + length);
        shiftBranchInstructions(offset, -length);
    }
    void setInstruction(int offset, uint8_t lower, uint8_t higher) {
        checkOffset(offset);
        if (offset > static_cast<int>(inst_.size()) * 2) throw std::invalid_argument("offset too high");
        inst_.at(offset / 2) = {lower, higher};
    }

private:
    struct Inst {
        uint8_t lower, higher;
        bool isBLSetup() const { return 0xF0 <= higher && higher <= 0xF7; }
        bool isBLOffset() const { return 0xF8 <= higher; }
    };
    std::vector<Inst> inst_;

    static void checkOffset(int offset) {
        if (offset % 2 != 0) throw std::invalid_argument("offset must be even");
        if (offset < 0) throw std::invalid_argument("offset must be positive");
    }
    void shiftBranchInstructions(int startOffset, int shift) {
        int curr = startOffset;
        for (size_t i = startOffset / 2; i < inst_.size(); i++) {
            Inst& in = inst_[i];
            curr += 2;
            if (in.isBLSetup()) {
                Inst& in2 = inst_.at(i + 1);
                if (!in2.isBLOffset()) throw std::runtime_error("BL setup not followed by BL offset");
                int disp = getBLDisplacement(in, in2) + shift;
                if (!blDestinationIncludedInShift(startOffset, curr, disp)) setBLDisplacement(in, in2, disp);
            }
        }
    }
    bool blDestinationIncludedInShift(int startOffset, int blInstOffset, int disp) const {
        return blInstOffset + disp >= startOffset && (blInstOffset + disp) / 2 <= static_cast<int>(inst_.size());
    }
    static int getBLDisplacement(const Inst& s, const Inst& o) {
        int disp = ((s.higher & 0b11) << 19) + (s.lower << 11) + ((o.higher & 0b111) << 8) + o.lower;
        if ((s.higher & 0b100) > 0) disp = -disp;
        return disp;
    }
    static void setBLDisplacement(Inst& s, Inst& o, int disp) {
        int sign = disp > 0 ? 0 : 1;
        if (sign == 1) disp = -disp;
        if (disp >= 2 << 22) throw std::invalid_argument("number too large to be represented by 21 bits + sign");
        uint32_t d = static_cast<uint32_t>(disp);
        s.higher = static_cast<uint8_t>(0xF0 + (sign << 2) + ((d >> 19) & 0b11));
        s.lower = static_cast<uint8_t>((d >> 11) & 0xFF);
        o.higher = static_cast<uint8_t>(0xF8 + ((d >> 8) & 0b111));
        o.lower = static_cast<uint8_t>(d & 0xFF);
    }
};
}  // namespace

// ---------------- DSStaticPokemon / RoamingPokemon ----------------

Species* Gen4RomHandler::staticGetPokemon(const DSStaticPokemon& s) const {
    const auto& e = s.speciesEntries.at(0);
    return species_.at(readWord(scriptNarc_.files.at(e.file), e.offset));
}

void Gen4RomHandler::staticSetPokemon(const DSStaticPokemon& s, Species* pk) {
    int value = pk->baseNumber();
    for (const auto& e : s.speciesEntries) writeWord(scriptNarc_.files.at(e.file), e.offset, value);
    if (s.gameCorner) {
        // StaticPokemonGameCorner: el Casino nombra al Pokémon en sus textos
        for (const auto& te : s.textEntries) {
            auto strings = getStrings(te.textIndex);
            const std::string& orig = strings.at(te.stringNumber);
            size_t post = orig.find('\\');
            strings.at(te.stringNumber) = jc::toUpperCase(pk->name()) + orig.substr(post);
            setStrings(te.textIndex, strings);
        }
    }
}

int Gen4RomHandler::staticGetLevel(const DSStaticPokemon& s, int i) const {
    if (static_cast<int>(s.levelEntries.size()) <= i) return 1;
    const auto& e = s.levelEntries[i];
    return signedByte(scriptNarc_.files.at(e.file), e.offset);
}

void Gen4RomHandler::staticSetLevel(const DSStaticPokemon& s, int level, int i) {
    if (static_cast<int>(s.levelEntries.size()) > i) {
        const auto& e = s.levelEntries[i];
        scriptNarc_.files.at(e.file).at(e.offset) = static_cast<uint8_t>(level);
    }
}

// ---------------- Huevo Misterioso (HGSS) ----------------

bool Gen4RomHandler::isMysteryEggCommandImproved(const Bytes& ov) const {
    int offset = romEntry_->getInt("MysteryEggCommandOffset") + mysteryEggImprovementIdentifierOffset;
    return ov.at(offset) == mysteryEggImprovementIdentifier;
}

void Gen4RomHandler::improveMysteryEggCommand(Bytes& ov) {
    // Versión mejorada del comando de AdAstra: admite cualquier especie y un movimiento extra (4 bytes cada uno)
    int offset = romEntry_->getInt("MysteryEggCommandOffset");
    Bytes before(ov.begin() + offset, ov.begin() + offset + mysteryEggCommandLength);
    ARMThumbCode tc(before);
    tc.insertInstructions(216, {0x04, ARMThumbCode::ADDSP_imm7, 0xf8, ARMThumbCode::POP_pc, 0x00, ARMThumbCode::LSL_imm_0});
    tc.setInstruction(108, 0x1D, ARMThumbCode::LDRPC_r1);
    tc.setInstruction(96, 0x01, ARMThumbCode::ADD_i8r4);
    tc.setInstruction(66, 0x26, ARMThumbCode::LDRPC_r1);
    tc.removeInstructions(30, 4);
    tc.insertInstructions(30, {0x57, ARMThumbCode::BGE});
    Bytes after = tc.toBytes();
    std::copy(after.begin(), after.end(), ov.begin() + offset);
}

JavaHashSet<int> Gen4RomHandler::getEffectiveEggMoves(const Species* pk) const {
    auto all = getEggMoves();
    JavaHashSet<int> out;
    std::vector<const Species*> stack{pk}, visited;
    while (!stack.empty()) {
        const Species* curr = stack.back();
        stack.pop_back();
        visited.push_back(curr);
        for (const auto& evo : curr->evolutionsTo())
            if (std::find(visited.begin(), visited.end(), evo->from()) == visited.end()) stack.push_back(evo->from());
        auto it = all.find(curr->number());
        if (it != all.end()) out.addAll(it->second);
    }
    return out;
}

std::optional<int> Gen4RomHandler::getMysteryEggMove(const Species* pk) const {
    JavaHashSet<int> ids = getEffectiveEggMoves(pk);
    if (ids.empty()) {
        auto tutors = getMoveTutorMoves();
        auto comp = getMoveTutorCompatibility().at(pk->number());
        for (size_t i = 0; i < tutors.size(); i++)
            if (comp.at(i + 1)) ids.add(tutors[i]);
    }
    if (ids.empty()) {
        auto tms = getTMMoves();
        auto comp = getTMHMCompatibility().at(pk->number());
        for (size_t i = 0; i < tms.size(); i++)
            if (comp.at(i + 1)) ids.add(tms[i]);
    }
    if (ids.empty()) return std::nullopt;
    auto strength = [&](int id) {
        const Move& m = moves_.at(id).value();
        if (m.category == MoveCategory::STATUS) return 0.0;
        double s = m.power * m.hitCount * m.hitratio;
        s *= m.category == MoveCategory::PHYSICAL ? pk->baseStats().attack : pk->baseStats().spatk;
        if (m.type == pk->primaryType(false) || m.type == pk->secondaryType(false)) s *= stabMultiplier;
        return s;
    };
    std::vector<int> v = ids.toVector();
    std::stable_sort(v.begin(), v.end(), [&](int a, int b) { return strength(a) < strength(b); });
    return v.back();
}

void Gen4RomHandler::setMysteryEgg(const StaticEncounter& se) {
    Bytes ov = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    int offset = romEntry_->getInt("MysteryEggCommandOffset");
    if (!isMysteryEggCommandImproved(ov)) improveMysteryEggCommand(ov);
    offset += mysteryEggCommandLength;
    writeLong(ov, offset, se.species()->baseNumber());
    offset += 4;
    auto extra = getMysteryEggMove(se.species()->baseForme());
    writeLong(ov, offset, extra ? moves_.at(*extra)->number : 0);
    writeOverlay(romEntry_->getInt("FieldOvlNumber"), ov);
}

// ---------------- errantes ----------------

void Gen4RomHandler::applyDiamondPearlRoamerPatch() {
    int offset = romEntry_->getInt("RoamingPokemonFunctionStartOffset");
    // la entrada sin usar de Darkrai pasa a ser la constante de Cresselia, cargada relativa al PC
    writeLong(arm9_, offset + 244, species::cresselia);
    arm9_.at(offset + 42) = 0x32;
    arm9_.at(offset + 43) = 0x4F;
    arm9_.at(offset + 44) = 0x00;
    arm9_.at(offset + 45) = 0x00;
}

void Gen4RomHandler::getRoamers(std::vector<StaticEncounter>& statics) {
    const auto& roamers = romEntry_->getRoamingPokemon();
    if (romEntry_->romType() == Type_DP) {
        int offset = romEntry_->getInt("RoamingPokemonFunctionStartOffset");
        if (readWord(arm9_, offset + 44) != 0) applyDiamondPearlRoamerPatch();
    } else {
        int first = roamers.front().speciesCodeOffsets.at(0);
        if (static_cast<int>(arm9_.size()) < first || readWord(arm9_, first) == 0)
            genericIPSPatch(arm9_, "NewRoamerSubroutineTweak");
    }
    for (const auto& r : roamers) {
        StaticEncounter se(pokes_.at(readWord(arm9_, r.speciesCodeOffsets.at(0))));
        se.level = r.levelCodeOffsets.empty() ? 1 : static_cast<int8_t>(arm9_.at(r.levelCodeOffsets[0]));
        statics.push_back(se);
    }
}

void Gen4RomHandler::setRoamers(std::vector<StaticEncounter>::const_iterator& it) {
    for (const auto& r : romEntry_->getRoamingPokemon()) {
        const StaticEncounter& se = *it++;
        const Species* pk = se.species()->baseForme();
        int value = pk->number();
        for (int o : r.speciesCodeOffsets) writeWord(arm9_, o, value);
        for (const auto& e : r.speciesScriptOffsets) writeWord(scriptNarc_.files.at(e.file), e.offset, value);
        int gender = pk->genderRatio == 0xFE ? 1 : 0;  // 0 = macho (vale para los sin género)
        for (const auto& e : r.genderOffsets) writeWord(scriptNarc_.files.at(e.file), e.offset, gender);
        for (int o : r.levelCodeOffsets) arm9_.at(o) = static_cast<uint8_t>(se.level);
    }
}

void Gen4RomHandler::patchDistortionWorldGroundCheck() {
    // Combate con Giratina en el Mundo Distorsión: el tipo de suelo 23 no inicializa la animación de
    // entrada; se le hace usar la misma rama que la hierba normal para que no se cuelgue.
    Bytes f = readOverlay(romEntry_->getInt("FieldOvlNumber"));
    std::string prefix(distortionWorldGroundCheckPrefix);
    int offset = find(f, prefix);
    if (offset > 0) {
        offset += static_cast<int>(prefix.size()) / 2;
        f.at(offset + 2 * 23) = 0x30;
        writeOverlay(romEntry_->getInt("FieldOvlNumber"), f);
    }
}

// ---------------- estáticos ----------------

std::vector<StaticEncounter> Gen4RomHandler::getStaticPokemon() {
    std::vector<StaticEncounter> sp;
    if (!romEntry_->hasStaticPokemonSupport()) return sp;
    const auto& eggOffsets = romEntry_->getArray("StaticEggPokemonOffsets");
    const auto& statics = romEntry_->getStaticPokemon();
    for (size_t i = 0; i < statics.size(); i++) {
        const DSStaticPokemon& s = statics[i];
        Species* base = staticGetPokemon(s);
        int forme = s.formeEntries.empty() ? 0 : signedByte(scriptNarc_.files.at(s.formeEntries[0].file), s.formeEntries[0].offset);
        StaticEncounter se(base);
        se.holder.setAltFormeAllowed();
        se.holder.setFormeNumber(forme);
        se.level = staticGetLevel(s, 0);
        se.isEgg = std::find(eggOffsets.begin(), eggOffsets.end(), static_cast<int>(i)) != eggOffsets.end();
        for (size_t le = 1; le < s.levelEntries.size(); le++) {
            StaticEncounter linked(base);
            linked.level = staticGetLevel(s, static_cast<int>(le));
            se.linkedEncounters.push_back(linked);
        }
        sp.push_back(se);
    }
    const auto& trades = romEntry_->getArray("StaticPokemonTrades");
    if (!trades.empty()) {
        auto narc = readNARC(romEntry_->getFile("InGameTrades"));
        const auto& scripts = romEntry_->getArray("StaticPokemonTradeScripts");
        const auto& levelOffsets = romEntry_->getArray("StaticPokemonTradeLevelOffsets");
        for (size_t i = 0; i < trades.size(); i++) {
            StaticEncounter se(pokes_.at(readLong(narc.files.at(trades[i]), 0)));
            se.level = signedByte(scriptNarc_.files.at(scripts.at(i)), levelOffsets.at(i));
            sp.push_back(se);
        }
    }
    if (romEntry_->getInt("MysteryEggOffset") > 0) {
        Bytes ov = readOverlay(romEntry_->getInt("FieldOvlNumber"));
        int n = isMysteryEggCommandImproved(ov)
                    ? readLong(ov, romEntry_->getInt("MysteryEggCommandOffset") + mysteryEggCommandLength)
                    : ov.at(romEntry_->getInt("MysteryEggOffset"));
        StaticEncounter se(pokes_.at(n));
        se.isEgg = true;
        sp.push_back(se);
    }
    if (romEntry_->getInt("FossilTableOffset") > 0) {
        int base = romEntry_->getInt("FossilTableOffset");
        int level = signedByte(scriptNarc_.files.at(romEntry_->getInt("FossilLevelScriptNumber")),
                               romEntry_->getInt("FossilLevelOffset"));
        Bytes ovl;
        const Bytes* ft = &arm9_;
        if (romEntry_->romType() == Type_HGSS) {
            ovl = readOverlay(romEntry_->getInt("FossilTableOvlNumber"));
            ft = &ovl;
        }
        for (int f = 0; f < fossilCount; f++) {
            StaticEncounter se(pokes_.at(readWord(*ft, base + 2 + f * 4)));
            se.level = level;
            sp.push_back(se);
        }
    }
    if (roamerRandomizationEnabled_) getRoamers(sp);
    return sp;
}

bool Gen4RomHandler::setStaticPokemon(const std::vector<StaticEncounter>& staticPokemon) {
    if (!romEntry_->hasStaticPokemonSupport()) return false;
    const auto& trades = romEntry_->getArray("StaticPokemonTrades");
    size_t meggsize = romEntry_->getInt("MysteryEggOffset") > 0 ? 1 : 0;
    size_t fossilsize = romEntry_->getInt("FossilTableOffset") > 0 ? 7 : 0;
    if (staticPokemon.size() != romEntry_->getStaticPokemon().size() + trades.size() + meggsize + fossilsize +
                                    romEntry_->getRoamingPokemon().size())
        return false;
    auto it = staticPokemon.cbegin();
    for (const DSStaticPokemon& s : romEntry_->getStaticPokemon()) {
        const StaticEncounter& se = *it++;
        staticSetPokemon(s, se.species()->baseForme());
        for (const auto& e : s.formeEntries)
            scriptNarc_.files.at(e.file).at(e.offset) = static_cast<uint8_t>(se.holder.formeNumber());
        staticSetLevel(s, se.level, 0);
        for (size_t i = 0; i < se.linkedEncounters.size(); i++)
            staticSetLevel(s, se.linkedEncounters[i].level, static_cast<int>(i) + 1);
    }
    if (!trades.empty()) {
        auto narc = readNARC(romEntry_->getFile("InGameTrades"));
        const auto& scripts = romEntry_->getArray("StaticPokemonTradeScripts");
        const auto& levelOffsets = romEntry_->getArray("StaticPokemonTradeLevelOffsets");
        for (size_t i = 0; i < trades.size(); i++) {
            const StaticEncounter& se = *it++;
            const Species* t = se.species()->baseForme();
            // especie y habilidad (siempre la 1, por sencillez)
            writeLong(narc.files.at(trades[i]), 0, t->number());
            writeLong(narc.files.at(trades[i]), 0x1C, t->ability1);
            scriptNarc_.files.at(scripts.at(i)).at(levelOffsets.at(i)) = static_cast<uint8_t>(se.level);
            if (i == 1) {
                // Kenya (el Spearow con carta)
                replaceAllStringsInEntry(romEntry_->getInt("KenyaTextOffset"),
                                         {{jc::toUpperCase(pokes_[species::spearow]->name()), t->name()}});
            }
        }
        writeNARC(romEntry_->getFile("InGameTrades"), narc);
    }
    if (romEntry_->getInt("MysteryEggOffset") > 0) setMysteryEgg(*it++);
    if (romEntry_->getInt("FossilTableOffset") > 0) {
        int base = romEntry_->getInt("FossilTableOffset");
        Bytes& levelScript = scriptNarc_.files.at(romEntry_->getInt("FossilLevelScriptNumber"));
        int levelOffset = romEntry_->getInt("FossilLevelOffset");
        if (romEntry_->romType() == Type_HGSS) {
            Bytes ft = readOverlay(romEntry_->getInt("FossilTableOvlNumber"));
            for (int f = 0; f < fossilCount; f++) {
                const StaticEncounter& se = *it++;
                writeWord(ft, base + 2 + f * 4, se.species()->baseNumber());
                levelScript.at(levelOffset) = static_cast<uint8_t>(se.level);
            }
            writeOverlay(romEntry_->getInt("FossilTableOvlNumber"), ft);
        } else {
            for (int f = 0; f < fossilCount; f++) {
                const StaticEncounter& se = *it++;
                writeWord(arm9_, base + 2 + f * 4, se.species()->baseNumber());
                levelScript.at(levelOffset) = static_cast<uint8_t>(se.level);
            }
        }
    }
    if (roamerRandomizationEnabled_) setRoamers(it);
    if (romEntry_->romType() == Type_Plat) patchDistortionWorldGroundCheck();
    return true;
}

std::vector<int> Gen4RomHandler::getMainGameLegendaries() const { return romEntry_->getArray("MainGameLegendaries"); }
std::vector<int> Gen4RomHandler::getSpecialMusicStatics() const { return romEntry_->getArray("SpecialMusicStatics"); }
bool Gen4RomHandler::hasStaticMusicFix() const {
    return romEntry_->hasTweakFile("NewIndexToMusicTweak") || romEntry_->romType() == Type_HGSS;
}

}  // namespace pr::gen4
