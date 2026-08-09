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
    
    for (int i = 0; i < numRings; ++i) {
        if (ringAdj[i].size() > 2) return input; // Branching, out of scope
    }
    
    // 4. Find endpoints of the linear chain
    int startRing = -1;
    for (int i = 0; i < numRings; ++i) {
        if (ringAdj[i].size() <= 1) {
            startRing = i;
            break;
        }
    }
    if (startRing == -1) return input; // Cycle of rings? Out of scope
    
    // 5. Traverse the chain
    std::vector<int> chain;
    int curr = startRing;
    int prev = -1;
    while (curr != -1) {
        chain.push_back(curr);
        int next = -1;
        for (int nxt : ringAdj[curr]) {
            if (nxt != prev) {
                next = nxt;
                break;
            }
        }
        prev = curr;
        curr = next;
    }
    if (chain.size() != static_cast<size_t>(numRings)) return input; // Disconnected
    
    input.ringSizes.assign(numRings, 6);
    if (numRings == 1) return input; // Trivial 1-ring system
    
    // 6. Assign directions
    // Pick arbitrary direction for first fusion
    std::pair<int, int> b0 = sharedAtoms[{chain[0], chain[1]}];
    int u = b0.first;
    int v = b0.second;
    int currentDir = 0;
    
    // FusedRingEdge uses node IDs which correspond to the sequence in `chain`
    // Wait, the Phase 42 expected indices 0,1,2... So we map chain[i] -> i in the output,
    // because `input.ringSizes` has size N and we assume indices 0..N-1.
    input.fusions.push_back({0, 1, currentDir});
    
    for (int i = 1; i < numRings - 1; ++i) {
        int r = chain[i];
        int next_r = chain[i+1];
        
        // Build cycle for r starting with u -> v
        std::vector<int> cycle = buildDirectedCycle(rings[r], u, v, adj);
        
        std::pair<int, int> b_out = sharedAtoms[{r, next_r}];
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
            input.ringSizes.clear(); // Error state
            return input;
        }
        
        currentDir = (currentDir + 3 + k) % 6;
        input.fusions.push_back({i, i+1, currentDir});
        
        // Next ring's incoming bond should be traversed in reverse (out_v -> out_u)
        u = out_v;
        v = out_u;
    }
    
    return input;
}
