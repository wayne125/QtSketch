$content = Get-Content src/app/IupacNamer.cpp -Raw

$old_block = @'
    // --- Detect unsupported fusion topologies (P-25.3.1.1.2, P-25.4, P-25.5) ---
    {
        int N_rings = allSSSRRings.size();
        bool hasBridged = false;
        bool hasPeri = false;
        bool hasP25_5 = false;
        
        std::vector<std::pair<int, int>> periPairs;

        for (int i = 0; i < N_rings; ++i) {
            for (int j = i + 1; j < N_rings; ++j) {
                std::vector<int> shared;
                for (int n : allSSSRRings[i]) {
                    if (allSSSRRings[j].count(n)) shared.push_back(n);
                }
                
                if (shared.size() >= 2) {
                    int components = 0;
                    std::set<int> visited;
                    for (int startNode : shared) {
                        if (!visited.count(startNode)) {
                            components++;
                            std::vector<int> q = {startNode};
                            visited.insert(startNode);
                            size_t head = 0;
                            while (head < q.size()) {
                                int curr = q[head++];
                                for (int nei : g.nodes[curr].neighbors) {
                                    if (std::find(shared.begin(), shared.end(), nei) != shared.end() && !visited.count(nei)) {
                                        visited.insert(nei);
                                        q.push_back(nei);
                                    }
                                }
                            }
                        }
                    }
                    
                    if (components > 1 || shared.size() > 3) {
                        hasBridged = true;
                    } else if (shared.size() == 3) {
                        hasPeri = true;
                        periPairs.push_back({i, j});
                    }
                }
            }
        }
        
        if (hasPeri && N_rings >= 3) {
            for (const auto &pair : periPairs) {
                int r1 = pair.first;
                int r2 = pair.second;
                for (int i = 0; i < N_rings; ++i) {
                    if (i != r1 && i != r2) {
                        bool touches = false;
                        for (int n : allSSSRRings[i]) {
                            if (allSSSRRings[r1].count(n) || allSSSRRings[r2].count(n)) {
                                touches = true;
                                break;
                            }
                        }
                        if (touches) {
                            hasP25_5 = true;
                            break;
                        }
                    }
                }
                if (hasP25_5) break;
            }
        }
        
        if (hasBridged) {
            return {false, "", "bridged fused ring systems (P-25.4) are not yet supported."};
        }
        if (hasP25_5) {
            return {false, "", "three-component ortho- and peri-fused systems (P-25.5) are not yet supported."};
        }
        if (hasPeri) {
            return {false, "", "ortho- and peri-fused ring systems (P-25.3.1.1.2) are not yet supported."};
        }
    }
'@

$new_block = @'
    // --- Detect unsupported fusion topologies (P-25.3.1.1.2, P-25.4, P-25.5) ---
    {
        int N_rings = allSSSRRings.size();
        bool hasBridged = false;
        bool hasPeri = false;
        bool hasP25_5 = false;
        
        std::vector<std::pair<int, int>> periPairs;

        for (int i = 0; i < N_rings; ++i) {
            bool ring1HasDouble = false;
            for (int n : allSSSRRings[i]) {
                for (int order : g.nodes[n].bondOrders) if (order == 2) ring1HasDouble = true;
            }
        
            for (int j = i + 1; j < N_rings; ++j) {
                bool ring2HasDouble = false;
                for (int n : allSSSRRings[j]) {
                    for (int order : g.nodes[n].bondOrders) if (order == 2) ring2HasDouble = true;
                }
            
                std::vector<int> shared;
                for (int n : allSSSRRings[i]) {
                    if (allSSSRRings[j].count(n)) shared.push_back(n);
                }
                
                if (shared.size() >= 2) {
                    int components = 0;
                    std::set<int> visited;
                    for (int startNode : shared) {
                        if (!visited.count(startNode)) {
                            components++;
                            std::vector<int> q = {startNode};
                            visited.insert(startNode);
                            size_t head = 0;
                            while (head < q.size()) {
                                int curr = q[head++];
                                for (int nei : g.nodes[curr].neighbors) {
                                    if (std::find(shared.begin(), shared.end(), nei) != shared.end() && !visited.count(nei)) {
                                        visited.insert(nei);
                                        q.push_back(nei);
                                    }
                                }
                            }
                        }
                    }
                    
                    // Only apply P-25 fusion rejection if BOTH rings have unsaturation. 
                    // Otherwise it is an alicyclic von Baeyer bridged system which should fall through to its own generic error.
                    if (ring1HasDouble && ring2HasDouble) {
                        if (components > 1 || shared.size() > 3) {
                            hasBridged = true;
                        } else if (shared.size() == 3) {
                            hasPeri = true;
                            periPairs.push_back({i, j});
                        }
                    }
                }
            }
        }
        
        if (hasPeri && N_rings >= 3) {
            for (const auto &pair : periPairs) {
                int r1 = pair.first;
                int r2 = pair.second;
                for (int i = 0; i < N_rings; ++i) {
                    if (i != r1 && i != r2) {
                        bool touches = false;
                        for (int n : allSSSRRings[i]) {
                            if (allSSSRRings[r1].count(n) || allSSSRRings[r2].count(n)) {
                                touches = true;
                                break;
                            }
                        }
                        if (touches) {
                            hasP25_5 = true;
                            break;
                        }
                    }
                }
                if (hasP25_5) break;
            }
        }
        
        if (hasBridged) {
            return {false, "", "bridged fused ring systems (P-25.4) are not yet supported."};
        }
        if (hasP25_5) {
            return {false, "", "three-component ortho- and peri-fused systems (P-25.5) are not yet supported."};
        }
        if (hasPeri) {
            return {false, "", "ortho- and peri-fused ring systems (P-25.3.1.1.2) are not yet supported."};
        }
    }
'@

$content = $content -replace [regex]::Escape($old_block), $new_block
Set-Content src/app/IupacNamer.cpp $content
