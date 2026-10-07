#include "FusedRingOrientation.h"
#include <queue>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>

namespace {
constexpr double PI = 3.14159265358979323846;

// Apothem (center-to-edge-midpoint distance) of a regular N-gon with side
// length 1. Circumradius isn't needed -- only apothem-sum placement is used.
double apothem(int ringSize) {
    return 1.0 / (2.0 * std::tan(PI / ringSize));
}

// The absolute angle (radians, standard math convention: 0 = +x axis,
// counterclockwise positive) of the midpoint of a ring's bondIndex-th edge,
// given the ring's own absolute rotation (the angle its own vertex 0 sits
// at). Edge k's midpoint sits exactly between vertex k and vertex k+1, each
// vertex spaced 2*PI/ringSize apart.
double edgeMidpointAngle(double ringRotation, int ringSize, int bondIndex) {
    double vertexSpacing = 2.0 * PI / ringSize;
    return ringRotation + (bondIndex + 0.5) * vertexSpacing;
}
}  // namespace

struct PlacedRing {
    double x = 0.0, y = 0.0;
    double rotation = 0.0;  // absolute angle of this ring's own vertex 0
    bool placed = false;
};

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

    int num_rings = static_cast<int>(input.ringSizes.size());
    if (input.fusions.size() >= static_cast<size_t>(num_rings)) {
        res.success = false;
        res.error = "Non-tree ring systems (ortho-and-peri-fused like phenalene/pyrene) are not yet supported.";
        return res;
    }

    // adj[ring] = list of (neighborRing, bondIndexWithinRing, isOutgoing)
    // -- isOutgoing true means THIS ring's bondIndex is the one recorded in
    // input.fusions (ring1==this ring); false means we're looking at it from
    // ring2's side and need the mirrored angle (edge midpoint + PI).
    struct AdjEntry { int neighbor; int bondIndex; bool isOutgoing; };
    std::vector<std::vector<AdjEntry>> adj(num_rings);
    for (const auto &edge : input.fusions) {
        adj[edge.ring1].push_back({edge.ring2, edge.bondIndex, true});
        adj[edge.ring2].push_back({edge.ring1, edge.bondIndex, false});
    }

    std::vector<PlacedRing> placed(num_rings);
    placed[0] = {0.0, 0.0, 0.0, true};

    std::queue<int> q;
    q.push(0);
    int visited_count = 1;

    const double TOLERANCE = 1e-6;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const auto &nbr : adj[u]) {
            int v = nbr.neighbor;
            if (placed[v].placed) continue;
            double edgeAngle;
            if (nbr.isOutgoing) {
                edgeAngle = edgeMidpointAngle(placed[u].rotation, input.ringSizes[u], nbr.bondIndex);
            } else {
                edgeAngle = edgeMidpointAngle(placed[u].rotation, input.ringSizes[u], nbr.bondIndex) + PI;
            }
            double dist = apothem(input.ringSizes[u]) + apothem(input.ringSizes[v]);
            double vx = placed[u].x + dist * std::cos(edgeAngle);
            double vy = placed[u].y + dist * std::sin(edgeAngle);
            double vRotation = edgeAngle + PI - 0.5 * (2.0 * PI / input.ringSizes[v]);
            placed[v] = {vx, vy, vRotation, true};
            visited_count++;
            q.push(v);
        }
    }

    if (visited_count != num_rings) {
        res.success = false;
        res.error = "Disconnected ring system";
        return res;
    }

    for (int i = 0; i < num_rings; i++) {
        for (int j = i + 1; j < num_rings; j++) {
            double dx = placed[i].x - placed[j].x;
            double dy = placed[i].y - placed[j].y;
            double distSq = dx * dx + dy * dy;
            double minExpectedDist = std::min(apothem(input.ringSizes[i]), apothem(input.ringSizes[j])) * 0.5;
            if (distSq < minExpectedDist * minExpectedDist) {
                res.success = false;
                res.error = "Geometric contradiction: Rings overlap";
                return res;
            }
        }
    }

    // Candidate orientations: for each real fusion bond, the rotation that
    // aligns that bond's direction with the horizontal axis, times 2 for a
    // reflection (flip y) -- a small, chemically-meaningful candidate set
    // (not continuous/arbitrary rotation search), generalizing the old
    // code's 12 discrete hex symmetries.
    std::vector<double> candidateRotations;
    for (const auto &edge : input.fusions) {
        double a = edgeMidpointAngle(placed[edge.ring1].rotation, input.ringSizes[edge.ring1], edge.bondIndex);
        candidateRotations.push_back(-a);
    }
    if (candidateRotations.empty()) candidateRotations.push_back(0.0);

    ScoreTuple best_score = {-1, -1e9, -1e9, -1e9};
    std::vector<std::pair<double,double>> best_pos;
    std::vector<double> best_rotation;
    double best_y_main = 0, best_x_mid = 0;

    for (double rot : candidateRotations) {
        for (int reflect = 0; reflect < 2; ++reflect) {
            std::vector<std::pair<double,double>> cur_pos(num_rings);
            std::vector<double> cur_rotation(num_rings);
            for (int i = 0; i < num_rings; i++) {
                double x = placed[i].x * std::cos(rot) - placed[i].y * std::sin(rot);
                double y = placed[i].x * std::sin(rot) + placed[i].y * std::cos(rot);
                if (reflect) y = -y;
                cur_pos[i] = {x, y};
                // Each ring's own rotation transforms the same way its
                // position's angle would: reflect (negate) first, then add
                // the system-wide rotation -- keeps ringRotation consistent
                // with ringPos in the final output frame.
                cur_rotation[i] = (reflect ? -placed[i].rotation : placed[i].rotation) + rot;
            }

            // Group rings into horizontal rows (same y within tolerance) to
            // find the largest contiguous row -- mirrors the old code's
            // r_groups-by-hex-row logic, generalized to real y with a
            // tolerance instead of exact integer r equality.
            std::vector<std::pair<double,double>> sortedByY = cur_pos;
            std::sort(sortedByY.begin(), sortedByY.end(), [](auto &a, auto &b){ return a.second < b.second; });

            int max_block_size = 0;
            double block_y = 0.0;
            for (size_t i = 0; i < sortedByY.size(); ) {
                size_t j = i;
                while (j < sortedByY.size() && std::abs(sortedByY[j].second - sortedByY[i].second) < 0.1) j++;
                int blockSize = static_cast<int>(j - i);
                if (blockSize > max_block_size) {
                    max_block_size = blockSize;
                    block_y = sortedByY[i].second;
                }
                i = j;
            }

            double y_main = block_y;
            double x_mid = 0.0;
            {
                double sum = 0.0; int cnt = 0;
                for (auto &p : cur_pos) {
                    if (std::abs(p.second - y_main) < 0.1) { sum += p.first; cnt++; }
                }
                if (cnt > 0) x_mid = sum / cnt;
            }

            double ur = 0, ll = 0, above = 0;
            for (int i = 0; i < num_rings; i++) {
                double y_rel = cur_pos[i].second - y_main;
                double x_rel = cur_pos[i].first - x_mid;
                bool above_axis = (y_rel > 0.1), below_axis = (y_rel < -0.1), bisect_h = (std::abs(y_rel) <= 0.1);
                bool right_axis = (x_rel > 0.1), left_axis = (x_rel < -0.1), bisect_v = (std::abs(x_rel) <= 0.1);

                if (above_axis && right_axis) ur += 1.0;
                else if (bisect_h && right_axis) ur += 0.5;
                else if (above_axis && bisect_v) ur += 0.5;
                else if (bisect_h && bisect_v) ur += 0.25;

                if (below_axis && left_axis) ll += 1.0;
                else if (bisect_h && left_axis) ll += 0.5;
                else if (below_axis && bisect_v) ll += 0.5;
                else if (bisect_h && bisect_v) ll += 0.25;

                if (above_axis) above += 1.0;
                else if (bisect_h) above += 0.5;
            }

            ScoreTuple cand_score = {max_block_size, ur, -ll, above};
            if (best_score < cand_score) {
                best_score = cand_score;
                best_pos = cur_pos;
                best_rotation = cur_rotation;
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
        double yA = best_pos[a].second, yB = best_pos[b].second;
        double xA = best_pos[a].first, xB = best_pos[b].first;
        bool a_in_main = (std::abs(yA - best_y_main) <= 0.1);
        bool b_in_main = (std::abs(yB - best_y_main) <= 0.1);
        if (a_in_main != b_in_main) return a_in_main > b_in_main;
        if (a_in_main) return xA < xB;
        if (std::abs(yA - yB) > 0.1) return yA > yB;
        return xA < xB;
    });

    res.ringOrder = sorted_rings;
    res.ringPos = best_pos;
    res.ringRotation = best_rotation;
    return res;
}
