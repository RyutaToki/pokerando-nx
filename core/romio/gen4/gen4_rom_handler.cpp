// Gen4RomHandler: carga y guardado de la ROM, objetos, Pokémon, evoluciones y textos.
#include "romio/gen4/gen4_rom_handler.hpp"

#include <algorithm>
#include <stdexcept>

#include "constants/global_constants.hpp"
#include "constants/global_extra.hpp"
#include "romio/gen4/gen4_constants.hpp"
#include "romio/rom_functions.hpp"
#include "text/gen4_text.hpp"
#include "util/crc32.hpp"
#include "util/java_compat.hpp"

namespace pr::gen4 {

namespace {
int readWord(const Bytes& d, int o) { return d.at(o) | (d.at(o + 1) << 8); }
void writeWord(Bytes& d, int o, int v) {
    d.at(o) = static_cast<uint8_t>(v & 0xFF);
    d.at(o + 1) = static_cast<uint8_t>((v >> 8) & 0xFF);
}
int readFullInt(const Bytes& d, int o) {
    return static_cast<int>(static_cast<uint32_t>(d.at(o)) | (static_cast<uint32_t>(d.at(o + 1)) << 8) |
                            (static_cast<uint32_t>(d.at(o + 2)) << 16) | (static_cast<uint32_t>(d.at(o + 3)) << 24));
}
void writeFullInt(Bytes& d, int o, int v) {
    for (int i = 0; i < 4; i++) d.at(o + i) = static_cast<uint8_t>((static_cast<uint32_t>(v) >> (8 * i)) & 0xFF);
}
uint32_t crcOf(const Bytes& b) { return crc32(b.data(), b.size()); }

const Gen4RomEntry* entryFor(const std::string& code, int version) {
    for (const auto& e : Gen4RomEntry::all())
        if (e->romCode() == code && e->version() == version) return e.get();
    return nullptr;
}
}  // namespace

bool Gen4RomHandler::isLoadable(const std::string& ndsCode, int version) { return entryFor(ndsCode, version) != nullptr; }

// ---------------- AbstractDSRomHandler ----------------

Bytes Gen4RomHandler::readFile(const std::string& path) const {
    auto f = baseRom_->getFile(path);
    if (!f) throw std::runtime_error("No existe el archivo " + path + " en la ROM");
    return *f;
}

void Gen4RomHandler::writeFile(const std::string& path, const Bytes& data) { baseRom_->writeFile(path, data); }

Bytes Gen4RomHandler::readOverlay(int number) const {
    auto o = baseRom_->getOverlay(number);
    if (!o) throw std::runtime_error("No existe el overlay " + std::to_string(number));
    return *o;
}

void Gen4RomHandler::writeOverlay(int number, const Bytes& data) { baseRom_->writeOverlay(number, data); }

int Gen4RomHandler::find(const Bytes& data, const std::string& hex) const {
    auto found = romfn::search(data, romfn::hexToBytes(hex));
    if (found.empty()) return -1;
    if (found.size() > 1) return -2;
    return found[0];
}

void Gen4RomHandler::extendARM9(int extendBy, int repointExtendBy, const Bytes& prefix) {
    if (repointExtendBy > extendBy) throw std::invalid_argument("repointExtendBy can't be larger than extendBy");
    if (arm9Extended_) throw std::logic_error("Don't try to extend the ARM9 more than once.");
    const int arm9Offset = gen4::arm9Offset;
    tcmCopyingPointersOffset_ = romfn::searchForFirst(arm9_, 0, prefix);
    tcmCopyingPointersOffset_ += static_cast<int>(prefix.size());
    int oldDestPointersOffset = readFullInt(arm9_, tcmCopyingPointersOffset_) - arm9Offset;
    int itcmSrcOffset = readFullInt(arm9_, tcmCopyingPointersOffset_ + 8) - arm9Offset;
    int itcmSizeOffset = oldDestPointersOffset + 4;
    int oldITCMSize = readFullInt(arm9_, itcmSizeOffset);
    if (oldITCMSize + extendBy > 0x8000) throw std::invalid_argument("Can't extend the section which is copied to ITCM past 32 KiB.");
    int oldDTCMOffset = itcmSrcOffset + oldITCMSize;
    Bytes newARM9 = arm9_;
    newARM9.resize(arm9_.size() + extendBy, 0);
    writeFullInt(newARM9, tcmCopyingPointersOffset_, oldDestPointersOffset + extendBy + arm9Offset);
    writeFullInt(newARM9, tcmCopyingPointersOffset_ + 4, static_cast<int>(newARM9.size()) + arm9Offset);
    writeFullInt(newARM9, itcmSizeOffset, oldITCMSize + extendBy);
    // System.arraycopy sobre el mismo array (se comporta como memmove)
    std::copy_backward(newARM9.begin() + oldDTCMOffset, newARM9.begin() + arm9_.size(), newARM9.begin() + arm9_.size() + extendBy);
    if (repointExtendBy > 0) arm9FreedSpace_.free(itcmSrcOffset + oldITCMSize + extendBy - repointExtendBy, repointExtendBy);
    arm9Extended_ = true;
    arm9_ = std::move(newARM9);
}

bool Gen4RomHandler::genericIPSPatch(Bytes& data, const std::string& ctName) {
    if (!romEntry_->hasTweakFile(ctName))
        throw std::runtime_error("IPS Patch " + ctName + " is not supported by this ROM (" + romEntry_->name() + ").");
    romfn::applyPatch(data, romEntry_->getTweakFile(ctName));
    return true;
}

// ---------------- carga ----------------

void Gen4RomHandler::loadRom(const std::string& fileName) {
    baseRom_ = std::make_unique<nds::NDSRom>(fileName);
    romEntry_ = entryFor(baseRom_->getCode(), static_cast<int8_t>(baseRom_->getVersion()));
    if (!romEntry_) throw std::runtime_error("ROM de 4.ª generación no reconocida");
    arm9_ = baseRom_->getARM9();
    loadedFN_ = fileName;
    loadedROM();
    estimateEvolutionLevels();
}

void Gen4RomHandler::loadedROM() {
    msgNarc_ = readNARC(romEntry_->getFile("Text"));
    scriptNarc_ = readNARC(romEntry_->getFile("Scripts"));
    eventNarc_ = readNARC(romEntry_->getFile("Events"));
    loadItems();
    loadSpeciesStats();
    loadMoves();
    // TODO Gen 4: loadTrainerClasses(), loadTrainers(), loadPokemonPalettes()
    abilityNames_ = getStrings(romEntry_->getInt("AbilityNamesTextOffset"));
    int rt = romEntry_->romType();
    roamerRandomizationEnabled_ = (rt == Type_DP && !romEntry_->getRoamingPokemon().empty()) ||
                                  (rt == Type_Plat && romEntry_->hasTweakFile("NewRoamerSubroutineTweak")) ||
                                  (rt == Type_HGSS && romEntry_->hasTweakFile("NewRoamerSubroutineTweak"));
    computeCRC32sForRom();
    int patchExtendBy = romEntry_->getInt("Arm9PatchExtensionSize");
    int repointExtendBy = romEntry_->getInt("Arm9RepointExtensionSize");
    int extendBy = patchExtendBy + repointExtendBy;
    if (extendBy != 0) extendARM9(extendBy, repointExtendBy, romfn::hexToBytes(romEntry_->getString("TCMCopyingPrefix")));
    if (rt == Type_HGSS && romEntry_->hasTweakFile("NewCatchingTutorialSubroutineTweak"))
        genericIPSPatch(arm9_, "NewCatchingTutorialSubroutineTweak");
}

void Gen4RomHandler::computeCRC32sForRom() {
    actualArm9CRC32_ = crcOf(arm9_);
    romEntry_->overlayExpectedCRC32s().forEach([&](const int& n, const int64_t&) { actualOverlayCRC32s_.put(n, crcOf(readOverlay(n))); });
    for (const auto& key : romEntry_->getFileKeys()) actualFileCRC32s_[key] = crcOf(readFile(romEntry_->getFile(key)));
}

bool Gen4RomHandler::isRomValid() const {
    if (romEntry_->arm9ExpectedCRC32() != static_cast<int64_t>(actualArm9CRC32_)) return false;
    bool ok = true;
    romEntry_->overlayExpectedCRC32s().forEach([&](const int& n, const int64_t& expected) {
        if (ok && expected != static_cast<int64_t>(*actualOverlayCRC32s_.get(n))) ok = false;
    });
    if (!ok) return false;
    for (const auto& key : romEntry_->getFileKeys())
        if (romEntry_->getFileExpectedCRC32(key) != static_cast<int64_t>(actualFileCRC32s_.at(key))) return false;
    return true;
}

void Gen4RomHandler::loadItems() {
    items_.clear();
    items_.push_back(std::nullopt);
    auto names = getStrings(romEntry_->getInt("ItemNamesTextOffset"));
    for (size_t i = 1; i < names.size(); i++) items_.push_back(Item{static_cast<int>(i), names[i]});
    bannedItems().forEach([&](const int& id) {
        if (id < static_cast<int>(items_.size())) items_[id]->allowed = false;
    });
    for (int i = items::tm01; i < items::tm01 + tmCount; i++) items_.at(i)->tm = true;
    badItems().forEach([&](const int& id) {
        if (id < static_cast<int>(items_.size())) items_[id]->bad = true;
    });
}

std::vector<std::string> Gen4RomHandler::readPokemonNames() const {
    std::vector<std::string> names(pokemonCount + 1);
    auto list = getStrings(romEntry_->getInt("PokemonNamesTextOffset"));
    for (int i = 1; i <= pokemonCount; i++) names[i] = list.at(i);
    return names;
}

int Gen4RomHandler::generationOf(const Species& pk) const {
    if (!pk.isBaseForme()) return pk.baseForme()->generation();
    if (pk.number() >= species::turtwig) return 4;
    if (pk.number() >= species::treecko) return 3;
    if (pk.number() >= species::chikorita) return 2;
    return 1;
}

void Gen4RomHandler::loadSpeciesStats() {
    pokeNarc_ = readNARC(romEntry_->getFile("PokemonStats"));
    auto pokeNames = readPokemonNames();
    int formeCount = getFormeCount(romEntry_->romType());
    speciesStorage_.clear();
    pokes_.assign(pokemonCount + formeCount + 1, nullptr);
    auto make = [&](int n) {
        speciesStorage_.push_back(std::make_unique<Species>(n));
        return speciesStorage_.back().get();
    };
    for (int i = 1; i <= pokemonCount; i++) {
        pokes_[i] = make(i);
        loadBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i));
        pokes_[i]->setName(pokeNames[i]);
        pokes_[i]->setGeneration(generationOf(*pokes_[i]));
        if (const char* suf = global::baseFormeSuffix(i)) pokes_[i]->setFormeSuffix(suf);
    }
    int i = pokemonCount + 1;
    for (const auto& [k, fi] : formeMappings()) {
        if (i >= static_cast<int>(pokes_.size())) break;
        pokes_[i] = make(i);
        loadBasicPokeStats(*pokes_[i], pokeNarc_.files.at(k));
        pokes_[i]->setName(pokeNames[fi.baseForme]);
        pokes_[i]->setFormeSuffix(getFormeSuffixByBaseForme(fi.baseForme, fi.formeNumber));
        pokes_[fi.baseForme]->addAltForme(fi.formeNumber, pokes_[i]);
        pokes_[i]->setGeneration(generationOf(*pokes_[i]));
        i++;
    }
    species_.assign(pokes_.begin(), pokes_.begin() + pokemonCount + 1);
    speciesInclFormes_ = pokes_;
    populateEvolutions();
}

void Gen4RomHandler::loadBasicPokeStats(Species& pk, const Bytes& stats) {
    if (pk.number() == species::shedinja) {
        pk.setBaseStats(BaseStats::makeShedinja(stats.at(bsAttackOffset), stats.at(bsDefenseOffset), stats.at(bsSpAtkOffset),
                                                stats.at(bsSpDefOffset), stats.at(bsSpeedOffset)));
    } else {
        pk.setBaseStats(BaseStats(stats.at(bsHPOffset), stats.at(bsAttackOffset), stats.at(bsDefenseOffset), stats.at(bsSpAtkOffset),
                                  stats.at(bsSpDefOffset), stats.at(bsSpeedOffset)));
    }
    pk.setPrimaryType(typeFromByte(stats.at(bsPrimaryTypeOffset)));
    pk.setSecondaryType(typeFromByte(stats.at(bsSecondaryTypeOffset)));
    if (pk.secondaryType(false) == pk.primaryType(false)) pk.setSecondaryType(std::nullopt);
    pk.catchRate = stats.at(bsCatchRateOffset);
    pk.evYield = EVYield::fromWord(readWord(stats, bsEVYieldOffset));
    pk.growthCurve = expCurveFromByte(stats.at(bsGrowthCurveOffset));
    pk.ability1 = stats.at(bsAbility1Offset);
    pk.ability2 = stats.at(bsAbility2Offset);
    auto itemAt = [&](int id) -> std::optional<int> {
        if (id < 0 || id >= static_cast<int>(items_.size()) || !items_[id]) return std::nullopt;
        return id;
    };
    auto item1 = itemAt(readWord(stats, bsCommonHeldItemOffset));
    auto item2 = itemAt(readWord(stats, bsRareHeldItemOffset));
    if (item1 == item2) {
        pk.guaranteedHeldItem = item1;
    } else {
        pk.commonHeldItem = item1;
        pk.rareHeldItem = item2;
    }
    pk.genderRatio = stats.at(bsGenderRatioOffset);
    // new BreedingInfo(primario, secundario, ciclos): el secundario queda vacío si es igual al primario
    pk.breedingInfo.primary = eggGroupFromID(stats.at(bsPrimaryEggGroupOffset));
    EggGroup second = eggGroupFromID(stats.at(bsSecondaryEggGroupOffset));
    pk.breedingInfo.secondary = second == pk.breedingInfo.primary ? std::nullopt : std::optional<EggGroup>(second);
    pk.breedingInfo.eggCycles = stats.at(bsEggCyclesOffset);
    int cosmetic = cosmeticFormsOf(pk.number());
    if (cosmetic > 0 && romEntry_->romType() != Type_DP)
        for (int i = 0; i < cosmetic; i++) pk.addCosmeticAltForme(i + 1);
}

void Gen4RomHandler::populateEvolutions() {
    for (Species* pk : pokes_) {
        if (!pk) continue;
        pk->evolutionsFrom().clear();
        pk->evolutionsTo().clear();
    }
    auto evoNARC = readNARC(romEntry_->getFile("PokemonEvolutions"));
    for (int i = 1; i <= pokemonCount; i++) {
        Species* pk = pokes_[i];
        const Bytes& evoEntry = evoNARC.files.at(i);
        for (int evo = 0; evo < 7; evo++) {
            int method = readWord(evoEntry, evo * 6);
            int sp = readWord(evoEntry, evo * 6 + 4);
            if (method >= 1 && method <= evolutionMethodCount && sp >= 1) {
                EvolutionType et = evolutionTypeFromIndex(method);
                int extraInfo = readWord(evoEntry, evo * 6 + 2);
                auto evol = std::make_shared<Evolution>(pokes_[i], pokes_.at(sp), et, extraInfo);
                bool contains = std::any_of(pk->evolutionsFrom().begin(), pk->evolutionsFrom().end(),
                                            [&](const EvolutionPtr& e) { return e->sameAs(*evol); });
                if (!contains) {
                    pk->evolutionsFrom().push_back(evol);
                    pokes_[sp]->evolutionsTo().push_back(evol);
                }
            }
        }
    }
    addBurmyAltFormeEvolutions();
}

void Gen4RomHandler::writeEvolutions() {
    splitLevelItemEvolutions();
    auto evoNARC = readNARC(romEntry_->getFile("PokemonEvolutions"));
    for (int i = 1; i <= pokemonCount; i++) {
        Bytes& evoEntry = evoNARC.files.at(i);
        Species* pk = pokes_[i];
        if (pk->number() == species::nincada) writeShedinjaEvolution();
        int written = 0;
        for (const auto& evo : pk->evolutionsFrom()) {
            if (evo->type() == EvolutionType::NONE) continue;
            writeWord(evoEntry, written * 6, evolutionTypeToIndex(evo->type()));
            writeWord(evoEntry, written * 6 + 2, evo->extraInfo());
            writeWord(evoEntry, written * 6 + 4, evo->to()->number());
            if (++written == 7) break;
        }
        while (written < 7) {
            writeWord(evoEntry, written * 6, 0);
            writeWord(evoEntry, written * 6 + 2, 0);
            writeWord(evoEntry, written * 6 + 4, 0);
            written++;
        }
    }
    writeNARC(romEntry_->getFile("PokemonEvolutions"), evoNARC);
    mergeLevelItemEvolutions();
}

void Gen4RomHandler::writeShedinjaEvolution() {
    Species* nincada = pokes_[species::nincada];
    if (nincada->evolutionsFrom().size() < 2) return;
    Species* extra = nincada->evolutionsFrom()[1]->to();
    int offset = find(arm9_, std::string(shedinjaSpeciesLocator));
    if (offset > 0) {
        int low, high;
        if (extra->number() < 256) {
            low = extra->number();
            high = 0;
        } else {
            low = 255;
            high = extra->number() - 255;
        }
        arm9_[offset] = static_cast<uint8_t>(low);
        arm9_[offset + 1] = 0x20;
        arm9_[offset + 2] = static_cast<uint8_t>(high);
        arm9_[offset + 3] = 0x30;
    }
}

SpeciesSet Gen4RomHandler::getAltFormes() const {
    int formeCount = getFormeCount(romEntry_->romType());
    std::vector<Species*> v(pokes_.begin() + pokemonCount + 1, pokes_.begin() + pokemonCount + formeCount + 1);
    return SpeciesSet::of(v);
}

SpeciesSet Gen4RomHandler::getIrregularFormes() const { return SpeciesSet(); }

// ---------------- textos ----------------

std::vector<std::string> Gen4RomHandler::getStrings(int index) const {
    auto d = text4::readTexts(msgNarc_.files.at(index));
    lastStringsCompressed_ = d.compressed;
    return d.strings;
}

void Gen4RomHandler::setStrings(int index, const std::vector<std::string>& strings, bool compressed) {
    msgNarc_.files.at(index) = text4::makeEncryptedFile(strings, compressed);
}

// ---------------- guardado ----------------

void Gen4RomHandler::prepareSaveRom() {
    // AbstractRomHandler.prepareSaveRom: guarda todo lo cargado
    saveMoves();
    saveSpeciesStats();
    // TODO Gen 4: savePokemonPalettes(), saveTrainerClasses(), saveTrainers()
    // AbstractDSRomHandler: la ARM9
    baseRom_->writeARM9(arm9_);
    // Gen4RomHandler
    writeNARC(romEntry_->getFile("Text"), msgNarc_);
    writeNARC(romEntry_->getFile("Scripts"), scriptNarc_);
    writeNARC(romEntry_->getFile("Events"), eventNarc_);
}

void Gen4RomHandler::saveRomTo(const std::string& fileName) {
    prepareSaveRom();
    baseRom_->saveTo(fileName);
}

void Gen4RomHandler::saveSpeciesStats() {
    int namesIdx = romEntry_->getInt("PokemonNamesTextOffset");
    auto namesList = getStrings(namesIdx);
    int formeCount = getFormeCount(romEntry_->romType());
    if (jc::equalsIgnoreCase(romEntry_->getString("HasExtraPokemonNames"), "Yes")) {
        auto namesList2 = getStrings(namesIdx + 1);
        for (int i = 1; i <= pokemonCount + formeCount; i++) {
            if (i > pokemonCount) {
                saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i + formeOffset));
                continue;
            }
            saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i));
            std::string oldName = namesList.at(i);
            namesList[i] = pokes_[i]->name();
            namesList2.at(i) = romfn::replaceAll(namesList2.at(i), oldName, pokes_[i]->name());
        }
        setStrings(namesIdx + 1, namesList2, false);
    } else {
        for (int i = 1; i <= pokemonCount + formeCount; i++) {
            if (i > pokemonCount) {
                saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i + formeOffset));
                continue;
            }
            saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i));
            namesList.at(i) = pokes_[i]->name();
        }
    }
    setStrings(namesIdx, namesList, false);
    writeNARC(romEntry_->getFile("PokemonStats"), pokeNarc_);
    writeEvolutions();
}

void Gen4RomHandler::saveBasicPokeStats(const Species& pk, Bytes& stats) {
    const BaseStats& bs = pk.baseStats();
    stats.at(bsHPOffset) = static_cast<uint8_t>(bs.getHp());
    stats.at(bsAttackOffset) = static_cast<uint8_t>(bs.attack);
    stats.at(bsDefenseOffset) = static_cast<uint8_t>(bs.defense);
    stats.at(bsSpeedOffset) = static_cast<uint8_t>(bs.speed);
    stats.at(bsSpAtkOffset) = static_cast<uint8_t>(bs.spatk);
    stats.at(bsSpDefOffset) = static_cast<uint8_t>(bs.spdef);
    stats.at(bsPrimaryTypeOffset) = static_cast<uint8_t>(typeToByte(pk.primaryType(false)));
    if (!pk.secondaryType(false)) stats.at(bsSecondaryTypeOffset) = stats.at(bsPrimaryTypeOffset);
    else stats.at(bsSecondaryTypeOffset) = static_cast<uint8_t>(typeToByte(pk.secondaryType(false)));
    stats.at(bsCatchRateOffset) = static_cast<uint8_t>(pk.catchRate);
    writeWord(stats, bsEVYieldOffset, pk.evYield.toWord());
    stats.at(bsGrowthCurveOffset) = static_cast<uint8_t>(expCurveToByte(*pk.growthCurve));
    stats.at(bsAbility1Offset) = static_cast<uint8_t>(pk.ability1);
    stats.at(bsAbility2Offset) = static_cast<uint8_t>(pk.ability2);
    if (pk.guaranteedHeldItem) {
        writeWord(stats, bsCommonHeldItemOffset, *pk.guaranteedHeldItem);
        writeWord(stats, bsRareHeldItemOffset, *pk.guaranteedHeldItem);
    } else {
        writeWord(stats, bsCommonHeldItemOffset, pk.commonHeldItem.value_or(0));
        writeWord(stats, bsRareHeldItemOffset, pk.rareHeldItem.value_or(0));
    }
    const BreedingInfo& bi = pk.breedingInfo;
    stats.at(bsPrimaryEggGroupOffset) = static_cast<uint8_t>(eggGroupToID(bi.primary));
    stats.at(bsSecondaryEggGroupOffset) = static_cast<uint8_t>(bi.secondary ? eggGroupToID(*bi.secondary) : eggGroupToID(bi.primary));
    stats.at(bsEggCyclesOffset) = static_cast<uint8_t>(bi.eggCycles);
}

// ---------------- datos simples ----------------

std::vector<std::pair<int, std::vector<int>>> Gen4RomHandler::getAbilityVariations() const {
    JavaHashMap<int, std::vector<int>> m;
    m.put(abilities::insomnia, {abilities::insomnia, abilities::vitalSpirit});
    m.put(abilities::clearBody, {abilities::clearBody, abilities::whiteSmoke});
    m.put(abilities::hugePower, {abilities::hugePower, abilities::purePower});
    m.put(abilities::battleArmor, {abilities::battleArmor, abilities::shellArmor});
    m.put(abilities::cloudNine, {abilities::cloudNine, abilities::airLock});
    m.put(abilities::filter, {abilities::filter, abilities::solidRock});
    std::vector<std::pair<int, std::vector<int>>> out;
    m.forEach([&](const int& k, const std::vector<int>& v) { out.emplace_back(k, v); });
    return out;
}

std::vector<int> Gen4RomHandler::getUselessAbilities() const { return {abilities::forecast, abilities::multitype, abilities::flowerGift}; }
int Gen4RomHandler::highestAbilityIndex() const { return gen4::highestAbilityIndex; }
bool Gen4RomHandler::isTrainerPokemonAlwaysUseAbility1() const { return romEntry_->romType() != Type_HGSS; }
int Gen4RomHandler::internalStringLength(const std::string& s) const { return static_cast<int>(jc::utf8ToU32(s).size()); }
int Gen4RomHandler::getMaxMoveNameLength() const { return 15; }
bool Gen4RomHandler::isEnglish() const { return romCode().size() >= 4 && romCode()[3] == 'E'; }
int Gen4RomHandler::maxTrainerNameLength() const { return 10; }
int Gen4RomHandler::maxTrainerClassNameLength() const { return 12; }
std::vector<ExpCurve> Gen4RomHandler::getExpCurves() const {
    return {ExpCurve::MEDIUM_FAST, ExpCurve::MEDIUM_SLOW, ExpCurve::FAST, ExpCurve::SLOW, ExpCurve::ERRATIC, ExpCurve::FLUCTUATING};
}

}  // namespace pr::gen4
