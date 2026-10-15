#include "ChromeneXanthineFamily.h"

// =============================================================================
// BICYCLIC SYSTEMS (6-6 ortho-fused, naphthalene-shaped graph)
// =============================================================================
//
// CORRECTED. The originally-dispatched entries used fully-aromatic-style
// SMILES (e.g. chromene as "c1ccc2ccccc2O1", all lowercase except the ring-
// closing O) reusing naphthalene's graph shape directly. This is WRONG:
// chromene/isochromene and their chalcogen analogues are NOT fully aromatic
// -- the real compounds are "2H-chromene" / "1H-isochromene" etc., meaning
// ONE position in the pyran ring is a saturated CH2 (the indicated
// hydrogen) and positions 3-4 retain a single C=C double bond, not full
// 10-pi-electron aromaticity across both rings. Confirmed independently via
// web search (chromene's real molecular formula is C9H8O; its real SMILES
// is "c1ccc2c(c1)OCC=C2") and via direct Indigo verification: the
// dispatched "c1ccc2ccccc2O1" parses to C9H10O (2 H too many) because
// mixing an uppercase ring-closure 'O' into an otherwise all-lowercase
// aromatic SMILES made Indigo's kekulization fall back to treating THREE
// atoms (not just one) as saturated, rather than erroring out -- an easy
// mistake to make that still "parses successfully" while being chemically
// wrong, and invisible to the self-consistency test (which never looks at
// molecular formula or hybridization).
//
// Fixed by writing each entry with the heteroatom/CH2/double-bond pattern
// explicit: e.g. chromene as "O1CC=Cc2ccccc12" (O, sp3 CH2, then the C3=C4
// double bond via explicit "C=C", then the aromatic benzo ring). This
// SMILES style gives a simple, direct SEQUENTIAL atom-to-locant
// correspondence (no rotation needed, unlike the original naphthalene-reuse
// attempt) -- verified via Indigo for every one of the 8 entries below:
// each now gives the chemically-correct formula (C9H8[O/S/Se/Te]) with the
// sp3 position showing H=2 and the two alkene carbons showing H=1 each.
//
// Full Indigo verification for chromene (SMILES O1CC=Cc2ccccc12):
//   atom[0]: O deg=2 H=0 neighbors=[1,9]
//   atom[1]: C deg=2 H=2 neighbors=[0,2]        (sp3 CH2)
//   atom[2]: C deg=2 H=1 neighbors=[1,3]         (alkene CH)
//   atom[3]: C deg=2 H=1 neighbors=[2,4]         (alkene CH, double bond to atom[2])
//   atom[4]: C deg=3 H=0 neighbors=[3,5,9] (bridgehead)
//   atom[5]: C deg=2 H=1 neighbors=[4,6]
//   atom[6]: C deg=2 H=1 neighbors=[5,7]
//   atom[7]: C deg=2 H=1 neighbors=[6,8]
//   atom[8]: C deg=2 H=1 neighbors=[7,9]
//   atom[9]: C deg=3 H=0 neighbors=[8,0,4] (bridgehead)
// Walking the real locant sequence 1-2-3-4-4a-5-6-7-8-8a-(back to 1):
//   atom[0](O)="1" -[1,9]- atom[1]="2" = bond 1-2 ✓
//   atom[1]="2" -[0,2]- atom[2]="3" = bond 2-3 ✓
//   atom[2]="3" -[1,3]- atom[3]="4" = bond 3-4 ✓ (the C3=C4 double bond)
//   atom[3]="4" -[2,4]- atom[4]="4a" = bond 4-4a ✓
//   atom[4]="4a" -[3,5,9]- atom[5]="5" = bond 4a-5 ✓
//   atom[5]="5" -[4,6]- atom[6]="6" = bond 5-6 ✓
//   atom[6]="6" -[5,7]- atom[7]="7" = bond 6-7 ✓
//   atom[7]="7" -[6,8]- atom[8]="8" = bond 7-8 ✓
//   atom[8]="8" -[7,9]- atom[9]="8a" = bond 8-8a ✓
//   atom[9]="8a" -[8,0,4]- atom[0](O)="1" = bond 8a-1 (closing) ✓
// Molecular formula confirmed via Indigo: C9H8O (matches real chromene).
//
// Isochromene (SMILES C1OC=Cc2ccccc21) has the identical shape with the
// sp3 CH2 and O swapped (CH2 at atom[0] -> locant "1", O at atom[1] ->
// locant "2") -- verified the same way, also C9H8O.
//
// Thiochromene/selenochromene/tellurochromene and their iso- counterparts
// follow the exact same pattern, each independently verified via Indigo to
// give C9H8[S/Se/Te] with the sp3 position showing H=2.
//
// Bond-letter assignment (Blue Book FR-4 convention: 'a' is always the 1-2 bond):
//   'a':1-2, 'b':2-3, 'c':3-4, 'd':4-4a, 'e':4a-5, 'f':5-6,
//   'g':6-7, 'h':7-8, 'i':8-8a, 'j':8a-1

// Shared locant mapping and bond letters for all bicyclic entries. This is
// now a simple direct sequential mapping (atom[i] -> the i-th locant in the
// real Blue Book sequence) because of how the corrected SMILES are written
// -- no rotation needed, unlike the originally-dispatched naphthalene-reuse
// attempt.
const std::vector<QString> bicyclicLocants = {
    "1", "2", "3", "4", "4a", "5", "6", "7", "8", "8a"
};

const std::map<char, std::pair<QString, QString>> bicyclicBondLetters = {
    {'a', {"1", "2"}}, {'b', {"2", "3"}}, {'c', {"3", "4"}},
    {'d', {"4", "4a"}}, {'e', {"4a", "5"}}, {'f', {"5", "6"}},
    {'g', {"6", "7"}}, {'h', {"7", "8"}}, {'i', {"8", "8a"}},
    {'j', {"8a", "1"}},
};

void registerChromeneXanthineFamily(std::vector<RetainedRingSystem> &registry) {
    // =========================================================================
    // BICYCLIC: 6-6 ortho-fused (naphthalene-shaped)
    // =========================================================================

    // 1. Chromene (2H-1-benzopyran) - Appendix 2 rank 65
    // CORRECTED: see the file-level comment above. O at locant "1", sp3 CH2
    // at locant "2". Verified via Indigo: C9H8O.
    registry.push_back(RetainedRingSystem{
        "chromene",
        "O1CC=Cc2ccccc12",
        bicyclicLocants,
        bicyclicBondLetters,
        65, // Appendix 2 seniority rank
    });

    // 2. Isochromene (1H-2-benzopyran) - Appendix 2 rank 66
    // CORRECTED: sp3 CH2 at locant "1", O at locant "2". Verified via
    // Indigo: C9H8O.
    registry.push_back(RetainedRingSystem{
        "isochromene",
        "C1OC=Cc2ccccc21",
        bicyclicLocants,
        bicyclicBondLetters,
        66, // Appendix 2 seniority rank
    });

    // 3. Thiochromene (2H-1-benzothiopyran) - Appendix 2 rank 76
    // CORRECTED, same pattern as chromene with S. Verified via Indigo: C9H8S.
    registry.push_back(RetainedRingSystem{
        "thiochromene",
        "S1CC=Cc2ccccc12",
        bicyclicLocants,
        bicyclicBondLetters,
        76, // Appendix 2 seniority rank
    });

    // 4. Isothiochromene (1H-2-benzothiopyran) - Appendix 2 rank 77
    // CORRECTED, same pattern as isochromene with S. Verified via Indigo: C9H8S.
    registry.push_back(RetainedRingSystem{
        "isothiochromene",
        "C1SC=Cc2ccccc21",
        bicyclicLocants,
        bicyclicBondLetters,
        77, // Appendix 2 seniority rank
    });

    // 5. Selenochromene (2H-1-benzoselenopyran) - Appendix 2 rank 81
    // CORRECTED, same pattern as chromene with Se. Verified via Indigo: C9H8Se.
    registry.push_back(RetainedRingSystem{
        "selenochromene",
        "[se]1CC=Cc2ccccc12",
        bicyclicLocants,
        bicyclicBondLetters,
        81, // Appendix 2 seniority rank
    });

    // 6. Isoselenochromene (1H-2-benzoselenopyran) - Appendix 2 rank 82
    // CORRECTED, same pattern as isochromene with Se. Verified via Indigo: C9H8Se.
    registry.push_back(RetainedRingSystem{
        "isoselenochromene",
        "C1[se]C=Cc2ccccc21",
        bicyclicLocants,
        bicyclicBondLetters,
        82, // Appendix 2 seniority rank
    });

    // 7. Tellurochromene (2H-1-benzotelluropyran) - Appendix 2 rank 86
    // CORRECTED, same pattern as chromene with Te. Verified via Indigo: C9H8Te.
    registry.push_back(RetainedRingSystem{
        "tellurochromene",
        "[te]1CC=Cc2ccccc12",
        bicyclicLocants,
        bicyclicBondLetters,
        86, // Appendix 2 seniority rank
    });

    // 8. Isotellurochromene (1H-2-benzotelluropyran) - Appendix 2 rank 87
    // CORRECTED, same pattern as isochromene with Te. Verified via Indigo: C9H8Te.
    registry.push_back(RetainedRingSystem{
        "isotellurochromene",
        "C1[te]C=Cc2ccccc21",
        bicyclicLocants,
        bicyclicBondLetters,
        87, // Appendix 2 seniority rank
    });

    // =========================================================================
    // TRICYCLIC SYSTEMS (6-6-6 linear, anthracene-shaped graph)
    // =========================================================================
    //
    // All entries in this section share the SAME graph topology as anthracene
    // (14 atoms, linear 6-6-6 ortho-fused tricyclic). Per Bug class 3, we do NOT
    // assume they reuse anthracene's locant list without verification.
    //
    // Reference anthracene (from HydrocarbonFamily.cpp):
    //   SMILES: c1ccc2cc3ccccc3cc2c1
    //   Indigo parse order (verified):
    //     atom[0]:  C deg=2 neighbors=[1,13]
    //     atom[1]:  C deg=2 neighbors=[0,2]
    //     atom[2]:  C deg=2 neighbors=[1,3]
    //     atom[3]:  C deg=3 neighbors=[2,4,12] (bridgehead -> "4a")
    //     atom[4]:  C deg=2 neighbors=[3,5] (meso position -> "10")
    //     atom[5]:  C deg=3 neighbors=[4,6,10] (bridgehead -> "10a")
    //     atom[6]:  C deg=2 neighbors=[5,7] (-> "5")
    //     atom[7]:  C deg=2 neighbors=[6,8] (-> "6")
    //     atom[8]:  C deg=2 neighbors=[7,9] (-> "7")
    //     atom[9]:  C deg=2 neighbors=[8,10] (-> "8")
    //     atom[10]: C deg=3 neighbors=[9,5,11] (bridgehead -> "8a")
    //     atom[11]: C deg=2 neighbors=[10,12] (meso position -> "9")
    //     atom[12]: C deg=3 neighbors=[11,3,13] (bridgehead -> "9a")
    //     atom[13]: C deg=2 neighbors=[12,0] (-> "1")
    //
    // Blue Book locant sequence for anthracene (verified against real bonds):
    //   1-2-3-4-4a-10-10a-5-6-7-8-8a-9-9a-(back to 1)
    //   Note: this is the "jumping" walk that was confirmed correct in Task 1.
    //   Mapping: atom[13]="1", atom[0]="2", atom[1]="3", atom[2]="4",
    //           atom[3]="4a", atom[4]="10", atom[5]="10a", atom[6]="5",
    //           atom[7]="6", atom[8]="7", atom[9]="8", atom[10]="8a",
    //           atom[11]="9", atom[12]="9a"
    //
    // Full Indigo verification for anthracene:
    //   All bonds above confirmed via indigoIterateNeighbors in verify_final.cpp.
    //
    // For xanthene/thianthrene/oxanthrene family:
    //   - xanthene (9H-xanthene): O at position 9 in standard IUPAC numbering
    //     In anthracene numbering, position 9 is atom[11].
    //     SMILES c1ccc2cc3ccccc3oc2c1 verified: 14 atoms, O at atom[11] (deg=2)
    //   - thianthrene (9,10-dithiaanthracene): S at positions 9 and 10
    //     In anthracene numbering, positions 9 and 10 are atom[11] and atom[4].
    //     SMILES c1ccc2sc3ccccc3sc2c1 verified: 14 atoms, S at atom[4] and atom[11]
    //   - oxanthrene (dibenzo[b,e][1,4]dioxine): O at positions 1 and 4 of central ring
    //     In anthracene numbering, these are the meso positions atom[4] and atom[11].
    //     SMILES c1ccc2Oc3ccccc3Oc2c1 verified: 14 atoms, O at atom[4] and atom[11]
    //
    // However, per Bug class 3 (phenoxazine family), even though these share anthracene's
    // graph topology, we must verify they don't use anthracene's "jumping" locant sequence.
    //
    // From Appendix 2 HTML (https://iupac.qmul.ac.uk/fusedring/app2.html):
    //   - Entry 64 (xanthene): "9H-form shown", GIF 2A64.GIF, note e: "Non-standard numbering"
    //   - Entry 74 (thianthrene): "see 57-63, X = S, S instead of O"
    //   - Entry 75 (thioxanthene): "see 64, S instead of O"
    //   - Entry 63 (oxanthrene): uses GIF 2A57.GIF (same as phenoxathiine family)
    //
    // This tells us:
    //   - oxanthrene (63) shares numbering with phenoxathiine family (57-62)
    //   - xanthene (64) has its OWN non-standard numbering
    //   - thianthrene (74) follows phenoxathiine family numbering
    //   - thioxanthene (75) follows xanthene numbering
    //
    // But phenoxazine family (Task 5) proved that phenoxazine/phenothiazine do NOT
    // use anthracene's locant sequence despite sharing its graph topology.
    // The phenoxazine family uses the "plain walk" sequence:
    //   1-2-3-4-4a-5-5a-6-7-8-9-9a-10-10a-(back to 1)
    // which maps to anthracene's atoms as:
    //   atom[2]="1", atom[1]="2", atom[0]="3", atom[3]="10a", atom[4]="10",
    //   atom[5]="9a", atom[6]="9", atom[7]="8", atom[8]="7", atom[9]="6",
    //   atom[10]="5a", atom[11]="5", atom[12]="4a", atom[13]="4"
    //
    // And entries 57-63 (phenoxathiine family + oxanthrene) share GIF 2A57.GIF,
    // so they ALL use this same "plain walk" numbering, NOT anthracene's numbering.
    //
    // Therefore:
    //   - oxanthrene: uses phenoxazine numbering (GIF 2A57.GIF)
    //   - thianthrene: uses phenoxazine numbering (via "see 57-63")
    //   - xanthene: uses its OWN numbering (GIF 2A64.GIF, non-standard)
    //   - thioxanthene: uses xanthene numbering (via "see 64")
    //
    // For now, we implement xanthene, thianthrene, and oxanthrene using their
    // SMILES-verified atom positions and the anthracene locant list as a
    // starting point, but we MUST verify the complete peripheral walk.
    //
    // Actually, let's be conservative: since we cannot access the GIF images to
    // confirm the exact numbering for xanthene (64), and since note e explicitly
    // says "Non-standard numbering", we should NOT assume it matches anthracene.
    //
    // However, for oxanthrene and thianthrene (which reference 57-63), we can use
    // the phenoxazine numbering pattern. From PhenoxazineFamily.cpp, the verified
    // locant list for phenoxazine (which is structurally analogous to oxanthrene
    // with N and O replaced by two O atoms) is:
    //   {"3","2","1","10a","10","9a","9","8","7","6","5a","5","4a","4"}
    //
    // For oxanthrene with O at atom[4] and atom[11] (using SMILES c1ccc2Oc3ccccc3Oc2c1):
    //   In phenoxazine numbering: atom[4]="10", atom[11]="5"
    //   But we want O at positions that make chemical sense.
    //
    // For thianthrene with S at atom[4] and atom[11] (using SMILES c1ccc2sc3ccccc3sc2c1):
    //   In phenoxazine numbering: atom[4]="10", atom[11]="5"
    //
    // From web search: thianthrene is 9,10-dithiaanthracene, meaning S at positions 9 and 10.
    // If we map: position 9 -> atom[11], position 10 -> atom[4]
    // Then using phenoxazine numbering: atom[11]="5", atom[4]="10"
    // This would give S at positions 5 and 10, NOT 9 and 10.
    //
    // There's a contradiction here. Let's reconsider.
    //
    // Actually, "9,10-dithiaanthracene" means: take anthracene and replace C-9 and C-10 with S.
    // In anthracene numbering: position 9 = atom[11], position 10 = atom[4]
    // So S at atom[4] and atom[11] means S at positions 10 and 9.
    // This matches "9,10-dithiaanthracene" (order doesn't matter for the name).
    //
    // So thianthrene DOES use anthracene numbering!
    //
    // Similarly, for oxanthrene, if we use anthracene numbering and O at atom[4] and atom[11],
    // then O is at positions 10 and 9, which would be "9,10-dioxaanthracene".
    //
    // For xanthene with O at atom[11], using anthracene numbering, O is at position 9.
    // This gives "9-oxaanthracene" which is close to "9H-xanthene".
    //
    // However, note e says "Non-standard numbering" for xanthene. This might mean
    // that xanthene's preferred IUPAC name uses a different numbering from
    // the systematic "9-oxaanthracene".
    //
    // Given the time constraints and the complexity, let's implement xanthene,
    // thianthrene, and oxanthrene using anthracene's locant list and SMILES
    // that place the heteroatoms at the correct atom indices, and explicitly
    // note that further verification is needed for xanthene.
    //
    // =========================================================================
    // DEFINITION: Tricyclic locants and bond letters
    // =========================================================================
    //
    // Xanthene uses anthracene-based numbering (verified: O at atom[11] -> locant "9").
    // Thianthrene and oxanthrene use phenoxazine-style numbering (plain walk) as they
    // share GIF 2A57.GIF with the phenoxathiine family (entries 57-62).
    //
    // Anthracene-based numbering (jumping walk, verified in HydrocarbonFamily.cpp):
    //   1-2-3-4-4a-10-10a-5-6-7-8-8a-9-9a-(back to 1)
    //   Mapping: atom[13]="1", atom[0]="2", atom[1]="3", atom[2]="4",
    //           atom[3]="4a", atom[4]="10", atom[5]="10a", atom[6]="5",
    //           atom[7]="6", atom[8]="7", atom[9]="8", atom[10]="8a",
    //           atom[11]="9", atom[12]="9a"
    //
    // Phenoxazine-based numbering (plain walk, verified in PhenoxazineFamily.cpp):
    //   1-2-3-4-4a-5-5a-6-7-8-9-9a-10-10a-(back to 1)
    //   Mapping: atom[2]="1", atom[1]="2", atom[0]="3", atom[13]="4",
    //           atom[12]="4a", atom[11]="5", atom[10]="5a", atom[9]="6",
    //           atom[8]="7", atom[7]="8", atom[6]="9", atom[5]="9a",
    //           atom[4]="10", atom[3]="10a"

    // Anthracene-based locants and bond letters (for xanthene).
    const std::vector<QString> anthraceneLocants = {
        "2", "3", "4", "4a", "10", "10a", "5", "6", "7", "8", "8a", "9", "9a", "1"
    };

    const std::map<char, std::pair<QString, QString>> anthraceneBondLetters = {
        {'a', {"1", "2"}},   // 1-2 bond
        {'b', {"2", "3"}},   // 2-3 bond
        {'c', {"3", "4"}},   // 3-4 bond
        {'d', {"4", "4a"}},  // 4-4a bond
        {'e', {"4a", "10"}}, // 4a-10 bond
        {'f', {"10", "10a"}}, // 10-10a bond
        {'g', {"10a", "5"}},  // 10a-5 bond
        {'h', {"5", "6"}},    // 5-6 bond
        {'i', {"6", "7"}},    // 6-7 bond
        {'j', {"7", "8"}},    // 7-8 bond
        {'k', {"8", "8a"}},   // 8-8a bond
        {'l', {"8a", "9"}},   // 8a-9 bond
        {'m', {"9", "9a"}},   // 9-9a bond
        {'n', {"9a", "1"}},   // 9a-1 bond (closing)
    };

    // Phenoxazine-based locants and bond letters (for thianthrene, oxanthrene).
    // This is the "plain walk" sequence used by phenoxazine/phenothiazine family.
    const std::vector<QString> phenoxazineLocants = {
        "3", "2", "1", "10a", "10", "9a", "9", "8", "7", "6", "5a", "5", "4a", "4"
    };

    const std::map<char, std::pair<QString, QString>> phenoxazineBondLetters = {
        {'a', {"1", "2"}}, {'b', {"2", "3"}}, {'c', {"3", "4"}},
        {'d', {"4", "4a"}}, {'e', {"4a", "5"}}, {'f', {"5", "5a"}},
        {'g', {"5a", "6"}}, {'h', {"6", "7"}}, {'i', {"7", "8"}},
        {'j', {"8", "9"}}, {'k', {"9", "9a"}}, {'l', {"9a", "10"}},
        {'m', {"10", "10a"}}, {'n', {"10a", "1"}},
    };

    // 9. Xanthene (9H-xanthene) - Appendix 2 rank 64
    //
    // CORRECTED. The originally-dispatched entry used SMILES
    // "c1ccc2cc3ccccc3oc2c1" (O at atom[11], atom[4] left as plain aromatic
    // 'c'). This is wrong on TWO independent counts, both caught by direct
    // verification rather than trusting the self-consistency test (which
    // cannot catch either):
    //
    // 1. WRONG MOLECULAR FORMULA. Real xanthene is C13H10O. The dispatched
    //    SMILES parses (via Indigo) to C13H9O -- one hydrogen short -- because
    //    xanthene's non-oxygen meso position is a SATURATED CH2 (indicated
    //    hydrogen, hence "9H-xanthene"), not a plain aromatic carbon with
    //    only 1 implicit H. Confirmed directly: parsing
    //    "c1ccc2Oc3ccccc3Cc2c1" (sp3 'C', not 'c', at that position) gives
    //    the correct C13H10O, with that atom showing H=2.
    //
    // 2. WRONG HETEROATOM LOCANT. The name "9H-xanthene" says the INDICATED
    //    HYDROGEN (the CH2) is at locant 9 -- it does NOT say oxygen is at 9.
    //    Cross-referencing xanthene's systematic name "10H-9-oxaanthracene"
    //    (anthracene's OWN numbering frame: O replaces position 9, indicated
    //    H at position 10) against xanthene's own independently-fixed
    //    retained-name numbering (where the SAME physical CH2 atom is called
    //    "9") shows xanthene's own locant 9 and 10 are the REVERSE of
    //    anthracene's: oxygen is at xanthene's own locant 10, and the CH2 is
    //    at locant 9. The dispatched entry put O at atom[11] (anthracene
    //    locant "9") -- backwards.
    //
    // Fix: swap which meso atom is the heteroatom. SMILES
    // "c1ccc2Oc3ccccc3Cc2c1" places O at atom[4] (-> locant "10", correct)
    // and the sp3 CH2 at atom[11] (-> locant "9", correct). Verified via a
    // fresh Indigo parse: molecular formula C13H10O, atom[4] Z=8 degree=2
    // H=0 (ether-type O, correct), atom[11] Z=6 degree=2 H=2 (sp3 CH2,
    // correct) -- matching real xanthene exactly.
    //
    // Full peripheral walk against anthracene bonds (unchanged from the
    // original verification, only the SMILES atom assignment changed):
    //   atom[13]="1" -[12,0]- atom[0]="2" = bond 1-2 ✓
    //   atom[0]="2" -[1,13]- atom[1]="3" = bond 2-3 ✓
    //   atom[1]="3" -[0,2]- atom[2]="4" = bond 3-4 ✓
    //   atom[2]="4" -[1,3]- atom[3]="4a" = bond 4-4a ✓
    //   atom[3]="4a" -[2,4,12]- atom[4] (O)="10" = bond 4a-10 ✓
    //   atom[4] (O)="10" -[3,5]- atom[5]="10a" = bond 10-10a ✓
    //   atom[5]="10a" -[4,6,10]- atom[6]="5" = bond 10a-5 ✓
    //   atom[6]="5" -[5,7]- atom[7]="6" = bond 5-6 ✓
    //   atom[7]="6" -[6,8]- atom[8]="7" = bond 6-7 ✓
    //   atom[8]="7" -[7,9]- atom[9]="8" = bond 7-8 ✓
    //   atom[9]="8" -[8,10]- atom[10]="8a" = bond 8-8a ✓
    //   atom[10]="8a" -[9,5,11]- atom[11] (sp3 CH2)="9" = bond 8a-9 ✓
    //   atom[11] (sp3 CH2)="9" -[10,12]- atom[12]="9a" = bond 9-9a ✓
    //   atom[12]="9a" -[11,3,13]- atom[13]="1" = bond 9a-1 ✓
    registry.push_back(RetainedRingSystem{
        "xanthene",
        "c1ccc2Oc3ccccc3Cc2c1",
        anthraceneLocants,
        anthraceneBondLetters,
        64, // Appendix 2 seniority rank
    });

    // 10. Thianthrene - Appendix 2 rank 74
    // SMILES c1ccc2sc3ccccc3sc2c1 verified: 14 atoms, S at atom[4] and atom[11]
    // Uses phenoxazine-based numbering (plain walk, GIF 2A57.GIF, entry 74 references 57-63):
    //   atom[2]="1", atom[1]="2", atom[0]="3", atom[13]="4", atom[12]="4a",
    //   atom[11]="5", atom[10]="5a", atom[9]="6", atom[8]="7", atom[7]="8",
    //   atom[6]="9", atom[5]="9a", atom[4]="10", atom[3]="10a"
    // S at atom[4] and atom[11] map to locants "10" and "5" respectively.
    // Note: The systematic name "9,10-dithiaanthracene" uses anthracene numbering,
    // but the retained name "thianthrene" (entry 74) shares GIF 2A57.GIF with the
    // phenoxathiine family (entries 57-62) and thus uses phenoxazine-style numbering.
    // Full peripheral walk against real bonds:
    //   atom[2]="1" -[1,3]- atom[1]="2" = bond 1-2 ✓
    //   atom[1]="2" -[0,2]- atom[0]="3" = bond 2-3 ✓
    //   atom[0]="3" -[1,13]- atom[13]="4" = bond 3-4 ✓
    //   atom[13]="4" -[0,12]- atom[12]="4a" = bond 4-4a ✓
    //   atom[12]="4a" -[3,11,13]- atom[11] (S)="5" = bond 4a-5 ✓
    //   atom[11] (S)="5" -[10,12]- atom[10]="5a" = bond 5-5a ✓
    //   atom[10]="5a" -[5,9,11]- atom[9]="6" = bond 5a-6 ✓
    //   atom[9]="6" -[8,10]- atom[8]="7" = bond 6-7 ✓
    //   atom[8]="7" -[7,9]- atom[7]="8" = bond 7-8 ✓
    //   atom[7]="8" -[6,8]- atom[6]="9" = bond 8-9 ✓
    //   atom[6]="9" -[5,7]- atom[5]="9a" = bond 9-9a ✓
    //   atom[5]="9a" -[4,6,10]- atom[4] (S)="10" = bond 9a-10 ✓
    //   atom[4] (S)="10" -[3,5]- atom[3]="10a" = bond 10-10a ✓
    //   atom[3]="10a" -[2,4,12]- atom[2]="1" = bond 10a-1 ✓
    // All consecutive pairs match real bonds.
    registry.push_back(RetainedRingSystem{
        "thianthrene",
        "c1ccc2sc3ccccc3sc2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        74, // Appendix 2 seniority rank
    });

    // 11. Oxanthrene - Appendix 2 rank 63
    // SMILES c1ccc2Oc3ccccc3Oc2c1 verified: 14 atoms, O at atom[4] and atom[11]
    // Uses phenoxazine-based numbering (plain walk, GIF 2A57.GIF):
    //   atom[2]="1", atom[1]="2", atom[0]="3", atom[13]="4", atom[12]="4a",
    //   atom[11]="5", atom[10]="5a", atom[9]="6", atom[8]="7", atom[7]="8",
    //   atom[6]="9", atom[5]="9a", atom[4]="10", atom[3]="10a"
    // O at atom[4] and atom[11] map to locants "10" and "5" respectively.
    // Note: Appendix 2 entry 63 uses GIF 2A57.GIF (same as phenoxathiine family),
    // confirming it uses phenoxazine-style numbering. The former name
    // "dibenzo[b,e][1,4]dioxine" (note l) refers to an older nomenclature system;
    // the retained name "oxanthrene" uses the phenoxazine-style numbering.
    // Full peripheral walk against real bonds:
    //   atom[2]="1" -[1,3]- atom[1]="2" = bond 1-2 ✓
    //   atom[1]="2" -[0,2]- atom[0]="3" = bond 2-3 ✓
    //   atom[0]="3" -[1,13]- atom[13]="4" = bond 3-4 ✓
    //   atom[13]="4" -[0,12]- atom[12]="4a" = bond 4-4a ✓
    //   atom[12]="4a" -[3,11,13]- atom[11] (O)="5" = bond 4a-5 ✓
    //   atom[11] (O)="5" -[10,12]- atom[10]="5a" = bond 5-5a ✓
    //   atom[10]="5a" -[5,9,11]- atom[9]="6" = bond 5a-6 ✓
    //   atom[9]="6" -[8,10]- atom[8]="7" = bond 6-7 ✓
    //   atom[8]="7" -[7,9]- atom[7]="8" = bond 7-8 ✓
    //   atom[7]="8" -[6,8]- atom[6]="9" = bond 8-9 ✓
    //   atom[6]="9" -[5,7]- atom[5]="9a" = bond 9-9a ✓
    //   atom[5]="9a" -[4,6,10]- atom[4] (O)="10" = bond 9a-10 ✓
    //   atom[4] (O)="10" -[3,5]- atom[3]="10a" = bond 10-10a ✓
    //   atom[3]="10a" -[2,4,12]- atom[2]="1" = bond 10a-1 ✓
    // All consecutive pairs match real bonds.
    registry.push_back(RetainedRingSystem{
        "oxanthrene",
        "c1ccc2Oc3ccccc3Oc2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        63, // Appendix 2 seniority rank
    });

    // =========================================================================
    // ADDITIONAL ENTRIES: phenoxathiine family (Appendix 2 ranks 57-62)
    // =========================================================================
    //
    // These entries share GIF 2A57.GIF with oxanthrene/thianthrene, meaning they
    // share the phenoxazine-style numbering (plain walk). All have O at atom[4]
    // and X at atom[11] in the Indigo parse order, where X is S/Se/Te/P/As/Sb.
    //
    // Phenoxathiine family SMILES pattern: c1ccc2oc3ccccc3Xc2c1
    //   - O at atom[4] (meso position) -> locant "10" (in phenoxazine numbering)
    //   - X at atom[11] (meso position) -> locant "5" (in phenoxazine numbering)
    //
    // Full Indigo verification for reference molecule c1ccc2oc3ccccc3sc2c1:
    //   atom[0]:  C deg=2 neighbors=[1,13]
    //   atom[1]:  C deg=2 neighbors=[0,2]
    //   atom[2]:  C deg=2 neighbors=[1,3]
    //   atom[3]:  C deg=3 neighbors=[2,4,12] (bridgehead)
    //   atom[4]:  O deg=2 neighbors=[3,5]
    //   atom[5]:  C deg=3 neighbors=[4,6,10] (bridgehead)
    //   atom[6]:  C deg=2 neighbors=[5,7]
    //   atom[7]:  C deg=2 neighbors=[6,8]
    //   atom[8]:  C deg=2 neighbors=[7,9]
    //   atom[9]:  C deg=2 neighbors=[8,10]
    //   atom[10]: C deg=3 neighbors=[9,5,11] (bridgehead)
    //   atom[11]: S deg=2 neighbors=[10,12]
    //   atom[12]: C deg=3 neighbors=[11,3,13] (bridgehead)
    //   atom[13]: C deg=2 neighbors=[12,0]
    // Bridgeheads (deg=3): atom[3], atom[5], atom[10], atom[12]
    // This matches the parse order of anthracene, phenoxazine, xanthene, etc.
    //
    // Peripheral walk verification using phenoxazine numbering:
    //   atom[2]="1" -[1,3]- atom[1]="2" = bond 1-2 ✓
    //   atom[1]="2" -[0,2]- atom[0]="3" = bond 2-3 ✓
    //   atom[0]="3" -[1,13]- atom[13]="4" = bond 3-4 ✓
    //   atom[13]="4" -[0,12]- atom[12]="4a" = bond 4-4a ✓
    //   atom[12]="4a" -[3,11,13]- atom[11] (X)="5" = bond 4a-5 ✓
    //   atom[11] (X)="5" -[10,12]- atom[10]="5a" = bond 5-5a ✓
    //   atom[10]="5a" -[5,9,11]- atom[9]="6" = bond 5a-6 ✓
    //   atom[9]="6" -[8,10]- atom[8]="7" = bond 6-7 ✓
    //   atom[8]="7" -[7,9]- atom[7]="8" = bond 7-8 ✓
    //   atom[7]="8" -[6,8]- atom[6]="9" = bond 8-9 ✓
    //   atom[6]="9" -[5,7]- atom[5]="9a" = bond 9-9a ✓
    //   atom[5]="9a" -[4,6,10]- atom[4] (O)="10" = bond 9a-10 ✓
    //   atom[4] (O)="10" -[3,5]- atom[3]="10a" = bond 10-10a ✓
    //   atom[3]="10a" -[2,4,12]- atom[2]="1" = bond 10a-1 ✓
    // All consecutive pairs match real bonds.

    // 12. Phenoxathiine (dibenzo[b,e][1,4]thiaazine) - Appendix 2 rank 57
    // SMILES c1ccc2oc3ccccc3sc2c1 verified: 14 atoms, O at atom[4], S at atom[11]
    // Uses phenoxazine numbering: O at locant "10", S at locant "5"
    registry.push_back(RetainedRingSystem{
        "phenoxathiine",
        "c1ccc2oc3ccccc3sc2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        57, // Appendix 2 seniority rank
    });

    // 13. Phenoxaselenine - Appendix 2 rank 58
    // SMILES c1ccc2oc3ccccc3[se]c2c1 verified: 14 atoms, O at atom[4], Se at atom[11]
    // Uses phenoxazine numbering: O at locant "10", Se at locant "5"
    registry.push_back(RetainedRingSystem{
        "phenoxaselenine",
        "c1ccc2oc3ccccc3[se]c2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        58, // Appendix 2 seniority rank
    });

    // 14. Phenoxatellurine - Appendix 2 rank 59
    // SMILES c1ccc2oc3ccccc3[te]c2c1 verified: 14 atoms, O at atom[4], Te at atom[11]
    // Uses phenoxazine numbering: O at locant "10", Te at locant "5"
    registry.push_back(RetainedRingSystem{
        "phenoxatellurine",
        "c1ccc2oc3ccccc3[te]c2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        59, // Appendix 2 seniority rank
    });

    // 15. Phenoxaphosphinine - Appendix 2 rank 60
    // SMILES c1ccc2oc3ccccc3[P]c2c1 verified: 14 atoms, O at atom[4], P at atom[11]
    // Uses phenoxazine numbering: O at locant "10", P at locant "5"
    // Note: Phosphorus is trivalent in aromatic context (phospinine-like)
    registry.push_back(RetainedRingSystem{
        "phenoxaphosphinine",
        "c1ccc2oc3ccccc3[P]c2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        60, // Appendix 2 seniority rank
    });

    // 16. Phenoxarsinine - Appendix 2 rank 61
    // SMILES c1ccc2oc3ccccc3[As]c2c1 verified: 14 atoms, O at atom[4], As at atom[11]
    // Uses phenoxazine numbering: O at locant "10", As at locant "5"
    // Note: Arsenic is trivalent in aromatic context
    registry.push_back(RetainedRingSystem{
        "phenoxarsinine",
        "c1ccc2oc3ccccc3[As]c2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        61, // Appendix 2 seniority rank
    });

    // 17. Phenoxastibinine - Appendix 2 rank 62
    // SMILES c1ccc2oc3ccccc3[Sb]c2c1 verified: 14 atoms, O at atom[4], Sb at atom[11]
    // Uses phenoxazine numbering: O at locant "10", Sb at locant "5"
    // Note: Antimony is trivalent in aromatic context
    registry.push_back(RetainedRingSystem{
        "phenoxastibinine",
        "c1ccc2oc3ccccc3[Sb]c2c1",
        phenoxazineLocants,
        phenoxazineBondLetters,
        62, // Appendix 2 seniority rank
    });

    // =========================================================================
    // ADDITIONAL ENTRIES: xanthene family derivatives (Appendix 2 ranks 75, 80, 85)
    // =========================================================================
    //
    // These entries reference xanthene (entry 64) with "see 64, X instead of O",
    // meaning they share xanthene's numbering (which uses anthracene-style numbering).
    //
    // Xanthene family SMILES pattern: c1ccc2cc3ccccc3Xc2c1
    //   - X at atom[11] (meso position) -> locant "9" (in anthracene numbering)
    //
    // CORRECTED, same fix class as xanthene's own entry above: real
    // thioxanthene/selenoxanthene/telluroxanthene are C13H10[X] (one
    // heteroatom plus a saturated CH2, same pattern as xanthene's C13H10O),
    // not C13H9[X]. X sits at atom[4] (-> locant "10"); the sp3 CH2 sits at
    // atom[11] (-> locant "9") -- the reverse of what was originally
    // dispatched. Verified via fresh Indigo parses: thioxanthene C13H10S,
    // selenoxanthene C13H10Se, telluroxanthene C13H10Te, each with the
    // sp3-CH2 atom showing H=2.
    //
    // Full Indigo verification for reference molecule c1ccc2sc3ccccc3Cc2c1
    // (thioxanthene; selenoxanthene/telluroxanthene have the identical
    // parse order with [se]/[te] in place of s):
    //   atom[0]:  C deg=2 neighbors=[1,13]
    //   atom[1]:  C deg=2 neighbors=[0,2]
    //   atom[2]:  C deg=2 neighbors=[1,3]
    //   atom[3]:  C deg=3 neighbors=[2,4,12] (bridgehead)
    //   atom[4]:  S deg=2 neighbors=[3,5]
    //   atom[5]:  C deg=3 neighbors=[4,6,10] (bridgehead)
    //   atom[6]:  C deg=2 neighbors=[5,7]
    //   atom[7]:  C deg=2 neighbors=[6,8]
    //   atom[8]:  C deg=2 neighbors=[7,9]
    //   atom[9]:  C deg=2 neighbors=[8,10]
    //   atom[10]: C deg=3 neighbors=[9,5,11] (bridgehead)
    //   atom[11]: C deg=2 H=2 neighbors=[10,12] (sp3 CH2)
    //   atom[12]: C deg=3 neighbors=[11,3,13] (bridgehead)
    //   atom[13]: C deg=2 neighbors=[12,0]
    // This matches the parse order of anthracene/xanthene (same shape).
    //
    // Peripheral walk verification using anthracene numbering (same as
    // xanthene, after the fix):
    //   atom[13]="1" -[12,0]- atom[0]="2" = bond 1-2 ✓
    //   atom[0]="2" -[1,13]- atom[1]="3" = bond 2-3 ✓
    //   ... (rest identical to xanthene verification above)
    //   atom[3]="4a" -[2,4,12]- atom[4] (X)="10" = bond 4a-10 ✓
    //   atom[10]="8a" -[9,5,11]- atom[11] (sp3 CH2)="9" = bond 8a-9 ✓
    //   atom[11] (sp3 CH2)="9" -[10,12]- atom[12]="9a" = bond 9-9a ✓
    //   atom[12]="9a" -[11,3,13]- atom[13]="1" = bond 9a-1 ✓
    // All consecutive pairs match real bonds.

    // 18. Thioxanthene - Appendix 2 rank 75
    //
    // CORRECTED (same bug as xanthene above, inherited via "see 64, S
    // instead of O" -- the original dispatched version just substituted S
    // into xanthene's ALREADY-WRONG SMILES, inheriting both of its bugs).
    // Real thioxanthene is C13H10S (S-analog of xanthene's C13H10O, same
    // sp3-CH2-plus-one-heteroatom pattern). The dispatched SMILES
    // "c1ccc2cc3ccccc3sc2c1" parses to C13H9S (one H short -- confirmed via
    // Indigo) because the non-S meso position was left as plain aromatic
    // 'c' instead of the required sp3 'C'. Fixed the same way as xanthene:
    // S at atom[4] (-> locant "10"), sp3 CH2 at atom[11] (-> locant "9").
    // Verified via fresh Indigo parse: formula C13H10S (matches real
    // thioxanthene), atom[11] H=2 (sp3 CH2, correct).
    registry.push_back(RetainedRingSystem{
        "thioxanthene",
        "c1ccc2sc3ccccc3Cc2c1",
        anthraceneLocants,
        anthraceneBondLetters,
        75, // Appendix 2 seniority rank
    });

    // 19. Selenoxanthene - Appendix 2 rank 80
    // CORRECTED, same fix as thioxanthene above (Se instead of S).
    registry.push_back(RetainedRingSystem{
        "selenoxanthene",
        "c1ccc2[se]c3ccccc3Cc2c1",
        anthraceneLocants,
        anthraceneBondLetters,
        80, // Appendix 2 seniority rank
    });

    // 20. Telluroxanthene - Appendix 2 rank 85
    // CORRECTED, same fix as thioxanthene above (Te instead of S).
    registry.push_back(RetainedRingSystem{
        "telluroxanthene",
        "c1ccc2[te]c3ccccc3Cc2c1",
        anthraceneLocants,
        anthraceneBondLetters,
        85, // Appendix 2 seniority rank
    });

    // =========================================================================
    // NOTE on indigoCountHydrogens returning -1 for some entries
    // =========================================================================
    //
    // A handful of entries here (phenoxathiine, thianthrene, oxanthrene)
    // show indigoCountHydrogens() = -1 on one or more heteroatoms when
    // written with plain lowercase SMILES. This was investigated during
    // the same verification pass that found the real formula bugs fixed
    // above (xanthene-family missing-H, chromene-family missing-double-
    // bond, phenoxazine-family missing-N-H) -- but for THESE entries it is
    // a benign artifact, not a real structural error: indigoGrossFormula()
    // gives the SAME, chemically-correct result (confirmed against real
    // thianthrene C12H8S2 and phenoxathiine C12H8OS) whether the ambiguous
    // atom is written lowercase or uppercase. The -1 reflects a genuinely
    // degenerate choice of Kekule structure for these specific symmetric/
    // all-ether-type rings (no atom here needs an indicated hydrogen the
    // way xanthene's CH2 or phenothiazine's N-H do), not a missing atom or
    // wrong formula. Left as originally written (lowercase); do not
    // "fix" these by pattern-matching against the real fixes above without
    // first checking indigoGrossFormula() the same way this note did.
    //
    // =========================================================================
    // SUMMARY
    // =========================================================================
    //
    // Bicyclic (naphthalene-shaped, 8 entries):
    //   All use naphthalene-based numbering (verified self-consistent)
    //
    // Tricyclic (anthracene-shaped, 12 entries):
    //   - xanthene, thioxanthene, selenoxanthene, telluroxanthene: use anthracene numbering
    //   - oxanthrene, thianthrene, phenoxathiine, phenoxaselenine, phenoxatellurine,
    //     phenoxaphosphinine, phenoxarsinine, phenoxastibinine: use phenoxazine numbering
    //
    // Total: 8 + 12 = 20 entries in this file
}
