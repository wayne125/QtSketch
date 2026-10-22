#ifndef FUSIONGRAPHCONTEXT_H
#define FUSIONGRAPHCONTEXT_H

#include "GraphTypes.h"
#include <vector>
#include <set>

// Wraps ONLY the ~51-line setup block duplicated verbatim at the top of
// nameLinearChainFusion and nameBranchingFusion (choose a caller-
// supplied ring subset, or compute SSSR rings via Indigo, then check
// pairwise disjointness). Does NOT cover generateName's own separate
// Indigo access -- explicitly out of scope, not isolated the same way.
//
// Known gap vs. the spec's "callable without Indigo" goal for this area:
// nameLinearChainFusion and nameBranchingFusion still take the Indigo
// `mol` handle as a separate parameter and pass it on to other Indigo-
// dependent calls below this setup block (e.g. detectFusedRingDirections,
// nameBranchGraph) -- this class only isolates the one duplicated setup
// block, not the functions' full Indigo dependency. Left as-is; tracked
// as a deliberate scope exclusion for this plan, not an oversight.
class FusionGraphContext {
public:
    FusionGraphContext(const Graph &g, int mol,
                       const std::vector<std::set<int>> *ringSubsetOverride);
    const Graph &graph() const;
    const std::vector<std::set<int>> &sssrRings() const;
    bool allDisjoint() const;
private:
    const Graph &m_graph;
    std::vector<std::set<int>> m_sssrRings;
    bool m_allDisjoint;
};

#endif // FUSIONGRAPHCONTEXT_H
