#include "TricyclicNFamily.h"

// Appendix 2 heterocyclic retained names -- tricyclic N-heterocycles.
//
// NOTE: The existing IupacNamer.cpp has special-case logic for WHOLE MOLECULES
// that are exactly acridine or carbazole (lines ~6522-6527). That logic handles
// the case where the entire molecule IS acridine/carbazole. This registry,
// by contrast, handles acridine/carbazole appearing as a COMPONENT of a larger
// fused system (e.g., benzo[...]acridine). These are separate use cases and
// do not conflict.
//
// Reference anthracene (from HydrocarbonFamily.cpp):
//   SMILES: c1ccc2cc3ccccc3cc2c1
//   atom[0] -> "2",   atom[1] -> "3",   atom[2] -> "4",
//   atom[3] -> "4a",  atom[4] -> "10",  atom[5] -> "10a",
//   atom[6] -> "5",   atom[7] -> "6",   atom[8] -> "7",
//   atom[9] -> "8",   atom[10] -> "8a", atom[11] -> "9",
//   atom[12] -> "9a", atom[13] -> "1"
//   Bridgeheads (deg=3): atom[3] ("4a"), atom[5] ("10a"),
//                        atom[10] ("8a"), atom[12] ("9a")
//   The two meso positions in the central ring: atom[4] (locant "10")
//   and atom[11] (locant "9").
//
// Strategy per Task 4 spec:
// - phenazine, acridine: Reuse anthracene skeleton (low-risk).
// - phenanthridine: Angular (phenanthrene-shaped), needs phenanthrene skeleton.
// - carbazole: 6-5-6 system (not 6-6-6). DONE (Task 1 of the 2026-10-17
//   retained-hydro-assembly plan) -- fresh SMILES verified via Indigo
//   neighbor dump, see entry 3 below.
// - perimidine, pteridine, purine: Need verification.
// - phenanthroline isomers (10): All need phenanthrene skeleton with 2N.

void registerTricyclicNFamily(std::vector<RetainedRingSystem> &registry) {
    // =========================================================================
    // ANTHRACENE-BASED (linear 6-6-6, 14 atoms):
    // Reuse the verified anthracene skeleton. Low-risk approach proven in
    // Task 2 (quinoline family reused naphthalene skeleton).
    // =========================================================================

    // --- 1. Phenazine (rank 8) ------------------------------------------------
    // Dibenzo-pyrazine: anthracene with BOTH meso positions replaced by N.
    // SMILES: c1ccc2nc3ccccc3nc2c1
    //   - Same atom count as anthracene: 14
    //   - Same atom-parse order as anthracene (verified via Indigo)
    //   - atom[4]=N (locant "10"), atom[11]=N (locant "9")
    //   - All other atoms remain C in the same positions
    //
    // Full Indigo verification output (from scratch program):
    //   atom[0]: C deg=2 neighbors=[1,13]
    //   atom[1]: C deg=2 neighbors=[0,2]
    //   atom[2]: C deg=2 neighbors=[1,3]
    //   atom[3]: C deg=3 neighbors=[2,4,12] BRIDGEHEAD
    //   atom[4]: N deg=2 neighbors=[3,5] NITROGEN
    //   atom[5]: C deg=3 neighbors=[4,6,10] BRIDGEHEAD
    //   atom[6]: C deg=2 neighbors=[5,7]
    //   atom[7]: C deg=2 neighbors=[6,8]
    //   atom[8]: C deg=2 neighbors=[7,9]
    //   atom[9]: C deg=2 neighbors=[8,10]
    //   atom[10]: C deg=3 neighbors=[9,5,11] BRIDGEHEAD
    //   atom[11]: N deg=2 neighbors=[10,12] NITROGEN
    //   atom[12]: C deg=3 neighbors=[11,3,13] BRIDGEHEAD
    //   atom[13]: C deg=2 neighbors=[12,0]
    //
    // Reuse anthracene's verified locant mapping and bond-letter assignment.
    // Bond letters follow Blue Book FR-4 convention: 'a' is always the 1,2-bond,
    // 'b' the 2,3-bond, etc., based on the peripheral locant sequence.
    // Peripheral sequence: 1-2-3-4-4a-10-10a-5-6-7-8-8a-9-9a-1 (cyclic)
    registry.push_back(RetainedRingSystem{
        "phenazine",
        "c1ccc2nc3ccccc3nc2c1",
        {"2","3","4","4a","10","10a","5","6","7","8","8a","9","9a","1"},
        {
            {'a', {"1","2"}},   // 1-2 bond
            {'b', {"2","3"}},   // 2-3 bond
            {'c', {"3","4"}},   // 3-4 bond
            {'d', {"4","4a"}},  // 4-4a bond
            {'e', {"4a","10"}}, // 4a-10 bond
            {'f', {"10","10a"}},// 10-10a bond
            {'g', {"10a","5"}},  // 10a-5 bond
            {'h', {"5","6"}},    // 5-6 bond
            {'i', {"6","7"}},    // 6-7 bond
            {'j', {"7","8"}},    // 7-8 bond
            {'k', {"8","8a"}},   // 8-8a bond
            {'l', {"8a","9"}},   // 8a-9 bond
            {'m', {"9","9a"}},   // 9-9a bond
            {'n', {"9a","1"}},   // 9a-1 bond (closing the cycle)
        },
        8, // Appendix 2 seniority rank
    });

    // --- 2. Acridine (rank 20) -------------------------------------------------
    // Anthracene with ONE meso position (locant 10 = atom[4]) replaced by N.
    // SMILES: c1ccc2nc3ccccc3cc2c1
    //   - Same atom count as anthracene: 14
    //   - Same atom-parse order as anthracene (verified via Indigo)
    //   - atom[4]=N (locant "10")
    //   - All other atoms remain C in the same positions
    //
    // Full Indigo verification output (from scratch program):
    //   atom[0]: C deg=2 neighbors=[1,13]
    //   atom[1]: C deg=2 neighbors=[0,2]
    //   atom[2]: C deg=2 neighbors=[1,3]
    //   atom[3]: C deg=3 neighbors=[2,4,12] BRIDGEHEAD
    //   atom[4]: N deg=2 neighbors=[3,5] NITROGEN
    //   atom[5]: C deg=3 neighbors=[4,6,10] BRIDGEHEAD
    //   atom[6]: C deg=2 neighbors=[5,7]
    //   atom[7]: C deg=2 neighbors=[6,8]
    //   atom[8]: C deg=2 neighbors=[7,9]
    //   atom[9]: C deg=2 neighbors=[8,10]
    //   atom[10]: C deg=3 neighbors=[9,5,11] BRIDGEHEAD
    //   atom[11]: C deg=2 neighbors=[10,12]
    //   atom[12]: C deg=3 neighbors=[11,3,13] BRIDGEHEAD
    //   atom[13]: C deg=2 neighbors=[12,0]
    //
    // Reuse anthracene's verified locant mapping and bond-letter assignment.
    // Note: atom[4] is N at locant "10"; atom[11] remains C at locant "9".
    registry.push_back(RetainedRingSystem{
        "acridine",
        "c1ccc2nc3ccccc3cc2c1",
        {"2","3","4","4a","10","10a","5","6","7","8","8a","9","9a","1"},
        {
            {'a', {"1","2"}},   // 1-2 bond
            {'b', {"2","3"}},   // 2-3 bond
            {'c', {"3","4"}},   // 3-4 bond
            {'d', {"4","4a"}},  // 4-4a bond
            {'e', {"4a","10"}}, // 4a-10 bond
            {'f', {"10","10a"}},// 10-10a bond
            {'g', {"10a","5"}},  // 10a-5 bond
            {'h', {"5","6"}},    // 5-6 bond
            {'i', {"6","7"}},    // 6-7 bond
            {'j', {"7","8"}},    // 7-8 bond
            {'k', {"8","8a"}},   // 8-8a bond
            {'l', {"8a","9"}},   // 8a-9 bond
            {'m', {"9","9a"}},   // 9-9a bond
            {'n', {"9a","1"}},   // 9a-1 bond (closing the cycle)
        },
        20, // Appendix 2 seniority rank
    });

    // --- 3. Carbazole (rank 22) -------------------------------------------
    // 6-5-6 system: two benzo rings fused to a central pyrrole ring (NH).
    // SMILES: c1cccc2c3ccccc3[nH]c12 (13 atoms, C12H9N) -- UNBRANCHED (pure
    // chain + ring-closure digits only, same style as the existing
    // acridine/phenazine entries above), chosen specifically so Indigo's
    // atom-parse order equals the periphery-walk order. (A prior branched
    // SMILES, c1ccc2c(c1)[nH]c1ccccc21, was chemically self-consistent but
    // failed validateRetainedRingSystem's cyclic-array-adjacency check,
    // since a "(...)" branch breaks the parse-order == periphery-order
    // property every other entry in this file relies on -- fixed here by
    // re-deriving an unbranched SMILES instead.)
    //
    // Full Indigo verification output (from scratch program):
    //   atom[0]: C neighbors=[1,12]    -> locant "1"
    //   atom[1]: C neighbors=[0,2]     -> locant "2"
    //   atom[2]: C neighbors=[1,3]     -> locant "3"
    //   atom[3]: C neighbors=[2,4]     -> locant "4"
    //   atom[4]: C neighbors=[3,5,12]  -> locant "4a"  BRIDGEHEAD
    //   atom[5]: C neighbors=[4,6,10]  -> locant "4b"  BRIDGEHEAD
    //   atom[6]: C neighbors=[5,7]     -> locant "5"
    //   atom[7]: C neighbors=[6,8]     -> locant "6"
    //   atom[8]: C neighbors=[7,9]     -> locant "7"
    //   atom[9]: C neighbors=[8,10]    -> locant "8"
    //   atom[10]: C neighbors=[9,5,11] -> locant "8a"  BRIDGEHEAD
    //   atom[11]: N neighbors=[10,12]  -> locant "9" (implicitH=1, N-H)
    //   atom[12]: C neighbors=[11,0,4] -> locant "9a"  BRIDGEHEAD
    //
    // Ring SSSR confirmed: {0,1,2,3,4,12}=benzo1, {4,5,10,11,12}=central
    // pyrrole, {5,6,7,8,9,10}=benzo2 -- matches real carbazole exactly.
    // Mapped to Blue Book carbazole numbering (periphery walk
    // 1-2-3-4-4a-4b-5-6-7-8-8a-9-9a-1; central ring = 4a-4b-8a-9-9a).
    // Cross-checked independently against this project's own pre-existing
    // test comment at tests/iupac_namer_test.cpp:~1120, which separately
    // derived carbazole's indicated-H locant as "9H" for an unrelated
    // regression guard -- both derivations agree.
    registry.push_back(RetainedRingSystem{
        "carbazole",
        "c1cccc2c3ccccc3[nH]c12",
        {"1","2","3","4","4a","4b","5","6","7","8","8a","9","9a"},
        {
            {'a', {"1","2"}},   // 1-2 bond
            {'b', {"2","3"}},   // 2-3 bond
            {'c', {"3","4"}},   // 3-4 bond
            {'d', {"4","4a"}},  // 4-4a bond
            {'e', {"4a","4b"}}, // 4a-4b bond
            {'f', {"4b","5"}},  // 4b-5 bond
            {'g', {"5","6"}},   // 5-6 bond
            {'h', {"6","7"}},   // 6-7 bond
            {'i', {"7","8"}},   // 7-8 bond
            {'j', {"8","8a"}},  // 8-8a bond
            {'k', {"8a","9"}},  // 8a-9 bond
            {'l', {"9","9a"}},  // 9-9a bond
            {'m', {"9a","1"}},  // 9a-1 bond (closing the cycle)
        },
        22, // Appendix 2 seniority rank
        "", // fusionPrefix: unverified, leave empty
        "9", // indicatedHydrogenLocant: carbazole's N-H
    });

    // =========================================================================
    // DEferred ENTRIES: See notes below for each.
    // =========================================================================
    //
    // PHENANTHRENE-BASED (angular 6-6-6, 14 atoms):
    // These require the phenanthrene skeleton (c1cc2cc3cccccc3c2cc1), NOT
    // anthracene. The atom-parse order differs from anthracene, so fresh
    // verification required per Task 4 spec: "For every one of these, you MUST
    // do the full walk-the-real-bond-list verification from Bug class 2 above."
    //
    // Deferred entries:
    // - phenanthridine (rank 21): Angular with N at position 5.
    //   Requires: (1) correct phenanthrene-based SMILES, (2) Indigo verification
    //   of atom indices and neighbors, (3) mapping to correct Blue Book locants.
    //
    // OTHER TRICYCLICS:
    // - perimidine (rank 19): Structure requires verification.
    // - pteridine (rank 23): Structure requires verification.
    //
    // PHENANTHROLINE ISOMERS (ranks 9-18, 10 total):
    // All are angular 6-6-6 with 2N in different positions.
    // Each requires: (1) correct phenanthrene-based SMILES with 2N,
    // (2) full Indigo verification, (3) correct locant mapping.
    // The 10 isomers are: 1,2- through 1,10- and 4,5-phenanthroline
    // (exact list from Blue Book Appendix 2 entries 9-18).
    //
    // DEferred: All of the above due to time/verification complexity.
    // Partial completion explicitly acceptable per Task 4 spec:
    // "Do as many as you can verify confidently... a wrong phenanthroline
    // entry silently poisons every future molecule matching it... an honest
    // partial result is better than a forced complete one here."
}
