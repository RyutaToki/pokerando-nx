// Gen5RomHandler: carga y guardado de la ROM, objetos, Pokémon, evoluciones y textos.
#include "romio/gen5/gen5_rom_handler.hpp"

#include <algorithm>
#include <stdexcept>

#include "constants/global_constants.hpp"
#include "constants/global_extra.hpp"
#include "romio/gen5/gen5_constants.hpp"
#include "romio/rom_functions.hpp"
#include "text/gen5_text.hpp"
#include "util/crc32.hpp"
#include "util/java_compat.hpp"

namespace pr::gen5 {

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

const Gen5RomEntry* entryFor(const std::string& code, int version) {
    for (const auto& e : Gen5RomEntry::all())
        if (e->romCode() == code && e->version() == version) return e.get();
    return nullptr;
}
}  // namespace

bool Gen5RomHandler::isLoadable(const std::string& ndsCode, int version) { return entryFor(ndsCode, version) != nullptr; }

// ---------------- AbstractDSRomHandler ----------------

Bytes Gen5RomHandler::readFile(const std::string& path) const {
    auto f = baseRom_->getFile(path);
    if (!f) throw std::runtime_error("No existe el archivo " + path + " en la ROM");
    return *f;
}

void Gen5RomHandler::writeFile(const std::string& path, const Bytes& data) { baseRom_->writeFile(path, data); }

Bytes Gen5RomHandler::readOverlay(int number) const {
    auto o = baseRom_->getOverlay(number);
    if (!o) throw std::runtime_error("No existe el overlay " + std::to_string(number));
    return *o;
}

void Gen5RomHandler::writeOverlay(int number, const Bytes& data) { baseRom_->writeOverlay(number, data); }

int Gen5RomHandler::find(const Bytes& data, const std::string& hex) const {
    auto found = romfn::search(data, romfn::hexToBytes(hex));
    if (found.empty()) return -1;
    if (found.size() > 1) return -2;
    return found[0];
}

void Gen5RomHandler::extendARM9(int extendBy, int repointExtendBy, const Bytes& prefix) {
    if (repointExtendBy > extendBy) throw std::invalid_argument("repointExtendBy can't be larger than extendBy");
    if (arm9Extended_) throw std::logic_error("Don't try to extend the ARM9 more than once.");
    const int arm9Offset = gen5::arm9Offset;
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

// en Gen 5, a diferencia de Gen 4, un parche que la ROM no tiene se ignora sin error
bool Gen5RomHandler::genericIPSPatch(Bytes& data, const std::string& ctName) {
    if (!romEntry_->hasTweakFile(ctName)) return false;
    romfn::applyPatch(data, romEntry_->getTweakFile(ctName));
    return true;
}

// ---------------- carga ----------------

void Gen5RomHandler::loadRom(const std::string& fileName) {
    baseRom_ = std::make_unique<nds::NDSRom>(fileName);
    romEntry_ = entryFor(baseRom_->getCode(), static_cast<int8_t>(baseRom_->getVersion()));
    if (!romEntry_) throw std::runtime_error("ROM de 5.ª generación no reconocida");
    arm9_ = baseRom_->getARM9();
    loadedFN_ = fileName;
    loadedROM();
    estimateEvolutionLevels();
}

void Gen5RomHandler::loadedROM() {
    stringsNarc_ = readNARC(romEntry_->getFile("TextStrings"));
    storyTextNarc_ = readNARC(romEntry_->getFile("TextStory"));
    scriptNarc_ = readNARC(romEntry_->getFile("Scripts"));
    if (romEntry_->romType() == Type_BW2) shopNarc_ = readNARC(romEntry_->getFile("ShopItems"));
    loadItems();
    loadSpeciesStats();
    loadMoves();
    loadTrainerClasses();
    loadTrainers();
    loadPokemonPalettes();
    abilityNames_ = getStrings(false, romEntry_->getInt("AbilityNamesTextOffset"));
    const auto& names = romEntry_->romType() == Type_BW ? bw1ShopNames : bw2ShopNames;
    shopNames_.assign(names.begin(), names.end());
    loadedWildMapNames_ = false;
    computeCRC32sForRom();
    // toda la ampliación de la ARM9 se hace aquí (hueco para parches y para datos movidos)
    int patchExtendBy = romEntry_->getInt("Arm9PatchExtensionSize");
    int repointExtendBy = romEntry_->getInt("Arm9RepointExtensionSize");
    int extendBy = patchExtendBy + repointExtendBy;
    if (extendBy != 0) extendARM9(extendBy, repointExtendBy, romfn::hexToBytes(romEntry_->getString("TCMCopyingPrefix")));
}

void Gen5RomHandler::computeCRC32sForRom() {
    actualArm9CRC32_ = crcOf(arm9_);
    romEntry_->overlayExpectedCRC32s().forEach([&](const int& n, const int64_t&) { actualOverlayCRC32s_.put(n, crcOf(readOverlay(n))); });
    for (const auto& key : romEntry_->getFileKeys()) actualFileCRC32s_[key] = crcOf(readFile(romEntry_->getFile(key)));
}

bool Gen5RomHandler::isRomValid() const {
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

void Gen5RomHandler::loadItems() {
    items_.clear();
    items_.push_back(std::nullopt);
    auto names = getStrings(false, romEntry_->getInt("ItemNamesTextOffset"));
    for (size_t i = 1; i < names.size(); i++) items_.push_back(Item{static_cast<int>(i), names[i]});
    bannedItems().forEach([&](const int& id) {
        if (id < static_cast<int>(items_.size())) items_[id]->allowed = false;
    });
    for (int i = items::tm01; i <= items::tm92; i++) items_.at(i)->tm = true;
    for (int i = items::tm93; i <= items::tm95; i++) items_.at(i)->tm = true;
    getBadItems(romEntry_->romType()).forEach([&](const int& id) {
        if (id < static_cast<int>(items_.size())) items_[id]->bad = true;
    });
}

std::vector<std::string> Gen5RomHandler::readPokemonNames() const {
    std::vector<std::string> names(pokemonCount + 1);
    auto list = getStrings(false, romEntry_->getInt("PokemonNamesTextOffset"));
    for (int i = 1; i <= pokemonCount; i++) names[i] = list.at(i);
    return names;
}

int Gen5RomHandler::generationOf(const Species& pk) const {
    if (!pk.isBaseForme()) return pk.baseForme()->generation();
    if (pk.number() >= species::victini) return 5;
    if (pk.number() >= species::turtwig) return 4;
    if (pk.number() >= species::treecko) return 3;
    if (pk.number() >= species::chikorita) return 2;
    return 1;
}

void Gen5RomHandler::loadSpeciesStats() {
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
    for (const auto& [k, fi] : formeMappings_) {
        pokes_.at(i) = make(i);  // Java: pokes[i] fuera de rango lanzaría igual
        loadBasicPokeStats(*pokes_[i], pokeNarc_.files.at(k));
        pokes_[i]->setName(pokeNames[fi.baseForme]);
        pokes_[i]->setFormeSuffix(getFormeSuffixByBaseForme(fi.baseForme, fi.formeNumber));
        pokes_[fi.baseForme]->addAltForme(fi.formeNumber, pokes_[i]);
        if (i == species::Gen5Formes::keldeoCosmetic1) pokes_[i]->setEssentiallyCosmetic();
        pokes_[i]->setGeneration(generationOf(*pokes_[i]));
        i++;
    }
    species_.assign(pokes_.begin(), pokes_.begin() + pokemonCount + 1);
    speciesInclFormes_ = pokes_;
    populateEvolutions();
}

void Gen5RomHandler::loadBasicPokeStats(Species& pk, const Bytes& stats) {
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
    pk.growthCurve = expCurveFromByte(static_cast<int8_t>(stats.at(bsGrowthCurveOffset)));
    pk.ability1 = stats.at(bsAbility1Offset);
    pk.ability2 = stats.at(bsAbility2Offset);
    pk.ability3 = stats.at(bsAbility3Offset);
    // items.get(id): Java lanza si el índice no existe
    auto itemAt = [&](int id) -> std::optional<int> {
        if (!items_.at(id)) return std::nullopt;
        return id;
    };
    auto item1 = itemAt(readWord(stats, bsCommonHeldItemOffset));
    auto item2 = itemAt(readWord(stats, bsRareHeldItemOffset));
    if (item1 == item2) {
        pk.guaranteedHeldItem = item1;
    } else {
        pk.commonHeldItem = item1;
        pk.rareHeldItem = item2;
        pk.darkGrassHeldItem = itemAt(readWord(stats, bsDarkGrassHeldItemOffset));
    }
    pk.genderRatio = stats.at(bsGenderRatioOffset);
    // new BreedingInfo(primario, secundario, ciclos): el secundario queda vacío si es igual al primario
    pk.breedingInfo.primary = eggGroupFromID(stats.at(bsPrimaryEggGroupOffset));
    EggGroup second = eggGroupFromID(stats.at(bsSecondaryEggGroupOffset));
    pk.breedingInfo.secondary = second == pk.breedingInfo.primary ? std::nullopt : std::optional<EggGroup>(second);
    pk.breedingInfo.eggCycles = stats.at(bsEggCyclesOffset);

    int formeCount = stats.at(bsFormeCountOffset);
    if (formeCount > 1) {
        graphicalFormeCounts_[pk.number()] = formeCount;
        formeGraphicsIndices_[pk.number()] = romEntry_->getInt("FormeGraphicsOffset") + readWord(stats, bsFormeSpriteOffset);
        int firstFormeOffset = readWord(stats, bsFormeOffset);
        if (firstFormeOffset != 0) {
            // supone que las formas están en el mismo orden que sus números
            for (int i = 1; i < formeCount; i++) formeMappings_[firstFormeOffset + i - 1] = FormeInfo{pk.number(), i};
        } else if (pk.number() <= species::genesect) {
            bool ignore = std::find(ignoreTrueCosmeticFormes.begin(), ignoreTrueCosmeticFormes.end(), pk.number()) !=
                          ignoreTrueCosmeticFormes.end();
            for (int i = 0; i < formeCount; i++) {
                pk.addCosmeticAltForme(i + 1);
                if (ignore) pk.setIgnoreCosmeticAltForme(i + 1);
            }
        }
    }
}

void Gen5RomHandler::populateEvolutions() {
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
                if (et == EvolutionType::HIGH_BEAUTY) continue;  // quita la evolución «dividida» de Feebas
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

void Gen5RomHandler::writeEvolutions() {
    splitLevelItemEvolutions();
    auto evoNARC = readNARC(romEntry_->getFile("PokemonEvolutions"));
    for (int i = 1; i <= pokemonCount; i++) {
        Bytes& evoEntry = evoNARC.files.at(i);
        Species* pk = pokes_[i];
        if (pk->number() == species::nincada && romEntry_->hasTweakFile("ShedinjaEvolutionTweak")) writeShedinjaEvolution();
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

void Gen5RomHandler::writeShedinjaEvolution() {
    Species* nincada = pokes_[species::nincada];
    // con «Limitar Pokémon» o evoluciones aleatorias en cada nivel Nincada puede quedarse sin las suyas
    if (nincada->evolutionsFrom().size() < 2) return;
    Species* extra = nincada->evolutionsFrom()[1]->to();
    int ovl = romEntry_->getInt("EvolutionOvlNumber");
    Bytes evolutionOverlay = readOverlay(ovl);
    genericIPSPatch(evolutionOverlay, "ShedinjaEvolutionOvlTweak");
    writeOverlay(ovl, evolutionOverlay);
    genericIPSPatch(arm9_, "ShedinjaEvolutionTweak");  // la ARM9 ya se amplió en loadedROM
    int offset = romEntry_->getInt("ShedinjaSpeciesOffset");
    if (offset > 0) writeFullInt(arm9_, offset, extra->number());
}

SpeciesSet Gen5RomHandler::getAltFormes() const {
    int formeCount = getFormeCount(romEntry_->romType());
    std::vector<Species*> v(pokes_.begin() + pokemonCount + 1, pokes_.begin() + pokemonCount + formeCount + 1);
    return SpeciesSet::of(v);
}

SpeciesSet Gen5RomHandler::getIrregularFormes() const {
    std::vector<Species*> v;
    for (int i : gen5::getIrregularFormes(romEntry_->romType())) v.push_back(pokes_.at(i));
    return SpeciesSet::of(v);
}

// ---------------- textos ----------------

std::vector<std::string> Gen5RomHandler::getStrings(bool isStoryText, int index) const {
    const auto& narc = isStoryText ? storyTextNarc_ : stringsNarc_;
    return text5::readTexts(narc.files.at(index));
}

void Gen5RomHandler::setStrings(bool isStoryText, int index, const std::vector<std::string>& strings) {
    auto& narc = isStoryText ? storyTextNarc_ : stringsNarc_;
    narc.files.at(index) = text5::saveEntry(narc.files.at(index), strings);
}

// ---------------- guardado ----------------

void Gen5RomHandler::prepareSaveRom() {
    // AbstractRomHandler.prepareSaveRom: guarda todo lo cargado
    saveMoves();
    saveSpeciesStats();
    savePokemonPalettes();
    saveTrainerClasses();
    saveTrainers();
    // AbstractDSRomHandler: la ARM9
    baseRom_->writeARM9(arm9_);
    // Gen5RomHandler
    writeNARC(romEntry_->getFile("TextStrings"), stringsNarc_);
    writeNARC(romEntry_->getFile("TextStory"), storyTextNarc_);
    writeNARC(romEntry_->getFile("Scripts"), scriptNarc_);
}

void Gen5RomHandler::saveRomTo(const std::string& fileName) {
    prepareSaveRom();
    baseRom_->saveTo(fileName);
}

void Gen5RomHandler::saveSpeciesStats() {
    int namesIdx = romEntry_->getInt("PokemonNamesTextOffset");
    auto nameList = getStrings(false, namesIdx);
    int formeCount = getFormeCount(romEntry_->romType());
    int formeOffset = getFormeOffset(romEntry_->romType());
    for (int i = 1; i <= pokemonCount + formeCount; i++) {
        if (i > pokemonCount) {
            saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i + formeOffset));
            continue;
        }
        saveBasicPokeStats(*pokes_[i], pokeNarc_.files.at(i));
        nameList.at(i) = pokes_[i]->name();
    }
    setStrings(false, namesIdx, nameList);
    writeNARC(romEntry_->getFile("PokemonStats"), pokeNarc_);
    writeEvolutions();
}

void Gen5RomHandler::saveBasicPokeStats(const Species& pk, Bytes& stats) {
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
    stats.at(bsAbility3Offset) = static_cast<uint8_t>(pk.ability3);
    if (pk.guaranteedHeldItem) {
        writeWord(stats, bsCommonHeldItemOffset, *pk.guaranteedHeldItem);
        writeWord(stats, bsRareHeldItemOffset, *pk.guaranteedHeldItem);
        writeWord(stats, bsDarkGrassHeldItemOffset, 0);
    } else {
        writeWord(stats, bsCommonHeldItemOffset, pk.commonHeldItem.value_or(0));
        writeWord(stats, bsRareHeldItemOffset, pk.rareHeldItem.value_or(0));
        writeWord(stats, bsDarkGrassHeldItemOffset, pk.darkGrassHeldItem.value_or(0));
    }
    const BreedingInfo& bi = pk.breedingInfo;
    stats.at(bsPrimaryEggGroupOffset) = static_cast<uint8_t>(eggGroupToID(bi.primary));
    stats.at(bsSecondaryEggGroupOffset) = static_cast<uint8_t>(bi.secondary ? eggGroupToID(*bi.secondary) : eggGroupToID(bi.primary));
    stats.at(bsEggCyclesOffset) = static_cast<uint8_t>(bi.eggCycles);
}

// ---------------- datos simples ----------------

std::vector<std::pair<int, std::vector<int>>> Gen5RomHandler::getAbilityVariations() const {
    JavaHashMap<int, std::vector<int>> m;
    m.put(abilities::insomnia, {abilities::insomnia, abilities::vitalSpirit});
    m.put(abilities::clearBody, {abilities::clearBody, abilities::whiteSmoke});
    m.put(abilities::hugePower, {abilities::hugePower, abilities::purePower});
    m.put(abilities::battleArmor, {abilities::battleArmor, abilities::shellArmor});
    m.put(abilities::cloudNine, {abilities::cloudNine, abilities::airLock});
    m.put(abilities::filter, {abilities::filter, abilities::solidRock});
    m.put(abilities::roughSkin, {abilities::roughSkin, abilities::ironBarbs});
    m.put(abilities::moldBreaker, {abilities::moldBreaker, abilities::turboblaze, abilities::teravolt});
    std::vector<std::pair<int, std::vector<int>>> out;
    m.forEach([&](const int& k, const std::vector<int>& v) { out.emplace_back(k, v); });
    return out;
}

std::vector<int> Gen5RomHandler::getUselessAbilities() const { return uselessAbilities; }
int Gen5RomHandler::highestAbilityIndex() const { return gen5::highestAbilityIndex; }

// ssd: la longitud cuenta cada «\xNNNN» como un carácter
int Gen5RomHandler::internalStringLength(const std::string& s) const {
    int len = text5::javaLength(s);
    size_t offs = 0;
    while ((offs = s.find("\\x", offs)) != std::string::npos) {
        len -= 5;
        offs++;
    }
    return len;
}

bool Gen5RomHandler::isEnglish() const { return romCode().size() >= 4 && romCode()[3] == 'O'; }
std::vector<ExpCurve> Gen5RomHandler::getExpCurves() const {
    return {ExpCurve::MEDIUM_FAST, ExpCurve::MEDIUM_SLOW, ExpCurve::FAST, ExpCurve::SLOW, ExpCurve::ERRATIC, ExpCurve::FLUCTUATING};
}
std::string Gen5RomHandler::getPaletteFilesID() const { return "BW"; }
std::vector<int> Gen5RomHandler::getFieldMoves() const { return fieldMoves; }
std::vector<int> Gen5RomHandler::getEarlyRequiredHMMoves() const {
    return romEntry_->romType() == Type_BW2 ? std::vector<int>{} : bw1EarlyRequiredHMMoves;
}
std::vector<int> Gen5RomHandler::getDoublesTrainerClasses() const { return romEntry_->getArray("DoublesTrainerClasses"); }
std::vector<int> Gen5RomHandler::getMainGameLegendaries() const { return romEntry_->getArray("MainGameLegendaries"); }
std::vector<int> Gen5RomHandler::getSpecialMusicStatics() const { return romEntry_->getArray("SpecialMusicStatics"); }
ItemSet Gen5RomHandler::getOPShopItems() const {
    std::vector<int> ids;
    opShopItems().forEach([&](const int& i) { ids.push_back(i); });
    return idsToItemSet(ids);
}

}  // namespace pr::gen5
