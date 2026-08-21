#include <vector>
#include <algorithm>

// Returns the hydro locants (1-based) if a valid mancude superset is found, else empty.
std::vector<int> findHydroLocants(const Graph& g, const std::vector<int>& ringCycle) {
    int n = ringCycle.size();
    std::vector<int> actualDB(n, 0); // actualDB[i] = 1 if bond between i and i+1 is double
    int actualCount = 0;
    for (int i = 0; i < n; ++i) {
        int u = ringCycle[i];
        int v = ringCycle[(i + 1) % n];
        for (const auto& gb : g.bonds) {
            if ((gb.u == u && gb.v == v) || (gb.u == v && gb.v == u)) {
                if (gb.order == 2) {
                    actualDB[i] = 1;
                    actualCount++;
                }
                break;
            }
        }
    }
    
    // Determine spare valences for mancude form
    std::vector<int> spare(n, 1);
    for (int i = 0; i < n; ++i) {
        int z = g.nodes[ringCycle[i]].atomicNumber;
        if (z == 8 || z == 16 || z == 34 || z == 52) {
            spare[i] = 0;
        }
    }
    
    // Backtracking to find the maximal double bond arrangement that contains actualDB
    int maxDB = 0;
    std::vector<int> bestMissing;
    
    // We can just iterate all valid double bond patterns
    // A pattern is valid if it doesn't violate spare valences, no two adjacent DBs, and contains actualDB.
    // For small rings (n <= 10), simple recursion is very fast.
    
    auto solve = [&](auto& self, int idx, int currentDB, std::vector<int>& currentMissing) -> void {
        if (idx >= n) {
            // Check if valid around the wrap-around
            // If DB at n-1 and DB at 0, invalid
            // Wait, this is ensured if we just check.
            if (currentDB > maxDB) {
                maxDB = currentDB;
                bestMissing = currentMissing;
            }
            return;
        }
        
        // Option 1: No double bond here
        if (actualDB[idx] == 0) {
            self(self, idx + 1, currentDB, currentMissing);
        }
        
        // Option 2: Double bond here
        // Can we put a DB here?
        bool canPut = true;
        if (spare[idx] == 0 || spare[(idx + 1) % n] == 0) canPut = false;
        // Cannot be adjacent to a previous DB (idx-1)
        // Wait, need state of previous bond.
        // It's easier to just build the DB array and check validity at the end.
    };
    
    return bestMissing;
}
