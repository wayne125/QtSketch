#include "NamingValidation.h"

// Returns true if two nodes are directly connected by a bond in the graph
static bool areConnected(const Graph &g, int u, int v) {
    if (u < 0 || v < 0 || u >= static_cast<int>(g.nodes.size()) || v >= static_cast<int>(g.nodes.size())) {
        return false;
    }
    const auto &neighbors = g.nodes[u].neighbors;
    for (int n : neighbors) {
        if (n == v) {
            return true;
        }
    }
    return false;
}

bool validateRingCycle(const Graph &g, const std::vector<int> &cycle) {
    if (cycle.empty()) {
        return false;
    }
    
    // Check every element is distinct
    std::set<int> seen;
    for (int node : cycle) {
        if (seen.count(node)) {
            return false;  // Duplicate node in cycle
        }
        seen.insert(node);
    }
    
    // Check every consecutive pair (including last-to-first wraparound) is a real bond
    size_t n = cycle.size();
    for (size_t i = 0; i < n; ++i) {
        int a = cycle[i];
        int b = cycle[(i + 1) % n];
        if (!areConnected(g, a, b)) {
            return false;  // No bond between consecutive nodes
        }
    }
    
    return true;
}

bool validateFusionInput(const Graph &g, const FusedRingSystemInput &input) {
    // Check that ringSizes is consistent
    if (input.ringSizes.empty()) {
        return input.fusions.empty();  // Empty system is valid
    }
    
    // Check that ringAtoms has the right size if it's non-empty
    if (!input.ringAtoms.empty() && input.ringAtoms.size() != input.ringSizes.size()) {
        return false;
    }
    
    // Check that ringDirectedCycles has the right size if it's non-empty
    if (!input.ringDirectedCycles.empty() && input.ringDirectedCycles.size() != input.ringSizes.size()) {
        return false;
    }
    
    for (const auto &edge : input.fusions) {
        int ring1 = edge.ring1;
        int ring2 = edge.ring2;
        
        // Check ring indices are valid
        if (ring1 < 0 || ring1 >= static_cast<int>(input.ringSizes.size()) ||
            ring2 < 0 || ring2 >= static_cast<int>(input.ringSizes.size())) {
            return false;
        }
        
        // If we have ringDirectedCycles, use it to validate bondIndex
        if (!input.ringDirectedCycles.empty() && ring1 < static_cast<int>(input.ringDirectedCycles.size()) &&
            !input.ringDirectedCycles[ring1].empty()) {
            
            const std::vector<int> &ring1Cycle = input.ringDirectedCycles[ring1];
            int ring1Size = static_cast<int>(ring1Cycle.size());
            
            // Check bondIndex is within valid range for ring1's cycle
            if (edge.bondIndex.value < 0 || edge.bondIndex.value >= ring1Size) {
                return false;  // bondIndex out of range for the cycle
            }
            
            // Find the two bridgehead atoms at bondIndex position in ring1's cycle
            int idx = edge.bondIndex.value;
            int bhA = ring1Cycle[idx];
            int bhB = ring1Cycle[(idx + 1) % ring1Size];
            
            // Verify that both bridgehead atoms are members of both rings' atom sets
            if (input.ringAtoms.empty()) {
                // Can't validate atom membership without ringAtoms
                continue;
            }
            
            if (ring1 >= static_cast<int>(input.ringAtoms.size()) ||
                ring2 >= static_cast<int>(input.ringAtoms.size())) {
                return false;
            }
            
            const std::set<int> &ring1Atoms = input.ringAtoms[ring1];
            const std::set<int> &ring2Atoms = input.ringAtoms[ring2];
            
            if (!ring1Atoms.count(bhA) || !ring1Atoms.count(bhB)) {
                return false;  // Bridgeheads not in ring1
            }
            if (!ring2Atoms.count(bhA) || !ring2Atoms.count(bhB)) {
                return false;  // Bridgeheads not in ring2
            }
            
            // Verify that there is a real bond between bhA and bhB in the graph
            if (!areConnected(g, bhA, bhB)) {
                return false;  // No bond between the supposed bridgeheads
            }
        } else {
            // Without ringDirectedCycles, we can only check basic bounds
            // bondIndex should be in range [0, ringSizes[ring1])
            if (edge.bondIndex.value < 0 || edge.bondIndex.value >= input.ringSizes[ring1]) {
                return false;
            }
        }
    }
    
    return true;
}
