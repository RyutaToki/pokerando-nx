#include "gamedata/species.hpp"

#include <algorithm>
#include <deque>
#include <stdexcept>

#include "constants/species_ids.hpp"
#include "util/java_random.hpp"

namespace pr {

int JavaHashCode<Species*>::operator()(const Species* s) const { return 31 + s->number(); }
bool SpeciesPtrEq::operator()(const Species* a, const Species* b) const {
    return a == b || (a && b && a->number() == b->number());
}

namespace {
using namespace species;
const std::vector<int> kLegendaries = {
    articuno, zapdos, moltres, mewtwo, mew, raikou, entei, suicune, lugia, hoOh, celebi, regirock, regice,
    registeel, latias, latios, kyogre, groudon, rayquaza, jirachi, deoxys, uxie, mesprit, azelf, dialga, palkia,
    heatran, regigigas, giratina, cresselia, phione, manaphy, darkrai, shaymin, arceus, victini, cobalion,
    terrakion, virizion, tornadus, thundurus, reshiram, zekrom, landorus, kyurem, keldeo, meloetta, genesect,
    xerneas, yveltal, zygarde, diancie, hoopa, volcanion, typeNull, silvally, tapuKoko, tapuLele, tapuBulu,
    tapuFini, cosmog, cosmoem, solgaleo, lunala, necrozma, magearna, marshadow, zeraora};
const std::vector<int> kStrongLegendaries = {mewtwo, lugia, hoOh, kyogre, groudon, rayquaza, dialga,
                                              palkia, regigigas, giratina, arceus, reshiram, zekrom, kyurem,
                                              xerneas, yveltal, cosmog, cosmoem, solgaleo, lunala};
const std::vector<int> kUltraBeasts = {nihilego, buzzwole, pheromosa, xurkitree, celesteela, kartana,
                                        guzzlord, poipole, naganadel, stakataka, blacephalon};
bool inList(const std::vector<int>& l, int v) { return std::find(l.begin(), l.end(), v) != l.end(); }
}  // namespace

Species::Species(int number) : number_(number) {
    // new HashMap<>(Map.of(0, this))
    JavaHashMap<int, Species*> init;
    init.put(0, this);
    formes_.putAll(init);
}

void Species::setConceptualBaseForme(Species* c) {
    if (conceptualBaseForme_) throw std::logic_error(numberAndFullName() + " ya tiene forma base conceptual");
    if (isBaseForme()) throw std::logic_error(numberAndFullName() + " debe ser una forma alternativa");
    if (c->isBaseForme()) throw std::invalid_argument("conceptualBaseForme debe ser una forma alternativa");
    if (c->baseForme_ != baseForme_) throw std::logic_error("deben compartir la misma forma base");
    conceptualBaseForme_ = c;
    formeSuffix_ = c->formeSuffix_ + formeSuffix_;
}

void Species::addAltForme(int formeNumber, Species* altForme) {
    if (altForme->number_ == number_) throw std::invalid_argument("No se puede añadir como su propia forma");
    if (!isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma alternativa");
    if (formes_.containsKey(formeNumber)) throw std::logic_error(numberAndFullName() + " ya tiene esa forma");
    if (!altForme->isBaseForme()) throw std::logic_error("la forma ya pertenece a otra especie");
    formes_.put(formeNumber, altForme);
    altForme->baseForme_ = this;
    altForme->formeNumber_ = formeNumber;
}

void Species::addCosmeticAltForme(int formeNumber) {
    if (!isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma alternativa");
    if (formes_.containsKey(formeNumber)) throw std::logic_error(numberAndFullName() + " ya tiene esa forma");
    formes_.put(formeNumber, this);
    cosmeticFormeNumbers_.push_back(formeNumber);
}

Species* Species::getForme(int formeNumber) {
    if (!isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma alternativa");
    Species* const* f = formes_.get(formeNumber);
    if (!f) throw std::out_of_range(numberAndFullName() + " no tiene la forma " + std::to_string(formeNumber));
    return *f;
}

SpeciesSet Species::altFormes() {
    std::vector<Species*> values;
    formes_.forEach([&](const int&, Species* const& v) { values.push_back(v); });
    return SpeciesSet::of(values).filter([this](Species* pk) { return pk->number() != number_; });
}

int Species::randomCosmeticFormeNumber(JavaRandom& random) const {
    auto pick = [&] { return cosmeticFormeNumbers_[random.nextInt(static_cast<int>(cosmeticFormeNumbers_.size()))]; };
    int f = pick();
    while (inList(ignoreCosmeticFormeNumbers_, f)) f = pick();
    return f;
}

void Species::setMegaEvolution(std::optional<int> item) {
    if (isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma base");
    megaEvolution_ = true;
    megaEvolutionItem_ = item;
}
void Species::setAlolan() {
    if (isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma base");
    alolan_ = true;
}
void Species::setEssentiallyCosmetic() {
    if (isBaseForme()) throw std::logic_error(numberAndFullName() + " es una forma base");
    essentiallyCosmetic_ = true;
    cosmeticFormeNumbers_.push_back(formeNumber_);
}

bool Species::isLegendary() const { return inList(kLegendaries, baseForme()->number_); }
bool Species::isStrongLegendary() const { return inList(kStrongLegendaries, baseForme()->number_); }
bool Species::isUltraBeast() const { return inList(kUltraBeasts, number_); }

void Species::setPrimaryType(std::optional<Type> t) {
    primary_ = t;
    if (!hasSetPrimary_) {
        origPrimary_ = t;
        hasSetPrimary_ = true;
    }
}

void Species::setSecondaryType(std::optional<Type> t) {
    if (hasSetPrimary_ && t == primary_) t.reset();  // para no tener NORMAL/NORMAL y similares
    secondary_ = t;
    if (!hasSetSecondary_) {
        origSecondary_ = t;
        hasSetSecondary_ = true;
    }
}

bool Species::hasType(std::optional<Type> t, bool useOriginal) const {
    if (!t) return false;
    return primaryType(useOriginal) == t || secondaryType(useOriginal) == t;
}

bool Species::hasSharedType(const Species& o) const {
    return primary_ == o.primary_ || primary_ == o.secondary_ ||
           (secondary_ && (secondary_ == o.primary_ || secondary_ == o.secondary_));
}

void Species::setBaseStats(const BaseStats& bs) {
    baseStats_ = bs;
    if (originalBST_ == -1) originalBST_ = bs.bst();
}

const std::vector<EvolutionPtr>& Species::evolutionsOfForme(bool to) {
    auto acc = [to](Species* s) -> std::vector<EvolutionPtr>& { return to ? s->evolutionsTo_ : s->evolutionsFrom_; };
    Species* species = this;
    if (acc(species).empty()) {
        if (isBaseForme()) {
            for (Species* alt : altFormes().toVector()) {
                if (!acc(alt).empty()) {
                    species = alt;
                    break;
                }
            }
        } else {
            while (!species->isBaseForme() && acc(species->conceptualBaseForme()).empty()) {
                species = species->conceptualBaseForme();
            }
            species = species->conceptualBaseForme();
        }
    }
    return acc(species);
}

bool Species::isLegalEvolutionAtLevel(int level, double mod) {
    bool legal = true;
    for (const auto& evo : evolutionsOfForme(true)) {
        if (level >= mod * evo->estimatedEvoLvl()) return true;
        legal = false;
    }
    return legal;
}

bool Species::hasLegalEvolutionAtLevel(int level, double mod) {
    for (const auto& evo : evolutionsOfForme(false)) {
        if (level >= mod * evo->estimatedEvoLvl()) return true;
    }
    return false;
}

SpeciesSet Species::evolvedSpecies(bool useOriginal) {
    SpeciesSet r;
    if (useOriginal) {
        if (originalEvolvedForms_) r = SpeciesSet::of(*originalEvolvedForms_);
    } else {
        for (const auto& evo : evolutionsFrom_) r.add(evo->to());
    }
    if (!isBaseForme()) r.addAll(baseForme_->evolvedSpecies(useOriginal));
    return r;
}

SpeciesSet Species::preEvolvedSpecies(bool useOriginal) {
    SpeciesSet r;
    if (useOriginal) {
        if (originalPreEvolvedForms_) r = SpeciesSet::of(*originalPreEvolvedForms_);
    } else {
        for (const auto& evo : evolutionsTo_) r.add(evo->from());
    }
    if (!isBaseForme()) r.addAll(baseForme_->preEvolvedSpecies(useOriginal));
    return r;
}

SpeciesSet Species::family(bool useOriginal) {
    SpeciesSet fam;
    std::deque<Species*> toAdd{this};
    while (!toAdd.empty()) {
        Species* adding = toAdd.front();
        toAdd.pop_front();
        if (fam.contains(adding)) continue;
        fam.add(adding);
        for (Species* s : adding->evolvedSpecies(useOriginal).toVector()) toAdd.push_back(s);
        for (Species* s : adding->preEvolvedSpecies(useOriginal).toVector()) toAdd.push_back(s);
    }
    return fam;
}

int Species::relation(Species* relative, bool useOriginal) {
    std::deque<std::pair<Species*, int>> toCheck{{this, 0}};
    SpeciesSet checked;
    while (!toCheck.empty()) {
        auto [cur, pos] = toCheck.front();
        toCheck.pop_front();
        if (checked.contains(cur)) continue;
        checked.add(cur);
        if (cur == relative) return pos;
        for (Species* e : cur->evolvedSpecies(useOriginal).toVector()) toCheck.emplace_back(e, pos + 1);
        for (Species* e : cur->preEvolvedSpecies(useOriginal).toVector()) toCheck.emplace_back(e, pos - 1);
    }
    throw std::invalid_argument("No se puede calcular la relación con un Pokémon no emparentado");
}

SpeciesSet Species::relativesAtPosition(int position, bool useOriginal) {
    std::deque<std::pair<Species*, int>> toCheck{{this, 0}};
    SpeciesSet checked, relatives;
    while (!toCheck.empty()) {
        auto [cur, pos] = toCheck.front();
        toCheck.pop_front();
        if (checked.contains(cur)) continue;
        checked.add(cur);
        if (pos == position) relatives.add(cur);
        for (Species* e : cur->evolvedSpecies(useOriginal).toVector()) toCheck.emplace_back(e, pos + 1);
        for (Species* e : cur->preEvolvedSpecies(useOriginal).toVector()) toCheck.emplace_back(e, pos - 1);
    }
    return relatives;
}

SpeciesSet Species::relativesAtPositionSameBranch(int position, bool useOriginal) {
    SpeciesSet current(this);
    if (position == 0) return current;
    int step = position > 0 ? 1 : -1;
    for (int i = 0; i != position; i += step) {
        SpeciesSet next;
        for (Species* s : current.toVector())
            next.addAll(position > 0 ? s->evolvedSpecies(useOriginal) : s->preEvolvedSpecies(useOriginal));
        current = std::move(next);
    }
    return current;
}

int Species::stagesAfter(bool useOriginal) {
    int stages = 0;
    SpeciesSet current(this), checked;
    while (!current.empty()) {
        SpeciesSet next;
        for (Species* s : current.toVector()) {
            if (checked.contains(s)) continue;
            next.addAll(s->evolvedSpecies(useOriginal));
            checked.add(s);
        }
        if (!next.empty()) stages++;
        current = std::move(next);
    }
    return stages;
}

int Species::stagesBefore(bool useOriginal) {
    int stages = 0;
    SpeciesSet current(this), checked;
    while (!current.empty()) {
        SpeciesSet prev;
        for (Species* s : current.toVector()) {
            if (checked.contains(s)) continue;
            prev.addAll(s->preEvolvedSpecies(useOriginal));
            checked.add(s);
        }
        if (!prev.empty()) stages++;
        current = std::move(prev);
    }
    return stages;
}

void Species::saveOriginalData() {
    // SpeciesSet.unmodifiable(...) hace una copia; guardamos el orden de esa copia
    originalEvolvedForms_ = SpeciesSet::unmodifiable(evolvedSpecies(false).toVector()).toVector();
    originalPreEvolvedForms_ = SpeciesSet::unmodifiable(preEvolvedSpecies(false).toVector()).toVector();
}

}  // namespace pr
