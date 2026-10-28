#pragma once
#include "../RetainedRingTypes.h"
#include <vector>

// Appendix 2 heterocyclic retained names -- Chromene/Xanthene families
// Bicyclic (6-6, naphthalene-shaped): chromene, isochromene, thiochromene, isothiochromene,
//   selenochromene, isoselenochromene, tellurochromene, isotellurochromene
// Tricyclic (6-6-6, anthracene-shaped): xanthene, oxanthrene, thioxanthene, thianthrene,
//   selenoxanthene, telluroxanthene, phenoxathiine, phenoxaselenine, phenoxatellurine,
//   phenoxaphosphinine, phenoxarsinine, phenoxastibinine
void registerChromeneXanthineFamily(std::vector<RetainedRingSystem> &registry);
