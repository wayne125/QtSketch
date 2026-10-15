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
// - carbazole: 6-5-6 system (not 6-6-6), needs fresh SMILES.
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
    // CARBAZOLE (6-5-6, 13 atoms):
    // - carbazole (rank 22): Two benzene rings + pyrrole (NH).
    //   Requires: (1) correct 13-atom SMILES, (2) full Indigo verification,
    //   (3) 6-5-6 locant mapping (different from 6-6-6 anthracene/phenanthrene).
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
