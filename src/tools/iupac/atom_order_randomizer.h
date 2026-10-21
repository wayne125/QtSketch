#pragma once
#include <string>

/**
 * Randomizes the atom order of a SMILES string using a seeded permutation.
 * Returns a new SMILES string with atoms reordered, or empty string on error.
 *
 * Uses Indigo to load the SMILES, permute atom indices, and save the new SMILES.
 * Preserves connectivity and atom properties (symbol, charge, isotope, etc.).
 *
 * @param smiles Input SMILES string
 * @param seed Integer seed for the random permutation
 * @return Reordered SMILES string, or empty string on error
 */
std::string randomizeAtomOrder(const std::string& smiles, int seed);