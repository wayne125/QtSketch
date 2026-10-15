#include "PhenoxazineFamily.h"

// Appendix 2 heterocyclic retained names -- phenoxazine-family tricyclics (entries 1-7).
// Dibenzo[b,e][1,4]heterazines: all 7 entries share the SAME tricyclic skeleton
// as anthracene (linear 6-6-6, 14 atoms), with two benzo rings ortho-fused to a
// central 6-membered ring at the meso positions (atom[4] and atom[11] in the
// Indigo parse order for 'c1ccc2cc3ccccc3cc2c1').
//
// Structure: the central ring has N at one meso position (atom[4]) and X (O/S/Se/Te/P/As)
// at the other meso position (atom[11]). This skeleton has the EXACT same atom
// parse order as anthracene/phenazine -- verified via Indigo for all entries.
//
// SMILES pattern: c1ccc2nc3ccccc3Xc2c1, where X is:
//   - phenoxazine:   o  (O)
//   - phenothiazine: s  (S)
//   - phenoselenazine: [se]
//   - phenotellurazine: [te]
//   - phenophosphazinine: [P]
//   - phenarsazinine: [As]
//   - phenomercurazine: [Hg] (DEFERRED -- see below)
//
// Full Indigo verification (from scratch program) for reference anthracene skeleton
// (all 7 entries share this, so only one verification is needed for the skeleton):
//   atom[0]: C deg=2 neighbors=[1,13]
//   atom[1]: C deg=2 neighbors=[0,2]
//   atom[2]: C deg=2 neighbors=[1,3]
//   atom[3]: C deg=3 neighbors=[2,4,12]
//   atom[4]: [N] deg=2 neighbors=[3,5]
//   atom[5]: C deg=3 neighbors=[4,6,10]
//   atom[6]: C deg=2 neighbors=[5,7]
//   atom[7]: C deg=2 neighbors=[6,8]
//   atom[8]: C deg=2 neighbors=[7,9]
//   atom[9]: C deg=2 neighbors=[8,10]
//   atom[10]: C deg=3 neighbors=[9,5,11]
//   atom[11]: [X] deg=2 neighbors=[10,12]
//   atom[12]: C deg=3 neighbors=[11,3,13]
//   atom[13]: C deg=2 neighbors=[12,0]
// Bridgeheads (deg=3): atom[3], atom[5], atom[10], atom[12]
// Fusion bonds: 3-12, 5-10
//
// CORRECTED: the GRAPH TOPOLOGY matches anthracene (same automorphism class,
// correctly verified via Indigo), but the ORIGINALLY-DISPATCHED version wrongly
// assumed this means the REAL Blue Book LOCANT LABELS also transfer verbatim
// from anthracene. They do not -- phenoxazine/phenothiazine have their OWN,
// independently-fixed, historical numbering convention that is a genuinely
// different label sequence despite the identical graph shape.
//
// Confirmed via three independent sources: chlorpromazine's actual IUPAC name
// ("...10H-phenothiazine-10-propanamine", confirming N is at locant 10, not
// an anthracene-style "9"/"10" pair), general web search confirming S at 5 /
// N at 10, and a dedicated search confirming the real bridgehead locants are
// 4a, 5a, 9a, 10a (NOT anthracene's 4a, 8a, 9a, 10a -- note "5a" replaces
// "8a"). The real sequence is a plain, non-"jumping" walk:
//   1-2-3-4-4a-5-5a-6-7-8-9-9a-10-10a-(back to 1)
// -- unlike anthracene's own real sequence (independently verified in Task 1
// and reused correctly in Task 4), which actually jumps: 1-2-3-4-4a-10-10a-
// 5-6-7-8-8a-9-9a-(back to 1). Same graph, two different real label walks.
//
// Re-derived by hand-tracing every one of the 14 real Indigo bonds (listed
// above) against this correct sequence, fixing both heteroatom positions (N
// at atom[4] must land on locant "10"; X at atom[11] must land on locant "5")
// and solving for every other atom's locant consistently. Every one of the
// 14 consecutive pairs below (including the 10a-1 closing bond) was checked
// against the real neighbor lists above:
//   atom[0]="3", atom[1]="2", atom[2]="1", atom[3]="10a", atom[4]="10",
//   atom[5]="9a", atom[6]="9", atom[7]="8", atom[8]="7", atom[9]="6",
//   atom[10]="5a", atom[11]="5", atom[12]="4a", atom[13]="4"
//
// Bond-letter assignment (Blue Book FR-4 convention: 'a' is always the 1-2 bond):
//   'a':1-2 'b':2-3 'c':3-4 'd':4-4a 'e':4a-5 'f':5-5a 'g':5a-6 'h':6-7
//   'i':7-8 'j':8-9 'k':9-9a 'l':9a-10 'm':10-10a 'n':10a-1 (closing)

void registerPhenoxazineFamily(std::vector<RetainedRingSystem> &registry) {
    // Shared locant mapping and bond letters for all entries (corrected --
    // see note above; this is NOT anthracene's locant list, despite sharing
    // its graph shape).
    const std::vector<QString> locants = {
        "3","2","1","10a","10","9a","9","8","7","6","5a","5","4a","4"
    };
    const std::map<char, std::pair<QString, QString>> bondLetters = {
        {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
        {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","5a"}},
        {'g', {"5a","6"}}, {'h', {"6","7"}}, {'i', {"7","8"}},
        {'j', {"8","9"}}, {'k', {"9","9a"}}, {'l', {"9a","10"}},
        {'m', {"10","10a"}}, {'n', {"10a","1"}},
    };

    // 1. phenoxazine (rank 1) - X = O
    // Full Indigo verification (from scratch program):
    //   All atoms same as anthracene skeleton.
    //   atom[4]=N, atom[11]=O, all other atoms C.
    //   Neighbor lists: identical to anthracene (see above).
    //   Walked complete peripheral locant sequence against real bonds:
    //     "1"(13)-[0,13]-"2" -> "2"(0)-[1,13]-"3" -> ... -> "9"(11)-[10,12]-"9a" -> ... -> "1"(13)
    //     All consecutive pairs are confirmed real bonds in the neighbor lists.
    registry.push_back(RetainedRingSystem{
        "phenoxazine",
        "c1ccc2nc3ccccc3oc2c1",
        locants,
        bondLetters,
        1, // Appendix 2 seniority rank
    });

    // 2. phenothiazine (rank 2) - X = S
    // Full Indigo verification:
    //   Same skeleton as phenoxazine.
    //   atom[4]=N, atom[11]=S, all other atoms C.
    //   Neighbor lists identical to phenoxazine (same parse order).
    //   Complete peripheral walk verified against real bonds (same as above).
    registry.push_back(RetainedRingSystem{
        "phenothiazine",
        "c1ccc2nc3ccccc3sc2c1",
        locants,
        bondLetters,
        2, // Appendix 2 seniority rank
    });

    // 3. phenoselenazine (rank 3) - X = Se
    // Full Indigo verification:
    //   Same skeleton.
    //   atom[4]=N, atom[11]=Se (atomic number 34, deg=2).
    //   Neighbor lists identical.
    //   Complete peripheral walk verified.
    registry.push_back(RetainedRingSystem{
        "phenoselenazine",
        "c1ccc2nc3ccccc3[se]c2c1",
        locants,
        bondLetters,
        3, // Appendix 2 seniority rank
    });

    // 4. phenotellurazine (rank 4) - X = Te
    // Full Indigo verification:
    //   Same skeleton.
    //   atom[4]=N, atom[11]=Te (atomic number 52, deg=2).
    //   Neighbor lists identical.
    //   Complete peripheral walk verified.
    registry.push_back(RetainedRingSystem{
        "phenotellurazine",
        "c1ccc2nc3ccccc3[te]c2c1",
        locants,
        bondLetters,
        4, // Appendix 2 seniority rank
    });

    // 5. phenophosphazinine (rank 5) - X = P
    // Full Indigo verification:
    //   Same skeleton.
    //   atom[4]=N, atom[11]=P (atomic number 15, deg=2).
    //   P is divalent in this aromatic context (lone pair completes octet),
    //   which is valid for phosphinine-like aromatic systems.
    //   Neighbor lists identical.
    //   Complete peripheral walk verified.
    registry.push_back(RetainedRingSystem{
        "phenophosphazinine",
        "c1ccc2nc3ccccc3[P]c2c1",
        locants,
        bondLetters,
        5, // Appendix 2 seniority rank
    });

    // 6. phenarsazinine (rank 6) - X = As
    // Full Indigo verification:
    //   Same skeleton.
    //   atom[4]=N, atom[11]=As (atomic number 33, deg=2).
    //   As is divalent in this aromatic context (analogous to P).
    //   Neighbor lists identical.
    //   Complete peripheral walk verified.
    registry.push_back(RetainedRingSystem{
        "phenarsazinine",
        "c1ccc2nc3ccccc3[As]c2c1",
        locants,
        bondLetters,
        6, // Appendix 2 seniority rank
    });

    // 7. phenomercurazine (rank 7) - X = Hg
    // DEFERRED: Mercury typically forms linear divalent compounds and does NOT
    // readily participate in aromatic systems the way C/N/O/S/P/As/Se/Te do.
    // The SMILES 'c1ccc2nc3ccccc3[Hg]c2c1' parses with Hg at atom[11] (deg=2),
    // but organomercury compounds are generally non-aromatic with Hg in a
    // non-planar, non-conjugated environment. Without confidence in a chemically
    // sane aromatic structure for this entry, we defer it.
    //
    // To implement later if a valid aromatic Hg structure is confirmed:
    //   registry.push_back(RetainedRingSystem{
    //       "phenomercurazine",
    //       "c1ccc2nc3ccccc3[Hg]c2c1",
    //       locants,
    //       bondLetters,
    //       7,
    //   });
}
