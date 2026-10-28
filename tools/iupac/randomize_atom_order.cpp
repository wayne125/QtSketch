// tools/iupac/randomize_atom_order.cpp
// CLI tool: takes a SMILES and a seed, produces an equivalent SMILES
// with atoms in a different but deterministic (seeded) order. The real
// permutation + equivalence self-check lives in AtomOrderRandomizer,
// shared with iupac_golden_test's determinism check.

#include "AtomOrderRandomizer.h"
#include "indigo.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: randomize_atom_order <SMILES> <seed>" << std::endl;
        return 1;
    }

    std::string smiles = argv[1];
    int seed = std::atoi(argv[2]);

    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);

    std::string result;
    bool ok = randomizeAtomOrder(smiles, seed, result);

    indigoReleaseSessionId(sid);

    if (!ok) {
        std::cerr << "Error: could not produce a verified reordering for this "
                     "molecule (stereo-sensitive, or the equivalence self-check failed)"
                  << std::endl;
        return 1;
    }

    std::cout << result << std::endl;
    return 0;
}
