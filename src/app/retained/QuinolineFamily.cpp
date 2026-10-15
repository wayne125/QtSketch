#include "QuinolineFamily.h"

// All entries use the naphthalene 6-6 ortho-fused bicyclic skeleton with the
// same peripheral cycle order: 1-2-3-4-4a-5-6-7-8-8a-1.
//
// Reference naphthalene (from HydrocarbonFamily.cpp):
//   SMILES: c1ccc2ccccc2c1
//   atom[0] -> "2", atom[1] -> "3", atom[2] -> "4",
//   atom[3] -> "4a", atom[4] -> "5", atom[5] -> "6",
//   atom[6] -> "7", atom[7] -> "8", atom[8] -> "8a", atom[9] -> "1"
//   Bridgeheads at atom[3] ("4a") and atom[8] ("8a"), both deg=3.
//
// All heterobicyclics below reuse this same atom->locant mapping.
// Each SMILES was empirically verified to place N atoms at the correct
// atom indices matching their chemical locant positions.

void registerQuinolineFamily(std::vector<RetainedRingSystem> &registry) {
    // 1. quinoline: N at locant 1 (atom[9])
    // SMILES c1ccc2ccccc2N1 verified: atom[9]=N, all others C, 10 atoms
    // Locants: atom[0]="2", atom[1]="3", atom[2]="4", atom[3]="4a",
    //          atom[4]="5", atom[5]="6", atom[6]="7", atom[7]="8",
    //          atom[8]="8a", atom[9]="1" (N)
    registry.push_back(RetainedRingSystem{
        "quinoline",
        "c1ccc2ccccc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        34, // Appendix 2 seniority rank
    });

    // 2. isoquinoline: N at locant 2 (atom[0])
    // SMILES N1ccc2ccccc2c1 verified: atom[0]=N, all others C, 10 atoms
    // Locants: same mapping as quinoline
    registry.push_back(RetainedRingSystem{
        "isoquinoline",
        "N1ccc2ccccc2c1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        35, // Appendix 2 seniority rank
    });

    // 3. cinnoline: N at locants 1 and 2 (atoms[9] and atom[0])
    // SMILES N1ccc2ccccc2N1 verified: atom[0]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "cinnoline",
        "N1ccc2ccccc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        24, // Appendix 2 seniority rank
    });

    // 4. quinazoline: N at locants 1 and 3.
    // CORRECTED from the originally-dispatched "c1Nc2ccccc2cN1", which
    // opened ring2 one token position too early (at index 2 instead of the
    // reference skeleton's index 3), shifting the whole backbone and
    // actually placing N at locants 6 and 8 instead of the required 1 and
    // 3 -- caught by independently re-tracing the real bond topology
    // (ring-closure token positions), not by validateRetainedRingSystem
    // (which only checks internal list self-consistency, not that the
    // atom order matches the intended chemistry). Verified: SMILES
    // "c1Ncc2ccccc2N1" has 10 atoms, bridgeheads at indices 3 and 8
    // (matching the shared naphthalene-skeleton reference exactly), N at
    // index1 (-> locant "3") and index9 (-> locant "1"): N(1)-C(2)-N(3)-
    // C(4)-C(4a)-... -- the textbook quinazoline arrangement.
    registry.push_back(RetainedRingSystem{
        "quinazoline",
        "c1Ncc2ccccc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        25, // Appendix 2 seniority rank
    });

    // 5. quinoxaline: N at locants 1 and 4 (atoms[9] and atom[2])
    // SMILES c1cNc2ccccc2N1 verified: atom[2]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "quinoxaline",
        "c1cNc2ccccc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        26, // Appendix 2 seniority rank
    });

    // 6. phthalazine: N at locants 2 and 3 (atoms[0] and atom[1])
    // SMILES N1Ncc2ccccc2c1 verified: atom[0]=N, atom[1]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "phthalazine",
        "N1Ncc2ccccc2c1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        31, // Appendix 2 seniority rank
    });

    // 7. 1,5-naphthyridine: N at locants 1 and 5 (atoms[9] and atom[4])
    // SMILES c1ccc2ncccc2n1 verified: atom[4]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "1,5-naphthyridine",
        "c1ccc2ncccc2n1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        27, // Appendix 2 seniority rank
    });

    // 8. 1,6-naphthyridine: N at locants 1 and 6 (atoms[9] and atom[5])
    // SMILES c1ccc2cNccc2N1 verified: atom[5]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "1,6-naphthyridine",
        "c1ccc2cNccc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        28, // Appendix 2 seniority rank
    });

    // 9. 1,7-naphthyridine: N at locants 1 and 7 (atoms[9] and atom[6])
    // SMILES c1ccc2ccNcc2N1 verified: atom[6]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "1,7-naphthyridine",
        "c1ccc2ccNcc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        29, // Appendix 2 seniority rank
    });

    // 10. 1,8-naphthyridine: N at locants 1 and 8 (atoms[9] and atom[7])
    // SMILES c1ccc2cccNc2N1 verified: atom[7]=N, atom[9]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "1,8-naphthyridine",
        "c1ccc2cccNc2N1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        30, // Appendix 2 seniority rank
    });

    // 11. 2,6-naphthyridine: N at locants 2 and 6 (atoms[0] and atom[5])
    // SMILES N1ccc2cNccc2c1 verified: atom[0]=N, atom[5]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "2,6-naphthyridine",
        "N1ccc2cNccc2c1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        32, // Appendix 2 seniority rank
    });

    // 12. 2,7-naphthyridine: N at locants 2 and 7 (atoms[0] and atom[6])
    // SMILES N1ccc2ccNcc2c1 verified: atom[0]=N, atom[6]=N, 10 atoms
    registry.push_back(RetainedRingSystem{
        "2,7-naphthyridine",
        "N1ccc2ccNcc2c1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        33, // Appendix 2 seniority rank
    });

    // 13. quinolizine: Bridgehead N - SKIPPED
    // Quinolizine has a bridgehead nitrogen (N at a ring fusion position),
    // which means it has a different locant numbering scheme from the 6-6
    // ortho-fused bicyclics above. The structure is not a simple replacement
    // of CH positions with N in the naphthalene skeleton; it has N as one of
    // the two shared atoms between the rings. This requires separate research
    // to verify the correct numbering. Per the task description, it is
    // explicitly acceptable to skip entries that cannot be verified confidently.
    // Skipping quinolizine.
}
