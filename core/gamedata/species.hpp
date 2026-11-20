// Port de Species (com.uprfvx.romio.gamedata.Species).
// Los objetos se identifican por número (equals/hashCode de Java usan solo el número).
#pragma once
#include <optional>
#include <string>
#include <vector>

#include "gamedata/evolution.hpp"
#include "gamedata/species_set.hpp"
#include "gamedata/types.hpp"
#include "graphics/palette.hpp"
#include "util/java_hash.hpp"

namespace pr {

class Species;
class SpeciesSet;
class JavaRandom;


class Species {
public:
    explicit Species(int number);

    int number() const { return number_; }
    int baseNumber() const { return baseForme()->number(); }
    const std::string& name() const { return name_; }
    void setName(std::string n) { name_ = std::move(n); }
    const std::string& formeSuffix() const { return formeSuffix_; }
    void setFormeSuffix(std::string s) { formeSuffix_ = std::move(s); }
    std::string fullName() const { return name_ + formeSuffix_; }
    std::string numberAndFullName() const { return "#" + std::to_string(number_) + " - " + fullName(); }
    int generation() const { return generation_; }
    void setGeneration(int g) { generation_ = g; }

    // --- formas ---
    Species* baseForme() { return isBaseForme() ? this : baseForme_; }
    const Species* baseForme() const { return isBaseForme() ? this : baseForme_; }
    bool isBaseForme() const { return baseForme_ == nullptr; }
    Species* conceptualBaseForme() { return conceptualBaseForme_ ? conceptualBaseForme_ : baseForme(); }
    void setConceptualBaseForme(Species* c);
    void addAltForme(int formeNumber, Species* altForme);
    void addCosmeticAltForme(int formeNumber);
    Species* getForme(int formeNumber);
    bool isValidFormeNumber(int formeNumber) const { return formes_.containsKey(formeNumber); }
    SpeciesSet altFormes();
    int formeNumber() const { return formeNumber_; }
    const std::vector<int>& cosmeticFormeNumbers() const { return cosmeticFormeNumbers_; }
    int randomCosmeticFormeNumber(JavaRandom& random) const;
    bool isMegaEvolution() const { return megaEvolution_; }
    void setMegaEvolution(std::optional<int> item);
    bool needsMegaEvolutionItem() const { return megaEvolutionItem_.has_value(); }
    std::optional<int> megaEvolutionItem() const { return megaEvolutionItem_; }
    bool isAlolan() const { return alolan_; }
    void setAlolan();
    bool isEssentiallyCosmetic() const { return essentiallyCosmetic_; }
    void setEssentiallyCosmetic();

    bool isLegendary() const;
    bool isStrongLegendary() const;
    bool isUltraBeast() const;

    // --- tipos ---
    std::optional<Type> primaryType(bool useOriginal = false) const { return useOriginal ? origPrimary_ : primary_; }
    std::optional<Type> secondaryType(bool useOriginal = false) const { return useOriginal ? origSecondary_ : secondary_; }
    void setPrimaryType(std::optional<Type> t);
    void setSecondaryType(std::optional<Type> t);
    bool hasType(std::optional<Type> t, bool useOriginal) const;
    bool hasSecondaryType(bool useOriginal) const { return secondaryType(useOriginal).has_value(); }
    bool hasSharedType(const Species& other) const;

    // --- stats ---
    BaseStats& baseStats() { return baseStats_; }
    const BaseStats& baseStats() const { return baseStats_; }
    void setBaseStats(const BaseStats& bs);
    int bst(bool useOriginal) const { return useOriginal ? originalBST_ : baseStats_.bst(); }

    int ability1 = 0, ability2 = 0, ability3 = 0;
    int catchRate = 0;
    int expYield = 0;
    EVYield evYield;
    // ids estándar de objeto; nullopt = ninguno
    std::optional<int> guaranteedHeldItem, commonHeldItem, rareHeldItem, darkGrassHeldItem;
    int genderRatio = 0;
    int callRate = 0;
    int frontImageDimensions = 0;
    std::optional<ExpCurve> growthCurve;
    BreedingInfo breedingInfo;
    std::optional<Palette> normalPalette, shinyPalette;

    // --- evoluciones (los mismos objetos en "from" del origen y "to" del destino) ---
    std::vector<EvolutionPtr>& evolutionsFrom() { return evolutionsFrom_; }
    std::vector<EvolutionPtr>& evolutionsTo() { return evolutionsTo_; }
    const std::vector<EvolutionPtr>& evolutionsFrom() const { return evolutionsFrom_; }
    const std::vector<EvolutionPtr>& evolutionsTo() const { return evolutionsTo_; }
    bool isLegalEvolutionAtLevel(int level, double evoLvlModifier);
    bool hasLegalEvolutionAtLevel(int level, double evoLvlModifier);

    SpeciesSet evolvedSpecies(bool useOriginal);
    SpeciesSet preEvolvedSpecies(bool useOriginal);
    SpeciesSet family(bool useOriginal);
    int relation(Species* relative, bool useOriginal);
    SpeciesSet relativesAtPosition(int position, bool useOriginal);
    SpeciesSet relativesAtPositionSameBranch(int position, bool useOriginal);
    int stagesAfter(bool useOriginal);
    int stagesBefore(bool useOriginal);
    void saveOriginalData();
    void copyBaseFormeAbilities(const Species& other) {
        ability1 = other.ability1;
        ability2 = other.ability2;
        ability3 = other.ability3;
    }
    int compareTo(const Species& o) const { return number_ - o.number_; }

private:
    const std::vector<EvolutionPtr>& evolutionsOfForme(bool to);

    std::string name_;
    int number_;
    JavaHashMap<int, Species*> formes_;
    std::vector<int> cosmeticFormeNumbers_{0};
    std::vector<int> ignoreCosmeticFormeNumbers_;
    std::string formeSuffix_;
    int formeNumber_ = 0;
    Species* baseForme_ = nullptr;
    Species* conceptualBaseForme_ = nullptr;
    bool megaEvolution_ = false;
    std::optional<int> megaEvolutionItem_;
    bool alolan_ = false;
    bool essentiallyCosmetic_ = false;
    int generation_ = -1;
    // conjuntos originales (se guardan como vectores en orden de iteración de Java)
    std::optional<std::vector<Species*>> originalEvolvedForms_, originalPreEvolvedForms_;
    std::optional<Type> primary_, secondary_, origPrimary_, origSecondary_;
    bool hasSetPrimary_ = false, hasSetSecondary_ = false;
    BaseStats baseStats_;
    int originalBST_ = -1;
    std::vector<EvolutionPtr> evolutionsFrom_, evolutionsTo_;
};

}  // namespace pr

