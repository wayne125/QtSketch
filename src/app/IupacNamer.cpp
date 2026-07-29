#include "IupacNamer.h"
#include "indigo.h"
#include <QStringList>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <queue>
#include <cctype>
#include <climits>
#include <cmath>
#include <iostream>

struct GraphNode {
    int id;           // 0..N-1 index in heavy atom graph
    int indigoIdx;    // original indigo atom index
    int atomicNumber; // Z (6=C, 7=N, 8=O, 16=S, 9=F, 17=Cl, 35=Br, 53=I)
    int totalH;       // total hydrogen count (explicit + implicit)
    std::vector<int> neighbors; // neighbor indices in GraphNode array
    std::vector<int> bondOrders;// bond orders (1, 2, 3, 4=aromatic)
};

struct GraphBond {
    int u, v; // node indices
    int order; // 1, 2, 3, 4
};

struct Graph {
    std::vector<GraphNode> nodes;
    std::vector<GraphBond> bonds;
};

std::map<int, QString> computePeripheralNumbering(
    const Graph &g,
    const std::set<int> &ring1Nodes,
    const std::set<int> &ring2Nodes,
    int bhA,
    int bhB,
    const std::set<int> &substituentBearingNodes = {},
    bool preferIndicatedHydrogenLocant = false);

std::map<int, QString> computePeripheralNumbering3Ring(
    const Graph &g,
    const std::set<int> &ring1,
    const std::set<int> &ring2,
    const std::set<int> &ring3,
    const std::set<int> &bridgeheads,
    const std::set<int> &substituentBearingNodes = {},
    bool preferIndicatedHydrogenLocant = false);


std::map<int, QString> computePeripheralNumberingForMol(int mol);

namespace {

enum class GroupType {
    NONE = 0,
    SULFONIC_ACID, // Sulfonic acid
    BORONIC_ACID,  // Boronic acid
    ACID,          // Carboxylic acid
    ESTER,         // Ester
    ACYL_HALIDE,   // Acyl halide
    AMIDE,         // Amide
    NITRILE,       // Nitrile
    ALDEHYDE,      // Aldehyde
    THIAL,         // Thial (C=S aldehyde analog)
    KETONE,        // Ketone
    THIONE,        // Thione (C=S ketone analog)
    ALCOHOL,       // Alcohol
    THIOL,         // Thiol
    AMINE,         // Amine
    PHOSPHINE      // Phosphine
};

// Halogen prefix lookup
QString halogenPrefix(int z) {
    switch (z) {
        case 9:  return "fluoro";
        case 17: return "chloro";
        case 35: return "bromo";
        case 53: return "iodo";
        default: return "";
    }
}

bool isPlainBenzeneRing(int mol, const Graph &g, const std::map<int, int> &indigoToGraphIdx, int alkylRoot, int sO) {
    if (alkylRoot < 0 || alkylRoot >= static_cast<int>(g.nodes.size())) return false;
    if (g.nodes[alkylRoot].atomicNumber != 6) return false;

    int sssrIter = indigoIterateSSSR(mol);
    if (sssrIter < 0) return false;

    bool found = false;
    int subMol = 0;
    while ((subMol = indigoNext(sssrIter)) != 0) {
        std::set<int> ringIndigoIndices;
        int ringAtomIter = indigoIterateAtoms(subMol);
        if (ringAtomIter >= 0) {
            int atomHandle = 0;
            while ((atomHandle = indigoNext(ringAtomIter)) != 0) {
                ringIndigoIndices.insert(indigoIndex(atomHandle));
                indigoFree(atomHandle);
            }
            indigoFree(ringAtomIter);
        }
        indigoFree(subMol);

        if (found) continue;

        if (ringIndigoIndices.size() == 6) {
            std::set<int> candidateRingNodes;
            std::vector<int> ringHeteroNodes;
            bool validNodes = true;
            for (int idx : ringIndigoIndices) {
                if (indigoToGraphIdx.count(idx)) {
                    int gIdx = indigoToGraphIdx.at(idx);
                    candidateRingNodes.insert(gIdx);
                    if (g.nodes[gIdx].atomicNumber != 6) {
                        ringHeteroNodes.push_back(gIdx);
                    }
                } else {
                    validNodes = false;
                }
            }

            if (validNodes && candidateRingNodes.size() == 6 && candidateRingNodes.count(alkylRoot) > 0) {
                if (ringHeteroNodes.empty()) {
                    bool allAromatic = true;
                    bool allSingle = true;
                    bool hasDouble = false;
                    bool hasTriple = false;

                    for (const auto &gb : g.bonds) {
                        if (candidateRingNodes.count(gb.u) && candidateRingNodes.count(gb.v)) {
                            if (gb.order != 4) allAromatic = false;
                            if (gb.order != 1) allSingle = false;
                            if (gb.order == 2) hasDouble = true;
                            if (gb.order == 3) hasTriple = true;
                        }
                    }

                    bool isBenzene = (!hasTriple) && (allAromatic || (!allSingle && !hasDouble));
                    if (isBenzene) {
                        int nodesWithExo = 0;
                        bool validSubstituent = true;

                        for (int rNode : candidateRingNodes) {
                            int exoCount = 0;
                            for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                                int nei = g.nodes[rNode].neighbors[j];
                                if (!candidateRingNodes.count(nei)) {
                                    exoCount++;
                                    if (rNode == alkylRoot && nei == sO && g.nodes[rNode].bondOrders[j] == 1) {
                                        // attached to oxygen of ester
                                    } else {
                                        validSubstituent = false;
                                    }
                                }
                            }
                            if (exoCount > 1) {
                                validSubstituent = false;
                            } else if (exoCount == 1) {
                                nodesWithExo++;
                            }
                        }

                        if (validSubstituent && nodesWithExo == 1) {
                            found = true;
                        }
                    }
                }
            }
        }
    }
    indigoFree(sssrIter);
    return found;
}

// Chain length root prefixes C1..C20
QString chainRoot(int len) {
    static const QString roots[] = {
        "", "meth", "eth", "prop", "but", "pent", "hex", "hept", "oct", "non", "dec",
        "undec", "dodec", "tridec", "tetradec", "pentadec", "hexadec", "heptadec",
        "octadec", "nonadec", "icos"
    };
    if (len >= 1 && len <= 20) return roots[len];
    return "";
}

// Multiplying prefixes
QString multiPrefix(int count) {
    static const QString prefixes[] = {
        "", "", "di", "tri", "tetra", "penta", "hexa", "hepta", "octa", "nona", "deca"
    };
    if (count >= 2 && count <= 10) return prefixes[count];
    return "";
}

// Helper: check if character is a vowel
bool isVowel(QChar c) {
    c = c.toLower();
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

bool tryGeneralHeterocycle(const Graph &g, const std::vector<int> &ringHeteroNodes, int ringSize, QString &outNameFragment) {
    if (ringSize != 5 && ringSize != 6) return false;
    for (int nIdx : ringHeteroNodes) {
        int z = g.nodes[nIdx].atomicNumber;
        if (z != 8 && z != 16 && z != 7) return false;
    }
    outNameFragment = "<GENERAL_HETEROCYCLE>";
    return true;
}

enum class RingType { BENZENE, FURAN, THIOPHENE, SELENOPHENE, TELLUROPHENE, PYRROLE, PYRIDINE, PHOSPHININE, CYCLOALKANE, CYCLOALKENE, IMIDAZOLE, PYRIMIDINE, PYRAZOLE, OXAZOLE, ISOXAZOLE, THIAZOLE, ISOTHIAZOLE, SELENAZOLE, ISOSELENAZOLE, PYRIDAZINE, PYRAZINE, PIPERIDINE, PYRROLIDINE, TETRAHYDROFURAN, TETRAHYDROTHIOPHENE, GENERAL_HETEROCYCLE };

QString getFusionPrefixShared(RingType t) {
    if (t == RingType::FURAN) return "furo";
    if (t == RingType::THIOPHENE) return "thieno";
    if (t == RingType::SELENOPHENE) return "selenolo";
    if (t == RingType::TELLUROPHENE) return "tellurolo";
    if (t == RingType::PHOSPHININE) return "phosphinino";
    if (t == RingType::PYRIDINE) return "pyrido";
    if (t == RingType::PYRIMIDINE) return "pyrimido";
    if (t == RingType::PYRIDAZINE) return "pyridazino";
    if (t == RingType::PYRAZINE) return "pyrazino";
    if (t == RingType::OXAZOLE) return "oxazolo";
    if (t == RingType::ISOXAZOLE) return "isoxazolo";
    if (t == RingType::THIAZOLE) return "thiazolo";
    if (t == RingType::ISOTHIAZOLE) return "isothiazolo";
    if (t == RingType::SELENAZOLE) return "selenazolo";
    if (t == RingType::ISOSELENAZOLE) return "isoselenazolo";
    if (t == RingType::PYRROLE) return "pyrrolo";
    if (t == RingType::IMIDAZOLE) return "imidazo";
    if (t == RingType::PYRAZOLE) return "pyrazolo";
    return "";
}

QString getBaseNameShared(RingType t) {
    if (t == RingType::FURAN) return "furan";
    if (t == RingType::THIOPHENE) return "thiophene";
    if (t == RingType::SELENOPHENE) return "selenophene";
    if (t == RingType::TELLUROPHENE) return "tellurophene";
    if (t == RingType::PHOSPHININE) return "phosphinine";
    if (t == RingType::PYRIDINE) return "pyridine";
    if (t == RingType::PYRIMIDINE) return "pyrimidine";
    if (t == RingType::PYRIDAZINE) return "pyridazine";
    if (t == RingType::PYRAZINE) return "pyrazine";
    if (t == RingType::OXAZOLE) return "oxazole";
    if (t == RingType::ISOXAZOLE) return "isoxazole";
    if (t == RingType::THIAZOLE) return "thiazole";
    if (t == RingType::ISOTHIAZOLE) return "isothiazole";
    if (t == RingType::SELENAZOLE) return "selenazole";
    if (t == RingType::ISOSELENAZOLE) return "isoselenazole";
    if (t == RingType::PYRROLE) return "pyrrole";
    if (t == RingType::IMIDAZOLE) return "imidazole";
    if (t == RingType::PYRAZOLE) return "pyrazole";
    return "";
}


bool classifyMonocyclicHeteroRing(const Graph &g, const std::vector<int> &ringHeteroNodes, int ringSize, const std::vector<int> &ringCycle, RingType &outType, QString &outNameRoot, QString &outErrorMsg, bool allowSaturated = false) {
    outErrorMsg.clear();
    bool heteroAromatic = true;
    bool allSingleInRing = true;
    for (const auto &gb : g.bonds) {
        bool uIn = (std::find(ringCycle.begin(), ringCycle.end(), gb.u) != ringCycle.end());
        bool vIn = (std::find(ringCycle.begin(), ringCycle.end(), gb.v) != ringCycle.end());
        if (uIn && vIn) {
            if (gb.order != 4) heteroAromatic = false;
            if (gb.order != 1) allSingleInRing = false;
        }
    }

    if (!ringHeteroNodes.empty() && !heteroAromatic) {
        if (allowSaturated && allSingleInRing && ringHeteroNodes.size() == 1) {
            int hNode = ringHeteroNodes[0];
            int hZ = g.nodes[hNode].atomicNumber;
            if (ringSize == 6 && hZ == 7) {
                outType = RingType::PIPERIDINE; outNameRoot = "piperidine"; return true;
            } else if (ringSize == 5 && hZ == 7) {
                outType = RingType::PYRROLIDINE; outNameRoot = "pyrrolidine"; return true;
            } else if (ringSize == 5 && hZ == 8) {
                outType = RingType::TETRAHYDROFURAN; outNameRoot = "tetrahydrofuran"; return true;
            } else if (ringSize == 5 && hZ == 16) {
                outType = RingType::TETRAHYDROTHIOPHENE; outNameRoot = "tetrahydrothiophene"; return true;
            }
        }
        if (!heteroAromatic) {
            outErrorMsg = "Saturated or partially unsaturated heterocycles are not yet supported; only the fully aromatic (maximally unsaturated) forms are supported in this phase.";
            return false;
        }
    }

    if (ringHeteroNodes.size() == 1) {
        int hNode = ringHeteroNodes[0];
        int hZ = g.nodes[hNode].atomicNumber;
        if (ringSize == 5 && hZ == 8) {
            outType = RingType::FURAN; outNameRoot = "furan"; return true;
        } else if (ringSize == 5 && hZ == 16) {
            outType = RingType::THIOPHENE; outNameRoot = "thiophene"; return true;
        } else if (ringSize == 5 && hZ == 34) {
            outType = RingType::SELENOPHENE; outNameRoot = "selenophene"; return true;
        } else if (ringSize == 5 && hZ == 52) {
            outType = RingType::TELLUROPHENE; outNameRoot = "tellurophene"; return true;
        } else if (ringSize == 5 && hZ == 7) {
            outType = RingType::PYRROLE; outNameRoot = "pyrrole"; return true;
        } else if (ringSize == 6 && hZ == 7) {
            outType = RingType::PYRIDINE; outNameRoot = "pyridine"; return true;
        } else if (ringSize == 6 && hZ == 15) {
            outType = RingType::PHOSPHININE; outNameRoot = "phosphinine"; return true;
        } else {
            if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                outType = RingType::GENERAL_HETEROCYCLE; return true;
            } else {
                outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                return false;
            }
        }
    } else if (ringHeteroNodes.size() == 2) {
        int h1 = ringHeteroNodes[0];
        int h2 = ringHeteroNodes[1];
        int z1 = g.nodes[h1].atomicNumber;
        int z2 = g.nodes[h2].atomicNumber;
        auto getRingDist = [&](int u, int v) {
            int idxU = -1, idxV = -1;
            for (int i = 0; i < ringSize; ++i) {
                if (ringCycle[i] == u) idxU = i;
                if (ringCycle[i] == v) idxV = i;
            }
            if (idxU == -1 || idxV == -1) return -1;
            int diff = std::abs(idxU - idxV);
            return std::min(diff, ringSize - diff);
        };
        int dist = getRingDist(h1, h2);

        if (z1 == 7 && z2 == 7) {
            int h1Count = g.nodes[h1].totalH;
            int h2Count = g.nodes[h2].totalH;

            if (ringSize == 5 && dist == 1 && ((h1Count >= 1 && h2Count <= 0) || (h1Count <= 0 && h2Count >= 1))) {
                outType = RingType::PYRAZOLE; outNameRoot = "pyrazole"; return true;
            } else if (ringSize == 5 && dist == 2 && ((h1Count >= 1 && h2Count <= 0) || (h1Count <= 0 && h2Count >= 1))) {
                outType = RingType::IMIDAZOLE; outNameRoot = "imidazole"; return true;
            } else if (ringSize == 6 && dist == 1 && h1Count <= 0 && h2Count <= 0) {
                outType = RingType::PYRIDAZINE; outNameRoot = "pyridazine"; return true;
            } else if (ringSize == 6 && dist == 2 && h1Count <= 0 && h2Count <= 0) {
                outType = RingType::PYRIMIDINE; outNameRoot = "pyrimidine"; return true;
            } else if (ringSize == 6 && dist == 3 && h1Count <= 0 && h2Count <= 0) {
                outType = RingType::PYRAZINE; outNameRoot = "pyrazine"; return true;
            } else {
                if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                    outType = RingType::GENERAL_HETEROCYCLE; return true;
                } else {
                    outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                    return false;
                }
            }
        } else if ((z1 == 8 && z2 == 7) || (z1 == 7 && z2 == 8)) {
            if (ringSize == 5 && dist == 1) {
                outType = RingType::ISOXAZOLE; outNameRoot = "isoxazole"; return true;
            } else if (ringSize == 5 && dist == 2) {
                outType = RingType::OXAZOLE; outNameRoot = "oxazole"; return true;
            } else {
                if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                    outType = RingType::GENERAL_HETEROCYCLE; return true;
                } else {
                    outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                    return false;
                }
            }
        } else if ((z1 == 16 && z2 == 7) || (z1 == 7 && z2 == 16)) {
            if (ringSize == 5 && dist == 1) {
                outType = RingType::ISOTHIAZOLE; outNameRoot = "isothiazole"; return true;
            } else if (ringSize == 5 && dist == 2) {
                outType = RingType::THIAZOLE; outNameRoot = "thiazole"; return true;
            } else {
                if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                    outType = RingType::GENERAL_HETEROCYCLE; return true;
                } else {
                    outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                    return false;
                }
            }
        } else if ((z1 == 34 && z2 == 7) || (z1 == 7 && z2 == 34)) {
            if (ringSize == 5 && dist == 1) {
                outType = RingType::ISOSELENAZOLE; outNameRoot = "isoselenazole"; return true;
            } else if (ringSize == 5 && dist == 2) {
                outType = RingType::SELENAZOLE; outNameRoot = "selenazole"; return true;
            } else {
                if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                    outType = RingType::GENERAL_HETEROCYCLE; return true;
                } else {
                    outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                    return false;
                }
            }
        } else {
            if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                outType = RingType::GENERAL_HETEROCYCLE; return true;
            } else {
                outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
                return false;
            }
        }
    } else if (ringHeteroNodes.size() > 2) {
        if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
            outType = RingType::GENERAL_HETEROCYCLE; return true;
        } else {
            outErrorMsg = "Heterocycles other than furan, thiophene, pyrrole, pyridine, pyrazole, oxazole, isoxazole, thiazole, isothiazole, imidazole, pyridazine, pyrimidine, and pyrazine are not supported in Phase 2.";
            return false;
        }
    }
    return false;
}

// Forward declarations
QString nameBranchGraph(const Graph &g, int rootIdx, int parentIdx,
                               const std::vector<std::set<int>> &allIndependentRings = {},
                               const std::set<int> &forbiddenNodes = {});

QString nameRingAsSubstituent(const Graph &g, const std::set<int> &ringNodes, int attachmentNode, int parentLinkNode,
                                     const std::vector<std::set<int>> &allIndependentRings = {},
                                     const std::set<int> &forbiddenNodes = {}) {
    if (ringNodes.empty() || !ringNodes.count(attachmentNode)) return "";
    std::set<int> combinedForbidden = forbiddenNodes;
    for (int n : ringNodes) combinedForbidden.insert(n);
    int ringSize = static_cast<int>(ringNodes.size());
    if (ringSize != 5 && ringSize != 6) return "";

    std::vector<int> ringCycle;
    int startNode = *ringNodes.begin();
    int current = startNode;
    int previous = -1;

    for (int step = 0; step < ringSize; ++step) {
        ringCycle.push_back(current);
        int nextNode = -1;
        for (int nei : g.nodes[current].neighbors) {
            if (ringNodes.count(nei) && nei != previous) {
                nextNode = nei;
                break;
            }
        }
        if (nextNode == -1) break;
        previous = current;
        current = nextNode;
    }

    if (static_cast<int>(ringCycle.size()) != ringSize) return "";

    std::vector<int> ringHeteroNodes;
    for (int idx : ringNodes) {
        if (g.nodes[idx].atomicNumber != 6) ringHeteroNodes.push_back(idx);
    }

    RingType rType;
    QString parentNameRoot;
    QString classErr;

    if (!ringHeteroNodes.empty()) {
        if (!classifyMonocyclicHeteroRing(g, ringHeteroNodes, ringSize, ringCycle, rType, parentNameRoot, classErr, true)) {
            return "";
        }
    } else {
        bool allAromatic = true;
        bool allSingle = true;
        bool hasDouble = false;
        bool hasTriple = false;

        for (const auto &gb : g.bonds) {
            if (ringNodes.count(gb.u) && ringNodes.count(gb.v)) {
                if (gb.order != 4) allAromatic = false;
                if (gb.order != 1) allSingle = false;
                if (gb.order == 2) hasDouble = true;
                if (gb.order == 3) hasTriple = true;
            }
        }

        if (hasTriple) return "";

        if (ringSize == 6 && (allAromatic || (!allSingle && !hasDouble))) {
            rType = RingType::BENZENE; parentNameRoot = "benzene";
        } else if (allSingle) {
            rType = RingType::CYCLOALKANE; parentNameRoot = "cyclo" + chainRoot(ringSize) + "ane";
        } else {
            rType = RingType::CYCLOALKENE; parentNameRoot = "cyclo" + chainRoot(ringSize);
        }
    }

    std::vector<std::vector<int>> ringCandidates;
    if (rType == RingType::FURAN || rType == RingType::THIOPHENE || rType == RingType::PYRROLE || rType == RingType::PYRIDINE ||
        rType == RingType::PIPERIDINE || rType == RingType::PYRROLIDINE || rType == RingType::TETRAHYDROFURAN || rType == RingType::TETRAHYDROTHIOPHENE) {
        int hNode = ringHeteroNodes[0];
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hNode) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        ringCandidates.push_back(fwd);
        ringCandidates.push_back(bwd);
    } else if (rType == RingType::IMIDAZOLE || rType == RingType::PYRAZOLE) {
        int hNH = -1, hN = -1;
        if (g.nodes[ringHeteroNodes[0]].totalH >= 1) {
            hNH = ringHeteroNodes[0]; hN = ringHeteroNodes[1];
        } else {
            hNH = ringHeteroNodes[1]; hN = ringHeteroNodes[0];
        }
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hNH) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        int reqOtherIdx = (rType == RingType::IMIDAZOLE) ? 2 : 1;
        if (fwd[reqOtherIdx] == hN) ringCandidates.push_back(fwd);
        if (bwd[reqOtherIdx] == hN) ringCandidates.push_back(bwd);
    } else if (rType == RingType::PYRIMIDINE || rType == RingType::PYRIDAZINE || rType == RingType::PYRAZINE) {
        int n1 = ringHeteroNodes[0], n2 = ringHeteroNodes[1];
        int idx1 = -1, idx2 = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == n1) idx1 = i;
            if (ringCycle[i] == n2) idx2 = i;
        }
        int reqOtherIdx = (rType == RingType::PYRIDAZINE) ? 1 : ((rType == RingType::PYRIMIDINE) ? 2 : 3);
        std::vector<int> fwd1(ringSize), bwd1(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd1[i] = ringCycle[(idx1 + i) % ringSize];
            bwd1[i] = ringCycle[(idx1 - i + ringSize) % ringSize];
        }
        if (fwd1[reqOtherIdx] == n2) ringCandidates.push_back(fwd1);
        if (bwd1[reqOtherIdx] == n2) ringCandidates.push_back(bwd1);

        std::vector<int> fwd2(ringSize), bwd2(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd2[i] = ringCycle[(idx2 + i) % ringSize];
            bwd2[i] = ringCycle[(idx2 - i + ringSize) % ringSize];
        }
        if (fwd2[reqOtherIdx] == n1) ringCandidates.push_back(fwd2);
        if (bwd2[reqOtherIdx] == n1) ringCandidates.push_back(bwd2);
    } else if (rType == RingType::OXAZOLE || rType == RingType::ISOXAZOLE || rType == RingType::THIAZOLE || rType == RingType::ISOTHIAZOLE) {
        int hOS = -1, hN = -1;
        int z0 = g.nodes[ringHeteroNodes[0]].atomicNumber;
        if (z0 == 8 || z0 == 16) {
            hOS = ringHeteroNodes[0]; hN = ringHeteroNodes[1];
        } else {
            hOS = ringHeteroNodes[1]; hN = ringHeteroNodes[0];
        }
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hOS) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        int reqNIdx = (rType == RingType::ISOXAZOLE || rType == RingType::ISOTHIAZOLE) ? 1 : 2;
        if (fwd[reqNIdx] == hN) ringCandidates.push_back(fwd);
        if (bwd[reqNIdx] == hN) ringCandidates.push_back(bwd);
    } else {
        for (int st = 0; st < ringSize; ++st) {
            std::vector<int> fwd(ringSize), bwd(ringSize);
            for (int i = 0; i < ringSize; ++i) {
                fwd[i] = ringCycle[(st + i) % ringSize];
                bwd[i] = ringCycle[(st - i + ringSize) % ringSize];
            }
            ringCandidates.push_back(fwd);
            ringCandidates.push_back(bwd);
        }
    }

    struct CandidateScore {
        int attachLocant;
        std::vector<int> sortedSubLocants;
        std::vector<std::pair<QString, int>> namedSubstituents;
    };

    std::vector<CandidateScore> validScores;

    for (const auto &cand : ringCandidates) {
        std::map<int, int> locantMap;
        for (int i = 0; i < ringSize; ++i) {
            locantMap[cand[i]] = i + 1;
        }

        int attachLocant = locantMap[attachmentNode];

        CandidateScore cs;
        cs.attachLocant = attachLocant;

        bool candValid = true;
        for (int i = 0; i < ringSize; ++i) {
            int rNode = cand[i];
            int locant = i + 1;

            for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                int nei = g.nodes[rNode].neighbors[j];
                if (ringNodes.count(nei)) continue;
                if (rNode == attachmentNode && nei == parentLinkNode) continue;

                int order = g.nodes[rNode].bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                QString subName;

                if (nZ == 8 && order == 1) {
                    int alkylNei = -1;
                    for (int oNei : g.nodes[nei].neighbors) {
                        if (oNei != rNode) { alkylNei = oNei; break; }
                    }
                    if (alkylNei != -1) {
                        QString alkylName = nameBranchGraph(g, alkylNei, nei, allIndependentRings);
                        if (alkylName.isEmpty()) { candValid = false; break; }
                        if (alkylName.startsWith("(") && alkylName.endsWith(")")) {
                            subName = "[(" + alkylName.mid(1, alkylName.length() - 2) + ")oxy]";
                        } else if (alkylName.startsWith("[") && alkylName.endsWith("]")) {
                            subName = "[(" + alkylName.mid(1, alkylName.length() - 2) + ")oxy]";
                        } else if (alkylName == "methyl" || alkylName == "ethyl" || alkylName == "propyl" || alkylName == "butyl") {
                            alkylName.chop(2);
                            subName = alkylName + "oxy";
                        } else {
                            subName = "[(" + alkylName + ")oxy]";
                        }
                    } else {
                        subName = "hydroxy";
                    }
                } else if (nZ == 7 && order == 1) {
                    int cAcyl = -1;
                    for (int nNei : g.nodes[nei].neighbors) {
                        if (nNei != rNode && g.nodes[nNei].atomicNumber == 6) {
                            for (size_t k = 0; k < g.nodes[nNei].neighbors.size(); ++k) {
                                int cNei = g.nodes[nNei].neighbors[k];
                                if (g.nodes[cNei].atomicNumber == 8 && g.nodes[nNei].bondOrders[k] == 2) {
                                    cAcyl = nNei; break;
                                }
                            }
                        }
                    }
                    if (cAcyl != -1) {
                        int rGroup = -1;
                        for (int aNei : g.nodes[cAcyl].neighbors) {
                            if (aNei != nei && g.nodes[aNei].atomicNumber != 8) {
                                rGroup = aNei; break;
                            }
                        }
                        if (rGroup != -1) {
                            QString rName = nameBranchGraph(g, rGroup, cAcyl, allIndependentRings);
                            if (!rName.isEmpty()) {
                                if (rName.endsWith("phenyl")) {
                                    rName.chop(6);
                                    subName = rName + "benzamido";
                                } else if (rName.endsWith("yl")) {
                                    rName.chop(2);
                                    subName = rName + "amido";
                                } else {
                                    subName = rName + "amido";
                                }
                            }
                        }
                    }
                    if (subName.isEmpty()) {
                        subName = "amino";
                    }
                } else if (nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) {
                    subName = halogenPrefix(nZ);
                } else {
                    subName = nameBranchGraph(g, nei, rNode, allIndependentRings, combinedForbidden);
                }

                if (subName.isEmpty()) { candValid = false; break; }
                cs.sortedSubLocants.push_back(locant);
                cs.namedSubstituents.push_back({subName, locant});
            }
            if (!candValid) break;
        }

        if (candValid) {
            std::sort(cs.sortedSubLocants.begin(), cs.sortedSubLocants.end());
            validScores.push_back(cs);
        }
    }

    if (validScores.empty()) return "";

    auto bestIt = std::min_element(validScores.begin(), validScores.end(),
        [](const CandidateScore &a, const CandidateScore &b) {
            if (a.attachLocant != b.attachLocant) return a.attachLocant < b.attachLocant;
            if (a.sortedSubLocants != b.sortedSubLocants) return a.sortedSubLocants < b.sortedSubLocants;
            return false;
        });

    const CandidateScore &best = *bestIt;

    std::map<QString, std::vector<int>> prefixLocantsMap;
    for (const auto &ns : best.namedSubstituents) {
        prefixLocantsMap[ns.first].push_back(ns.second);
    }

    struct PrefixGroup {
        QString baseName;
        QString formattedStr;
    };
    std::vector<PrefixGroup> pGroups;
    for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
        QString pName = it->first;
        std::vector<int> locs = it->second;
        std::sort(locs.begin(), locs.end());

        QStringList locStrs;
        for (int l : locs) locStrs.append(QString::number(l));

        QString pStr = locStrs.join(",");
        if (locs.size() > 1) {
            pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
        } else {
            pStr += "-" + pName;
        }

        PrefixGroup pg;
        pg.baseName = (pName.startsWith("(") || pName.startsWith("[")) ? pName.mid(1) : pName;
        pg.formattedStr = pStr;
        pGroups.push_back(pg);
    }

    std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
        return a.baseName.toLower() < b.baseName.toLower();
    });

    QString prefixPart;
    if (!pGroups.empty()) {
        QStringList pStrs;
        for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
        prefixPart = pStrs.join("-");
    }

    QString res;
    if (rType == RingType::BENZENE) {
        res = prefixPart.isEmpty() ? "phenyl" : (prefixPart + "phenyl");
    } else if (rType == RingType::CYCLOALKANE) {
        QString stem = "cyclo" + chainRoot(ringSize) + "yl";
        res = prefixPart.isEmpty() ? stem : (prefixPart + stem);
    } else {
        QString stem;
        if (rType == RingType::PYRIDINE) stem = "pyridin";
        else if (rType == RingType::PIPERIDINE) stem = "piperidin";
        else if (rType == RingType::PYRROLIDINE) stem = "pyrrolidin";
        else if (rType == RingType::FURAN) stem = "furan";
        else if (rType == RingType::THIOPHENE) stem = "thiophen";
        else if (rType == RingType::PYRROLE) stem = "pyrrol";
        else if (rType == RingType::IMIDAZOLE) stem = "imidazol";
        else if (rType == RingType::PYRAZOLE) stem = "pyrazol";
        else if (rType == RingType::PYRIMIDINE) stem = "pyrimidin";
        else if (rType == RingType::PYRIDAZINE) stem = "pyridazin";
        else if (rType == RingType::PYRAZINE) stem = "pyrazin";
        else if (rType == RingType::OXAZOLE) stem = "oxazol";
        else if (rType == RingType::ISOXAZOLE) stem = "isoxazol";
        else if (rType == RingType::THIAZOLE) stem = "thiazol";
        else if (rType == RingType::ISOTHIAZOLE) stem = "isothiazol";
        else if (rType == RingType::TETRAHYDROFURAN) stem = "tetrahydrofuran";
        else if (rType == RingType::TETRAHYDROTHIOPHENE) stem = "tetrahydrothiophen";
        else stem = parentNameRoot;

        QString suffix = QString("-%1-yl").arg(best.attachLocant);
        res = prefixPart + stem + suffix;
    }

    if (rType != RingType::BENZENE && rType != RingType::CYCLOALKANE) {
        if (res.contains("[")) return "{" + res + "}";
        if (res.contains("(")) return "[" + res + "]";
        return "(" + res + ")";
    }
    if (!prefixPart.isEmpty()) {
        if (res.contains("[")) return "{" + res + "}";
        if (res.contains("(")) return "[" + res + "]";
        return "(" + res + ")";
    }
    return res;
}

QString nameBranchGraph(const Graph &g, int rootIdx, int parentIdx, const std::vector<std::set<int>> &allIndependentRings, const std::set<int> &forbiddenNodes) {
    if (forbiddenNodes.count(rootIdx)) return "";

    if (!allIndependentRings.empty()) {
        for (size_t i = 0; i < allIndependentRings.size(); ++i) {
            for (size_t j = i + 1; j < allIndependentRings.size(); ++j) {
                const auto &r1 = allIndependentRings[i];
                const auto &r2 = allIndependentRings[j];
                if (r1.count(rootIdx) || r2.count(rootIdx)) {
                    std::vector<int> shared;
                    for (int n : r1) if (r2.count(n)) shared.push_back(n);
                    if (shared.size() == 2) {
                        int bhA = shared[0], bhB = shared[1];
                        bool bhBonded = false;
                        for (int nei : g.nodes[bhA].neighbors) if (nei == bhB) { bhBonded = true; break; }
                        if (bhBonded) {
                            std::set<int> naphNodes = r1;
                            for (int n : r2) naphNodes.insert(n);
                            if (naphNodes.count(rootIdx)) {
                                bool is1Type = false;
                                for (int nei : g.nodes[rootIdx].neighbors) {
                                    if (nei == bhA || nei == bhB) { is1Type = true; break; }
                                }
                                return is1Type ? "naphthalen-1-yl" : "naphthalen-2-yl";
                            }
                        }
                    }
                }
            }
        }

        for (const auto &rNodes : allIndependentRings) {
            if (rNodes.count(rootIdx)) {
                bool isFusedOrBridged = false;
                for (const auto &otherR : allIndependentRings) {
                    if (&otherR != &rNodes) {
                        for (int n : rNodes) {
                            if (otherR.count(n)) { isFusedOrBridged = true; break; }
                        }
                    }
                    if (isFusedOrBridged) break;
                }
                if (isFusedOrBridged) return "";

                std::set<int> combinedForbidden = forbiddenNodes;
                for (int n : rNodes) combinedForbidden.insert(n);
                return nameRingAsSubstituent(g, rNodes, rootIdx, parentIdx, allIndependentRings, combinedForbidden);
            }
        }
    }

    int rZ = g.nodes[rootIdx].atomicNumber;
    if (rZ == 9 || rZ == 17 || rZ == 35 || rZ == 53) {
        return halogenPrefix(rZ);
    }

    if (rZ == 8) {
        int alkylNei = -1;
        for (int nei : g.nodes[rootIdx].neighbors) {
            if (nei != parentIdx && !forbiddenNodes.count(nei)) { alkylNei = nei; break; }
        }
        if (alkylNei != -1) {
            std::set<int> newForbidden = forbiddenNodes;
            newForbidden.insert(rootIdx);
            QString alkylName = nameBranchGraph(g, alkylNei, rootIdx, allIndependentRings, newForbidden);
            if (alkylName.isEmpty()) return "";
            if (alkylName.startsWith("(") && alkylName.endsWith(")")) {
                return alkylName + "oxy";
            } else if (alkylName == "methyl" || alkylName == "ethyl" || alkylName == "propyl" || alkylName == "butyl") {
                alkylName.chop(2);
                return alkylName + "oxy";
            } else {
                return "(" + alkylName + ")oxy";
            }
        }
        return "hydroxy";
    }

    if (rZ == 7) {
        bool isAzide = false;
        for (size_t k = 0; k < g.nodes[rootIdx].neighbors.size(); ++k) {
            int nNei = g.nodes[rootIdx].neighbors[k];
            if (g.nodes[nNei].atomicNumber == 7 && g.nodes[rootIdx].bondOrders[k] >= 2) {
                isAzide = true;
                break;
            }
        }
        if (isAzide) {
            return "";
        }

        int cAcyl = -1;
        for (int nei : g.nodes[rootIdx].neighbors) {
            if (nei != parentIdx && g.nodes[nei].atomicNumber == 6) {
                for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                    int cNei = g.nodes[nei].neighbors[k];
                    if (g.nodes[cNei].atomicNumber == 8 && g.nodes[nei].bondOrders[k] == 2) {
                        cAcyl = nei; break;
                    }
                }
            }
        }
        if (cAcyl != -1) {
            int rGroup = -1;
            for (int aNei : g.nodes[cAcyl].neighbors) {
                if (aNei != rootIdx && g.nodes[aNei].atomicNumber != 8) {
                    rGroup = aNei; break;
                }
            }
            if (rGroup != -1) {
                QString rName = nameBranchGraph(g, rGroup, cAcyl, allIndependentRings, forbiddenNodes);
                if (!rName.isEmpty()) {
                    QString cleanName = rName;
                    if (cleanName.startsWith("(") && cleanName.endsWith(")")) {
                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                    }
                    if (cleanName.startsWith("[") && cleanName.endsWith("]")) {
                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                    }
                    if (cleanName.startsWith("{") && cleanName.endsWith("}")) {
                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                    }
                    if (cleanName.endsWith("phenyl")) {
                        cleanName.chop(6);
                        QString bName = cleanName + "benzamido";
                        return cleanName.isEmpty() ? bName : ("(" + bName + ")");
                    } else if (cleanName.endsWith("yl")) {
                        cleanName.chop(2);
                        QString aName = cleanName + "amido";
                        return cleanName.isEmpty() ? aName : ("(" + aName + ")");
                    }
                    QString aName = cleanName + "amido";
                    return cleanName.isEmpty() ? aName : ("(" + aName + ")");
                }
            }
        }
        return "amino";
    }

    std::vector<int> path;
    int curr = rootIdx;
    int prev = parentIdx;
    path.push_back(curr);

    while (true) {
        int nextC = -1;
        int maxDist = -1;

        for (size_t i = 0; i < g.nodes[curr].neighbors.size(); ++i) {
            int nei = g.nodes[curr].neighbors[i];
            if (nei == prev) continue;
            if (g.nodes[nei].atomicNumber == 6) {
                std::vector<int> stack = {nei};
                std::map<int, int> dist;
                dist[nei] = 1;
                dist[curr] = 0;
                int localMax = 1;
                while (!stack.empty()) {
                    int u = stack.back();
                    stack.pop_back();
                    int d = dist[u];
                    if (d > localMax) localMax = d;
                    for (int nxt : g.nodes[u].neighbors) {
                        if (g.nodes[nxt].atomicNumber == 6 && dist.find(nxt) == dist.end() && nxt != prev) {
                            dist[nxt] = d + 1;
                            stack.push_back(nxt);
                        }
                    }
                }
                if (localMax > maxDist) {
                    maxDist = localMax;
                    nextC = nei;
                }
            }
        }
        if (nextC == -1) break;
        prev = curr;
        curr = nextC;
        path.push_back(curr);
    }

    int bLen = static_cast<int>(path.size());
    std::set<int> pathSet(path.begin(), path.end());

    std::map<int, QStringList> locantPrefixes;
    for (int i = 0; i < bLen; ++i) {
        int cNode = path[i];
        int locant = i + 1;

        for (size_t j = 0; j < g.nodes[cNode].neighbors.size(); ++j) {
            int nei = g.nodes[cNode].neighbors[j];
            if (nei == parentIdx && i == 0) continue;
            if (pathSet.count(nei)) continue;

            int z = g.nodes[nei].atomicNumber;
            if (z == 9 || z == 17 || z == 35 || z == 53) {
                locantPrefixes[locant].append(halogenPrefix(z));
            } else if (z == 6 || z == 8 || z == 7) {
                QString subBranch = nameBranchGraph(g, nei, cNode, allIndependentRings);
                if (subBranch.isEmpty()) return "";
                locantPrefixes[locant].append(subBranch);
            } else {
                return "";
            }
        }
    }

    QString root = chainRoot(bLen);

    if (!locantPrefixes.empty()) {
        std::map<QString, std::vector<int>> groupLocants;
        for (auto it = locantPrefixes.begin(); it != locantPrefixes.end(); ++it) {
            int loc = it->first;
            for (const QString &p : it->second) {
                groupLocants[p].push_back(loc);
            }
        }

        QStringList formattedPrefixes;
        for (auto it = groupLocants.begin(); it != groupLocants.end(); ++it) {
            QString pName = it->first;
            std::vector<int> locs = it->second;
            std::sort(locs.begin(), locs.end());

            QStringList locStrs;
            for (int l : locs) locStrs.append(QString::number(l));

            QString prefixStr = locStrs.join(",");
            if (locs.size() > 1) {
                prefixStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
            } else {
                prefixStr += "-" + pName;
            }
            formattedPrefixes.append(prefixStr);
        }

        QString pStr = formattedPrefixes.join("-");
        return QString("(%1%2yl)").arg(pStr, root);
    }

    return root + "yl";
}

// Function to name one side (acyl chain) of an acid anhydride
QString nameAcidChainFrom(const Graph &g, int startCarbon, const std::set<int> &excludeNodes) {
    if (startCarbon < 0 || startCarbon >= static_cast<int>(g.nodes.size())) return "";
    if (g.nodes[startCarbon].atomicNumber != 6) return "";

    std::vector<std::vector<int>> candidatePaths;
    std::vector<int> currentPath = {startCarbon};
    std::vector<bool> visited(g.nodes.size(), false);
    visited[startCarbon] = true;

    auto dfs = [&](auto self, int curr) -> void {
        candidatePaths.push_back(currentPath);
        for (int nei : g.nodes[curr].neighbors) {
            if (g.nodes[nei].atomicNumber == 6 && !excludeNodes.count(nei) && !visited[nei]) {
                visited[nei] = true;
                currentPath.push_back(nei);
                self(self, nei);
                currentPath.pop_back();
                visited[nei] = false;
            }
        }
    };
    dfs(dfs, startCarbon);

    if (candidatePaths.empty()) return "";

    int maxLength = -1, maxUnsat = -1;
    for (const auto &p : candidatePaths) {
        int len = static_cast<int>(p.size());
        int uCount = 0;
        for (size_t i = 0; i + 1 < p.size(); ++i) {
            int u = p[i], v = p[i+1];
            for (size_t k = 0; k < g.nodes[u].neighbors.size(); ++k) {
                if (g.nodes[u].neighbors[k] == v) {
                    if (g.nodes[u].bondOrders[k] > 1) uCount++;
                    break;
                }
            }
        }
        if (len > maxLength) {
            maxLength = len;
            maxUnsat = uCount;
        } else if (len == maxLength) {
            if (uCount > maxUnsat) maxUnsat = uCount;
        }
    }

    if (maxLength > 20 || maxLength < 1) return "";

    std::vector<std::vector<int>> topPaths;
    for (const auto &p : candidatePaths) {
        int len = static_cast<int>(p.size());
        int uCount = 0;
        for (size_t i = 0; i + 1 < p.size(); ++i) {
            int u = p[i], v = p[i+1];
            for (size_t k = 0; k < g.nodes[u].neighbors.size(); ++k) {
                if (g.nodes[u].neighbors[k] == v) {
                    if (g.nodes[u].bondOrders[k] > 1) uCount++;
                    break;
                }
            }
        }
        if (len == maxLength && uCount == maxUnsat) {
            topPaths.push_back(p);
        }
    }

    struct PathSignature {
        std::vector<int> parentChain;
        std::vector<int> principalLocants;
        std::vector<int> doubleBondLocants;
        std::vector<int> tripleBondLocants;
        std::vector<int> substituentLocants;
        std::vector<std::pair<QString,int>> namedSubstituents;
    };

    std::vector<PathSignature> directedSignatures;

    for (const auto &dChain : topPaths) {
        PathSignature sig;
        sig.parentChain = dChain;
        sig.principalLocants = {1};
        std::set<int> chainSet(dChain.begin(), dChain.end());

        for (size_t i = 0; i < dChain.size(); ++i) {
            int c = dChain[i];
            int locant = static_cast<int>(i + 1);

            if (i + 1 < dChain.size()) {
                int nextC = dChain[i+1];
                for (size_t k = 0; k < g.nodes[c].neighbors.size(); ++k) {
                    if (g.nodes[c].neighbors[k] == nextC) {
                        int bo = g.nodes[c].bondOrders[k];
                        if (bo == 2) sig.doubleBondLocants.push_back(locant);
                        else if (bo == 3) sig.tripleBondLocants.push_back(locant);
                        break;
                    }
                }
            }

            for (size_t j = 0; j < g.nodes[c].neighbors.size(); ++j) {
                int nei = g.nodes[c].neighbors[j];
                int order = g.nodes[c].bondOrders[j];
                if (chainSet.count(nei)) continue;
                if (excludeNodes.count(nei)) continue;

                int nz = g.nodes[nei].atomicNumber;
                if (i == 0 && nz == 8 && order == 2) {
                    continue;
                }

                QString subName;
                if (nz == 9 || nz == 17 || nz == 35 || nz == 53) {
                    subName = halogenPrefix(nz);
                } else if (nz == 8) {
                    if (order == 1) subName = "hydroxy";
                    else if (order == 2) subName = "oxo";
                } else if (nz == 7) {
                    bool isAzide = false;
                    for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                        int nNei = g.nodes[nei].neighbors[k];
                        if (g.nodes[nNei].atomicNumber == 7 && g.nodes[nei].bondOrders[k] >= 2) {
                            isAzide = true; break;
                        }
                    }
                    if (!isAzide && order == 1) subName = "amino";
                } else if (nz == 6) {
                    subName = nameBranchGraph(g, nei, c);
                    if (subName.isEmpty()) return "";
                }

                if (!subName.isEmpty()) {
                    if (subName.startsWith("(")) subName = subName.mid(1);
                    sig.substituentLocants.push_back(locant);
                    sig.namedSubstituents.push_back({subName, locant});
                }
            }
        }
        std::sort(sig.doubleBondLocants.begin(), sig.doubleBondLocants.end());
        std::sort(sig.tripleBondLocants.begin(), sig.tripleBondLocants.end());
        std::sort(sig.substituentLocants.begin(), sig.substituentLocants.end());
        directedSignatures.push_back(sig);
    }

    if (directedSignatures.empty()) return "";

    auto bestIt = std::min_element(directedSignatures.begin(), directedSignatures.end(),
        [](const PathSignature &a, const PathSignature &b) {
            if (a.principalLocants != b.principalLocants) return a.principalLocants < b.principalLocants;
            if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;
            if (a.tripleBondLocants != b.tripleBondLocants) return a.tripleBondLocants < b.tripleBondLocants;
            if (a.substituentLocants != b.substituentLocants) return a.substituentLocants < b.substituentLocants;

            auto firstAlpha = [](const std::vector<std::pair<QString,int>> &named) {
                return std::min_element(named.begin(), named.end(),
                    [](const auto &x, const auto &y) { return x.first.toLower() < y.first.toLower(); });
            };
            if (!a.namedSubstituents.empty()) {
                QString alphaName = firstAlpha(a.namedSubstituents)->first;
                auto findLocant = [&](const std::vector<std::pair<QString,int>> &named) {
                    int best = INT_MAX;
                    for (const auto &ns : named) if (ns.first == alphaName) best = std::min(best, ns.second);
                    return best;
                };
                int aLoc = findLocant(a.namedSubstituents);
                int bLoc = findLocant(b.namedSubstituents);
                if (aLoc != bLoc) return aLoc < bLoc;
            }
            return false;
        });

    PathSignature bestSig = *bestIt;
    int k = static_cast<int>(bestSig.parentChain.size());

    std::map<int, QStringList> locantSubstituents;
    for (const auto &ns : bestSig.namedSubstituents) {
        locantSubstituents[ns.second].append(ns.first);
    }

    std::map<QString, std::vector<int>> prefixLocantsMap;
    for (auto it = locantSubstituents.begin(); it != locantSubstituents.end(); ++it) {
        int loc = it->first;
        for (const QString &p : it->second) {
            prefixLocantsMap[p].push_back(loc);
        }
    }

    struct PrefixGroup {
        QString baseName;
        QString formattedStr;
    };
    std::vector<PrefixGroup> pGroups;

    for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
        QString pName = it->first;
        std::vector<int> locs = it->second;
        std::sort(locs.begin(), locs.end());

        QString pStr;
        if (k == 1) {
            pStr = (locs.size() > 1 ? multiPrefix(static_cast<int>(locs.size())) : QString()) + pName;
        } else {
            QStringList locStrs;
            for (int l : locs) locStrs.append(QString::number(l));
            pStr = locStrs.join(",");
            if (locs.size() > 1) {
                pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
            } else {
                pStr += "-" + pName;
            }
        }

        PrefixGroup pg;
        pg.baseName = pName.startsWith("(") ? pName.mid(1) : pName;
        pg.formattedStr = pStr;
        pGroups.push_back(pg);
    }

    std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
        return a.baseName.toLower() < b.baseName.toLower();
    });

    QString prefixPart;
    if (!pGroups.empty()) {
        QStringList pStrs;
        for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
        prefixPart = pStrs.join(k == 1 ? "" : "-");
    }

    QString rootStr = chainRoot(k);
    if (rootStr.isEmpty()) return "";

    std::vector<int> dbLocs = bestSig.doubleBondLocants;
    std::vector<int> tbLocs = bestSig.tripleBondLocants;

    QString infix;
    if (dbLocs.empty() && tbLocs.empty()) {
        infix = "an";
    } else if (!dbLocs.empty() && tbLocs.empty()) {
        if (dbLocs.size() == 1) {
            if (k <= 2) infix = "en";
            else infix = QString("-%1-en").arg(dbLocs[0]);
        } else {
            rootStr += "a";
            QStringList lStrs;
            for (int l : dbLocs) lStrs.append(QString::number(l));
            infix = QString("-%1-%2en").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
        }
    } else if (dbLocs.empty() && !tbLocs.empty()) {
        if (tbLocs.size() == 1) {
            if (k <= 2) infix = "yn";
            else infix = QString("-%1-yn").arg(tbLocs[0]);
        } else {
            rootStr += "a";
            QStringList lStrs;
            for (int l : tbLocs) lStrs.append(QString::number(l));
            infix = QString("-%1-%2yn").arg(lStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size())));
        }
    } else {
        QStringList dStrs, tStrs;
        for (int l : dbLocs) dStrs.append(QString::number(l));
        for (int l : tbLocs) tStrs.append(QString::number(l));

        QString dPart = (dbLocs.size() > 1) ? QString("%1-%2en").arg(dStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size()))) : QString("%1-en").arg(dStrs[0]);
        QString tPart = (tbLocs.size() > 1) ? QString("%1-%2yn").arg(tStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size()))) : QString("%1-yn").arg(tStrs[0]);
        infix = QString("-%1-%2").arg(dPart, tPart);
    }

    QString stem = rootStr + infix;
    QString sfx = "oic acid";

    QChar checkC;
    for (QChar ch : sfx) {
        if (ch.isLetter()) { checkC = ch; break; }
    }
    if (!isVowel(checkC)) {
        stem += "e";
    }

    return prefixPart + stem + sfx;
}


struct StereoResult {
    bool ok;
    QString prefix;
    QString error;
};

StereoResult formatStereoPrefix(
    const std::map<int, QChar> &stereoByGraphId,
    const std::map<int, int> &graphIdToLocant,
    const std::set<int> &handledBranchStereoIds = {})
{
    if (stereoByGraphId.empty()) {
        return {true, "", ""};
    }

    std::vector<std::pair<int, QChar>> locantStereo;
    for (const auto &pair : stereoByGraphId) {
        int graphId = pair.first;
        QChar letter = pair.second;
        if (handledBranchStereoIds.count(graphId)) {
            continue;
        }
        auto it = graphIdToLocant.find(graphId);
        if (it == graphIdToLocant.end()) {
            return {false, "", "Stereocenters on substituent branches are not supported in this phase."};
        }
        locantStereo.push_back({it->second, letter});
    }

    if (locantStereo.empty()) {
        return {true, "", ""};
    }

    std::sort(locantStereo.begin(), locantStereo.end(),
              [](const std::pair<int, QChar> &a, const std::pair<int, QChar> &b) {
                  return a.first < b.first;
              });

    QStringList parts;
    for (const auto &p : locantStereo) {
        parts.append(QString("%1%2").arg(p.first).arg(p.second));
    }

    QString prefix = QString("(%1)-").arg(parts.join(","));
    return {true, prefix, ""};
}

QString formatBranchStereoPrefix(
    const Graph &g,
    int rootIdx,
    int parentIdx,
    const std::map<int, QChar> &stereoByGraphId,
    std::set<int> &handledBranchStereoIds)
{
    std::vector<int> path;
    int curr = rootIdx;
    int prev = parentIdx;
    path.push_back(curr);

    while (true) {
        int nextC = -1;
        int maxDist = -1;

        for (size_t i = 0; i < g.nodes[curr].neighbors.size(); ++i) {
            int nei = g.nodes[curr].neighbors[i];
            if (nei == prev) continue;
            if (g.nodes[nei].atomicNumber == 6) {
                std::vector<int> stack = {nei};
                std::map<int, int> dist;
                dist[nei] = 1;
                dist[curr] = 0;
                int localMax = 1;
                while (!stack.empty()) {
                    int u = stack.back();
                    stack.pop_back();
                    int d = dist[u];
                    if (d > localMax) localMax = d;
                    for (int nxt : g.nodes[u].neighbors) {
                        if (g.nodes[nxt].atomicNumber == 6 && dist.find(nxt) == dist.end() && nxt != prev) {
                            dist[nxt] = d + 1;
                            stack.push_back(nxt);
                        }
                    }
                }
                if (localMax > maxDist) {
                    maxDist = localMax;
                    nextC = nei;
                }
            }
        }
        if (nextC == -1) break;
        prev = curr;
        curr = nextC;
        path.push_back(curr);
    }

    std::vector<std::pair<int, QChar>> locantStereo;
    for (size_t i = 0; i < path.size(); ++i) {
        int cNode = path[i];
        int locant = static_cast<int>(i + 1);
        auto it = stereoByGraphId.find(cNode);
        if (it != stereoByGraphId.end()) {
            locantStereo.push_back({locant, it->second});
            handledBranchStereoIds.insert(cNode);
        }
    }

    if (locantStereo.empty()) {
        return "";
    }

    std::sort(locantStereo.begin(), locantStereo.end(),
              [](const std::pair<int, QChar> &a, const std::pair<int, QChar> &b) {
                  return a.first < b.first;
              });

    QStringList parts;
    for (const auto &p : locantStereo) {
        parts.append(QString("%1%2").arg(p.first).arg(p.second));
    }

    return QString("(%1)-").arg(parts.join(","));
}

StereoResult processDoubleBondStereo(
    int mol,
    const Graph &g,
    const std::set<int> &ringNodeSet,
    const std::map<int, int> &graphIdToLocant,
    std::map<int, QChar> &stereoByGraphId)
{
    for (const auto &bond : g.bonds) {
        if (bond.order != 2) continue;

        int u = bond.u;
        int v = bond.v;

        if (g.nodes[u].atomicNumber != 6 || g.nodes[v].atomicNumber != 6) {
            continue;
        }

        // 1. Ring double bonds: skip silently
        if (ringNodeSet.count(u) > 0 || ringNodeSet.count(v) > 0) {
            continue;
        }

        // 2. Count non-H heavy atom neighbors excluding the double-bond partner
        std::vector<int> uNeighbors;
        for (int nei : g.nodes[u].neighbors) {
            if (nei != v) uNeighbors.push_back(nei);
        }
        std::vector<int> vNeighbors;
        for (int nei : g.nodes[v].neighbors) {
            if (nei != u) vNeighbors.push_back(nei);
        }

        int deg_u = static_cast<int>(uNeighbors.size());
        int deg_v = static_cast<int>(vNeighbors.size());

        // Terminal alkene (=CH2): skip silently (not stereogenic)
        if (deg_u == 0 || deg_v == 0) {
            continue;
        }

        // Determine high-priority substituent on u and v (by atomic number of first atom)
        auto findHighPrioritySubstituent = [&](const std::vector<int> &neighbors, int &chosenSub) -> StereoResult {
            if (neighbors.size() == 1) {
                chosenSub = neighbors[0];
                return {true, "", ""};
            } else if (neighbors.size() == 2) {
                int a = neighbors[0];
                int b = neighbors[1];
                int zA = g.nodes[a].atomicNumber;
                int zB = g.nodes[b].atomicNumber;
                if (zA > zB) {
                    chosenSub = a;
                    return {true, "", ""};
                } else if (zB > zA) {
                    chosenSub = b;
                    return {true, "", ""};
                } else {
                    return {false, "", "E/Z determination requires comparing substituents beyond the first atom, which is not supported in this phase."};
                }
            }
            return {false, "", "E/Z determination for trisubstituted or tetrasubstituted double bonds is not supported in this phase."};
        };

        int r1 = -1, r2 = -1;
        StereoResult subResU = findHighPrioritySubstituent(uNeighbors, r1);
        if (!subResU.ok) {
            return subResU;
        }

        StereoResult subResV = findHighPrioritySubstituent(vNeighbors, r2);
        if (!subResV.ok) {
            return subResV;
        }

        // 3. Check if double bond carbons are on main chain/ring
        auto it_u = graphIdToLocant.find(u);
        auto it_v = graphIdToLocant.find(v);
        if (it_u == graphIdToLocant.end() || it_v == graphIdToLocant.end()) {
            return {false, "", "E/Z descriptors on substituent branches are not supported in this phase."};
        }

        int loc_u = it_u->second;
        int loc_v = it_v->second;
        int targetGraphId = (loc_u < loc_v) ? u : v;

        // 4. Retrieve 2D coordinates for u, v, r1, r2
        int handle_u = indigoGetAtom(mol, g.nodes[u].indigoIdx);
        int handle_v = indigoGetAtom(mol, g.nodes[v].indigoIdx);
        int handle_r1 = indigoGetAtom(mol, g.nodes[r1].indigoIdx);
        int handle_r2 = indigoGetAtom(mol, g.nodes[r2].indigoIdx);

        float ux = 0, uy = 0, vx = 0, vy = 0, r1x = 0, r1y = 0, r2x = 0, r2y = 0;
        bool coordsOk = true;

        if (handle_u > 0) {
            float* p = indigoXYZ(handle_u);
            if (p) { ux = p[0]; uy = p[1]; } else coordsOk = false;
            indigoFree(handle_u);
        } else coordsOk = false;

        if (handle_v > 0) {
            float* p = indigoXYZ(handle_v);
            if (p) { vx = p[0]; vy = p[1]; } else coordsOk = false;
            indigoFree(handle_v);
        } else coordsOk = false;

        if (handle_r1 > 0) {
            float* p = indigoXYZ(handle_r1);
            if (p) { r1x = p[0]; r1y = p[1]; } else coordsOk = false;
            indigoFree(handle_r1);
        } else coordsOk = false;

        if (handle_r2 > 0) {
            float* p = indigoXYZ(handle_r2);
            if (p) { r2x = p[0]; r2y = p[1]; } else coordsOk = false;
            indigoFree(handle_r2);
        } else coordsOk = false;

        if (!coordsOk) {
            return {false, "", "E/Z geometry could not be determined for this structure (missing or degenerate coordinates)."};
        }

        float ab_x = vx - ux;
        float ab_y = vy - uy;

        float side1 = ab_x * (r1y - uy) - ab_y * (r1x - ux);
        float side2 = ab_x * (r2y - uy) - ab_y * (r2x - ux);

        const float EPSILON = 1e-6f;
        if (std::abs(side1) < EPSILON || std::abs(side2) < EPSILON) {
            return {false, "", "E/Z geometry could not be determined for this structure (missing or degenerate coordinates)."};
        }

        bool sameSide = (side1 > 0 && side2 > 0) || (side1 < 0 && side2 < 0);
        QChar ezLetter = sameSide ? 'Z' : 'E';

        stereoByGraphId[targetGraphId] = ezLetter;
    }

    return {true, "", ""};
}

} // anonymous namespace

IupacResult IupacNamer::generateName(int mol) {
    if (mol < 0) {
        return {false, "", "Invalid molecule handle."};
    }

    // Aromatize molecule first so aromatic bonds are reliably order 4
    indigoAromatize(mol);

    int ringCount = indigoCountSSSR(mol);

    if (indigoCountComponents(mol) > 1) {
        return {false, "", "Multi-component structures are not supported in Phase 1."};
    }

    // Iterate atoms & check unsupported features
    int atomIter = indigoIterateAtoms(mol);
    if (atomIter < 0) {
        return {false, "", "Failed to iterate molecule atoms."};
    }

    std::map<int, int> indigoToGraphIdx;
    std::vector<int> heavyAtomIndices;
    std::map<int, int> explicitHCounts;

    int atomHandle = 0;
    while ((atomHandle = indigoNext(atomIter)) != 0) {
        int idx = indigoIndex(atomHandle);

        int charge = 0;
        if (indigoGetCharge(atomHandle, &charge) == 1 && charge != 0) {
            int z = indigoAtomicNumber(atomHandle);
            bool isExemptCharge = false;
            if (z == 7 && charge == 1) {
                int neiCount = 0, cCount = 0, oZeroDblCount = 0, oNegSglCount = 0;
                int neiIter = indigoIterateNeighbors(atomHandle);
                int nei = 0;
                while ((nei = indigoNext(neiIter)) != 0) {
                    neiCount++;
                    int nZ = indigoAtomicNumber(nei);
                    int nCharge = 0;
                    indigoGetCharge(nei, &nCharge);
                    int bondHandle = indigoBond(nei);
                    int order = indigoBondOrder(bondHandle);
                    indigoFree(bondHandle);

                    if (nZ == 6 && nCharge == 0 && order == 1) cCount++;
                    else if (nZ == 8 && nCharge == 0 && order == 2) oZeroDblCount++;
                    else if (nZ == 8 && nCharge == -1 && order == 1) oNegSglCount++;
                    indigoFree(nei);
                }
                indigoFree(neiIter);
                if (neiCount == 3 && cCount == 1 && oZeroDblCount == 1 && oNegSglCount == 1) {
                    isExemptCharge = true;
                }

                if (!isExemptCharge) {
                    int aNeiCount = 0, nZeroDblCount = 0, nNegDblCount = 0;
                    int aNeiIter = indigoIterateNeighbors(atomHandle);
                    int aNei = 0;
                    while ((aNei = indigoNext(aNeiIter)) != 0) {
                        aNeiCount++;
                        int nZ = indigoAtomicNumber(aNei);
                        int nCharge = 0;
                        indigoGetCharge(aNei, &nCharge);
                        int bondHandle = indigoBond(aNei);
                        int order = indigoBondOrder(bondHandle);
                        indigoFree(bondHandle);

                        if (nZ == 7 && nCharge == 0 && order == 2) nZeroDblCount++;
                        else if (nZ == 7 && nCharge == -1 && order == 2) nNegDblCount++;
                        indigoFree(aNei);
                    }
                    indigoFree(aNeiIter);
                    if (aNeiCount == 2 && nZeroDblCount == 1 && nNegDblCount == 1) {
                        isExemptCharge = true;
                    }
                }
            } else if (z == 8 && charge == -1) {
                int neiCount = 0, nPosCount = 0;
                int neiIter = indigoIterateNeighbors(atomHandle);
                int nei = 0;
                while ((nei = indigoNext(neiIter)) != 0) {
                    neiCount++;
                    int nZ = indigoAtomicNumber(nei);
                    int nCharge = 0;
                    indigoGetCharge(nei, &nCharge);
                    int bondHandle = indigoBond(nei);
                    int order = indigoBondOrder(bondHandle);
                    indigoFree(bondHandle);

                    if (nZ == 7 && nCharge == 1 && order == 1) nPosCount++;
                    indigoFree(nei);
                }
                indigoFree(neiIter);
                if (neiCount == 1 && nPosCount == 1) {
                    isExemptCharge = true;
                }
            } else if (z == 7 && charge == -1) {
                int neiCount = 0, nPosDblCount = 0;
                int neiIter = indigoIterateNeighbors(atomHandle);
                int nei = 0;
                while ((nei = indigoNext(neiIter)) != 0) {
                    neiCount++;
                    int nZ = indigoAtomicNumber(nei);
                    int nCharge = 0;
                    indigoGetCharge(nei, &nCharge);
                    int bondHandle = indigoBond(nei);
                    int order = indigoBondOrder(bondHandle);
                    indigoFree(bondHandle);

                    if (nZ == 7 && nCharge == 1 && order == 2) nPosDblCount++;
                    indigoFree(nei);
                }
                indigoFree(neiIter);
                if (neiCount == 1 && nPosDblCount == 1) {
                    isExemptCharge = true;
                }
            }

            if (!isExemptCharge) {
                indigoFree(atomHandle);
                indigoFree(atomIter);
                return {false, "", "Charged atoms are not supported in Phase 1."};
            }
        }

        int rad = 0;
        if (indigoGetRadicalElectrons(atomHandle, &rad) == 1 && rad > 0) {
            indigoFree(atomHandle);
            indigoFree(atomIter);
            return {false, "", "Radicals are not supported in Phase 1."};
        }

        int iso = indigoIsotope(atomHandle);
        if (iso > 0) {
            indigoFree(atomHandle);
            indigoFree(atomIter);
            return {false, "", "Isotopic labeling is not supported in Phase 1."};
        }

        int z = indigoAtomicNumber(atomHandle);
        if (z == 0) {
            indigoFree(atomHandle);
            indigoFree(atomIter);
            return {false, "", "Pseudoatoms and R-sites are not supported in Phase 1."};
        }

        if (z == 1) {
            int neiIter = indigoIterateNeighbors(atomHandle);
            if (neiIter >= 0) {
                int nei = 0;
                while ((nei = indigoNext(neiIter)) != 0) {
                    int neiIdx = indigoIndex(nei);
                    explicitHCounts[neiIdx]++;
                    indigoFree(nei);
                }
                indigoFree(neiIter);
            }
        } else if (z == 6 || z == 7 || z == 8 || z == 16 || z == 15 || z == 5 || z == 9 || z == 17 || z == 35 || z == 53 || z == 34 || z == 52) {
            heavyAtomIndices.push_back(idx);
        } else {
            indigoFree(atomHandle);
            indigoFree(atomIter);
            return {false, "", QString("Unsupported element (Z=%1) for Phase 1.").arg(z)};
        }
        indigoFree(atomHandle);
    }
    indigoFree(atomIter);

    if (heavyAtomIndices.empty()) {
        return {false, "", "No heavy atoms found in structure."};
    }

    Graph g;
    for (size_t i = 0; i < heavyAtomIndices.size(); ++i) {
        int idx = heavyAtomIndices[i];
        indigoToGraphIdx[idx] = static_cast<int>(i);

        int atomObj = indigoGetAtom(mol, idx);
        int z = indigoAtomicNumber(atomObj);

        int implicitH = indigoCountImplicitHydrogens(atomObj);
        int expH = explicitHCounts[idx];

        GraphNode node;
        node.id = static_cast<int>(i);
        node.indigoIdx = idx;
        node.atomicNumber = z;
        node.totalH = implicitH + expH;
        g.nodes.push_back(node);
        indigoFree(atomObj);
    }

    int bondIter = indigoIterateBonds(mol);
    if (bondIter >= 0) {
        int bondHandle = 0;
        while ((bondHandle = indigoNext(bondIter)) != 0) {
            int srcHandle = indigoSource(bondHandle);
            int dstHandle = indigoDestination(bondHandle);
            int src = indigoIndex(srcHandle);
            int dst = indigoIndex(dstHandle);
            int order = indigoBondOrder(bondHandle);

            if (indigoToGraphIdx.count(src) && indigoToGraphIdx.count(dst)) {
                int u = indigoToGraphIdx[src];
                int v = indigoToGraphIdx[dst];

                g.nodes[u].neighbors.push_back(v);
                g.nodes[u].bondOrders.push_back(order);
                g.nodes[v].neighbors.push_back(u);
                g.nodes[v].bondOrders.push_back(order);

                GraphBond gb;
                gb.u = u;
                gb.v = v;
                gb.order = order;
                g.bonds.push_back(gb);
            }
            indigoFree(srcHandle);
            indigoFree(dstHandle);
            indigoFree(bondHandle);
        }
        indigoFree(bondIter);
    }


    for (size_t i = 0; i < g.nodes.size(); ++i) {
        if (g.nodes[i].atomicNumber != 6) continue;
        int dblCarbonNeiCount = 0;
        for (size_t j = 0; j < g.nodes[i].neighbors.size(); ++j) {
            if (g.nodes[i].bondOrders[j] == 2) {
                int nei = g.nodes[i].neighbors[j];
                if (g.nodes[nei].atomicNumber == 6) {
                    dblCarbonNeiCount++;
                }
            }
        }
        if (dblCarbonNeiCount >= 2) {
            return {false, "", "Allenes and cumulated double bonds are not supported in this phase."};
        }
    }

    // Ensure 2D coordinates are calculated for stereochemistry determination
    indigoLayout(mol);

    indigoAddCIPStereoDescriptors(mol);
    std::map<int, QChar> stereoByGraphId;
    int stereoIter = indigoIterateStereocenters(mol);
    if (stereoIter >= 0) {
        int atom = 0;
        while ((atom = indigoNext(stereoIter)) != 0) {
            if (atom == -1) break;
            int idx = indigoIndex(atom);
            int cip = indigoStereocenterCIPDescriptor(atom);
            if (cip == 2 || cip == 3) {
                indigoFree(atom);
                indigoFree(stereoIter);
                return {false, "", "Pseudo-asymmetric stereocenters are not supported in this phase."};
            } else if (cip == 4) {
                if (indigoToGraphIdx.count(idx)) {
                    stereoByGraphId[indigoToGraphIdx[idx]] = 'S';
                }
            } else if (cip == 5) {
                if (indigoToGraphIdx.count(idx)) {
                    stereoByGraphId[indigoToGraphIdx[idx]] = 'R';
                }
            }
            indigoFree(atom);
        }
        indigoFree(stereoIter);
    }

    std::map<int, int> carbonSulfonicAcid; // carbonNode -> sulfurNode
    std::map<int, int> carbonThiol;        // carbonNode -> sulfurNode
    std::set<int> thioetherSulfurs;       // sulfurNodes
    std::set<int> sulfoxideSulfurs;       // sulfurNodes
    std::set<int> sulfoneSulfurs;         // sulfurNodes
    std::set<int> disulfideSulfurs;       // sulfurNodes
    std::map<int, std::vector<int>> carbonAzide;      // carbonNode -> vector of azide N1 nodes
    std::map<int, int> carbonPhosphine;   // carbonNode -> phosphorusNode
    std::map<int, int> carbonBoronicAcid; // carbonNode -> boronNode

    for (size_t i = 0; i < g.nodes.size(); ++i) {
        const GraphNode &node = g.nodes[i];
        if (node.atomicNumber == 16) {
            int dblO = 0, sglO_OH = 0, sglC = 0, dblC = 0, sglS = 0, sglN = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;

                if (nZ == 8 && order == 2) dblO++;
                else if (nZ == 8 && order == 1 && (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) sglO_OH++;
                else if (nZ == 6 && order == 2) {
                    dblC++;
                } else if (nZ == 6) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 7) {
                    sglN++;
                } else if (nZ == 16 && order == 1) {
                    sglS++;
                }
            }

            if (dblO == 2 && sglO_OH == 1 && sglC == 1 && node.neighbors.size() == 4) {
                carbonSulfonicAcid[cNeighbors[0]] = static_cast<int>(i);
            } else if (sglC == 1 && node.neighbors.size() == 1 && node.totalH >= 1) {
                carbonThiol[cNeighbors[0]] = static_cast<int>(i);
            } else if ((sglC + sglN) == 2 && dblO == 0 && sglO_OH == 0 && sglS == 0 && node.neighbors.size() == 2) {
                thioetherSulfurs.insert(static_cast<int>(i));
            } else if (dblO == 1 && sglC == 2 && node.neighbors.size() == 3) {
                sulfoxideSulfurs.insert(static_cast<int>(i));
            } else if (dblO == 2 && sglC == 2 && node.neighbors.size() == 4 && sglO_OH == 0) {
                sulfoneSulfurs.insert(static_cast<int>(i));
            } else if (sglS == 1 && sglC == 1 && dblO == 0 && sglO_OH == 0 && node.neighbors.size() == 2) {
                disulfideSulfurs.insert(static_cast<int>(i));
            } else if (dblC == 1 && node.neighbors.size() == 1) {
                // Thiocarbonyl sulfur (C=S)
            } else {
                return {false, "", "Sulfur-containing groups other than thiols, thioethers, and sulfonic acids are not supported in this phase."};
            }
        } else if (node.atomicNumber == 7) {
            int sglC = -1;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) sglC = nei;
            }
            if (sglC != -1 && node.totalH == 0 && node.neighbors.size() == 2) {
                int midN = -1;
                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    if (g.nodes[nei].atomicNumber == 7 && order == 2) {
                        midN = nei;
                    }
                }
                if (midN != -1 && g.nodes[midN].totalH == 0 && g.nodes[midN].neighbors.size() == 2) {
                    const GraphNode &mNode = g.nodes[midN];
                    int termN = -1;
                    for (size_t k = 0; k < mNode.neighbors.size(); ++k) {
                        int mNei = mNode.neighbors[k];
                        int mOrder = mNode.bondOrders[k];
                        if (mNei != static_cast<int>(i) && g.nodes[mNei].atomicNumber == 7 && mOrder == 2) {
                            termN = mNei;
                        }
                    }
                    if (termN != -1 && g.nodes[termN].neighbors.size() == 1 && g.nodes[termN].totalH == 0) {
                        carbonAzide[sglC].push_back(static_cast<int>(i));
                    }
                }
            }
        } else if (node.atomicNumber == 15) {
            bool inAromaticRing = false;
            for (int order : node.bondOrders) {
                if (order == 4) inAromaticRing = true;
            }
            if (inAromaticRing) continue;

            int sglC = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                }
            }
            if (sglC == 1 && node.neighbors.size() == 1 && node.totalH >= 1) {
                carbonPhosphine[cNeighbors[0]] = static_cast<int>(i);
            } else {
                return {false, "", "Phosphorus-containing groups other than phosphine are not supported in this phase."};
            }
        } else if (node.atomicNumber == 5) {
            int sglC = 0, sglO_OH = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 8 && order == 1 && (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) {
                    sglO_OH++;
                }
            }
            if (sglC == 1 && sglO_OH == 2 && node.neighbors.size() == 3) {
                carbonBoronicAcid[cNeighbors[0]] = static_cast<int>(i);
            } else {
                return {false, "", "Boron-containing groups other than boronic acid are not supported in this phase."};
            }
        }
    }

    struct RingSubstituentInfo {
        std::set<int> ringNodes;
        QString prefixName;
        int attachmentChainCarbon = -1;
    };
    std::vector<RingSubstituentInfo> ringSubstituentInfos;
    std::set<int> allRingSubstituentNodes;

    std::set<int> allSSSRNodes;
    std::vector<std::set<int>> allSSSRRings;
    if (ringCount > 0) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rNodes;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int atomHandle = 0;
                    while ((atomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(atomHandle);
                        if (indigoToGraphIdx.count(idx)) {
                            int gIdx = indigoToGraphIdx[idx];
                            allSSSRNodes.insert(gIdx);
                            rNodes.insert(gIdx);
                        }
                        indigoFree(atomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                allSSSRRings.push_back(rNodes);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);
        }
    }

    if (ringCount == 2 && allSSSRRings.size() == 2) {
        const std::set<int> &ring1Nodes = allSSSRRings[0];
        const std::set<int> &ring2Nodes = allSSSRRings[1];
        if (ring1Nodes.size() == 6 && ring2Nodes.size() == 6) {
            std::vector<int> sharedNodes;
            for (int n : ring1Nodes) {
                if (ring2Nodes.count(n)) sharedNodes.push_back(n);
            }
            if (sharedNodes.empty()) {
                auto checkRingAssemblyOneSide = [&](const std::set<int> &rNodes, const std::set<int> &otherRNodes) -> QString {
                    if (rNodes.size() != 6) return "";
                    for (int idx : rNodes) {
                        if (g.nodes[idx].atomicNumber != 6) return "";
                    }
                    bool allAromatic = true;
                    bool allSingle = true;
                    bool hasDouble = false;
                    bool hasTriple = false;
                    for (const auto &gb : g.bonds) {
                        if (rNodes.count(gb.u) && rNodes.count(gb.v)) {
                            if (gb.order != 4) allAromatic = false;
                            if (gb.order != 1) allSingle = false;
                            if (gb.order == 2) hasDouble = true;
                            if (gb.order == 3) hasTriple = true;
                        }
                    }
                    bool isBenzene = (!hasTriple) && (allAromatic || (!allSingle && !hasDouble));
                    bool isCyclohexane = allSingle && (!hasDouble) && (!hasTriple) && (!allAromatic);
                    if (!isBenzene && !isCyclohexane) return "";

                    int nodesWithExo = 0;
                    bool validSubstituent = true;
                    for (int rNode : rNodes) {
                        int exoCount = 0;
                        for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                            int nei = g.nodes[rNode].neighbors[j];
                            if (!rNodes.count(nei)) {
                                exoCount++;
                                if (!otherRNodes.count(nei) || g.nodes[rNode].bondOrders[j] != 1) {
                                    validSubstituent = false;
                                }
                            }
                        }
                        if (exoCount > 1) {
                            validSubstituent = false;
                        } else if (exoCount == 1) {
                            nodesWithExo++;
                        }
                    }
                    if (!validSubstituent || nodesWithExo != 1) return "";
                    return isBenzene ? "phenyl" : "cyclohexyl";
                };

                QString type1 = checkRingAssemblyOneSide(ring1Nodes, ring2Nodes);
                QString type2 = checkRingAssemblyOneSide(ring2Nodes, ring1Nodes);
                if (!type1.isEmpty() && !type2.isEmpty() && type1 == type2) {
                    return {true, "bi" + type1, ""};
                }
            }
        }
    }

    for (const auto &rNodes : allSSSRRings) {
        bool isFusedOrBridged = false;
        for (const auto &otherR : allSSSRRings) {
            if (&otherR != &rNodes) {
                for (int n : rNodes) {
                    if (otherR.count(n)) { isFusedOrBridged = true; break; }
                }
            }
            if (isFusedOrBridged) break;
        }

        if (isFusedOrBridged) {
            continue;
        }

        int foundAttachChainNode = -1;
        int attachRingNode = -1;
        int mainChainExoCount = 0;
        bool validSubstituent = true;

        for (int rNode : rNodes) {
            for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                int nei = g.nodes[rNode].neighbors[j];
                if (!rNodes.count(nei)) {
                    bool isOtherRingDirect = false;
                    for (const auto &otherR : allSSSRRings) {
                        if (&otherR != &rNodes && otherR.count(nei)) {
                            isOtherRingDirect = true;
                            break;
                        }
                    }
                    if (!isOtherRingDirect) {
                        int nZ = g.nodes[nei].atomicNumber;
                        bool isChainC = (nZ == 6 && !allSSSRNodes.count(nei));
                        bool isEsterO = false;
                        if (nZ == 8 && g.nodes[rNode].bondOrders[j] == 1) {
                            for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                                int oNei = g.nodes[nei].neighbors[k];
                                if (oNei != rNode && g.nodes[oNei].atomicNumber == 6 && !allSSSRNodes.count(oNei)) {
                                    for (size_t l = 0; l < g.nodes[oNei].neighbors.size(); ++l) {
                                        if (g.nodes[oNei].bondOrders[l] == 2 && g.nodes[g.nodes[oNei].neighbors[l]].atomicNumber == 8) {
                                            isEsterO = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if (isChainC) {
                            bool isDirectExocyclicGroup = false;
                            const GraphNode &attachNode = g.nodes[nei];
                            bool hasDoubleO = false, hasSingleO = false, hasSingleN = false, hasTripleN = false, hasHalogen = false;
                            for (size_t k = 0; k < attachNode.neighbors.size(); ++k) {
                                int aNei = attachNode.neighbors[k];
                                int order = attachNode.bondOrders[k];
                                int aZ = g.nodes[aNei].atomicNumber;
                                if (aZ == 8 && order == 2) hasDoubleO = true;
                                else if (aZ == 8 && order == 1) hasSingleO = true;
                                else if (aZ == 7 && order == 1) hasSingleN = true;
                                else if (aZ == 7 && order == 3) hasTripleN = true;
                                else if (aZ == 9 || aZ == 17 || aZ == 35 || aZ == 53) hasHalogen = true;
                            }
                            if ((hasDoubleO && hasSingleO) || (hasDoubleO && hasHalogen) || (hasDoubleO && hasSingleN) || hasTripleN || (hasDoubleO && attachNode.totalH >= 1)) {
                                isDirectExocyclicGroup = true;
                            }

                            if (!isDirectExocyclicGroup) {
                                foundAttachChainNode = nei;
                                attachRingNode = rNode;
                                mainChainExoCount++;
                            }
                        } else if (isEsterO) {
                            foundAttachChainNode = nei;
                            attachRingNode = rNode;
                            mainChainExoCount++;
                        }
                    } else {
                        validSubstituent = false;
                    }
                }
            }
        }

        if (validSubstituent && mainChainExoCount == 1 && foundAttachChainNode != -1 && attachRingNode != -1) {
            bool hasPrincipalGroupOrMultipleRings = (allSSSRRings.size() > 1);
            if (!hasPrincipalGroupOrMultipleRings) {
                std::queue<int> q;
                std::set<int> visited;
                q.push(foundAttachChainNode);
                visited.insert(foundAttachChainNode);
                visited.insert(attachRingNode);

                while (!q.empty()) {
                    int curr = q.front();
                    q.pop();
                    const GraphNode &node = g.nodes[curr];
                    if (node.atomicNumber != 6 && node.atomicNumber != 1) {
                        hasPrincipalGroupOrMultipleRings = true;
                        break;
                    }
                    for (int nei : node.neighbors) {
                        if (!visited.count(nei) && !rNodes.count(nei)) {
                            visited.insert(nei);
                            q.push(nei);
                        }
                    }
                }
            }

            if (hasPrincipalGroupOrMultipleRings) {
                QString pName = nameRingAsSubstituent(g, rNodes, attachRingNode, foundAttachChainNode, allSSSRRings);
                if (!pName.isEmpty()) {
                    ringSubstituentInfos.push_back({rNodes, pName, foundAttachChainNode});
                }
            }
        }
    }

    if (allSSSRRings.size() == 2) {
        for (size_t i = 0; i < allSSSRRings.size(); ++i) {
            for (size_t j = i + 1; j < allSSSRRings.size(); ++j) {
                const auto &r1 = allSSSRRings[i];
                const auto &r2 = allSSSRRings[j];
                if (r1.size() == 6 && r2.size() == 6) {
                    std::vector<int> sharedNodes;
                    for (int n : r1) if (r2.count(n)) sharedNodes.push_back(n);
                    if (sharedNodes.size() == 2) {
                        int bhA = sharedNodes[0];
                        int bhB = sharedNodes[1];
                        bool bhBonded = false;
                        for (size_t k = 0; k < g.nodes[bhA].neighbors.size(); ++k) {
                            if (g.nodes[bhA].neighbors[k] == bhB) {
                                bhBonded = true; break;
                            }
                        }
                        if (bhBonded) {
                            std::set<int> naphNodes = r1;
                            for (int n : r2) naphNodes.insert(n);
                            bool allCarbon = true;
                            for (int n : naphNodes) {
                                if (g.nodes[n].atomicNumber != 6) { allCarbon = false; break; }
                            }
                            bool allAromatic = true;
                            for (const auto &gb : g.bonds) {
                                if (naphNodes.count(gb.u) && naphNodes.count(gb.v)) {
                                    if (gb.order != 4) { allAromatic = false; break; }
                                }
                            }
                            if (allCarbon && allAromatic && naphNodes.size() == 10) {
                                int foundAttachChainNode = -1;
                                int attachRingNode = -1;
                                int mainChainExoCount = 0;
                                bool validSubstituent = true;

                                for (int rNode : naphNodes) {
                                    for (size_t k = 0; k < g.nodes[rNode].neighbors.size(); ++k) {
                                        int nei = g.nodes[rNode].neighbors[k];
                                        if (!naphNodes.count(nei)) {
                                            int nZ = g.nodes[nei].atomicNumber;
                                            bool isChainC = (nZ == 6 && !allSSSRNodes.count(nei));
                                            if (isChainC) {
                                                bool isDirectExocyclicGroup = false;
                                                const GraphNode &attachNode = g.nodes[nei];
                                                bool hasDoubleO = false, hasSingleO = false, hasSingleN = false, hasTripleN = false, hasHalogen = false;
                                                for (size_t m = 0; m < attachNode.neighbors.size(); ++m) {
                                                    int aNei = attachNode.neighbors[m];
                                                    int order = attachNode.bondOrders[m];
                                                    int aZ = g.nodes[aNei].atomicNumber;
                                                    if (aZ == 8 && order == 2) hasDoubleO = true;
                                                    else if (aZ == 8 && order == 1) hasSingleO = true;
                                                    else if (aZ == 7 && order == 1) hasSingleN = true;
                                                    else if (aZ == 7 && order == 3) hasTripleN = true;
                                                    else if (aZ == 9 || aZ == 17 || aZ == 35 || aZ == 53) hasHalogen = true;
                                                }
                                                if ((hasDoubleO && hasSingleO) || (hasDoubleO && hasHalogen) || (hasDoubleO && hasSingleN) || hasTripleN || (hasDoubleO && attachNode.totalH >= 1)) {
                                                    isDirectExocyclicGroup = true;
                                                }
                                                if (!isDirectExocyclicGroup) {
                                                    foundAttachChainNode = nei;
                                                    attachRingNode = rNode;
                                                    mainChainExoCount++;
                                                } else {
                                                    validSubstituent = false;
                                                }
                                            } else {
                                                validSubstituent = false;
                                            }
                                        }
                                    }
                                }

                                if (validSubstituent && mainChainExoCount == 1 && foundAttachChainNode != -1 && attachRingNode != -1) {
                                    bool is1Type = false;
                                    for (int nei : g.nodes[attachRingNode].neighbors) {
                                        if (nei == bhA || nei == bhB) {
                                            is1Type = true; break;
                                        }
                                    }
                                    QString pName = is1Type ? "(naphthalen-1-yl)" : "(naphthalen-2-yl)";
                                    ringSubstituentInfos.push_back({naphNodes, pName, foundAttachChainNode});
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (const auto &info : ringSubstituentInfos) {
        for (int n : info.ringNodes) {
            allRingSubstituentNodes.insert(n);
        }
    }

    // Branch to Phase 1 (acyclic) if ringCount == 0 or ring-substituted chain
    if (ringCount == 0 || !ringSubstituentInfos.empty()) {

        std::map<int, std::vector<int>> carbonNitro;      // carbonNode -> vector of nitro N nodes
        std::map<int, std::vector<int>> carbonIsocyanate; // carbonNode -> vector of isocyanate N nodes
        std::set<int> isocyanateCarbons;

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            const GraphNode &node = g.nodes[i];
            if (node.atomicNumber == 7) {
                int sglC = -1, dblC_iso = -1;
                std::vector<int> oxygens;
                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    int nZ = g.nodes[nei].atomicNumber;
                    if (nZ == 6 && order == 1) sglC = nei;
                    else if (nZ == 6 && order == 2) dblC_iso = nei;
                    else if (nZ == 8) oxygens.push_back(nei);
                }

                if (sglC != -1 && oxygens.size() == 2 && node.totalH == 0) {
                    carbonNitro[sglC].push_back(static_cast<int>(i));
                } else if (sglC != -1 && dblC_iso != -1 && node.totalH == 0 && node.neighbors.size() == 2) {
                    bool isoOk = false;
                    const GraphNode &isoNode = g.nodes[dblC_iso];
                    if (isoNode.neighbors.size() == 2) {
                        for (size_t k = 0; k < isoNode.neighbors.size(); ++k) {
                            int isoNei = isoNode.neighbors[k];
                            int isoOrder = isoNode.bondOrders[k];
                            if (g.nodes[isoNei].atomicNumber == 8 && isoOrder == 2 && g.nodes[isoNei].neighbors.size() == 1) {
                                isoOk = true;
                            }
                        }
                    }
                    if (isoOk) {
                        carbonIsocyanate[sglC].push_back(static_cast<int>(i));
                        isocyanateCarbons.insert(dblC_iso);
                    }
                }
            }
        }

        // Functional-group pattern matching (Phase 1 & Phase 6)
        std::map<int, GroupType> carbonGroup;
        std::map<int, int> acylHalideHalogen;
        std::map<int, int> esterAlkylRoot;
        std::map<int, int> esterOxygen;
        std::set<int> etherOxygens;

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            const GraphNode &node = g.nodes[i];
            if (node.atomicNumber == 6) {
                if (isocyanateCarbons.count(static_cast<int>(i))) continue;
                std::vector<int> doubleO, singleO, singleN, tripleN, halogens, doubleS;

                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    int nZ = g.nodes[nei].atomicNumber;

                    if (nZ == 8 && order == 2) doubleO.push_back(nei);
                    else if (nZ == 8 && order == 1) singleO.push_back(nei);
                    else if (nZ == 7 && order == 1) {
                        bool isNitroIsoOrAzide = false;
                        if (carbonNitro.count(i) && std::find(carbonNitro[i].begin(), carbonNitro[i].end(), nei) != carbonNitro[i].end()) isNitroIsoOrAzide = true;
                        if (carbonIsocyanate.count(i) && std::find(carbonIsocyanate[i].begin(), carbonIsocyanate[i].end(), nei) != carbonIsocyanate[i].end()) isNitroIsoOrAzide = true;
                        if (carbonAzide.count(i) && std::find(carbonAzide[i].begin(), carbonAzide[i].end(), nei) != carbonAzide[i].end()) isNitroIsoOrAzide = true;
                        if (!isNitroIsoOrAzide) singleN.push_back(nei);
                    }
                    else if (nZ == 7 && order == 3) tripleN.push_back(nei);
                    else if ((nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) && order == 1) halogens.push_back(nei);
                    else if (nZ == 16 && order == 2) doubleS.push_back(nei);
                }

                if (!doubleO.empty()) {
                    for (int sO : singleO) {
                        int cCount = 0;
                        bool isAnhydride = false;
                        int alkylRootNode = -1;
                        for (size_t k = 0; k < g.nodes[sO].neighbors.size(); ++k) {
                            int oNei = g.nodes[sO].neighbors[k];
                            if (g.nodes[oNei].atomicNumber == 6) {
                                cCount++;
                                if (oNei != static_cast<int>(i)) {
                                    alkylRootNode = oNei;
                                    for (size_t l = 0; l < g.nodes[oNei].neighbors.size(); ++l) {
                                        int oNeiNei = g.nodes[oNei].neighbors[l];
                                        if (g.nodes[oNeiNei].atomicNumber == 8 && g.nodes[oNei].bondOrders[l] == 2) {
                                            isAnhydride = true;
                                        }
                                    }
                                }
                            }
                        }
                        if (isAnhydride) {
                            auto getPlainChainLength = [&](int startC, int bridgeO) -> int {
                                int count = 0;
                                int curr = startC;
                                int prev = bridgeO;

                                while (true) {
                                    count++;
                                    const GraphNode &node = g.nodes[curr];
                                    if (node.atomicNumber != 6) return -1;

                                    if (curr == startC) {
                                        int doubleOCount = 0;
                                        int nextC = -1;
                                        int nextCCount = 0;

                                        for (size_t j = 0; j < node.neighbors.size(); ++j) {
                                            int nei = node.neighbors[j];
                                            int order = node.bondOrders[j];
                                            int nZ = g.nodes[nei].atomicNumber;

                                            if (nei == bridgeO) {
                                                if (nZ != 8 || order != 1) return -1;
                                                continue;
                                            }
                                            if (nZ == 8 && order == 2) {
                                                doubleOCount++;
                                            } else if (nZ == 6 && order == 1) {
                                                nextC = nei;
                                                nextCCount++;
                                            } else {
                                                return -1;
                                            }
                                        }

                                        if (doubleOCount != 1) return -1;
                                        if (nextCCount > 1) return -1;

                                        if (nextCCount == 0) {
                                            return count;
                                        }
                                        prev = curr;
                                        curr = nextC;
                                    } else {
                                        int nextC = -1;
                                        int nextCCount = 0;

                                        for (size_t j = 0; j < node.neighbors.size(); ++j) {
                                            int nei = node.neighbors[j];
                                            int order = node.bondOrders[j];
                                            int nZ = g.nodes[nei].atomicNumber;

                                            if (nei == prev) {
                                                if (order != 1) return -1;
                                                continue;
                                            }

                                            if (nZ == 6 && order == 1) {
                                                nextC = nei;
                                                nextCCount++;
                                            } else {
                                                return -1;
                                            }
                                        }

                                        if (nextCCount > 1) return -1;

                                        if (nextCCount == 0) {
                                            return count;
                                        }
                                        prev = curr;
                                        curr = nextC;
                                    }
                                }
                            };

                            int c1 = static_cast<int>(i);
                            int c2 = alkylRootNode;
                            int len1 = getPlainChainLength(c1, sO);
                            int len2 = getPlainChainLength(c2, sO);

                            if (len1 != -1 && len2 != -1 && len1 == len2) {
                                QString rootStr = chainRoot(len1);
                                if (rootStr.isEmpty()) {
                                    return {false, "", "Acid anhydrides with chain length > 20 are not supported."};
                                }
                                QString infix = "an";
                                QString stem = rootStr + infix;
                                QString sfx = "oic anhydride";

                                QChar checkC;
                                for (QChar ch : sfx) {
                                    if (ch.isLetter()) { checkC = ch; break; }
                                }
                                if (!isVowel(checkC)) {
                                    stem += "e";
                                }
                                QString name = stem + sfx;
                                return {true, name, ""};
                            }

                            // General path for mixed, substituted, or branched acid anhydrides
                            std::set<int> exclude1;
                            std::vector<int> q1 = {sO};
                            exclude1.insert(sO);
                            while (!q1.empty()) {
                                int u = q1.back();
                                q1.pop_back();
                                for (int nei : g.nodes[u].neighbors) {
                                    if (nei != c1 && !exclude1.count(nei)) {
                                        exclude1.insert(nei);
                                        q1.push_back(nei);
                                    }
                                }
                            }

                            std::set<int> exclude2;
                            std::vector<int> q2 = {sO};
                            exclude2.insert(sO);
                            while (!q2.empty()) {
                                int u = q2.back();
                                q2.pop_back();
                                for (int nei : g.nodes[u].neighbors) {
                                    if (nei != c2 && !exclude2.count(nei)) {
                                        exclude2.insert(nei);
                                        q2.push_back(nei);
                                    }
                                }
                            }

                            QString name1 = nameAcidChainFrom(g, c1, exclude1);
                            QString name2 = nameAcidChainFrom(g, c2, exclude2);

                            if (name1.isEmpty() || name2.isEmpty()) {
                                return {false, "", "Acid anhydride could not be named."};
                            }
                            if (!name1.endsWith(" acid") || !name2.endsWith(" acid")) {
                                return {false, "", "Acid anhydride could not be named."};
                            }

                            QString acyl1 = name1.left(name1.length() - 5);
                            QString acyl2 = name2.left(name2.length() - 5);

                            QString finalName;
                            if (acyl1 == acyl2) {
                                finalName = acyl1 + " anhydride";
                            } else {
                                if (acyl1.toLower() < acyl2.toLower()) {
                                    finalName = acyl1 + " " + acyl2 + " anhydride";
                                } else {
                                    finalName = acyl2 + " " + acyl1 + " anhydride";
                                }
                            }
                            return {true, finalName, ""};
                        }
                        if (cCount == 2 && alkylRootNode != -1) {
                            carbonGroup[i] = GroupType::ESTER;
                            esterAlkylRoot[i] = alkylRootNode;
                            esterOxygen[i] = sO;
                        }
                    }
                }

                if (carbonGroup.count(i) && carbonGroup[i] == GroupType::ESTER) continue;

                if (!doubleO.empty() && !singleO.empty()) {
                    bool hasOH = false;
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            hasOH = true; break;
                        }
                    }
                    if (hasOH) carbonGroup[i] = GroupType::ACID;
                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    carbonGroup[i] = GroupType::AMIDE;
                } else if (!tripleN.empty()) {
                    carbonGroup[i] = GroupType::NITRILE;
                } else if (!doubleO.empty() && (node.totalH >= 1 || node.neighbors.size() <= 2)) {
                    carbonGroup[i] = GroupType::ALDEHYDE;
                } else if (!doubleO.empty()) {
                    carbonGroup[i] = GroupType::KETONE;
                } else if (!doubleS.empty() && (node.totalH >= 1 || node.neighbors.size() <= 2)) {
                    carbonGroup[i] = GroupType::THIAL;
                } else if (!doubleS.empty()) {
                    carbonGroup[i] = GroupType::THIONE;
                } else if (!singleO.empty()) {
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            carbonGroup[i] = GroupType::ALCOHOL; break;
                        }
                    }
                } else if (!singleN.empty()) {
                    carbonGroup[i] = GroupType::AMINE;
                } else if (carbonSulfonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::SULFONIC_ACID;
                } else if (carbonBoronicAcid.count(i)) {
                    carbonGroup[i] = GroupType::BORONIC_ACID;
                } else if (carbonThiol.count(i)) {
                    carbonGroup[i] = GroupType::THIOL;
                } else if (carbonPhosphine.count(i)) {
                    carbonGroup[i] = GroupType::PHOSPHINE;
                }
            } else if (node.atomicNumber == 8) {
                if (node.neighbors.size() == 2 && node.bondOrders[0] == 1 && node.bondOrders[1] == 1) {
                    int n1 = node.neighbors[0];
                    int n2 = node.neighbors[1];
                    if (g.nodes[n1].atomicNumber == 6 && g.nodes[n2].atomicNumber == 6) {
                        etherOxygens.insert(node.id);
                    }
                }
            }
        }

        int acylHalideZ = -1;
        int esterCount = 0;
        for (auto it = carbonGroup.begin(); it != carbonGroup.end(); ++it) {
            if (it->second == GroupType::ACYL_HALIDE) {
                int z = acylHalideHalogen[it->first];
                if (acylHalideZ == -1) acylHalideZ = z;
                else if (acylHalideZ != z) {
                    return {false, "", "Multiple acyl halide groups with different halogens are not supported in this phase."};
                }
            } else if (it->second == GroupType::ESTER) {
                esterCount++;
            }
        }

        if (esterCount > 1) {
            return {false, "", "Multiple ester groups are not supported in this phase."};
        }

        GroupType winningType = GroupType::NONE;
        static const GroupType seniorityOrder[] = {
            GroupType::SULFONIC_ACID, GroupType::ACID, GroupType::BORONIC_ACID, GroupType::ESTER, GroupType::ACYL_HALIDE, GroupType::AMIDE, GroupType::NITRILE,
            GroupType::ALDEHYDE, GroupType::THIAL, GroupType::KETONE, GroupType::THIONE, GroupType::ALCOHOL, GroupType::THIOL, GroupType::AMINE, GroupType::PHOSPHINE
        };

        for (GroupType gt : seniorityOrder) {
            for (auto it = carbonGroup.begin(); it != carbonGroup.end(); ++it) {
                if (it->second == gt) {
                    winningType = gt; break;
                }
            }
            if (winningType != GroupType::NONE) break;
        }

        if (ringSubstituentInfos.size() == 1 && winningType == GroupType::NONE) {
            // Non-ring portion has no principal group (e.g. plain ethylbenzene).
            // Fall through to monocyclic path.
        } else {
        std::set<int> principalCarbons;
        if (winningType != GroupType::NONE) {
            for (auto it = carbonGroup.begin(); it != carbonGroup.end(); ++it) {
                if (it->second == winningType) {
                    principalCarbons.insert(it->first);
                }
            }
        }

        std::vector<int> carbons;
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            if (g.nodes[i].atomicNumber == 6 && !allRingSubstituentNodes.count(static_cast<int>(i))) carbons.push_back(static_cast<int>(i));
        }

        if (carbons.empty()) {
            return {false, "", "No carbon atoms present in structure."};
        }

        std::vector<std::vector<int>> candidatePaths;
        if (carbons.size() == 1) {
            candidatePaths.push_back({carbons[0]});
        } else {
            for (int c : carbons) {
                candidatePaths.push_back({c});
            }
            for (size_t i = 0; i < carbons.size(); ++i) {
                for (size_t j = i + 1; j < carbons.size(); ++j) {
                    std::vector<int> path;
                    std::vector<bool> visited(g.nodes.size(), false);
                    std::vector<std::vector<int>> found;

                    auto dfs = [&](auto self, int curr, int target) -> void {
                        visited[curr] = true;
                        path.push_back(curr);
                        if (curr == target) {
                            found.push_back(path);
                        } else {
                            for (int nei : g.nodes[curr].neighbors) {
                                if (g.nodes[nei].atomicNumber == 6 && !allRingSubstituentNodes.count(nei) && !visited[nei]) {
                                    self(self, nei, target);
                                }
                            }
                        }
                        path.pop_back();
                        visited[curr] = false;
                    };

                    dfs(dfs, carbons[i], carbons[j]);
                    for (const auto &p : found) candidatePaths.push_back(p);
                }
            }
        }

        int maxPrincipal = -1, maxLength = -1, maxUnsat = -1;

        for (const auto &p : candidatePaths) {
            int pCount = 0;
            for (int c : p) if (principalCarbons.count(c)) pCount++;
            int len = static_cast<int>(p.size());

            int uCount = 0;
            for (size_t i = 0; i + 1 < p.size(); ++i) {
                int u = p[i], v = p[i+1];
                for (size_t k = 0; k < g.nodes[u].neighbors.size(); ++k) {
                    if (g.nodes[u].neighbors[k] == v) {
                        if (g.nodes[u].bondOrders[k] > 1) uCount++;
                        break;
                    }
                }
            }

            if (pCount > maxPrincipal) {
                maxPrincipal = pCount; maxLength = len; maxUnsat = uCount;
            } else if (pCount == maxPrincipal) {
                if (len > maxLength) {
                    maxLength = len; maxUnsat = uCount;
                } else if (len == maxLength) {
                    if (uCount > maxUnsat) maxUnsat = uCount;
                }
            }
        }

        if (maxLength > 20) {
            return {false, "", "Carbon chain length exceeds the C20 cap for Phase 1."};
        }

        std::vector<std::vector<int>> topPaths;
        for (const auto &p : candidatePaths) {
            int pCount = 0;
            for (int c : p) if (principalCarbons.count(c)) pCount++;
            int len = static_cast<int>(p.size());
            int uCount = 0;
            for (size_t i = 0; i + 1 < p.size(); ++i) {
                int u = p[i], v = p[i+1];
                for (size_t k = 0; k < g.nodes[u].neighbors.size(); ++k) {
                    if (g.nodes[u].neighbors[k] == v) {
                        if (g.nodes[u].bondOrders[k] > 1) uCount++;
                        break;
                    }
                }
            }

            if (pCount == maxPrincipal && len == maxLength && uCount == maxUnsat) {
                topPaths.push_back(p);
            }
        }

        struct PathSignature {
            std::vector<int> parentChain;
            std::vector<int> principalLocants;
            std::vector<int> doubleBondLocants;
            std::vector<int> tripleBondLocants;
            std::vector<int> substituentLocants;
            std::vector<std::pair<QString,int>> namedSubstituents;
        };

        std::vector<PathSignature> directedSignatures;

        for (const auto &p : topPaths) {
            std::vector<std::vector<int>> dirs = {p};
            if (p.size() > 1) {
                std::vector<int> revP = p;
                std::reverse(revP.begin(), revP.end());
                dirs.push_back(revP);
            }

            for (const auto &dChain : dirs) {
                PathSignature sig;
                sig.parentChain = dChain;
                std::set<int> chainSet(dChain.begin(), dChain.end());

                for (size_t i = 0; i < dChain.size(); ++i) {
                    int c = dChain[i];
                    int locant = static_cast<int>(i + 1);
                    bool isPrincipalC = principalCarbons.count(c) > 0;

                    if (isPrincipalC) {
                        sig.principalLocants.push_back(locant);
                    }

                    if (i + 1 < dChain.size()) {
                        int nextC = dChain[i+1];
                        for (size_t k = 0; k < g.nodes[c].neighbors.size(); ++k) {
                            if (g.nodes[c].neighbors[k] == nextC) {
                                int bo = g.nodes[c].bondOrders[k];
                                if (bo == 2) sig.doubleBondLocants.push_back(locant);
                                else if (bo == 3) sig.tripleBondLocants.push_back(locant);
                                break;
                            }
                        }
                    }

                    for (int nei : g.nodes[c].neighbors) {
                        if (chainSet.count(nei)) continue;
                        sig.substituentLocants.push_back(locant);

                        int nz = g.nodes[nei].atomicNumber;
                        QString subName;
                        if (isPrincipalC && (nz == 7 || nz == 8)) {
                        } else if (nz == 9 || nz == 17 || nz == 35 || nz == 53) {
                            subName = halogenPrefix(nz);
                        } else if (nz == 8 || nz == 7) {
                            subName = (nz == 8) ? "hydroxy" : "amino";
                        } else if (nz == 6) {
                            if (allRingSubstituentNodes.count(nei)) {
                                for (const auto &info : ringSubstituentInfos) {
                                    if (info.ringNodes.count(nei)) {
                                        subName = info.prefixName;
                                        break;
                                    }
                                }
                            } else {
                                subName = nameBranchGraph(g, nei, c, allSSSRRings, chainSet);
                            }
                            if (subName.startsWith("(")) subName = subName.mid(1);
                        }
                        if (!subName.isEmpty()) {
                            sig.namedSubstituents.push_back({subName, locant});
                        }
                    }
                }
                std::sort(sig.principalLocants.begin(), sig.principalLocants.end());
                std::sort(sig.doubleBondLocants.begin(), sig.doubleBondLocants.end());
                std::sort(sig.tripleBondLocants.begin(), sig.tripleBondLocants.end());
                std::sort(sig.substituentLocants.begin(), sig.substituentLocants.end());
                directedSignatures.push_back(sig);
            }
        }

        auto bestIt = std::min_element(directedSignatures.begin(), directedSignatures.end(),
            [](const PathSignature &a, const PathSignature &b) {
                if (a.principalLocants != b.principalLocants) return a.principalLocants < b.principalLocants;
                if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;
                if (a.tripleBondLocants != b.tripleBondLocants) return a.tripleBondLocants < b.tripleBondLocants;
                if (a.substituentLocants != b.substituentLocants) return a.substituentLocants < b.substituentLocants;

                auto firstAlpha = [](const std::vector<std::pair<QString,int>> &named) {
                    return std::min_element(named.begin(), named.end(),
                        [](const auto &x, const auto &y) { return x.first.toLower() < y.first.toLower(); });
                };
                if (!a.namedSubstituents.empty()) {
                    QString alphaName = firstAlpha(a.namedSubstituents)->first;
                    auto findLocant = [&](const std::vector<std::pair<QString,int>> &named) {
                        int best = INT_MAX;
                        for (const auto &ns : named) if (ns.first == alphaName) best = std::min(best, ns.second);
                        return best;
                    };
                    int aLoc = findLocant(a.namedSubstituents);
                    int bLoc = findLocant(b.namedSubstituents);
                    if (aLoc != bLoc) return aLoc < bLoc;
                }
                return false;
            });

        if (directedSignatures.empty() || bestIt == directedSignatures.end()) {
            return {false, "", "Could not determine a valid parent chain for this structure."};
        }
        PathSignature bestSig = *bestIt;
        std::map<int, int> graphIdToLocant;
        for (size_t i = 0; i < bestSig.parentChain.size(); ++i) {
            graphIdToLocant[bestSig.parentChain[i]] = static_cast<int>(i + 1);
        }
        StereoResult ezRes = processDoubleBondStereo(mol, g, allRingSubstituentNodes, graphIdToLocant, stereoByGraphId);
        if (!ezRes.ok) {
            return {false, "", ezRes.error};
        }
        std::set<int> handledBranchStereoIds;
        StereoResult stereoRes;

        if (winningType == GroupType::ESTER && !stereoByGraphId.empty()) {
            return {false, "", "Stereodescriptors combined with ester naming are not supported in this phase."};
        }

        std::vector<int> parentChain = bestSig.parentChain;
        int k = static_cast<int>(parentChain.size());
        std::set<int> chainSet(parentChain.begin(), parentChain.end());

        std::map<int, QStringList> locantSubstituents;

        for (int i = 0; i < k; ++i) {
            int cNode = parentChain[i];
            int locant = i + 1;
            bool isPrincipalCarbon = principalCarbons.count(cNode) > 0;

            if (carbonNitro.count(cNode)) {
                for (size_t nIdx = 0; nIdx < carbonNitro[cNode].size(); ++nIdx) {
                    locantSubstituents[locant].append("nitro");
                }
            }
            if (carbonIsocyanate.count(cNode)) {
                for (size_t nIdx = 0; nIdx < carbonIsocyanate[cNode].size(); ++nIdx) {
                    locantSubstituents[locant].append("isocyanato");
                }
            }
            if (carbonAzide.count(cNode)) {
                for (size_t nIdx = 0; nIdx < carbonAzide[cNode].size(); ++nIdx) {
                    locantSubstituents[locant].append("azido");
                }
            }
            if (carbonSulfonicAcid.count(cNode) && winningType != GroupType::SULFONIC_ACID) {
                locantSubstituents[locant].append("sulfo");
            }
            if (carbonBoronicAcid.count(cNode) && winningType != GroupType::BORONIC_ACID) {
                locantSubstituents[locant].append("borono");
            }
            if (carbonThiol.count(cNode) && winningType != GroupType::THIOL) {
                locantSubstituents[locant].append("sulfanyl");
            }
            if (carbonPhosphine.count(cNode) && winningType != GroupType::PHOSPHINE) {
                locantSubstituents[locant].append("phosphino");
            }
            if (carbonGroup.count(cNode) && winningType != carbonGroup[cNode]) {
                if (carbonGroup[cNode] == GroupType::ACID) {
                    locantSubstituents[locant].append("carboxy");
                } else if (carbonGroup[cNode] == GroupType::ACYL_HALIDE) {
                    locantSubstituents[locant].append(halogenPrefix(acylHalideHalogen[cNode]) + "carbonyl");
                } else if (carbonGroup[cNode] == GroupType::ESTER) {
                    int alkylRoot = esterAlkylRoot[cNode];
                    int sO = esterOxygen[cNode];
                    QString alkylName = nameBranchGraph(g, alkylRoot, sO, allSSSRRings);
                    if (alkylName.isEmpty()) {
                        return {false, "", "Unsupported ester alkyl group."};
                    }
                    if (alkylName.endsWith("yl")) {
                        alkylName.chop(2);
                        alkylName += "oxycarbonyl";
                    }
                    locantSubstituents[locant].append(alkylName);
                }
            }

            for (size_t j = 0; j < g.nodes[cNode].neighbors.size(); ++j) {
                int nei = g.nodes[cNode].neighbors[j];
                int order = g.nodes[cNode].bondOrders[j];
                if (chainSet.count(nei)) continue;

                int z = g.nodes[nei].atomicNumber;
                if (isPrincipalCarbon && (z == 7 || z == 8 || z == 16 || z == 9 || z == 17 || z == 35 || z == 53)) {
                    continue;
                }
                if (carbonSulfonicAcid.count(cNode) && carbonSulfonicAcid[cNode] == nei) continue;
                if (carbonThiol.count(cNode) && carbonThiol[cNode] == nei) continue;
                if (carbonNitro.count(cNode) && std::find(carbonNitro[cNode].begin(), carbonNitro[cNode].end(), nei) != carbonNitro[cNode].end()) continue;
                if (carbonIsocyanate.count(cNode) && std::find(carbonIsocyanate[cNode].begin(), carbonIsocyanate[cNode].end(), nei) != carbonIsocyanate[cNode].end()) continue;
                if (carbonAzide.count(cNode) && std::find(carbonAzide[cNode].begin(), carbonAzide[cNode].end(), nei) != carbonAzide[cNode].end()) continue;

                if (carbonGroup.count(cNode) && winningType != carbonGroup[cNode]) {
                    if (carbonGroup[cNode] == GroupType::ACID || carbonGroup[cNode] == GroupType::ACYL_HALIDE || carbonGroup[cNode] == GroupType::ESTER) {
                        if (z == 8 || z == 9 || z == 17 || z == 35 || z == 53) continue;
                    }
                }

                if (z == 16) {
                    if (thioetherSulfurs.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            alkylName += "sulfanyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (sulfoxideSulfurs.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            alkylName += "sulfinyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (sulfoneSulfurs.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            alkylName += "sulfonyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (disulfideSulfurs.count(nei)) {
                        int s2Nei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 16 && oNei != cNode) { s2Nei = oNei; break; }
                        }
                        if (s2Nei != -1) {
                            int alkylNei = -1;
                            for (int oNei : g.nodes[s2Nei].neighbors) {
                                if (g.nodes[oNei].atomicNumber == 6 && oNei != nei) { alkylNei = oNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, s2Nei, allSSSRRings);
                                alkylName += "disulfanyl";
                                locantSubstituents[locant].append(alkylName);
                            }
                        }
                    }
                } else if (z == 9 || z == 17 || z == 35 || z == 53) {
                    locantSubstituents[locant].append(halogenPrefix(z));
                } else if (z == 8) {
                    if (order == 1) {
                        if (etherOxygens.count(nei)) {
                            int alkylNei = -1;
                            for (int oNei : g.nodes[nei].neighbors) {
                                if (oNei != cNode) { alkylNei = oNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                                if (alkylName.endsWith("yl")) {
                                    alkylName.chop(2); alkylName += "oxy";
                                }
                                locantSubstituents[locant].append(alkylName);
                            }
                        } else if (winningType != GroupType::ALCOHOL) {
                            locantSubstituents[locant].append("hydroxy");
                        }
                    } else if (order == 2) {
                        if (winningType != GroupType::ALDEHYDE && winningType != GroupType::KETONE) {
                            locantSubstituents[locant].append("oxo");
                        }
                    }
                } else if (z == 7) {
                    if (order == 1 && winningType != GroupType::AMINE) {
                        int cAcyl = -1;
                        for (int nNei : g.nodes[nei].neighbors) {
                            if (nNei != cNode && g.nodes[nNei].atomicNumber == 6) {
                                for (size_t k = 0; k < g.nodes[nNei].neighbors.size(); ++k) {
                                    int cNei = g.nodes[nNei].neighbors[k];
                                    if (g.nodes[cNei].atomicNumber == 8 && g.nodes[nNei].bondOrders[k] == 2) {
                                        cAcyl = nNei; break;
                                    }
                                }
                            }
                        }
                        if (cAcyl != -1) {
                            int rGroup = -1;
                            for (int aNei : g.nodes[cAcyl].neighbors) {
                                if (aNei != nei && g.nodes[aNei].atomicNumber != 8) {
                                    rGroup = aNei; break;
                                }
                            }
                            if (rGroup != -1) {
                                QString rName = nameBranchGraph(g, rGroup, cAcyl, allSSSRRings);
                                if (!rName.isEmpty()) {
                                    QString cleanName = rName;
                                    if (cleanName.startsWith("(") && cleanName.endsWith(")")) {
                                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                                    }
                                    if (cleanName.startsWith("[") && cleanName.endsWith("]")) {
                                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                                    }
                                    if (cleanName.startsWith("{") && cleanName.endsWith("}")) {
                                        cleanName = cleanName.mid(1, cleanName.length() - 2);
                                    }
                                    if (cleanName.endsWith("phenyl")) {
                                        cleanName.chop(6);
                                        QString bName = cleanName + "benzamido";
                                        locantSubstituents[locant].append(cleanName.isEmpty() ? bName : ("(" + bName + ")"));
                                    } else if (cleanName.endsWith("yl")) {
                                        cleanName.chop(2);
                                        QString aName = cleanName + "amido";
                                        locantSubstituents[locant].append(cleanName.isEmpty() ? aName : ("(" + aName + ")"));
                                    } else {
                                        QString aName = cleanName + "amido";
                                        locantSubstituents[locant].append(cleanName.isEmpty() ? aName : ("(" + aName + ")"));
                                    }
                                }
                            }
                        } else {
                            locantSubstituents[locant].append("amino");
                        }
                    }
                } else if (z == 6) {
                    if (allRingSubstituentNodes.count(nei)) {
                        for (const auto &info : ringSubstituentInfos) {
                            if (info.ringNodes.count(nei)) {
                                locantSubstituents[locant].append(info.prefixName);
                                break;
                            }
                        }
                    } else {
                        QString bName = nameBranchGraph(g, nei, cNode, allSSSRRings);
                        QString branchStereo = formatBranchStereoPrefix(g, nei, cNode, stereoByGraphId, handledBranchStereoIds);
                        if (!branchStereo.isEmpty()) {
                            if (bName.startsWith("(") && bName.endsWith(")")) {
                                QString inner = bName.mid(1, bName.length() - 2);
                                bName = QString("[%1%2]").arg(branchStereo, inner);
                            } else {
                                bName = QString("[%1%2]").arg(branchStereo, bName);
                            }
                        }
                        locantSubstituents[locant].append(bName);
                    }
                }
            }
        }

        stereoRes = formatStereoPrefix(stereoByGraphId, graphIdToLocant, handledBranchStereoIds);
        if (!stereoRes.ok) {
            return {false, "", stereoRes.error};
        }

        std::map<QString, std::vector<int>> prefixLocantsMap;
        for (auto it = locantSubstituents.begin(); it != locantSubstituents.end(); ++it) {
            int loc = it->first;
            for (const QString &p : it->second) {
                prefixLocantsMap[p].push_back(loc);
            }
        }

        struct PrefixGroup {
            QString baseName;
            QString formattedStr;
        };
        std::vector<PrefixGroup> pGroups;

        for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
            QString pName = it->first;
            std::vector<int> locs = it->second;
            std::sort(locs.begin(), locs.end());

            QString pStr;
            if (k == 1) {
                // A single-carbon parent (methane) has only one possible attachment point --
                // locants are always "1" and add no information, so IUPAC omits them entirely
                // (e.g. "bromochlorofluoromethane", not "1-bromo-1-chloro-1-fluoromethane").
                pStr = (locs.size() > 1 ? multiPrefix(static_cast<int>(locs.size())) : QString()) + pName;
            } else if (k == 2 && prefixLocantsMap.size() == 1 && locs.size() == 1 && locs[0] == 1 && winningType == GroupType::NONE) {
                pStr = pName;
            } else {
                QStringList locStrs;
                for (int l : locs) locStrs.append(QString::number(l));
                pStr = locStrs.join(",");
                if (locs.size() > 1) {
                    pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
                } else {
                    pStr += "-" + pName;
                }
            }

            PrefixGroup pg;
            pg.baseName = (pName.startsWith("(") || pName.startsWith("[")) ? pName.mid(1) : pName;
            pg.formattedStr = pStr;
            pGroups.push_back(pg);
        }

        std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
            return a.baseName.toLower() < b.baseName.toLower();
        });

        QString prefixPart;
        if (!pGroups.empty()) {
            QStringList pStrs;
            for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
            // With a single-carbon parent there are no locant numbers to separate (see the
            // k==1 branch above), so the prefixes concatenate directly with no hyphens at all
            // ("bromochlorofluoro-", not "bromo-chloro-fluoro-"); otherwise each already-
            // locant-prefixed group is hyphen-joined as before.
            prefixPart = pStrs.join(k == 1 ? "" : "-");
        }

        QString rootStr = chainRoot(k);

        std::vector<int> dbLocs = bestSig.doubleBondLocants;
        std::vector<int> tbLocs = bestSig.tripleBondLocants;

        QString infix;
        if (dbLocs.empty() && tbLocs.empty()) {
            infix = "an";
        } else if (!dbLocs.empty() && tbLocs.empty()) {
            if (dbLocs.size() == 1) {
                if (k <= 2) infix = "en";
                else infix = QString("-%1-en").arg(dbLocs[0]);
            } else {
                rootStr += "a";
                QStringList lStrs;
                for (int l : dbLocs) lStrs.append(QString::number(l));
                infix = QString("-%1-%2en").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
            }
        } else if (dbLocs.empty() && !tbLocs.empty()) {
            if (tbLocs.size() == 1) {
                if (k <= 2) infix = "yn";
                else infix = QString("-%1-yn").arg(tbLocs[0]);
            } else {
                rootStr += "a";
                QStringList lStrs;
                for (int l : tbLocs) lStrs.append(QString::number(l));
                infix = QString("-%1-%2yn").arg(lStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size())));
            }
        } else {
            QStringList dStrs, tStrs;
            for (int l : dbLocs) dStrs.append(QString::number(l));
            for (int l : tbLocs) tStrs.append(QString::number(l));

            QString dPart = (dbLocs.size() > 1) ? QString("%1-%2en").arg(dStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size()))) : QString("%1-en").arg(dStrs[0]);
            QString tPart = (tbLocs.size() > 1) ? QString("%1-%2yn").arg(tStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size()))) : QString("%1-yn").arg(tStrs[0]);
            infix = QString("-%1-%2").arg(dPart, tPart);
        }

        QString fullName = prefixPart + rootStr + infix;
        int pCount = static_cast<int>(bestSig.principalLocants.size());
        if (winningType != GroupType::NONE) {
            QString stem = rootStr + infix;
            QString sfx;
            if (winningType == GroupType::SULFONIC_ACID) {
                if (pCount == 1) sfx = (k <= 2) ? "sulfonic acid" : QString("-%1-sulfonic acid").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2sulfonic acid").arg(lStrs.join(","), multiPrefix(pCount));
                }
            } else if (winningType == GroupType::ACID) sfx = (pCount == 2) ? "dioic acid" : "oic acid";
            else if (winningType == GroupType::ESTER) sfx = "oate";
            else if (winningType == GroupType::ACYL_HALIDE) {
                QString hName;
                int hz = acylHalideHalogen[bestSig.parentChain[0]];
                for (int pc : bestSig.parentChain) {
                    if (acylHalideHalogen.count(pc)) { hz = acylHalideHalogen[pc]; break; }
                }
                if (hz == 9) hName = "fluoride";
                else if (hz == 17) hName = "chloride";
                else if (hz == 35) hName = "bromide";
                else if (hz == 53) hName = "iodide";

                sfx = (pCount == 2) ? ("dioyl " + hName) : ("oyl " + hName);
            } else if (winningType == GroupType::AMIDE) sfx = (pCount == 2) ? "diamide" : "amide";
            else if (winningType == GroupType::NITRILE) sfx = (pCount == 2) ? "dinitrile" : "nitrile";
            else if (winningType == GroupType::ALDEHYDE) sfx = (pCount == 2) ? "dial" : "al";
            else if (winningType == GroupType::THIAL) sfx = (pCount == 2) ? "dithial" : "thial";
            else if (winningType == GroupType::KETONE) {
                if (pCount == 1) sfx = QString("-%1-one").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2one").arg(lStrs.join(","), multiPrefix(pCount));
                }
            } else if (winningType == GroupType::THIONE) {
                if (pCount == 1) sfx = QString("-%1-thione").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2thione").arg(lStrs.join(","), multiPrefix(pCount));
                }
            } else if (winningType == GroupType::ALCOHOL) {
                if (pCount == 1) sfx = (k <= 2) ? "ol" : QString("-%1-ol").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2ol").arg(lStrs.join(","), multiPrefix(pCount));
                }
            } else if (winningType == GroupType::THIOL) {
                if (pCount == 1) sfx = (k <= 2) ? "thiol" : QString("-%1-thiol").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2thiol").arg(lStrs.join(","), multiPrefix(pCount));
                }
            } else if (winningType == GroupType::AMINE) {
                if (pCount == 1) sfx = (k <= 2) ? "amine" : QString("-%1-amine").arg(bestSig.principalLocants[0]);
                else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    sfx = QString("-%1-%2amine").arg(lStrs.join(","), multiPrefix(pCount));
                }
            }

            QChar checkC;
            for (QChar ch : sfx) {
                if (ch.isLetter()) { checkC = ch; break; }
            }

            if (!isVowel(checkC)) {
                stem += "e";
            }
            if (sfx.startsWith("-")) {
                fullName = prefixPart + stem + sfx;
            } else {
                fullName = prefixPart + stem + sfx;
            }
        } else {
            fullName = prefixPart + rootStr + infix + "e";
        }

        if (winningType == GroupType::PHOSPHINE) {
            int pCarbon = -1;
            for (int pc : principalCarbons) {
                if (carbonGroup.count(pc) && carbonGroup[pc] == GroupType::PHOSPHINE) {
                    pCarbon = pc; break;
                }
            }
            if (pCarbon != -1) {
                int pNode = carbonPhosphine[pCarbon];
                QString alkylName = nameBranchGraph(g, pCarbon, pNode);
                if (alkylName.isEmpty()) {
                    return {false, "", "Unsupported phosphine alkyl group."};
                }
                fullName = stereoRes.prefix + alkylName + "phosphine";
                return {true, fullName, ""};
            }
        } else if (winningType == GroupType::BORONIC_ACID) {
            int bCarbon = -1;
            for (int pc : principalCarbons) {
                if (carbonGroup.count(pc) && carbonGroup[pc] == GroupType::BORONIC_ACID) {
                    bCarbon = pc; break;
                }
            }
            if (bCarbon != -1) {
                int bNode = carbonBoronicAcid[bCarbon];
                QString alkylName = nameBranchGraph(g, bCarbon, bNode);
                if (alkylName.isEmpty()) {
                    return {false, "", "Unsupported boronic acid alkyl group."};
                }
                fullName = stereoRes.prefix + alkylName + "boronic acid";
                return {true, fullName, ""};
            }
        }

        if (winningType == GroupType::ESTER) {
            int esterCarbon = -1;
            for (int pc : principalCarbons) {
                if (carbonGroup.count(pc) && carbonGroup[pc] == GroupType::ESTER) {
                    esterCarbon = pc;
                    break;
                }
            }
            if (esterCarbon != -1) {
                int alkylRoot = esterAlkylRoot[esterCarbon];
                int sO = esterOxygen[esterCarbon];
                QString alkylName;
                if (isPlainBenzeneRing(mol, g, indigoToGraphIdx, alkylRoot, sO)) {
                    alkylName = "phenyl";
                } else {
                    alkylName = nameBranchGraph(g, alkylRoot, sO, allSSSRRings);
                }
                if (alkylName.isEmpty()) {
                    return {false, "", "Unsupported ester alkyl group."};
                }
                fullName = alkylName + " " + fullName;
            }
        }

        fullName = stereoRes.prefix + fullName;
        return {true, fullName, ""};
        }
    }

    // --- Phase 27: Saturated Unsubstituted Bicyclic Hydrocarbon Path (von Baeyer: bicyclo[a.b.c]alkane) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        rAtoms.insert(indigoIndex(ringAtomHandle));
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2) {
                std::set<int> ringUnionNodes;
                for (const auto &r : sssrRings) {
                    for (int idx : r) {
                        if (indigoToGraphIdx.count(idx)) {
                            ringUnionNodes.insert(indigoToGraphIdx[idx]);
                        }
                    }
                }

                // 1. Precondition gate (fall through silently if any fail):
                // - Every atom in union must be carbon (atomicNumber == 6)
                // - NO bond within ring-atom union may have order != 1
                // - EVERY atom in ring-atom union must have ZERO neighbors outside ring-atom union
                bool validPreconditions = true;

                for (int n : ringUnionNodes) {
                    if (g.nodes[n].atomicNumber != 6) {
                        validPreconditions = false;
                        break;
                    }
                    for (int nei : g.nodes[n].neighbors) {
                        if (!ringUnionNodes.count(nei)) {
                            validPreconditions = false;
                            break;
                        }
                    }
                    if (!validPreconditions) break;
                }

                if (validPreconditions) {
                    for (const auto &gb : g.bonds) {
                        if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v)) {
                            if (gb.order != 1) {
                                validPreconditions = false;
                                break;
                            }
                        }
                    }
                }

                if (validPreconditions) {
                    // 2. Bridgehead identification:
                    // Find atoms in ringUnionNodes with exactly 3 neighbors ALSO in ringUnionNodes
                    std::vector<int> bridgeheads;
                    bool validDegrees = true;

                    for (int n : ringUnionNodes) {
                        int ringDegree = 0;
                        for (int nei : g.nodes[n].neighbors) {
                            if (ringUnionNodes.count(nei)) {
                                ringDegree++;
                            }
                        }
                        if (ringDegree == 3) {
                            bridgeheads.push_back(n);
                        } else if (ringDegree != 2) {
                            validDegrees = false;
                            break;
                        }
                    }

                    if (validDegrees && bridgeheads.size() == 2) {
                        int bhA = bridgeheads[0];
                        int bhB = bridgeheads[1];

                        // 3. Bridge-path tracing:
                        // Find the 3 neighbors of bhA in ringUnionNodes
                        std::vector<int> bhANeighbors;
                        for (int nei : g.nodes[bhA].neighbors) {
                            if (ringUnionNodes.count(nei)) {
                                bhANeighbors.push_back(nei);
                            }
                        }

                        if (bhANeighbors.size() == 3) {
                            std::vector<int> bridgeLengths;
                            bool traceOk = true;

                            for (int startNei : bhANeighbors) {
                                if (startNei == bhB) {
                                    // Direct bond between bridgeheads -> interior count 0
                                    bridgeLengths.push_back(0);
                                } else {
                                    int prev = bhA;
                                    int curr = startNei;
                                    int length = 1;
                                    bool reachedB = false;

                                    while (true) {
                                        if (curr == bhB) {
                                            reachedB = true;
                                            break;
                                        }
                                        if (length > (int)ringUnionNodes.size()) {
                                            break;
                                        }
                                        int nextN = -1;
                                        for (int nei : g.nodes[curr].neighbors) {
                                            if (ringUnionNodes.count(nei) && nei != prev) {
                                                nextN = nei;
                                                break;
                                            }
                                        }
                                        if (nextN == -1) break;
                                        prev = curr;
                                        curr = nextN;
                                        if (curr != bhB) {
                                            length++;
                                        }
                                    }

                                    if (reachedB) {
                                        bridgeLengths.push_back(length);
                                    } else {
                                        traceOk = false;
                                        break;
                                    }
                                }
                            }

                            if (traceOk && bridgeLengths.size() == 3) {
                                int sumBridges = bridgeLengths[0] + bridgeLengths[1] + bridgeLengths[2];
                                if (sumBridges + 2 != (int)ringUnionNodes.size()) {
                                    return {false, "", "Internal error: invalid bicyclic bridge decomposition."};
                                }

                                std::sort(bridgeLengths.rbegin(), bridgeLengths.rend());
                                int totalCarbons = (int)ringUnionNodes.size();
                                QString root = chainRoot(totalCarbons);
                                if (root.isEmpty()) {
                                    return {false, "", "Unsupported bicyclic ring size."};
                                }

                                QString fullName = QString("bicyclo[%1.%2.%3]%4ane")
                                    .arg(bridgeLengths[0])
                                    .arg(bridgeLengths[1])
                                    .arg(bridgeLengths[2])
                                    .arg(root);

                                return {true, fullName, ""};
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Phase 28: Saturated Unsubstituted Spiro Hydrocarbon Path (spiro[a.b]alkane) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        rAtoms.insert(indigoIndex(ringAtomHandle));
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2) {
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx.at(idx));
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx.at(idx));
                }

                std::set<int> ringUnionNodes;
                for (int n : ring1Nodes) ringUnionNodes.insert(n);
                for (int n : ring2Nodes) ringUnionNodes.insert(n);

                std::set<int> sharedNodes;
                for (int n : ring1Nodes) {
                    if (ring2Nodes.count(n)) sharedNodes.insert(n);
                }

                if (sharedNodes.size() == 1) {
                    int spiroNode = *sharedNodes.begin();

                    bool validPreconditions = true;

                    for (int n : ringUnionNodes) {
                        if (g.nodes[n].atomicNumber != 6) {
                            validPreconditions = false;
                            break;
                        }
                        for (int nei : g.nodes[n].neighbors) {
                            if (!ringUnionNodes.count(nei)) {
                                validPreconditions = false;
                                break;
                            }
                        }
                        if (!validPreconditions) break;

                        int ringDegree = 0;
                        for (int nei : g.nodes[n].neighbors) {
                            if (ringUnionNodes.count(nei)) {
                                ringDegree++;
                            }
                        }

                        if (n == spiroNode) {
                            if (ringDegree != 4) {
                                validPreconditions = false;
                                break;
                            }
                        } else {
                            if (ringDegree != 2) {
                                validPreconditions = false;
                                break;
                            }
                        }
                    }

                    if (validPreconditions) {
                        for (const auto &gb : g.bonds) {
                            if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v)) {
                                if (gb.order != 1) {
                                    validPreconditions = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (validPreconditions) {
                        int sizeA = static_cast<int>(ring1Nodes.size()) - 1;
                        int sizeB = static_cast<int>(ring2Nodes.size()) - 1;

                        if (sizeA > sizeB) {
                            std::swap(sizeA, sizeB);
                        }

                        int totalCarbons = static_cast<int>(ringUnionNodes.size());
                        if (totalCarbons != sizeA + sizeB + 1) {
                            return {false, "", "Internal error: invalid spiro decomposition."};
                        }

                        QString root = chainRoot(totalCarbons);
                        if (root.isEmpty()) {
                            return {false, "", "Unsupported spiro ring size."};
                        }

                        QString fullName = QString("spiro[%1.%2]%3ane")
                            .arg(sizeA)
                            .arg(sizeB)
                            .arg(root);

                        return {true, fullName, ""};
                    }
                }
            }
        }
    }

    // --- Phase 31: Benzo-fused Heterobicyclic Ring Systems (ringCount == 2) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(ringAtomHandle);
                        rAtoms.insert(idx);
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2) {
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
                }

                std::vector<int> sharedNodes;
                for (int n : ring1Nodes) {
                    if (ring2Nodes.count(n)) sharedNodes.push_back(n);
                }

                if (sharedNodes.size() == 2) {
                    int bhA = sharedNodes[0];
                    int bhB = sharedNodes[1];
                    bool bhBonded = false;
                    for (int nei : g.nodes[bhA].neighbors) {
                        if (nei == bhB) { bhBonded = true; break; }
                    }

                    if (bhBonded && g.nodes[bhA].atomicNumber == 6 && g.nodes[bhB].atomicNumber == 6) {
                        std::set<int> allSystemNodes = ring1Nodes;
                        allSystemNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

                        auto isBenzoRing = [&](const std::set<int> &rNodes) -> bool {
                            if (rNodes.size() != 6) return false;
                            for (int n : rNodes) {
                                if (g.nodes[n].atomicNumber != 6) return false;
                            }
                            for (const auto &gb : g.bonds) {
                                if (rNodes.count(gb.u) && rNodes.count(gb.v)) {
                                    if (gb.order != 4) return false;
                                }
                            }
                            return true;
                        };

                        bool r1Benzo = isBenzoRing(ring1Nodes);
                        bool r2Benzo = isBenzoRing(ring2Nodes);

                        if (r1Benzo != r2Benzo) {
                            // Fusion atoms bhA and bhB must NOT bear exocyclic substituents
                            for (int nei : g.nodes[bhA].neighbors) {
                                if (!allSystemNodes.count(nei)) return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                            }
                            for (int nei : g.nodes[bhB].neighbors) {
                                if (!allSystemNodes.count(nei)) return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                            }

                            // Every other ring atom may have at most 1 exocyclic substituent
                            for (int n : allSystemNodes) {
                                int exocyclicCount = 0;
                                for (int nei : g.nodes[n].neighbors) {
                                    if (!allSystemNodes.count(nei)) exocyclicCount++;
                                }
                                if (exocyclicCount > 1) {
                                    return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                                }
                            }

                            const std::set<int> &heteroNodes = r1Benzo ? ring2Nodes : ring1Nodes;
                            int hSize = static_cast<int>(heteroNodes.size());

                            if (hSize == 5 || hSize == 6) {
                                std::vector<int> heteroCycle;
                                int startNode = *heteroNodes.begin();
                                int current = startNode;
                                int previous = -1;
                                for (int step = 0; step < hSize; ++step) {
                                    heteroCycle.push_back(current);
                                    int nextNode = -1;
                                    for (int nei : g.nodes[current].neighbors) {
                                        if (heteroNodes.count(nei) && nei != previous) {
                                            nextNode = nei;
                                            break;
                                        }
                                    }
                                    if (nextNode == -1) break;
                                    previous = current;
                                    current = nextNode;
                                }

                                if (static_cast<int>(heteroCycle.size()) == hSize) {
                                    std::vector<int> ringHeteroNodes;
                                    for (int n : heteroNodes) {
                                        if (g.nodes[n].atomicNumber != 6) {
                                            ringHeteroNodes.push_back(n);
                                        }
                                    }

                                    RingType hType;
                                    QString dummyNameRoot, classErr;
                                    if (classifyMonocyclicHeteroRing(g, ringHeteroNodes, hSize, heteroCycle, hType, dummyNameRoot, classErr)) {
                                        auto getDistInCycle = [&](int uIdx, int vIdx) {
                                            int diff = std::abs(uIdx - vIdx);
                                            return std::min(diff, hSize - diff);
                                        };

                                        int idxA = -1, idxB = -1;
                                        for (int i = 0; i < hSize; ++i) {
                                            if (heteroCycle[i] == bhA) idxA = i;
                                            if (heteroCycle[i] == bhB) idxB = i;
                                        }

                                        if (idxA != -1 && idxB != -1) {
                                            QString resultName = "";

                                            if (hType == RingType::FURAN || hType == RingType::THIOPHENE ||
                                                hType == RingType::SELENOPHENE || hType == RingType::TELLUROPHENE ||
                                                hType == RingType::PHOSPHININE ||
                                                hType == RingType::PYRROLE || hType == RingType::PYRIDINE) {
                                                if (ringHeteroNodes.size() == 1) {
                                                    int hNode = ringHeteroNodes[0];
                                                    int idxH = -1;
                                                    for (int i = 0; i < hSize; ++i) {
                                                        if (heteroCycle[i] == hNode) idxH = i;
                                                    }
                                                    if (idxH != -1) {
                                                        int dA = getDistInCycle(idxH, idxA);
                                                        int dB = getDistInCycle(idxH, idxB);
                                                        int minDist = std::min(dA, dB);

                                                        if (minDist == 1) {
                                                            if (hType == RingType::FURAN) resultName = "benzofuran";
                                                            else if (hType == RingType::THIOPHENE) resultName = "benzothiophene";
                                                            else if (hType == RingType::SELENOPHENE) resultName = "benzoselenophene";
                                                            else if (hType == RingType::TELLUROPHENE) resultName = "benzotellurophene";
                                                            else if (hType == RingType::PHOSPHININE) resultName = "benzophosphinine";
                                                            else if (hType == RingType::PYRROLE) resultName = "indole";
                                                            else if (hType == RingType::PYRIDINE) resultName = "quinoline";
                                                        } else if (minDist == 2) {
                                                            if (hType == RingType::FURAN) resultName = "isobenzofuran";
                                                            else if (hType == RingType::THIOPHENE) resultName = "isobenzothiophene";
                                                            else if (hType == RingType::SELENOPHENE) resultName = "isobenzoselenophene";
                                                            else if (hType == RingType::TELLUROPHENE) resultName = "isobenzotellurophene";
                                                            else if (hType == RingType::PHOSPHININE) resultName = "isobenzophosphinine";
                                                            else if (hType == RingType::PYRROLE) resultName = "isoindole";
                                                            else if (hType == RingType::PYRIDINE) resultName = "isoquinoline";
                                                        }
                                                    }
                                                }
                                            } else if (hType == RingType::IMIDAZOLE) {
                                                resultName = "benzimidazole";
                                            } else if (hType == RingType::PYRIMIDINE) {
                                                resultName = "quinazoline";
                                            } else if (hType == RingType::PYRAZINE) {
                                                resultName = "quinoxaline";
                                            } else if (hType == RingType::PYRIDAZINE) {
                                                if (ringHeteroNodes.size() == 2) {
                                                    int h1 = ringHeteroNodes[0];
                                                    int h2 = ringHeteroNodes[1];
                                                    int idxH1 = -1, idxH2 = -1;
                                                    for (int i = 0; i < hSize; ++i) {
                                                        if (heteroCycle[i] == h1) idxH1 = i;
                                                        if (heteroCycle[i] == h2) idxH2 = i;
                                                    }
                                                    if (idxH1 != -1 && idxH2 != -1) {
                                                        int minDist = std::min({
                                                            getDistInCycle(idxH1, idxA),
                                                            getDistInCycle(idxH1, idxB),
                                                            getDistInCycle(idxH2, idxA),
                                                            getDistInCycle(idxH2, idxB)
                                                        });
                                                        if (minDist == 1) resultName = "cinnoline";
                                                        else if (minDist == 2) resultName = "phthalazine";
                                                    }
                                                }
                                            }

                                            if (!resultName.isEmpty()) {
                                                std::vector<std::pair<int, int>> ringSubstituents;
                                                for (int n : allSystemNodes) {
                                                    for (int nei : g.nodes[n].neighbors) {
                                                        if (!allSystemNodes.count(nei)) {
                                                            ringSubstituents.push_back({n, nei});
                                                        }
                                                    }
                                                }

                                                std::set<int> substituentBearingNodes;
                                                for (const auto &subPair : ringSubstituents) {
                                                    substituentBearingNodes.insert(subPair.first);
                                                }

                                                std::map<int, QString> locantMap = computePeripheralNumbering(g, ring1Nodes, ring2Nodes, bhA, bhB, substituentBearingNodes);
                                                if (locantMap.empty()) {
                                                    return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                                                }

                                                std::vector<std::pair<QString, int>> namedSubstituents;
                                                for (const auto &subPair : ringSubstituents) {
                                                    int rNode = subPair.first;
                                                    int sNode = subPair.second;
                                                    QString locStr = locantMap[rNode];
                                                    bool okInt = false;
                                                    int locVal = locStr.toInt(&okInt);
                                                    if (!okInt || locStr.isEmpty() || locStr.endsWith("a") || locStr.endsWith("b")) {
                                                        return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                                                    }

                                                    QString subName = nameBranchGraph(g, sNode, rNode);
                                                    if (subName.isEmpty()) {
                                                        return {false, "", "Unrecognized or unsupported substituent on ring."};
                                                    }
                                                    if (subName.startsWith("(")) {
                                                        subName = subName.mid(1);
                                                        if (subName.endsWith(")")) subName.chop(1);
                                                    }
                                                    namedSubstituents.push_back({subName, locVal});
                                                }

                                                std::map<QString, std::vector<int>> prefixLocantsMap;
                                                for (const auto &ns : namedSubstituents) {
                                                    prefixLocantsMap[ns.first].push_back(ns.second);
                                                }

                                                struct PrefixGroup {
                                                    QString baseName;
                                                    QString formattedStr;
                                                };
                                                std::vector<PrefixGroup> pGroups;
                                                for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
                                                    QString pName = it->first;
                                                    std::vector<int> locs = it->second;
                                                    std::sort(locs.begin(), locs.end());

                                                    QStringList locStrs;
                                                    for (int l : locs) locStrs.append(QString::number(l));

                                                    QString pStr = locStrs.join(",");
                                                    if (locs.size() > 1) {
                                                        pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
                                                    } else {
                                                        pStr += "-" + pName;
                                                    }

                                                    PrefixGroup pg;
                                                    pg.baseName = pName.startsWith("(") ? pName.mid(1) : pName;
                                                    pg.formattedStr = pStr;
                                                    pGroups.push_back(pg);
                                                }

                                                std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
                                                    return a.baseName.toLower() < b.baseName.toLower();
                                                });

                                                QString prefixPart;
                                                if (!pGroups.empty()) {
                                                    QStringList pStrs;
                                                    for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
                                                    prefixPart = pStrs.join("-");
                                                }

                                                return {true, prefixPart + resultName, ""};
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Phase 32: Purine (Imidazole + Pyrimidine Ortho-Fused Bicyclic System) (ringCount == 2) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(ringAtomHandle);
                        rAtoms.insert(idx);
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2) {
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
                }

                std::vector<int> sharedNodes;
                for (int n : ring1Nodes) {
                    if (ring2Nodes.count(n)) sharedNodes.push_back(n);
                }

                if (sharedNodes.size() == 2) {
                    int bhA = sharedNodes[0];
                    int bhB = sharedNodes[1];
                    bool bhBonded = false;
                    for (int nei : g.nodes[bhA].neighbors) {
                        if (nei == bhB) { bhBonded = true; break; }
                    }

                    if (bhBonded && g.nodes[bhA].atomicNumber == 6 && g.nodes[bhB].atomicNumber == 6) {
                        std::set<int> allSystemNodes = ring1Nodes;
                        allSystemNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

                        bool allUnsubstituted = true;
                        for (int n : allSystemNodes) {
                            for (int nei : g.nodes[n].neighbors) {
                                if (!allSystemNodes.count(nei)) {
                                    allUnsubstituted = false;
                                    break;
                                }
                            }
                            if (!allUnsubstituted) break;
                        }

                        if (allUnsubstituted) {
                            auto buildCycle = [&](const std::set<int> &rNodes) -> std::vector<int> {
                                int rSize = static_cast<int>(rNodes.size());
                                std::vector<int> cycle;
                                if (rSize < 3) return cycle;
                                int startNode = *rNodes.begin();
                                int current = startNode;
                                int previous = -1;
                                for (int step = 0; step < rSize; ++step) {
                                    cycle.push_back(current);
                                    int nextNode = -1;
                                    for (int nei : g.nodes[current].neighbors) {
                                        if (rNodes.count(nei) && nei != previous) {
                                            nextNode = nei;
                                            break;
                                        }
                                    }
                                    if (nextNode == -1) break;
                                    previous = current;
                                    current = nextNode;
                                }
                                if (static_cast<int>(cycle.size()) != rSize) return {};
                                return cycle;
                            };

                            std::vector<int> cycle1 = buildCycle(ring1Nodes);
                            std::vector<int> cycle2 = buildCycle(ring2Nodes);

                            if (cycle1.size() == ring1Nodes.size() && cycle2.size() == ring2Nodes.size()) {
                                std::vector<int> rHetero1, rHetero2;
                                for (int n : ring1Nodes) {
                                    if (g.nodes[n].atomicNumber != 6) rHetero1.push_back(n);
                                }
                                for (int n : ring2Nodes) {
                                    if (g.nodes[n].atomicNumber != 6) rHetero2.push_back(n);
                                }

                                RingType type1, type2;
                                QString dummyRoot1, dummyRoot2, classErr1, classErr2;
                                bool c1Ok = classifyMonocyclicHeteroRing(g, rHetero1, static_cast<int>(ring1Nodes.size()), cycle1, type1, dummyRoot1, classErr1);
                                bool c2Ok = classifyMonocyclicHeteroRing(g, rHetero2, static_cast<int>(ring2Nodes.size()), cycle2, type2, dummyRoot2, classErr2);

                                if (c1Ok && c2Ok) {
                                    bool isPurine = (type1 == RingType::IMIDAZOLE && type2 == RingType::PYRIMIDINE) ||
                                                    (type1 == RingType::PYRIMIDINE && type2 == RingType::IMIDAZOLE);
                                    if (isPurine) {
                                        return {true, "purine", ""};
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Phase 33: Pentalene (Unsubstituted 5-5 Fused Bicyclic Hydrocarbon, C8H6) (ringCount == 2) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(ringAtomHandle);
                        rAtoms.insert(idx);
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2 && sssrRings[0].size() == 5 && sssrRings[1].size() == 5) {
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
                }

                if (ring1Nodes.size() == 5 && ring2Nodes.size() == 5) {
                    std::vector<int> sharedNodes;
                    for (int n : ring1Nodes) {
                        if (ring2Nodes.count(n)) sharedNodes.push_back(n);
                    }

                    if (sharedNodes.size() == 2) {
                        int bhA = sharedNodes[0];
                        int bhB = sharedNodes[1];
                        bool bhBonded = false;
                        for (int nei : g.nodes[bhA].neighbors) {
                            if (nei == bhB) { bhBonded = true; break; }
                        }

                        if (bhBonded && g.nodes[bhA].atomicNumber == 6 && g.nodes[bhB].atomicNumber == 6) {
                            std::set<int> allSystemNodes = ring1Nodes;
                            allSystemNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

                            if (allSystemNodes.size() == 8) {
                                bool validPentalene = true;

                                for (int n : allSystemNodes) {
                                    if (g.nodes[n].atomicNumber != 6) {
                                        validPentalene = false;
                                        break;
                                    }
                                    for (int nei : g.nodes[n].neighbors) {
                                        if (!allSystemNodes.count(nei)) {
                                            validPentalene = false;
                                            break;
                                        }
                                    }
                                    if (!validPentalene) break;
                                }

                                if (validPentalene) {
                                    for (const auto &gb : g.bonds) {
                                        if (allSystemNodes.count(gb.u) && allSystemNodes.count(gb.v)) {
                                            if (gb.order != 1 && gb.order != 2) {
                                                validPentalene = false;
                                                break;
                                            }
                                        }
                                    }
                                }

                                if (validPentalene) {
                                    for (int n : allSystemNodes) {
                                        if (n == bhA || n == bhB) {
                                            if (g.nodes[n].totalH != 0) {
                                                validPentalene = false;
                                                break;
                                            }
                                        } else {
                                            if (g.nodes[n].totalH != 1) {
                                                validPentalene = false;
                                                break;
                                            }
                                        }
                                    }
                                }

                                if (validPentalene) {
                                    return {true, "pentalene", ""};
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Phase 34: 3H-Pyrrolizine (C7H7N, 5-5 Fused Bicyclic with Bridgehead Nitrogen) ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(ringAtomHandle);
                        rAtoms.insert(idx);
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2 && sssrRings[0].size() == 5 && sssrRings[1].size() == 5) {
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
                }

                if (ring1Nodes.size() == 5 && ring2Nodes.size() == 5) {
                    std::vector<int> sharedNodes;
                    for (int n : ring1Nodes) {
                        if (ring2Nodes.count(n)) sharedNodes.push_back(n);
                    }

                    if (sharedNodes.size() == 2) {
                        int bhA = sharedNodes[0];
                        int bhB = sharedNodes[1];
                        bool bhBonded = false;
                        for (int nei : g.nodes[bhA].neighbors) {
                            if (nei == bhB) { bhBonded = true; break; }
                        }

                        if (bhBonded && ((g.nodes[bhA].atomicNumber == 7 && g.nodes[bhB].atomicNumber == 6) || (g.nodes[bhA].atomicNumber == 6 && g.nodes[bhB].atomicNumber == 7))) {
                            std::set<int> allSystemNodes = ring1Nodes;
                            allSystemNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

                            if (allSystemNodes.size() == 8 && g.nodes.size() == 8) {
                                bool validPyrrolizine = true;
                                int nCount = 0;
                                int cCount = 0;

                                for (int n : allSystemNodes) {
                                    if (g.nodes[n].atomicNumber == 7) {
                                        nCount++;
                                    } else if (g.nodes[n].atomicNumber == 6) {
                                        cCount++;
                                    } else {
                                        validPyrrolizine = false;
                                        break;
                                    }

                                    for (int nei : g.nodes[n].neighbors) {
                                        if (!allSystemNodes.count(nei)) {
                                            validPyrrolizine = false;
                                            break;
                                        }
                                    }
                                    if (!validPyrrolizine) break;
                                }

                                if (validPyrrolizine && nCount == 1 && cCount == 7) {
                                    for (const auto &gb : g.bonds) {
                                        if (allSystemNodes.count(gb.u) && allSystemNodes.count(gb.v)) {
                                            if (gb.order != 1 && gb.order != 2 && gb.order != 4) {
                                                validPyrrolizine = false;
                                                break;
                                            }
                                        }
                                    }
                                } else {
                                    validPyrrolizine = false;
                                }

                                if (validPyrrolizine) {
                                    if (g.nodes[bhA].totalH != 0 || g.nodes[bhB].totalH != 0) {
                                        validPyrrolizine = false;
                                    } else {
                                        int countH1 = 0;
                                        int countH2 = 0;
                                        for (int n : allSystemNodes) {
                                            if (n == bhA || n == bhB) continue;
                                            if (g.nodes[n].totalH == 1) countH1++;
                                            else if (g.nodes[n].totalH == 2) countH2++;
                                            else {
                                                validPyrrolizine = false;
                                                break;
                                            }
                                        }
                                        if (countH1 != 5 || countH2 != 1) {
                                            validPyrrolizine = false;
                                        }
                                    }
                                }

                                if (validPyrrolizine) {
                                    return {true, "3H-pyrrolizine", ""};
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Phase 35: General Two-Heterocycle Fusion Nomenclature ---
    if (ringCount == 2) {
        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter >= 0) {
            std::vector<std::set<int>> sssrRings;
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        int idx = indigoIndex(ringAtomHandle);
                        rAtoms.insert(idx);
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            if (sssrRings.size() == 2 &&
                (sssrRings[0].size() == 5 || sssrRings[0].size() == 6) &&
                (sssrRings[1].size() == 5 || sssrRings[1].size() == 6)) {
                
                std::set<int> ring1Nodes, ring2Nodes;
                for (int idx : sssrRings[0]) {
                    if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
                }
                for (int idx : sssrRings[1]) {
                    if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
                }

                if ((ring1Nodes.size() == 5 || ring1Nodes.size() == 6) &&
                    (ring2Nodes.size() == 5 || ring2Nodes.size() == 6)) {

                    std::vector<int> sharedNodes;
                    for (int n : ring1Nodes) {
                        if (ring2Nodes.count(n)) sharedNodes.push_back(n);
                    }

                    if (sharedNodes.size() == 2) {
                        int bhA = sharedNodes[0];
                        int bhB = sharedNodes[1];
                        bool bhBonded = false;
                        for (int nei : g.nodes[bhA].neighbors) {
                            if (nei == bhB) { bhBonded = true; break; }
                        }

                        // Both fusion atoms must be Carbon (atomicNumber == 6)
                        if (bhBonded && g.nodes[bhA].atomicNumber == 6 && g.nodes[bhB].atomicNumber == 6) {
                            std::set<int> allSystemNodes = ring1Nodes;
                            allSystemNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

                            if (allSystemNodes.size() == ring1Nodes.size() + ring2Nodes.size() - 2 &&
                                g.nodes.size() == allSystemNodes.size()) {

                                auto buildCycle = [&](const std::set<int> &rNodes) -> std::vector<int> {
                                    int rSize = static_cast<int>(rNodes.size());
                                    std::vector<int> cycle;
                                    int startNode = *rNodes.begin();
                                    cycle.push_back(startNode);
                                    int current = startNode;
                                    int previous = -1;
                                    for (int step = 1; step < rSize; ++step) {
                                        int nextNode = -1;
                                        for (int nei : g.nodes[current].neighbors) {
                                            if (rNodes.count(nei) && nei != previous) {
                                                if (step == rSize - 1) {
                                                    bool connectedToStart = false;
                                                    for (int startNei : g.nodes[nei].neighbors) {
                                                        if (startNei == startNode) { connectedToStart = true; break; }
                                                    }
                                                    if (!connectedToStart) continue;
                                                }
                                                nextNode = nei;
                                                break;
                                            }
                                        }
                                        if (nextNode == -1) break;
                                        previous = current;
                                        current = nextNode;
                                        cycle.push_back(current);
                                    }
                                    if (static_cast<int>(cycle.size()) != rSize) return {};
                                    return cycle;
                                };

                                std::vector<int> cycle1 = buildCycle(ring1Nodes);
                                std::vector<int> cycle2 = buildCycle(ring2Nodes);

                                if (cycle1.size() == ring1Nodes.size() && cycle2.size() == ring2Nodes.size()) {
                                    std::vector<int> rHetero1, rHetero2;
                                    for (int n : ring1Nodes) {
                                        if (g.nodes[n].atomicNumber != 6) rHetero1.push_back(n);
                                    }
                                    for (int n : ring2Nodes) {
                                        if (g.nodes[n].atomicNumber != 6) rHetero2.push_back(n);
                                    }

                                    RingType type1, type2;
                                    QString dummyRoot1, dummyRoot2, classErr1, classErr2;
                                    bool c1Ok = classifyMonocyclicHeteroRing(g, rHetero1, static_cast<int>(ring1Nodes.size()), cycle1, type1, dummyRoot1, classErr1);
                                    bool c2Ok = classifyMonocyclicHeteroRing(g, rHetero2, static_cast<int>(ring2Nodes.size()), cycle2, type2, dummyRoot2, classErr2);

                                    auto isAllowedType = [](RingType t) {
                                        return t == RingType::FURAN || t == RingType::THIOPHENE ||
                                               t == RingType::PYRIDINE || t == RingType::PYRIMIDINE ||
                                               t == RingType::PYRIDAZINE || t == RingType::PYRAZINE ||
                                               t == RingType::OXAZOLE || t == RingType::ISOXAZOLE ||
                                               t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE ||
                                               t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE ||
                                               t == RingType::PYRROLE || t == RingType::IMIDAZOLE ||
                                               t == RingType::PYRAZOLE || t == RingType::SELENOPHENE ||
                                               t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE;
                                    };

                                    auto isNHType = [](RingType t) {
                                        return t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE;
                                    };

                                    if (c1Ok && c2Ok && isAllowedType(type1) && isAllowedType(type2)) {
                                        // Unsubstituted check: H count
                                        bool type1IsNH = isNHType(type1);
                                        bool type2IsNH = isNHType(type2);
                                        bool validH = true;

                                        if (type1IsNH && type2IsNH) {
                                            // Spec item 4: Only ONE component ring can be pyrrole/imidazole/pyrazole
                                            validH = false;
                                        } else {
                                            int nhNode = -1;
                                            for (int n : allSystemNodes) {
                                                if (n == bhA || n == bhB) {
                                                    if (g.nodes[n].atomicNumber != 6 || g.nodes[n].totalH != 0) { validH = false; break; }
                                                } else if (g.nodes[n].atomicNumber != 6) {
                                                    if (g.nodes[n].atomicNumber == 7 && g.nodes[n].totalH >= 1) {
                                                        if (nhNode != -1) { validH = false; break; }
                                                        if (g.nodes[n].totalH > 1) { validH = false; break; }
                                                        nhNode = n;
                                                    } else {
                                                        if (g.nodes[n].totalH > 0) {
                                                            validH = false;
                                                            break;
                                                        }
                                                    }
                                                } else {
                                                    if (g.nodes[n].totalH != 1) { validH = false; break; }
                                                }
                                            }
                                            if (validH) {
                                                if ((type1IsNH || type2IsNH) && nhNode == -1) {
                                                    validH = false;
                                                }
                                                if (!type1IsNH && !type2IsNH && nhNode != -1) {
                                                    validH = false;
                                                }
                                                if (type1IsNH && !ring1Nodes.count(nhNode)) {
                                                    validH = false;
                                                }
                                                if (type2IsNH && !ring2Nodes.count(nhNode)) {
                                                    validH = false;
                                                }
                                            }
                                        }


                                        if (validH) {
                                            // Numbering-candidate generator for a monocyclic component, anchored to
                                            // its own heteroatom positions per its Hantzsch-Widman/retained-name
                                            // numbering convention. Used both to find the chosen base component's
                                            // fusion-letter position (below) and, before base choice is even made,
                                            // by FR-2.3/P-25.3.2.4 rule (j) to compare each ring's own bridgehead
                                            // (fusion carbon) locants as the final base-vs-attached tie-break.
                                            auto getCandidates = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle) {
                                                std::vector<std::vector<int>> cands;
                                                if (t == RingType::FURAN || t == RingType::THIOPHENE || t == RingType::SELENOPHENE || t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE || t == RingType::PYRIDINE || t == RingType::PYRROLE) {
                                                    if (hNodes.size() == 1) {
                                                        int hNode = hNodes[0];
                                                        int hIdx = -1;
                                                        for (int i = 0; i < rSize; ++i) {
                                                            if (rCycle[i] == hNode) { hIdx = i; break; }
                                                        }
                                                        if (hIdx != -1) {
                                                            std::vector<int> fwd(rSize), bwd(rSize);
                                                            for (int i = 0; i < rSize; ++i) {
                                                                fwd[i] = rCycle[(hIdx + i) % rSize];
                                                                bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                            }
                                                            cands.push_back(fwd);
                                                            cands.push_back(bwd);
                                                        }
                                                    }
                                                } else if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE) {
                                                    if (hNodes.size() == 2) {
                                                        int n1 = hNodes[0];
                                                        int n2 = hNodes[1];
                                                        int idx1 = -1, idx2 = -1;
                                                        for (int i = 0; i < rSize; ++i) {
                                                            if (rCycle[i] == n1) idx1 = i;
                                                            if (rCycle[i] == n2) idx2 = i;
                                                        }
                                                        if (idx1 != -1 && idx2 != -1) {
                                                            int reqOtherIdx = (t == RingType::PYRIDAZINE) ? 1 : ((t == RingType::PYRIMIDINE) ? 2 : 3);
                                                            std::vector<int> fwd1(rSize), bwd1(rSize);
                                                            for (int i = 0; i < rSize; ++i) {
                                                                fwd1[i] = rCycle[(idx1 + i) % rSize];
                                                                bwd1[i] = rCycle[(idx1 - i + rSize) % rSize];
                                                            }
                                                            if (fwd1[reqOtherIdx] == n2) cands.push_back(fwd1);
                                                            if (bwd1[reqOtherIdx] == n2) cands.push_back(bwd1);

                                                            std::vector<int> fwd2(rSize), bwd2(rSize);
                                                            for (int i = 0; i < rSize; ++i) {
                                                                fwd2[i] = rCycle[(idx2 + i) % rSize];
                                                                bwd2[i] = rCycle[(idx2 - i + rSize) % rSize];
                                                            }
                                                            if (fwd2[reqOtherIdx] == n1) cands.push_back(fwd2);
                                                            if (bwd2[reqOtherIdx] == n1) cands.push_back(bwd2);
                                                        }
                                                    }
                                                } else if (t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) {
                                                    if (hNodes.size() == 2) {
                                                        int hNH = -1, hN = -1;
                                                        if (g.nodes[hNodes[0]].totalH >= 1) {
                                                            hNH = hNodes[0];
                                                            hN = hNodes[1];
                                                        } else {
                                                            hNH = hNodes[1];
                                                            hN = hNodes[0];
                                                        }
                                                        int hIdx = -1;
                                                        for (int i = 0; i < rSize; ++i) {
                                                            if (rCycle[i] == hNH) { hIdx = i; break; }
                                                        }
                                                        if (hIdx != -1) {
                                                            std::vector<int> fwd(rSize), bwd(rSize);
                                                            for (int i = 0; i < rSize; ++i) {
                                                                fwd[i] = rCycle[(hIdx + i) % rSize];
                                                                bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                            }
                                                            int reqOtherIdx = (t == RingType::IMIDAZOLE) ? 2 : 1;
                                                            if (fwd[reqOtherIdx] == hN) cands.push_back(fwd);
                                                            if (bwd[reqOtherIdx] == hN) cands.push_back(bwd);
                                                        }
                                                    }
                                                } else if (t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE) {
                                                    if (hNodes.size() == 2) {
                                                        int hOS = -1, hN = -1;
                                                        int z0 = g.nodes[hNodes[0]].atomicNumber;
                                                        if (z0 == 8 || z0 == 16 || z0 == 34) {
                                                            hOS = hNodes[0];
                                                            hN = hNodes[1];
                                                        } else {
                                                            hOS = hNodes[1];
                                                            hN = hNodes[0];
                                                        }
                                                        int hIdx = -1;
                                                        for (int i = 0; i < rSize; ++i) {
                                                            if (rCycle[i] == hOS) { hIdx = i; break; }
                                                        }
                                                        if (hIdx != -1) {
                                                            std::vector<int> fwd(rSize), bwd(rSize);
                                                            for (int i = 0; i < rSize; ++i) {
                                                                fwd[i] = rCycle[(hIdx + i) % rSize];
                                                                bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                            }
                                                            int reqNIdx = (t == RingType::ISOXAZOLE || t == RingType::ISOTHIAZOLE) ? 1 : 2;
                                                            if (fwd[reqNIdx] == hN) cands.push_back(fwd);
                                                            if (bwd[reqNIdx] == hN) cands.push_back(bwd);
                                                        }
                                                    }
                                                }
                                                return cands;
                                            };

                                            // FR-2.3/P-25.3.2.4 rule (j): a component with the lower locants for
                                            // its own peripheral fusion (bridgehead) carbon atoms, using each
                                            // ring's own candidate numbering(s) from getCandidates above. Computed
                                            // up front (regardless of whether earlier rules already decide the
                                            // case) since it's cheap and only consulted if everything through
                                            // rule (i) ties.
                                            auto getFusionLocants = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle) -> std::pair<int, int> {
                                                std::vector<std::vector<int>> cands = getCandidates(t, hNodes, rSize, rCycle);
                                                std::pair<int, int> best = {999, 999};
                                                for (const auto &cand : cands) {
                                                    int posA = -1, posB = -1;
                                                    for (int i = 0; i < rSize; ++i) {
                                                        if (cand[i] == bhA) posA = i;
                                                        if (cand[i] == bhB) posB = i;
                                                    }
                                                    if (posA != -1 && posB != -1) {
                                                        int locA = posA + 1;
                                                        int locB = posB + 1;
                                                        std::pair<int, int> pairVal = {std::min(locA, locB), std::max(locA, locB)};
                                                        if (pairVal < best) best = pairVal;
                                                    }
                                                }
                                                return best;
                                            };

                                            // Seniority comparison: decide base (ring A) vs attached (ring B)
                                            auto getRankHetero = [](RingType t) {
                                                if (t == RingType::PYRIDINE || t == RingType::PYRIMIDINE ||
                                                    t == RingType::PYRIDAZINE || t == RingType::PYRAZINE ||
                                                    t == RingType::OXAZOLE || t == RingType::ISOXAZOLE ||
                                                    t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE ||
                                                    t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE ||
                                                    t == RingType::PYRROLE || t == RingType::IMIDAZOLE ||
                                                    t == RingType::PYRAZOLE) return 1; // N-containing
                                                if (t == RingType::FURAN) return 2; // O-containing
                                                if (t == RingType::THIOPHENE) return 3; // S-containing
                                                if (t == RingType::SELENOPHENE) return 4; // Se-containing
                                                if (t == RingType::TELLUROPHENE) return 5; // Te-containing
                                                if (t == RingType::PHOSPHININE) return 6; // P-containing
                                                return 99;
                                            };

                                            auto getNumHetero = [](RingType t) {
                                                if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE ||
                                                    t == RingType::PYRAZINE || t == RingType::OXAZOLE ||
                                                    t == RingType::ISOXAZOLE || t == RingType::THIAZOLE ||
                                                    t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE ||
                                                    t == RingType::ISOSELENAZOLE || t == RingType::IMIDAZOLE ||
                                                    t == RingType::PYRAZOLE) return 2;
                                                return 1;
                                            };

                                            int rHetero1Rank = getRankHetero(type1);
                                            int rHetero2Rank = getRankHetero(type2);
                                            int size1 = static_cast<int>(ring1Nodes.size());
                                            int size2 = static_cast<int>(ring2Nodes.size());
                                            int nHet1 = getNumHetero(type1);
                                            int nHet2 = getNumHetero(type2);

                                            int baseChoice = 0; // 1 -> ring1 is base, 2 -> ring2 is base
                                            if (type1 == type2) {
                                                baseChoice = 1; // Moot / identical components
                                            } else {
                                                if (rHetero1Rank < rHetero2Rank) baseChoice = 1;
                                                else if (rHetero2Rank < rHetero1Rank) baseChoice = 2;
                                                else {
                                                    // FR-2.3 rule (c): larger ring size wins
                                                    if (size1 > size2) baseChoice = 1;
                                                    else if (size2 > size1) baseChoice = 2;
                                                    else {
                                                        // FR-2.3 rule (d): greater heteroatom count wins
                                                        if (nHet1 > nHet2) baseChoice = 1;
                                                        else if (nHet2 > nHet1) baseChoice = 2;
                                                        else {
                                                            // FR-2.3 rule (e): greater variety of heteroatoms wins
                                                            auto getVariety = [](const std::vector<int> &hNodes, const Graph &gr) {
                                                                std::set<int> elems;
                                                                for (int n : hNodes) elems.insert(gr.nodes[n].atomicNumber);
                                                                return static_cast<int>(elems.size());
                                                            };
                                                            int variety1 = getVariety(rHetero1, g);
                                                            int variety2 = getVariety(rHetero2, g);
                                                            if (variety1 > variety2) baseChoice = 1;
                                                            else if (variety2 > variety1) baseChoice = 2;
                                                            else {
                                                                // FR-2.3 rule (f): heteroatoms highest in the
                                                                // Hantzsch-Widman seniority order F,Cl,Br,I,O,S,Se,Te,N,P,...
                                                                // win. Among this codebase's supported elements
                                                                // (N,O,S), O outranks S outranks N.
                                                                auto altRank = [](int z) {
                                                                    switch (z) {
                                                                        case 9: return 1;  // F
                                                                        case 17: return 2; // Cl
                                                                        case 35: return 3; // Br
                                                                        case 53: return 4; // I
                                                                        case 8: return 5;  // O
                                                                        case 16: return 6; // S
                                                                        case 34: return 7; // Se
                                                                        case 52: return 8; // Te
                                                                        case 7: return 9;  // N
                                                                        case 15: return 10; // P
                                                                        default: return 99;
                                                                    }
                                                                };
                                                                auto getTopAltRank = [&](const std::vector<int> &hNodes) {
                                                                    int best = 99;
                                                                    for (int n : hNodes) best = std::min(best, altRank(g.nodes[n].atomicNumber));
                                                                    return best;
                                                                };
                                                                int alt1 = getTopAltRank(rHetero1);
                                                                int alt2 = getTopAltRank(rHetero2);
                                                                if (alt1 < alt2) baseChoice = 1;
                                                                else if (alt2 < alt1) baseChoice = 2;
                                                                else {
                                                                    // FR-2.3 rule (g), preferred orientation
                                                                    // (FR-5.2), has no discriminating power for a
                                                                    // strictly two-ring system - orientation only
                                                                    // distinguishes components when 3+ rings compete
                                                                    // for row/quadrant placement. Skipped (always
                                                                    // tied here), proceed straight to rule (h).

                                                                    // FR-2.3 rule (h): lower locants for heteroatoms,
                                                                    // using each ring's own independent (pre-fusion)
                                                                    // traditional numbering - e.g. pyridazine's N,N
                                                                    // are 1,2 while pyrazine's are 1,4, so pyridazine
                                                                    // wins even though both are all-N, same size,
                                                                    // same heteroatom count. (Official worked example:
                                                                    // "pyrazino[2,3-d]pyridazine, pyridazine [1,2]
                                                                    // preferred to pyrazine [1,4]".)
                                                                    auto getOwnLocants = [](RingType t) -> std::vector<int> {
                                                                        switch (t) {
                                                                            case RingType::PYRIDINE:
                                                                            case RingType::PYRROLE:
                                                                            case RingType::FURAN:
                                                                            case RingType::THIOPHENE:
                                                                            case RingType::SELENOPHENE:
                                                                            case RingType::TELLUROPHENE:
                                                                            case RingType::PHOSPHININE:
                                                                                return {1};
                                                                            case RingType::PYRIDAZINE:
                                                                            case RingType::ISOXAZOLE:
                                                                            case RingType::ISOTHIAZOLE:
                                                                            case RingType::PYRAZOLE:
                                                                            case RingType::ISOSELENAZOLE:
                                                                                return {1, 2};
                                                                            case RingType::PYRIMIDINE:
                                                                            case RingType::OXAZOLE:
                                                                            case RingType::THIAZOLE:
                                                                            case RingType::IMIDAZOLE:
                                                                            case RingType::SELENAZOLE:
                                                                                return {1, 3};
                                                                            case RingType::PYRAZINE:
                                                                                return {1, 4};
                                                                            default:
                                                                                return {};
                                                                        }
                                                                    };
                                                                    std::vector<int> ownLoc1 = getOwnLocants(type1);
                                                                    std::vector<int> ownLoc2 = getOwnLocants(type2);
                                                                    if (ownLoc1 < ownLoc2) baseChoice = 1;
                                                                    else if (ownLoc2 < ownLoc1) baseChoice = 2;
                                                                    else {
                                                                        // FR-2.3/P-25.3.2.4 rule (i): if the
                                                                        // own-numbering locant sets are identical
                                                                        // (can only happen for ring types built
                                                                        // from the same heteroatom multiset, since
                                                                        // (e)/(f) already separated differing
                                                                        // element sets earlier), no further
                                                                        // element-identity difference remains to
                                                                        // compare position-by-position - this
                                                                        // branch is reachable in principle but not
                                                                        // by any currently supported ring-type
                                                                        // pair. Fall through to rule (j).

                                                                        // FR-2.3/P-25.3.2.4 rule (j) (the final
                                                                        // tie-break in the official list): a
                                                                        // component with the lower locants for its
                                                                        // own peripheral fusion (bridgehead) carbon
                                                                        // atoms. Official worked example:
                                                                        // "acephenanthyleno[5,4-k]aceanthrylene -
                                                                        // the locant 2a in aceanthrylene is lower
                                                                        // than 3a in acephenanthrylene."
                                                                        std::pair<int, int> fusionLoc1 = getFusionLocants(type1, rHetero1, size1, cycle1);
                                                                        std::pair<int, int> fusionLoc2 = getFusionLocants(type2, rHetero2, size2, cycle2);
                                                                        if (fusionLoc1 < fusionLoc2) baseChoice = 1;
                                                                        else if (fusionLoc2 < fusionLoc1) baseChoice = 2;
                                                                        else {
                                                                            // Genuinely tied through every rule
                                                                            // (a)-(j) in the official list - no
                                                                            // further rule exists to break this.
                                                                            baseChoice = 0;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            if (baseChoice != 0) {
                                                RingType baseType = (baseChoice == 1) ? type1 : type2;
                                                RingType attType  = (baseChoice == 1) ? type2 : type1;
                                                const std::vector<int> &baseCycle = (baseChoice == 1) ? cycle1 : cycle2;
                                                const std::vector<int> &attCycle  = (baseChoice == 1) ? cycle2 : cycle1;
                                                const std::vector<int> &baseHetero = (baseChoice == 1) ? rHetero1 : rHetero2;
                                                const std::vector<int> &attHetero  = (baseChoice == 1) ? rHetero2 : rHetero1;
                                                int baseSize = static_cast<int>(baseCycle.size());
                                                int attSize  = static_cast<int>(attCycle.size());

                                                // getCandidates is defined earlier (before base-vs-attached choice
                                                // was decided) so FR-2.3/P-25.3.2.4 rule (j) could reuse it.
                                                std::vector<std::vector<int>> baseCands = getCandidates(baseType, baseHetero, baseSize, baseCycle);
                                                std::vector<std::vector<int>> attCands  = getCandidates(attType, attHetero, attSize, attCycle);

                                                if (!baseCands.empty() && !attCands.empty()) {
                                                    struct BaseInfo {
                                                        int letterIdx;
                                                        int nodeStart;
                                                        int nodeEnd;
                                                    };
                                                    std::vector<BaseInfo> validBaseInfos;
                                                    int minLetterIdx = 999;

                                                    for (const auto &bCand : baseCands) {
                                                        int posA = -1, posB = -1;
                                                        for (int i = 0; i < baseSize; ++i) {
                                                            if (bCand[i] == bhA) posA = i;
                                                            if (bCand[i] == bhB) posB = i;
                                                        }
                                                        if (posA != -1 && posB != -1) {
                                                            int locA = posA + 1;
                                                            int locB = posB + 1;
                                                            int minL = std::min(locA, locB);
                                                            int maxL = std::max(locA, locB);
                                                            int letterIdx = -1;
                                                            int nStart = -1, nEnd = -1;
                                                            if (maxL == minL + 1) {
                                                                letterIdx = minL - 1;
                                                                nStart = (locA == minL) ? bhA : bhB;
                                                                nEnd = (locA == minL) ? bhB : bhA;
                                                            } else if (minL == 1 && maxL == baseSize) {
                                                                letterIdx = baseSize - 1;
                                                                nStart = (locA == baseSize) ? bhA : bhB;
                                                                nEnd = (locA == 1) ? bhA : bhB;
                                                            }
                                                            if (letterIdx != -1) {
                                                                if (letterIdx < minLetterIdx) {
                                                                    minLetterIdx = letterIdx;
                                                                    validBaseInfos.clear();
                                                                    validBaseInfos.push_back({letterIdx, nStart, nEnd});
                                                                } else if (letterIdx == minLetterIdx) {
                                                                    validBaseInfos.push_back({letterIdx, nStart, nEnd});
                                                                }
                                                            }
                                                        }
                                                    }

                                                        if (minLetterIdx < 999 && !validBaseInfos.empty()) {
                                                            std::pair<int, int> bestAttPair = {999, 999};

                                                            for (const auto &bInfo : validBaseInfos) {
                                                            for (const auto &aCand : attCands) {
                                                                int posStart = -1, posEnd = -1;
                                                                for (int i = 0; i < attSize; ++i) {
                                                                    if (aCand[i] == bInfo.nodeStart) posStart = i;
                                                                    if (aCand[i] == bInfo.nodeEnd) posEnd = i;
                                                                }
                                                                if (posStart != -1 && posEnd != -1) {
                                                                    int attLoc1 = posStart + 1;
                                                                    int attLoc2 = posEnd + 1;
                                                                    std::pair<int, int> pairVal = {attLoc1, attLoc2};
                                                                    if (pairVal < bestAttPair) {
                                                                        bestAttPair = pairVal;
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        if (bestAttPair.first < 999) {
                                                            auto getFusionPrefix = [](RingType t) -> QString { return getFusionPrefixShared(t); };
                                                            auto getBaseName = [](RingType t) -> QString { return getBaseNameShared(t); };

                                                            char baseLetterChar = 'a' + minLetterIdx;
                                                            QString resultName = QString("%1[%2,%3-%4]%5")
                                                                                    .arg(getFusionPrefix(attType))
                                                                                    .arg(bestAttPair.first)
                                                                                    .arg(bestAttPair.second)
                                                                                    .arg(baseLetterChar)
                                                                                    .arg(getBaseName(baseType));

                                                            if (type1IsNH || type2IsNH) {
                                                                int nhNode = -1;
                                                                for (int n : allSystemNodes) {
                                                                    if (g.nodes[n].atomicNumber == 7 && g.nodes[n].totalH >= 1) {
                                                                        nhNode = n;
                                                                        break;
                                                                    }
                                                                }
                                                                if (nhNode != -1) {
                                                                    std::map<int, QString> periphMap = computePeripheralNumbering(g, ring1Nodes, ring2Nodes, bhA, bhB, {}, true);
                                                                    if (!periphMap.count(nhNode)) {
                                                                        return {false, "", "Failed to compute peripheral locant for indicated hydrogen."};
                                                                    }
                                                                    QString locStr = periphMap[nhNode];
                                                                    bool ok = false;
                                                                    locStr.toInt(&ok);
                                                                    if (!ok) {
                                                                        return {false, "", "Indicated hydrogen locant is letter-suffixed."};
                                                                    }
                                                                    resultName = locStr + "H-" + resultName;
                                                                }
                                                            }

                                                            return {true, resultName, ""};
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


// --- Phase 44: Three-Heterocycle Fusion Chain Nomenclature ---
    if (ringCount == 3) {
        int sssrIter = indigoIterateSSSR(mol);
        bool allDisjoint = true;
        std::vector<std::set<int>> sssrRings;
        if (sssrIter >= 0) {
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        rAtoms.insert(indigoIndex(ringAtomHandle));
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);

            for (size_t i = 0; i < sssrRings.size() && allDisjoint; ++i) {
                for (size_t j = i + 1; j < sssrRings.size(); ++j) {
                    for (int a : sssrRings[i]) {
                        if (sssrRings[j].count(a)) {
                            allDisjoint = false;
                            break;
                        }
                    }
                    if (!allDisjoint) break;
                }
            }
        } else {
            allDisjoint = false;
        }

        if (!allDisjoint) {
            bool handled = false;
            if (sssrRings.size() == 3) {
                int shared[3][3] = {0};
                std::vector<int> sharedNodesPairs[3][3];
                for (int i = 0; i < 3; ++i) {
                    for (int j = i + 1; j < 3; ++j) {
                        for (int a : sssrRings[i]) {
                            if (sssrRings[j].count(a)) {
                                if (indigoToGraphIdx.count(a)) {
                                    sharedNodesPairs[i][j].push_back(indigoToGraphIdx[a]);
                                    sharedNodesPairs[j][i].push_back(indigoToGraphIdx[a]);
                                }
                            }
                        }
                        shared[i][j] = shared[j][i] = sharedNodesPairs[i][j].size();
                    }
                }

                int centerRingIdx = -1;
                for (int i = 0; i < 3; ++i) {
                    int neighbors = 0;
                    for (int j = 0; j < 3; ++j) {
                        if (shared[i][j] == 2) neighbors++;
                    }
                    if (neighbors == 2) {
                        centerRingIdx = i;
                    }
                }

                if (centerRingIdx != -1) {
                    int end1Idx = -1, end2Idx = -1;
                    for (int i = 0; i < 3; ++i) {
                        if (i != centerRingIdx) {
                            if (end1Idx == -1) end1Idx = i;
                            else end2Idx = i;
                        }
                    }

                    if (shared[end1Idx][end2Idx] == 0) {
                        auto getRingNodes = [&](int rIdx) {
                            std::set<int> res;
                            for (int idx : sssrRings[rIdx]) {
                                if (indigoToGraphIdx.count(idx)) res.insert(indigoToGraphIdx[idx]);
                            }
                            return res;
                        };

                        std::set<int> nodes[3] = {getRingNodes(0), getRingNodes(1), getRingNodes(2)};
                        
                        bool all5or6 = true;
                        for (int i=0; i<3; ++i) {
                            if (nodes[i].size() != 5 && nodes[i].size() != 6) all5or6 = false;
                        }

                        if (all5or6) {
                            bool allFusionsCarbon = true;
                            for (int endIdx : {end1Idx, end2Idx}) {
                                const auto& pair = sharedNodesPairs[centerRingIdx][endIdx];
                                if (pair.size() == 2) {
                                    int a = pair[0], b = pair[1];
                                    if (g.nodes[a].atomicNumber != 6 || g.nodes[b].atomicNumber != 6) allFusionsCarbon = false;
                                    bool bonded = false;
                                    for (int nei : g.nodes[a].neighbors) if (nei == b) bonded = true;
                                    if (!bonded) allFusionsCarbon = false;
                                } else {
                                    allFusionsCarbon = false;
                                }
                            }

                            if (allFusionsCarbon) {
                                auto buildCycle = [&](const std::set<int> &rNodes) -> std::vector<int> {
                                    int rSize = static_cast<int>(rNodes.size());
                                    std::vector<int> cycle;
                                    int startNode = *rNodes.begin();
                                    cycle.push_back(startNode);
                                    int current = startNode;
                                    int previous = -1;
                                    for (int step = 1; step < rSize; ++step) {
                                        int nextNode = -1;
                                        for (int nei : g.nodes[current].neighbors) {
                                            if (rNodes.count(nei) && nei != previous) {
                                                if (step == rSize - 1) {
                                                    bool connectedToStart = false;
                                                    for (int startNei : g.nodes[nei].neighbors) {
                                                        if (startNei == startNode) { connectedToStart = true; break; }
                                                    }
                                                    if (!connectedToStart) continue;
                                                }
                                                nextNode = nei;
                                                break;
                                            }
                                        }
                                        if (nextNode == -1) break;
                                        previous = current;
                                        current = nextNode;
                                        cycle.push_back(current);
                                    }
                                    if (static_cast<int>(cycle.size()) != rSize) return {};
                                    return cycle;
                                };

                                std::vector<int> cycles[3];
                                std::vector<int> rHetero[3];
                                RingType types[3];
                                bool classOk[3] = {false};
                                for (int i = 0; i < 3; ++i) {
                                    cycles[i] = buildCycle(nodes[i]);
                                    for (int n : nodes[i]) {
                                        if (g.nodes[n].atomicNumber != 6) rHetero[i].push_back(n);
                                    }
                                    if (cycles[i].size() == nodes[i].size()) {
                                        QString d1, d2;
                                        classOk[i] = classifyMonocyclicHeteroRing(g, rHetero[i], static_cast<int>(nodes[i].size()), cycles[i], types[i], d1, d2);
                                    }
                                }

                                auto isAllowedType = [](RingType t) {
                                    return t == RingType::FURAN || t == RingType::THIOPHENE ||
                                           t == RingType::PYRIDINE || t == RingType::PYRIMIDINE ||
                                           t == RingType::PYRIDAZINE || t == RingType::PYRAZINE ||
                                           t == RingType::OXAZOLE || t == RingType::ISOXAZOLE ||
                                           t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE ||
                                           t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE ||
                                           t == RingType::PYRROLE || t == RingType::IMIDAZOLE ||
                                           t == RingType::PYRAZOLE || t == RingType::SELENOPHENE ||
                                           t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE;
                                };

                                auto isNHType = [](RingType t) {
                                    return t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE;
                                };

                                if (classOk[0] && classOk[1] && classOk[2] && 
                                    isAllowedType(types[0]) && isAllowedType(types[1]) && isAllowedType(types[2])) {
                                    
                                    auto getCandidates = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle) {
                                        std::vector<std::vector<int>> cands;
                                        if (t == RingType::FURAN || t == RingType::THIOPHENE || t == RingType::SELENOPHENE || t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE || t == RingType::PYRIDINE || t == RingType::PYRROLE) {
                                            if (hNodes.size() == 1) {
                                                int hNode = hNodes[0];
                                                int hIdx = -1;
                                                for (int i = 0; i < rSize; ++i) {
                                                    if (rCycle[i] == hNode) { hIdx = i; break; }
                                                }
                                                if (hIdx != -1) {
                                                    std::vector<int> fwd(rSize), bwd(rSize);
                                                    for (int i = 0; i < rSize; ++i) {
                                                        fwd[i] = rCycle[(hIdx + i) % rSize];
                                                        bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                    }
                                                    cands.push_back(fwd);
                                                    cands.push_back(bwd);
                                                }
                                            }
                                        } else if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE) {
                                            if (hNodes.size() == 2) {
                                                int n1 = hNodes[0];
                                                int n2 = hNodes[1];
                                                int idx1 = -1, idx2 = -1;
                                                for (int i = 0; i < rSize; ++i) {
                                                    if (rCycle[i] == n1) idx1 = i;
                                                    if (rCycle[i] == n2) idx2 = i;
                                                }
                                                if (idx1 != -1 && idx2 != -1) {
                                                    int reqOtherIdx = (t == RingType::PYRIDAZINE) ? 1 : ((t == RingType::PYRIMIDINE) ? 2 : 3);
                                                    std::vector<int> fwd1(rSize), bwd1(rSize);
                                                    for (int i = 0; i < rSize; ++i) {
                                                        fwd1[i] = rCycle[(idx1 + i) % rSize];
                                                        bwd1[i] = rCycle[(idx1 - i + rSize) % rSize];
                                                    }
                                                    if (fwd1[reqOtherIdx] == n2) cands.push_back(fwd1);
                                                    if (bwd1[reqOtherIdx] == n2) cands.push_back(bwd1);

                                                    std::vector<int> fwd2(rSize), bwd2(rSize);
                                                    for (int i = 0; i < rSize; ++i) {
                                                        fwd2[i] = rCycle[(idx2 + i) % rSize];
                                                        bwd2[i] = rCycle[(idx2 - i + rSize) % rSize];
                                                    }
                                                    if (fwd2[reqOtherIdx] == n1) cands.push_back(fwd2);
                                                    if (bwd2[reqOtherIdx] == n1) cands.push_back(bwd2);
                                                }
                                            }
                                        } else if (t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) {
                                            if (hNodes.size() == 2) {
                                                int hNH = -1, hN = -1;
                                                if (g.nodes[hNodes[0]].totalH >= 1) {
                                                    hNH = hNodes[0];
                                                    hN = hNodes[1];
                                                } else {
                                                    hNH = hNodes[1];
                                                    hN = hNodes[0];
                                                }
                                                int hIdx = -1;
                                                for (int i = 0; i < rSize; ++i) {
                                                    if (rCycle[i] == hNH) { hIdx = i; break; }
                                                }
                                                if (hIdx != -1) {
                                                    std::vector<int> fwd(rSize), bwd(rSize);
                                                    for (int i = 0; i < rSize; ++i) {
                                                        fwd[i] = rCycle[(hIdx + i) % rSize];
                                                        bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                    }
                                                    int reqOtherIdx = (t == RingType::IMIDAZOLE) ? 2 : 1;
                                                    if (fwd[reqOtherIdx] == hN) cands.push_back(fwd);
                                                    if (bwd[reqOtherIdx] == hN) cands.push_back(bwd);
                                                }
                                            }
                                        } else if (t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE) {
                                            if (hNodes.size() == 2) {
                                                int hOS = -1, hN = -1;
                                                int z0 = g.nodes[hNodes[0]].atomicNumber;
                                                if (z0 == 8 || z0 == 16) {
                                                    hOS = hNodes[0];
                                                    hN = hNodes[1];
                                                } else {
                                                    hOS = hNodes[1];
                                                    hN = hNodes[0];
                                                }
                                                int hIdx = -1;
                                                for (int i = 0; i < rSize; ++i) {
                                                    if (rCycle[i] == hOS) { hIdx = i; break; }
                                                }
                                                if (hIdx != -1) {
                                                    std::vector<int> fwd(rSize), bwd(rSize);
                                                    for (int i = 0; i < rSize; ++i) {
                                                        fwd[i] = rCycle[(hIdx + i) % rSize];
                                                        bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                                    }
                                                    int reqNIdx = (t == RingType::ISOXAZOLE || t == RingType::ISOTHIAZOLE) ? 1 : 2;
                                                    if (fwd[reqNIdx] == hN) cands.push_back(fwd);
                                                    if (bwd[reqNIdx] == hN) cands.push_back(bwd);
                                                }
                                            }
                                        }
                                        return cands;
                                    };

                                    auto getFusionLocants = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle, int bhA, int bhB) -> std::pair<int, int> {
                                        std::vector<std::vector<int>> cands = getCandidates(t, hNodes, rSize, rCycle);
                                        std::pair<int, int> best = {999, 999};
                                        for (const auto &cand : cands) {
                                            int posA = -1, posB = -1;
                                            for (int i = 0; i < rSize; ++i) {
                                                if (cand[i] == bhA) posA = i;
                                                if (cand[i] == bhB) posB = i;
                                            }
                                            if (posA != -1 && posB != -1) {
                                                int locA = posA + 1;
                                                int locB = posB + 1;
                                                std::pair<int, int> pairVal = {std::min(locA, locB), std::max(locA, locB)};
                                                if (pairVal < best) best = pairVal;
                                            }
                                        }
                                        return best;
                                    };

                                    auto compareSeniority = [&](int idx1, int idx2) -> int {
                                        RingType t1 = types[idx1];
                                        RingType t2 = types[idx2];
                                        int size1 = nodes[idx1].size();
                                        int size2 = nodes[idx2].size();
                                        const auto &rH1 = rHetero[idx1];
                                        const auto &rH2 = rHetero[idx2];
                                        
                                        if (t1 == t2) return idx1; // tie goes to first

                                        auto getRankHetero = [](RingType t) {
                                            if (t == RingType::PYRIDINE || t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE || t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE || t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) return 1;
                                            if (t == RingType::FURAN) return 2;
                                            if (t == RingType::THIOPHENE) return 3;
                                            if (t == RingType::SELENOPHENE) return 4;
                                            if (t == RingType::TELLUROPHENE) return 5;
                                            if (t == RingType::PHOSPHININE) return 6;
                                            return 99;
                                        };

                                        auto getNumHetero = [](RingType t) {
                                            if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE || t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) return 2;
                                            return 1;
                                        };

                                        int rH1Rank = getRankHetero(t1), rH2Rank = getRankHetero(t2);
                                        if (rH1Rank < rH2Rank) return idx1;
                                        if (rH2Rank < rH1Rank) return idx2;

                                        if (size1 > size2) return idx1;
                                        if (size2 > size1) return idx2;

                                        int nHet1 = getNumHetero(t1), nHet2 = getNumHetero(t2);
                                        if (nHet1 > nHet2) return idx1;
                                        if (nHet2 > nHet1) return idx2;

                                        auto getVariety = [&](const std::vector<int> &hNodes) {
                                            std::set<int> elems;
                                            for (int n : hNodes) elems.insert(g.nodes[n].atomicNumber);
                                            return static_cast<int>(elems.size());
                                        };
                                        int v1 = getVariety(rH1), v2 = getVariety(rH2);
                                        if (v1 > v2) return idx1;
                                        if (v2 > v1) return idx2;

                                        auto altRank = [](int z) {
                                            switch (z) { case 9: return 1; case 17: return 2; case 35: return 3; case 53: return 4; case 8: return 5; case 16: return 6; case 34: return 7; case 52: return 8; case 7: return 9; case 15: return 10; default: return 99; }
                                        };
                                        auto getTopAltRank = [&](const std::vector<int> &hNodes) {
                                            int best = 99;
                                            for (int n : hNodes) best = std::min(best, altRank(g.nodes[n].atomicNumber));
                                            return best;
                                        };
                                        int alt1 = getTopAltRank(rH1), alt2 = getTopAltRank(rH2);
                                        if (alt1 < alt2) return idx1;
                                        if (alt2 < alt1) return idx2;

                                        auto getOwnLocants = [](RingType t) -> std::vector<int> {
                                            switch (t) {
                                                case RingType::PYRIDINE: case RingType::PYRROLE: case RingType::FURAN: case RingType::THIOPHENE: case RingType::SELENOPHENE: case RingType::TELLUROPHENE: case RingType::PHOSPHININE: return {1};
                                                case RingType::PYRIDAZINE: case RingType::ISOXAZOLE: case RingType::ISOTHIAZOLE: case RingType::PYRAZOLE: case RingType::ISOSELENAZOLE: return {1, 2};
                                                case RingType::PYRIMIDINE: case RingType::OXAZOLE: case RingType::THIAZOLE: case RingType::IMIDAZOLE: case RingType::SELENAZOLE: return {1, 3};
                                                case RingType::PYRAZINE: return {1, 4};
                                                default: return {};
                                            }
                                        };
                                        std::vector<int> ownLoc1 = getOwnLocants(t1), ownLoc2 = getOwnLocants(t2);
                                        if (ownLoc1 < ownLoc2) return idx1;
                                        if (ownLoc2 < ownLoc1) return idx2;

                                        int bhA = sharedNodesPairs[idx1][idx2][0];
                                        int bhB = sharedNodesPairs[idx1][idx2][1];
                                        std::pair<int, int> fl1 = getFusionLocants(t1, rH1, size1, cycles[idx1], bhA, bhB);
                                        std::pair<int, int> fl2 = getFusionLocants(t2, rH2, size2, cycles[idx2], bhA, bhB);
                                        if (fl1 < fl2) return idx1;
                                        if (fl2 < fl1) return idx2;

                                        return idx1; // absolute tie
                                    };

                                    // Pairwise tournament: A vs B, winner vs C
                                    int w1 = compareSeniority(end1Idx, centerRingIdx);
                                    int baseChoice = compareSeniority(w1, end2Idx);

                                    if (baseChoice != centerRingIdx) {
                                        int baseIdx = baseChoice;
                                        int midIdx = centerRingIdx;
                                        int farIdx = (baseChoice == end1Idx) ? end2Idx : end1Idx;
                                        
                                        RingType baseType = types[baseIdx];
                                        RingType midType = types[midIdx];
                                        RingType farType = types[farIdx];
                                        
                                        int bhMidBaseA = sharedNodesPairs[midIdx][baseIdx][0];
                                        int bhMidBaseB = sharedNodesPairs[midIdx][baseIdx][1];
                                        
                                        int bhFarMidA = sharedNodesPairs[farIdx][midIdx][0];
                                        int bhFarMidB = sharedNodesPairs[farIdx][midIdx][1];
                                        
                                        std::vector<std::vector<int>> baseCands = getCandidates(baseType, rHetero[baseIdx], nodes[baseIdx].size(), cycles[baseIdx]);
                                        std::vector<std::vector<int>> midCands = getCandidates(midType, rHetero[midIdx], nodes[midIdx].size(), cycles[midIdx]);
                                        std::vector<std::vector<int>> farCands = getCandidates(farType, rHetero[farIdx], nodes[farIdx].size(), cycles[farIdx]);
                                        
                                        if (!baseCands.empty() && !midCands.empty() && !farCands.empty()) {
                                            auto getLetterAndNodes = [&](const std::vector<int>& bCand, int bhA, int bhB) -> std::tuple<int, int, int> {
                                                int posA = -1, posB = -1;
                                                for (int i = 0; i < (int)bCand.size(); ++i) {
                                                    if (bCand[i] == bhA) posA = i;
                                                    if (bCand[i] == bhB) posB = i;
                                                }
                                                if (posA != -1 && posB != -1) {
                                                    int locA = posA + 1;
                                                    int locB = posB + 1;
                                                    int minL = std::min(locA, locB);
                                                    int maxL = std::max(locA, locB);
                                                    int baseSize = bCand.size();
                                                    if (maxL == minL + 1) {
                                                        return {minL - 1, (locA == minL) ? bhA : bhB, (locA == minL) ? bhB : bhA};
                                                    } else if (minL == 1 && maxL == baseSize) {
                                                        return {baseSize - 1, (locA == baseSize) ? bhA : bhB, (locA == 1) ? bhA : bhB};
                                                    }
                                                }
                                                return {-1, -1, -1};
                                            };
                                            
                                            struct Solution2 {
                                                int baseLetter;
                                                std::pair<int, int> midBasePair;
                                                std::pair<int, int> midFarPair;
                                                std::pair<int, int> farPair;
                                            };
                                            
                                            std::vector<Solution2> validSolutions;
                                            
                                            for (const auto& bCand : baseCands) {
                                                auto tBase = getLetterAndNodes(bCand, bhMidBaseA, bhMidBaseB);
                                                int baseLetter = std::get<0>(tBase);
                                                int nsBase = std::get<1>(tBase);
                                                int neBase = std::get<2>(tBase);
                                                if (baseLetter != -1) {
                                                    for (const auto& mCand : midCands) {
                                                        int pStart = -1, pEnd = -1;
                                                        for (size_t i = 0; i < mCand.size(); ++i) {
                                                            if (mCand[i] == nsBase) pStart = i;
                                                            if (mCand[i] == neBase) pEnd = i;
                                                        }
                                                        if (pStart != -1 && pEnd != -1) {
                                                            std::pair<int, int> midBasePair = {pStart + 1, pEnd + 1};
                                                            
                                                            int fStart = -1, fEnd = -1;
                                                            for (size_t i = 0; i < mCand.size(); ++i) {
                                                                if (mCand[i] == bhFarMidA) fStart = i;
                                                                if (mCand[i] == bhFarMidB) fEnd = i;
                                                            }
                                                            if (fStart != -1 && fEnd != -1) {
                                                                for (const auto& fCand : farCands) {
                                                                    int fs = -1, fe = -1;
                                                                    for (size_t i = 0; i < fCand.size(); ++i) {
                                                                        if (fCand[i] == mCand[fStart]) fs = i;
                                                                        if (fCand[i] == mCand[fEnd]) fe = i;
                                                                    }
                                                                    if (fs != -1 && fe != -1) {
                                                                        validSolutions.push_back({baseLetter, midBasePair, {fStart + 1, fEnd + 1}, {fs + 1, fe + 1}});
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            if (!validSolutions.empty()) {
                                                Solution2 bestSol = validSolutions[0];
                                                for (size_t i = 1; i < validSolutions.size(); ++i) {
                                                    const auto& sol = validSolutions[i];
                                                    if (sol.baseLetter < bestSol.baseLetter) { bestSol = sol; continue; }
                                                    if (sol.baseLetter > bestSol.baseLetter) continue;
                                                    
                                                    if (sol.midBasePair < bestSol.midBasePair) { bestSol = sol; continue; }
                                                    if (sol.midBasePair > bestSol.midBasePair) continue;
                                                    
                                                    if (sol.farPair < bestSol.farPair) { bestSol = sol; continue; }
                                                    if (sol.farPair > bestSol.farPair) continue;
                                                }
                                                
                                                QString farPref = getFusionPrefixShared(farType);
                                                QString midPref = getFusionPrefixShared(midType);
                                                QString baseName = getBaseNameShared(baseType);
                                                
                                                QString resultName = QString("%1[%2',%3':%4,%5]%6[%7,%8-%9]%10")
                                                    .arg(farPref)
                                                    .arg(bestSol.farPair.first).arg(bestSol.farPair.second)
                                                    .arg(bestSol.midFarPair.first).arg(bestSol.midFarPair.second)
                                                    .arg(midPref)
                                                    .arg(bestSol.midBasePair.first).arg(bestSol.midBasePair.second)
                                                    .arg((char)('a' + bestSol.baseLetter))
                                                    .arg(baseName);
                                                    
                                                if (isNHType(types[0]) || isNHType(types[1]) || isNHType(types[2])) {
                                                    int nhNode = -1;
                                                    for (int i = 0; i < 3; ++i) {
                                                        for (int n : nodes[i]) {
                                                            if (g.nodes[n].atomicNumber == 7 && g.nodes[n].totalH >= 1) {
                                                                nhNode = n;
                                                                break;
                                                            }
                                                        }
                                                        if (nhNode != -1) break;
                                                    }
                                                    if (nhNode != -1) {
                                                        std::set<int> bheads = {bhMidBaseA, bhMidBaseB, bhFarMidA, bhFarMidB};
                                                        std::map<int, QString> periphMap = computePeripheralNumbering3Ring(g, nodes[0], nodes[1], nodes[2], bheads, {}, true);
                                                        if (periphMap.count(nhNode)) {
                                                            QString locStr = periphMap[nhNode];
                                                            bool ok = false;
                                                            locStr.toInt(&ok);
                                                            if (ok) {
                                                                resultName = locStr + "H-" + resultName;
                                                            }
                                                        }
                                                    }
                                                }
                                                    
                                                return {true, resultName, ""};
                                            }
                                        }
                                        return {false, "", "Failed to generate candidates for second-order attached component."};
                                    }

                                    // Middle ring is base.
                                    RingType baseType = types[centerRingIdx];
                                    int baseSize = nodes[centerRingIdx].size();
                                    const std::vector<int>& baseCycle = cycles[centerRingIdx];
                                    const std::vector<int>& baseHetero = rHetero[centerRingIdx];

                                    int bh1_A = sharedNodesPairs[centerRingIdx][end1Idx][0];
                                    int bh1_B = sharedNodesPairs[centerRingIdx][end1Idx][1];
                                    int bh2_A = sharedNodesPairs[centerRingIdx][end2Idx][0];
                                    int bh2_B = sharedNodesPairs[centerRingIdx][end2Idx][1];

                                    std::vector<std::vector<int>> baseCands = getCandidates(baseType, baseHetero, baseSize, baseCycle);
                                    std::vector<std::vector<int>> attCands1 = getCandidates(types[end1Idx], rHetero[end1Idx], nodes[end1Idx].size(), cycles[end1Idx]);
                                    std::vector<std::vector<int>> attCands2 = getCandidates(types[end2Idx], rHetero[end2Idx], nodes[end2Idx].size(), cycles[end2Idx]);

                                    if (!baseCands.empty() && !attCands1.empty() && !attCands2.empty()) {
                                        auto getLetterAndNodes = [&](const std::vector<int>& bCand, int bhA, int bhB) -> std::tuple<int, int, int> {
                                            int posA = -1, posB = -1;
                                            for (int i = 0; i < baseSize; ++i) {
                                                if (bCand[i] == bhA) posA = i;
                                                if (bCand[i] == bhB) posB = i;
                                            }
                                            if (posA != -1 && posB != -1) {
                                                int locA = posA + 1;
                                                int locB = posB + 1;
                                                int minL = std::min(locA, locB);
                                                int maxL = std::max(locA, locB);
                                                if (maxL == minL + 1) {
                                                    return {minL - 1, (locA == minL) ? bhA : bhB, (locA == minL) ? bhB : bhA};
                                                } else if (minL == 1 && maxL == baseSize) {
                                                    return {baseSize - 1, (locA == baseSize) ? bhA : bhB, (locA == 1) ? bhA : bhB};
                                                }
                                            }
                                            return {-1, -1, -1};
                                        };

                                        struct Solution {
                                            int l1, l2; // letters
                                            std::pair<int,int> p1, p2; // locant pairs for end1, end2
                                        };
                                        std::vector<Solution> validSolutions;

                                        for (const auto& bCand : baseCands) {
                                            auto [letter1, ns1, ne1] = getLetterAndNodes(bCand, bh1_A, bh1_B);
                                            auto [letter2, ns2, ne2] = getLetterAndNodes(bCand, bh2_A, bh2_B);

                                            if (letter1 != -1 && letter2 != -1) {
                                                std::pair<int, int> bestAttPair1 = {999, 999};
                                                for (const auto &aCand : attCands1) {
                                                    int pStart = -1, pEnd = -1;
                                                    for (size_t i = 0; i < aCand.size(); ++i) {
                                                        if (aCand[i] == ns1) pStart = i;
                                                        if (aCand[i] == ne1) pEnd = i;
                                                    }
                                                    if (pStart != -1 && pEnd != -1) {
                                                        std::pair<int, int> p = {pStart + 1, pEnd + 1};
                                                        if (p < bestAttPair1) bestAttPair1 = p;
                                                    }
                                                }

                                                std::pair<int, int> bestAttPair2 = {999, 999};
                                                for (const auto &aCand : attCands2) {
                                                    int pStart = -1, pEnd = -1;
                                                    for (size_t i = 0; i < aCand.size(); ++i) {
                                                        if (aCand[i] == ns2) pStart = i;
                                                        if (aCand[i] == ne2) pEnd = i;
                                                    }
                                                    if (pStart != -1 && pEnd != -1) {
                                                        std::pair<int, int> p = {pStart + 1, pEnd + 1};
                                                        if (p < bestAttPair2) bestAttPair2 = p;
                                                    }
                                                }
                                                
                                                if (bestAttPair1.first < 999 && bestAttPair2.first < 999) {
                                                    validSolutions.push_back({letter1, letter2, bestAttPair1, bestAttPair2});
                                                }
                                            }
                                        }

                                        if (!validSolutions.empty()) {
                                            QString pref1 = getFusionPrefixShared(types[end1Idx]);
                                            QString pref2 = getFusionPrefixShared(types[end2Idx]);
                                            
                                            Solution bestSol = validSolutions[0];
                                            for (size_t i = 1; i < validSolutions.size(); ++i) {
                                                const auto& sol = validSolutions[i];
                                                std::vector<int> lSet_best = {bestSol.l1, bestSol.l2};
                                                std::vector<int> lSet_curr = {sol.l1, sol.l2};
                                                std::sort(lSet_best.begin(), lSet_best.end());
                                                std::sort(lSet_curr.begin(), lSet_curr.end());
                                                
                                                if (lSet_curr < lSet_best) {
                                                    bestSol = sol;
                                                } else if (lSet_curr == lSet_best) {
                                                    bool e1First = (pref1 < pref2);
                                                    std::vector<int> order_best = e1First ? std::vector<int>{bestSol.l1, bestSol.l2} : std::vector<int>{bestSol.l2, bestSol.l1};
                                                    std::vector<int> order_curr = e1First ? std::vector<int>{sol.l1, sol.l2} : std::vector<int>{sol.l2, sol.l1};
                                                    if (order_curr < order_best) {
                                                        bestSol = sol;
                                                    }
                                                }
                                            }
                                            
                                            QString block1 = QString("%1[%2,%3-%4]").arg(pref1).arg(bestSol.p1.first).arg(bestSol.p1.second).arg((char)('a' + bestSol.l1));
                                            QString block2 = QString("%1[%2,%3-%4]").arg(pref2).arg(bestSol.p2.first).arg(bestSol.p2.second).arg((char)('a' + bestSol.l2));
                                            
                                            QString resultName;
                                            if (pref1 < pref2) {
                                                resultName = block1 + block2 + getBaseNameShared(baseType);
                                            } else {
                                                resultName = block2 + block1 + getBaseNameShared(baseType);
                                            }
                                            
                                            if (isNHType(types[0]) || isNHType(types[1]) || isNHType(types[2])) {
                                                int nhNode = -1;
                                                for (int i = 0; i < 3; ++i) {
                                                    for (int n : nodes[i]) {
                                                        if (g.nodes[n].atomicNumber == 7 && g.nodes[n].totalH >= 1) {
                                                            nhNode = n;
                                                            break;
                                                        }
                                                    }
                                                    if (nhNode != -1) break;
                                                }
                                                if (nhNode != -1) {
                                                    std::set<int> bheads = {bh1_A, bh1_B, bh2_A, bh2_B};
                                                    std::map<int, QString> periphMap = computePeripheralNumbering3Ring(g, nodes[0], nodes[1], nodes[2], bheads, {}, true);
                                                    if (!periphMap.count(nhNode)) {
                                                        return {false, "", "Failed to compute peripheral locant for indicated hydrogen."};
                                                    }
                                                    QString locStr = periphMap[nhNode];
                                                    bool ok = false;
                                                    locStr.toInt(&ok);
                                                    if (!ok) {
                                                        return {false, "", "Indicated hydrogen locant is letter-suffixed."};
                                                    }
                                                    resultName = locStr + "H-" + resultName;
                                                }
                                            }
                                            return {true, resultName, ""};
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (!handled) {
                return {false, "", "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."};
            }
        }
    }

    

// --- Phase 48: Four-Heterocycle Fusion Chain Nomenclature ---
    if (ringCount > 4) {
        return {false, "", "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."};
    }
    if (ringCount == 4) {
        int sssrIter = indigoIterateSSSR(mol);
        bool allDisjoint = true;
        std::vector<std::set<int>> sssrRings;
        if (sssrIter >= 0) {
            int subMol = 0;
            while ((subMol = indigoNext(sssrIter)) != 0) {
                std::set<int> rAtoms;
                int ringAtomIter = indigoIterateAtoms(subMol);
                if (ringAtomIter >= 0) {
                    int ringAtomHandle = 0;
                    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                        rAtoms.insert(indigoIndex(ringAtomHandle));
                        indigoFree(ringAtomHandle);
                    }
                    indigoFree(ringAtomIter);
                }
                sssrRings.push_back(rAtoms);
                indigoFree(subMol);
            }
            indigoFree(sssrIter);
            for (size_t i = 0; i < sssrRings.size() && allDisjoint; ++i) {
                for (size_t j = i + 1; j < sssrRings.size(); ++j) {
                    for (int a : sssrRings[i]) {
                        if (sssrRings[j].count(a)) {
                            allDisjoint = false;
                            break;
                        }
                    }
                    if (!allDisjoint) break;
                }
            }
        } else {
            allDisjoint = false;
        }

        if (!allDisjoint && sssrRings.size() == 4) {
            int shared[4][4] = {0};
            std::vector<int> sharedNodesPairs[4][4];
            for (int i = 0; i < 4; ++i) {
                for (int j = i + 1; j < 4; ++j) {
                    for (int a : sssrRings[i]) {
                        if (sssrRings[j].count(a)) {
                            if (indigoToGraphIdx.count(a)) {
                                sharedNodesPairs[i][j].push_back(indigoToGraphIdx[a]);
                                sharedNodesPairs[j][i].push_back(indigoToGraphIdx[a]);
                            }
                        }
                    }
                    shared[i][j] = shared[j][i] = sharedNodesPairs[i][j].size();
                }
            }

            int degree[4] = {0};
            bool validFusion = true;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    if (shared[i][j] == 2) degree[i]++;
                    else if (shared[i][j] != 0) validFusion = false;
                }
            }

            int ends = 0, centers = 0;
            for (int i = 0; i < 4; ++i) {
                if (degree[i] == 1) ends++;
                else if (degree[i] == 2) centers++;
            }

            if (validFusion && ends == 2 && centers == 2) {
                // Walk the chain from one end to build the ordered ring sequence.
                int startEnd = -1;
                for (int i = 0; i < 4; ++i) if (degree[i] == 1) { startEnd = i; break; }
                std::vector<int> chain;
                std::set<int> visited;
                int curr = startEnd;
                for (int step = 0; step < 4 && curr != -1; ++step) {
                    chain.push_back(curr);
                    visited.insert(curr);
                    int nextRing = -1;
                    for (int j = 0; j < 4; ++j) {
                        if (j != curr && shared[curr][j] == 2 && !visited.count(j)) { nextRing = j; break; }
                    }
                    curr = nextRing;
                }

                if (chain.size() == 4) {
                    std::set<int> nodes4[4];
                    for (int i = 0; i < 4; ++i) {
                        for (int a : sssrRings[i]) if (indigoToGraphIdx.count(a)) nodes4[i].insert(indigoToGraphIdx[a]);
                    }

                    auto buildCycle4 = [&](const std::set<int> &rNodes) -> std::vector<int> {
                        int rSize = static_cast<int>(rNodes.size());
                        std::vector<int> cycle;
                        int startNode = *rNodes.begin();
                        cycle.push_back(startNode);
                        int current = startNode;
                        int previous = -1;
                        for (int step = 1; step < rSize; ++step) {
                            int nextNode = -1;
                            for (int nei : g.nodes[current].neighbors) {
                                if (rNodes.count(nei) && nei != previous) {
                                    if (step == rSize - 1) {
                                        bool connectedToStart = false;
                                        for (int startNei : g.nodes[nei].neighbors) {
                                            if (startNei == startNode) { connectedToStart = true; break; }
                                        }
                                        if (!connectedToStart) continue;
                                    }
                                    nextNode = nei;
                                    break;
                                }
                            }
                            if (nextNode == -1) break;
                            previous = current;
                            current = nextNode;
                            cycle.push_back(current);
                        }
                        if (static_cast<int>(cycle.size()) != rSize) return {};
                        return cycle;
                    };

                    std::vector<int> cycles4[4];
                    std::vector<int> rHetero4[4];
                    RingType types4[4];
                    bool classOk4[4] = {false};
                    bool allClassified = true;
                    for (int i = 0; i < 4; ++i) {
                        cycles4[i] = buildCycle4(nodes4[i]);
                        for (int n : nodes4[i]) if (g.nodes[n].atomicNumber != 6) rHetero4[i].push_back(n);
                        if (cycles4[i].size() == nodes4[i].size()) {
                            QString d1, d2;
                            classOk4[i] = classifyMonocyclicHeteroRing(g, rHetero4[i], static_cast<int>(nodes4[i].size()), cycles4[i], types4[i], d1, d2);
                        }
                        if (!classOk4[i]) allClassified = false;
                    }

                    auto isAllowedType4 = [](RingType t) {
                        return t == RingType::FURAN || t == RingType::THIOPHENE ||
                               t == RingType::PYRIDINE || t == RingType::PYRIMIDINE ||
                               t == RingType::PYRIDAZINE || t == RingType::PYRAZINE ||
                               t == RingType::OXAZOLE || t == RingType::ISOXAZOLE ||
                               t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE ||
                               t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE ||
                               t == RingType::PYRROLE || t == RingType::IMIDAZOLE ||
                               t == RingType::PYRAZOLE || t == RingType::SELENOPHENE ||
                               t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE;
                    };
                    auto isNHType4 = [](RingType t) {
                        return t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE;
                    };

                    bool allAllowed = allClassified;
                    for (int i = 0; i < 4 && allAllowed; ++i) if (!isAllowedType4(types4[i])) allAllowed = false;
                    bool anyNH = false;
                    for (int i = 0; i < 4; ++i) if (isNHType4(types4[i])) anyNH = true;

                    if (allAllowed && !anyNH) {
                        auto getCandidates4 = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle) {
                            std::vector<std::vector<int>> cands;
                            if (t == RingType::FURAN || t == RingType::THIOPHENE || t == RingType::SELENOPHENE || t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE || t == RingType::PYRIDINE || t == RingType::PYRROLE) {
                                if (hNodes.size() == 1) {
                                    int hNode = hNodes[0];
                                    int hIdx = -1;
                                    for (int i = 0; i < rSize; ++i) if (rCycle[i] == hNode) { hIdx = i; break; }
                                    if (hIdx != -1) {
                                        std::vector<int> fwd(rSize), bwd(rSize);
                                        for (int i = 0; i < rSize; ++i) {
                                            fwd[i] = rCycle[(hIdx + i) % rSize];
                                            bwd[i] = rCycle[(hIdx - i + rSize) % rSize];
                                        }
                                        cands.push_back(fwd);
                                        cands.push_back(bwd);
                                    }
                                }
                            } else if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE) {
                                if (hNodes.size() == 2) {
                                    int n1 = hNodes[0], n2 = hNodes[1];
                                    int idx1 = -1, idx2 = -1;
                                    for (int i = 0; i < rSize; ++i) { if (rCycle[i] == n1) idx1 = i; if (rCycle[i] == n2) idx2 = i; }
                                    if (idx1 != -1 && idx2 != -1) {
                                        int reqOtherIdx = (t == RingType::PYRIDAZINE) ? 1 : ((t == RingType::PYRIMIDINE) ? 2 : 3);
                                        std::vector<int> fwd1(rSize), bwd1(rSize);
                                        for (int i = 0; i < rSize; ++i) { fwd1[i] = rCycle[(idx1 + i) % rSize]; bwd1[i] = rCycle[(idx1 - i + rSize) % rSize]; }
                                        if (fwd1[reqOtherIdx] == n2) cands.push_back(fwd1);
                                        if (bwd1[reqOtherIdx] == n2) cands.push_back(bwd1);
                                        std::vector<int> fwd2(rSize), bwd2(rSize);
                                        for (int i = 0; i < rSize; ++i) { fwd2[i] = rCycle[(idx2 + i) % rSize]; bwd2[i] = rCycle[(idx2 - i + rSize) % rSize]; }
                                        if (fwd2[reqOtherIdx] == n1) cands.push_back(fwd2);
                                        if (bwd2[reqOtherIdx] == n1) cands.push_back(bwd2);
                                    }
                                }
                            } else if (t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE) {
                                if (hNodes.size() == 2) {
                                    int hOS = -1, hN = -1;
                                    int z0 = g.nodes[hNodes[0]].atomicNumber;
                                    if (z0 == 8 || z0 == 16 || z0 == 34) { hOS = hNodes[0]; hN = hNodes[1]; }
                                    else { hOS = hNodes[1]; hN = hNodes[0]; }
                                    int hIdx = -1;
                                    for (int i = 0; i < rSize; ++i) if (rCycle[i] == hOS) { hIdx = i; break; }
                                    if (hIdx != -1) {
                                        std::vector<int> fwd(rSize), bwd(rSize);
                                        for (int i = 0; i < rSize; ++i) { fwd[i] = rCycle[(hIdx + i) % rSize]; bwd[i] = rCycle[(hIdx - i + rSize) % rSize]; }
                                        int reqNIdx = (t == RingType::ISOXAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::ISOSELENAZOLE) ? 1 : 2;
                                        if (fwd[reqNIdx] == hN) cands.push_back(fwd);
                                        if (bwd[reqNIdx] == hN) cands.push_back(bwd);
                                    }
                                }
                            }
                            return cands;
                        };

                        auto getFusionLocants4 = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle, int bhA, int bhB) -> std::pair<int, int> {
                            std::vector<std::vector<int>> cands = getCandidates4(t, hNodes, rSize, rCycle);
                            std::pair<int, int> best = {999, 999};
                            for (const auto &cand : cands) {
                                int posA = -1, posB = -1;
                                for (int i = 0; i < rSize; ++i) { if (cand[i] == bhA) posA = i; if (cand[i] == bhB) posB = i; }
                                if (posA != -1 && posB != -1) {
                                    int locA = posA + 1, locB = posB + 1;
                                    std::pair<int,int> pv = {std::min(locA,locB), std::max(locA,locB)};
                                    if (pv < best) best = pv;
                                }
                            }
                            return best;
                        };

                        auto compareSeniority4 = [&](int idx1, int idx2) -> int {
                            RingType t1 = types4[idx1], t2 = types4[idx2];
                            int size1 = static_cast<int>(nodes4[idx1].size()), size2 = static_cast<int>(nodes4[idx2].size());
                            const auto &rH1 = rHetero4[idx1];
                            const auto &rH2 = rHetero4[idx2];
                            if (t1 == t2) return idx1;

                            auto getRankHetero = [](RingType t) {
                                if (t == RingType::PYRIDINE || t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE || t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE || t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) return 1;
                                if (t == RingType::FURAN) return 2;
                                if (t == RingType::THIOPHENE) return 3;
                                if (t == RingType::SELENOPHENE) return 4;
                                if (t == RingType::TELLUROPHENE) return 5;
                                if (t == RingType::PHOSPHININE) return 6;
                                return 99;
                            };
                            auto getNumHetero = [](RingType t) {
                                if (t == RingType::PYRIMIDINE || t == RingType::PYRIDAZINE || t == RingType::PYRAZINE || t == RingType::OXAZOLE || t == RingType::ISOXAZOLE || t == RingType::THIAZOLE || t == RingType::ISOTHIAZOLE || t == RingType::SELENAZOLE || t == RingType::ISOSELENAZOLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) return 2;
                                return 1;
                            };

                            int rH1Rank = getRankHetero(t1), rH2Rank = getRankHetero(t2);
                            if (rH1Rank < rH2Rank) return idx1;
                            if (rH2Rank < rH1Rank) return idx2;
                            if (size1 > size2) return idx1;
                            if (size2 > size1) return idx2;
                            int nHet1 = getNumHetero(t1), nHet2 = getNumHetero(t2);
                            if (nHet1 > nHet2) return idx1;
                            if (nHet2 > nHet1) return idx2;

                            auto getVariety = [&](const std::vector<int> &hNodes) {
                                std::set<int> elems;
                                for (int n : hNodes) elems.insert(g.nodes[n].atomicNumber);
                                return static_cast<int>(elems.size());
                            };
                            int v1 = getVariety(rH1), v2 = getVariety(rH2);
                            if (v1 > v2) return idx1;
                            if (v2 > v1) return idx2;

                            auto altRank = [](int z) {
                                switch (z) { case 9: return 1; case 17: return 2; case 35: return 3; case 53: return 4; case 8: return 5; case 16: return 6; case 34: return 7; case 52: return 8; case 7: return 9; case 15: return 10; default: return 99; }
                            };
                            auto getTopAltRank = [&](const std::vector<int> &hNodes) {
                                int best = 99;
                                for (int n : hNodes) best = std::min(best, altRank(g.nodes[n].atomicNumber));
                                return best;
                            };
                            int alt1 = getTopAltRank(rH1), alt2 = getTopAltRank(rH2);
                            if (alt1 < alt2) return idx1;
                            if (alt2 < alt1) return idx2;

                            auto getOwnLocants = [](RingType t) -> std::vector<int> {
                                switch (t) {
                                    case RingType::PYRIDINE: case RingType::PYRROLE: case RingType::FURAN: case RingType::THIOPHENE: case RingType::SELENOPHENE: case RingType::TELLUROPHENE: case RingType::PHOSPHININE: return {1};
                                    case RingType::PYRIDAZINE: case RingType::ISOXAZOLE: case RingType::ISOTHIAZOLE: case RingType::PYRAZOLE: case RingType::ISOSELENAZOLE: return {1, 2};
                                    case RingType::PYRIMIDINE: case RingType::OXAZOLE: case RingType::THIAZOLE: case RingType::IMIDAZOLE: case RingType::SELENAZOLE: return {1, 3};
                                    case RingType::PYRAZINE: return {1, 4};
                                    default: return {};
                                }
                            };
                            std::vector<int> ownLoc1 = getOwnLocants(t1), ownLoc2 = getOwnLocants(t2);
                            if (ownLoc1 < ownLoc2) return idx1;
                            if (ownLoc2 < ownLoc1) return idx2;

                            if (sharedNodesPairs[idx1][idx2].size() == 2) {
                                int bhA = sharedNodesPairs[idx1][idx2][0];
                                int bhB = sharedNodesPairs[idx1][idx2][1];
                                std::pair<int,int> fl1 = getFusionLocants4(t1, rH1, size1, cycles4[idx1], bhA, bhB);
                                std::pair<int,int> fl2 = getFusionLocants4(t2, rH2, size2, cycles4[idx2], bhA, bhB);
                                if (fl1 < fl2) return idx1;
                                if (fl2 < fl1) return idx2;
                            }
                            return idx1;
                        };

                        // Cascading tournament down the chain: a linear 4-ring chain can
                        // only be named without a third-order attached component if the
                        // winner is one of the two INTERIOR rings (position 1 or 2) - an
                        // end-ring winner (position 0 or 3) would need the far ring cited
                        // three levels deep, which is out of scope (FR-2.3/P-25.3.4.1.1
                        // only defines first- and second-order attached components).
                        int w1 = compareSeniority4(chain[0], chain[1]);
                        int w2 = compareSeniority4(w1, chain[2]);
                        int winner = compareSeniority4(w2, chain[3]);

                        int winnerPos = -1;
                        for (int p = 0; p < 4; ++p) if (chain[p] == winner) winnerPos = p;

                        if (winnerPos == 0 || winnerPos == 3) {
                            return {false, "", "End ring as base in a 4-ring fusion chain requires third-order attached components, which are not supported."};
                        }

                        if (winnerPos == 1 || winnerPos == 2) {
                            int baseIdx = winner;
                            int nearIdx = (winnerPos == 1) ? chain[0] : chain[3];
                            int midIdx  = (winnerPos == 1) ? chain[2] : chain[1];
                            int farIdx  = (winnerPos == 1) ? chain[3] : chain[0];

                            RingType baseType = types4[baseIdx];
                            RingType nearType = types4[nearIdx];
                            RingType midType = types4[midIdx];
                            RingType farType = types4[farIdx];

                            int baseSize = static_cast<int>(nodes4[baseIdx].size());
                            const std::vector<int> &baseCycle = cycles4[baseIdx];
                            const std::vector<int> &baseHetero = rHetero4[baseIdx];

                            int bhNearA = sharedNodesPairs[baseIdx][nearIdx][0];
                            int bhNearB = sharedNodesPairs[baseIdx][nearIdx][1];
                            int bhMidA = sharedNodesPairs[baseIdx][midIdx][0];
                            int bhMidB = sharedNodesPairs[baseIdx][midIdx][1];
                            int bhFarMidA = sharedNodesPairs[midIdx][farIdx][0];
                            int bhFarMidB = sharedNodesPairs[midIdx][farIdx][1];

                            std::vector<std::vector<int>> baseCands = getCandidates4(baseType, baseHetero, baseSize, baseCycle);
                            std::vector<std::vector<int>> nearCands = getCandidates4(nearType, rHetero4[nearIdx], static_cast<int>(nodes4[nearIdx].size()), cycles4[nearIdx]);
                            std::vector<std::vector<int>> midCands = getCandidates4(midType, rHetero4[midIdx], static_cast<int>(nodes4[midIdx].size()), cycles4[midIdx]);
                            std::vector<std::vector<int>> farCands = getCandidates4(farType, rHetero4[farIdx], static_cast<int>(nodes4[farIdx].size()), cycles4[farIdx]);

                            if (!baseCands.empty() && !nearCands.empty() && !midCands.empty() && !farCands.empty()) {
                                auto getLetterAndNodes = [&](const std::vector<int>& bCand, int bhA, int bhB) -> std::tuple<int,int,int> {
                                    int posA = -1, posB = -1;
                                    for (int i = 0; i < baseSize; ++i) { if (bCand[i]==bhA) posA=i; if (bCand[i]==bhB) posB=i; }
                                    if (posA != -1 && posB != -1) {
                                        int locA = posA+1, locB = posB+1;
                                        int minL = std::min(locA,locB), maxL = std::max(locA,locB);
                                        if (maxL == minL+1) return {minL-1, (locA==minL)?bhA:bhB, (locA==minL)?bhB:bhA};
                                        else if (minL==1 && maxL==baseSize) return {baseSize-1, (locA==baseSize)?bhA:bhB, (locA==1)?bhA:bhB};
                                    }
                                    return {-1,-1,-1};
                                };

                                struct Solution4 {
                                    int letterNear, letterMid;
                                    std::pair<int,int> nearPair, midBasePair, midFarPair, farPair;
                                };
                                std::vector<Solution4> validSolutions;

                                for (const auto &bCand : baseCands) {
                                    auto [letterNear, nsN, neN] = getLetterAndNodes(bCand, bhNearA, bhNearB);
                                    auto [letterMid, nsM, neM] = getLetterAndNodes(bCand, bhMidA, bhMidB);
                                    if (letterNear == -1 || letterMid == -1) continue;

                                    std::pair<int,int> bestNearPair = {999,999};
                                    for (const auto &aCand : nearCands) {
                                        int pS=-1,pE=-1;
                                        for (size_t i=0;i<aCand.size();++i) { if (aCand[i]==nsN) pS=i; if (aCand[i]==neN) pE=i; }
                                        if (pS!=-1 && pE!=-1) {
                                            std::pair<int,int> p = {pS+1,pE+1};
                                            if (p < bestNearPair) bestNearPair = p;
                                        }
                                    }
                                    if (bestNearPair.first >= 999) continue;

                                    for (const auto &mCand : midCands) {
                                        int pStart=-1,pEnd=-1;
                                        for (size_t i=0;i<mCand.size();++i) { if (mCand[i]==nsM) pStart=i; if (mCand[i]==neM) pEnd=i; }
                                        if (pStart==-1 || pEnd==-1) continue;
                                        std::pair<int,int> midBasePair = {pStart+1, pEnd+1};

                                        int fStart=-1, fEnd=-1;
                                        for (size_t i=0;i<mCand.size();++i) { if (mCand[i]==bhFarMidA) fStart=i; if (mCand[i]==bhFarMidB) fEnd=i; }
                                        if (fStart==-1 || fEnd==-1) continue;

                                        for (const auto &fCand : farCands) {
                                            int fs=-1, fe=-1;
                                            for (size_t i=0;i<fCand.size();++i) { if (fCand[i]==mCand[fStart]) fs=i; if (fCand[i]==mCand[fEnd]) fe=i; }
                                            if (fs==-1 || fe==-1) continue;
                                            validSolutions.push_back({letterNear, letterMid, bestNearPair, midBasePair, {fStart+1, fEnd+1}, {fs+1, fe+1}});
                                        }
                                    }
                                }

                                if (!validSolutions.empty()) {
                                    QString nearPref = getFusionPrefixShared(nearType);
                                    QString midPref = getFusionPrefixShared(midType);
                                    QString farPref = getFusionPrefixShared(farType);
                                    QString baseName = getBaseNameShared(baseType);

                                    // The alphabetically-first attached component (by its own
                                    // fusion-prefix name) is entitled to the lower base letter;
                                    // a plain sorted-set comparison can't tell "near=b,mid=e"
                                    // apart from "near=e,mid=b" since both give the same set.
                                    bool nearFirst = (nearPref < midPref);
                                    auto sortKey = [&](const Solution4 &s) {
                                        int firstLetter = nearFirst ? s.letterNear : s.letterMid;
                                        int secondLetter = nearFirst ? s.letterMid : s.letterNear;
                                        return std::make_tuple(firstLetter, secondLetter, s.nearPair, s.midBasePair, s.midFarPair, s.farPair);
                                    };
                                    Solution4 bestSol = validSolutions[0];
                                    for (size_t i = 1; i < validSolutions.size(); ++i) {
                                        const auto &sol = validSolutions[i];
                                        if (sortKey(sol) < sortKey(bestSol)) bestSol = sol;
                                    }

                                    QString nearBlock = QString("%1[%2,%3-%4]").arg(nearPref).arg(bestSol.nearPair.first).arg(bestSol.nearPair.second).arg((char)('a' + bestSol.letterNear));
                                    QString midBlock = QString("%1[%2,%3-%4]").arg(midPref).arg(bestSol.midBasePair.first).arg(bestSol.midBasePair.second).arg((char)('a' + bestSol.letterMid));

                                    // Normalize the correspondence to ascending order on the
                                    // outer (unprimed, mid-relative) side, swapping both linked
                                    // pairs together so the primed<->unprimed correspondence
                                    // between the same physical atoms is preserved.
                                    std::pair<int,int> farPairOut = bestSol.farPair;
                                    std::pair<int,int> midFarPairOut = bestSol.midFarPair;
                                    if (midFarPairOut.first > midFarPairOut.second) {
                                        std::swap(farPairOut.first, farPairOut.second);
                                        std::swap(midFarPairOut.first, midFarPairOut.second);
                                    }
                                    QString farBlock = QString("%1[%2',%3':%4,%5]").arg(farPref).arg(farPairOut.first).arg(farPairOut.second).arg(midFarPairOut.first).arg(midFarPairOut.second);

                                    QString resultName;
                                    if (nearPref < midPref) {
                                        resultName = nearBlock + farBlock + midBlock + baseName;
                                    } else {
                                        resultName = farBlock + midBlock + nearBlock + baseName;
                                    }

                                    return {true, resultName, ""};
                                }
                            }
                        }
                    }
                }
            }
        }

        return {false, "", "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."};
    }

    // --- Phase 3: Ortho-fused Bicyclic Aromatic Path (ringCount == 2: Naphthalene) ---
    if (ringCount == 2) {

        int sssrIter = indigoIterateSSSR(mol);
        if (sssrIter < 0) {
            return {false, "", "Failed to iterate SSSR rings."};
        }

        std::vector<std::set<int>> sssrRings;
        int subMol = 0;
        while ((subMol = indigoNext(sssrIter)) != 0) {
            std::set<int> rAtoms;
            int ringAtomIter = indigoIterateAtoms(subMol);
            if (ringAtomIter >= 0) {
                int ringAtomHandle = 0;
                while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                    int idx = indigoIndex(ringAtomHandle);
                    rAtoms.insert(idx);
                    indigoFree(ringAtomHandle);
                }
                indigoFree(ringAtomIter);
            }
            sssrRings.push_back(rAtoms);
            indigoFree(subMol);
        }
        indigoFree(sssrIter);

        if (sssrRings.size() != 2) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        std::set<int> ring1Nodes, ring2Nodes;
        for (int idx : sssrRings[0]) {
            if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
        }
        for (int idx : sssrRings[1]) {
            if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
        }

        // 1. Both rings must be size 6
        if (ring1Nodes.size() != 6 || ring2Nodes.size() != 6) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        // 2. Intersection must be exactly 2 shared nodes
        std::vector<int> sharedNodes;
        for (int n : ring1Nodes) {
            if (ring2Nodes.count(n)) sharedNodes.push_back(n);
        }
        if (sharedNodes.size() != 2) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        // 3. Shared nodes must be directly bonded to each other
        int bhA = sharedNodes[0];
        int bhB = sharedNodes[1];
        bool bhBonded = false;
        for (size_t i = 0; i < g.nodes[bhA].neighbors.size(); ++i) {
            if (g.nodes[bhA].neighbors[i] == bhB) {
                bhBonded = true;
                break;
            }
        }
        if (!bhBonded) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        // 4. Union of ring nodes must be 10 atoms, all carbon (Z == 6)
        std::set<int> ringNodeSet;
        for (int n : ring1Nodes) ringNodeSet.insert(n);
        for (int n : ring2Nodes) ringNodeSet.insert(n);
        if (ringNodeSet.size() != 10) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }
        for (int n : ringNodeSet) {
            if (g.nodes[n].atomicNumber != 6) {
                return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
            }
        }

        // 5. All bonds within the ring system must be aromatic (order == 4)
        for (const auto &gb : g.bonds) {
            if (ringNodeSet.count(gb.u) && ringNodeSet.count(gb.v)) {
                if (gb.order != 4) {
                    return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
                }
            }
        }

        // 6. Bridgehead carbons (bhA, bhB) cannot have external neighbors outside ringNodeSet
        for (int nei : g.nodes[bhA].neighbors) {
            if (!ringNodeSet.count(nei)) {
                return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
            }
        }
        for (int nei : g.nodes[bhB].neighbors) {
            if (!ringNodeSet.count(nei)) {
                return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
            }
        }

        // Walk 4 peripheral nodes in Ring 1 from bhA to bhB
        int p1_start = -1;
        for (int nei : g.nodes[bhA].neighbors) {
            if (ring1Nodes.count(nei) && nei != bhB) {
                p1_start = nei;
                break;
            }
        }
        if (p1_start == -1) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        std::vector<int> ring1Path;
        int curr = p1_start;
        int prev = bhA;
        for (int i = 0; i < 4; ++i) {
            ring1Path.push_back(curr);
            int nextNode = -1;
            for (int nei : g.nodes[curr].neighbors) {
                if (ring1Nodes.count(nei) && nei != prev) {
                    nextNode = nei;
                    break;
                }
            }
            prev = curr;
            curr = nextNode;
        }
        if (ring1Path.size() != 4 || curr != bhB) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        // Walk 4 peripheral nodes in Ring 2 from bhA to bhB
        int p2_start = -1;
        for (int nei : g.nodes[bhA].neighbors) {
            if (ring2Nodes.count(nei) && nei != bhB) {
                p2_start = nei;
                break;
            }
        }
        if (p2_start == -1) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        std::vector<int> ring2Path;
        curr = p2_start;
        prev = bhA;
        for (int i = 0; i < 4; ++i) {
            ring2Path.push_back(curr);
            int nextNode = -1;
            for (int nei : g.nodes[curr].neighbors) {
                if (ring2Nodes.count(nei) && nei != prev) {
                    nextNode = nei;
                    break;
                }
            }
            prev = curr;
            curr = nextNode;
        }
        if (ring2Path.size() != 4 || curr != bhB) {
            return {false, "", "Fused ring systems other than naphthalene are not supported in this phase."};
        }

        // Generate the 4 canonical locant candidate maps
        // Candidate 1: bhA -> R1 -> bhB -> rev(R2) -> bhA
        // Candidate 2: bhA -> R2 -> bhB -> rev(R1) -> bhA
        // Candidate 3: bhB -> rev(R1) -> bhA -> R2 -> bhB
        // Candidate 4: bhB -> rev(R2) -> bhA -> R1 -> bhB
        std::vector<std::map<int, int>> candidateMaps(4);

        for (int i = 0; i < 4; ++i) candidateMaps[0][ring1Path[i]] = i + 1;
        for (int i = 0; i < 4; ++i) candidateMaps[0][ring2Path[3 - i]] = i + 5;

        for (int i = 0; i < 4; ++i) candidateMaps[1][ring2Path[i]] = i + 1;
        for (int i = 0; i < 4; ++i) candidateMaps[1][ring1Path[3 - i]] = i + 5;

        for (int i = 0; i < 4; ++i) candidateMaps[2][ring1Path[3 - i]] = i + 1;
        for (int i = 0; i < 4; ++i) candidateMaps[2][ring2Path[i]] = i + 5;

        for (int i = 0; i < 4; ++i) candidateMaps[3][ring2Path[3 - i]] = i + 1;
        for (int i = 0; i < 4; ++i) candidateMaps[3][ring1Path[i]] = i + 5;

        // Pattern matching functional groups across whole molecule
        std::map<int, GroupType> carbonGroup;
        std::map<int, int> acylHalideHalogen;
        std::map<int, int> esterAlkylRoot;
        std::map<int, int> esterOxygen;
        std::set<int> etherOxygens;

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            const GraphNode &node = g.nodes[i];
            if (node.atomicNumber == 6) {
                std::vector<int> doubleO, singleO, singleN, tripleN, halogens;

                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    int nZ = g.nodes[nei].atomicNumber;

                    if (nZ == 8 && order == 2) doubleO.push_back(nei);
                    else if (nZ == 8 && order == 1) singleO.push_back(nei);
                    else if (nZ == 7 && order == 1) {
                        bool isNitroIsoOrAzide = false;
                        if (carbonAzide.count(i) && std::find(carbonAzide[i].begin(), carbonAzide[i].end(), nei) != carbonAzide[i].end()) isNitroIsoOrAzide = true;
                        if (!isNitroIsoOrAzide) singleN.push_back(nei);
                    }
                    else if (nZ == 7 && order == 3) tripleN.push_back(nei);
                    else if ((nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) && order == 1) halogens.push_back(nei);
                }

                if (!doubleO.empty()) {
                    for (int sO : singleO) {
                        int cCount = 0;
                        int alkylRootNode = -1;
                        for (size_t k = 0; k < g.nodes[sO].neighbors.size(); ++k) {
                            int oNei = g.nodes[sO].neighbors[k];
                            if (g.nodes[oNei].atomicNumber == 6) {
                                cCount++;
                                if (oNei != static_cast<int>(i)) {
                                    alkylRootNode = oNei;
                                }
                            }
                        }
                        if (cCount == 2 && alkylRootNode != -1) {
                            carbonGroup[i] = GroupType::ESTER;
                            esterAlkylRoot[i] = alkylRootNode;
                            esterOxygen[i] = sO;
                        }
                    }
                }

                if (carbonGroup.count(i) && carbonGroup[i] == GroupType::ESTER) continue;

                if (!doubleO.empty() && !singleO.empty()) {
                    bool hasOH = false;
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            hasOH = true; break;
                        }
                    }
                    if (hasOH) carbonGroup[i] = GroupType::ACID;
                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    carbonGroup[i] = GroupType::AMIDE;
                } else if (!tripleN.empty()) {
                    carbonGroup[i] = GroupType::NITRILE;
                } else if (!doubleO.empty() && (node.totalH >= 1 || node.neighbors.size() <= 2)) {
                    carbonGroup[i] = GroupType::ALDEHYDE;
                } else if (!doubleO.empty()) {
                    carbonGroup[i] = GroupType::KETONE;
                } else if (!singleO.empty()) {
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            carbonGroup[i] = GroupType::ALCOHOL; break;
                        }
                    }
                } else if (singleN.size() > 0) {
                    carbonGroup[i] = GroupType::AMINE;
                } else if (carbonSulfonicAcid.count(i) && ringNodeSet.count(i) > 0) {
                    carbonGroup[i] = GroupType::SULFONIC_ACID;
                } else if (carbonThiol.count(i) && ringNodeSet.count(i) > 0) {
                    carbonGroup[i] = GroupType::THIOL;
                }
            } else if (node.atomicNumber == 8) {
                if (node.neighbors.size() == 2 && node.bondOrders[0] == 1 && node.bondOrders[1] == 1) {
                    int n1 = node.neighbors[0];
                    int n2 = node.neighbors[1];
                    if (g.nodes[n1].atomicNumber == 6 && g.nodes[n2].atomicNumber == 6) {
                        etherOxygens.insert(node.id);
                    }
                }
            }
        }

        auto groupRank = [](GroupType gt) -> int {
            switch (gt) {
                case GroupType::SULFONIC_ACID: return 1;
                case GroupType::ACID: return 2;
                case GroupType::BORONIC_ACID: return 3;
                case GroupType::ESTER: return 4;
                case GroupType::ACYL_HALIDE: return 5;
                case GroupType::AMIDE: return 6;
                case GroupType::NITRILE: return 7;
                case GroupType::ALDEHYDE: return 8;
                case GroupType::THIAL: return 9;
                case GroupType::KETONE: return 10;
                case GroupType::THIONE: return 11;
                case GroupType::ALCOHOL: return 12;
                case GroupType::THIOL: return 13;
                case GroupType::AMINE: return 14;
                case GroupType::PHOSPHINE: return 15;
                default: return 16;
            }
        };

        GroupType winningRingGroup = GroupType::NONE;
        GroupType winningChainGroup = GroupType::NONE;

        for (const auto &pair : carbonGroup) {
            int cNode = pair.first;
            GroupType gt = pair.second;
            bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
            if (!isOnOrExocyclic) {
                for (int nei : g.nodes[cNode].neighbors) {
                    if (ringNodeSet.count(nei) > 0) {
                        isOnOrExocyclic = true;
                        break;
                    }
                }
            }
            if (isOnOrExocyclic) {
                if (groupRank(gt) < groupRank(winningRingGroup)) winningRingGroup = gt;
            } else {
                if (groupRank(gt) < groupRank(winningChainGroup)) winningChainGroup = gt;
            }
        }

        if (groupRank(winningChainGroup) < groupRank(winningRingGroup)) {
            return {false, "", "A chain-based principal group outranks the ring in this structure; ring-vs-chain seniority is not yet supported in Phase 2."};
        }

        GroupType winningType = winningRingGroup;
        std::set<int> principalCarbons;
        if (winningType != GroupType::NONE) {
            for (const auto &pair : carbonGroup) {
                int cNode = pair.first;
                GroupType gt = pair.second;
                if (gt == winningType) {
                    bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
                    if (!isOnOrExocyclic) {
                        for (int nei : g.nodes[cNode].neighbors) {
                            if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                        }
                    }
                    if (isOnOrExocyclic) principalCarbons.insert(cNode);
                }
            }
        }

        struct NaphthaleneSignature {
            std::map<int, int> locantMap;
            std::vector<int> principalLocants;
            std::vector<int> substituentLocants;
            std::vector<std::pair<QString, int>> namedSubstituents;
        };

        std::vector<NaphthaleneSignature> signatures;

        for (const auto &lMap : candidateMaps) {
            NaphthaleneSignature sig;
            sig.locantMap = lMap;

            for (int pC : principalCarbons) {
                if (lMap.count(pC)) {
                    sig.principalLocants.push_back(lMap.at(pC));
                } else {
                    for (int nei : g.nodes[pC].neighbors) {
                        if (lMap.count(nei)) {
                            sig.principalLocants.push_back(lMap.at(nei));
                            break;
                        }
                    }
                }
            }
            std::sort(sig.principalLocants.begin(), sig.principalLocants.end());

            for (auto const &pair : lMap) {
                int rNode = pair.first;
                int locant = pair.second;
                bool isPrincipalRNode = (principalCarbons.count(rNode) > 0);

                for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                    int nei = g.nodes[rNode].neighbors[j];
                    int order = g.nodes[rNode].bondOrders[j];
                    if (ringNodeSet.count(nei)) continue;

                    if (winningType != GroupType::NONE) {
                        if ((winningType == GroupType::ALCOHOL || winningType == GroupType::KETONE || winningType == GroupType::AMINE || winningType == GroupType::SULFONIC_ACID || winningType == GroupType::THIOL) && isPrincipalRNode) {
                            int nz = g.nodes[nei].atomicNumber;
                            bool isAzide = (carbonAzide.count(rNode) && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end());
                            if (!isAzide && (nz == 7 || nz == 8 || nz == 16)) continue;
                        }
                        if (principalCarbons.count(nei) > 0) continue;
                    }

                    int nz = g.nodes[nei].atomicNumber;
                    QString subName;
                    if (nz == 9 || nz == 17 || nz == 35 || nz == 53) {
                        subName = halogenPrefix(nz);
                    } else if (nz == 8) {
                        if (order == 1) {
                            if (etherOxygens.count(nei)) {
                                int alkylNei = -1;
                                for (int oNei : g.nodes[nei].neighbors) {
                                    if (oNei != rNode) { alkylNei = oNei; break; }
                                }
                                if (alkylNei != -1) {
                                    QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                    if (alkylName.endsWith("yl")) {
                                        alkylName.chop(2); alkylName += "oxy";
                                    }
                                    subName = alkylName;
                                }
                            } else if (winningType != GroupType::ALCOHOL) {
                                subName = "hydroxy";
                            }
                        } else if (order == 2) {
                            if (winningType != GroupType::ALDEHYDE && winningType != GroupType::KETONE) {
                                subName = "oxo";
                            }
                        }
                    } else if (nz == 7) {
                        bool isAzide = false;
                        for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                            int nNei = g.nodes[nei].neighbors[k];
                            if (g.nodes[nNei].atomicNumber == 7 && g.nodes[nei].bondOrders[k] >= 2) {
                                isAzide = true; break;
                            }
                        }
                        if (carbonAzide.count(rNode) && ringNodeSet.count(rNode) > 0 && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end()) {
                            subName = "azido";
                        } else if (!isAzide && order == 1 && winningType != GroupType::AMINE) {
                            subName = "amino";
                        }
                    } else if (nz == 16) {
                        if (carbonSulfonicAcid.count(rNode) && carbonSulfonicAcid[rNode] == nei && winningType != GroupType::SULFONIC_ACID) {
                            subName = "sulfo";
                        } else if (carbonThiol.count(rNode) && carbonThiol[rNode] == nei && winningType != GroupType::THIOL) {
                            subName = "sulfanyl";
                        } else if (order == 1) {
                            if (thioetherSulfurs.count(nei)) {
                                int alkylNei = -1;
                                for (int sNei : g.nodes[nei].neighbors) {
                                    if (sNei != rNode) { alkylNei = sNei; break; }
                                }
                                if (alkylNei != -1) {
                                    QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                    subName = alkylName + "sulfanyl";
                                }
                            }
                        }
                    } else if (nz == 6) {
                        if (carbonGroup.count(nei) && winningType != carbonGroup[nei]) {
                            if (carbonGroup[nei] == GroupType::ACID) {
                                subName = "carboxy";
                            } else if (carbonGroup[nei] == GroupType::ACYL_HALIDE) {
                                subName = halogenPrefix(acylHalideHalogen[nei]) + "carbonyl";
                            } else if (carbonGroup[nei] == GroupType::ESTER) {
                                int alkylRoot = esterAlkylRoot[nei];
                                int sO = esterOxygen[nei];
                                QString alkylName = nameBranchGraph(g, alkylRoot, sO);
                                if (alkylName.isEmpty()) {
                                    return {false, "", "Unsupported ester alkyl group."};
                                }
                                if (alkylName.endsWith("yl")) {
                                    alkylName.chop(2);
                                    alkylName += "oxycarbonyl";
                                }
                                subName = alkylName;
                            }
                        }
                        if (subName.isEmpty()) {
                            subName = nameBranchGraph(g, nei, rNode, allSSSRRings, ring2Nodes);
                            if (subName.startsWith("(")) {
                                subName = subName.mid(1);
                                if (subName.endsWith(")")) subName.chop(1);
                            }
                        }
                    }

                    if (!subName.isEmpty()) {
                        sig.substituentLocants.push_back(locant);
                        sig.namedSubstituents.push_back({subName, locant});
                    } else {
                        return {false, "", "Unrecognized or unsupported substituent on ring."};
                    }
                }
            }
            std::sort(sig.substituentLocants.begin(), sig.substituentLocants.end());
            signatures.push_back(sig);
        }

        auto bestIt = std::min_element(signatures.begin(), signatures.end(),
            [](const NaphthaleneSignature &a, const NaphthaleneSignature &b) {
                if (a.principalLocants != b.principalLocants) return a.principalLocants < b.principalLocants;
                if (a.substituentLocants != b.substituentLocants) return a.substituentLocants < b.substituentLocants;

                auto firstAlpha = [](const std::vector<std::pair<QString,int>> &named) {
                    return std::min_element(named.begin(), named.end(),
                        [](const auto &x, const auto &y) { return x.first.toLower() < y.first.toLower(); });
                };
                if (!a.namedSubstituents.empty()) {
                    QString alphaName = firstAlpha(a.namedSubstituents)->first;
                    auto findLocant = [&](const std::vector<std::pair<QString,int>> &named) {
                        int best = INT_MAX;
                        for (const auto &ns : named) if (ns.first == alphaName) best = std::min(best, ns.second);
                        return best;
                    };
                    int aLoc = findLocant(a.namedSubstituents);
                    int bLoc = findLocant(b.namedSubstituents);
                    if (aLoc != bLoc) return aLoc < bLoc;
                }
                return false;
            });

        NaphthaleneSignature bestSig = *bestIt;
        std::map<int, int> graphIdToLocant = bestSig.locantMap;
        std::set<int> naphthRingNodes(ring1Nodes.begin(), ring1Nodes.end());
        naphthRingNodes.insert(ring2Nodes.begin(), ring2Nodes.end());
        StereoResult ezRes = processDoubleBondStereo(mol, g, naphthRingNodes, graphIdToLocant, stereoByGraphId);
        if (!ezRes.ok) {
            return {false, "", ezRes.error};
        }
        StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, graphIdToLocant);
        if (!stereoRes.ok) {
            return {false, "", stereoRes.error};
        }

        std::map<QString, std::vector<int>> prefixLocantsMap;
        for (const auto &ns : bestSig.namedSubstituents) {
            prefixLocantsMap[ns.first].push_back(ns.second);
        }

        struct PrefixGroup {
            QString baseName;
            QString formattedStr;
        };
        std::vector<PrefixGroup> pGroups;
        for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
            QString pName = it->first;
            std::vector<int> locs = it->second;
            std::sort(locs.begin(), locs.end());

            QStringList locStrs;
            for (int l : locs) locStrs.append(QString::number(l));

            QString pStr = locStrs.join(",");
            if (locs.size() > 1) {
                pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
            } else {
                pStr += "-" + pName;
            }

            PrefixGroup pg;
            pg.baseName = pName.startsWith("(") ? pName.mid(1) : pName;
            pg.formattedStr = pStr;
            pGroups.push_back(pg);
        }

        std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
            return a.baseName.toLower() < b.baseName.toLower();
        });

        QString prefixPart;
        if (!pGroups.empty()) {
            QStringList pStrs;
            for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
            prefixPart = pStrs.join("-");
        }

        QString fullName;
        if (winningType == GroupType::NONE) {
            fullName = prefixPart + "naphthalene";
        } else {
            bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE ||
                                winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||
                                winningType == GroupType::ACYL_HALIDE || winningType == GroupType::ESTER);
            int pCount = static_cast<int>(bestSig.principalLocants.size());

            if (isExocyclic) {
                QString sfx;
                if (winningType == GroupType::ACID) sfx = (pCount == 1) ? "carboxylic acid" : "dicarboxylic acid";
                else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";
                else if (winningType == GroupType::NITRILE) sfx = (pCount == 1) ? "carbonitrile" : "dicarbonitrile";
                else if (winningType == GroupType::ALDEHYDE) sfx = (pCount == 1) ? "carbaldehyde" : "dicarbaldehyde";
                else if (winningType == GroupType::ESTER) sfx = (pCount == 1) ? "carboxylate" : (multiPrefix(pCount) + "carboxylate");
                else if (winningType == GroupType::ACYL_HALIDE) {
                    QString hName;
                    int hz = acylHalideHalogen.empty() ? 17 : acylHalideHalogen.begin()->second;
                    for (int pc : principalCarbons) {
                        if (acylHalideHalogen.count(pc)) { hz = acylHalideHalogen[pc]; break; }
                    }
                    if (hz == 9) hName = "fluoride";
                    else if (hz == 17) hName = "chloride";
                    else if (hz == 35) hName = "bromide";
                    else if (hz == 53) hName = "iodide";
                    sfx = (pCount == 1) ? ("carbonyl " + hName) : ("dicarbonyl " + hName);
                }

                if (pCount == 1) {
                    fullName = prefixPart + QString("naphthalene-%1-%2").arg(bestSig.principalLocants[0]).arg(sfx);
                } else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    fullName = prefixPart + QString("naphthalene-%1-%2").arg(lStrs.join(","), sfx);
                }
            } else {
                QString sfx;
                if (winningType == GroupType::SULFONIC_ACID) sfx = (pCount == 1) ? "sulfonic acid" : "disulfonic acid";
                else if (winningType == GroupType::THIOL) sfx = (pCount == 1) ? "thiol" : "dithiol";
                else if (winningType == GroupType::ALCOHOL) sfx = (pCount == 1) ? "ol" : "diol";
                else if (winningType == GroupType::KETONE) sfx = (pCount == 1) ? "one" : "dione";
                else if (winningType == GroupType::AMINE) sfx = (pCount == 1) ? "amine" : "diamine";

                if (pCount == 1) {
                    QString root = (!sfx.isEmpty() && isVowel(sfx[0])) ? "naphthalen" : "naphthalene";
                    fullName = prefixPart + QString("%1-%2-%3").arg(root).arg(bestSig.principalLocants[0]).arg(sfx);
                } else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    fullName = prefixPart + QString("naphthalene-%1-%2").arg(lStrs.join(","), sfx);
                }
            }
        }

        if (winningType == GroupType::ESTER) {
            int esterCarbon = -1;
            for (int pc : principalCarbons) {
                if (carbonGroup.count(pc) && carbonGroup[pc] == GroupType::ESTER) {
                    esterCarbon = pc;
                    break;
                }
            }
            if (esterCarbon != -1) {
                int alkylRoot = esterAlkylRoot[esterCarbon];
                int sO = esterOxygen[esterCarbon];
                QString alkylName = nameBranchGraph(g, alkylRoot, sO, allSSSRRings);
                if (alkylName.isEmpty()) {
                    return {false, "", "Unsupported ester alkyl group."};
                }
                fullName = alkylName + " " + fullName;
            }
        }

        fullName = stereoRes.prefix + fullName;
        return {true, fullName, ""};
    }

    // --- Phase 2: Monocyclic Ring Path (ringCount == 1) ---
    int sssrIter = indigoIterateSSSR(mol);
    if (sssrIter < 0) {
        return {false, "", "Failed to iterate SSSR rings."};
    }
    int subMol = indigoNext(sssrIter);
    if (subMol <= 0) {
        indigoFree(sssrIter);
        return {false, "", "Failed to extract ring submolecule."};
    }

    std::set<int> ringIndigoAtomIndices;
    int ringAtomIter = indigoIterateAtoms(subMol);
    int ringAtomHandle = 0;
    while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
        int idx = indigoIndex(ringAtomHandle);
        ringIndigoAtomIndices.insert(idx);
        indigoFree(ringAtomHandle);
    }
    indigoFree(ringAtomIter);
    indigoFree(subMol);
    indigoFree(sssrIter);

    std::set<int> ringNodeSet;
    for (int idx : ringIndigoAtomIndices) {
        if (indigoToGraphIdx.count(idx)) {
            ringNodeSet.insert(indigoToGraphIdx[idx]);
        }
    }

    int ringSize = static_cast<int>(ringNodeSet.size());
    if (ringSize < 3 || ringSize > 20) {
        return {false, "", "Ring size is outside the supported range for Phase 2."};
    }

    std::vector<int> ringCycle;
    int startNode = *ringNodeSet.begin();
    int current = startNode;
    int previous = -1;

    for (int step = 0; step < ringSize; ++step) {
        ringCycle.push_back(current);
        int nextNode = -1;
        for (int nei : g.nodes[current].neighbors) {
            if (ringNodeSet.count(nei) && nei != previous) {
                nextNode = nei;
                break;
            }
        }
        if (nextNode == -1) break;
        previous = current;
        current = nextNode;
    }

    if (static_cast<int>(ringCycle.size()) != ringSize) {
        return {false, "", "Failed to extract valid ring cycle."};
    }

    struct RingBond { int u, v, order; };
    std::vector<RingBond> ringBonds;
    for (const auto &gb : g.bonds) {
        if (ringNodeSet.count(gb.u) && ringNodeSet.count(gb.v)) {
            ringBonds.push_back({gb.u, gb.v, gb.order});
        }
    }

    std::vector<int> ringHeteroNodes;
    for (int nodeIdx : ringNodeSet) {
        if (g.nodes[nodeIdx].atomicNumber != 6) {
            ringHeteroNodes.push_back(nodeIdx);
        }
    }

    RingType rType;
    QString parentNameRoot;

    if (!ringHeteroNodes.empty()) {
        QString classErr;
        if (!classifyMonocyclicHeteroRing(g, ringHeteroNodes, ringSize, ringCycle, rType, parentNameRoot, classErr)) {
            return {false, "", classErr};
        }
    } else {
        bool allAromatic = true;
        bool allSingle = true;
        bool hasDouble = false;
        bool hasTriple = false;

        for (const auto &rb : ringBonds) {
            if (rb.order != 4) allAromatic = false;
            if (rb.order != 1) allSingle = false;
            if (rb.order == 2) hasDouble = true;
            if (rb.order == 3) hasTriple = true;
        }

        if (hasTriple) {
            return {false, "", "Cycloalkynes are not supported in Phase 2."};
        }

        if (ringSize == 6 && (allAromatic || (!allSingle && !hasDouble))) {
            rType = RingType::BENZENE; parentNameRoot = "benzene";
        } else if (allSingle) {
            rType = RingType::CYCLOALKANE; parentNameRoot = "cyclo" + chainRoot(ringSize) + "ane";
        } else {
            rType = RingType::CYCLOALKENE; parentNameRoot = "cyclo" + chainRoot(ringSize);
        }
    }

    // Pattern matching functional groups across whole molecule
    std::map<int, GroupType> carbonGroup;
    std::map<int, int> acylHalideHalogen;
    std::map<int, int> esterAlkylRoot;
    std::map<int, int> esterOxygen;
    std::set<int> etherOxygens;

    for (size_t i = 0; i < g.nodes.size(); ++i) {
        const GraphNode &node = g.nodes[i];
        if (node.atomicNumber == 6) {
            std::vector<int> doubleO, singleO, singleN, tripleN, halogens;

            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;

                if (nZ == 8 && order == 2) doubleO.push_back(nei);
                else if (nZ == 8 && order == 1) singleO.push_back(nei);
                else if (nZ == 7 && order == 1) {
                    bool isNitroIsoOrAzide = false;
                    if (carbonAzide.count(i) && std::find(carbonAzide[i].begin(), carbonAzide[i].end(), nei) != carbonAzide[i].end()) isNitroIsoOrAzide = true;
                    if (!isNitroIsoOrAzide) singleN.push_back(nei);
                }
                else if (nZ == 7 && order == 3) tripleN.push_back(nei);
                else if ((nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) && order == 1) halogens.push_back(nei);
            }

            if (!doubleO.empty()) {
                for (int sO : singleO) {
                    int cCount = 0;
                    int alkylRootNode = -1;
                    for (size_t k = 0; k < g.nodes[sO].neighbors.size(); ++k) {
                        int oNei = g.nodes[sO].neighbors[k];
                        if (g.nodes[oNei].atomicNumber == 6) {
                            cCount++;
                            if (oNei != static_cast<int>(i)) {
                                alkylRootNode = oNei;
                            }
                        }
                    }
                    if (cCount == 2 && alkylRootNode != -1) {
                        carbonGroup[i] = GroupType::ESTER;
                        esterAlkylRoot[i] = alkylRootNode;
                        esterOxygen[i] = sO;
                    }
                }
            }

            if (carbonGroup.count(i) && carbonGroup[i] == GroupType::ESTER) continue;

            if (!doubleO.empty() && !singleO.empty()) {
                bool hasOH = false;
                for (int sO : singleO) {
                    if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                        hasOH = true; break;
                    }
                }
                if (hasOH) carbonGroup[i] = GroupType::ACID;
            } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                carbonGroup[i] = GroupType::ACYL_HALIDE;
                acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
            } else if (!doubleO.empty() && !singleN.empty()) {
                carbonGroup[i] = GroupType::AMIDE;
            } else if (!tripleN.empty()) {
                carbonGroup[i] = GroupType::NITRILE;
            } else if (!doubleO.empty() && (node.totalH >= 1 || node.neighbors.size() <= 2)) {
                carbonGroup[i] = GroupType::ALDEHYDE;
            } else if (!doubleO.empty()) {
                carbonGroup[i] = GroupType::KETONE;
            } else if (!singleO.empty()) {
                for (int sO : singleO) {
                    if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                        carbonGroup[i] = GroupType::ALCOHOL; break;
                    }
                }
            } else if (!singleN.empty()) {
                carbonGroup[i] = GroupType::AMINE;
            } else if (carbonSulfonicAcid.count(i) && ringNodeSet.count(i) > 0) {
                carbonGroup[i] = GroupType::SULFONIC_ACID;
            } else if (carbonThiol.count(i) && ringNodeSet.count(i) > 0) {
                carbonGroup[i] = GroupType::THIOL;
            }
        } else if (node.atomicNumber == 8) {
            if (node.neighbors.size() == 2 && node.bondOrders[0] == 1 && node.bondOrders[1] == 1) {
                int n1 = node.neighbors[0];
                int n2 = node.neighbors[1];
                if (g.nodes[n1].atomicNumber == 6 && g.nodes[n2].atomicNumber == 6) {
                    etherOxygens.insert(node.id);
                }
            }
        }
    }

    auto groupRank = [](GroupType gt) -> int {
        switch (gt) {
            case GroupType::SULFONIC_ACID: return 1;
            case GroupType::ACID: return 2;
            case GroupType::BORONIC_ACID: return 3;
            case GroupType::ESTER: return 4;
            case GroupType::ACYL_HALIDE: return 5;
            case GroupType::AMIDE: return 6;
            case GroupType::NITRILE: return 7;
            case GroupType::ALDEHYDE: return 8;
            case GroupType::THIAL: return 9;
            case GroupType::KETONE: return 10;
            case GroupType::THIONE: return 11;
            case GroupType::ALCOHOL: return 12;
            case GroupType::THIOL: return 13;
            case GroupType::AMINE: return 14;
            case GroupType::PHOSPHINE: return 15;
            default: return 16;
        }
    };

    GroupType winningRingGroup = GroupType::NONE;
    GroupType winningChainGroup = GroupType::NONE;

    for (const auto &pair : carbonGroup) {
        int cNode = pair.first;
        GroupType gt = pair.second;
        bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
        if (!isOnOrExocyclic) {
            for (int nei : g.nodes[cNode].neighbors) {
                if (ringNodeSet.count(nei) > 0) {
                    isOnOrExocyclic = true;
                    break;
                }
            }
        }
        if (isOnOrExocyclic) {
            if (groupRank(gt) < groupRank(winningRingGroup)) winningRingGroup = gt;
        } else {
            if (groupRank(gt) < groupRank(winningChainGroup)) winningChainGroup = gt;
        }
    }

    if (groupRank(winningChainGroup) < groupRank(winningRingGroup)) {
        return {false, "", "A chain-based principal group outranks the ring in this structure; ring-vs-chain seniority is not yet supported in Phase 2."};
    }

    GroupType winningType = winningRingGroup;
    std::set<int> principalCarbons;
    if (winningType != GroupType::NONE) {
        for (const auto &pair : carbonGroup) {
            int cNode = pair.first;
            GroupType gt = pair.second;
            if (gt == winningType) {
                bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
                if (!isOnOrExocyclic) {
                    for (int nei : g.nodes[cNode].neighbors) {
                        if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                    }
                }
                if (isOnOrExocyclic) principalCarbons.insert(cNode);
            }
        }
    }

    std::vector<std::vector<int>> ringCandidates;
    if (rType == RingType::FURAN || rType == RingType::THIOPHENE || rType == RingType::PYRROLE || rType == RingType::PYRIDINE) {
        int hNode = ringHeteroNodes[0];
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hNode) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        ringCandidates.push_back(fwd);
        ringCandidates.push_back(bwd);
    } else if (rType == RingType::IMIDAZOLE || rType == RingType::PYRAZOLE) {
        int hNH = -1, hN = -1;
        if (g.nodes[ringHeteroNodes[0]].totalH >= 1) {
            hNH = ringHeteroNodes[0];
            hN = ringHeteroNodes[1];
        } else {
            hNH = ringHeteroNodes[1];
            hN = ringHeteroNodes[0];
        }
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hNH) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        int reqOtherIdx = (rType == RingType::IMIDAZOLE) ? 2 : 1;
        if (fwd[reqOtherIdx] == hN) ringCandidates.push_back(fwd);
        if (bwd[reqOtherIdx] == hN) ringCandidates.push_back(bwd);
    } else if (rType == RingType::PYRIMIDINE || rType == RingType::PYRIDAZINE || rType == RingType::PYRAZINE) {
        int n1 = ringHeteroNodes[0];
        int n2 = ringHeteroNodes[1];
        int idx1 = -1, idx2 = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == n1) idx1 = i;
            if (ringCycle[i] == n2) idx2 = i;
        }
        int reqOtherIdx = (rType == RingType::PYRIDAZINE) ? 1 : ((rType == RingType::PYRIMIDINE) ? 2 : 3);
        std::vector<int> fwd1(ringSize), bwd1(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd1[i] = ringCycle[(idx1 + i) % ringSize];
            bwd1[i] = ringCycle[(idx1 - i + ringSize) % ringSize];
        }
        if (fwd1[reqOtherIdx] == n2) ringCandidates.push_back(fwd1);
        if (bwd1[reqOtherIdx] == n2) ringCandidates.push_back(bwd1);

        std::vector<int> fwd2(ringSize), bwd2(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd2[i] = ringCycle[(idx2 + i) % ringSize];
            bwd2[i] = ringCycle[(idx2 - i + ringSize) % ringSize];
        }
        if (fwd2[reqOtherIdx] == n1) ringCandidates.push_back(fwd2);
        if (bwd2[reqOtherIdx] == n1) ringCandidates.push_back(bwd2);
    } else if (rType == RingType::OXAZOLE || rType == RingType::ISOXAZOLE || rType == RingType::THIAZOLE || rType == RingType::ISOTHIAZOLE) {
        int hOS = -1, hN = -1;
        int z0 = g.nodes[ringHeteroNodes[0]].atomicNumber;
        if (z0 == 8 || z0 == 16) {
            hOS = ringHeteroNodes[0];
            hN = ringHeteroNodes[1];
        } else {
            hOS = ringHeteroNodes[1];
            hN = ringHeteroNodes[0];
        }
        int hIdx = -1;
        for (int i = 0; i < ringSize; ++i) {
            if (ringCycle[i] == hOS) { hIdx = i; break; }
        }
        std::vector<int> fwd(ringSize), bwd(ringSize);
        for (int i = 0; i < ringSize; ++i) {
            fwd[i] = ringCycle[(hIdx + i) % ringSize];
            bwd[i] = ringCycle[(hIdx - i + ringSize) % ringSize];
        }
        int reqNIdx = (rType == RingType::ISOXAZOLE || rType == RingType::ISOTHIAZOLE) ? 1 : 2;
        if (fwd[reqNIdx] == hN) ringCandidates.push_back(fwd);
        if (bwd[reqNIdx] == hN) ringCandidates.push_back(bwd);
    } else if (rType == RingType::GENERAL_HETEROCYCLE) {
        int minRank = 99;
        for (int nodeIdx : ringHeteroNodes) {
            int z = g.nodes[nodeIdx].atomicNumber;
            int r = (z == 8) ? 0 : ((z == 16) ? 1 : ((z == 7) ? 2 : z));
            if (r < minRank) minRank = r;
        }

        for (int st = 0; st < ringSize; ++st) {
            int nodeIdx = ringCycle[st];
            int z = g.nodes[nodeIdx].atomicNumber;
            int r = (z == 8) ? 0 : ((z == 16) ? 1 : ((z == 7) ? 2 : z));
            if (r == minRank) {
                std::vector<int> fwd(ringSize), bwd(ringSize);
                for (int i = 0; i < ringSize; ++i) {
                    fwd[i] = ringCycle[(st + i) % ringSize];
                    bwd[i] = ringCycle[(st - i + ringSize) % ringSize];
                }
                ringCandidates.push_back(fwd);
                ringCandidates.push_back(bwd);
            }
        }
    } else {
        for (int st = 0; st < ringSize; ++st) {
            std::vector<int> fwd(ringSize), bwd(ringSize);
            for (int i = 0; i < ringSize; ++i) {
                fwd[i] = ringCycle[(st + i) % ringSize];
                bwd[i] = ringCycle[(st - i + ringSize) % ringSize];
            }
            ringCandidates.push_back(fwd);
            ringCandidates.push_back(bwd);
        }
    }

    struct RingSignature {
        std::vector<int> ringChain;
        std::vector<int> heteroatomLocants;
        std::vector<int> heteroatomSeniorityAtLocants;
        std::vector<int> principalLocants;
        std::vector<int> doubleBondLocants;
        std::vector<int> tripleBondLocants;
        std::vector<int> substituentLocants;
        std::vector<std::pair<QString, int>> namedSubstituents;
    };

    std::vector<RingSignature> ringSignatures;

    for (const auto &cand : ringCandidates) {
        RingSignature sig;
        sig.ringChain = cand;
        std::map<int, int> locantMap;
        for (int i = 0; i < ringSize; ++i) {
            locantMap[cand[i]] = i + 1;
            int nodeIdx = cand[i];
            int z = g.nodes[nodeIdx].atomicNumber;
            if (z != 6) {
                int locant = i + 1;
                sig.heteroatomLocants.push_back(locant);
                int rank = 99;
                if (z == 8) rank = 0;      // O
                else if (z == 16) rank = 1;// S
                else if (z == 7) rank = 2; // N
                else rank = z;
                sig.heteroatomSeniorityAtLocants.push_back(rank);
            }
        }

        for (int pC : principalCarbons) {
            if (ringNodeSet.count(pC)) {
                sig.principalLocants.push_back(locantMap[pC]);
            } else {
                for (int nei : g.nodes[pC].neighbors) {
                    if (ringNodeSet.count(nei)) {
                        sig.principalLocants.push_back(locantMap[nei]);
                        break;
                    }
                }
            }
        }
        std::sort(sig.principalLocants.begin(), sig.principalLocants.end());

        if (rType == RingType::CYCLOALKENE) {
            for (const auto &rb : ringBonds) {
                if (rb.order == 2) {
                    int l1 = locantMap[rb.u];
                    int l2 = locantMap[rb.v];
                    int bLoc = std::min(l1, l2);
                    if ((l1 == 1 && l2 == ringSize) || (l1 == ringSize && l2 == 1)) bLoc = ringSize;
                    sig.doubleBondLocants.push_back(bLoc);
                }
            }
            std::sort(sig.doubleBondLocants.begin(), sig.doubleBondLocants.end());
        }

        for (int i = 0; i < ringSize; ++i) {
            int rNode = cand[i];
            int locant = i + 1;
            bool isPrincipalRNode = (ringNodeSet.count(rNode) > 0 && principalCarbons.count(rNode) > 0);

            for (size_t j = 0; j < g.nodes[rNode].neighbors.size(); ++j) {
                int nei = g.nodes[rNode].neighbors[j];
                int order = g.nodes[rNode].bondOrders[j];
                if (ringNodeSet.count(nei)) continue;

                if (winningType != GroupType::NONE) {
                    if ((winningType == GroupType::ALCOHOL || winningType == GroupType::KETONE || winningType == GroupType::AMINE || winningType == GroupType::SULFONIC_ACID || winningType == GroupType::THIOL) && isPrincipalRNode) {
                        int nz = g.nodes[nei].atomicNumber;
                        bool isAzide = (carbonAzide.count(rNode) && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end());
                        if (!isAzide && (nz == 7 || nz == 8 || nz == 16)) continue;
                    }
                    if (principalCarbons.count(nei) > 0) continue;
                }

                int nz = g.nodes[nei].atomicNumber;
                QString subName;
                if (nz == 9 || nz == 17 || nz == 35 || nz == 53) {
                    subName = halogenPrefix(nz);
                } else if (nz == 8) {
                    if (order == 1) {
                        if (etherOxygens.count(nei)) {
                            int alkylNei = -1;
                            for (int oNei : g.nodes[nei].neighbors) {
                                if (oNei != rNode) { alkylNei = oNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                if (alkylName.endsWith("yl")) {
                                    alkylName.chop(2); alkylName += "oxy";
                                }
                                subName = alkylName;
                            }
                        } else if (winningType != GroupType::ALCOHOL) {
                            subName = "hydroxy";
                        }
                    } else if (order == 2) {
                        if (winningType != GroupType::ALDEHYDE && winningType != GroupType::KETONE) {
                            subName = "oxo";
                        }
                    }
                } else if (nz == 7) {
                    bool isAzide = false;
                    for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                        int nNei = g.nodes[nei].neighbors[k];
                        if (g.nodes[nNei].atomicNumber == 7 && g.nodes[nei].bondOrders[k] >= 2) {
                            isAzide = true; break;
                        }
                    }
                    if (carbonAzide.count(rNode) && ringNodeSet.count(rNode) > 0 && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end()) {
                        subName = "azido";
                    } else if (!isAzide && order == 1 && winningType != GroupType::AMINE) {
                        subName = "amino";
                    }
                } else if (nz == 16) {
                    if (carbonSulfonicAcid.count(rNode) && carbonSulfonicAcid[rNode] == nei && winningType != GroupType::SULFONIC_ACID) {
                        subName = "sulfo";
                    } else if (carbonThiol.count(rNode) && carbonThiol[rNode] == nei && winningType != GroupType::THIOL) {
                        subName = "sulfanyl";
                    } else if (order == 1) {
                        if (thioetherSulfurs.count(nei)) {
                            int alkylNei = -1;
                            for (int sNei : g.nodes[nei].neighbors) {
                                if (sNei != rNode) { alkylNei = sNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                subName = alkylName + "sulfanyl";
                            }
                        }
                    }
                } else if (nz == 6) {
                    if (carbonGroup.count(nei) && winningType != carbonGroup[nei]) {
                        if (carbonGroup[nei] == GroupType::ACID) {
                            subName = "carboxy";
                        } else if (carbonGroup[nei] == GroupType::ACYL_HALIDE) {
                            subName = halogenPrefix(acylHalideHalogen[nei]) + "carbonyl";
                        } else if (carbonGroup[nei] == GroupType::ESTER) {
                            int alkylRoot = esterAlkylRoot[nei];
                            int sO = esterOxygen[nei];
                            QString alkylName = nameBranchGraph(g, alkylRoot, sO);
                            if (alkylName.isEmpty()) {
                                return {false, "", "Unsupported ester alkyl group."};
                            }
                            if (alkylName.endsWith("yl")) {
                                alkylName.chop(2);
                                alkylName += "oxycarbonyl";
                            }
                            subName = alkylName;
                        }
                    }
                    if (subName.isEmpty()) {
                        subName = nameBranchGraph(g, nei, rNode, allSSSRRings, ringNodeSet);
                        if (subName.startsWith("(")) {
                            subName = subName.mid(1);
                            if (subName.endsWith(")")) subName.chop(1);
                        }
                    }
                }

                if (!subName.isEmpty()) {
                    sig.substituentLocants.push_back(locant);
                    sig.namedSubstituents.push_back({subName, locant});
                } else {
                    return {false, "", "Unrecognized or unsupported substituent on ring."};
                }
            }
        }
        std::sort(sig.substituentLocants.begin(), sig.substituentLocants.end());
        ringSignatures.push_back(sig);
    }

    auto bestIt = std::min_element(ringSignatures.begin(), ringSignatures.end(),
        [](const RingSignature &a, const RingSignature &b) {
            if (a.heteroatomLocants != b.heteroatomLocants) return a.heteroatomLocants < b.heteroatomLocants;
            if (a.heteroatomSeniorityAtLocants != b.heteroatomSeniorityAtLocants) return a.heteroatomSeniorityAtLocants < b.heteroatomSeniorityAtLocants;
            if (a.principalLocants != b.principalLocants) return a.principalLocants < b.principalLocants;
            if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;
            if (a.tripleBondLocants != b.tripleBondLocants) return a.tripleBondLocants < b.tripleBondLocants;
            if (a.substituentLocants != b.substituentLocants) return a.substituentLocants < b.substituentLocants;

            auto firstAlpha = [](const std::vector<std::pair<QString,int>> &named) {
                return std::min_element(named.begin(), named.end(),
                    [](const auto &x, const auto &y) { return x.first.toLower() < y.first.toLower(); });
            };
            if (!a.namedSubstituents.empty()) {
                QString alphaName = firstAlpha(a.namedSubstituents)->first;
                auto findLocant = [&](const std::vector<std::pair<QString,int>> &named) {
                    int best = INT_MAX;
                    for (const auto &ns : named) if (ns.first == alphaName) best = std::min(best, ns.second);
                    return best;
                };
                int aLoc = findLocant(a.namedSubstituents);
                int bLoc = findLocant(b.namedSubstituents);
                if (aLoc != bLoc) return aLoc < bLoc;
            }
            return false;
        });

    RingSignature bestSig = *bestIt;
    if (rType == RingType::GENERAL_HETEROCYCLE) {
        std::vector<int> oLocs, sLocs, nLocs;
        for (size_t i = 0; i < bestSig.ringChain.size(); ++i) {
            int nodeIdx = bestSig.ringChain[i];
            int z = g.nodes[nodeIdx].atomicNumber;
            int locant = static_cast<int>(i + 1);
            if (z == 8) oLocs.push_back(locant);
            else if (z == 16) sLocs.push_back(locant);
            else if (z == 7) nLocs.push_back(locant);
        }

        std::vector<int> allLocs;
        allLocs.insert(allLocs.end(), oLocs.begin(), oLocs.end());
        allLocs.insert(allLocs.end(), sLocs.begin(), sLocs.end());
        allLocs.insert(allLocs.end(), nLocs.begin(), nLocs.end());

        QStringList locStrs;
        for (int l : allLocs) locStrs.append(QString::number(l));
        QString locantPrefix = locStrs.join(",") + "-";

        QString elemPrefixes;
        auto appendElem = [&](int count, const QString &aPrefix) {
            if (count == 0) return;
            QString p;
            if (count == 1) {
                p = aPrefix;
            } else {
                if (aPrefix == "aza" && count == 4) {
                    p = "tetraza";
                } else {
                    p = multiPrefix(count) + aPrefix;
                }
            }
            if (elemPrefixes.endsWith("a") && p.startsWith("a")) {
                elemPrefixes.chop(1);
            }
            elemPrefixes += p;
        };

        appendElem(static_cast<int>(oLocs.size()), "oxa");
        appendElem(static_cast<int>(sLocs.size()), "thia");
        appendElem(static_cast<int>(nLocs.size()), "aza");

        QString stem = (ringSize == 5) ? "ole" : "ine";
        if (elemPrefixes.endsWith("a")) {
            elemPrefixes.chop(1);
        }

        parentNameRoot = locantPrefix + elemPrefixes + stem;
    }
    std::map<int, int> graphIdToLocant;
    for (size_t i = 0; i < bestSig.ringChain.size(); ++i) {
        graphIdToLocant[bestSig.ringChain[i]] = static_cast<int>(i + 1);
    }
    StereoResult ezRes = processDoubleBondStereo(mol, g, ringNodeSet, graphIdToLocant, stereoByGraphId);
    if (!ezRes.ok) {
        return {false, "", ezRes.error};
    }
    StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, graphIdToLocant);
    if (!stereoRes.ok) {
        return {false, "", stereoRes.error};
    }

    std::map<QString, std::vector<int>> prefixLocantsMap;
    for (const auto &ns : bestSig.namedSubstituents) {
        prefixLocantsMap[ns.first].push_back(ns.second);
    }

    struct PrefixGroup {
        QString baseName;
        QString formattedStr;
    };
    std::vector<PrefixGroup> pGroups;
    for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
        QString pName = it->first;
        std::vector<int> locs = it->second;
        std::sort(locs.begin(), locs.end());

        QStringList locStrs;
        for (int l : locs) locStrs.append(QString::number(l));

        QString pStr;
        if (prefixLocantsMap.size() == 1 && locs.size() == 1 && locs[0] == 1 && winningType == GroupType::NONE &&
            (rType == RingType::BENZENE || rType == RingType::CYCLOALKANE || rType == RingType::CYCLOALKENE)) {
            // Locant "1" is only omittable when this is the ONLY substituent on the ring --
            // position is unambiguous with nothing else to distinguish it from (e.g.
            // "chlorocyclohexane"). With a second, different substituent also present (even if
            // that one also happens to be numbered "1"), the locant must stay explicit to
            // disambiguate which position is which (e.g. "1-chloro-2-methylcyclohexane").
            pStr = pName;
        } else {
            pStr = locStrs.join(",");
            if (locs.size() > 1) {
                pStr += "-" + multiPrefix(static_cast<int>(locs.size())) + pName;
            } else {
                pStr += "-" + pName;
            }
        }

        PrefixGroup pg;
        pg.baseName = pName.startsWith("(") ? pName.mid(1) : pName;
        pg.formattedStr = pStr;
        pGroups.push_back(pg);
    }

    std::sort(pGroups.begin(), pGroups.end(), [](const PrefixGroup &a, const PrefixGroup &b) {
        return a.baseName.toLower() < b.baseName.toLower();
    });

    QString rootStr = parentNameRoot;
    if (rType == RingType::CYCLOALKENE) {
        std::vector<int> dbLocs = bestSig.doubleBondLocants;
        if (dbLocs.size() == 1 && winningType == GroupType::NONE && pGroups.empty()) {
            rootStr += "en";
        } else if (dbLocs.size() == 1) {
            rootStr += QString("-%1-en").arg(dbLocs[0]);
        } else if (dbLocs.size() > 1) {
            rootStr += "a";
            QStringList lStrs;
            for (int l : dbLocs) lStrs.append(QString::number(l));
            rootStr += QString("-%1-%2en").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
        }
    }

    QString prefixPart;
    if (!pGroups.empty()) {
        QStringList pStrs;
        for (const auto &pg : pGroups) pStrs.append(pg.formattedStr);
        prefixPart = pStrs.join("-");
        if (!rootStr.isEmpty() && rootStr[0].isDigit()) {
            prefixPart += "-";
        }
    }

    QString fullName;
    if (winningType == GroupType::NONE) {
        if (rType == RingType::BENZENE || rType == RingType::FURAN || rType == RingType::THIOPHENE ||
            rType == RingType::PYRROLE || rType == RingType::PYRIDINE || rType == RingType::CYCLOALKANE ||
            rType == RingType::IMIDAZOLE || rType == RingType::PYRIMIDINE ||
            rType == RingType::PYRAZOLE || rType == RingType::OXAZOLE || rType == RingType::ISOXAZOLE ||
            rType == RingType::THIAZOLE || rType == RingType::ISOTHIAZOLE || rType == RingType::PYRIDAZINE ||
            rType == RingType::PYRAZINE || rType == RingType::GENERAL_HETEROCYCLE ||
            rType == RingType::SELENOPHENE || rType == RingType::TELLUROPHENE ||
            rType == RingType::PHOSPHININE || rType == RingType::SELENAZOLE ||
            rType == RingType::ISOSELENAZOLE) {
            fullName = prefixPart + rootStr;
        } else {
            fullName = prefixPart + rootStr + "e";
        }
    } else {
        bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE ||
                            winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||
                            winningType == GroupType::ACYL_HALIDE || winningType == GroupType::ESTER);
        int pCount = static_cast<int>(bestSig.principalLocants.size());

        if (isExocyclic) {
            QString sfx;
            if (winningType == GroupType::ACID) sfx = (pCount == 1) ? "carboxylic acid" : "dicarboxylic acid";
            else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";
            else if (winningType == GroupType::NITRILE) sfx = (pCount == 1) ? "carbonitrile" : "dicarbonitrile";
            else if (winningType == GroupType::ALDEHYDE) sfx = (pCount == 1) ? "carbaldehyde" : "dicarbaldehyde";
            else if (winningType == GroupType::ESTER) sfx = (pCount == 1) ? "carboxylate" : (multiPrefix(pCount) + "carboxylate");
            else if (winningType == GroupType::ACYL_HALIDE) {
                QString hName;
                int hz = acylHalideHalogen.empty() ? 17 : acylHalideHalogen.begin()->second;
                for (int pc : principalCarbons) {
                    if (acylHalideHalogen.count(pc)) { hz = acylHalideHalogen[pc]; break; }
                }
                if (hz == 9) hName = "fluoride";
                else if (hz == 17) hName = "chloride";
                else if (hz == 35) hName = "bromide";
                else if (hz == 53) hName = "iodide";
                sfx = (pCount == 1) ? ("carbonyl " + hName) : ("dicarbonyl " + hName);
            }

            if (pCount == 1) {
                // No space between the ring root and the suffix -- "carboxylic acid"'s own
                // internal space (between "carboxylic" and "acid") is already correctly
                // placed; the root concatenates directly onto it: "cyclohexanecarboxylic acid".
                fullName = prefixPart + rootStr + sfx;
            } else {
                QStringList lStrs;
                for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                fullName = prefixPart + rootStr + QString("-%1-%2").arg(lStrs.join(","), sfx);
            }
        } else {
            QString sfx;
            if (winningType == GroupType::SULFONIC_ACID) sfx = (pCount == 1) ? "sulfonic acid" : "disulfonic acid";
            else if (winningType == GroupType::THIOL) sfx = (pCount == 1) ? "thiol" : "dithiol";
            else if (winningType == GroupType::ALCOHOL) sfx = (pCount == 1) ? "ol" : "diol";
            else if (winningType == GroupType::KETONE) sfx = (pCount == 1) ? "one" : "dione";
            else if (winningType == GroupType::AMINE) sfx = (pCount == 1) ? "amine" : "diamine";

            if (pCount == 1) {
                QString stem = rootStr;
                if (stem.endsWith("e") && !sfx.isEmpty() && isVowel(sfx[0])) stem.chop(1);
                fullName = prefixPart + stem + sfx;
            } else {
                QStringList lStrs;
                for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                fullName = prefixPart + rootStr + QString("-%1-%2").arg(lStrs.join(","), sfx);
            }
        }
    }

    if (winningType == GroupType::ESTER) {
        int esterCarbon = -1;
        for (int pc : principalCarbons) {
            if (carbonGroup.count(pc) && carbonGroup[pc] == GroupType::ESTER) {
                esterCarbon = pc;
                break;
            }
        }
        if (esterCarbon != -1) {
            int alkylRoot = esterAlkylRoot[esterCarbon];
            int sO = esterOxygen[esterCarbon];
            QString alkylName = nameBranchGraph(g, alkylRoot, sO, allSSSRRings);
            if (alkylName.isEmpty()) {
                return {false, "", "Unsupported ester alkyl group."};
            }
            fullName = alkylName + " " + fullName;
        }
    }

    fullName = stereoRes.prefix + fullName;
    return {true, fullName, ""};
}

std::map<int, QString> computePeripheralNumbering3Ring(
    const Graph &g,
    const std::set<int> &ring1,
    const std::set<int> &ring2,
    const std::set<int> &ring3,
    const std::set<int> &bridgeheads,
    const std::set<int> &substituentBearingNodes,
    bool preferIndicatedHydrogenLocant)
{
    std::map<int, QString> emptyMap;

    std::set<int> allNodes = ring1;
    allNodes.insert(ring2.begin(), ring2.end());
    allNodes.insert(ring3.begin(), ring3.end());

    std::map<std::pair<int, int>, int> edgeRingCount;
    auto addRingEdges = [&](const std::set<int> &r) {
        for (int u : r) {
            for (int v : g.nodes[u].neighbors) {
                if (r.count(v) && u < v) {
                    edgeRingCount[{u, v}]++;
                }
            }
        }
    };
    addRingEdges(ring1);
    addRingEdges(ring2);
    addRingEdges(ring3);

    std::map<int, std::vector<int>> periphGraph;
    for (const auto &kv : edgeRingCount) {
        if (kv.second == 1) {
            int u = kv.first.first;
            int v = kv.first.second;
            periphGraph[u].push_back(v);
            periphGraph[v].push_back(u);
        }
    }

    if (periphGraph.empty()) return emptyMap;
    int startNode = periphGraph.begin()->first;
    std::vector<int> cycle;
    int curr = startNode;
    int prev = -1;
    while (true) {
        cycle.push_back(curr);
        if (periphGraph[curr].size() != 2) return emptyMap;
        int next = (periphGraph[curr][0] == prev) ? periphGraph[curr][1] : periphGraph[curr][0];
        prev = curr;
        curr = next;
        if (curr == startNode) break;
    }

    struct Candidate {
        std::map<int, QString> locantMap;
        std::vector<int> heteroatomLocants;
        std::vector<std::pair<int, int>> heteroatomSeniorityAtLocants;
        std::vector<int> substituentLocants;
        int candidateIndex;
    };

    std::vector<Candidate> candidates;
    int candIdx = 0;
    int N = cycle.size();

    for (int dir = -1; dir <= 1; dir += 2) {
        for (int i = 0; i < N; ++i) {
            int firstNode = cycle[i];
            int lastNode = cycle[(i - dir + N) % N];
            
            if (!bridgeheads.count(firstNode) && bridgeheads.count(lastNode)) {
                Candidate cand;
                cand.candidateIndex = candIdx++;
                
                int loc = 1;
                int lastNonBhLoc = 0;
                int numBhSince = 0;
                
                for (int step = 0; step < N; ++step) {
                    int currIdx = (i + step * dir + N * N) % N;
                    int u = cycle[currIdx];
                    
                    if (!bridgeheads.count(u)) {
                        numBhSince = 0;
                        cand.locantMap[u] = QString::number(loc);
                        lastNonBhLoc = loc;
                        loc++;
                    } else {
                        numBhSince++;
                        char suffix = 'a' + numBhSince - 1;
                        cand.locantMap[u] = QString::number(lastNonBhLoc) + suffix;
                    }
                }
                
                for (int node : allNodes) {
                    int z = g.nodes[node].atomicNumber;
                    if (z != 6) {
                        QString locStr = cand.locantMap[node];
                        int numLoc = 0;
                        std::string s = locStr.toStdString();
                        for (char c : s) {
                            if (std::isdigit(static_cast<unsigned char>(c))) {
                                numLoc = numLoc * 10 + (c - '0');
                            } else {
                                break;
                            }
                        }
                        int rank = 99;
                        if (z == 8) rank = 0;
                        else if (z == 16) rank = 1;
                        else if (z == 7) {
                            if (preferIndicatedHydrogenLocant && g.nodes[node].totalH >= 1) rank = 2;
                            else if (preferIndicatedHydrogenLocant) rank = 3;
                            else rank = 2;
                        }
                        else rank = z + 10;
        
                        cand.heteroatomLocants.push_back(numLoc);
                        cand.heteroatomSeniorityAtLocants.push_back({numLoc, rank});
                    }
                }
        
                for (int node : substituentBearingNodes) {
                    auto it = cand.locantMap.find(node);
                    if (it != cand.locantMap.end()) {
                        QString locStr = it->second;
                        int numLoc = 0;
                        std::string s = locStr.toStdString();
                        for (char c : s) {
                            if (std::isdigit(static_cast<unsigned char>(c))) {
                                numLoc = numLoc * 10 + (c - '0');
                            } else {
                                break;
                            }
                        }
                        cand.substituentLocants.push_back(numLoc);
                    }
                }
        
                std::sort(cand.heteroatomLocants.begin(), cand.heteroatomLocants.end());
                std::sort(cand.heteroatomSeniorityAtLocants.begin(), cand.heteroatomSeniorityAtLocants.end());
                std::sort(cand.substituentLocants.begin(), cand.substituentLocants.end());
        
                candidates.push_back(cand);
            }
        }
    }

    if (candidates.empty()) return emptyMap;

    auto bestIt = std::min_element(candidates.begin(), candidates.end(),
        [](const Candidate &a, const Candidate &b) {
            if (a.heteroatomLocants != b.heteroatomLocants)
                return a.heteroatomLocants < b.heteroatomLocants;
            if (a.heteroatomSeniorityAtLocants != b.heteroatomSeniorityAtLocants)
                return a.heteroatomSeniorityAtLocants < b.heteroatomSeniorityAtLocants;
            if (a.substituentLocants != b.substituentLocants)
                return a.substituentLocants < b.substituentLocants;
            return a.candidateIndex < b.candidateIndex;
        });

    return bestIt->locantMap;
}

std::map<int, QString> computePeripheralNumbering(
    const Graph &g,
    const std::set<int> &ring1Nodes,
    const std::set<int> &ring2Nodes,
    int bhA,
    int bhB,
    const std::set<int> &substituentBearingNodes,
    bool preferIndicatedHydrogenLocant)
{
    std::map<int, QString> emptyMap;

    if (ring1Nodes.empty() || ring2Nodes.empty()) return emptyMap;
    if (!ring1Nodes.count(bhA) || !ring1Nodes.count(bhB)) return emptyMap;
    if (!ring2Nodes.count(bhA) || !ring2Nodes.count(bhB)) return emptyMap;
    if (bhA == bhB) return emptyMap;

    bool bhBonded = false;
    for (int nei : g.nodes[bhA].neighbors) {
        if (nei == bhB) { bhBonded = true; break; }
    }
    if (!bhBonded) return emptyMap;

    auto extractOuterPath = [&](const std::set<int> &rNodes, int startBh, int endBh) -> std::vector<int> {
        std::vector<int> path;
        int curr = startBh;
        int prev = endBh;
        while (true) {
            int nextNode = -1;
            for (int nei : g.nodes[curr].neighbors) {
                if (rNodes.count(nei) && nei != prev) {
                    nextNode = nei;
                    break;
                }
            }
            if (nextNode == -1 || nextNode == endBh) break;
            path.push_back(nextNode);
            prev = curr;
            curr = nextNode;
        }
        return path;
    };

    std::vector<int> P1 = extractOuterPath(ring1Nodes, bhA, bhB);
    std::vector<int> P2 = extractOuterPath(ring2Nodes, bhA, bhB);

    if (P1.empty() || P2.empty()) return emptyMap;

    std::vector<int> P1_rev = P1;
    std::reverse(P1_rev.begin(), P1_rev.end());
    std::vector<int> P2_rev = P2;
    std::reverse(P2_rev.begin(), P2_rev.end());

    struct Candidate {
        std::map<int, QString> locantMap;
        std::vector<int> heteroatomLocants;
        std::vector<std::pair<int, int>> heteroatomSeniorityAtLocants;
        std::vector<int> substituentLocants;
        int candidateIndex;
    };

    std::vector<Candidate> candidates;

    auto buildCandidate = [&](int candIdx, const std::vector<int> &firstPath, int midBh, const std::vector<int> &secondPath, int endBh) {
        Candidate cand;
        cand.candidateIndex = candIdx;
        int loc = 1;
        for (int node : firstPath) {
            cand.locantMap[node] = QString::number(loc++);
        }
        int firstLen = static_cast<int>(firstPath.size());
        cand.locantMap[midBh] = QString::number(firstLen) + "a";

        for (int node : secondPath) {
            cand.locantMap[node] = QString::number(loc++);
        }
        int totalOuter = static_cast<int>(firstPath.size() + secondPath.size());
        cand.locantMap[endBh] = QString::number(totalOuter) + "a";

        std::set<int> allNodes = ring1Nodes;
        allNodes.insert(ring2Nodes.begin(), ring2Nodes.end());

        for (int node : allNodes) {
            int z = g.nodes[node].atomicNumber;
            if (z != 6) {
                QString locStr = cand.locantMap[node];
                int numLoc = 0;
                std::string s = locStr.toStdString();
                for (char c : s) {
                    if (std::isdigit(static_cast<unsigned char>(c))) {
                        numLoc = numLoc * 10 + (c - '0');
                    } else {
                        break;
                    }
                }
                int rank = 99;
                if (z == 8) rank = 0;       // O
                else if (z == 16) rank = 1; // S
                else if (z == 7) {
                    if (preferIndicatedHydrogenLocant && g.nodes[node].totalH >= 1) rank = 2; // NH (saturated heteroatom)
                    else if (preferIndicatedHydrogenLocant) rank = 3;                           // =N- (unsaturated heteroatom)
                    else rank = 2;
                }
                else rank = z + 10;

                cand.heteroatomLocants.push_back(numLoc);
                cand.heteroatomSeniorityAtLocants.push_back({numLoc, rank});
            }
        }

        for (int node : substituentBearingNodes) {
            auto it = cand.locantMap.find(node);
            if (it != cand.locantMap.end()) {
                QString locStr = it->second;
                int numLoc = 0;
                std::string s = locStr.toStdString();
                for (char c : s) {
                    if (std::isdigit(static_cast<unsigned char>(c))) {
                        numLoc = numLoc * 10 + (c - '0');
                    } else {
                        break;
                    }
                }
                cand.substituentLocants.push_back(numLoc);
            }
        }

        std::sort(cand.heteroatomLocants.begin(), cand.heteroatomLocants.end());
        std::sort(cand.heteroatomSeniorityAtLocants.begin(), cand.heteroatomSeniorityAtLocants.end());
        std::sort(cand.substituentLocants.begin(), cand.substituentLocants.end());

        candidates.push_back(cand);
    };

    buildCandidate(1, P1, bhB, P2_rev, bhA);
    buildCandidate(2, P2, bhB, P1_rev, bhA);
    buildCandidate(3, P1_rev, bhA, P2, bhB);
    buildCandidate(4, P2_rev, bhA, P1, bhB);

    if (candidates.empty()) return emptyMap;

    auto bestIt = std::min_element(candidates.begin(), candidates.end(),
        [](const Candidate &a, const Candidate &b) {
            if (a.heteroatomLocants != b.heteroatomLocants)
                return a.heteroatomLocants < b.heteroatomLocants;
            if (a.heteroatomSeniorityAtLocants != b.heteroatomSeniorityAtLocants)
                return a.heteroatomSeniorityAtLocants < b.heteroatomSeniorityAtLocants;
            if (a.substituentLocants != b.substituentLocants)
                return a.substituentLocants < b.substituentLocants;
            return a.candidateIndex < b.candidateIndex;
        });

    return bestIt->locantMap;
}

std::map<int, QString> computePeripheralNumberingForMol(int mol) {
    std::map<int, QString> result;
    if (mol <= 0) return result;

    Graph g;
    std::map<int, int> indigoToGraphIdx;
    std::vector<int> heavyAtomIndices;

    int atomIter = indigoIterateAtoms(mol);
    if (atomIter < 0) return result;
    int atomHandle = 0;
    while ((atomHandle = indigoNext(atomIter)) != 0) {
        int idx = indigoIndex(atomHandle);
        int z = indigoAtomicNumber(atomHandle);
        if (z > 1) {
            heavyAtomIndices.push_back(idx);
        }
        indigoFree(atomHandle);
    }
    indigoFree(atomIter);

    if (heavyAtomIndices.empty()) return result;

    for (size_t i = 0; i < heavyAtomIndices.size(); ++i) {
        int idx = heavyAtomIndices[i];
        indigoToGraphIdx[idx] = static_cast<int>(i);

        int aObj = indigoGetAtom(mol, idx);
        int z = indigoAtomicNumber(aObj);
        int implicitH = indigoCountImplicitHydrogens(aObj);

        GraphNode node;
        node.id = static_cast<int>(i);
        node.indigoIdx = idx;
        node.atomicNumber = z;
        node.totalH = implicitH;
        g.nodes.push_back(node);
        indigoFree(aObj);
    }

    int bondIter = indigoIterateBonds(mol);
    if (bondIter >= 0) {
        int bondHandle = 0;
        while ((bondHandle = indigoNext(bondIter)) != 0) {
            int srcHandle = indigoSource(bondHandle);
            int dstHandle = indigoDestination(bondHandle);
            int src = indigoIndex(srcHandle);
            int dst = indigoIndex(dstHandle);
            int order = indigoBondOrder(bondHandle);

            if (indigoToGraphIdx.count(src) && indigoToGraphIdx.count(dst)) {
                int u = indigoToGraphIdx[src];
                int v = indigoToGraphIdx[dst];

                g.nodes[u].neighbors.push_back(v);
                g.nodes[u].bondOrders.push_back(order);
                g.nodes[v].neighbors.push_back(u);
                g.nodes[v].bondOrders.push_back(order);

                GraphBond gb;
                gb.u = u;
                gb.v = v;
                gb.order = order;
                g.bonds.push_back(gb);
            }
            indigoFree(srcHandle);
            indigoFree(dstHandle);
            indigoFree(bondHandle);
        }
        indigoFree(bondIter);
    }

    int sssrIter = indigoIterateSSSR(mol);
    if (sssrIter < 0) return result;

    std::vector<std::set<int>> sssrRings;
    int subMol = 0;
    while ((subMol = indigoNext(sssrIter)) != 0) {
        std::set<int> rAtoms;
        int ringAtomIter = indigoIterateAtoms(subMol);
        if (ringAtomIter >= 0) {
            int rAtomHandle = 0;
            while ((rAtomHandle = indigoNext(ringAtomIter)) != 0) {
                int idx = indigoIndex(rAtomHandle);
                rAtoms.insert(idx);
                indigoFree(rAtomHandle);
            }
            indigoFree(ringAtomIter);
        }
        sssrRings.push_back(rAtoms);
        indigoFree(subMol);
    }
    indigoFree(sssrIter);

    if (sssrRings.size() != 2) return result;

    std::set<int> ring1Nodes, ring2Nodes;
    for (int idx : sssrRings[0]) {
        if (indigoToGraphIdx.count(idx)) ring1Nodes.insert(indigoToGraphIdx[idx]);
    }
    for (int idx : sssrRings[1]) {
        if (indigoToGraphIdx.count(idx)) ring2Nodes.insert(indigoToGraphIdx[idx]);
    }

    std::vector<int> sharedNodes;
    for (int n : ring1Nodes) {
        if (ring2Nodes.count(n)) sharedNodes.push_back(n);
    }

    if (sharedNodes.size() != 2) return result;

    int bhA = sharedNodes[0];
    int bhB = sharedNodes[1];

    return computePeripheralNumbering(g, ring1Nodes, ring2Nodes, bhA, bhB);
}


