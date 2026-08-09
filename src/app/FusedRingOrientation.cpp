#include "FusedRingOrientation.h"
#include <queue>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>

struct HexCoord {
    int q, r, s;
    bool operator==(const HexCoord& o) const { return q == o.q && r == o.r && s == o.s; }
    bool operator!=(const HexCoord& o) const { return !(*this == o); }
};

// Returns the hex grid offset for a given direction (0 to 5)
// 0: Right (+q, 0, -s)
// 1: Bottom Right (0, +r, -s)
// 2: Bottom Left (-q, +r, 0)
// 3: Left (-q, 0, +s)
// 4: Top Left (0, -r, +s)
// 5: Top Right (+q, -r, 0)
static HexCoord getDirOffset(int dir) {
    switch(dir) {
        case 0: return {1, 0, -1};
        case 1: return {0, 1, -1};
        case 2: return {-1, 1, 0};
        case 3: return {-1, 0, 1};
        case 4: return {0, -1, 1};
        case 5: return {1, -1, 0};
    }
    return {0, 0, 0};
}

static HexCoord applySym(HexCoord p, int sym) {
    int q = p.q, r = p.r, s = p.s;
    switch(sym) {
        case 0: return {q, r, s};
        case 1: return {q, s, r};
        case 2: return {r, q, s};
        case 3: return {r, s, q};
        case 4: return {s, q, r};
        case 5: return {s, r, q};
        case 6: return {-q, -r, -s};
        case 7: return {-q, -s, -r};
        case 8: return {-r, -q, -s};
        case 9: return {-r, -s, -q};
        case 10: return {-s, -q, -r};
        case 11: return {-s, -r, -q};
    }
    return p;
}

struct ScoreTuple {
    int maxRow;
    double ur;
    double inv_ll; 
    double above;
    
    bool operator<(const ScoreTuple& o) const {
        if (maxRow != o.maxRow) return maxRow < o.maxRow;
        if (ur != o.ur) return ur < o.ur;
        if (inv_ll != o.inv_ll) return inv_ll < o.inv_ll;
        return above < o.above;
    }
};

FusedRingOrientationResult computePreferredOrientation(const FusedRingSystemInput &input) {
    FusedRingOrientationResult res;
    if (input.ringSizes.empty()) {
        res.success = true;
        return res;
    }
    
    int num_rings = input.ringSizes.size();
    if (input.fusions.size() >= num_rings) {
        res.success = false;
        res.error = "Non-tree ring systems (ortho-and-peri-fused like phenalene/pyrene) are not yet supported.";
        return res;
    }
    
    std::vector<std::vector<std::pair<int, int>>> adj(num_rings);
    for (const auto& edge : input.fusions) {
        adj[edge.ring1].push_back({edge.ring2, edge.dir});
        adj[edge.ring2].push_back({edge.ring1, (edge.dir + 3) % 6});
    }
    
    std::vector<HexCoord> base_pos(num_rings);
    std::vector<bool> visited(num_rings, false);
    
    base_pos[0] = {0, 0, 0};
    visited[0] = true;
    
    std::queue<int> q;
    q.push(0);
    int visited_count = 1;
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const auto& neighbor : adj[u]) {
            int v = neighbor.first;
            int dir = neighbor.second;
            HexCoord offset = getDirOffset(dir);
            HexCoord expected = {base_pos[u].q + offset.q, base_pos[u].r + offset.r, base_pos[u].s + offset.s};
            
            if (visited[v]) {
                if (base_pos[v] != expected) {
                    res.success = false;
                    res.error = "Geometric contradiction: cycle doesn't close on hex grid";
                    return res;
                }
            } else {
                base_pos[v] = expected;
                visited[v] = true;
                visited_count++;
                q.push(v);
            }
        }
    }
    
    if (visited_count != num_rings) {
        res.success = false;
        res.error = "Disconnected ring system";
        return res;
    }
    
    for (int i = 0; i < num_rings; i++) {
        for (int j = i + 1; j < num_rings; j++) {
            if (base_pos[i] == base_pos[j]) {
                res.success = false;
                res.error = "Geometric contradiction: Rings overlap";
                return res;
            }
        }
    }

    ScoreTuple best_score = {-1, -1e9, -1e9, -1e9};
    std::vector<HexCoord> best_pos;
    double best_y_main = 0;
    double best_x_mid = 0;
    
    for (int sym = 0; sym < 12; sym++) {
        std::vector<HexCoord> cur_pos(num_rings);
        for (int i = 0; i < num_rings; i++) {
            cur_pos[i] = applySym(base_pos[i], sym);
        }
        
        std::map<int, std::vector<int>> r_groups;
        for (int i = 0; i < num_rings; i++) {
            r_groups[cur_pos[i].r].push_back(cur_pos[i].q);
        }
        
        int max_block_size = 0;
        std::vector<std::pair<int, std::pair<int, int>>> candidate_blocks;
        
        for (auto& kv : r_groups) {
            int r = kv.first;
            auto& qs = kv.second;
            std::sort(qs.begin(), qs.end());
            
            int cur_block = 1;
            int block_q_min = qs[0];
            
            for (size_t i = 1; i < qs.size(); i++) {
                if (qs[i] == qs[i-1] + 1) {
                    cur_block++;
                } else {
                    if (cur_block > max_block_size) {
                        max_block_size = cur_block;
                        candidate_blocks.clear();
                        candidate_blocks.push_back({r, {block_q_min, qs[i-1]}});
                    } else if (cur_block == max_block_size) {
                        candidate_blocks.push_back({r, {block_q_min, qs[i-1]}});
                    }
                    cur_block = 1;
                    block_q_min = qs[i];
                }
            }
            if (cur_block > max_block_size) {
                max_block_size = cur_block;
                candidate_blocks.clear();
                candidate_blocks.push_back({r, {block_q_min, qs.back()}});
            } else if (cur_block == max_block_size) {
                candidate_blocks.push_back({r, {block_q_min, qs.back()}});
            }
        }
        
        for (const auto& block : candidate_blocks) {
            int r_main = block.first;
            int q_min = block.second.first;
            int q_max = block.second.second;
            
            double x_mid = (q_min + q_max) / 2.0 + 0.5 * r_main;
            double y_main = -r_main;
            
            double ur = 0;
            double ll = 0;
            double above = 0;
            
            for (int i = 0; i < num_rings; i++) {
                double x = cur_pos[i].q + 0.5 * cur_pos[i].r;
                double y = -cur_pos[i].r;
                
                double y_rel = y - y_main;
                double x_rel = x - x_mid;
                
                bool above_axis = (y_rel > 0.1);
                bool below_axis = (y_rel < -0.1);
                bool bisect_h = (std::abs(y_rel) <= 0.1);
                
                bool right_axis = (x_rel > 0.1);
                bool left_axis = (x_rel < -0.1);
                bool bisect_v = (std::abs(x_rel) <= 0.1);
                
                // Rule B
                if (above_axis && right_axis) ur += 1.0;
                else if (bisect_h && right_axis) ur += 0.5;
                else if (above_axis && bisect_v) ur += 0.5;
                else if (bisect_h && bisect_v) ur += 0.25;
                
                // Rule C
                if (below_axis && left_axis) ll += 1.0;
                else if (bisect_h && left_axis) ll += 0.5;
                else if (below_axis && bisect_v) ll += 0.5;
                else if (bisect_h && bisect_v) ll += 0.25;
                
                // Rule D
                if (above_axis) above += 1.0;
                else if (bisect_h) above += 0.5;
            }
            
            ScoreTuple cand_score = {max_block_size, ur, -ll, above};
            if (best_score < cand_score) {
                best_score = cand_score;
                best_pos = cur_pos;
                best_y_main = y_main;
                best_x_mid = x_mid;
            }
        }
    }
    
    res.success = true;
    res.ringsInHorizontalRow = best_score.maxRow;
    res.ringsInUpperRightQuadrant = best_score.ur;
    res.ringsInLowerLeftQuadrant = -best_score.inv_ll;
    res.ringsAboveHorizontalRow = best_score.above;
    
    std::vector<int> sorted_rings(num_rings);
    for (int i = 0; i < num_rings; i++) sorted_rings[i] = i;
    
    std::sort(sorted_rings.begin(), sorted_rings.end(), [&](int a, int b) {
        double yA = -best_pos[a].r;
        double yB = -best_pos[b].r;
        double xA = best_pos[a].q + 0.5 * best_pos[a].r;
        double xB = best_pos[b].q + 0.5 * best_pos[b].r;
        
        bool a_in_main = (std::abs(yA - best_y_main) <= 0.1);
        bool b_in_main = (std::abs(yB - best_y_main) <= 0.1);
        
        if (a_in_main != b_in_main) return a_in_main > b_in_main; // true first
        
        if (a_in_main) {
            return xA < xB; // left to right
        } else {
            if (std::abs(yA - yB) > 0.1) {
                return yA > yB; // top to bottom
            }
            return xA < xB;
        }
    });
    
    res.ringOrder = sorted_rings;
    return res;
}
