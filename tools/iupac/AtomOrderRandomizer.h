// tools/iupac/AtomOrderRandomizer.h
// Shared between the standalone randomize_atom_order CLI tool and
// iupac_golden_test's determinism check.
#pragma once

#include <string>

// Loads `smiles`, builds a seeded permutation of its atom order, and
// returns (via outSmiles) an equivalent SMILES whose emitted atom order
// differs from a straight re-parse of the input. Returns false (leaving
// outSmiles untouched) if the molecule is stereo-sensitive (this
// reordering does not re-derive stereo descriptors for the new atom
// order) or if the self-check fails: the reordered molecule must be the
// same molecule (same Indigo canonical SMILES) as the original, and for
// molecules with more than 2 atoms its raw SMILES must actually differ
// from a re-parse of the original -- so a future regression back to a
// no-op shows up as a visible skip/failure, not a silent pass.
bool randomizeAtomOrder(const std::string& smiles, int seed, std::string& outSmiles);
