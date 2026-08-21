$content = Get-Content src/app/IupacNamer.cpp -Raw

$old_block = @'
        // 2. Detect True Peri-Fusion (P-25.3.1.1.2): 3 rings sharing a central atom
        std::set<int> periCenters;
        for (int i = 0; i < N_rings; ++i) {
            for (int j = i + 1; j < N_rings; ++j) {
                for (int k = j + 1; k < N_rings; ++k) {
                    // Check if they all share a common atom
                    std::vector<int> common;
                    for (int n : allSSSRRings[i]) {
                        if (allSSSRRings[j].count(n) && allSSSRRings[k].count(n)) common.push_back(n);
                    }
                    if (!common.empty()) {
                        hasPeri = true;
                        for (int c : common) periCenters.insert(c);
                    }
                }
            }
        }
'@

$new_block = @'
        // 2. Detect True Peri-Fusion (P-25.3.1.1.2): 3 rings sharing a central atom
        std::set<int> periCenters;
        for (int i = 0; i < N_rings; ++i) {
            bool riDouble = false; for (int n : allSSSRRings[i]) { for (int order : g.nodes[n].bondOrders) if (order == 2 || order == 4) riDouble = true; }
            for (int j = i + 1; j < N_rings; ++j) {
                bool rjDouble = false; for (int n : allSSSRRings[j]) { for (int order : g.nodes[n].bondOrders) if (order == 2 || order == 4) rjDouble = true; }
                for (int k = j + 1; k < N_rings; ++k) {
                    bool rkDouble = false; for (int n : allSSSRRings[k]) { for (int order : g.nodes[n].bondOrders) if (order == 2 || order == 4) rkDouble = true; }
                    
                    if (riDouble && rjDouble && rkDouble) {
                        std::vector<int> common;
                        for (int n : allSSSRRings[i]) {
                            if (allSSSRRings[j].count(n) && allSSSRRings[k].count(n)) common.push_back(n);
                        }
                        if (!common.empty()) {
                            hasPeri = true;
                            for (int c : common) periCenters.insert(c);
                        }
                    }
                }
            }
        }
'@

$content = $content -replace [regex]::Escape($old_block), $new_block
Set-Content src/app/IupacNamer.cpp $content
