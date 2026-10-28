#pragma once
#include <QString>
#include <vector>
#include <map>
#include <utility>

// One retained polycyclic ring system (Blue Book Appendix 1 hydrocarbon or
// Appendix 2 heterocyclic parent component), used as a single fusion
// component (base or attached) instead of decomposing it into monocyclic
// pieces. See docs/superpowers/specs/2026-10-15-retained-polycyclic-components-design.md.
struct RetainedRingSystem {
    QString name;                  // e.g. "isoquinoline"

    // Canonical SMILES for this system ALONE (not embedded in any larger
    // molecule). Written so that parsing it with Indigo and iterating
    // indigoIterateAtoms in order gives atoms in EXACTLY the order
    // peripheralLocants (below) assumes. Getting this order wrong silently
    // mislabels every molecule this entry ever matches -- see
    // validateRetainedRingSystem.
    QString referenceSmiles;

    // peripheralLocants[i] is this system's own fixed Blue Book locant
    // (e.g. "1", "4a", "8a") for the atom at position i in referenceSmiles's
    // own indigoIterateAtoms order. Size MUST equal the atom count of
    // referenceSmiles.
    std::vector<QString> peripheralLocants;

    // Bond-letter -> (locant, locant) map for citing this system as an
    // ATTACHED component (e.g. isoquinoline's own 'g' bond, used in
    // "naphtho[2,3-g]isoquinoline"). Letters follow Blue Book FR-4
    // peripheral-bond-letter assignment: 'a' is the 1,2-bond, 'b' the
    // 2,3-bond, etc., skipping fusion (interior) bonds.
    std::map<char, std::pair<QString, QString>> peripheralBondLetters;

    // Position in Appendix 1 (hydrocarbons) or Appendix 2 (heterocycles)'s
    // own seniority list, LOWER = MORE senior. Appendix 1 and Appendix 2
    // are separate seniority lists in the real Blue Book (a heterocyclic
    // component is always preferred over a hydrocarbon one when both
    // contain the qualifying heteroatom per FR-2.3(a) -- seniorityRank only
    // breaks ties WITHIN one appendix; cross-appendix comparison is handled
    // by the matcher checking heteroatom content first, per FR-2.3(a),
    // before ever comparing seniorityRank).
    int seniorityRank = 0;

    // Task 10: this system's own FUSION PREFIX word, used when citing this
    // system as an ATTACHED component (e.g. "naphtho" for naphthalene, per
    // Blue Book FR-4 / the same irregular-stem convention IupacNamer.cpp's
    // getFusionPrefixShared(RingType) already follows for monocyclic rings
    // -- see its doc comment for examples like "benzo", "pyrido"). Appended
    // as a NEW TRAILING field so every pre-existing positional aggregate
    // initializer across src/app/retained/*.cpp (which omit it) still
    // compiles unchanged and simply defaults to "" (this struct has no
    // user-declared constructor, so it stays an aggregate). Deliberately
    // left empty ("") for every entry not yet verified as a correct fusion
    // prefix -- RetainedRingMatcher.cpp's attached-component assembly
    // treats an empty fusionPrefix as "can't confidently cite this system
    // as an attached component" and falls back rather than guessing a word
    // form. Only "naphthalene" (HydrocarbonFamily.cpp) has this populated
    // so far, since it's the only entry Task 10's ground-truth molecule
    // (naphtho[2,3-g]isoquinoline) actually needs.
    QString fusionPrefix;

    // Locant of the atom whose hydrogen is "indicated hydrogen" (Blue Book
    // P-25.7.1) rather than ordinary ring hydrogen -- e.g. "9" for
    // carbazole's NH. Empty ("") for every retained system that doesn't
    // need one. When non-empty, citing this system's bare name must
    // prepend "<locant>H-" using the same format IupacNamer.cpp's
    // existing ihPrefix convention uses (IupacNamer.cpp, the
    // "1H-" / "1H,3H-" format near the indicated-H locant assembly code).
    QString indicatedHydrogenLocant;

    // Locants of interior atoms that do not lie on the simple outer peripheral cycle.
    // For peri-fused systems (e.g. perimidine's central triple junction "9b").
    // Empty for all simple ortho-fused systems.
    std::vector<QString> interiorLocants;
};

// Parses sys.referenceSmiles standalone, confirms:
//   1. It parses at all.
//   2. Its atom count equals sys.peripheralLocants.size().
//   3. Every locant string in peripheralLocants is distinct.
//   4. Every bond-letter's pair of locants in peripheralBondLetters actually
//      appears, in order, as two consecutive entries of peripheralLocants
//      (cyclically) -- catching a transposed locant, not just a wrong-length
//      list.
// Returns a human-readable error string on failure, empty string on success.
// `mol` is unused by the current implementation but kept so a future
// Indigo-session-aware check can be added without an interface change.
QString validateRetainedRingSystem(int mol, const RetainedRingSystem &sys);
