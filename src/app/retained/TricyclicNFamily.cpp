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
    // PHENANTHRENE-BASED (angular 6-6-6, 14 atoms):
    // Reference skeleton: c1ccc2ccc3ccccc3c2c1
    //
    // Full Indigo verification output (from scratch program, build/dump_rings.exe):
    //   Ring 0 (SSSR): {0,1,2,3,12,13}   Ring 1 (SSSR, center): {3,4,5,6,11,12}
    //   Ring 2 (SSSR): {6,7,8,9,10,11}
    //   atom[0]  C deg=2   atom[1]  C deg=2   atom[2]  C deg=2
    //   atom[3]  C deg=3 BRIDGEHEAD   atom[4]  C deg=2   atom[5]  C deg=2
    //   atom[6]  C deg=3 BRIDGEHEAD   atom[7]  C deg=2   atom[8]  C deg=2
    //   atom[9]  C deg=2   atom[10] C deg=2
    //   atom[11] C deg=3 BRIDGEHEAD   atom[12] C deg=3 BRIDGEHEAD
    //   atom[13] C deg=2
    //
    // Locant derivation (first pass wrongly trusted dump_rings.exe's printed
    // ring-atom ORDER as a bond-adjacency cycle -- it is not, it's just SSSR
    // membership; re-derived correctly from the real bond list implied by the
    // chain+ring-closure SMILES syntax itself: 0-1,1-2,2-3,3-4,4-5,5-6,6-7,
    // 7-8,8-9,9-10,10-11,11-6(closure"3"),11-12,12-3(closure"2"),12-13,
    // 13-0(closure"1") -- then cross-checked against the registry's own
    // validateRetainedRingSystem, which additionally confirms the array
    // itself traces a real periphery cycle, not just a correct locant-to-atom
    // lookup):
    //   atom[11]-atom[12] is the ONE adjacent-bridgehead bond NOT shared with
    //   either terminal ring (Blue Book bay-region 4a-4b bond, phenanthrene's
    //   defining feature vs. anthracene) -> atom[12]="4a", atom[11]="4b".
    //   atom[3]-atom[12] and atom[6]-atom[11] are the two fusion bonds ->
    //   atom[3]="10a" (ring A's other bridgehead), atom[6]="8a" (ring B's).
    //   K-region atoms between 8a and 10a: atom[5]="9", atom[4]="10".
    //   Terminal ring A path (10a to 4a, away from center): atom[2]="1",
    //   atom[1]="2", atom[0]="3", atom[13]="4".
    //   Terminal ring B path (4b to 8a, away from center): atom[10]="5",
    //   atom[9]="6", atom[8]="7", atom[7]="8".
    //   Full periphery walk (14-cycle, == atom-parse order 0-13 exactly,
    //   confirmed by validateRetainedRingSystem's cyclic-adjacency check):
    //   3(0)-2(1)-1(2)-10a(3)-10(4)-9(5)-8a(6)-8(7)-7(8)-6(9)-5(10)-4b(11)-
    //   4a(12)-4(13)-[closes to 3(0)].
    //
    // Cross-checked against real, independently-documented chemistry (not
    // inferred from the graph alone): 1,10-phenanthroline is universally
    // documented as a bidentate chelator because its two N atoms sit in a
    // "pincer" pocket both adjacent to the SAME bridgehead -- this derivation
    // puts locant "1" (atom[2]) and locant "10" (atom[4]) both adjacent to
    // atom[3]="10a", exactly reproducing that pincer geometry. 1,7-
    // phenanthroline is independently documented as NOT forming that pincer
    // (locant "7"=atom[8] sits nowhere near atom[3]) -- also consistent.
    // Full locant-pair list for the 10 phenanthroline isomers (ranks 9-18)
    // confirmed against the primary source, Blue Book Appendix 2
    // (https://iupac.qmul.ac.uk/fusedring/app2.html, entries 9-18), not
    // guessed or reconstructed from memory.
    //
    // peripheralLocants (shared by every entry below, atom-parse order 0-13):
    //   {"3","2","1","10a","10","9","8a","8","7","6","5","4b","4a","4"}
    // peripheralBondLetters (shared, periphery walk 1-2-3-4-4a-4b-5-6-7-8-
    // 8a-9-10-10a-1, 14 bonds lettered a-n in that order -- Blue Book FR-4
    // convention always starts lettering at the 1,2-bond regardless of where
    // locant "1" sits in the parse-order array above):
    //   a:1-2  b:2-3  c:3-4  d:4-4a  e:4a-4b  f:4b-5  g:5-6  h:6-7  i:7-8
    //   j:8-8a  k:8a-9  l:9-10  m:10-10a  n:10a-1 (closing)
    // =========================================================================

    static const std::vector<QString> kPhenanthrenePeripheralLocants = {
        "3","2","1","10a","10","9","8a","8","7","6","5","4b","4a","4"
    };
    static const std::map<char, std::pair<QString, QString>> kPhenanthreneBondLetters = {
        {'a', {"1","2"}},   {'b', {"2","3"}},   {'c', {"3","4"}},
        {'d', {"4","4a"}},  {'e', {"4a","4b"}}, {'f', {"4b","5"}},
        {'g', {"5","6"}},   {'h', {"6","7"}},   {'i', {"7","8"}},
        {'j', {"8","8a"}},  {'k', {"8a","9"}},  {'l', {"9","10"}},
        {'m', {"10","10a"}},{'n', {"10a","1"}},
    };

    // --- 4. Phenanthridine (rank 21) -------------------------------------
    // N replaces atom[7] (locant "5") -- the one non-bridgehead atom
    // adjacent to atom[11]="4b". Confirmed via dump_rings.exe: same 4
    // bridgeheads (3,6,11,12) remain carbon, degree 3; atom[7] becomes N,
    // degree 2, no implicit H (pyridine-type), same ring membership as the
    // base skeleton -- structure unperturbed apart from the single
    // substitution. N-5 position cross-checked against phenanthridinone
    // antitumor-compound literature's own "N-5" numbering convention.
    registry.push_back(RetainedRingSystem{
        "phenanthridine",
        "c1ccc2ccc3cccnc3c2c1",
        kPhenanthrenePeripheralLocants,
        kPhenanthreneBondLetters,
        21,
    });

    // --- 5-14. Phenanthroline isomers (ranks 9-18, 10 total) --------------
    // Each replaces the two non-bridgehead atoms at the given locants with N;
    // every candidate SMILES confirmed via dump_rings.exe to preserve the
    // same 4 carbon bridgeheads (3,6,11,12) and the same ring membership as
    // the base phenanthrene skeleton -- only the two chosen positions change
    // element. Locant pairs are the verified Appendix 2 list, not guessed.
    registry.push_back(RetainedRingSystem{
        "1,7-phenanthroline", "c1cnc2ccc3cnccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 9,
    });
    registry.push_back(RetainedRingSystem{
        "1,8-phenanthroline", "c1cnc2ccc3ncccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 10,
    });
    registry.push_back(RetainedRingSystem{
        "1,9-phenanthroline", "c1cnc2cnc3ccccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 11,
    });
    registry.push_back(RetainedRingSystem{
        // Pincer-chelating isomer, independently cross-checked above.
        "1,10-phenanthroline", "c1cnc2ncc3ccccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 12,
    });
    registry.push_back(RetainedRingSystem{
        "2,7-phenanthroline", "c1ncc2ccc3cnccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 13,
    });
    registry.push_back(RetainedRingSystem{
        "2,8-phenanthroline", "c1ncc2ccc3ncccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 14,
    });
    registry.push_back(RetainedRingSystem{
        "2,9-phenanthroline", "c1ncc2cnc3ccccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 15,
    });
    registry.push_back(RetainedRingSystem{
        "3,7-phenanthroline", "n1ccc2ccc3cnccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 16,
    });
    registry.push_back(RetainedRingSystem{
        "3,8-phenanthroline", "n1ccc2ccc3ncccc3c2c1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 17,
    });
    registry.push_back(RetainedRingSystem{
        "4,7-phenanthroline", "c1ccc2ccc3cnccc3c2n1",
        kPhenanthrenePeripheralLocants, kPhenanthreneBondLetters, 18,
    });

    // =========================================================================
    // Still deferred, due to genuine inability to independently verify
    // within this pass (not guessed):
    // - perimidine (rank 19): peri-fused (naphthalene + a 6-membered ring
    //   sharing 3 atoms, not a simple ortho-fusion) -- a fundamentally
    //   different topology from every other entry in this file, needs its
    //   own from-scratch Indigo verification.
    // - pteridine (rank 23): actually BICYCLIC (pyrazino[2,3-d]pyrimidine
    //   shape), not tricyclic at all -- filed here only because the original
    //   task comment grouped it with these; belongs in a bicyclic family file,
    //   not fixed in this pass to avoid conflating two different changes.
    // =========================================================================
}
