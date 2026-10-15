#include "HydrocarbonFamily.h"

void registerHydrocarbonFamily(std::vector<RetainedRingSystem> &registry) {
    // Naphthalene -- THE motivating entry (naphtho[2,3-g]isoquinoline).
    // SMILES 'c1ccc2ccccc2c1' parsed by Indigo produces atoms in order:
    // atom 0: C beta (neighbors 1, 9) -> locant "2"
    // atom 1: C beta (neighbors 0, 2) -> locant "3"
    // atom 2: C alpha (neighbors 1, 3) -> locant "4"
    // atom 3: C bridgehead (neighbors 2, 4, 8) -> locant "4a"
    // atom 4: C alpha (neighbors 3, 5) -> locant "5"
    // atom 5: C beta (neighbors 4, 6) -> locant "6"
    // atom 6: C beta (neighbors 5, 7) -> locant "7"
    // atom 7: C alpha (neighbors 6, 8) -> locant "8"
    // atom 8: C bridgehead (neighbors 3, 7, 9) -> locant "8a"
    // atom 9: C alpha (neighbors 0, 8) -> locant "1"
    registry.push_back(RetainedRingSystem{
        "naphthalene",
        "c1ccc2ccccc2c1",
        {"2","3","4","4a","5","6","7","8","8a","1"},
        {
            {'a', {"1","2"}}, {'b', {"2","3"}}, {'c', {"3","4"}},
            {'d', {"4","4a"}}, {'e', {"4a","5"}}, {'f', {"5","6"}},
            {'g', {"6","7"}}, {'h', {"7","8"}}, {'i', {"8","8a"}},
            {'j', {"8a","1"}},
        },
        56, // Appendix 1 seniority position
    });

    // Anthracene -- 14 carbons; 9 and 10 are the two unique "meso" positions.
    // SMILES 'c1ccc2cc3ccccc3cc2c1' parsed by Indigo produces atoms in order:
    // atom 0: C beta (neighbors 1, 13) -> locant "2"
    // atom 1: C beta (neighbors 0, 2) -> locant "3"
    // atom 2: C alpha (neighbors 1, 3) -> locant "4"
    // atom 3: C bridgehead (neighbors 2, 4, 12) -> locant "4a"
    // atom 4: C meso (neighbors 3, 5) -> locant "10"
    // atom 5: C bridgehead (neighbors 4, 6, 10) -> locant "10a"
    // atom 6: C alpha (neighbors 5, 7) -> locant "5"
    // atom 7: C beta (neighbors 6, 8) -> locant "6"
    // atom 8: C beta (neighbors 7, 9) -> locant "7"
    // atom 9: C alpha (neighbors 8, 10) -> locant "8"
    // atom 10: C bridgehead (neighbors 5, 9, 11) -> locant "8a"
    // atom 11: C meso (neighbors 10, 12) -> locant "9"
    // atom 12: C bridgehead (neighbors 3, 11, 13) -> locant "9a"
    // atom 13: C alpha (neighbors 0, 12) -> locant "1"
    registry.push_back(RetainedRingSystem{
        "anthracene",
        "c1ccc2cc3ccccc3cc2c1",
        {"2","3","4","4a","10","10a","5","6","7","8","8a","9","9a","1"},
        {}, // bond letters added in a later task once an attached-component
            // use case for "anthra-" actually needs them -- empty map is
            // valid (base-component-only use still works without it)
        46,
    });
}
