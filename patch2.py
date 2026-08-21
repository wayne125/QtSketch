import sys

with open('src/app/IupacNamer.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Update the gate
gate_old = '''                // Every ring-union atom must be carbon (no skeletal heteroatoms in scope).
                for (int n : ringUnionNodes) {
                    if (g.nodes[n].atomicNumber != 6) { validPreconditions = false; break; }
                }'''
gate_new = '''                // Skeletal heteroatoms must be supported by hwSeniorityRank.
                for (int n : ringUnionNodes) {
                    if (g.nodes[n].atomicNumber != 6 && hwSeniorityRank(g.nodes[n].atomicNumber) == 99) { validPreconditions = false; break; }
                }'''
code = code.replace(gate_old, gate_new)

# 2. Add heteroatom fields to NumberingCand
cand_old = '''                                struct NumberingCand {
                                    std::map<int,int> locantOf;
                                    std::vector<int> doubleBondLocants;
                                    std::vector<int> tripleBondLocants;
                                    std::vector<int> subLocants;
                                    std::vector<std::pair<QString,int>> namedSubs;
                                };'''
cand_new = '''                                struct NumberingCand {
                                    std::map<int,int> locantOf;
                                    std::vector<int> heteroatomLocants;
                                    std::vector<int> heteroatomSeniorityLocants;
                                    std::vector<int> doubleBondLocants;
                                    std::vector<int> tripleBondLocants;
                                    std::vector<int> subLocants;
                                    std::vector<std::pair<QString,int>> namedSubs;
                                };'''
code = code.replace(cand_old, cand_new)

# 3. Fill the hetero fields
fill_old = '''                                        for (const auto &rs : ringSubstituents) {
                                            auto it = cand.locantOf.find(rs.first);
                                            if (it == cand.locantOf.end()) { cand.subLocants.clear(); break; }'''
fill_new = '''                                        for (int n : ringUnionNodes) {
                                            if (g.nodes[n].atomicNumber != 6) {
                                                cand.heteroatomLocants.push_back(cand.locantOf[n]);
                                            }
                                        }
                                        std::sort(cand.heteroatomLocants.begin(), cand.heteroatomLocants.end());
                                        
                                        // P-23.3.2.2: low locants assigned by decreasing heteroatom seniority
                                        // To implement this, we can group locants by seniority rank (which is already 0=O, 1=S etc)
                                        // and then concatenate them. Comparing these vectors lexicographically will perfectly match the rule.
                                        std::map<int, std::vector<int>> locsByRank;
                                        for (int n : ringUnionNodes) {
                                            if (g.nodes[n].atomicNumber != 6) {
                                                locsByRank[hwSeniorityRank(g.nodes[n].atomicNumber)].push_back(cand.locantOf[n]);
                                            }
                                        }
                                        for (auto &kv : locsByRank) {
                                            std::sort(kv.second.begin(), kv.second.end());
                                            for (int l : kv.second) cand.heteroatomSeniorityLocants.push_back(l);
                                        }

                                        for (const auto &rs : ringSubstituents) {
                                            auto it = cand.locantOf.find(rs.first);
                                            if (it == cand.locantOf.end()) { cand.subLocants.clear(); break; }'''
code = code.replace(fill_old, fill_new)

# 4. Update the comparator
comp_old = '''                                    auto bestIt = std::min_element(cands.begin(), cands.end(),
                                        [](const NumberingCand &a, const NumberingCand &b) {
                                            if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;'''
comp_new = '''                                    auto bestIt = std::min_element(cands.begin(), cands.end(),
                                        [](const NumberingCand &a, const NumberingCand &b) {
                                            if (a.heteroatomLocants != b.heteroatomLocants) return a.heteroatomLocants < b.heteroatomLocants;
                                            if (a.heteroatomSeniorityLocants != b.heteroatomSeniorityLocants) return a.heteroatomSeniorityLocants < b.heteroatomSeniorityLocants;
                                            if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;'''
code = code.replace(comp_old, comp_new)

# 5. Build and prepend heteroPrefix
build_old = '''                                    QString prefixPart;
                                    if (!pGroups.empty()) {
                                        QStringList pStrs;
                                        for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
                                        prefixPart = pStrs.join("-");
                                    }'''
build_new = '''                                    QString prefixPart;
                                    if (!pGroups.empty()) {
                                        QStringList pStrs;
                                        for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
                                        prefixPart = pStrs.join("-");
                                    }

                                    std::map<int, std::vector<int>> locantsByZ;
                                    for (int n : ringUnionNodes) {
                                        if (g.nodes[n].atomicNumber != 6) {
                                            locantsByZ[g.nodes[n].atomicNumber].push_back(best.locantOf[n]);
                                        }
                                    }
                                    QString heteroPrefix = buildSkeletalReplacementPrefix(locantsByZ, false);
                                    if (!heteroPrefix.isEmpty()) {
                                        if (prefixPart.isEmpty()) prefixPart = heteroPrefix;
                                        else prefixPart = heteroPrefix + "-" + prefixPart;
                                    }'''
code = code.replace(build_old, build_new)

with open('src/app/IupacNamer.cpp', 'w', encoding='utf-8') as f:
    f.write(code)
