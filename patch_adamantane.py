import re

with open("src/app/IupacNamer.cpp", "r", encoding="utf-8") as f:
    content = f.read()

anchor = "// --- Phase 27: Saturated Unsubstituted Bicyclic Hydrocarbon Path (von Baeyer: bicyclo[a.b.c]alkane) ---"

patch = """    // --- Phase 26: Retained Names for Adamantane and Cubane ---
    if ((ringCount == 3 && allSSSRNodes.size() == 10) || (ringCount == 5 && allSSSRNodes.size() == 8)) {
        bool validSkeleton = true;
        std::vector<int> ringNodesVec(allSSSRNodes.begin(), allSSSRNodes.end());
        std::map<int, int> graphToRingIdx;
        for (size_t i = 0; i < ringNodesVec.size(); ++i) {
            graphToRingIdx[ringNodesVec[i]] = (int)i;
            if (g.nodes[ringNodesVec[i]].atomicNumber != 6) validSkeleton = false;
        }

        std::vector<std::vector<int>> ringAdj(ringNodesVec.size());
        if (validSkeleton) {
            for (const auto &gb : g.bonds) {
                if (allSSSRNodes.count(gb.u) && allSSSRNodes.count(gb.v)) {
                    if (gb.order != 1) { validSkeleton = false; break; }
                    ringAdj[graphToRingIdx[gb.u]].push_back(graphToRingIdx[gb.v]);
                    ringAdj[graphToRingIdx[gb.v]].push_back(graphToRingIdx[gb.u]);
                }
            }
        }

        if (validSkeleton) {
            std::set<int> visited;
            std::vector<int> q = {0};
            visited.insert(0);
            size_t head = 0;
            while (head < q.size()) {
                int curr = q[head++];
                for (int nei : ringAdj[curr]) {
                    if (!visited.count(nei)) {
                        visited.insert(nei);
                        q.push_back(nei);
                    }
                }
            }
            if (visited.size() != ringNodesVec.size()) validSkeleton = false;
        }

        std::vector<std::vector<int>> refGraph;
        QString baseName;
        if (validSkeleton && ringCount == 3) {
            int deg3 = 0, deg2 = 0;
            for (const auto& adj : ringAdj) {
                if (adj.size() == 3) deg3++;
                else if (adj.size() == 2) deg2++;
                else { validSkeleton = false; break; }
            }
            if (validSkeleton && deg3 == 4 && deg2 == 6) {
                refGraph = {{1,7,8}, {0,2}, {1,3,9}, {2,4}, {3,5,8}, {4,6}, {5,7,9}, {6,0}, {0,4}, {2,6}};
                baseName = "adamantane";
            } else {
                validSkeleton = false;
            }
        } else if (validSkeleton && ringCount == 5) {
            int deg3 = 0;
            for (const auto& adj : ringAdj) {
                if (adj.size() == 3) deg3++;
                else { validSkeleton = false; break; }
            }
            if (validSkeleton && deg3 == 8) {
                refGraph = {
                    {1, 5, 7}, {0, 2, 4}, {1, 3, 7}, {2, 4, 6},
                    {1, 3, 5}, {0, 4, 6}, {3, 5, 7}, {0, 2, 6}
                };
                baseName = "cubane";
            } else {
                validSkeleton = false;
            }
        }

        if (validSkeleton) {
            bool subsOk = true;
            std::vector<std::pair<int, QString>> ringSubstituents;
            for (int rIdx : allSSSRNodes) {
                for (size_t j = 0; j < g.nodes[rIdx].neighbors.size(); ++j) {
                    int nei = g.nodes[rIdx].neighbors[j];
                    if (allSSSRNodes.count(nei)) continue;
                    QString subName = simpleRingSubstituentName(g, rIdx, nei, allSSSRNodes);
                    if (subName.isEmpty()) { subsOk = false; break; }
                    if (subName.startsWith("(") && subName.endsWith(")"))
                        subName = subName.mid(1, subName.length() - 2);
                    ringSubstituents.push_back({rIdx, subName});
                }
                if (!subsOk) break;
            }

            if (subsOk) {
                std::vector<std::vector<int>> allMappings;
                std::vector<int> currentMapping(ringNodesVec.size(), -1);
                std::vector<bool> mappedRef(refGraph.size(), false);
                
                auto findIsos = [&](auto& self, int u) -> void {
                    if (u == (int)ringNodesVec.size()) {
                        allMappings.push_back(currentMapping);
                        return;
                    }
                    for (int i = 0; i < (int)refGraph.size(); ++i) {
                        if (!mappedRef[i]) {
                            if (ringAdj[u].size() != refGraph[i].size()) continue;
                            bool edgeMatch = true;
                            for (int v = 0; v < u; ++v) {
                                bool edgeG = false;
                                for (int nei : ringAdj[u]) if (nei == v) edgeG = true;
                                bool edgeRef = false;
                                for (int nei : refGraph[i]) if (nei == currentMapping[v]) edgeRef = true;
                                if (edgeG != edgeRef) { edgeMatch = false; break; }
                            }
                            if (edgeMatch) {
                                currentMapping[u] = i;
                                mappedRef[i] = true;
                                self(self, u + 1);
                                mappedRef[i] = false;
                            }
                        }
                    }
                };
                findIsos(findIsos, 0);

                if (!allMappings.empty()) {
                    std::vector<int> bestLocants;
                    std::vector<std::pair<QString, int>> bestNamedSubs;
                    std::map<int, int> bestLocantOf;
                    bool first = true;

                    for (const auto& mapping : allMappings) {
                        std::vector<int> candLocants;
                        std::vector<std::pair<QString, int>> candNamedSubs;
                        std::map<int, int> candLocantOf;
                        for (size_t i = 0; i < ringNodesVec.size(); ++i) {
                            candLocantOf[ringNodesVec[i]] = mapping[i] + 1;
                        }
                        for (const auto& sub : ringSubstituents) {
                            int locant = candLocantOf[sub.first];
                            candLocants.push_back(locant);
                            candNamedSubs.push_back({sub.second, locant});
                        }
                        std::sort(candLocants.begin(), candLocants.end());

                        if (first || candLocants < bestLocants) {
                            bestLocants = candLocants;
                            bestNamedSubs = candNamedSubs;
                            bestLocantOf = candLocantOf;
                            first = false;
                        }
                    }

                    QString fullName = baseName;
                    if (!bestNamedSubs.empty()) {
                        std::map<QString, std::vector<int>> subsByName;
                        for (const auto& ns : bestNamedSubs) {
                            subsByName[ns.first].push_back(ns.second);
                        }
                        QString prefix;
                        for (auto it = subsByName.begin(); it != subsByName.end(); ++it) {
                            std::sort(it->second.begin(), it->second.end());
                            QStringList locStrs;
                            for (int loc : it->second) locStrs.push_back(QString::number(loc));
                            if (!prefix.isEmpty()) prefix += "-";
                            prefix += locStrs.join(",");
                            prefix += "-";
                            if (it->second.size() > 1) {
                                prefix += numericalMultiplier(it->second.size());
                            }
                            prefix += it->first;
                        }
                        if (fullName.startsWith(QChar('a')) || fullName.startsWith(QChar('e')) ||
                            fullName.startsWith(QChar('i')) || fullName.startsWith(QChar('o')) ||
                            fullName.startsWith(QChar('u'))) {
                            if (prefix.endsWith("a") || prefix.endsWith("o")) {
                                prefix.chop(1);
                            }
                        }
                        fullName = prefix + fullName;
                    }

                    StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, bestLocantOf, std::set<int>());
                    if (!stereoRes.ok) return {false, "", stereoRes.error};
                    
                    fullName = stereoRes.prefix + fullName;
                    return {true, fullName, ""};
                }
            }
        }
    }

"""

if anchor in content:
    new_content = content.replace(anchor, patch + anchor)
    with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
        f.write(new_content)
    print("Patched successfully")
else:
    print("Anchor not found!")
