// tools/iupac/randomize_atom_order.cpp
// Tool to randomize atom order in a SMILES string using Indigo.
// Takes a SMILES and a seed, produces an equivalent SMILES with atoms
// in a different but deterministic order.
//
// For now, this is a simplified implementation that just returns the
// original SMILES. A full implementation would use molfile parsing to
// reorder atoms, but the current Indigo API doesn't provide simple
// buffer read/write functions.
//
// The determinism check in iupac_golden_test.cpp runs generateName
// multiple times on the same input to verify there's no internal
// nondeterminism.

#include "../../src/tools/iupac/atom_order_randomizer.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: randomize_atom_order <SMILES> <seed>" << std::endl;
        return 1;
    }
    
    std::string smiles = argv[1];
    int seed = std::atoi(argv[2]);
    
    std::string result = randomizeAtomOrder(smiles, seed);
    
    if (result.empty()) {
        std::cerr << "Error: Could not process SMILES" << std::endl;
        return 1;
    }
    
    std::cout << result << std::endl;
    return 0;
}
