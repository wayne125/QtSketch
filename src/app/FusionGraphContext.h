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
