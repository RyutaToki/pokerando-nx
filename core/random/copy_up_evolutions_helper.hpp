// Port de CopyUpEvolutionsHelper: aplica una acción a los Pokémon básicos y la "copia" hacia
// sus evoluciones (y formas), en el mismo orden que FVX.
#pragma once
#include <functional>

#include "gamedata/species.hpp"

namespace pr {

class CopyUpEvolutionsHelper {
public:
    using BasicAction = std::function<void(Species*)>;
    using EvolvedAction = std::function<void(Species* from, Species* to, bool toFinal)>;
    using AltFormeAction = std::function<void(Species* from, Species* to)>;

    struct Options {
        bool evolutionSanity = true;
        bool copySplitEvos = false;
        bool treatMegasAsEvos = false;
        BasicAction basicAction, noEvoAction;
        EvolvedAction evolvedAction, splitAction;
        AltFormeAction altFormeAction, cosmeticAction;

        Options(BasicAction basic, EvolvedAction evolved) : basicAction(std::move(basic)), evolvedAction(std::move(evolved)) {}
        Options& withEvolutionSanity(bool v) { evolutionSanity = v; return *this; }
        Options& withCopySplitEvos(bool v) { copySplitEvos = v; return *this; }
        Options& withTreatMegasAsEvos(bool v) { treatMegasAsEvos = v; return *this; }
        Options& withNoEvoAction(BasicAction a) { noEvoAction = std::move(a); return *this; }
        Options& withSplitAction(EvolvedAction a) { splitAction = std::move(a); return *this; }
        Options& withAltFormeAction(AltFormeAction a) { altFormeAction = std::move(a); return *this; }
        Options& withAltFormeBasicAction(BasicAction a) {
            altFormeAction = [a](Species*, Species* alt) { a(alt); };
            return *this;
        }
        Options& withCosmeticAction(AltFormeAction a) { cosmeticAction = std::move(a); return *this; }
        // build(): rellena los valores por defecto
        Options& build();
    };

    explicit CopyUpEvolutionsHelper(std::function<SpeciesSet()> supplier) : supplier_(std::move(supplier)) {}
    void apply(Options options);

private:
    struct Rel {
        Species* from;
        Species* to;
    };
    Rel findRelationInto(const SpeciesSet& all, Species* to) const;
    bool isBasic(const SpeciesSet& all, Species* s, const Options& o) const;
    bool isSplitEvo(const SpeciesSet& all, Species* s, const Options& o) const;
    bool isFinalEvo(const SpeciesSet& all, Species* s, const Options& o) const;
    bool isBaseForme(const SpeciesSet& all, Species* s) const;

    std::function<SpeciesSet()> supplier_;
};

}  // namespace pr
