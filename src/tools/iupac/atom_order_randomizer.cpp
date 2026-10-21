#include "atom_order_randomizer.h"
#include <indigo.h>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

std::string randomizeAtomOrder(const std::string& smiles, int seed) {
    // Load the input SMILES
    int mol = indigoLoadMoleculeFromString(smiles.c_str());
    if (mol <= 0) {
        return "";
    }
    indigoSetSessionId(0);
    
    // Debug: print atom count
    int atom_count = indigoCountAtoms(mol);

    // Collect all atoms
    std::vector<int> atoms;
    int atomIter = indigoIterateAtoms(mol);
    if (atomIter < 0) {
        indigoFree(mol);
        return "";
    }
    
    // Debug: print atom count
    // std::cerr << "Atom count: " << atom_count << std::endl;
    int atomHandle = 0;
    while ((atomHandle = indigoNext(atomIter)) != 0) {
        atoms.push_back(atomHandle);
    }
    indigoFree(atomIter);

    if (atoms.empty()) {
        indigoFree(mol);
        return "";
    }

    // Generate a seeded permutation of atom indices
    std::vector<size_t> indices(atoms.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }
    std::mt19937 rng(seed);
    std::shuffle(indices.begin(), indices.end(), rng);

    // Create a new molecule with permuted atom order
    int newMol = indigoCreateMolecule();
    if (newMol <= 0) {
        indigoFree(mol);
        return "";
    }

    // Add atoms in permuted order
    std::vector<int> newAtoms(atoms.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        int oldAtom = atoms[indices[i]];
        const char* symbol = indigoSymbol(oldAtom);
        int charge = 0;
        indigoGetCharge(oldAtom, &charge);
        int isotope = indigoIsotope(oldAtom);
        int radical = 0;
        indigoGetRadicalElectrons(oldAtom, &radical);
        int newAtom = indigoAddAtom(newMol, symbol);
        if (newAtom <= 0) {
            indigoFree(mol);
            indigoFree(newMol);
            return "";
        }
        if (charge != 0) {
            indigoSetCharge(newAtom, charge);
        }
        if (isotope != 0) {
            indigoSetIsotope(newAtom, isotope);
        }
        if (radical != 0) {
            indigoSetRadical(newAtom, radical);
        }
        newAtoms[i] = newAtom;
    }

    // Add bonds, translating through the permutation
    int bondIter = indigoIterateBonds(mol);
    if (bondIter < 0) {
        indigoFree(mol);
        indigoFree(newMol);
        return "";
    }
    int bondHandle = 0;
    while ((bondHandle = indigoNext(bondIter)) != 0) {
        int src = indigoSource(bondHandle);
        int dst = indigoDestination(bondHandle);
        int srcIdx = indigoIndex(src) - 1; // Indigo is 1-based
        int dstIdx = indigoIndex(dst) - 1;
        int bondOrder = indigoBondOrder(bondHandle);
        int newSrc = newAtoms[indices[srcIdx]];
        int newDst = newAtoms[indices[dstIdx]];
        int newBond = indigoAddBond(newSrc, newDst, bondOrder);
        if (newBond <= 0) {
            indigoFree(mol);
            indigoFree(newMol);
            return "";
        }
        // Copy bond stereo if present
        int stereo = indigoBondStereo(bondHandle);
        if (stereo != INDIGO_ABS) {
            // indigoSetBondStereo not available in this Indigo version; skip stereo for now
        }
    }
    indigoFree(bondIter);

    // Save the new molecule as SMILES
    const char* newSmiles = indigoSmiles(newMol);
    std::string result = (newSmiles != nullptr) ? newSmiles : "";
    if (result.empty()) {
        indigoFree(newMol);
        indigoFree(mol);
        return "";
    }

    indigoFree(mol);
    indigoFree(newMol);
    return result;
}