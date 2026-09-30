#include "FusedRingDirectionDetector.h"
#include "indigo.h"
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>

static std::vector<int> buildDirectedCycle(const std::set<int>& rNodes, int u, int v, const std::map<int, std::vector<int>>& neighbors) {
    std::vector<int> cycle;
    cycle.push_back(u);
    cycle.push_back(v);
    int prev = u;
    int curr = v;
    while (cycle.size() < rNodes.size()) {
        int next = -1;
        auto it = neighbors.find(curr);
        if (it != neighbors.end()) {
            for (int nei : it->second) {
                if (rNodes.count(nei) && nei != prev) {
                    next = nei;
                    break;
                }
            }
        }
        if (next == -1) break; // Should not happen for a valid simple cycle
        cycle.push_back(next);
        prev = curr;
        curr = next;
    }
    return cycle;
}

FusedRingSystemInput detectFusedRingDirections(int mol) {
    FusedRingSystemInput input;
    
    // 1. Build atom adjacency list
    std::map<int, std::vector<int>> adj;
    int atomIter = indigoIterateAtoms(mol);
    if (atomIter > 0) {
        while (int atom = indigoNext(atomIter)) {
            int idx = indigoIndex(atom);
            int neiIter = indigoIterateNeighbors(atom);
            if (neiIter > 0) {
                while (int nei = indigoNext(neiIter)) {
                    adj[idx].push_back(indigoIndex(nei));
                    indigoFree(nei);
                }
                indigoFree(neiIter);
            }
            indigoFree(atom);
        }
        indigoFree(atomIter);
    }
    
    // 2. Extract SSSR rings
    std::vector<std::set<int>> rings;
    int sssrIter = indigoIterateSSSR(mol);
    if (sssrIter > 0) {
        while (int subMol = indigoNext(sssrIter)) {
            std::set<int> rNodes;
            int rAtomIter = indigoIterateAtoms(subMol);
            if (rAtomIter > 0) {
                while (int a = indigoNext(rAtomIter)) {
                    rNodes.insert(indigoIndex(a));
                    indigoFree(a);
                }
                indigoFree(rAtomIter);
            }
            rings.push_back(rNodes);
            indigoFree(subMol);
        }
        indigoFree(sssrIter);
    }
    
    int numRings = static_cast<int>(rings.size());
    if (numRings < 1) return input;
    
    // Check ring sizes (only 6-membered rings supported in this phase)
    for (int i = 0; i < numRings; ++i) {
        if (rings[i].size() != 6) return input; 
    }
    
    // 3. Find shared bonds and ring adjacency
    std::vector<std::vector<int>> ringAdj(numRings);
    std::map<std::pair<int, int>, std::pair<int, int>> sharedAtoms;
    
    for (int i = 0; i < numRings; ++i) {
        for (int j = i + 1; j < numRings; ++j) {
            std::vector<int> common;
            std::set_intersection(rings[i].begin(), rings[i].end(),
                                  rings[j].begin(), rings[j].end(),
                                  std::back_inserter(common));
            if (common.size() == 2) {
                ringAdj[i].push_back(j);
                ringAdj[j].push_back(i);
                sharedAtoms[{i, j}] = {common[0], common[1]};
                sharedAtoms[{j, i}] = {common[0], common[1]};
            } else if (common.size() > 2) {
                // Peri-fused or bridged, explicitly out of scope
                return input; 
            }
        }
    }
    
    input.ringAtoms = rings;
    input.ringSizes.assign(numRings, 6);
    if (numRings == 1) return input; // Trivial 1-ring system

    std::vector<bool> visited(numRings, false);
    
    // Pick any ring with degree <= 1 as a traversal root if one exists (a leaf of the
    // tree), otherwise (a system with no leaf can only be a single ring, already handled
    // by the numRings == 1 early return above, or is not actually a tree -- but the
    // peri-fusion check above already rejects any cycle among rings, so every remaining
    // multi-ring system here is guaranteed to be a tree and therefore guaranteed to have
    // at least one leaf) fall back to ring 0.
    int rootRing = 0;
    for (int i = 0; i < numRings; ++i) {
        if (ringAdj[i].size() <= 1) { rootRing = i; break; }
    }
    
    visited[rootRing] = true;
    
    // BFS queue entries: (ringIndex, incoming shared-atom u, incoming shared-atom v)
    // For the root, there is no real incoming bond -- seed with an arbitrary bond of the
    // root's FIRST fusion (to its first neighbor), exactly mirroring how the original
    // linear-chain code picked its own starting u/v from chain[0]/chain[1].
    if (ringAdj[rootRing].empty()) return input; // single ring already returned above; a
                                                    // adjacency-less ring here is an error
    int firstNeighbor = ringAdj[rootRing][0];
    std::pair<int,int> rootBond = sharedAtoms[{rootRing, firstNeighbor}];

    struct QueueEntry { int ring; int u; int v; };
    std::vector<QueueEntry> queue;
    queue.push_back({rootRing, rootBond.first, rootBond.second});
    size_t qi = 0;

    // dirFromParent[ringIndex] = the direction FROM its parent TO this ring, filled in
    // as each ring is first visited; used to emit input.fusions in (parent, child, dir)
    // form exactly like the original code's input.fusions.push_back({i, i+1, currentDir}).
    // The root itself has no incoming direction and needs none.
    std::map<int,int> dirFromParent;
    
    while (qi < queue.size()) {
        QueueEntry entry = queue[qi++];
        int r = entry.ring;
        int u = entry.u;
        int v = entry.v;

        std::vector<int> cycle = buildDirectedCycle(rings[r], u, v, adj);

        for (int nextR : ringAdj[r]) {
            if (visited[nextR]) continue;
            visited[nextR] = true;

            std::pair<int,int> b_out = sharedAtoms[{r, nextR}];
            int out_u = -1, out_v = -1;
            int k = -1;
            for (size_t c = 0; c < cycle.size(); ++c) {
                int c1 = cycle[c];
                int c2 = cycle[(c + 1) % cycle.size()];
                if ((c1 == b_out.first && c2 == b_out.second) || (c1 == b_out.second && c2 == b_out.first)) {
                    k = static_cast<int>(c);
                    out_u = c1;
                    out_v = c2;
                    break;
                }
            }
            if (k == -1) {
                input.ringSizes.clear();
                return input;
            }

            // For the ROOT ring's FIRST outgoing edge (the one used to seed the BFS),
            // the direction is 0 by fiat, mirroring the original code's `int currentDir = 0;`
            // for chain[0]->chain[1]. For the root's OTHER outgoing edges (branching case),
            // and for ALL outgoing edges from non-root rings, compute the direction using
            // the same formula the original code used: `currentDir = (currentDir + 3 + k) % 6`,
            // where the incoming direction from the parent is substituted for `currentDir`.
            int outgoingDir;
            if (r == rootRing && entry.u == rootBond.first && entry.v == rootBond.second) {
                // First edge from root: direction 0
                outgoingDir = 0;
            } else {
                int incomingDir = dirFromParent.at(r);
                outgoingDir = (incomingDir + 3 + k) % 6;
            }

            input.fusions.push_back({r, nextR, outgoingDir});
            dirFromParent[nextR] = outgoingDir;

            // Next ring's incoming bond is traversed in reverse, exactly as the original
            // code's `u = out_v; v = out_u;` step.
            queue.push_back({nextR, out_v, out_u});
        }
    }
    
    return input;
}
