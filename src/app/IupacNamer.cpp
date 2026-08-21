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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

// Maps each stereo-defined double bond (keyed by its two atom indices, min first)
// to its E/Z CIP descriptor, read directly from Indigo's KET JSON "cip" bond field.
// Declared here (external linkage) so IndigoService.cpp can share it too -- see
// IUPAC Blue Book Coverage.md item 8. Defined below, outside the anonymous
// namespace, right after IupacNamer::generateName's own file-scope block begins.
std::map<std::pair<int,int>, QChar> computeIndigoBondCIP(int mol);

namespace {

enum class GroupType {
    NONE = 0,
    SULFONIC_ACID, // Sulfonic acid
    SULFONYL_HALIDE, // Sulfonyl halide
    SULFINIC_ACID, // Sulfinic acid
    BORONIC_ACID,  // Boronic acid
    ACID,          // Carboxylic acid
    ESTER,         // Ester
    ACYL_HALIDE,   // Acyl halide
    AMIDE,         // Amide
    HYDRAZIDE,     // Hydrazide
    NITRILE,       // Nitrile
    ALDEHYDE,      // Aldehyde
    THIAL,         // Thial (C=S aldehyde analog)
    KETONE,        // Ketone
    THIONE,        // Thione (C=S ketone analog)
    ALCOHOL,       // Alcohol
    THIOL,         // Thiol
    SELENOL,       // Selenol (Se analogue of alcohol)
    TELLUROL,      // Tellurol (Te analogue of alcohol)
    HYDROPEROXIDE, // Hydroperoxide
    AMINE,         // Amine
    IMINE,         // Imine (C=NH)
    PHOSPHONIC_ACID, // Phosphonic acid
    PHOSPHONIC_DIHALIDE, // Phosphonic dihalide
    ARSONIC_ACID, // Arsonic acid
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

// Halogen suffix lookup
QString halogenSuffixWord(int z) {
    switch (z) {
        case 9:  return "fluoride";
        case 17: return "chloride";
        case 35: return "bromide";
        case 53: return "iodide";
        default: return "";
    }
}

// Structural check for an isocyanate/isothiocyanate nitrogen (N=C=O or
// N=C=S) singly bonded to `fromCarbon`, independent of any precomputed map
// so it works the same in every duplicated classification region.
bool isIsocyanateNitrogen(int nNode, int fromCarbon, const Graph &g) {
    const GraphNode &n = g.nodes[nNode];
    if (n.atomicNumber != 7 || n.totalH != 0 || n.neighbors.size() != 2) return false;
    for (size_t k = 0; k < n.neighbors.size(); ++k) {
        int nn = n.neighbors[k];
        if (nn == fromCarbon || n.bondOrders[k] != 2 || g.nodes[nn].atomicNumber != 6) continue;
        const GraphNode &isoC = g.nodes[nn];
        if (isoC.neighbors.size() != 2) continue;
        for (size_t m = 0; m < isoC.neighbors.size(); ++m) {
            int isoNei = isoC.neighbors[m];
            int isoZ = g.nodes[isoNei].atomicNumber;
            if ((isoZ == 8 || isoZ == 16) && isoC.bondOrders[m] == 2 && g.nodes[isoNei].neighbors.size() == 1) {
                return true;
            }
        }
    }
    return false;
}

bool isAcylPseudohalide(int i, const Graph &g, const std::map<int, std::vector<int>> &carbonAzide) {
    if (carbonAzide.count(i)) return true;
    const GraphNode &node = g.nodes[i];
    for (size_t j = 0; j < node.neighbors.size(); ++j) {
        int nei = node.neighbors[j];
        int nZ = g.nodes[nei].atomicNumber;
        int order = node.bondOrders[j];

        // Acyl cyanide: -C(=O)-C#N (nitrile carbon attached via a C-C bond,
        // with no other heavy-atom substituents on that nitrile carbon).
        if (nZ == 6 && order == 1) {
            const GraphNode &neiNode = g.nodes[nei];
            int tripleNCount = 0;
            int otherHeavyAtoms = 0;
            for (size_t k = 0; k < neiNode.neighbors.size(); ++k) {
                int nn = neiNode.neighbors[k];
                if (g.nodes[nn].atomicNumber == 7 && neiNode.bondOrders[k] == 3) {
                    tripleNCount++;
                } else if (nn != i && g.nodes[nn].atomicNumber > 1) {
                    otherHeavyAtoms++;
                }
            }
            if (tripleNCount == 1 && otherHeavyAtoms == 0) return true;
        }

        // Acyl isocyanate/isothiocyanate: -C(=O)-N=C=O (or =S).
        if (nZ == 7 && order == 1 && isIsocyanateNitrogen(nei, i, g)) return true;
    }
    return false;
}

bool isPeroxyCarboxylicAcid(int i, const Graph &g) {
    const GraphNode &node = g.nodes[i];
    for (size_t j = 0; j < node.neighbors.size(); ++j) {
        int nei = node.neighbors[j];
        if (g.nodes[nei].atomicNumber == 8 && node.bondOrders[j] == 1) {
            const GraphNode &oNode = g.nodes[nei];
            if (oNode.neighbors.size() == 2) {
                for (size_t k = 0; k < oNode.neighbors.size(); ++k) {
                    int oNei = oNode.neighbors[k];
                    if (oNei != i && g.nodes[oNei].atomicNumber == 8 && oNode.bondOrders[k] == 1) {
                        const GraphNode &peroxyONode = g.nodes[oNei];
                        if (peroxyONode.totalH >= 1 || peroxyONode.neighbors.size() == 1) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
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

// Alphabetization key for substituent-citation sorting and numbering-tiebreak scoring.
// Strips wrapping brackets a substituent name may carry (from compound-substituent or
// stereo-prefix wrapping), then a leading stereo-descriptor parenthetical like "(1R)-", then
// a leading locant-digit(s)-hyphen sequence like "1-", down to the true alphabetic root --
// e.g. "[(1R)-1-chloroethyl]" -> "chloroethyl". Plain names like "bromo" pass through
// unchanged.
QString alphabetizationKey(const QString &pName) {
    QString s = pName;
    if ((s.startsWith("(") && s.endsWith(")")) || (s.startsWith("[") && s.endsWith("]"))) {
        s = s.mid(1, s.length() - 2);
    } else if (s.startsWith("(") || s.startsWith("[")) {
        s = s.mid(1);
    }
    if (s.startsWith("(")) {
        int closeParen = s.indexOf(')');
        if (closeParen != -1 && closeParen + 1 < s.length() && s[closeParen + 1] == '-') {
            s = s.mid(closeParen + 2);
        }
    }
    int i = 0;
    while (i < s.length() && (s[i].isDigit() || s[i] == ',')) i++;
    if (i > 0 && i < s.length() && s[i] == '-') {
        s = s.mid(i + 1);
    }
    return s;
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

// P-22.2.2.1.3 citation-order seniority rank for Hantzsch-Widman naming.
// Lower rank = more senior (gets locant 1 preference).
// Order: O > S > Se > Te > N > P > As > Sb > Bi > Si > Ge > Sn > Pb > B
// NOTE: This is NOT the same as getRankHetero used for fused-ring base-component
// seniority (P-25.3.2) -- that ranks N highest and is a completely different rule.
static int hwSeniorityRank(int z) {
    switch (z) {
        case  8: return  0; // O
        case 16: return  1; // S
        case 34: return  2; // Se
        case 52: return  3; // Te
        case  7: return  4; // N
        case 15: return  5; // P
        case 33: return  6; // As
        case 51: return  7; // Sb
        case 83: return  8; // Bi
        case 14: return  9; // Si
        case 32: return 10; // Ge
        case 50: return 11; // Sn
        case 82: return 12; // Pb
        case  5: return 13; // B
        default: return 99; // unsupported
    }
}

// Return the 'a'-replacement prefix for a supported heteroatom atomic number.
static QString hwAPrefix(int z) {
    switch (z) {
        case  8: return "oxa";
        case 16: return "thia";
        case 34: return "selena";
        case 52: return "tellura";
        case  7: return "aza";
        case 15: return "phospha";
        case 33: return "arsa";
        case 51: return "stiba";
        case 83: return "bisma";
        case 14: return "sila";
        case 32: return "germa";
        case 50: return "stanna";
        case 82: return "plumba";
        case  5: return "bora";
        default: return "";
    }
}

// P-22.2.2.1.6: For 6-membered rings, find least-senior heteroatom present.
// Group A (O,S,Se,Te,Bi) and Group B (N,Si,Ge,Sn,Pb) -> mancude stem '-ine'.
// Group C (P,As,Sb,B) -> mancude stem '-inine'.

// Build a skeletal replacement prefix chunk (e.g. "2,4-dioxa-6-aza") for a set of heteroatoms.
// Reuses hwSeniorityRank/hwAPrefix and citation order from LARGE_HETEROCYCLE.
static QString buildSkeletalReplacementPrefix(const std::map<int, std::vector<int>> &locantsByZ, bool omitSingleLocant) {
    int totalHeteroCount = 0;
    for (const auto &kv : locantsByZ) totalHeteroCount += static_cast<int>(kv.second.size());
    if (totalHeteroCount == 0) return "";
    
    if (omitSingleLocant && totalHeteroCount == 1) {
        return hwAPrefix(locantsByZ.begin()->first);
    }
    
    static const int citationOrder[] = {8, 16, 34, 52, 7, 15, 33, 51, 83, 14, 32, 50, 82, 5};
    static const int citationOrderLen = 14;
    
    QStringList chunks;
    for (int k = 0; k < citationOrderLen; ++k) {
        int z = citationOrder[k];
        auto it = locantsByZ.find(z);
        if (it == locantsByZ.end()) continue;
        
        std::vector<int> locs = it->second;
        std::sort(locs.begin(), locs.end());
        QStringList locStrs;
        for (int l : locs) locStrs.append(QString::number(l));
        
        QString aPrefix = hwAPrefix(z);
        QString prefixWord;
        if (locs.size() == 1) {
            prefixWord = aPrefix;
        } else {
            QString mp = multiPrefix(static_cast<int>(locs.size()));
            if (mp.endsWith('a') && isVowel(aPrefix[0])) mp.chop(1);
            prefixWord = mp + aPrefix;
        }
        chunks.append(locStrs.join(",") + "-" + prefixWord);
    }
    return chunks.join("-");
}

static QString hwSixMemberStem(const std::vector<int> &heteroAtomicNumbers) {
    // Find the element with the highest (least-senior) rank
    int leastSeniorRank = -1;
    int leastSeniorZ = -1;
    for (int z : heteroAtomicNumbers) {
        int r = hwSeniorityRank(z);
        if (r > leastSeniorRank) {
            leastSeniorRank = r;
            leastSeniorZ = z;
        }
    }
    // Group C elements: P(15), As(33), Sb(51), B(5)
    if (leastSeniorZ == 15 || leastSeniorZ == 33 || leastSeniorZ == 51 || leastSeniorZ == 5) {
        return "inine";
    }
    // Group A: O,S,Se,Te,Bi; Group B: N,Si,Ge,Sn,Pb -- both use '-ine'
    return "ine";
}

// P-22.2.2.1.5.1 / Table 2.5: General Hantzsch-Widman stem selection for ring sizes 3-10.
// Returns the unsaturated (mancude) stem for the given ring size and heteroatom composition.
// Verified against Blue Book Table 2.5 and specific PIN examples:
//   size 3: "irine" if ALL heteroatoms are nitrogen (z==7), else "irene"
//   size 4: "ete" (always)
//   size 5: "ole" (always)
//   size 6: delegates to hwSixMemberStem (ine/inine by least-senior element)
//   size 7: "epine" (always)
//   size 8: "ocine" (always)
//   size 9: "onine" (always)
//   size 10: "ecine" (always)
// For any other size, returns empty string (caller must guard against this).
static QString hwGeneralRingStem(int ringSize, const std::vector<int> &heteroAtomicNumbers, bool saturated = false) {
    switch (ringSize) {
        case 3: {
            // P-22.2.2.1.5.1: stem 'irine' is used in place of 'irene' for rings ONLY containing
            // nitrogen heteroatoms. Mixed composition (e.g. O+N) still uses '-irene'.
            if (heteroAtomicNumbers.empty()) return "irene"; // safety: no heteroatoms -> default
            for (int z : heteroAtomicNumbers) {
                if (z != 7) return "irene"; // 7 = nitrogen
            }
            return "irine";
        }
        case 4: return "ete";   // Confirmed by PINs: oxete, azete
        case 5: return "ole";   // 5-ring always "ole"
        case 6: return hwSixMemberStem(heteroAtomicNumbers); // delegate to existing logic
        case 7: return saturated ? "epane" : "epine"; // Confirmed by PINs: azepine, azepane
        case 8: return saturated ? "ocane" : "ocine"; // Confirmed by PIN: diazocine, azocane
        case 9: return saturated ? "onane" : "onine"; // Confirmed by PIN: dioxonine, azonane
        case 10: return saturated ? "ecane" : "ecine"; // Confirmed by PIN: diazecine, azecane
        default: return ""; // should never happen for sizes accepted by tryGeneralHeterocycle
    }
}

// P-14.7.1: Helper to find indicated hydrogen position in a monocyclic general heterocycle.
// Returns the 1-based locant of the saturated position (where both ring bonds are single
// and the atom carries at least one H), or -1 if no such position, or -2 if 2+ positions.
// Only checks ring-internal bonds (bonds to the two adjacent ring atoms).
// Note: After indigoAromatize(), bonds in aromatic systems have order 4 (not alternating 1/2).
// However, for odd-membered rings that cannot be fully aromatic (e.g., 7-membered with 4N),
// the bonds are still explicit single/double (not all order 4).
static int findIndicatedHydrogenLocant(const Graph &g, const std::vector<int> &ringChain) {
    int ringSize = static_cast<int>(ringChain.size());
    if (ringSize < 3) return -1; // not a valid ring

    int indicatedHLocant = -1;
    for (int i = 0; i < ringSize; ++i) {
        int nodeIdx = ringChain[i];
        const GraphNode &node = g.nodes[nodeIdx];

        // Find the two ring neighbors in the chain
        int prevIdx = ringChain[(i - 1 + ringSize) % ringSize];
        int nextIdx = ringChain[(i + 1) % ringSize];

        // Find bond orders to prev and next in the ring
        int orderToPrev = -1;
        int orderToNext = -1;

        for (size_t j = 0; j < node.neighbors.size(); ++j) {
            if (node.neighbors[j] == prevIdx) {
                orderToPrev = node.bondOrders[j];
            } else if (node.neighbors[j] == nextIdx) {
                orderToNext = node.bondOrders[j];
            }
        }

        // An atom has an indicated hydrogen position if:
        // - Both ring bonds are NOT double or aromatic (i.e., both are single, order 1)
        //   This means the atom is sp3-hybridized in the ring (saturated).
        // - The atom carries at least one hydrogen (totalH >= 1)
        // Note: In a Kekulized structure (after aromatization), aromatic bonds are order 4,
        // and explicit single/double bonds are orders 1/2. An atom with two single bonds
        // to its ring neighbors is the saturated position.
        
        if (orderToPrev == 1 && orderToNext == 1 && node.totalH >= 1) {
            if (indicatedHLocant != -1) {
                // Already found one, this is a second -> multiple indicated H positions
                return -2;
            }
            indicatedHLocant = i + 1; // 1-based locant
        }
    }

    // If no indicated hydrogen found via bond order check, but ring is odd-membered,
    // check if all bonds are aromatic (order 4). This can happen when Indigo treats
    // an odd-membered ring as aromatic even though it cannot be by Huckel's rule.
    // In such cases, structurally there MUST be one saturated position.
    if (indicatedHLocant == -1 && (ringSize == 3 || ringSize == 7 || ringSize == 9)) {
        bool allBondsAromatic = true;
        int firstHAtom = -1;
        
        for (int i = 0; i < ringSize; ++i) {
            int nodeIdx = ringChain[i];
            const GraphNode &node = g.nodes[nodeIdx];

            int prevIdx = ringChain[(i - 1 + ringSize) % ringSize];
            int nextIdx = ringChain[(i + 1) % ringSize];

            int orderToPrev = -1;
            int orderToNext = -1;

            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                if (node.neighbors[j] == prevIdx) {
                    orderToPrev = node.bondOrders[j];
                } else if (node.neighbors[j] == nextIdx) {
                    orderToNext = node.bondOrders[j];
                }
            }

            if (orderToPrev != 4 || orderToNext != 4) {
                allBondsAromatic = false;
            }

            // Track first atom with totalH >= 1
            if (firstHAtom == -1 && node.totalH >= 1) {
                firstHAtom = i + 1; // 1-based locant
            }
        }

        // If all bonds are aromatic and this is an odd-membered ring,
        // there must be exactly one saturated position. Use the first atom
        // with totalH >= 1 as the indicated hydrogen position.
        // Note: For odd-membered rings, structural constraints guarantee
        // exactly one such position exists in the mancude form.
        if (allBondsAromatic && firstHAtom != -1) {
            indicatedHLocant = firstHAtom;
        }
    }

    return indicatedHLocant; // -1 if none, positive if exactly one
}

// P-22.2.2.1: tryGeneralHeterocycle -- gates entry into GENERAL_HETEROCYCLE path.
// Validates all ring heteroatoms are in the P-22.2.2.1.3 supported set.
// Actual name is assembled downstream in the GENERAL_HETEROCYCLE branch at ~line 7251.
// Returns false (triggering rejection) for any unsupported element or ring size.
bool tryGeneralHeterocycle(const Graph &g, const std::vector<int> &ringHeteroNodes, int ringSize, QString &outNameFragment) {
    // P-22.2.2.1.1: Hantzsch-Widman applies to monocyclic rings of size 3-10.
    // P-22.2.4 covers rings of size 11+ (out of scope for this phase).
    if (ringSize < 3 || ringSize > 10) return false;
    for (int nIdx : ringHeteroNodes) {
        int z = g.nodes[nIdx].atomicNumber;
        if (hwSeniorityRank(z) == 99) return false; // unsupported element
    }
    // Return true with a non-empty placeholder; downstream code builds the real name.
    outNameFragment = "<GENERAL_HETEROCYCLE>";
    return true;
}

enum class RingType { BENZENE, FURAN, THIOPHENE, SELENOPHENE, TELLUROPHENE, PYRROLE, PYRIDINE, PHOSPHININE, CYCLOALKANE, CYCLOALKENE, IMIDAZOLE, PYRIMIDINE, PYRAZOLE, OXAZOLE, ISOXAZOLE, THIAZOLE, ISOTHIAZOLE, SELENAZOLE, ISOSELENAZOLE, PYRIDAZINE, PYRAZINE, PIPERIDINE, PYRROLIDINE, TETRAHYDROFURAN, TETRAHYDROTHIOPHENE, GENERAL_HETEROCYCLE, LARGE_HETEROCYCLE };

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

    // P-22.2.3/P-22.2.4: heteromonocycles of 11-20 ring members use skeletal
    // replacement ('a') nomenclature (cyclo+root+ane, or a mancude '-ene' chain)
    if (ringSize >= 11 && ringSize <= 20) {
        bool allSupportedElements = true;
        for (int nIdx : ringHeteroNodes) {
            if (hwSeniorityRank(g.nodes[nIdx].atomicNumber) == 99) { allSupportedElements = false; break; }
        }
        // A genuine simple monocycle has every ring atom with exactly 2
        // ring-internal neighbors; a fusion/bridge/spiro atom has 3+. This
        // permits exocyclic substituents (which don't count as ring-internal)
        // while still rejecting non-monocyclic topologies, which belong to
        // separate fused/bridged/spiro handling elsewhere in this file.
        bool isSimpleMonocycle = true;
        for (int nIdx : ringCycle) {
            int ringInternalDegree = 0;
            for (int nb : g.nodes[nIdx].neighbors) {
                if (std::find(ringCycle.begin(), ringCycle.end(), nb) != ringCycle.end()) ringInternalDegree++;
            }
            if (ringInternalDegree != 2) { isSimpleMonocycle = false; break; }
        }
        if (allSupportedElements && isSimpleMonocycle) {
            bool hasTriple = false;
            bool hasDouble = false;
            for (const auto &gb : g.bonds) {
                bool uIn = (std::find(ringCycle.begin(), ringCycle.end(), gb.u) != ringCycle.end());
                bool vIn = (std::find(ringCycle.begin(), ringCycle.end(), gb.v) != ringCycle.end());
                if (uIn && vIn) {
                    if (gb.order == 3) hasTriple = true;
                    if (gb.order == 2) hasDouble = true;
                }
            }

            if (hasTriple) {
                outErrorMsg = "Large heterocycles with triple bonds are not supported.";
                return false;
            }

            bool validMancude = false;
            if (!allSingleInRing && (hasDouble || heteroAromatic)) {
                std::vector<int> spareValence(ringSize);
                int zeroSpareCount = 0;
                for (int i = 0; i < ringSize; ++i) {
                    int z = g.nodes[ringCycle[i]].atomicNumber;
                    if (z == 8 || z == 16 || z == 34 || z == 52) {
                        spareValence[i] = 0;
                        zeroSpareCount++;
                    } else {
                        spareValence[i] = 1;
                    }
                }

                bool parityCheckOk = true;
                int maxDoubleBonds = 0;
                if (zeroSpareCount > 0) {
                    int startIdx = 0;
                    while (spareValence[startIdx] != 0) startIdx++;

                    int runLength = 0;
                    for (int i = 1; i <= ringSize; ++i) {
                        int idx = (startIdx + i) % ringSize;
                        if (spareValence[idx] == 1) {
                            runLength++;
                        } else {
                            if (runLength > 0) {
                                if (runLength % 2 != 0) {
                                    parityCheckOk = false;
                                    break;
                                }
                                maxDoubleBonds += runLength / 2;
                            }
                            runLength = 0;
                        }
                    }
                } else {
                    if (ringSize % 2 != 0) {
                        parityCheckOk = false;
                    } else {
                        maxDoubleBonds = ringSize / 2;
                    }
                }

                if (parityCheckOk) {
                    if (heteroAromatic) {
                        validMancude = true;
                    } else {
                        int actualDoubleBonds = 0;
                        bool validBonding = true;
                        for (int i = 0; i < ringSize; ++i) {
                            int u = ringCycle[i];
                            int v = ringCycle[(i + 1) % ringSize];
                            int bondOrder = 1;
                            for (const auto &gb : g.bonds) {
                                if ((gb.u == u && gb.v == v) || (gb.u == v && gb.v == u)) {
                                    bondOrder = gb.order;
                                    break;
                                }
                            }
                            if (bondOrder == 2) {
                                actualDoubleBonds++;
                                if (spareValence[i] == 0 || spareValence[(i + 1) % ringSize] == 0) {
                                    validBonding = false;
                                }
                            }
                        }

                        for (int i = 0; i < ringSize; ++i) {
                            int u = ringCycle[(i - 1 + ringSize) % ringSize];
                            int v = ringCycle[i];
                            int w = ringCycle[(i + 1) % ringSize];
                            int bo1 = 1, bo2 = 1;
                            for (const auto &gb : g.bonds) {
                                if ((gb.u == u && gb.v == v) || (gb.u == v && gb.v == u)) bo1 = gb.order;
                                if ((gb.u == v && gb.v == w) || (gb.u == w && gb.v == v)) bo2 = gb.order;
                            }
                            if (bo1 == 2 && bo2 == 2) validBonding = false;
                        }

                        if (validBonding && actualDoubleBonds == maxDoubleBonds) {
                            validMancude = true;
                        } else if (validBonding && actualDoubleBonds > 0) {
                            outErrorMsg = "Partially saturated large heterocycles are not supported.";
                            return false;
                        } else if (!validBonding) {
                            outErrorMsg = "Invalid double bond arrangement for mancude form.";
                            return false;
                        }
                    }
                } else {
                    outErrorMsg = "Ring composition cannot support a valid mancude form due to parity failure.";
                    return false;
                }
            }

            if (allSingleInRing || validMancude) {
                outType = RingType::LARGE_HETEROCYCLE;
                outNameRoot = "cyclo" + chainRoot(ringSize); // Defer the "ane" / "ene" ending
                return true;
            }
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
            } else if (ringSize >= 7 && ringSize <= 10) {
                if (tryGeneralHeterocycle(g, ringHeteroNodes, ringSize, outNameRoot)) {
                    outType = RingType::GENERAL_HETEROCYCLE; return true;
                }
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
                               const std::set<int> &forbiddenNodes = {},
                               const std::map<int, QChar> &stereoByGraphId = {},
                               std::set<int> *handledBranchStereoIds = nullptr);

QString nameRingAsSubstituent(const Graph &g, const std::set<int> &ringNodes, int attachmentNode, int parentLinkNode,
                                     const std::vector<std::set<int>> &allIndependentRings = {},
                                     const std::set<int> &forbiddenNodes = {},
                                     const std::map<int, QChar> &stereoByGraphId = {},
                                     std::set<int> *handledBranchStereoIds = nullptr) {
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
    } else if (rType == RingType::GENERAL_HETEROCYCLE) {
        // P-22.2.2.1.3: seed from positions of the most-senior heteroatom
        int minRank = 99;
        for (int nodeIdx : ringHeteroNodes) {
            int r = hwSeniorityRank(g.nodes[nodeIdx].atomicNumber);
            if (r < minRank) minRank = r;
        }
        for (int st = 0; st < ringSize; ++st) {
            int z = g.nodes[ringCycle[st]].atomicNumber;
            if (hwSeniorityRank(z) == minRank) {
                std::vector<int> fwd2(ringSize), bwd2(ringSize);
                for (int i = 0; i < ringSize; ++i) {
                    fwd2[i] = ringCycle[(st + i) % ringSize];
                    bwd2[i] = ringCycle[(st - i + ringSize) % ringSize];
                }
                ringCandidates.push_back(fwd2);
                ringCandidates.push_back(bwd2);
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

    struct CandidateScore {
        int attachLocant;
        std::vector<int> sortedSubLocants;
        std::vector<std::pair<QString, int>> namedSubstituents;
        QString hwNameRoot; // for GENERAL_HETEROCYCLE: pre-built locant+prefix+stem string
        std::vector<int> ringChain; // winning candidate's own ring-atom order, locant i+1 == cand[i]
    };

    std::vector<CandidateScore> validScores;

    for (const auto &cand : ringCandidates) {
        std::map<int, int> locantMap;
        for (int i = 0; i < ringSize; ++i) {
            locantMap[cand[i]] = i + 1;
        }

        int attachLocant = locantMap[attachmentNode];

        bool candValid = true;

        CandidateScore cs;
        cs.attachLocant = attachLocant;

        // For GENERAL_HETEROCYCLE, build the HW name from this candidate's ring chain
        if (rType == RingType::GENERAL_HETEROCYCLE) {
            static const int citOrd[] = {8, 16, 34, 52, 7, 15, 33, 51, 83, 14, 32, 50, 82, 5};
            static const int citOrdLen = 14;
            std::map<int, std::vector<int>> locsByZ;
            std::vector<int> allZ;
            for (int ii = 0; ii < ringSize; ++ii) {
                int zz = g.nodes[cand[ii]].atomicNumber;
                if (zz != 6) { locsByZ[zz].push_back(ii + 1); allZ.push_back(zz); }
            }
            std::vector<int> allLs;
            for (int k = 0; k < citOrdLen; ++k) {
                auto it2 = locsByZ.find(citOrd[k]);
                if (it2 != locsByZ.end()) for (int l : it2->second) allLs.push_back(l);
            }
            QStringList lStrs;
            for (int l : allLs) lStrs.append(QString::number(l));
            QString locPfx = lStrs.join(",") + "-";
            QString ePfxs;
            for (int k = 0; k < citOrdLen; ++k) {
                int zz = citOrd[k];
                auto it2 = locsByZ.find(zz);
                if (it2 == locsByZ.end()) continue;
                int cnt = static_cast<int>(it2->second.size());
                QString ap = hwAPrefix(zz);
                // P-22.2.2.1.2: final 'a' of multiplying prefix elides before a vowel.
                // e.g. "tetra"+"aza" -> "tetraza" (not "tetraaza"); "tri"+"oxa" -> "trioxa".
                QString mp = multiPrefix(cnt);
                if (mp.endsWith('a') && isVowel(ap[0])) mp.chop(1);
                QString chunk = mp + ap;
                if (!ePfxs.isEmpty() && ePfxs.endsWith('a') && chunk.startsWith('a')) ePfxs.chop(1);
                ePfxs += chunk;
            }
            QString st = hwGeneralRingStem(ringSize, allZ);
            if (st.isEmpty()) { /* should never happen for sizes accepted by tryGeneralHeterocycle */ }
            if (ePfxs.endsWith('a') && isVowel(st[0])) ePfxs.chop(1);

            // P-14.7.1: Check for indicated hydrogen (saturated ring position)
            int indicatedH = findIndicatedHydrogenLocant(g, cand);
            if (indicatedH == -2) {
                // Multiple indicated hydrogen positions - reject cleanly
                candValid = false;
            } else if (indicatedH > 0) {
                // Exactly one indicated hydrogen position - prepend "<locant>H-"
                cs.hwNameRoot = QString("%1H-").arg(indicatedH) + locPfx + ePfxs + st;
            } else {
                // No indicated hydrogen needed
                cs.hwNameRoot = locPfx + ePfxs + st;
            }
        }

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
            cs.ringChain = cand;
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
        pg.baseName = alphabetizationKey(pName);
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

    // IUPAC Blue Book Coverage.md item 6: a stereocenter on the ring's OWN atom
    // (as opposed to on a branch attached to a ring parent, P-93.5/93.6, already
    // handled elsewhere via formatBranchStereoPrefix) -- that function's chain-walk
    // numbering has no relation to a ring's real numbering, so it can't be reused
    // here; this ring already computed its own correct winning locant scheme
    // (best.ringChain), so it builds its own stereo prefix directly from that.
    if (!stereoByGraphId.empty()) {
        std::vector<std::pair<int, QChar>> locantStereo;
        for (size_t i = 0; i < best.ringChain.size(); ++i) {
            int nodeIdx = best.ringChain[i];
            auto it = stereoByGraphId.find(nodeIdx);
            if (it != stereoByGraphId.end()) {
                locantStereo.push_back({static_cast<int>(i + 1), it->second});
                if (handledBranchStereoIds) handledBranchStereoIds->insert(nodeIdx);
            }
        }
        if (!locantStereo.empty()) {
            std::sort(locantStereo.begin(), locantStereo.end(),
                      [](const std::pair<int, QChar> &a, const std::pair<int, QChar> &b) { return a.first < b.first; });
            QStringList parts;
            for (const auto &p : locantStereo) parts.append(QString("%1%2").arg(p.first).arg(p.second));
            prefixPart = QString("(%1)-").arg(parts.join(",")) + prefixPart;
        }
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
        else if (rType == RingType::GENERAL_HETEROCYCLE) stem = best.hwNameRoot;
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

QString nameBranchGraph(const Graph &g, int rootIdx, int parentIdx, const std::vector<std::set<int>> &allIndependentRings, const std::set<int> &forbiddenNodes,
                         const std::map<int, QChar> &stereoByGraphId, std::set<int> *handledBranchStereoIds) {
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
                return nameRingAsSubstituent(g, rNodes, rootIdx, parentIdx, allIndependentRings, combinedForbidden, stereoByGraphId, handledBranchStereoIds);
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

    if (rZ == 15) {
        // Phosphorus - handle phosphonic acid case, return empty to let functional group logic handle it
        return "";
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
            if (g.nodes[nei].atomicNumber == 6 && !forbiddenNodes.count(nei)) {
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
                        if (g.nodes[nxt].atomicNumber == 6 && dist.find(nxt) == dist.end() && nxt != prev && !forbiddenNodes.count(nxt)) {
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

// Phase 53 helper: classify an exocyclic branch off a saturated all-carbon
// bicyclic/spiro ring atom as a "simple substituent" permitted in this phase --
// specifically a plain saturated acyclic alkyl group (named via nameBranchGraph)
// or a bare terminal halogen (F/Cl/Br/I). Returns "" for anything outside that
// scope (heteroatom other than a terminal halogen, unsaturation, a ring closed
// back into the branch, or a principal-characteristic-group-bearing atom) so the
// caller falls through to the generic rejection below rather than guessing.
QString simpleRingSubstituentName(const Graph &g, int ringAtom, int exoNei,
                                  const std::set<int> &ringUnionNodes) {
    if (exoNei < 0 || exoNei >= static_cast<int>(g.nodes.size())) return "";
    if (ringUnionNodes.count(exoNei)) return "";

    int rZ = g.nodes[exoNei].atomicNumber;
    int order = -1;
    for (size_t k = 0; k < g.nodes[ringAtom].neighbors.size(); ++k) {
        if (g.nodes[ringAtom].neighbors[k] == exoNei) { order = g.nodes[ringAtom].bondOrders[k]; break; }
    }
    if (order != 1) return ""; // unsaturation on the attachment bond

    // Bare terminal halogen directly bonded to the ring atom.
    if (rZ == 9 || rZ == 17 || rZ == 35 || rZ == 53) {
        if (g.nodes[exoNei].neighbors.size() != 1) return "";
        return halogenPrefix(rZ);
    }
    if (rZ != 6) return ""; // heteroatom other than a terminal halogen

    // Validate the exo carbon subtree: all-carbon, all single bonds, acyclic,
    // and the only connection into the ring union is exoNei's single bond to
    // ringAtom (no deeper branch re-attaches to the ring system).
    int ringConns = 0;
    for (int n : g.nodes[exoNei].neighbors) if (ringUnionNodes.count(n)) ringConns++;
    if (ringConns != 1) return "";

    std::set<int> visited;
    std::vector<std::pair<int,int>> stk; // (node, prev)
    stk.push_back({exoNei, ringAtom});
    visited.insert(exoNei);

    auto bondOrderBetween = [&](int a, int b) {
        for (size_t k = 0; k < g.nodes[a].neighbors.size(); ++k)
            if (g.nodes[a].neighbors[k] == b) return g.nodes[a].bondOrders[k];
        return -1;
    };

    while (!stk.empty()) {
        int u = stk.back().first;
        int prev = stk.back().second;
        stk.pop_back();
        for (int n : g.nodes[u].neighbors) {
            if (n == prev) continue;
            if (u == exoNei && n == ringAtom) continue;       // allowed single attachment
            if (ringUnionNodes.count(n)) return "";           // branch re-touches the ring
            int bo = bondOrderBetween(u, n);
            if (bo != 1) return "";                           // unsaturation / aromatic
            if (g.nodes[n].atomicNumber != 6) return "";      // heteroatom inside branch
            if (visited.count(n)) return "";                   // cycle within the branch
            visited.insert(n);
            stk.push_back({n, u});
        }
    }

    return nameBranchGraph(g, exoNei, ringAtom, {}, ringUnionNodes);
}

// Function to name one side (acyl chain) of an acid anhydride
// === Phase 54 helpers (P-44.1.1 chain-as-parent, generalized) ===

// principalGroupSuffix: builds the principal-characteristic-group suffix for an
// acyclic chain parent, for any GroupType the pure-acyclic path already names.
// Extracted verbatim from the suffix-assembly block of the acyclic path (the
// `if (winningType != GroupType::NONE)` block) so the Phase 54 chain-as-parent
// case reuses the EXACT same per-class strings instead of a divergent copy.
// `acylHalideHalogenZ` is only consulted for ACYL_HALIDE (which the chain-as-
// parent path deliberately leaves rejecting; it is passed through purely so
// the acyclic path's own acyl-halide call site can share this helper).
// Returns "" for PHOSPHINE / BORONIC_ACID (named by separate early-return logic
// in the acyclic path) and leaves ESTER's alkyl prefix to that caller.
static QString principalGroupSuffix(GroupType winningType, int k, int pCount,
                                    const std::vector<int> &principalLocants,
                                    int acylHalideHalogenZ)
{
    QString sfx;
    if (winningType == GroupType::SULFONIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("sulfonic acid")
                                        : QString("-%1-sulfonic acid").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2sulfonic acid").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::SULFONYL_HALIDE) {
        QString hName = halogenSuffixWord(acylHalideHalogenZ);
        if (pCount == 1) sfx = (k <= 2) ? QString("sulfonyl %1").arg(hName)
                                        : QString("-%1-sulfonyl %2").arg(principalLocants[0]).arg(hName);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2sulfonyl %3").arg(lStrs.join(","), multiPrefix(pCount), hName);
        }
    } else if (winningType == GroupType::SULFINIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("sulfinic acid")
                                        : QString("-%1-sulfinic acid").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2sulfinic acid").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::PHOSPHONIC_DIHALIDE) {
        QString hName = "di" + halogenSuffixWord(acylHalideHalogenZ);
        if (pCount == 1) sfx = (k <= 2) ? QString("phosphonic %1").arg(hName)
                                        : QString("-%1-phosphonic %2").arg(principalLocants[0]).arg(hName);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2phosphonic %3").arg(lStrs.join(","), multiPrefix(pCount), hName);
        }
    } else if (winningType == GroupType::PHOSPHONIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("phosphonic acid")
                                        : QString("-%1-phosphonic acid").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2phosphonic acid").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::ARSONIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("arsonic acid")
                                        : QString("-%1-arsonic acid").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2arsonic acid").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::ACID) {
        sfx = (pCount == 2) ? QStringLiteral("dioic acid") : QStringLiteral("oic acid");
    } else if (winningType == GroupType::ESTER) {
        sfx = QStringLiteral("oate");
    } else if (winningType == GroupType::ACYL_HALIDE) {
        QString hName;
        if (acylHalideHalogenZ == 9) hName = "fluoride";
        else if (acylHalideHalogenZ == 17) hName = "chloride";
        else if (acylHalideHalogenZ == 35) hName = "bromide";
        else if (acylHalideHalogenZ == 53) hName = "iodide";
        sfx = (pCount == 2) ? ("dioyl " + hName) : ("oyl " + hName);
    } else if (winningType == GroupType::AMIDE) {
        sfx = (pCount == 2) ? QStringLiteral("diamide") : QStringLiteral("amide");
    } else if (winningType == GroupType::HYDRAZIDE) {
        sfx = (pCount == 2) ? QStringLiteral("dihydrazide") : QStringLiteral("hydrazide");
    } else if (winningType == GroupType::NITRILE) {
        sfx = (pCount == 2) ? QStringLiteral("dinitrile") : QStringLiteral("nitrile");
    } else if (winningType == GroupType::ALDEHYDE) {
        sfx = (pCount == 2) ? QStringLiteral("dial") : QStringLiteral("al");
    } else if (winningType == GroupType::THIAL) {
        sfx = (pCount == 2) ? QStringLiteral("dithial") : QStringLiteral("thial");
    } else if (winningType == GroupType::KETONE) {
        if (pCount == 1) sfx = QString("-%1-one").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2one").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::THIONE) {
        if (pCount == 1) sfx = QString("-%1-thione").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2thione").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::ALCOHOL) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("ol") : QString("-%1-ol").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2ol").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::THIOL) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("thiol") : QString("-%1-thiol").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2thiol").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::SELENOL) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("selenol") : QString("-%1-selenol").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2selenol").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::TELLUROL) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("tellurol") : QString("-%1-tellurol").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2tellurol").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::AMINE) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("amine") : QString("-%1-amine").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2amine").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::IMINE) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("imine") : QString("-%1-imine").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2imine").arg(lStrs.join(","), multiPrefix(pCount));
        }
    } else if (winningType == GroupType::HYDROPEROXIDE) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("peroxol") : QString("-%1-peroxol").arg(principalLocants[0]);
        else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2peroxol").arg(lStrs.join(","), multiPrefix(pCount));
        }
    }
    return sfx;
}

// isPrincipalGroupHeteroNeighbor: true if `nei` (a non-chain neighbour of a
// principal carbon) is part of the principal characteristic group itself and so
// must NOT be cited as a substituent prefix -- e.g. the =O/-OH of an acid, the
// =O of a ketone/aldehyde, the -NH2 of an amine, the triple-bonded N of a
// nitrile, the -SH of a thiol. Generalizes nameAcidChainFrom's old i==0 acid-O
// skip to every principal carbon of every winning class. Only the in-scope
// chain-as-parent classes are actually exercised; the rest are listed for
// symmetry and default to "do not skip" (their group atoms are handled by the
// rich classification maps in the acyclic path, which this helper does not
// duplicate).
static bool isPrincipalGroupHeteroNeighbor(GroupType winningType, const Graph &g,
                                           int /*principalC*/, int nei, int order)
{
    int nz = g.nodes[nei].atomicNumber;
    if (winningType == GroupType::ACID) {
        if (nz == 8 && order == 2) return true;                         // carbonyl =O
        if (nz == 8 && order == 1 &&
            (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) return true; // -OH
    } else if (winningType == GroupType::AMIDE) {
        if (nz == 8 && order == 2) return true;                         // =O
        if (nz == 7 && order == 1) return true;                         // -N<
    } else if (winningType == GroupType::HYDRAZIDE) {
        if (nz == 8 && order == 2) return true;                         // =O
        if (nz == 7 && order == 1) return true;                         // -N<
    } else if (winningType == GroupType::NITRILE) {
        if (nz == 7) return true;                                      // =N (triple)
    } else if (winningType == GroupType::ALDEHYDE || winningType == GroupType::KETONE) {
        if (nz == 8 && order == 2) return true;                         // =O
    } else if (winningType == GroupType::THIAL || winningType == GroupType::THIONE) {
        if (nz == 16 && order == 2) return true;                        // =S
    } else if (winningType == GroupType::ALCOHOL) {
        if (nz == 8 && order == 1) return true;                         // -OH
    } else if (winningType == GroupType::THIOL) {
        if (nz == 16 && order == 1) return true;                        // -SH
    } else if (winningType == GroupType::SELENOL) {
        if (nz == 34 && order == 1) return true;                        // -SeH
    } else if (winningType == GroupType::TELLUROL) {
        if (nz == 52 && order == 1) return true;                        // -TeH
    } else if (winningType == GroupType::HYDROPEROXIDE) {
        if (nz == 8 && order == 1) return true;                         // near -O- of -O-O-H
    } else if (winningType == GroupType::AMINE) {
        if (nz == 7 && order == 1) return true;                         // -N<
    } else if (winningType == GroupType::IMINE) {
        if (nz == 7 && order == 2) return true;                         // =NH
    } else if (winningType == GroupType::SULFONIC_ACID) {
        if (nz == 16 && order == 1) return true;                        // -SO3H
    } else if (winningType == GroupType::SULFINIC_ACID) {
        if (nz == 16 && order == 1) return true;                        // -SO2H
    } else if (winningType == GroupType::PHOSPHONIC_DIHALIDE) {
        if (nz == 15 && order == 1) return true;                        // -P(=O)X2
    } else if (winningType == GroupType::PHOSPHONIC_ACID) {
        if (nz == 15 && order == 1) return true;                        // -P(=O)(OH)2
    } else if (winningType == GroupType::ARSONIC_ACID) {
        if (nz == 33 && order == 1) return true;                        // -As(=O)(OH)2
    } else if (winningType == GroupType::ESTER) {
        if (nz == 8 && order == 2) return true;                         // =O
        if (nz == 8 && order == 1) return true;                         // ester -O-
    } else if (winningType == GroupType::ACYL_HALIDE) {
        if (nz == 8 && order == 2) return true;                         // =O
        if ((nz == 9 || nz == 17 || nz == 35 || nz == 53) && order == 1) return true; // halogen
    }
    return false;
}

// nameAcyclicChainParentWithSubstituents (Phase 54): the generalized acyclic
// chain-parent namer backing the P-44.1.1 chain-wins case for every principal
// GroupType the pure-acyclic path already names (ACID, AMIDE, NITRILE,
// ALDEHYDE, KETONE, ALCOHOL, THIOL, AMINE). It mirrors nameAcidChainFrom's
// chain walk + lowest-locant selection + prefix/infix formatting, with three
// generalizations: (a) the DFS is seeded from every winning-class principal
// carbon (a ketone/alcohol may sit mid-chain; a diol/diamine has two); (b)
// principalLocants is a real computed list, not hardcoded {1}; (c) both
// directions of every maximal candidate chain are considered, the lowest
// principal-locant set winning. The suffix always comes from the shared
// principalGroupSuffix helper so the per-class suffix table stays single-copy.
// `extraSubstituents` injects an attached ring as a substituent prefix exactly
// as in nameAcidChainFrom; `excludeNodes` keeps the walk acyclic (e.g. ring
// atoms). Returns the full name on success, or "" on failure.
QString nameAcyclicChainParentWithSubstituents(
    const Graph &g,
    const std::set<int> &seedCarbons,
    const std::set<int> &principalCarbons,
    const std::set<int> &excludeNodes,
    const std::vector<std::pair<int, QString>> &extraSubstituents,
    GroupType winningType,
    int acylHalideHalogenZ = -1)
{
    if (seedCarbons.empty()) return "";
    std::vector<std::vector<int>> candidatePaths;
    for (int startCarbon : seedCarbons) {
        if (startCarbon < 0 || startCarbon >= static_cast<int>(g.nodes.size())) return "";
        if (g.nodes[startCarbon].atomicNumber != 6) return "";

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
    }

    if (candidatePaths.empty()) return "";

    // Phase 54: combine complementary single-seed paths for mid-chain
    // principal groups (ketone, alcohol, amine) where the principal
    // carbon sits between two chain extensions.  Each DFS path goes
    // out-and-back from the seed; we merge pairs that diverge at step 1.
    // Group by seed carbon
    std::map<int, std::vector<std::vector<int>>> pathsBySeed;
    for (const auto &p : candidatePaths) pathsBySeed[p[0]].push_back(p);
    for (auto &kv : pathsBySeed) {
        auto &ps = kv.second;
        // Sort descending by length
        std::sort(ps.begin(), ps.end(),
            [](const std::vector<int> &a, const std::vector<int> &b) {
                return a.size() > b.size();
            });
        // Try combining the longest two that diverge at step 1
        for (size_t i = 0; i < ps.size(); ++i) {
            for (size_t j = i + 1; j < ps.size(); ++j) {
                if (ps[i].size() >= 2 && ps[j].size() >= 2 && ps[i][1] != ps[j][1]) {
                    std::vector<int> combined;
                    // Prepend reversed j (without seed) so the combined chain
                    // reads from one terminus through the seed to the other.
                    combined.insert(combined.end(), ps[j].rbegin(), ps[j].rend() - 1);
                    combined.insert(combined.end(), ps[i].begin(), ps[i].end());
                    candidatePaths.push_back(combined);
                    goto nextSeed;            // best pair found; move on
                }
            }
        }
        nextSeed:;
    }

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
            maxLength = len; maxUnsat = uCount;
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

    for (const auto &dChain0 : topPaths) {
        // Phase 54 generalization (c): consider both directions of every
        // maximal candidate chain; the lowest principal-locant set wins.
        std::vector<std::vector<int>> dirs = {dChain0};
        if (dChain0.size() > 1) {
            std::vector<int> revP = dChain0;
            std::reverse(revP.begin(), revP.end());
            dirs.push_back(revP);
        }

        for (const auto &dChain : dirs) {
            PathSignature sig;
            sig.parentChain = dChain;
            std::set<int> chainSet(dChain.begin(), dChain.end());
            std::map<int,int> nodeLocant;
            for (size_t nl = 0; nl < dChain.size(); ++nl) nodeLocant[dChain[nl]] = static_cast<int>(nl + 1);

            for (size_t i = 0; i < dChain.size(); ++i) {
                int c = dChain[i];
                int locant = static_cast<int>(i + 1);

                // Phase 54 generalization (b): real principal-locant list.
                if (principalCarbons.count(c)) {
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

                for (size_t j = 0; j < g.nodes[c].neighbors.size(); ++j) {
                    int nei = g.nodes[c].neighbors[j];
                    int order = g.nodes[c].bondOrders[j];
                    if (chainSet.count(nei)) continue;
                    if (excludeNodes.count(nei)) continue;

                    int nz = g.nodes[nei].atomicNumber;
                    // Phase 54 generalization: skip the principal group's own
                    // heteroatoms at any principal carbon (the =O/-OH of an acid,
                    // the =O of a ketone, the -NH2 of an amine, ...) -- they are
                    // part of the suffix, not substituent prefixes. This subsumes
                    // nameAcidChainFrom's old i==0 acid-O skip via the shared
                    // isPrincipalGroupHeteroNeighbor helper.
                    if (principalCarbons.count(c) &&
                        isPrincipalGroupHeteroNeighbor(winningType, g, c, nei, order)) {
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
            // Inject the extra (ring) substituents the same way nameAcidChainFrom
            // does: bypass the paren-stripping above; the locant is the chain
            // position of the carbon the ring attaches to.
            for (const auto &es : extraSubstituents) {
                auto esIt = nodeLocant.find(es.first);
                if (esIt != nodeLocant.end()) {
                    sig.substituentLocants.push_back(esIt->second);
                    sig.namedSubstituents.push_back({es.second, esIt->second});
                }
            }
            std::sort(sig.principalLocants.begin(), sig.principalLocants.end());
            std::sort(sig.doubleBondLocants.begin(), sig.doubleBondLocants.end());
            std::sort(sig.tripleBondLocants.begin(), sig.tripleBondLocants.end());
            std::sort(sig.substituentLocants.begin(), sig.substituentLocants.end());
            directedSignatures.push_back(sig);
        }
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
                    [](const auto &x, const auto &y) { return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
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
        pg.baseName = alphabetizationKey(pName);
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
    int pCount = static_cast<int>(bestSig.principalLocants.size());
    QString sfx = principalGroupSuffix(winningType, k, pCount, bestSig.principalLocants, acylHalideHalogenZ);

    QChar checkC;
    for (QChar ch : sfx) {
        if (ch.isLetter()) { checkC = ch; break; }
    }
    if (!isVowel(checkC)) {
        stem += "e";
    }

    return prefixPart + stem + sfx;
}


// Phase 54: the principal GroupType classes for which the chain-as-parent
// (P-44.1.1 chain-wins) case is implemented via nameChainParentWithRingSubstituent.
// Shared by both generateName() call sites so the supported-class set cannot
// drift between the naphthalene block and the monocyclic block (this codebase
// once lost work to a delegate fixing only one of two duplicate sites).
static bool isChainParentWithRingSubstituentSupported(GroupType gt) {
    return gt == GroupType::ACID || gt == GroupType::AMIDE || gt == GroupType::HYDRAZIDE || gt == GroupType::NITRILE ||
           gt == GroupType::ALDEHYDE || gt == GroupType::KETONE ||
           gt == GroupType::ALCOHOL || gt == GroupType::THIOL || gt == GroupType::SELENOL || gt == GroupType::TELLUROL || gt == GroupType::HYDROPEROXIDE || gt == GroupType::AMINE || gt == GroupType::IMINE || gt == GroupType::THIAL || gt == GroupType::THIONE || gt == GroupType::SULFONIC_ACID ||
           gt == GroupType::SULFINIC_ACID || gt == GroupType::PHOSPHONIC_ACID || gt == GroupType::PHOSPHONIC_DIHALIDE || gt == GroupType::ARSONIC_ACID || gt == GroupType::ESTER || gt == GroupType::ACYL_HALIDE || gt == GroupType::SULFONYL_HALIDE;
}

// Phase 52 / Phase 54 (P-44.1.1 chain-wins case): when a chain-attached
// principal group of the most-senior class outnumbers the ring-attached
// instances of that class, the chain is the senior parent structure and the
// ring is cited as a substituent prefix on it. As of Phase 54 this is no longer
// acid-only: `winningType` selects the winning class, and naming is delegated
// to the generalized nameAcyclicChainParentWithSubstituents, which handles the
// principal-locant list and the per-class suffix for ACID, AMIDE, NITRILE,
// ALDEHYDE, KETONE, ALCOHOL, THIOL and AMINE. Other classes (ESTER, ACYL_HALIDE,
// SULFONIC_ACID, BORONIC_ACID, THIAL, THIONE, PHOSPHINE) are deliberately out
// of scope and rejected by the caller (see the supported-class set at both
// call sites in generateName). Returns the full name on success, or "" on
// failure (e.g. multiple ring<->chain attachments, or an unsupported ring).

// Thin wrapper around nameAcyclicChainParentWithSubstituents for the legacy
// acid-anhydride halves (seed = single carboxyl carbon, principal = {start}).
// No default on extraSubstituents -- callers outside the anonymous namespace
// (generateName) must pass {} explicitly.
QString nameAcidChainFrom(const Graph &g, int startCarbon, const std::set<int> &excludeNodes,
                          const std::vector<std::pair<int, QString>> &extraSubstituents) {
    if (startCarbon < 0 || startCarbon >= static_cast<int>(g.nodes.size())) return "";
    if (g.nodes[startCarbon].atomicNumber != 6) return "";
    std::set<int> seed = {startCarbon};
    std::set<int> principal = {startCarbon};
    return nameAcyclicChainParentWithSubstituents(g, seed, principal, excludeNodes,
                                                  extraSubstituents, GroupType::ACID);
}

QString nameChainParentWithRingSubstituent(int mol, const std::map<int, int> &indigoToGraphIdx,
                                          const Graph &g,
                                          const std::set<int> &ringNodeSet,
                                          const std::vector<std::set<int>> &allSSSRRings,
                                          const std::map<int, GroupType> &carbonGroup,
                                          GroupType winningType,
                                          const std::map<int, QChar> &stereoByGraphId = {},
                                          std::set<int> *handledBranchStereoIds = nullptr) {
    // 1. Collect the pure-chain principal carbons of the winning class (carbons
    //    of winningType that are neither part of the ring nor exocyclic to it).
    //    These are the chain-parent's principal characteristic groups; the
    //    ring-side principal groups stay on the ring (now a substituent).
    std::set<int> principalCarbons;
    for (const auto &pr : carbonGroup) {
        if (pr.second != winningType) continue;
        int cNode = pr.first;
        if (ringNodeSet.count(cNode)) continue;
        principalCarbons.insert(cNode);
    }
    if (principalCarbons.empty()) return "";
    // If only exocyclic chain carbons have the winning type, the ring carries
    // them (e.g. cyclohexanecarboxylic acid) — chain-as-parent does not apply.
    {
        bool hasDeep = false;
        for (int pc : principalCarbons) {
            bool exo = false;
            for (int nei : g.nodes[pc].neighbors) {
                if (ringNodeSet.count(nei)) { exo = true; break; }
            }
            if (!exo) { hasDeep = true; break; }
        }
        if (!hasDeep) return "";
    }

    // 2. Determine the acyclic chain component reachable from the principal
    //    carbons (all carbons reachable without crossing the ring).
    std::set<int> chainSet;
    {
        std::vector<int> stack(principalCarbons.begin(), principalCarbons.end());
        std::vector<bool> vis(g.nodes.size(), false);
        for (int sc : principalCarbons) vis[sc] = true;
        while (!stack.empty()) {
            int cur = stack.back(); stack.pop_back();
            chainSet.insert(cur);
            for (int nei : g.nodes[cur].neighbors) {
                if (g.nodes[nei].atomicNumber == 6 && !ringNodeSet.count(nei) && !vis[nei]) {
                    vis[nei] = true;
                    stack.push_back(nei);
                }
            }
        }
    }

    // 3. Find the single ring<->chain attachment: a ring atom with a carbon
    //    neighbour that belongs to chainSet. Ring-borne alkyl substituents that
    //    are NOT reachable from the principal carbons (e.g. a methyl on the ring)
    //    are left out of chainSet on purpose; nameRingAsSubstituent picks them
    //    up as ring substituents. Multiple ring->chain attachments are out of
    //    scope here.
    int ipsoRingNode = -1, chainAttachCarbon = -1, attachCount = 0;
    for (int rNode : ringNodeSet) {
        for (int nei : g.nodes[rNode].neighbors) {
            if (g.nodes[nei].atomicNumber != 6) continue;
            if (chainSet.count(nei)) {
                ++attachCount;
                if (attachCount == 1) {
                    ipsoRingNode = rNode;
                    chainAttachCarbon = nei;
                } else if (rNode != ipsoRingNode) {
                    return "";
                }
            }
        }
    }
    if (ipsoRingNode < 0 || chainAttachCarbon < 0) return "";

    // 4. Name the ring as a substituent prefix (handles ring-borne substituents
    //    such as a methyl via its own locant). This is class-agnostic.
    QString ringPrefix = nameRingAsSubstituent(g, ringNodeSet, ipsoRingNode, chainAttachCarbon, allSSSRRings, ringNodeSet, stereoByGraphId, handledBranchStereoIds);
    if (ringPrefix.isEmpty()) return "";

    // 5. Build the chain-parent name with the ring injected as an extra
    //    substituent attached at the chain carbon that bonds to the ring. Ring
    //    atoms are excluded so the chain walk stays acyclic. Phase 54: delegate
    //    to the generalized chain-namer for every supported winning class; the
    //    ACID case reduces exactly to the former nameAcidChainFrom call.
    std::vector<std::pair<int, QString>> extraSubstituents = {{chainAttachCarbon, ringPrefix}};

    int acylHalideHalogenZ = -1;
    QString esterAlkylPrefix = "";

    if (winningType == GroupType::ACYL_HALIDE) {
        for (int pc : principalCarbons) {
            for (size_t i = 0; i < g.nodes[pc].neighbors.size(); ++i) {
                int nei = g.nodes[pc].neighbors[i];
                int nz = g.nodes[nei].atomicNumber;
                if ((nz == 9 || nz == 17 || nz == 35 || nz == 53) && g.nodes[pc].bondOrders[i] == 1) {
                    acylHalideHalogenZ = nz;
                    break;
                }
            }
            if (acylHalideHalogenZ != -1) break;
        }
    } else if (winningType == GroupType::SULFONYL_HALIDE) {
        for (int pc : principalCarbons) {
            for (size_t i = 0; i < g.nodes[pc].neighbors.size(); ++i) {
                int nei = g.nodes[pc].neighbors[i];
                if (g.nodes[nei].atomicNumber == 16) { // Find the sulfur
                    for (size_t j = 0; j < g.nodes[nei].neighbors.size(); ++j) {
                        int sNei = g.nodes[nei].neighbors[j];
                        int sNz = g.nodes[sNei].atomicNumber;
                        if ((sNz == 9 || sNz == 17 || sNz == 35 || sNz == 53) && g.nodes[nei].bondOrders[j] == 1) {
                            acylHalideHalogenZ = sNz;
                            break;
                        }
                    }
                }
                if (acylHalideHalogenZ != -1) break;
            }
            if (acylHalideHalogenZ != -1) break;
        }
    } else if (winningType == GroupType::ESTER) {
        int esterAlkylRoot = -1;
        int esterOxygen = -1;
        for (int pc : principalCarbons) {
            std::vector<int> singleO;
            for (size_t i = 0; i < g.nodes[pc].neighbors.size(); ++i) {
                int nei = g.nodes[pc].neighbors[i];
                if (g.nodes[nei].atomicNumber == 8 && g.nodes[pc].bondOrders[i] == 1) {
                    singleO.push_back(nei);
                }
            }
            for (int sO : singleO) {
                int cCount = 0;
                int alkylRootNode = -1;
                for (int oNei : g.nodes[sO].neighbors) {
                    if (g.nodes[oNei].atomicNumber == 6) {
                        cCount++;
                        if (oNei != pc) alkylRootNode = oNei;
                    }
                }
                if (cCount == 2 && alkylRootNode != -1) {
                    esterAlkylRoot = alkylRootNode;
                    esterOxygen = sO;
                    break;
                }
            }
            if (esterAlkylRoot != -1) break;
        }

        if (esterAlkylRoot != -1) {
            QString alkylName;
            if (isPlainBenzeneRing(mol, g, indigoToGraphIdx, esterAlkylRoot, esterOxygen)) {
                alkylName = "phenyl";
            } else {
                alkylName = nameBranchGraph(g, esterAlkylRoot, esterOxygen, allSSSRRings);
            }
            if (alkylName.isEmpty()) return "";
            esterAlkylPrefix = alkylName + " ";
        } else {
            return "";
        }
    }

    QString baseName = nameAcyclicChainParentWithSubstituents(g, principalCarbons, principalCarbons,
                                                   ringNodeSet, extraSubstituents, winningType, acylHalideHalogenZ);
    if (baseName.isEmpty()) return "";
    return esterAlkylPrefix + baseName;
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
    const std::map<std::pair<int,int>, QChar> bondCIP = computeIndigoBondCIP(mol);

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
        int deg_u = 0;
        for (int nei : g.nodes[u].neighbors) {
            if (nei != v) deg_u++;
        }
        int deg_v = 0;
        for (int nei : g.nodes[v].neighbors) {
            if (nei != u) deg_v++;
        }

        // Terminal alkene (=CH2): skip silently (not stereogenic)
        if (deg_u == 0 || deg_v == 0) {
            continue;
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

        // 4. Look up Indigo's own real CIP answer for this bond (2nd-shell-aware,
        // handles trisubstituted/tetrasubstituted cases the old hand-rolled
        // first-shell-only comparator could not). Absent = not stereogenic
        // (e.g. symmetric substituents) or geometry undefined -- skip silently.
        auto key = std::make_pair(std::min(g.nodes[u].indigoIdx, g.nodes[v].indigoIdx),
                                   std::max(g.nodes[u].indigoIdx, g.nodes[v].indigoIdx));
        auto cipIt = bondCIP.find(key);
        if (cipIt == bondCIP.end()) {
            continue;
        }

        stereoByGraphId[targetGraphId] = cipIt->second;
    }

    return {true, "", ""};
}

} // anonymous namespace

// Assumes generateName()'s multi-component rejection (indigoCountComponents(mol) > 1)
// stays in place: Indigo's JSON saver gives each disconnected component its own,
// separately-re-based "mol0"/"mol1"/... node with LOCAL atom indices, and this
// function flattens every molecule node's bonds into one map keyed only by index --
// if multi-component naming is ever supported, index collisions across components
// would silently attach a wrong E/Z letter to the wrong bond.
// External linkage deliberately (declared in IupacNamer.h) so IndigoService.cpp
// can share it too -- see IUPAC Blue Book Coverage.md item 8.
std::map<std::pair<int,int>, QChar> computeIndigoBondCIP(int mol) {
    std::map<std::pair<int,int>, QChar> result;
    // Assumes this runs in a dedicated/throwaway Indigo session (matching
    // IndigoService.cpp's identical pattern) -- this option is process/session-global
    // and is never reset, so setting it on the shared main session would make every
    // future indigoJson()/toKetJson() call on that session also emit "cip" fields.
    indigoSetOptionBool("json-saving-add-stereo-desc", 1);
    const char* ketStr = indigoJson(mol);
    if (!ketStr) return result;
    QJsonDocument ketDoc = QJsonDocument::fromJson(QByteArray(ketStr));
    QJsonObject ketRoot = ketDoc.object();
    QJsonArray nodes = ketRoot.value("root").toObject().value("nodes").toArray();
    for (const QJsonValue &nodeVal : nodes) {
        QString ref = nodeVal.toObject().value("$ref").toString();
        if (ref.isEmpty()) continue;
        QJsonObject molObj = ketRoot.value(ref).toObject();
        if (molObj.value("type").toString() != "molecule") continue;
        const QJsonArray bonds = molObj.value("bonds").toArray();
        for (const QJsonValue &bondVal : bonds) {
            QJsonObject bondObj = bondVal.toObject();
            QString label = bondObj.value("cip").toString();
            if (label != "E" && label != "Z") continue;
            QJsonArray bondAtoms = bondObj.value("atoms").toArray();
            if (bondAtoms.size() != 2) continue;
            int a1 = bondAtoms.at(0).toInt();
            int a2 = bondAtoms.at(1).toInt();
            result[{std::min(a1, a2), std::max(a1, a2)}] = label == "Z" ? QChar('Z') : QChar('E');
        }
    }
    return result;
}

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
        } else if (z == 6 || z == 7 || z == 8 || z == 16 || z == 15 || z == 33 || z == 5 || z == 9 || z == 17 || z == 35 || z == 53 || z == 34 || z == 52) {
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
            if (cip == 2) {
                if (indigoToGraphIdx.count(idx)) {
                    stereoByGraphId[indigoToGraphIdx[idx]] = 's';
                }
            } else if (cip == 3) {
                if (indigoToGraphIdx.count(idx)) {
                    stereoByGraphId[indigoToGraphIdx[idx]] = 'r';
                }
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
    std::map<int, int> carbonSulfinicAcid; // carbonNode -> sulfurNode
    std::map<int, int> carbonThiol;        // carbonNode -> sulfurNode
    std::map<int, int> carbonSelenol;     // carbonNode -> seleniumNode
    std::map<int, int> carbonTellurol;    // carbonNode -> telluriumNode
    std::set<int> thioetherSulfurs;       // sulfurNodes
    std::set<int> sulfoxideSulfurs;       // sulfurNodes
    std::set<int> sulfoneSulfurs;         // sulfurNodes
    std::set<int> disulfideSulfurs;       // sulfurNodes
    std::set<int> selenoetherSeleniums;    // seleniumNodes
    std::set<int> selenoxideSeleniums;     // seleniumNodes: R2Se=O (seleninyl)
    std::set<int> selenoneSeleniums;       // seleniumNodes: R2SeO2 (selenonyl)
    std::set<int> diselenideSeleniums;     // seleniumNodes: R-Se-Se-R' (diselanyl)
    std::set<int> telluroetherTelluriums;  // telluriumNodes
    std::set<int> telluroxideTelluriums;   // telluriumNodes: R2Te=O (tellurinyl)
    std::set<int> telluroneTelluriums;     // telluriumNodes: R2TeO2 (telluronyl)
    std::set<int> ditellurideTelluriums;   // telluriumNodes: R-Te-Te-R' (ditellanyl)
    std::map<int, std::vector<int>> carbonAzide;      // carbonNode -> vector of azide N1 nodes
    std::map<int, int> carbonPhosphine;        // carbonNode -> phosphorusNode
    std::map<int, int> carbonPhosphonicAcid;   // carbonNode -> phosphorusNode
    std::map<int, int> carbonPhosphonicDihalide; // carbonNode -> phosphorusNode
    std::map<int, int> phosphonicDihalideZ;    // carbonNode -> halogen atomic number
    std::map<int, int> carbonArsonicAcid;      // carbonNode -> arsenicNode
    std::map<int, int> carbonBoronicAcid;      // carbonNode -> boronNode
    std::map<int, int> carbonSulfonylHalide;   // carbonNode -> sulfurNode
    std::map<int, int> sulfonylHalideZ;        // carbonNode -> halogen atomic number

    for (size_t i = 0; i < g.nodes.size(); ++i) {
        const GraphNode &node = g.nodes[i];
        if (node.atomicNumber == 16) {
            int dblO = 0, sglO_OH = 0, sglC = 0, dblC = 0, sglS = 0, sglN = 0;
            std::vector<int> cNeighbors;
            std::vector<int> halogens;
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
                } else if ((nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) && order == 1) {
                    halogens.push_back(nei);
                }
            }

            if (dblO == 2 && sglO_OH == 1 && sglC == 1 && node.neighbors.size() == 4) {
                carbonSulfonicAcid[cNeighbors[0]] = static_cast<int>(i);
            } else if (dblO == 2 && sglC == 1 && halogens.size() == 1 && node.neighbors.size() == 4) {
                carbonSulfonylHalide[cNeighbors[0]] = static_cast<int>(i);
                sulfonylHalideZ[cNeighbors[0]] = g.nodes[halogens[0]].atomicNumber;
            } else if (dblO == 1 && sglO_OH == 1 && sglC == 1 && node.neighbors.size() == 3) {
                carbonSulfinicAcid[cNeighbors[0]] = static_cast<int>(i);
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
                return {false, "", "Sulfur-containing groups other than thiols, thioethers, and sulfonic/sulfinic acids are not supported in this phase."};
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

            int sglC = 0, dblO = 0, sglO_OH = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 8 && order == 2) {
                    dblO++;
                } else if (nZ == 8 && order == 1 && (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) {
                    sglO_OH++;
                }
            }
            std::vector<int> halogens;
            for (int nei : node.neighbors) {
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 9 || nZ == 17 || nZ == 35 || nZ == 53) {
                    halogens.push_back(nei);
                }
            }

            if (sglC == 1 && node.neighbors.size() == 1 && node.totalH >= 1) {
                carbonPhosphine[cNeighbors[0]] = static_cast<int>(i);
            } else if (sglC == 1 && dblO == 1 && sglO_OH == 2 && node.neighbors.size() == 4) {
                carbonPhosphonicAcid[cNeighbors[0]] = static_cast<int>(i);
            } else if (sglC == 1 && dblO == 1 && halogens.size() == 2 && node.neighbors.size() == 4 && g.nodes[halogens[0]].atomicNumber == g.nodes[halogens[1]].atomicNumber) {
                carbonPhosphonicDihalide[cNeighbors[0]] = static_cast<int>(i);
                phosphonicDihalideZ[cNeighbors[0]] = g.nodes[halogens[0]].atomicNumber;
            } else {
                return {false, "", "Phosphorus-containing groups other than phosphine and phosphonic acid are not supported in this phase."};
            }
        } else if (node.atomicNumber == 33) {
            bool inAromaticRing = false;
            for (int order : node.bondOrders) {
                if (order == 4) inAromaticRing = true;
            }
            if (inAromaticRing) continue;

            int sglC = 0, dblO = 0, sglO_OH = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 8 && order == 2) {
                    dblO++;
                } else if (nZ == 8 && order == 1 && (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) {
                    sglO_OH++;
                }
            }
            if (sglC == 1 && dblO == 1 && sglO_OH == 2 && node.neighbors.size() == 4) {
                carbonArsonicAcid[cNeighbors[0]] = static_cast<int>(i);
            } else {
                return {false, "", "Arsenic-containing groups other than arsonic acid are not supported in this phase."};
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
        } else if (node.atomicNumber == 34) {
            int sglC = 0, dblO = 0, sglSe = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 8 && order == 2) {
                    dblO++;
                } else if (nZ == 34 && order == 1) {
                    sglSe++;
                }
            }
            if (sglC == 1 && node.neighbors.size() == 1 && node.totalH >= 1) {
                carbonSelenol[cNeighbors[0]] = static_cast<int>(i);
            } else if (sglC == 2 && node.neighbors.size() == 2) {
                selenoetherSeleniums.insert(static_cast<int>(i));
            } else if (dblO == 1 && sglC == 2 && node.neighbors.size() == 3) {
                selenoxideSeleniums.insert(static_cast<int>(i));
            } else if (dblO == 2 && sglC == 2 && node.neighbors.size() == 4) {
                selenoneSeleniums.insert(static_cast<int>(i));
            } else if (sglSe == 1 && sglC == 1 && dblO == 0 && node.neighbors.size() == 2) {
                diselenideSeleniums.insert(static_cast<int>(i));
            }
            // Note: unlike phosphorus/boron, selenium is also used elsewhere in this file
            // for ring heterocycles (selenophene, selenazole, etc.) -- a Se atom that
            // doesn't match the plain -SeH shape above is left unclassified here (not
            // rejected) so those ring-heterocycle code paths still see it untouched.
        } else if (node.atomicNumber == 52) {
            int sglC = 0, dblO = 0, sglTe = 0;
            std::vector<int> cNeighbors;
            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;
                if (nZ == 6 && order == 1) {
                    sglC++;
                    cNeighbors.push_back(nei);
                } else if (nZ == 8 && order == 2) {
                    dblO++;
                } else if (nZ == 52 && order == 1) {
                    sglTe++;
                }
            }
            if (sglC == 1 && node.neighbors.size() == 1 && node.totalH >= 1) {
                carbonTellurol[cNeighbors[0]] = static_cast<int>(i);
            } else if (sglC == 2 && node.neighbors.size() == 2) {
                telluroetherTelluriums.insert(static_cast<int>(i));
            } else if (dblO == 1 && sglC == 2 && node.neighbors.size() == 3) {
                telluroxideTelluriums.insert(static_cast<int>(i));
            } else if (dblO == 2 && sglC == 2 && node.neighbors.size() == 4) {
                telluroneTelluriums.insert(static_cast<int>(i));
            } else if (sglTe == 1 && sglC == 1 && dblO == 0 && node.neighbors.size() == 2) {
                ditellurideTelluriums.insert(static_cast<int>(i));
            }
            // Note: unlike phosphorus/boron, tellurium is also used elsewhere in this file
            // for ring heterocycles (tellurophene, etc.) -- a Te atom that doesn't match
            // the plain -TeH shape above is left unclassified here (not rejected) so those
            // ring-heterocycle code paths still see it untouched.
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

    // PHASE 59: Guard for ringCount == 2 with disjoint rings
    // Check if the two rings are actually connected to each other (fused/bridged/spiro/directly bonded)
    // If not, they should be treated as separate rings connected via acyclic linkers, not as a fused system.
    bool twoRingsAreDisjoint = false;
    if (ringCount == 2 && allSSSRRings.size() == 2) {
        const std::set<int> &ring1Nodes = allSSSRRings[0];
        const std::set<int> &ring2Nodes = allSSSRRings[1];
        
        // Check if the two rings share any atoms (fused/bridged/spiro)
        bool shareAtoms = false;
        for (int n : ring1Nodes) {
            if (ring2Nodes.count(n)) {
                shareAtoms = true;
                break;
            }
        }
        
        // Check if the two rings are directly bonded (single bond between them, like biphenyl).
        // ring1Nodes/ring2Nodes (from allSSSRRings) are already graph node indices (translated
        // via indigoToGraphIdx when allSSSRRings was populated above) -- no further translation here.
        bool directlyBonded = false;
        if (!shareAtoms) {
            for (int g1 : ring1Nodes) {
                for (int g2 : ring2Nodes) {
                    for (size_t i = 0; i < g.nodes[g1].neighbors.size(); ++i) {
                        if (g.nodes[g1].neighbors[i] == g2) {
                            directlyBonded = true;
                            break;
                        }
                    }
                    if (directlyBonded) break;
                }
                if (directlyBonded) break;
            }
        }
        
        // If rings don't share atoms AND aren't directly bonded, they're disjoint
        twoRingsAreDisjoint = !shareAtoms && !directlyBonded;
    }

    if (ringCount == 2 && allSSSRRings.size() == 2) {
        const std::set<int> &ring1Nodes = allSSSRRings[0];
        const std::set<int> &ring2Nodes = allSSSRRings[1];
        if ((ring1Nodes.size() == 5 || ring1Nodes.size() == 6) && (ring2Nodes.size() == 5 || ring2Nodes.size() == 6) && ring1Nodes.size() == ring2Nodes.size()) {
            std::vector<int> sharedNodes;
            for (int n : ring1Nodes) {
                if (ring2Nodes.count(n)) sharedNodes.push_back(n);
            }
            if (sharedNodes.empty()) {
                auto checkRingAssemblyOneSide = [&](const std::set<int> &rNodes, const std::set<int> &otherRNodes) -> std::pair<QString, int> {
                    int attachNodeThis = -1;
                    int attachNodeOther = -1;
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
                                } else {
                                    attachNodeThis = rNode;
                                    attachNodeOther = nei;
                                }
                            }
                        }
                        if (exoCount > 1) {
                            validSubstituent = false;
                        } else if (exoCount == 1) {
                            nodesWithExo++;
                        }
                    }
                    if (!validSubstituent || nodesWithExo != 1 || attachNodeThis == -1) return {"", -1};

                    QString subName = nameRingAsSubstituent(g, rNodes, attachNodeThis, attachNodeOther, allSSSRRings);
                    if (subName.isEmpty()) return {"", -1};

                    while (subName.startsWith("(") || subName.startsWith("[") || subName.startsWith("{")) subName = subName.mid(1);
                    while (subName.endsWith(")") || subName.endsWith("]") || subName.endsWith("}")) subName.chop(1);

                    if (subName == "phenyl") return {"phenyl", 1};
                    if (subName == "cyclohexyl") return {"cyclohexyl", 1};

                    if (!subName.endsWith("-yl")) return {"", -1};
                    int lastDash = subName.lastIndexOf('-');
                    if (lastDash == -1) return {"", -1};
                    int prevDash = subName.lastIndexOf('-', lastDash - 1);
                    if (prevDash == -1) return {"", -1};

                    bool ok;
                    int locant = subName.mid(prevDash + 1, lastDash - prevDash - 1).toInt(&ok);
                    if (!ok) return {"", -1};

                    // Every bare monocyclic heterocycle parent name this codebase produces ends
                    // in a terminal "e" (pyridine, thiophene, pyrrole, oxazole, ... and every
                    // hwGeneralRingStem() form: irine/irene/ete/ole/ine/inine/epine/ocine/onine/
                    // ecine) EXCEPT the furan family (furan, tetrahydrofuran, ...), which never
                    // had one to begin with. A whitelist of specific curated ring-name endings
                    // would silently produce a wrong (missing-e) name for any general Hantzsch-
                    // Widman heterocycle not in the list (e.g. an uncommon-element or 7-10
                    // membered ring) -- restoring "e" by default and excluding only the one real
                    // exception is robust to every ring type nameRingAsSubstituent can produce,
                    // not just the ones this task happened to test.
                    QString stem = subName.left(prevDash);
                    QString parentName = stem;
                    if (!parentName.endsWith("furan") && !parentName.endsWith("e")) {
                        parentName += "e";
                    }

                    return {parentName, locant};
                };

                auto res1 = checkRingAssemblyOneSide(ring1Nodes, ring2Nodes);
                auto res2 = checkRingAssemblyOneSide(ring2Nodes, ring1Nodes);
                if (!res1.first.isEmpty() && !res2.first.isEmpty() && res1.first == res2.first) {
                    if (res1.first == "phenyl" || res1.first == "cyclohexyl") {
                        return {true, "bi" + res1.first, ""};
                    } else {
                        int loc1 = std::min(res1.second, res2.second);
                        int loc2 = std::max(res1.second, res2.second);
                        return {true, QString("%1,%2-bi%3").arg(loc1).arg(loc2).arg(res1.first), ""};
                    }
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

        if (validSubstituent && (mainChainExoCount == 1 || twoRingsAreDisjoint) && foundAttachChainNode != -1 && attachRingNode != -1) {
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
                // IUPAC Blue Book Coverage.md item 6: pName is used verbatim by later
                // consumers (a plain string append, no separate formatBranchStereoPrefix
                // call on these ring nodes) -- passing stereo through here is safe, no
                // double-processing risk, and lets a ring substituent's own stereocenter
                // reach the name instead of being silently dropped.
                std::set<int> ringInfoHandledStereoIds;
                QString pName = nameRingAsSubstituent(g, rNodes, attachRingNode, foundAttachChainNode, allSSSRRings, {}, stereoByGraphId, &ringInfoHandledStereoIds);
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
        std::map<int, int> carbonHydroperoxide; // carbonNode -> near-oxygen node
        std::map<int, int> carbonImine; // carbonNode -> nitrogenNode

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            const GraphNode &node = g.nodes[i];
            if (node.atomicNumber == 6) {
                if (isocyanateCarbons.count(static_cast<int>(i))) continue;
                std::vector<int> doubleO, singleO, singleN, tripleN, halogens, doubleS, doubleN;

                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    int nZ = g.nodes[nei].atomicNumber;

                    if (nZ == 8 && order == 2) doubleO.push_back(nei);
                  else if (nZ == 16 && order == 2) doubleS.push_back(nei);
                  else if (nZ == 7 && order == 2) doubleN.push_back(nei);
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

                            QString name1 = nameAcidChainFrom(g, c1, exclude1, {});
                            QString name2 = nameAcidChainFrom(g, c2, exclude2, {});

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
                            if (!halogens.empty()) {
                                return {false, "", "Esters with a coexisting halogen on the acyl carbon (chloroformate-type structures) are not supported in this phase."};
                            }
                            int singleC = 0;
                            for (int nei : node.neighbors) {
                                if (g.nodes[nei].atomicNumber == 6) singleC++;
                            }
                            if (singleC == 0 && node.totalH == 0) {
                                return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                            }
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
                    if (hasOH) {
                        carbonGroup[i] = GroupType::ACID;
                    } else if (isPeroxyCarboxylicAcid(static_cast<int>(i), g)) {
                        return {false, "", "Peroxycarboxylic acids are not supported in this phase."};
                    }
                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    if (halogens.size() > 1) {
                        return {false, "", "Carbonic acid halides with multiple halogens are not supported in this phase."};
                    }
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    if (!halogens.empty()) {
                        return {false, "", "Amides with coexisting halogens on the acyl carbon are not supported."};
                    }
                    bool isHydrazide = false;
                    for (int sN : singleN) {
                        for (size_t k = 0; k < g.nodes[sN].neighbors.size(); ++k) {
                            int nei = g.nodes[sN].neighbors[k];
                            if (nei != static_cast<int>(i) && g.nodes[nei].atomicNumber == 7 && g.nodes[sN].bondOrders[k] == 1) {
                                isHydrazide = true;
                                break;
                            }
                        }
                        if (isHydrazide) break;
                    }
                    if (isHydrazide) {
                        carbonGroup[i] = GroupType::HYDRAZIDE;
                    } else {
                        carbonGroup[i] = GroupType::AMIDE;
                    }
                } else if (!tripleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0 && node.totalH == 0) {
                        return {false, "", "Cyanic acid halide derivatives (rootless nitrile carbons) are not supported in this phase."};
                    }
                    carbonGroup[i] = GroupType::NITRILE;
                } else if (!doubleO.empty()) {
                    if (isAcylPseudohalide(static_cast<int>(i), g, carbonAzide)) {
                        return {false, "", "Acyl pseudohalides are not supported in this phase."};
                    }
                    if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                        carbonGroup[i] = GroupType::ALDEHYDE;
                    } else {
                        carbonGroup[i] = GroupType::KETONE;
                    }
                } else if (!doubleS.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0 && node.totalH == 0) {
                        return {false, "", "Carbonothioyl/thiocarbamoyl halide derivatives (rootless thiocarbonyl carbons) are not supported in this phase."};
                    }
                    if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                        carbonGroup[i] = GroupType::THIAL;
                    } else {
                        carbonGroup[i] = GroupType::THIONE;
                    }
                } else if (!doubleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0 && node.totalH == 0) {
                        return {false, "", "Carbonimidic/carbamimidic acid halide derivatives (rootless imine carbons) are not supported in this phase."};
                    }
                    int nNode = doubleN[0];
                    if (doubleN.size() == 1 && (g.nodes[nNode].totalH >= 1 || g.nodes[nNode].neighbors.size() == 1)) {
                        carbonImine[i] = nNode;
                        carbonGroup[i] = GroupType::IMINE;
                    } else {
                        return {false, "", "N-substituted imines, oximes, hydrazones, and amidines are not supported in this phase; only the unsubstituted C=NH imine is supported."};
                    }

                } else if (!singleO.empty()) {
                    bool foundGroup = false;
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            carbonGroup[i] = GroupType::ALCOHOL; foundGroup = true; break;
                        }
                    }
                    if (!foundGroup) {
                        for (int sO : singleO) {
                            if (g.nodes[sO].neighbors.size() == 2) {
                                for (int oNei : g.nodes[sO].neighbors) {
                                    if (oNei != static_cast<int>(i) && g.nodes[oNei].atomicNumber == 8) {
                                        // Check if this is a dialkyl peroxide (R-O-O-R')
                                        bool isFarOHydroperoxide = (g.nodes[oNei].totalH >= 1 || g.nodes[oNei].neighbors.size() == 1);
                                        bool isFarODialkyl = false;
                                        for (int farNei : g.nodes[oNei].neighbors) {
                                            if (farNei != sO && g.nodes[farNei].atomicNumber == 6) {
                                                isFarODialkyl = true;
                                                break;
                                            }
                                        }
                                        if (isFarOHydroperoxide) {
                                            carbonHydroperoxide[i] = sO; // carbonNode -> near-oxygen node
                                            carbonGroup[i] = GroupType::HYDROPEROXIDE;
                                            foundGroup = true;
                                            break;
                                        } else if (isFarODialkyl) {
                                            return {false, "", "Dialkyl peroxides are not supported in this phase."};
                                        }
                                    }
                                }
                            }
                            if (foundGroup) break;
                        }
                    }
                } else if (!singleN.empty()) {
                    carbonGroup[i] = GroupType::AMINE;
                } else if (carbonSulfonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::SULFONIC_ACID;
                } else if (carbonSulfonylHalide.count(i)) {
                    carbonGroup[i] = GroupType::SULFONYL_HALIDE;
                } else if (carbonSulfinicAcid.count(i)) {
                    carbonGroup[i] = GroupType::SULFINIC_ACID;
                } else if (carbonBoronicAcid.count(i)) {
                    carbonGroup[i] = GroupType::BORONIC_ACID;
                } else if (carbonPhosphonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::PHOSPHONIC_ACID;
                } else if (carbonPhosphonicDihalide.count(i)) {
                    carbonGroup[i] = GroupType::PHOSPHONIC_DIHALIDE;
                } else if (carbonArsonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::ARSONIC_ACID;
                } else if (carbonThiol.count(i)) {
                    carbonGroup[i] = GroupType::THIOL;
                } else if (carbonSelenol.count(i)) {
                    carbonGroup[i] = GroupType::SELENOL;
                } else if (carbonTellurol.count(i)) {
                    carbonGroup[i] = GroupType::TELLUROL;
                } else if (carbonHydroperoxide.count(i)) {
                    carbonGroup[i] = GroupType::HYDROPEROXIDE;
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
            GroupType::SULFONIC_ACID, GroupType::SULFINIC_ACID, GroupType::ACID, GroupType::PHOSPHONIC_ACID, GroupType::PHOSPHONIC_DIHALIDE, GroupType::ARSONIC_ACID, GroupType::BORONIC_ACID, GroupType::ESTER, GroupType::ACYL_HALIDE, GroupType::SULFONYL_HALIDE, GroupType::AMIDE, GroupType::HYDRAZIDE, GroupType::NITRILE,
            GroupType::ALDEHYDE, GroupType::THIAL, GroupType::KETONE, GroupType::THIONE, GroupType::ALCOHOL, GroupType::THIOL, GroupType::SELENOL, GroupType::TELLUROL, GroupType::HYDROPEROXIDE, GroupType::AMINE, GroupType::IMINE, GroupType::PHOSPHINE
        };

        for (GroupType gt : seniorityOrder) {
            for (auto it = carbonGroup.begin(); it != carbonGroup.end(); ++it) {
                if (it->second == gt) {
                    winningType = gt; break;
                }
            }
            if (winningType != GroupType::NONE) break;
        }

        // P-44.1.2: a ring containing any skeletal heteroatom outright beats a
        // plain-carbon chain (this codebase's chains are always plain-carbon), but
        // only as a competition between candidates that can actually bear the
        // winning group -- if the ring has zero instances of winningType, forcing
        // ring-as-parent here would strand the principal group with no suffix-
        // bearing parent, which is wrong regardless of heteroatom seniority.
        bool ringHasWinningTypeAndHeteroatom = false;
        if (ringSubstituentInfos.size() == 1 && winningType != GroupType::NONE) {
            bool ringHasHeteroatom = false;
            bool ringHasWinningType = false;
            for (int n : ringSubstituentInfos[0].ringNodes) {
                if (g.nodes[n].atomicNumber != 6) ringHasHeteroatom = true;
                auto it = carbonGroup.find(n);
                if (it != carbonGroup.end() && it->second == winningType) ringHasWinningType = true;
            }
            ringHasWinningTypeAndHeteroatom = ringHasHeteroatom && ringHasWinningType;
        }

        if (ringSubstituentInfos.size() == 1 && (winningType == GroupType::NONE || ringHasWinningTypeAndHeteroatom)) {
            // Non-ring portion has no principal group (e.g. plain ethylbenzene), or
            // the ring itself genuinely bears the winning principal group AND has a
            // heteroatom, so it outright beats the plain-carbon chain (P-44.1.2.1).
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
                        [](const auto &x, const auto &y) { return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
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
            if (carbonSulfonylHalide.count(cNode) && winningType != GroupType::SULFONYL_HALIDE) {
                locantSubstituents[locant].append(halogenPrefix(sulfonylHalideZ[cNode]) + "sulfonyl");
            }
            if (carbonSulfinicAcid.count(cNode) && winningType != GroupType::SULFINIC_ACID) {
                locantSubstituents[locant].append("sulfino");
            }
            if (carbonBoronicAcid.count(cNode) && winningType != GroupType::BORONIC_ACID) {
                locantSubstituents[locant].append("borono");
            }
            if (carbonPhosphonicAcid.count(cNode) && winningType != GroupType::PHOSPHONIC_ACID) {
                locantSubstituents[locant].append("phosphono");
            }
            if (carbonPhosphonicDihalide.count(cNode) && winningType != GroupType::PHOSPHONIC_DIHALIDE) {
                locantSubstituents[locant].append("di" + halogenPrefix(phosphonicDihalideZ[cNode]) + "phosphoryl");
            }
            if (carbonArsonicAcid.count(cNode) && winningType != GroupType::ARSONIC_ACID) {
                locantSubstituents[locant].append("arsono");
            }
            if (carbonThiol.count(cNode) && winningType != GroupType::THIOL) {
                locantSubstituents[locant].append("sulfanyl");
            }
            if (carbonSelenol.count(cNode) && winningType != GroupType::SELENOL) {
                locantSubstituents[locant].append("selanyl");
            }
            if (carbonTellurol.count(cNode) && winningType != GroupType::TELLUROL) {
                locantSubstituents[locant].append("tellanyl");
            }
            if (carbonPhosphine.count(cNode) && winningType != GroupType::PHOSPHINE) {
                locantSubstituents[locant].append("phosphino");
            }
            if (carbonHydroperoxide.count(cNode) && winningType != GroupType::HYDROPEROXIDE) {
                locantSubstituents[locant].append("hydroperoxy");
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
                if (isPrincipalCarbon && (z == 7 || z == 8 || z == 16 || z == 34 || z == 52 || z == 9 || z == 17 || z == 35 || z == 53)) {
                    continue;
                }
                if (carbonSulfonicAcid.count(cNode) && carbonSulfonicAcid[cNode] == nei) continue;
                if (carbonSulfonylHalide.count(cNode) && carbonSulfonylHalide[cNode] == nei) continue;
                if (carbonSulfinicAcid.count(cNode) && carbonSulfinicAcid[cNode] == nei) continue;
                if (carbonPhosphonicAcid.count(cNode) && carbonPhosphonicAcid[cNode] == nei) continue;
                if (carbonPhosphonicDihalide.count(cNode) && carbonPhosphonicDihalide[cNode] == nei) continue;
                if (carbonArsonicAcid.count(cNode) && carbonArsonicAcid[cNode] == nei) continue;
                if (carbonThiol.count(cNode) && carbonThiol[cNode] == nei) continue;
                if (carbonSelenol.count(cNode) && carbonSelenol[cNode] == nei) continue;
                if (carbonTellurol.count(cNode) && carbonTellurol[cNode] == nei) continue;
                if (carbonHydroperoxide.count(cNode) && carbonHydroperoxide[cNode] == nei) continue;
                if (carbonImine.count(cNode) && carbonImine[cNode] == nei) continue;
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
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
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
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
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
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
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
                                if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                                alkylName += "disulfanyl";
                                locantSubstituents[locant].append(alkylName);
                            }
                        }
                    }
                } else if (z == 34) {
                    if (selenoetherSeleniums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "selanyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (selenoxideSeleniums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "seleninyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (selenoneSeleniums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "selenonyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (diselenideSeleniums.count(nei)) {
                        int se2Nei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 34 && oNei != cNode) { se2Nei = oNei; break; }
                        }
                        if (se2Nei != -1) {
                            int alkylNei = -1;
                            for (int oNei : g.nodes[se2Nei].neighbors) {
                                if (g.nodes[oNei].atomicNumber == 6 && oNei != nei) { alkylNei = oNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, se2Nei, allSSSRRings);
                                if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                                alkylName += "diselanyl";
                                locantSubstituents[locant].append(alkylName);
                            }
                        }
                    }
                } else if (z == 52) {
                    if (telluroetherTelluriums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "tellanyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (telluroxideTelluriums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "tellurinyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (telluroneTelluriums.count(nei)) {
                        int alkylNei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 6 && oNei != cNode) { alkylNei = oNei; break; }
                        }
                        if (alkylNei != -1) {
                            QString alkylName = nameBranchGraph(g, alkylNei, nei, allSSSRRings);
                            if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                            alkylName += "telluronyl";
                            locantSubstituents[locant].append(alkylName);
                        }
                    } else if (ditellurideTelluriums.count(nei)) {
                        int te2Nei = -1;
                        for (int oNei : g.nodes[nei].neighbors) {
                            if (g.nodes[oNei].atomicNumber == 52 && oNei != cNode) { te2Nei = oNei; break; }
                        }
                        if (te2Nei != -1) {
                            int alkylNei = -1;
                            for (int oNei : g.nodes[te2Nei].neighbors) {
                                if (g.nodes[oNei].atomicNumber == 6 && oNei != nei) { alkylNei = oNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, te2Nei, allSSSRRings);
                                if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
                                alkylName += "ditellanyl";
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
                                if (alkylName.isEmpty()) return {false, "", "Unrecognized or unsupported substituent."};
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
                            // "amino" only correctly represents a plain, unsubstituted -NH2.
                            // If nei has any other heavy-atom neighbor (e.g. a chained N as in
                            // a non-principal hydrazide's -NH-NH2, or any other N-substituent),
                            // silently calling it "amino" would drop that neighbor from the name
                            // entirely -- reject cleanly instead of producing an incomplete name.
                            bool isPlainNH2 = true;
                            for (int nNei2 : g.nodes[nei].neighbors) {
                                if (nNei2 != cNode) { isPlainNH2 = false; break; }
                            }
                            if (!isPlainNH2) {
                                return {false, "", "Substituted amine/hydrazine substituents are not supported in this phase."};
                            }
                            locantSubstituents[locant].append("amino");
                        }
                    }
                    if (carbonImine.count(cNode) && winningType != GroupType::IMINE) {
                        locantSubstituents[locant].append("imino");
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
                        // IUPAC Blue Book Coverage.md item 6: does nei lead into a ring? If so,
                        // nameBranchGraph delegates to nameRingAsSubstituent, which now embeds
                        // any stereocenter on the RING'S OWN atom directly (using the ring's
                        // real numbering) -- formatBranchStereoPrefix's chain-walk numbering
                        // has no relation to ring numbering and must not also run on it below,
                        // or the same stereocenter could be found and wrapped a second time.
                        bool neiIsInRing = false;
                        for (const auto &r : allSSSRRings) {
                            if (r.count(nei)) { neiIsInRing = true; break; }
                        }
                        QString bName = neiIsInRing
                            ? nameBranchGraph(g, nei, cNode, allSSSRRings, {}, stereoByGraphId, &handledBranchStereoIds)
                            : nameBranchGraph(g, nei, cNode, allSSSRRings);
                        if (bName.isEmpty()) {
                            // unnameable substituent (e.g. fused/bridged ring branch, azide) --
                            // must fail the whole name, not silently drop the substituent
                            return {false, "", "Unrecognized or unsupported substituent."};
                        }
                        QString branchStereo = neiIsInRing ? QString() : formatBranchStereoPrefix(g, nei, cNode, stereoByGraphId, handledBranchStereoIds);
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
            pg.baseName = alphabetizationKey(pName);
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
            // Phase 54: per-class suffix is built by the shared helper so it
            // cannot drift from the chain-as-parent path. The acyl-halide
            // halogen lookup is reproduced verbatim from the pre-refactor block
            // (including the acylHalideHalogen[parentChain[0]] default).
            int acylHalideZForSfx = -1;
            if (winningType == GroupType::ACYL_HALIDE) {
                acylHalideZForSfx = acylHalideHalogen[bestSig.parentChain[0]];
                for (int pc : bestSig.parentChain) {
                    if (acylHalideHalogen.count(pc)) { acylHalideZForSfx = acylHalideHalogen[pc]; break; }
                }
            } else if (winningType == GroupType::SULFONYL_HALIDE) {
                acylHalideZForSfx = sulfonylHalideZ[bestSig.parentChain[0]];
                for (int pc : bestSig.parentChain) {
                    if (sulfonylHalideZ.count(pc)) { acylHalideZForSfx = sulfonylHalideZ[pc]; break; }
                }
            } else if (winningType == GroupType::PHOSPHONIC_DIHALIDE) {
                acylHalideZForSfx = phosphonicDihalideZ[bestSig.parentChain[0]];
                for (int pc : bestSig.parentChain) {
                    if (phosphonicDihalideZ.count(pc)) { acylHalideZForSfx = phosphonicDihalideZ[pc]; break; }
                }
            }
            QString sfx = principalGroupSuffix(winningType, k, pCount, bestSig.principalLocants, acylHalideZForSfx);

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

        // --- Phase 26: Retained Names for Adamantane and Cubane ---
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
            auto myGroupRank = [](GroupType gt) -> int {
                if (gt == GroupType::ACID) return 1;
                if (gt == GroupType::AMIDE) return 2;
                if (gt == GroupType::HYDRAZIDE) return 3;
                if (gt == GroupType::KETONE) return 4;
                if (gt == GroupType::ALCOHOL) return 5;
                if (gt == GroupType::AMINE) return 6;
                return 99;
            };

            GroupType winningType = GroupType::NONE;
            std::map<int, GroupType> myCarbonGroup;
            for (int rIdx : allSSSRNodes) {
                bool hasDblO = false, hasSglO_OH = false, hasSglN = false;
                for (size_t j = 0; j < g.nodes[rIdx].neighbors.size(); ++j) {
                    int nei = g.nodes[rIdx].neighbors[j];
                    int order = g.nodes[rIdx].bondOrders[j];
                    int z = g.nodes[nei].atomicNumber;
                    if (z == 8 && order == 2) hasDblO = true;
                    if (z == 8 && order == 1 && (g.nodes[nei].totalH >= 1 || g.nodes[nei].neighbors.size() == 1)) hasSglO_OH = true;
                    if (z == 7 && order == 1) hasSglN = true;
                }
                if (hasDblO && hasSglO_OH) myCarbonGroup[rIdx] = GroupType::ACID;
                else if (hasDblO && hasSglN) {
                    bool isHydrazide = false;
                    for (size_t j = 0; j < g.nodes[rIdx].neighbors.size(); ++j) {
                        int nei = g.nodes[rIdx].neighbors[j];
                        if (g.nodes[nei].atomicNumber == 7 && g.nodes[rIdx].bondOrders[j] == 1) {
                            for (size_t k = 0; k < g.nodes[nei].neighbors.size(); ++k) {
                                int n2 = g.nodes[nei].neighbors[k];
                                if (n2 != rIdx && g.nodes[n2].atomicNumber == 7 && g.nodes[nei].bondOrders[k] == 1) {
                                    isHydrazide = true;
                                    break;
                                }
                            }
                        }
                    }
                    myCarbonGroup[rIdx] = isHydrazide ? GroupType::HYDRAZIDE : GroupType::AMIDE;
                }
                else if (hasDblO) myCarbonGroup[rIdx] = GroupType::KETONE;
                else if (hasSglO_OH) myCarbonGroup[rIdx] = GroupType::ALCOHOL;
                else if (hasSglN) myCarbonGroup[rIdx] = GroupType::AMINE;
                
                if (myCarbonGroup.count(rIdx)) {
                    GroupType t = myCarbonGroup[rIdx];
                    if (winningType == GroupType::NONE || myGroupRank(t) < myGroupRank(winningType)) {
                        winningType = t;
                    }
                }
            }

            std::set<int> principalCarbons;
            if (winningType != GroupType::NONE) {
                for (auto p : myCarbonGroup) {
                    if (p.second == winningType) principalCarbons.insert(p.first);
                }
            }

            bool subsOk = true;
            std::vector<std::pair<int, QString>> ringSubstituents;
            for (int rIdx : allSSSRNodes) {
                for (size_t j = 0; j < g.nodes[rIdx].neighbors.size(); ++j) {
                    int nei = g.nodes[rIdx].neighbors[j];
                    if (allSSSRNodes.count(nei)) continue;
                    
                    if (principalCarbons.count(rIdx)) {
                        int order = g.nodes[rIdx].bondOrders[j];
                        int z = g.nodes[nei].atomicNumber;
                        bool isPrincipalAtom = false;
                        if (winningType == GroupType::ACID && z == 8) isPrincipalAtom = true;
                        if (winningType == GroupType::AMIDE && (z == 8 || z == 7)) isPrincipalAtom = true;
                        if (winningType == GroupType::HYDRAZIDE && (z == 8 || z == 7)) isPrincipalAtom = true;
                        if (winningType == GroupType::KETONE && z == 8 && order == 2) isPrincipalAtom = true;
                        if (winningType == GroupType::ALCOHOL && z == 8 && order == 1) isPrincipalAtom = true;
                        if (winningType == GroupType::AMINE && z == 7 && order == 1) isPrincipalAtom = true;
                        if (isPrincipalAtom) continue;
                    }

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
                    std::vector<int> bestPrincipalLocants;
                    std::map<int, int> bestLocantOf;
                    bool first = true;

                    for (const auto& mapping : allMappings) {
                        std::vector<int> candLocants;
                        std::vector<std::pair<QString, int>> candNamedSubs;
                        std::vector<int> candPrincipalLocants;
                        std::map<int, int> candLocantOf;
                        
                        for (size_t i = 0; i < ringNodesVec.size(); ++i) {
                            candLocantOf[ringNodesVec[i]] = mapping[i] + 1;
                            if (principalCarbons.count(ringNodesVec[i])) {
                                candPrincipalLocants.push_back(mapping[i] + 1);
                            }
                        }
                        for (const auto& sub : ringSubstituents) {
                            int locant = candLocantOf[sub.first];
                            candLocants.push_back(locant);
                            candNamedSubs.push_back({sub.second, locant});
                        }
                        std::sort(candLocants.begin(), candLocants.end());
                        std::sort(candPrincipalLocants.begin(), candPrincipalLocants.end());

                        std::vector<int> combinedScore = candPrincipalLocants;
                        combinedScore.insert(combinedScore.end(), candLocants.begin(), candLocants.end());

                        if (first) {
                            bestLocants = combinedScore;
                            bestNamedSubs = candNamedSubs;
                            bestPrincipalLocants = candPrincipalLocants;
                            bestLocantOf = candLocantOf;
                            first = false;
                        } else {
                            if (combinedScore < bestLocants) {
                                bestLocants = combinedScore;
                                bestNamedSubs = candNamedSubs;
                                bestPrincipalLocants = candPrincipalLocants;
                                bestLocantOf = candLocantOf;
                            }
                        }
                    }

                    QString fullName = baseName;
                    
                    if (winningType != GroupType::NONE) {
                        QString sfx = principalGroupSuffix(winningType, 10, bestPrincipalLocants.size(), bestPrincipalLocants, 0);
                        QChar checkC;
                        for (QChar ch : sfx) {
                            if (ch.isLetter()) { checkC = ch; break; }
                        }
                        if (isVowel(checkC)) {
                            fullName.chop(1); // Drop 'e' if suffix starts with vowel
                        }
                        
                        fullName += sfx;
                    }

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
                                prefix += multiPrefix(it->second.size());
                            }
                            prefix += it->first;
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

                // --- Phase 53: substituent-aware bicyclic gate (P-23.2.2 / P-23.2.3) ---
                // The old gate required every ring-union atom to have ZERO exocyclic
                // neighbours. Phase 53 relaxes that: exocyclic branches are now
                // permitted if each is a simple substituent this codebase can already
                // name -- a plain saturated acyclic alkyl group (named via
                // nameBranchGraph) or a bare terminal halogen. Anything else
                // (heteroatom other than a terminal halogen, unsaturation, a ring
                // fused back into a branch, or a principal-characteristic-group-bearing
                // carbon) causes a clean fall-through to the generic rejection below
                // rather than a guessed name.
                bool validPreconditions = true;

                // Skeletal heteroatoms must be supported by hwSeniorityRank.
                for (int n : ringUnionNodes) {
                    if (g.nodes[n].atomicNumber != 6 && hwSeniorityRank(g.nodes[n].atomicNumber) == 99) { validPreconditions = false; break; }
                }

                // No bond within the ring union may have an invalid order (aromaticity out of scope,
                // but double/triple bonds are permitted).
                if (validPreconditions) {
                    for (const auto &gb : g.bonds) {
                        if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v) && gb.order != 1 && gb.order != 2 && gb.order != 3) {
                            validPreconditions = false; break;
                        }
                    }
                }

                // Classify every exocyclic branch as a permitted simple substituent
                // (or fall through). Collect each (ringAtom, subName) pair so the
                // actual locants can be assigned once the per-atom numbering is chosen.
                std::vector<std::pair<int, QString>> ringSubstituents;
                if (validPreconditions) {
                    for (int n : ringUnionNodes) {
                        for (size_t j = 0; j < g.nodes[n].neighbors.size(); ++j) {
                            int nei = g.nodes[n].neighbors[j];
                            if (ringUnionNodes.count(nei)) continue;
                            QString subName = simpleRingSubstituentName(g, n, nei, ringUnionNodes);
                            if (subName.isEmpty()) { validPreconditions = false; break; }
                            if (subName.startsWith("(") && subName.endsWith(")"))
                                subName = subName.mid(1, subName.length() - 2);
                            ringSubstituents.push_back({n, subName});
                        }
                        if (!validPreconditions) break;
                    }
                }

                if (validPreconditions) {
                    // Bridgehead identification: ring atoms with ring-degree 3.
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

                        std::vector<int> bhANeighbors;
                        for (int nei : g.nodes[bhA].neighbors) {
                            if (ringUnionNodes.count(nei)) {
                                bhANeighbors.push_back(nei);
                            }
                        }

                        if (bhANeighbors.size() == 3) {
                            // Trace the three bridges from bhA to bhB, capturing the
                            // ordered INTERIOR atom lists (excluding the bridgeheads)
                            // needed for P-23.2.3 per-atom numbering.
                            struct Bridge { std::vector<int> interior; int length; };
                            std::vector<Bridge> bridges;
                            bool traceOk = true;

                            for (int startNei : bhANeighbors) {
                                Bridge br;
                                br.length = 0;
                                if (startNei == bhB) {
                                    bridges.push_back(br); // direct bridgehead-bridgehead bond, length 0
                                    continue;
                                }
                                int prev = bhA;
                                int curr = startNei;
                                bool reachedB = false;
                                while (true) {
                                    if (curr == bhB) { reachedB = true; break; }
                                    if ((int)br.interior.size() > (int)ringUnionNodes.size()) break;
                                    br.interior.push_back(curr);
                                    int nextN = -1;
                                    for (int nei : g.nodes[curr].neighbors) {
                                        if (ringUnionNodes.count(nei) && nei != prev) { nextN = nei; break; }
                                    }
                                    if (nextN == -1) break;
                                    prev = curr;
                                    curr = nextN;
                                }
                                if (reachedB) { br.length = (int)br.interior.size(); bridges.push_back(br); }
                                else { traceOk = false; break; }
                            }

                            if (traceOk && bridges.size() == 3) {
                                int sumBridges = bridges[0].length + bridges[1].length + bridges[2].length;
                                if (sumBridges + 2 != (int)ringUnionNodes.size()) {
                                    return {false, "", "Internal error: invalid bicyclic bridge decomposition."};
                                }
                                int totalCarbons = (int)ringUnionNodes.size();
                                QString root = chainRoot(totalCarbons);
                                if (root.isEmpty()) {
                                    return {false, "", "Unsupported bicyclic ring size."};
                                }

                                // Bracket descriptor (descending lengths) is unchanged.
                                std::vector<int> lengths = {bridges[0].length, bridges[1].length, bridges[2].length};
                                std::sort(lengths.rbegin(), lengths.rend());

                                // --- P-23.2.3 numbering candidate enumeration ---
                                // For each choice of starting bridgehead S in {bhA,bhB}
                                // and each ordering of the three bridges whose lengths
                                // are non-increasing, walk the fixed algorithm (longest
                                // bridge first S->T, then second-longest T->S, then the
                                // shortest "main bridge" S->T) assigning locants. When a
                                // choice remains (which bridgehead is locant 1, or which
                                // of equal-length bridges is walked first), keep the
                                // candidate giving the lowest locant set to the
                                // substituents present, compared as an ascending-order
                                // set at the first point of difference -- the same
                                // convention already used by PathSignature/RingSignature
                                // elsewhere in this file.
                                struct NumberingCand {
                                    std::map<int,int> locantOf;
                                    std::vector<int> heteroatomLocants;
                                    std::vector<int> heteroatomSeniorityLocants;
                                    std::vector<int> doubleBondLocants;
                                    std::vector<int> tripleBondLocants;
                                    std::vector<int> subLocants;
                                    std::vector<std::pair<QString,int>> namedSubs;
                                };
                                std::vector<NumberingCand> cands;
                                int starts[2] = { bhA, bhB };

                                for (int sIdx = 0; sIdx < 2; ++sIdx) {
                                    int S = starts[sIdx];
                                    int Other = (S == bhA) ? bhB : bhA;
                                    std::vector<int> perm = {0, 1, 2};
                                    do {
                                        if (!(bridges[perm[0]].length >= bridges[perm[1]].length &&
                                              bridges[perm[1]].length >= bridges[perm[2]].length)) continue;

                                        NumberingCand cand;
                                        int loc = 1;
                                        cand.locantOf[S] = loc; // starting bridgehead = locant 1

                                        // Longest bridge: S -> Other (this assigns Other its locant).
                                        {
                                            std::vector<int> path = bridges[perm[0]].interior;
                                            if (S == bhB) std::reverse(path.begin(), path.end());
                                            for (int atom : path) { loc++; cand.locantOf[atom] = loc; }
                                            loc++; cand.locantOf[Other] = loc;
                                        }
                                        // Second bridge: Other -> S (interiors only, S already locant 1).
                                        {
                                            std::vector<int> path = bridges[perm[1]].interior;
                                            if (S == bhA) std::reverse(path.begin(), path.end());
                                            for (int atom : path) { loc++; cand.locantOf[atom] = loc; }
                                        }
                                        // Shortest (main) bridge: S -> Other (interiors only).
                                        {
                                            std::vector<int> path = bridges[perm[2]].interior;
                                            if (S == bhB) std::reverse(path.begin(), path.end());
                                            for (int atom : path) { loc++; cand.locantOf[atom] = loc; }
                                        }

                                        if ((int)cand.locantOf.size() != totalCarbons) continue;

                                        for (int n : ringUnionNodes) {
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
                                            if (it == cand.locantOf.end()) { cand.subLocants.clear(); break; }
                                            cand.subLocants.push_back(it->second);
                                            cand.namedSubs.push_back({rs.second, it->second});
                                        }
                                        std::sort(cand.subLocants.begin(), cand.subLocants.end());

                                        for (const auto &gb : g.bonds) {
                                            if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v)) {
                                                if (gb.order == 2 || gb.order == 3) {
                                                    auto itU = cand.locantOf.find(gb.u);
                                                    auto itV = cand.locantOf.find(gb.v);
                                                    if (itU != cand.locantOf.end() && itV != cand.locantOf.end()) {
                                                        int minLoc = std::min(itU->second, itV->second);
                                                        if (gb.order == 2) cand.doubleBondLocants.push_back(minLoc);
                                                        if (gb.order == 3) cand.tripleBondLocants.push_back(minLoc);
                                                    }
                                                }
                                            }
                                        }
                                        std::sort(cand.doubleBondLocants.begin(), cand.doubleBondLocants.end());
                                        std::sort(cand.tripleBondLocants.begin(), cand.tripleBondLocants.end());

                                        cands.push_back(cand);
                                    } while (std::next_permutation(perm.begin(), perm.end()));
                                }

                                if (!cands.empty()) {
                                    auto bestIt = std::min_element(cands.begin(), cands.end(),
                                        [](const NumberingCand &a, const NumberingCand &b) {
                                            if (a.heteroatomLocants != b.heteroatomLocants) return a.heteroatomLocants < b.heteroatomLocants;
                                            if (a.heteroatomSeniorityLocants != b.heteroatomSeniorityLocants) return a.heteroatomSeniorityLocants < b.heteroatomSeniorityLocants;
                                            if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;
                                            if (a.tripleBondLocants != b.tripleBondLocants) return a.tripleBondLocants < b.tripleBondLocants;
                                            if (a.subLocants != b.subLocants) return a.subLocants < b.subLocants;
                                            auto firstAlpha = [](const std::vector<std::pair<QString,int>> &nm) {
                                                return std::min_element(nm.begin(), nm.end(),
                                                    [](const auto &x, const auto &y){ return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
                                            };
                                            if (!a.namedSubs.empty()) {
                                                QString alphaName = firstAlpha(a.namedSubs)->first;
                                                auto findLoc = [&](const std::vector<std::pair<QString,int>> &nm) {
                                                    int best = INT_MAX;
                                                    for (const auto &ns : nm) if (ns.first == alphaName) best = std::min(best, ns.second);
                                                    return best;
                                                };
                                                int aLoc = findLoc(a.namedSubs), bLoc = findLoc(b.namedSubs);
                                                if (aLoc != bLoc) return aLoc < bLoc;
                                            }
                                            return false;
                                        });
                                    NumberingCand best = *bestIt;

                                    // --- Substituent prefix (PrefixGroup convention) ---
                                    std::map<QString, std::vector<int>> prefixLocantsMap;
                                    for (const auto &ns : best.namedSubs)
                                        prefixLocantsMap[ns.first].push_back(ns.second);

                                    struct PrefixGroup { QString baseName; QString formattedStr; };
                                    std::vector<PrefixGroup> pGroups;
                                    for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
                                        QString pName = it->first;
                                        std::vector<int> locs = it->second;
                                        std::sort(locs.begin(), locs.end());
                                        QStringList locStrs;
                                        for (int l : locs) locStrs.append(QString::number(l));
                                        QString pStr = locStrs.join(",");
                                        if (locs.size() > 1) pStr += "-" + multiPrefix((int)locs.size()) + pName;
                                        else pStr += "-" + pName;
                                        PrefixGroup pg; pg.baseName = pName; pg.formattedStr = pStr;
                                        pGroups.push_back(pg);
                                    }
                                    std::sort(pGroups.begin(), pGroups.end(),
                                        [](const PrefixGroup &a, const PrefixGroup &b){ return a.baseName.toLower() < b.baseName.toLower(); });

                                    QString prefixPart;
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
                                    }

                                    std::vector<int> dbLocs = best.doubleBondLocants;
                                    std::vector<int> tbLocs = best.tripleBondLocants;

                                    if (lengths[2] == 0 && (!dbLocs.empty() || !tbLocs.empty())) {
                                        // Unsaturated ortho-fused rings fall through to fusion nomenclature
                                    } else {
                                        QString infix;
                                        if (dbLocs.empty() && tbLocs.empty()) {
                                            infix = "ane";
                                        } else if (!dbLocs.empty() && tbLocs.empty()) {
                                            if (dbLocs.size() == 1) {
                                                infix = QString("-%1-ene").arg(dbLocs[0]);
                                            } else {
                                                root += "a";
                                                QStringList lStrs;
                                                for (int l : dbLocs) lStrs.append(QString::number(l));
                                                infix = QString("-%1-%2ene").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
                                            }
                                        } else if (dbLocs.empty() && !tbLocs.empty()) {
                                            if (tbLocs.size() == 1) {
                                                infix = QString("-%1-yne").arg(tbLocs[0]);
                                            } else {
                                                root += "a";
                                                QStringList lStrs;
                                                for (int l : tbLocs) lStrs.append(QString::number(l));
                                                infix = QString("-%1-%2yne").arg(lStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size())));
                                            }
                                        } else {
                                            QStringList dStrs, tStrs;
                                            for (int l : dbLocs) dStrs.append(QString::number(l));
                                            for (int l : tbLocs) tStrs.append(QString::number(l));
                                            QString dPart = (dbLocs.size() > 1) ? QString("%1-%2en").arg(dStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size()))) : QString("%1-en").arg(dStrs[0]);
                                            QString tPart = (tbLocs.size() > 1) ? QString("%1-%2yne").arg(tStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size()))) : QString("%1-yne").arg(tStrs[0]);
                                            infix = QString("-%1-%2").arg(dPart, tPart);
                                        }

                                        StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, best.locantOf, std::set<int>());
                                        if (!stereoRes.ok) return {false, "", stereoRes.error};

                                        QString fullName = stereoRes.prefix + prefixPart + QString("bicyclo[%1.%2.%3]%4%5")
                                            .arg(lengths[0]).arg(lengths[1]).arg(lengths[2]).arg(root).arg(infix);

                                        return {true, fullName, ""};
                                    }
                                }
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

                    // --- Phase 53: substituent-aware spiro gate (P-24.2.1) ---
                    bool validPreconditions = true;

                    // Every ring-union atom must be carbon or a supported skeletal heteroatom.
                    for (int n : ringUnionNodes) {
                        if (g.nodes[n].atomicNumber != 6 && hwSeniorityRank(g.nodes[n].atomicNumber) == 99) { validPreconditions = false; break; }
                    }

                    // Ring-membership degrees: spiro atom ring-degree 4, all others 2.
                    if (validPreconditions) {
                        for (int n : ringUnionNodes) {
                            int ringDegree = 0;
                            for (int nei : g.nodes[n].neighbors) if (ringUnionNodes.count(nei)) ringDegree++;
                            if (n == spiroNode) { if (ringDegree != 4) { validPreconditions = false; break; } }
                            else { if (ringDegree != 2) { validPreconditions = false; break; } }
                        }
                    }

                    // No bond within the ring union may have an invalid order (aromaticity out of scope,
                    // but double/triple bonds are permitted).
                    if (validPreconditions) {
                        for (const auto &gb : g.bonds) {
                            if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v) && gb.order != 1 && gb.order != 2 && gb.order != 3) {
                                validPreconditions = false; break;
                            }
                        }
                    }

                    // Classify every exocyclic branch as a permitted simple substituent
                    // (plain saturated acyclic alkyl, or a bare terminal halogen); collect
                    // (ringAtom, subName) pairs for locant assignment after numbering.
                    std::vector<std::pair<int, QString>> ringSubstituents;
                    if (validPreconditions) {
                        for (int n : ringUnionNodes) {
                            for (size_t j = 0; j < g.nodes[n].neighbors.size(); ++j) {
                                int nei = g.nodes[n].neighbors[j];
                                if (ringUnionNodes.count(nei)) continue;
                                QString subName = simpleRingSubstituentName(g, n, nei, ringUnionNodes);
                                if (subName.isEmpty()) { validPreconditions = false; break; }
                                if (subName.startsWith("(") && subName.endsWith(")"))
                                    subName = subName.mid(1, subName.length() - 2);
                                ringSubstituents.push_back({n, subName});
                            }
                            if (!validPreconditions) break;
                        }
                    }

                    if (validPreconditions) {
                        int cnt1 = static_cast<int>(ring1Nodes.size()) - 1;
                        int cnt2 = static_cast<int>(ring2Nodes.size()) - 1;
                        int totalCarbons = static_cast<int>(ringUnionNodes.size());
                        if (totalCarbons != cnt1 + cnt2 + 1) {
                            return {false, "", "Internal error: invalid spiro decomposition."};
                        }
                        QString root = chainRoot(totalCarbons);
                        if (root.isEmpty()) {
                            return {false, "", "Unsupported spiro ring size."};
                        }
                        // Bracket descriptor: counts cited in ascending order.
                        int sizeA = std::min(cnt1, cnt2);
                        int sizeB = std::max(cnt1, cnt2);

                        const std::set<int> *rings[2] = { &ring1Nodes, &ring2Nodes };
                        int rcnt[2] = { cnt1, cnt2 };
                        int spiro = spiroNode;

                        // Walk the non-spiro chain of a ring from one spiro-adjacent end
                        // to the other, returning the ordered atom list.
                        auto walkRingChain = [&](const std::set<int> &ringSet, int prevStart, int startNode) -> std::vector<int> {
                            std::vector<int> chain;
                            int prev = prevStart, curr = startNode;
                            while (true) {
                                chain.push_back(curr);
                                int next = -1;
                                for (int nei : g.nodes[curr].neighbors)
                                    if (ringSet.count(nei) && nei != prev) { next = nei; break; }
                                if (next == -1 || next == spiro) break; // reached the far end (its neighbour is spiro)
                                prev = curr; curr = next;
                            }
                            return chain;
                        };

                        // Collect the two atoms of a ring adjacent to the spiro atom.
                        auto ringEnds = [&](const std::set<int> &ringSet) -> std::vector<int> {
                            std::vector<int> ends;
                            for (int n : ringSet) {
                                if (n == spiro) continue;
                                for (int nei : g.nodes[n].neighbors) if (nei == spiro) { ends.push_back(n); break; }
                            }
                            return ends;
                        };

                        // --- P-24.2.1 numbering candidate enumeration ---
                        // The smaller ring is numbered first; if the two rings are the
                        // same size, either may be first (enumerated). For each ring taken
                        // first there are two start-direction choices, and likewise two
                        // for the second ring. Keep the candidate giving the lowest locant
                        // set to the substituents present -- the same convention already
                        // used by PathSignature / RingSignature and the bicyclic path.
                        struct NumberingCand {
                            std::map<int,int> locantOf;
                            std::vector<int> heteroatomLocants;
                            std::vector<int> heteroatomSeniorityLocants;
                            std::vector<int> doubleBondLocants;
                            std::vector<int> tripleBondLocants;
                            std::vector<int> subLocants;
                            std::vector<std::pair<QString,int>> namedSubs;
                        };
                        std::vector<NumberingCand> cands;

                        for (int firstIdx = 0; firstIdx < 2; ++firstIdx) {
                            int secondIdx = 1 - firstIdx;
                            // The smaller ring must be numbered first; only when the two
                            // rings tie in size is the other order a valid numbering.
                            if (rcnt[firstIdx] > rcnt[secondIdx]) continue;

                            const std::set<int> &firstRing = *rings[firstIdx];
                            const std::set<int> &secondRing = *rings[secondIdx];
                            std::vector<int> firstEnds = ringEnds(firstRing);
                            std::vector<int> secondEnds = ringEnds(secondRing);
                            if (firstEnds.size() != 2 || secondEnds.size() != 2) continue;

                            for (int fd = 0; fd < 2; ++fd) {
                                std::vector<int> firstChain = walkRingChain(firstRing, spiro, firstEnds[fd]);
                                if ((int)firstChain.size() != rcnt[firstIdx]) continue;

                                for (int sd = 0; sd < 2; ++sd) {
                                    std::vector<int> secondChain = walkRingChain(secondRing, spiro, secondEnds[sd]);
                                    if ((int)secondChain.size() != rcnt[secondIdx]) continue;

                                    NumberingCand cand;
                                    int loc = 0;
                                    for (int atom : firstChain) { loc++; cand.locantOf[atom] = loc; }
                                    loc++; cand.locantOf[spiro] = loc;
                                    for (int atom : secondChain) { loc++; cand.locantOf[atom] = loc; }

                                    if ((int)cand.locantOf.size() != totalCarbons) continue;

                                    for (int n : ringUnionNodes) {
                                        if (g.nodes[n].atomicNumber != 6) {
                                            cand.heteroatomLocants.push_back(cand.locantOf[n]);
                                        }
                                    }
                                    std::sort(cand.heteroatomLocants.begin(), cand.heteroatomLocants.end());

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
                                        if (it == cand.locantOf.end()) { cand.subLocants.clear(); break; }
                                        cand.subLocants.push_back(it->second);
                                        cand.namedSubs.push_back({rs.second, it->second});
                                    }
                                    std::sort(cand.subLocants.begin(), cand.subLocants.end());

                                    for (const auto &gb : g.bonds) {
                                        if (ringUnionNodes.count(gb.u) && ringUnionNodes.count(gb.v)) {
                                            if (gb.order == 2 || gb.order == 3) {
                                                auto itU = cand.locantOf.find(gb.u);
                                                auto itV = cand.locantOf.find(gb.v);
                                                if (itU != cand.locantOf.end() && itV != cand.locantOf.end()) {
                                                    int minLoc = std::min(itU->second, itV->second);
                                                    if (gb.order == 2) cand.doubleBondLocants.push_back(minLoc);
                                                    if (gb.order == 3) cand.tripleBondLocants.push_back(minLoc);
                                                }
                                            }
                                        }
                                    }
                                    std::sort(cand.doubleBondLocants.begin(), cand.doubleBondLocants.end());
                                    std::sort(cand.tripleBondLocants.begin(), cand.tripleBondLocants.end());

                                    cands.push_back(cand);
                                }
                            }
                        }

                        if (!cands.empty()) {
                            auto bestIt = std::min_element(cands.begin(), cands.end(),
                                [](const NumberingCand &a, const NumberingCand &b) {
                                    if (a.heteroatomLocants != b.heteroatomLocants) return a.heteroatomLocants < b.heteroatomLocants;
                                    if (a.heteroatomSeniorityLocants != b.heteroatomSeniorityLocants) return a.heteroatomSeniorityLocants < b.heteroatomSeniorityLocants;
                                    if (a.doubleBondLocants != b.doubleBondLocants) return a.doubleBondLocants < b.doubleBondLocants;
                                    if (a.tripleBondLocants != b.tripleBondLocants) return a.tripleBondLocants < b.tripleBondLocants;
                                    if (a.subLocants != b.subLocants) return a.subLocants < b.subLocants;
                                    auto firstAlpha = [](const std::vector<std::pair<QString,int>> &nm) {
                                        return std::min_element(nm.begin(), nm.end(),
                                            [](const auto &x, const auto &y){ return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
                                    };
                                    if (!a.namedSubs.empty()) {
                                        QString alphaName = firstAlpha(a.namedSubs)->first;
                                        auto findLoc = [&](const std::vector<std::pair<QString,int>> &nm) {
                                            int best = INT_MAX;
                                            for (const auto &ns : nm) if (ns.first == alphaName) best = std::min(best, ns.second);
                                            return best;
                                        };
                                        int aLoc = findLoc(a.namedSubs), bLoc = findLoc(b.namedSubs);
                                        if (aLoc != bLoc) return aLoc < bLoc;
                                    }
                                    return false;
                                });
                            NumberingCand best = *bestIt;

                            // --- Substituent prefix assembly (PrefixGroup convention) ---
                            std::map<QString, std::vector<int>> prefixLocantsMap;
                            for (const auto &ns : best.namedSubs)
                                prefixLocantsMap[ns.first].push_back(ns.second);
                            struct PrefixGroup { QString baseName; QString formattedStr; };
                            std::vector<PrefixGroup> pGroups;
                            for (auto it = prefixLocantsMap.begin(); it != prefixLocantsMap.end(); ++it) {
                                QString pName = it->first;
                                std::vector<int> locs = it->second;
                                std::sort(locs.begin(), locs.end());
                                QStringList locStrs;
                                for (int l : locs) locStrs.append(QString::number(l));
                                QString pStr = locStrs.join(",");
                                if (locs.size() > 1) pStr += "-" + multiPrefix((int)locs.size()) + pName;
                                else pStr += "-" + pName;
                                PrefixGroup pg; pg.baseName = pName; pg.formattedStr = pStr;
                                pGroups.push_back(pg);
                            }
                            std::sort(pGroups.begin(), pGroups.end(),
                                [](const PrefixGroup &a, const PrefixGroup &b){ return a.baseName.toLower() < b.baseName.toLower(); });

                            QString prefixPart;
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
                            }

                            std::vector<int> dbLocs = best.doubleBondLocants;
                            std::vector<int> tbLocs = best.tripleBondLocants;
                            QString infix;
                            if (dbLocs.empty() && tbLocs.empty()) {
                                infix = "ane";
                            } else if (!dbLocs.empty() && tbLocs.empty()) {
                                if (dbLocs.size() == 1) {
                                    infix = QString("-%1-ene").arg(dbLocs[0]);
                                } else {
                                    root += "a";
                                    QStringList lStrs;
                                    for (int l : dbLocs) lStrs.append(QString::number(l));
                                    infix = QString("-%1-%2ene").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
                                }
                            } else if (dbLocs.empty() && !tbLocs.empty()) {
                                if (tbLocs.size() == 1) {
                                    infix = QString("-%1-yne").arg(tbLocs[0]);
                                } else {
                                    root += "a";
                                    QStringList lStrs;
                                    for (int l : tbLocs) lStrs.append(QString::number(l));
                                    infix = QString("-%1-%2yne").arg(lStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size())));
                                }
                            } else {
                                QStringList dStrs, tStrs;
                                for (int l : dbLocs) dStrs.append(QString::number(l));
                                for (int l : tbLocs) tStrs.append(QString::number(l));
                                QString dPart = (dbLocs.size() > 1) ? QString("%1-%2en").arg(dStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size()))) : QString("%1-en").arg(dStrs[0]);
                                QString tPart = (tbLocs.size() > 1) ? QString("%1-%2yne").arg(tStrs.join(","), multiPrefix(static_cast<int>(tbLocs.size()))) : QString("%1-yne").arg(tStrs[0]);
                                infix = QString("-%1-%2").arg(dPart, tPart);
                            }

                            StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, best.locantOf, std::set<int>());
                            if (!stereoRes.ok) return {false, "", stereoRes.error};

                            QString fullName = stereoRes.prefix + prefixPart + QString("spiro[%1.%2]%3%4")
                                .arg(sizeA).arg(sizeB).arg(root).arg(infix);
                            return {true, fullName, ""};
                        }
                    }
                }
            }
        }
    }

    // --- Detect unsupported fusion topologies (P-25.3.1.1.2, P-25.4, P-25.5) ---
    {
        int N_rings = allSSSRRings.size();
        bool hasBridged = false;
        bool hasPeri = false;
        bool hasP25_5 = false;
        
        // 1. Detect P-25.4 Bridged-Fused: Any pair of rings sharing > 2 atoms or disconnected components
        for (int i = 0; i < N_rings; ++i) {
            bool ring1HasDouble = false;
            for (int n : allSSSRRings[i]) {
                for (int order : g.nodes[n].bondOrders) if (order == 2 || order == 4) ring1HasDouble = true;
            }
            for (int j = i + 1; j < N_rings; ++j) {
                bool ring2HasDouble = false;
                for (int n : allSSSRRings[j]) {
                    for (int order : g.nodes[n].bondOrders) if (order == 2 || order == 4) ring2HasDouble = true;
                }
                std::vector<int> shared;
                for (int n : allSSSRRings[i]) if (allSSSRRings[j].count(n)) shared.push_back(n);
                
                if (shared.size() >= 2 && ring1HasDouble && ring2HasDouble) {
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
                    if (components > 1 || shared.size() >= 3) {
                        hasBridged = true;
                    }
                }
            }
        }
        
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
        
        // 3. Detect P-25.5 (3-component peri-fusion): > 1 peri center, or 4 rings involved in peri fusion
        if (hasPeri && periCenters.size() >= 2) {
            hasP25_5 = true;
        } else if (hasPeri && N_rings >= 4) {
            hasP25_5 = true;
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
                                                    pg.baseName = alphabetizationKey(pName);
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


// --- Phase 44 & 48: N-Heterocycle Fusion Chain Nomenclature ---
    if (ringCount >= 3) {
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
            int N = sssrRings.size();
            std::vector<std::vector<int>> shared(N, std::vector<int>(N, 0));
            std::vector<std::vector<std::vector<int>>> sharedNodesPairs(N, std::vector<std::vector<int>>(N));
            for (int i = 0; i < N; ++i) {
                for (int j = i + 1; j < N; ++j) {
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

            std::vector<int> degree(N, 0);
            bool validFusion = true;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    if (shared[i][j] == 2) degree[i]++;
                    else if (shared[i][j] != 0) validFusion = false;
                }
            }

            int ends = 0, centers = 0;
            for (int i = 0; i < N; ++i) {
                if (degree[i] == 1) ends++;
                else if (degree[i] == 2) centers++;
                else if (degree[i] > 2) validFusion = false;
            }

            if (validFusion && ends == 2 && centers == N - 2) {
                std::vector<std::set<int>> nodesN(N);
                for (int i = 0; i < N; ++i) {
                    for (int a : sssrRings[i]) if (indigoToGraphIdx.count(a)) nodesN[i].insert(indigoToGraphIdx[a]);
                }

                auto buildCycleN = [&](const std::set<int> &rNodes) -> std::vector<int> {
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

                std::vector<std::vector<int>> cyclesN(N);
                std::vector<std::vector<int>> rHeteroN(N);
                std::vector<RingType> typesN(N);
                std::vector<bool> classOkN(N, false);
                bool allClassified = true;
                for (int i = 0; i < N; ++i) {
                    cyclesN[i] = buildCycleN(nodesN[i]);
                    for (int n : nodesN[i]) if (g.nodes[n].atomicNumber != 6) rHeteroN[i].push_back(n);
                    if (cyclesN[i].size() == nodesN[i].size()) {
                        QString d1, d2;
                        classOkN[i] = classifyMonocyclicHeteroRing(g, rHeteroN[i], static_cast<int>(nodesN[i].size()), cyclesN[i], typesN[i], d1, d2);
                    }
                    if (!classOkN[i]) allClassified = false;
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

                bool allAllowed = allClassified;
                for (int i = 0; i < N && allAllowed; ++i) if (!isAllowedType(typesN[i])) allAllowed = false;

                if (allAllowed) {
                    auto getCandidates = [&](RingType t, const std::vector<int> &hNodes, int rSize, const std::vector<int> &rCycle) {
                        std::vector<std::vector<int>> cands;
                        if (t == RingType::FURAN || t == RingType::THIOPHENE || t == RingType::SELENOPHENE || t == RingType::TELLUROPHENE || t == RingType::PHOSPHININE || t == RingType::PYRIDINE || t == RingType::PYRROLE) {
                            if (hNodes.size() == 1) {
                                int hNode = hNodes[0];
                                int hIdx = -1;
                                for (int i = 0; i < rSize; ++i) if (rCycle[i] == hNode) { hIdx = i; break; }
                                if (hIdx != -1) {
                                    std::vector<int> fwd(rSize), bwd(rSize);
                                    for (int i = 0; i < rSize; ++i) { fwd[i] = rCycle[(hIdx + i) % rSize]; bwd[i] = rCycle[(hIdx - i + rSize) % rSize]; }
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
                        } else if (t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE) {
                            if (hNodes.size() == 2) {
                                int hNH = -1, hN = -1;
                                if (g.nodes[hNodes[0]].totalH >= 1) { hNH = hNodes[0]; hN = hNodes[1]; }
                                else { hNH = hNodes[1]; hN = hNodes[0]; }
                                int hIdx = -1;
                                for (int i = 0; i < rSize; ++i) if (rCycle[i] == hNH) { hIdx = i; break; }
                                if (hIdx != -1) {
                                    std::vector<int> fwd(rSize), bwd(rSize);
                                    for (int i = 0; i < rSize; ++i) { fwd[i] = rCycle[(hIdx + i) % rSize]; bwd[i] = rCycle[(hIdx - i + rSize) % rSize]; }
                                    int reqOtherIdx = (t == RingType::IMIDAZOLE) ? 2 : 1;
                                    if (fwd[reqOtherIdx] == hN) cands.push_back(fwd);
                                    if (bwd[reqOtherIdx] == hN) cands.push_back(bwd);
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
                    auto getVariety = [&](const std::vector<int> &hNodes) {
                        std::set<int> elems;
                        for (int n : hNodes) elems.insert(g.nodes[n].atomicNumber);
                        return static_cast<int>(elems.size());
                    };
                    auto altRank = [](int z) {
                        switch (z) { case 9: return 1; case 17: return 2; case 35: return 3; case 53: return 4; case 8: return 5; case 16: return 6; case 34: return 7; case 52: return 8; case 7: return 9; case 15: return 10; default: return 99; }
                    };
                    auto getTopAltRank = [&](const std::vector<int> &hNodes) {
                        int best = 99;
                        for (int n : hNodes) best = std::min(best, altRank(g.nodes[n].atomicNumber));
                        return best;
                    };
                    auto getOwnLocants = [](RingType t) -> std::vector<int> {
                        switch (t) {
                            case RingType::PYRIDINE: case RingType::PYRROLE: case RingType::FURAN: case RingType::THIOPHENE: case RingType::SELENOPHENE: case RingType::TELLUROPHENE: case RingType::PHOSPHININE: return {1};
                            case RingType::PYRIDAZINE: case RingType::ISOXAZOLE: case RingType::ISOTHIAZOLE: case RingType::PYRAZOLE: case RingType::ISOSELENAZOLE: return {1, 2};
                            case RingType::PYRIMIDINE: case RingType::OXAZOLE: case RingType::THIAZOLE: case RingType::IMIDAZOLE: case RingType::SELENAZOLE: return {1, 3};
                            case RingType::PYRAZINE: return {1, 4};
                            default: return {};
                        }
                    };

                    auto compareRingSeniority = [&](int idx1, int idx2) -> int {
                        RingType t1 = typesN[idx1], t2 = typesN[idx2];
                        if (t1 != t2) {
                            int r1 = getRankHetero(t1), r2 = getRankHetero(t2);
                            if (r1 != r2) return (r1 < r2) ? -1 : 1;
                            int size1 = static_cast<int>(nodesN[idx1].size()), size2 = static_cast<int>(nodesN[idx2].size());
                            if (size1 != size2) return (size1 > size2) ? -1 : 1;
                            int nHet1 = getNumHetero(t1), nHet2 = getNumHetero(t2);
                            if (nHet1 != nHet2) return (nHet1 > nHet2) ? -1 : 1;
                            int v1 = getVariety(rHeteroN[idx1]), v2 = getVariety(rHeteroN[idx2]);
                            if (v1 != v2) return (v1 > v2) ? -1 : 1;
                            int alt1 = getTopAltRank(rHeteroN[idx1]), alt2 = getTopAltRank(rHeteroN[idx2]);
                            if (alt1 != alt2) return (alt1 < alt2) ? -1 : 1;
                            std::vector<int> own1 = getOwnLocants(t1), own2 = getOwnLocants(t2);
                            if (own1 != own2) return (own1 < own2) ? -1 : 1;
                        }
                        return 0;
                    };

                    auto getTopo = [&](int root) {
                        std::vector<int> depths(N, -1);
                        std::vector<int> q;
                        q.push_back(root);
                        depths[root] = 0;
                        int head = 0;
                        int maxD = 0;
                        std::vector<int> counts(N + 1, 0);
                        while (head < (int)q.size()) {
                            int u = q[head++];
                            for (int v = 0; v < N; ++v) {
                                if (shared[u][v] == 2 && depths[v] == -1) {
                                    depths[v] = depths[u] + 1;
                                    maxD = std::max(maxD, depths[v]);
                                    counts[depths[v]]++;
                                    q.push_back(v);
                                }
                            }
                        }
                        return std::make_pair(maxD, counts);
                    };

                    std::vector<std::pair<int, std::vector<int>>> allTopos(N);
                    std::vector<int> candidateRoots;
                    for (int i = 0; i < N; ++i) {
                        allTopos[i] = getTopo(i);
                        candidateRoots.push_back(i);
                    }

                    std::vector<int> bestRoots;
                    for (int root : candidateRoots) {
                        if (bestRoots.empty()) {
                            bestRoots.push_back(root);
                        } else {
                            int senCmp = compareRingSeniority(root, bestRoots[0]);
                            if (senCmp < 0) { // root is better in seniority
                                bestRoots.clear();
                                bestRoots.push_back(root);
                            } else if (senCmp == 0) { // tied in seniority, compare topology
                                const auto& topoR = allTopos[root];
                                const auto& topoB = allTopos[bestRoots[0]];
                                if (topoR.first < topoB.first) {
                                    bestRoots.clear();
                                    bestRoots.push_back(root);
                                } else if (topoR.first == topoB.first) {
                                    bool better = false;
                                    bool worse = false;
                                    for (size_t d = 1; d < topoR.second.size(); ++d) {
                                        if (topoR.second[d] > topoB.second[d]) { better = true; break; }
                                        if (topoR.second[d] < topoB.second[d]) { worse = true; break; }
                                    }
                                    if (better) {
                                        bestRoots.clear();
                                        bestRoots.push_back(root);
                                    } else if (!worse) {
                                        bestRoots.push_back(root);
                                    }
                                }
                            }
                        }
                    }

                    std::vector<std::vector<std::vector<int>>> allCands(N);
                    bool hasValidCands = true;
                    for (int i = 0; i < N; ++i) {
                        allCands[i] = getCandidates(typesN[i], rHeteroN[i], static_cast<int>(nodesN[i].size()), cyclesN[i]);
                        if (allCands[i].empty()) hasValidCands = false;
                    }

                    if (hasValidCands) {
                        struct Block {
                            QString text;
                            std::vector<char> letters;
                            std::vector<int> firstOrder;
                            std::map<int, std::vector<int>> lowerLocs;
                            std::map<int, std::vector<int>> higherLocs;
                            bool operator<(const Block& o) const { return text < o.text; }
                        };

                        auto formatLocants = [](std::pair<int, int> locs, int level) -> QString {
                            QString primes = "";
                            for (int i = 1; i < level; ++i) primes += "'";
                            return QString("%1%2,%3%4").arg(locs.first).arg(primes).arg(locs.second).arg(primes);
                        };

                        auto getFaceLocants = [](const std::vector<int>& num, int nA, int nB) -> std::tuple<int, int, int, int> {
                            int S = num.size();
                            int idxA = -1, idxB = -1;
                            for (int i = 0; i < S; ++i) {
                                if (num[i] == nA) idxA = i;
                                if (num[i] == nB) idxB = i;
                            }
                            if ((idxA + 1) % S == idxB) return {idxA + 1, idxB + 1, nA, nB};
                            if ((idxB + 1) % S == idxA) return {idxB + 1, idxA + 1, nB, nA};
                            return {-1, -1, -1, -1};
                        };

                        struct NameScore {
                            std::vector<char> parentLettersSet;
                            std::vector<char> parentLettersCitation;
                            std::vector<int> firstOrderLocantsSet;
                            std::vector<int> firstOrderLocantsCitation;
                            std::map<int, std::vector<int>> lowerLocsSet;
                            std::map<int, std::vector<int>> lowerLocsCitation;
                            std::map<int, std::vector<int>> higherLocsSet;
                            std::map<int, std::vector<int>> higherLocsCitation;
                            QString finalName;
                            
                            bool operator<(const NameScore& o) const {
                                if (parentLettersSet != o.parentLettersSet) return parentLettersSet < o.parentLettersSet;
                                if (parentLettersCitation != o.parentLettersCitation) return parentLettersCitation < o.parentLettersCitation;
                                if (firstOrderLocantsSet != o.firstOrderLocantsSet) return firstOrderLocantsSet < o.firstOrderLocantsSet;
                                if (firstOrderLocantsCitation != o.firstOrderLocantsCitation) return firstOrderLocantsCitation < o.firstOrderLocantsCitation;
                                for (auto it = lowerLocsSet.begin(); it != lowerLocsSet.end(); ++it) {
                                    int d = it->first;
                                    auto itO = o.lowerLocsSet.find(d);
                                    if (itO == o.lowerLocsSet.end()) return true;
                                    if (it->second != itO->second) return it->second < itO->second;
                                    
                                    const auto& c1 = lowerLocsCitation.at(d);
                                    const auto& c2 = o.lowerLocsCitation.at(d);
                                    if (c1 != c2) return c1 < c2;
                                    
                                    const auto& h1 = higherLocsSet.at(d);
                                    const auto& h2 = o.higherLocsSet.at(d);
                                    if (h1 != h2) return h1 < h2;
                                    
                                    const auto& hc1 = higherLocsCitation.at(d);
                                    const auto& hc2 = o.higherLocsCitation.at(d);
                                    if (hc1 != hc2) return hc1 < hc2;
                                }
                                if (finalName != o.finalName) {
                                    return finalName < o.finalName;
                                }
                                return false;
                            }
                        };

                        NameScore bestScore;
                        bool scoreInit = false;

                        for (int root : bestRoots) {
                            std::vector<int> depths(N, -1);
                            std::vector<int> q;
                            q.push_back(root);
                            depths[root] = 0;
                            int head = 0;
                            while (head < (int)q.size()) {
                                int u = q[head++];
                                for (int v = 0; v < N; ++v) {
                                    if (shared[u][v] == 2 && depths[v] == -1) {
                                        depths[v] = depths[u] + 1;
                                        q.push_back(v);
                                    }
                                }
                            }

                            std::vector<int> assign(N, 0);
                            bool done = false;
                            while (!done) {
                                std::vector<std::vector<int>> currentNum(N);
                                for (int i = 0; i < N; ++i) currentNum[i] = allCands[i][assign[i]];

                                std::function<std::pair<bool, Block>(int, int)> buildBlock = [&](int u, int p) -> std::pair<bool, Block> {
                                    std::vector<Block> cBlocks;
                                    for (int v = 0; v < N; ++v) {
                                        if (shared[u][v] == 2 && v != p) {
                                            auto res = buildBlock(v, u);
                                            if (!res.first) return {false, Block()};
                                            cBlocks.push_back(res.second);
                                        }
                                    }
                                    std::sort(cBlocks.begin(), cBlocks.end());
                                    
                                    Block b;
                                    for (const auto& cb : cBlocks) {
                                        b.text += cb.text;
                                        b.letters.insert(b.letters.end(), cb.letters.begin(), cb.letters.end());
                                        b.firstOrder.insert(b.firstOrder.end(), cb.firstOrder.begin(), cb.firstOrder.end());
                                        for (const auto& kv : cb.lowerLocs) {
                                            b.lowerLocs[kv.first].insert(b.lowerLocs[kv.first].end(), kv.second.begin(), kv.second.end());
                                        }
                                        for (const auto& kv : cb.higherLocs) {
                                            b.higherLocs[kv.first].insert(b.higherLocs[kv.first].end(), kv.second.begin(), kv.second.end());
                                        }
                                    }

                                    QString myPrefix;
                                    if (u == root) myPrefix = getBaseNameShared(typesN[u]);
                                    else myPrefix = getFusionPrefixShared(typesN[u]);

                                    if (u == root) {
                                        b.text += myPrefix;
                                        return {true, b};
                                    } else {
                                        int d = depths[u];
                                        int bhA = sharedNodesPairs[u][p][0];
                                        int bhB = sharedNodesPairs[u][p][1];
                                        
                                        auto tP = getFaceLocants(currentNum[p], bhA, bhB);
                                        int pL1 = std::get<0>(tP), pL2 = std::get<1>(tP), ns = std::get<2>(tP), ne = std::get<3>(tP);
                                        if (pL1 == -1) return {false, Block()};
                                        
                                        int S = currentNum[p].size();
                                        int uL1 = -1, uL2 = -1;
                                        for (int i = 0; i < (int)currentNum[u].size(); ++i) {
                                            if (currentNum[u][i] == ns) uL1 = i + 1;
                                            if (currentNum[u][i] == ne) uL2 = i + 1;
                                        }
                                        if (uL1 == -1 || uL2 == -1) return {false, Block()};

                                        if (d == 1) {
                                            char faceLetter;
                                            if (std::min(pL1, pL2) == 1 && std::max(pL1, pL2) == S) faceLetter = 'a' + S - 1;
                                            else faceLetter = 'a' + std::min(pL1, pL2) - 1;
                                            
                                            b.letters.push_back(faceLetter);
                                            b.firstOrder.push_back(uL1); b.firstOrder.push_back(uL2);
                                            b.text += myPrefix + QString("[%1-%2]").arg(formatLocants({uL1, uL2}, d)).arg(faceLetter);
                                        } else {
                                            b.lowerLocs[d].push_back(pL1); b.lowerLocs[d].push_back(pL2);
                                            b.higherLocs[d].push_back(uL1); b.higherLocs[d].push_back(uL2);
                                            b.text += myPrefix + QString("[%1:%2]").arg(formatLocants({uL1, uL2}, d)).arg(formatLocants({pL1, pL2}, d - 1));
                                        }
                                        return {true, b};
                                    }
                                };

                                auto res = buildBlock(root, -1);
                                if (res.first) {
                                    Block b = res.second;
                                    NameScore score;
                                    score.finalName = b.text;
                                    score.parentLettersCitation = b.letters;
                                    score.parentLettersSet = b.letters; std::sort(score.parentLettersSet.begin(), score.parentLettersSet.end());
                                    score.firstOrderLocantsCitation = b.firstOrder;
                                    score.firstOrderLocantsSet = b.firstOrder; std::sort(score.firstOrderLocantsSet.begin(), score.firstOrderLocantsSet.end());
                                    
                                    for (const auto& kv : b.lowerLocs) {
                                        score.lowerLocsCitation[kv.first] = kv.second;
                                        score.lowerLocsSet[kv.first] = kv.second; std::sort(score.lowerLocsSet[kv.first].begin(), score.lowerLocsSet[kv.first].end());
                                    }
                                    for (const auto& kv : b.higherLocs) {
                                        score.higherLocsCitation[kv.first] = kv.second;
                                        score.higherLocsSet[kv.first] = kv.second; std::sort(score.higherLocsSet[kv.first].begin(), score.higherLocsSet[kv.first].end());
                                    }

                                    if (!scoreInit || score < bestScore) {
                                        bestScore = score;
                                        scoreInit = true;
                                    }
                                }

                                for (int i = 0; i < N; ++i) {
                                    assign[i]++;
                                    if (assign[i] < (int)allCands[i].size()) break;
                                    assign[i] = 0;
                                    if (i == N - 1) done = true;
                                }
                            }
                        }

                        if (scoreInit) {
                            QString resultName = bestScore.finalName;
                            
                            // Indicated hydrogen for exactly 3 rings, per original Phase 44 logic
                            if (N == 3) {
                                auto isNHType = [](RingType t) {
                                    return t == RingType::PYRROLE || t == RingType::IMIDAZOLE || t == RingType::PYRAZOLE;
                                };
                                if (isNHType(typesN[0]) || isNHType(typesN[1]) || isNHType(typesN[2])) {
                                    int nhNode = -1;
                                    for (int i = 0; i < 3; ++i) {
                                        for (int n : nodesN[i]) {
                                            if (g.nodes[n].atomicNumber == 7 && g.nodes[n].totalH >= 1) {
                                                nhNode = n; break;
                                            }
                                        }
                                        if (nhNode != -1) break;
                                    }
                                    if (nhNode != -1) {
                                        std::set<int> bheads;
                                        for (int i = 0; i < 3; ++i) {
                                            for (int j = i + 1; j < 3; ++j) {
                                                if (shared[i][j] == 2) {
                                                    bheads.insert(sharedNodesPairs[i][j][0]);
                                                    bheads.insert(sharedNodesPairs[i][j][1]);
                                                }
                                            }
                                        }
                                        std::map<int, QString> periphMap = computePeripheralNumbering3Ring(g, nodesN[0], nodesN[1], nodesN[2], bheads, {}, true);
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
                            }
                            if (resultName.isEmpty()) {
                                return {false, "", "Failed to generate valid generic fusion nomenclature components (unsupported ring type)."};
                            }
                            return {true, resultName, ""};
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
        std::map<int, int> carbonHydroperoxide; // carbonNode -> near-oxygen node
        std::map<int, int> carbonImine; // carbonNode -> nitrogenNode

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            const GraphNode &node = g.nodes[i];
            if (node.atomicNumber == 6) {
                std::vector<int> doubleO, singleO, singleN, tripleN, halogens, doubleS, doubleN;

                for (size_t j = 0; j < node.neighbors.size(); ++j) {
                    int nei = node.neighbors[j];
                    int order = node.bondOrders[j];
                    int nZ = g.nodes[nei].atomicNumber;

                    if (nZ == 8 && order == 2) doubleO.push_back(nei);
                  else if (nZ == 16 && order == 2) doubleS.push_back(nei);
                  else if (nZ == 7 && order == 2) doubleN.push_back(nei);
                    else if (nZ == 8 && order == 1) singleO.push_back(nei);
                    else if (nZ == 7 && order == 1) {
                        bool isNitroIsoOrAzide = false;
                        if (carbonAzide.count(i) && std::find(carbonAzide[i].begin(), carbonAzide[i].end(), nei) != carbonAzide[i].end()) isNitroIsoOrAzide = true;
                        if (isIsocyanateNitrogen(nei, static_cast<int>(i), g)) isNitroIsoOrAzide = true;
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
                            if (!halogens.empty()) {
                                return {false, "", "Esters with a coexisting halogen on the acyl carbon (chloroformate-type structures) are not supported in this phase."};
                            }
                            int singleC = 0;
                            for (int nei : node.neighbors) {
                                if (g.nodes[nei].atomicNumber == 6) singleC++;
                            }
                            if (singleC == 0 && node.totalH == 0) {
                                return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                            }
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
                    if (hasOH) {
                        carbonGroup[i] = GroupType::ACID;
                    } else if (isPeroxyCarboxylicAcid(static_cast<int>(i), g)) {
                        return {false, "", "Peroxycarboxylic acids are not supported in this phase."};
                    }
                } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                    if (halogens.size() > 1) {
                        return {false, "", "Carbonic acid halides with multiple halogens are not supported in this phase."};
                    }
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    carbonGroup[i] = GroupType::ACYL_HALIDE;
                    acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
                } else if (!doubleO.empty() && !singleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0) {
                        return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                    }
                    if (!halogens.empty()) {
                        return {false, "", "Amides with coexisting halogens on the acyl carbon are not supported."};
                    }
                    bool isHydrazide = false;
                    for (int sN : singleN) {
                        for (size_t k = 0; k < g.nodes[sN].neighbors.size(); ++k) {
                            int nei = g.nodes[sN].neighbors[k];
                            if (nei != static_cast<int>(i) && g.nodes[nei].atomicNumber == 7 && g.nodes[sN].bondOrders[k] == 1) {
                                isHydrazide = true;
                                break;
                            }
                        }
                        if (isHydrazide) break;
                    }
                    if (isHydrazide) {
                        carbonGroup[i] = GroupType::HYDRAZIDE;
                    } else {
                        carbonGroup[i] = GroupType::AMIDE;
                    }
                } else if (!tripleN.empty()) {
                    int singleC = 0;
                    for (int nei : node.neighbors) {
                        if (g.nodes[nei].atomicNumber == 6) singleC++;
                    }
                    if (singleC == 0 && node.totalH == 0) {
                        return {false, "", "Cyanic acid halide derivatives (rootless nitrile carbons) are not supported in this phase."};
                    }
                    carbonGroup[i] = GroupType::NITRILE;
                } else if (!doubleO.empty()) {
                    if (isAcylPseudohalide(static_cast<int>(i), g, carbonAzide)) {
                        return {false, "", "Acyl pseudohalides are not supported in this phase."};
                    }
                    if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                        carbonGroup[i] = GroupType::ALDEHYDE;
                    } else {
                        carbonGroup[i] = GroupType::KETONE;
                    }
              } else if (!doubleS.empty()) {
                  int singleC = 0;
                  for (int nei : node.neighbors) {
                      if (g.nodes[nei].atomicNumber == 6) singleC++;
                  }
                  if (singleC == 0 && node.totalH == 0) {
                      return {false, "", "Carbonothioyl/thiocarbamoyl halide derivatives (rootless thiocarbonyl carbons) are not supported in this phase."};
                  }
                  if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                      carbonGroup[i] = GroupType::THIAL;
                  } else {
                      carbonGroup[i] = GroupType::THIONE;
                  }
              } else if (!doubleN.empty()) {
                  int singleC = 0;
                  for (int nei : node.neighbors) {
                      if (g.nodes[nei].atomicNumber == 6) singleC++;
                  }
                  if (singleC == 0 && node.totalH == 0) {
                      return {false, "", "Carbonimidic/carbamimidic acid halide derivatives (rootless imine carbons) are not supported in this phase."};
                  }
                  int nNode = doubleN[0];
                  if (doubleN.size() == 1 && (g.nodes[nNode].totalH >= 1 || g.nodes[nNode].neighbors.size() == 1)) {
                      carbonImine[i] = nNode;
                      carbonGroup[i] = GroupType::IMINE;
                  } else {
                      return {false, "", "N-substituted imines, oximes, hydrazones, and amidines are not supported in this phase; only the unsubstituted C=NH imine is supported."};
                  }
                } else if (!singleO.empty()) {
                    bool foundGroup = false;
                    for (int sO : singleO) {
                        if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                            carbonGroup[i] = GroupType::ALCOHOL; foundGroup = true; break;
                        }
                    }
                    if (!foundGroup) {
                        for (int sO : singleO) {
                            if (g.nodes[sO].neighbors.size() == 2) {
                                for (int oNei : g.nodes[sO].neighbors) {
                                    if (oNei != static_cast<int>(i) && g.nodes[oNei].atomicNumber == 8) {
                                        // Check if this is a dialkyl peroxide (R-O-O-R')
                                        bool isFarOHydroperoxide = (g.nodes[oNei].totalH >= 1 || g.nodes[oNei].neighbors.size() == 1);
                                        bool isFarODialkyl = false;
                                        for (int farNei : g.nodes[oNei].neighbors) {
                                            if (farNei != sO && g.nodes[farNei].atomicNumber == 6) {
                                                isFarODialkyl = true;
                                                break;
                                            }
                                        }
                                        if (isFarOHydroperoxide) {
                                            carbonHydroperoxide[i] = sO; // carbonNode -> near-oxygen node
                                            carbonGroup[i] = GroupType::HYDROPEROXIDE;
                                            foundGroup = true;
                                            break;
                                        } else if (isFarODialkyl) {
                                            return {false, "", "Dialkyl peroxides are not supported in this phase."};
                                        }
                                    }
                                }
                            }
                            if (foundGroup) break;
                        }
                    }
                } else if (singleN.size() > 0) {
                    carbonGroup[i] = GroupType::AMINE;
                } else if (carbonSulfonicAcid.count(i) ) {
                    carbonGroup[i] = GroupType::SULFONIC_ACID;
                } else if (carbonSulfonylHalide.count(i) ) {
                    carbonGroup[i] = GroupType::SULFONYL_HALIDE;
                } else if (carbonThiol.count(i)) {
                    carbonGroup[i] = GroupType::THIOL;
                } else if (carbonSelenol.count(i)) {
                    carbonGroup[i] = GroupType::SELENOL;
                } else if (carbonTellurol.count(i)) {
                    carbonGroup[i] = GroupType::TELLUROL;
                } else if (carbonBoronicAcid.count(i)) {
                    carbonGroup[i] = GroupType::BORONIC_ACID;
                } else if (carbonPhosphonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::PHOSPHONIC_ACID;
                } else if (carbonPhosphonicDihalide.count(i)) {
                    carbonGroup[i] = GroupType::PHOSPHONIC_DIHALIDE;
                } else if (carbonArsonicAcid.count(i)) {
                    carbonGroup[i] = GroupType::ARSONIC_ACID;
                } else if (carbonHydroperoxide.count(i)) {
                    carbonGroup[i] = GroupType::HYDROPEROXIDE;
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

        auto groupRank = [](GroupType gt) -> int {
            switch (gt) {
                case GroupType::SULFONIC_ACID: return 1;
                case GroupType::SULFINIC_ACID: return 2;
                case GroupType::ACID: return 3;
                case GroupType::PHOSPHONIC_ACID: return 4;
                case GroupType::PHOSPHONIC_DIHALIDE: return 5;
                case GroupType::ARSONIC_ACID: return 6;
                case GroupType::BORONIC_ACID: return 7;
                case GroupType::ESTER: return 8;
                case GroupType::ACYL_HALIDE: return 9;
                case GroupType::SULFONYL_HALIDE: return 10;
                case GroupType::AMIDE: return 11;
                case GroupType::HYDRAZIDE: return 12;
                case GroupType::NITRILE: return 13;
                case GroupType::ALDEHYDE: return 14;
                case GroupType::THIAL: return 15;
                case GroupType::KETONE: return 16;
                case GroupType::THIONE: return 17;
                case GroupType::ALCOHOL: return 18;
                case GroupType::THIOL: return 19;
                case GroupType::SELENOL: return 20;
                case GroupType::TELLUROL: return 21;
                case GroupType::HYDROPEROXIDE: return 22;
                case GroupType::AMINE: return 23;
                case GroupType::IMINE: return 24;
                case GroupType::PHOSPHINE: return 25;
                default: return 26;
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
                    if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                }
            }
            if (isOnOrExocyclic) {
                if (groupRank(gt) < groupRank(winningRingGroup)) winningRingGroup = gt;
            } else {
                if (groupRank(gt) < groupRank(winningChainGroup)) winningChainGroup = gt;
            }
        }

        // Phase 52 (P-44.1.1 + P-44.1.2.2) / P-44.1.2: the principal characteristic group is
        // the single most-senior class across ring-attached and chain-attached instances
        // combined; among structures that genuinely bear an instance of that winning class,
        // the senior parent is chosen first by skeletal-atom seniority (P-44.1.2: a ring
        // containing any heteroatom outright beats a plain-carbon chain -- this codebase's
        // chains are always plain-carbon, so any ring heteroatom decides it), falling back to
        // instance count (P-44.1.1) with the ring winning ties (P-44.1.2.2) only when the ring
        // and chain tie on skeletal-atom seniority (both plain carbon, today's only other case).
        // P-44.1.2 only applies as a competition between candidates that can actually bear the
        // winning group as a suffix -- if the ring has ZERO instances of combinedWinner, it is
        // not a real candidate regardless of heteroatom seniority, and instance count alone
        // (which will correctly favour the chain) must decide, or the principal group would be
        // stranded off the chosen parent with no way to cite it as the required suffix.
        GroupType combinedWinner = (groupRank(winningRingGroup) <= groupRank(winningChainGroup)) ? winningRingGroup : winningChainGroup;
        if (combinedWinner != GroupType::NONE) {
            int ringCount = 0, chainCount = 0, chainDeepCount = 0;
            for (const auto &pair : carbonGroup) {
                if (pair.second != combinedWinner) continue;
                int cNode = pair.first;
                bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
                if (!isOnOrExocyclic) {
                    for (int nei : g.nodes[cNode].neighbors) {
                        if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                    }
                    if (!isOnOrExocyclic) ++chainDeepCount;
                }
                if (isOnOrExocyclic) ++ringCount; else ++chainCount;
            }
            bool ringHasHeteroatom = false;
            if (ringCount > 0) {
                for (int n : ringNodeSet) {
                    if (g.nodes[n].atomicNumber != 6) { ringHasHeteroatom = true; break; }
                }
            }
            if (!ringHasHeteroatom && (chainCount > ringCount || (chainCount == ringCount && chainDeepCount > 0))) {
                // Chain is the senior parent structure. Name it as parent with the ring cited
                // as a substituent prefix. Phase 54: this is no longer acid-only -- the
                // supported classes (ACID, AMIDE, NITRILE, ALDEHYDE, KETONE, ALCOHOL, THIOL,
                // AMINE, THIAL, THIONE, SULFONIC_ACID, ESTER, ACYL_HALIDE) are gated by isChainParentWithRingSubstituentSupported so both
                // call sites stay in lockstep; unsupported classes still reject cleanly.
                // Phase 61: BORONIC_ACID and PHOSPHINE are handled separately here because they
                // name the alkyl chain differently (alkyl + "boronic acid"/"phosphine" suffix).
                if (combinedWinner == GroupType::BORONIC_ACID || combinedWinner == GroupType::PHOSPHINE) {
                    int pOrBCarbon = -1;
                    for (const auto &pair : carbonGroup) {
                        if (pair.second == combinedWinner) {
                            bool isOnOrExocyclic = ringNodeSet.count(pair.first) > 0;
                            if (!isOnOrExocyclic) {
                                for (int nei : g.nodes[pair.first].neighbors) {
                                    if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                                }
                            }
                            if (!isOnOrExocyclic) {
                                pOrBCarbon = pair.first;
                                break;
                            }
                        }
                    }
                    if (pOrBCarbon != -1) {
                        int pOrBNode = (combinedWinner == GroupType::PHOSPHINE) ? carbonPhosphine[pOrBCarbon] : carbonBoronicAcid[pOrBCarbon];
                        QString alkylName = nameBranchGraph(g, pOrBCarbon, pOrBNode, allSSSRRings, ringNodeSet);
                        if (alkylName.isEmpty()) {
                            return {false, "", "Unsupported boronic acid/phosphine alkyl group."};
                        }
                        if (alkylName.startsWith("(") && alkylName.endsWith(")")) {
                            alkylName = alkylName.mid(1, alkylName.length() - 2);
                        }
                        QString fullName = alkylName + ((combinedWinner == GroupType::PHOSPHINE) ? "phosphine" : "boronic acid");
                        return {true, fullName, ""};
                    }
                } else if (isChainParentWithRingSubstituentSupported(combinedWinner)) {
                    std::set<int> handledBranchStereoIds;
                    QString chainName = nameChainParentWithRingSubstituent(mol, indigoToGraphIdx, g, ringNodeSet, allSSSRRings, carbonGroup, combinedWinner, stereoByGraphId, &handledBranchStereoIds);
                    if (!chainName.isEmpty()) return {true, chainName, ""};
                }
                return {false, "", "A chain-based principal group outranks the ring in this structure; chain-as-parent seniority (P-44.1.1) for this ring/class combination is not yet supported."};
            }
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
            std::set<int> handledBranchStereoIds;
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
                        if ((winningType == GroupType::ALCOHOL || winningType == GroupType::KETONE || winningType == GroupType::AMINE || winningType == GroupType::IMINE || winningType == GroupType::SULFONIC_ACID || winningType == GroupType::SULFONYL_HALIDE || winningType == GroupType::SULFINIC_ACID || winningType == GroupType::THIOL || winningType == GroupType::SELENOL || winningType == GroupType::TELLUROL || winningType == GroupType::HYDROPEROXIDE || winningType == GroupType::PHOSPHONIC_ACID || winningType == GroupType::PHOSPHONIC_DIHALIDE || winningType == GroupType::ARSONIC_ACID) && isPrincipalRNode) {
                            int nz = g.nodes[nei].atomicNumber;
                            bool isAzide = (carbonAzide.count(rNode) && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end());
                            if (!isAzide && (nz == 7 || nz == 8 || nz == 16 || nz == 34 || nz == 52 || nz == 15 || nz == 33)) continue;
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
                        } else if (!isAzide && order == 2 && carbonImine.count(rNode) && carbonImine[rNode] == nei && winningType != GroupType::IMINE) {
                            subName = "imino";
                        }
                    } else if (nz == 16) {
                        if (carbonSulfonicAcid.count(rNode) && carbonSulfonicAcid[rNode] == nei && winningType != GroupType::SULFONIC_ACID) {
                            subName = "sulfo";
                        } else if (carbonSulfonylHalide.count(rNode) && carbonSulfonylHalide[rNode] == nei && winningType != GroupType::SULFONYL_HALIDE) {
                            subName = halogenPrefix(sulfonylHalideZ[rNode]) + "sulfonyl";
                        } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                            subName = "sulfino";
                        } else if (carbonHydroperoxide.count(rNode) && carbonHydroperoxide[rNode] == nei && winningType != GroupType::HYDROPEROXIDE) {
                            subName = "hydroperoxy";
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
                    } else if (nz == 34) {
                        if (carbonSelenol.count(rNode) && carbonSelenol[rNode] == nei && winningType != GroupType::SELENOL) {
                            subName = "selanyl";
                        } else if (order == 1) {
                            if (selenoetherSeleniums.count(nei)) {
                                int alkylNei = -1;
                                for (int sNei : g.nodes[nei].neighbors) {
                                    if (sNei != rNode) { alkylNei = sNei; break; }
                                }
                                if (alkylNei != -1) {
                                    QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                    subName = alkylName + "selanyl";
                                }
                            }
                        }
                    } else if (nz == 52) {
                        if (carbonTellurol.count(rNode) && carbonTellurol[rNode] == nei && winningType != GroupType::TELLUROL) {
                            subName = "tellanyl";
                        } else if (order == 1) {
                            if (telluroetherTelluriums.count(nei)) {
                                int alkylNei = -1;
                                for (int sNei : g.nodes[nei].neighbors) {
                                    if (sNei != rNode) { alkylNei = sNei; break; }
                                }
                                if (alkylNei != -1) {
                                    QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                    subName = alkylName + "tellanyl";
                                }
                            }
                        }
                    } else if (nz == 15) {
                        if (carbonPhosphonicAcid.count(rNode) && carbonPhosphonicAcid[rNode] == nei && winningType != GroupType::PHOSPHONIC_ACID) {
                            subName = "phosphono";
                        } else if (carbonPhosphonicDihalide.count(rNode) && carbonPhosphonicDihalide[rNode] == nei && winningType != GroupType::PHOSPHONIC_DIHALIDE) {
                            subName = "di" + halogenPrefix(phosphonicDihalideZ[rNode]) + "phosphoryl";
                        }
                    } else if (nz == 33) {
                        if (carbonArsonicAcid.count(rNode) && carbonArsonicAcid[rNode] == nei && winningType != GroupType::ARSONIC_ACID) {
                            subName = "arsono";
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
                            if (!subName.isEmpty()) {
                                QString branchStereo = formatBranchStereoPrefix(g, nei, rNode, stereoByGraphId, sig.handledBranchStereoIds);
                                if (!branchStereo.isEmpty()) {
                                    if (subName.startsWith("(") && subName.endsWith(")")) {
                                        QString inner = subName.mid(1, subName.length() - 2);
                                        subName = QString("[%1%2]").arg(branchStereo, inner);
                                    } else {
                                        subName = QString("[%1%2]").arg(branchStereo, subName);
                                    }
                                } else if (subName.startsWith("(")) {
                                    subName = subName.mid(1);
                                    if (subName.endsWith(")")) subName.chop(1);
                                }
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
                        [](const auto &x, const auto &y) { return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
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
        StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, graphIdToLocant, bestSig.handledBranchStereoIds);
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
            pg.baseName = alphabetizationKey(pName);
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
            bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE || winningType == GroupType::HYDRAZIDE ||
                                winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||
                                winningType == GroupType::ACYL_HALIDE || winningType == GroupType::ESTER);
            int pCount = static_cast<int>(bestSig.principalLocants.size());

            if (isExocyclic) {
                QString sfx;
                if (winningType == GroupType::ACID) sfx = (pCount == 1) ? "carboxylic acid" : "dicarboxylic acid";
                else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";
                else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? "carbohydrazide" : "dicarbohydrazide";
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
                else if (winningType == GroupType::SULFONYL_HALIDE) {
                    QString hName;
                    int hz = sulfonylHalideZ.empty() ? 17 : sulfonylHalideZ.begin()->second;
                    for (int pc : principalCarbons) {
                        if (sulfonylHalideZ.count(pc)) { hz = sulfonylHalideZ[pc]; break; }
                    }
                    hName = halogenSuffixWord(hz);
                    sfx = (pCount == 1) ? ("sulfonyl " + hName) : ("disulfonyl " + hName);
                }
                else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";
                else if (winningType == GroupType::PHOSPHONIC_DIHALIDE) {
                QString hName;
                int hz = phosphonicDihalideZ.empty() ? 17 : phosphonicDihalideZ.begin()->second;
                for (int pc : principalCarbons) {
                    if (phosphonicDihalideZ.count(pc)) { hz = phosphonicDihalideZ[pc]; break; }
                }
                hName = "di" + halogenSuffixWord(hz);
                sfx = (pCount == 1) ? ("phosphonic " + hName) : ("diphosphonic " + hName);
            }
            else if (winningType == GroupType::PHOSPHONIC_ACID) sfx = (pCount == 1) ? "phosphonic acid" : "diphosphonic acid";
                else if (winningType == GroupType::ARSONIC_ACID) sfx = (pCount == 1) ? "arsonic acid" : "diarsonic acid";
                else if (winningType == GroupType::THIOL) sfx = (pCount == 1) ? "thiol" : "dithiol";
                else if (winningType == GroupType::SELENOL) sfx = (pCount == 1) ? "selenol" : "diselenol";
                else if (winningType == GroupType::TELLUROL) sfx = (pCount == 1) ? "tellurol" : "ditellurol";
                else if (winningType == GroupType::ALCOHOL) sfx = (pCount == 1) ? "ol" : "diol";
                else if (winningType == GroupType::HYDROPEROXIDE) sfx = (pCount == 1) ? "peroxol" : "diperoxol";
                else if (winningType == GroupType::KETONE) sfx = (pCount == 1) ? "one" : "dione";
                else if (winningType == GroupType::AMINE) sfx = (pCount == 1) ? "amine" : "diamine";
                else if (winningType == GroupType::IMINE) sfx = (pCount == 1) ? "imine" : "diimine";

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
        if (!classifyMonocyclicHeteroRing(g, ringHeteroNodes, ringSize, ringCycle, rType, parentNameRoot, classErr, true)) {
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
    std::map<int, int> carbonHydroperoxide; // carbonNode -> near-oxygen node
    std::map<int, int> carbonImine; // carbonNode -> nitrogenNode

    for (size_t i = 0; i < g.nodes.size(); ++i) {
        const GraphNode &node = g.nodes[i];
        if (node.atomicNumber == 6) {
            std::vector<int> doubleO, singleO, singleN, tripleN, halogens, doubleS, doubleN;

            for (size_t j = 0; j < node.neighbors.size(); ++j) {
                int nei = node.neighbors[j];
                int order = node.bondOrders[j];
                int nZ = g.nodes[nei].atomicNumber;

                if (nZ == 8 && order == 2) doubleO.push_back(nei);
                  else if (nZ == 16 && order == 2) doubleS.push_back(nei);
                  else if (nZ == 7 && order == 2) doubleN.push_back(nei);
                else if (nZ == 8 && order == 1) {
                    bool isRingBond = ringNodeSet.count(static_cast<int>(i)) && ringNodeSet.count(nei);
                    if (!isRingBond) singleO.push_back(nei);
                }
                else if (nZ == 7 && order == 1) {
                    bool isNitroIsoOrAzide = false;
                    if (carbonAzide.count(i) && std::find(carbonAzide[i].begin(), carbonAzide[i].end(), nei) != carbonAzide[i].end()) isNitroIsoOrAzide = true;
                    if (isIsocyanateNitrogen(nei, static_cast<int>(i), g)) isNitroIsoOrAzide = true;
                    // Phase 70: a ring-internal N-C bond (both atoms in ringNodeSet) is
                    // the ring itself, not an exocyclic amine substituent -- previously
                    // unreachable because every saturated-heterocycle-as-parent case was
                    // rejected before this scan could ever see one; LARGE_HETEROCYCLE's
                    // saturated large heteromonocycles are the first to reach it.
                    bool isRingBond = ringNodeSet.count(static_cast<int>(i)) && ringNodeSet.count(nei);
                    if (!isNitroIsoOrAzide && !isRingBond) singleN.push_back(nei);
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
                        if (!halogens.empty()) {
                            return {false, "", "Esters with a coexisting halogen on the acyl carbon (chloroformate-type structures) are not supported in this phase."};
                        }
                        int singleC = 0;
                        for (int nei : node.neighbors) {
                            if (g.nodes[nei].atomicNumber == 6) singleC++;
                        }
                        if (singleC == 0 && node.totalH == 0) {
                            return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                        }
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
                if (hasOH) {
                    carbonGroup[i] = GroupType::ACID;
                } else if (isPeroxyCarboxylicAcid(static_cast<int>(i), g)) {
                    return {false, "", "Peroxycarboxylic acids are not supported in this phase."};
                }
            } else if (!doubleO.empty() && !halogens.empty() && singleO.empty() && singleN.empty()) {
                if (halogens.size() > 1) {
                    return {false, "", "Carbonic acid halides with multiple halogens are not supported in this phase."};
                }
                int singleC = 0;
                for (int nei : node.neighbors) {
                    if (g.nodes[nei].atomicNumber == 6) singleC++;
                }
                if (singleC == 0) {
                    return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                }
                carbonGroup[i] = GroupType::ACYL_HALIDE;
                acylHalideHalogen[i] = g.nodes[halogens[0]].atomicNumber;
            } else if (!doubleO.empty() && !singleN.empty()) {
                int singleC = 0;
                for (int nei : node.neighbors) {
                    if (g.nodes[nei].atomicNumber == 6) singleC++;
                }
                if (singleC == 0) {
                    return {false, "", "Carbonic/carbamic acid derivatives (rootless acyl carbons) are not supported in this phase."};
                }
                if (!halogens.empty()) {
                    return {false, "", "Amides with coexisting halogens on the acyl carbon are not supported."};
                }
                bool isHydrazide = false;
                for (int sN : singleN) {
                    for (size_t k = 0; k < g.nodes[sN].neighbors.size(); ++k) {
                        int nei = g.nodes[sN].neighbors[k];
                        if (nei != static_cast<int>(i) && g.nodes[nei].atomicNumber == 7 && g.nodes[sN].bondOrders[k] == 1) {
                            isHydrazide = true;
                            break;
                        }
                    }
                    if (isHydrazide) break;
                }
                if (isHydrazide) {
                    carbonGroup[i] = GroupType::HYDRAZIDE;
                } else {
                    carbonGroup[i] = GroupType::AMIDE;
                }
            } else if (!tripleN.empty()) {
                int singleC = 0;
                for (int nei : node.neighbors) {
                    if (g.nodes[nei].atomicNumber == 6) singleC++;
                }
                if (singleC == 0 && node.totalH == 0) {
                    return {false, "", "Cyanic acid halide derivatives (rootless nitrile carbons) are not supported in this phase."};
                }
                carbonGroup[i] = GroupType::NITRILE;
            } else if (!doubleO.empty()) {
                if (isAcylPseudohalide(static_cast<int>(i), g, carbonAzide)) {
                    return {false, "", "Acyl pseudohalides are not supported in this phase."};
                }
                if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                    carbonGroup[i] = GroupType::ALDEHYDE;
                } else {
                    carbonGroup[i] = GroupType::KETONE;
                }
              } else if (!doubleS.empty()) {
                  int singleC = 0;
                  for (int nei : node.neighbors) {
                      if (g.nodes[nei].atomicNumber == 6) singleC++;
                  }
                  if (singleC == 0 && node.totalH == 0) {
                      return {false, "", "Carbonothioyl/thiocarbamoyl halide derivatives (rootless thiocarbonyl carbons) are not supported in this phase."};
                  }
                  if (node.totalH >= 1 || node.neighbors.size() <= 2) {
                      carbonGroup[i] = GroupType::THIAL;
                  } else {
                      carbonGroup[i] = GroupType::THIONE;
                  }
              } else if (!doubleN.empty()) {
                  int singleC = 0;
                  for (int nei : node.neighbors) {
                      if (g.nodes[nei].atomicNumber == 6) singleC++;
                  }
                  if (singleC == 0 && node.totalH == 0) {
                      return {false, "", "Carbonimidic/carbamimidic acid halide derivatives (rootless imine carbons) are not supported in this phase."};
                  }
                  int nNode = doubleN[0];
                  if (doubleN.size() == 1 && (g.nodes[nNode].totalH >= 1 || g.nodes[nNode].neighbors.size() == 1)) {
                      carbonImine[i] = nNode;
                      carbonGroup[i] = GroupType::IMINE;
                  } else {
                      return {false, "", "N-substituted imines, oximes, hydrazones, and amidines are not supported in this phase; only the unsubstituted C=NH imine is supported."};
                  }
            } else if (!singleO.empty()) {
                bool foundGroup = false;
                for (int sO : singleO) {
                    if (g.nodes[sO].totalH >= 1 || g.nodes[sO].neighbors.size() == 1) {
                        carbonGroup[i] = GroupType::ALCOHOL; foundGroup = true; break;
                    }
                }
                if (!foundGroup) {
                    for (int sO : singleO) {
                        if (g.nodes[sO].neighbors.size() == 2) {
                            for (int oNei : g.nodes[sO].neighbors) {
                                if (oNei != static_cast<int>(i) && g.nodes[oNei].atomicNumber == 8) {
                                    // Check if this is a dialkyl peroxide (R-O-O-R')
                                    bool isFarOHydroperoxide = (g.nodes[oNei].totalH >= 1 || g.nodes[oNei].neighbors.size() == 1);
                                    bool isFarODialkyl = false;
                                    for (int farNei : g.nodes[oNei].neighbors) {
                                        if (farNei != sO && g.nodes[farNei].atomicNumber == 6) {
                                            isFarODialkyl = true;
                                            break;
                                        }
                                    }
                                    if (isFarOHydroperoxide) {
                                        carbonHydroperoxide[i] = sO; // carbonNode -> near-oxygen node
                                        carbonGroup[i] = GroupType::HYDROPEROXIDE;
                                        foundGroup = true;
                                        break;
                                    } else if (isFarODialkyl) {
                                        return {false, "", "Dialkyl peroxides are not supported in this phase."};
                                    }
                                }
                            }
                        }
                        if (foundGroup) break;
                    }
                }
            } else if (!singleN.empty()) {
                carbonGroup[i] = GroupType::AMINE;
            } else if (carbonSulfonicAcid.count(i) ) {
                carbonGroup[i] = GroupType::SULFONIC_ACID;
            } else if (carbonSulfonylHalide.count(i) ) {
                carbonGroup[i] = GroupType::SULFONYL_HALIDE;
            } else if (carbonSulfinicAcid.count(i) ) {
                carbonGroup[i] = GroupType::SULFINIC_ACID;
            } else if (carbonThiol.count(i)) {
                carbonGroup[i] = GroupType::THIOL;
            } else if (carbonSelenol.count(i)) {
                carbonGroup[i] = GroupType::SELENOL;
            } else if (carbonTellurol.count(i)) {
                carbonGroup[i] = GroupType::TELLUROL;
            } else if (carbonBoronicAcid.count(i)) {
                carbonGroup[i] = GroupType::BORONIC_ACID;
            } else if (carbonPhosphonicAcid.count(i)) {
                carbonGroup[i] = GroupType::PHOSPHONIC_ACID;
            } else if (carbonPhosphonicDihalide.count(i)) {
                carbonGroup[i] = GroupType::PHOSPHONIC_DIHALIDE;
            } else if (carbonArsonicAcid.count(i)) {
                carbonGroup[i] = GroupType::ARSONIC_ACID;
            } else if (carbonHydroperoxide.count(i)) {
                carbonGroup[i] = GroupType::HYDROPEROXIDE;
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

    auto groupRank = [](GroupType gt) -> int {
        switch (gt) {
            case GroupType::SULFONIC_ACID: return 1;
            case GroupType::SULFINIC_ACID: return 2;
            case GroupType::ACID: return 3;
            case GroupType::PHOSPHONIC_ACID: return 4;
            case GroupType::PHOSPHONIC_DIHALIDE: return 5;
            case GroupType::ARSONIC_ACID: return 6;
            case GroupType::BORONIC_ACID: return 7;
            case GroupType::ESTER: return 8;
            case GroupType::ACYL_HALIDE: return 9;
            case GroupType::SULFONYL_HALIDE: return 10;
            case GroupType::AMIDE: return 11;
            case GroupType::HYDRAZIDE: return 12;
            case GroupType::NITRILE: return 13;
            case GroupType::ALDEHYDE: return 14;
            case GroupType::THIAL: return 15;
            case GroupType::KETONE: return 16;
            case GroupType::THIONE: return 17;
            case GroupType::ALCOHOL: return 18;
            case GroupType::THIOL: return 19;
            case GroupType::SELENOL: return 20;
            case GroupType::TELLUROL: return 21;
            case GroupType::HYDROPEROXIDE: return 22;
            case GroupType::AMINE: return 23;
            case GroupType::IMINE: return 24;
            case GroupType::PHOSPHINE: return 25;
            default: return 26;
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
                if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
            }
        }
        if (isOnOrExocyclic) {
            if (groupRank(gt) < groupRank(winningRingGroup)) winningRingGroup = gt;
        } else {
            if (groupRank(gt) < groupRank(winningChainGroup)) winningChainGroup = gt;
        }
    }

    // Phase 52 (P-44.1.1 + P-44.1.2.2) / P-44.1.2: the principal characteristic group is
    // the single most-senior class across ring-attached and chain-attached instances
    // combined; among structures that genuinely bear an instance of that winning class,
    // the senior parent is chosen first by skeletal-atom seniority (P-44.1.2: a ring
    // containing any heteroatom outright beats a plain-carbon chain -- this codebase's
    // chains are always plain-carbon, so any ring heteroatom decides it), falling back to
    // instance count (P-44.1.1) with the ring winning ties (P-44.1.2.2) only when the ring
    // and chain tie on skeletal-atom seniority (both plain carbon, today's only other case).
    // P-44.1.2 only applies as a competition between candidates that can actually bear the
    // winning group as a suffix -- if the ring has ZERO instances of combinedWinner, it is
    // not a real candidate regardless of heteroatom seniority, and instance count alone
    // (which will correctly favour the chain) must decide, or the principal group would be
    // stranded off the chosen parent with no way to cite it as the required suffix.
    GroupType combinedWinner = (groupRank(winningRingGroup) <= groupRank(winningChainGroup)) ? winningRingGroup : winningChainGroup;
    if (combinedWinner != GroupType::NONE) {
        int ringCount = 0, chainCount = 0, chainDeepCount = 0;
        for (const auto &pair : carbonGroup) {
            if (pair.second != combinedWinner) continue;
            int cNode = pair.first;
            bool isOnOrExocyclic = ringNodeSet.count(cNode) > 0;
            if (!isOnOrExocyclic) {
                for (int nei : g.nodes[cNode].neighbors) {
                    if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                }
                if (!isOnOrExocyclic) ++chainDeepCount;
            }
            if (isOnOrExocyclic) ++ringCount; else ++chainCount;
        }
        bool ringHasHeteroatom = false;
        if (ringCount > 0) {
            for (int n : ringNodeSet) {
                if (g.nodes[n].atomicNumber != 6) { ringHasHeteroatom = true; break; }
            }
        }
        if (!ringHasHeteroatom && (chainCount > ringCount || (chainCount == ringCount && chainDeepCount > 0))) {
            // Chain is the senior parent structure. Name it as parent with the ring cited
            // as a substituent prefix. Phase 54: this is no longer acid-only -- the
            // supported classes (ACID, AMIDE, NITRILE, ALDEHYDE, KETONE, ALCOHOL, THIOL,
            // AMINE, THIAL, THIONE, SULFONIC_ACID, ESTER, ACYL_HALIDE) are gated by isChainParentWithRingSubstituentSupported so both
            // call sites stay in lockstep; unsupported classes still reject cleanly.
            // Phase 61: BORONIC_ACID and PHOSPHINE are handled separately here because they
            // name the alkyl chain differently (alkyl + "boronic acid"/"phosphine" suffix).
            if (combinedWinner == GroupType::BORONIC_ACID || combinedWinner == GroupType::PHOSPHINE) {
                int pOrBCarbon = -1;
                for (const auto &pair : carbonGroup) {
                    if (pair.second == combinedWinner) {
                        bool isOnOrExocyclic = ringNodeSet.count(pair.first) > 0;
                        if (!isOnOrExocyclic) {
                            for (int nei : g.nodes[pair.first].neighbors) {
                                if (ringNodeSet.count(nei) > 0) { isOnOrExocyclic = true; break; }
                            }
                        }
                        if (!isOnOrExocyclic) {
                            pOrBCarbon = pair.first;
                            break;
                        }
                    }
                }
                if (pOrBCarbon != -1) {
                    int pOrBNode = (combinedWinner == GroupType::PHOSPHINE) ? carbonPhosphine[pOrBCarbon] : carbonBoronicAcid[pOrBCarbon];
                    QString alkylName = nameBranchGraph(g, pOrBCarbon, pOrBNode, allSSSRRings, ringNodeSet);
                    if (alkylName.isEmpty()) {
                        return {false, "", "Unsupported boronic acid/phosphine alkyl group."};
                    }
                    if (alkylName.startsWith("(") && alkylName.endsWith(")")) {
                        alkylName = alkylName.mid(1, alkylName.length() - 2);
                    }
                    QString fullName = alkylName + ((combinedWinner == GroupType::PHOSPHINE) ? "phosphine" : "boronic acid");
                    return {true, fullName, ""};
                }
            } else if (isChainParentWithRingSubstituentSupported(combinedWinner)) {
                std::set<int> handledBranchStereoIds;
                QString chainName = nameChainParentWithRingSubstituent(mol, indigoToGraphIdx, g, ringNodeSet, allSSSRRings, carbonGroup, combinedWinner, stereoByGraphId, &handledBranchStereoIds);
                if (!chainName.isEmpty()) return {true, chainName, ""};
            }
            return {false, "", "A chain-based principal group outranks the ring in this structure; chain-as-parent seniority (P-44.1.1) for this ring/class combination is not yet supported."};
        }
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
    if (rType == RingType::FURAN || rType == RingType::THIOPHENE || rType == RingType::PYRROLE || rType == RingType::PYRIDINE ||
        rType == RingType::PYRROLIDINE || rType == RingType::PIPERIDINE || rType == RingType::TETRAHYDROFURAN || rType == RingType::TETRAHYDROTHIOPHENE) {
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
        // P-22.2.2.1.3: seed candidates from positions of the most-senior heteroatom
        // (lowest hwSeniorityRank value). All such positions as both fwd and bwd.
        int minRank = 99;
        for (int nodeIdx : ringHeteroNodes) {
            int z = g.nodes[nodeIdx].atomicNumber;
            int r = hwSeniorityRank(z);
            if (r < minRank) minRank = r;
        }

        for (int st = 0; st < ringSize; ++st) {
            int nodeIdx = ringCycle[st];
            int z = g.nodes[nodeIdx].atomicNumber;
            int r = hwSeniorityRank(z);
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
        std::set<int> handledBranchStereoIds;
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
                // P-22.2.2.1.3: use full citation-order seniority rank
                sig.heteroatomSeniorityAtLocants.push_back(hwSeniorityRank(z));
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
        } else if (rType == RingType::LARGE_HETEROCYCLE) {
            bool hasDoubleOrArom = false;
            for (const auto &rb : ringBonds) {
                if (rb.order == 2 || rb.order == 4) hasDoubleOrArom = true;
            }
            if (hasDoubleOrArom) {
                std::vector<int> spareValence(ringSize);
                int zeroSpareCount = 0;
                for (int i = 0; i < ringSize; ++i) {
                    int z = g.nodes[cand[i]].atomicNumber;
                    if (z == 8 || z == 16 || z == 34 || z == 52) {
                        spareValence[i] = 0;
                        zeroSpareCount++;
                    } else {
                        spareValence[i] = 1;
                    }
                }
                
                if (zeroSpareCount > 0) {
                    int startIdx = 0;
                    while (spareValence[startIdx] != 0) startIdx++;
                    
                    int runLength = 0;
                    for (int i = 1; i <= ringSize; ++i) {
                        int idx = (startIdx + i) % ringSize;
                        if (spareValence[idx] == 1) {
                            runLength++;
                        } else {
                            if (runLength > 0) {
                                int runStart = (idx - runLength + ringSize) % ringSize;
                                for (int k = 0; k < runLength / 2; ++k) {
                                    int dbIdx = (runStart + 2 * k) % ringSize;
                                    sig.doubleBondLocants.push_back(dbIdx + 1);
                                }
                            }
                            runLength = 0;
                        }
                    }
                } else {
                    for (int k = 0; k < ringSize / 2; ++k) {
                        sig.doubleBondLocants.push_back(1 + 2 * k);
                    }
                }
                std::sort(sig.doubleBondLocants.begin(), sig.doubleBondLocants.end());
            }
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
                    if ((winningType == GroupType::ALCOHOL || winningType == GroupType::KETONE || winningType == GroupType::AMINE || winningType == GroupType::IMINE || winningType == GroupType::SULFONIC_ACID || winningType == GroupType::SULFONYL_HALIDE || winningType == GroupType::SULFINIC_ACID || winningType == GroupType::THIOL || winningType == GroupType::SELENOL || winningType == GroupType::TELLUROL || winningType == GroupType::HYDROPEROXIDE || winningType == GroupType::PHOSPHONIC_ACID || winningType == GroupType::PHOSPHONIC_DIHALIDE || winningType == GroupType::ARSONIC_ACID) && isPrincipalRNode) {
                        int nz = g.nodes[nei].atomicNumber;
                        bool isAzide = (carbonAzide.count(rNode) && std::find(carbonAzide[rNode].begin(), carbonAzide[rNode].end(), nei) != carbonAzide[rNode].end());
                        if (!isAzide && (nz == 7 || nz == 8 || nz == 16 || nz == 34 || nz == 52 || nz == 15 || nz == 33)) continue;
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
                    } else if (!isAzide && order == 2 && carbonImine.count(rNode) && carbonImine[rNode] == nei && winningType != GroupType::IMINE) {
                        subName = "imino";
                    }
                } else if (nz == 16) {
                    if (carbonSulfonicAcid.count(rNode) && carbonSulfonicAcid[rNode] == nei && winningType != GroupType::SULFONIC_ACID) {
                        subName = "sulfo";
                    } else if (carbonSulfonylHalide.count(rNode) && carbonSulfonylHalide[rNode] == nei && winningType != GroupType::SULFONYL_HALIDE) {
                        subName = halogenPrefix(sulfonylHalideZ[rNode]) + "sulfonyl";
                    } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                        subName = "sulfino";
                    } else if (carbonHydroperoxide.count(rNode) && carbonHydroperoxide[rNode] == nei && winningType != GroupType::HYDROPEROXIDE) {
                        subName = "hydroperoxy";
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
                } else if (nz == 34) {
                    if (carbonSelenol.count(rNode) && carbonSelenol[rNode] == nei && winningType != GroupType::SELENOL) {
                        subName = "selanyl";
                    } else if (order == 1) {
                        if (selenoetherSeleniums.count(nei)) {
                            int alkylNei = -1;
                            for (int sNei : g.nodes[nei].neighbors) {
                                if (sNei != rNode) { alkylNei = sNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                subName = alkylName + "selanyl";
                            }
                        }
                    }
                } else if (nz == 52) {
                    if (carbonTellurol.count(rNode) && carbonTellurol[rNode] == nei && winningType != GroupType::TELLUROL) {
                        subName = "tellanyl";
                    } else if (order == 1) {
                        if (telluroetherTelluriums.count(nei)) {
                            int alkylNei = -1;
                            for (int sNei : g.nodes[nei].neighbors) {
                                if (sNei != rNode) { alkylNei = sNei; break; }
                            }
                            if (alkylNei != -1) {
                                QString alkylName = nameBranchGraph(g, alkylNei, nei);
                                subName = alkylName + "tellanyl";
                            }
                        }
                    }
                } else if (nz == 15) {
                    if (carbonPhosphonicAcid.count(rNode) && carbonPhosphonicAcid[rNode] == nei && winningType != GroupType::PHOSPHONIC_ACID) {
                        subName = "phosphono";
                    } else if (carbonPhosphonicDihalide.count(rNode) && carbonPhosphonicDihalide[rNode] == nei && winningType != GroupType::PHOSPHONIC_DIHALIDE) {
                        subName = "di" + halogenPrefix(phosphonicDihalideZ[rNode]) + "phosphoryl";
                    }
                } else if (nz == 33) {
                    if (carbonArsonicAcid.count(rNode) && carbonArsonicAcid[rNode] == nei && winningType != GroupType::ARSONIC_ACID) {
                        subName = "arsono";
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
                        if (!subName.isEmpty()) {
                            QString branchStereo = formatBranchStereoPrefix(g, nei, rNode, stereoByGraphId, sig.handledBranchStereoIds);
                            if (!branchStereo.isEmpty()) {
                                if (subName.startsWith("(") && subName.endsWith(")")) {
                                    QString inner = subName.mid(1, subName.length() - 2);
                                    subName = QString("[%1%2]").arg(branchStereo, inner);
                                } else {
                                    subName = QString("[%1%2]").arg(branchStereo, subName);
                                }
                            } else if (subName.startsWith("(")) {
                                subName = subName.mid(1);
                                if (subName.endsWith(")")) subName.chop(1);
                            }
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
                    [](const auto &x, const auto &y) { return alphabetizationKey(x.first).toLower() < alphabetizationKey(y.first).toLower(); });
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
        // A ring where every ring-internal bond is a plain single bond is fully
        // saturated (only reachable here for sizes 7-10, single heteroatom --
        // see the saturated branch in classifyMonocyclicHeteroRing). For this
        // case, hwGeneralRingStem must build the "-ane"-family stem (azepane,
        // not azepine); indicated hydrogen (a mancude-form-only concept -- it
        // marks the one saturated position amid an otherwise-maximally-
        // unsaturated ring) does not apply at all, since every position is
        // already saturated; and, matching the same sole-heteroatom locant-
        // omission convention already established for the other saturated
        // ring-parent paths (LARGE_HETEROCYCLE's azacycloundecane family,
        // and PIPERIDINE/PYRROLIDINE/THF/THT), the heteroatom's own locant is
        // omitted even when a substituent/suffix elsewhere needs its own
        // locant (e.g. "azepan-2-one", not "1-azepan-2-one").
        bool ringFullySaturated = true;
        for (const auto &rb : ringBonds) {
            if (rb.order != 1) { ringFullySaturated = false; break; }
        }

        // P-22.2.2.1.3: Citation order for collecting locants and prefixes.
        // O > S > Se > Te > N > P > As > Sb > Bi > Si > Ge > Sn > Pb > B
        static const int citationOrder[] = {8, 16, 34, 52, 7, 15, 33, 51, 83, 14, 32, 50, 82, 5};
        static const int citationOrderLen = 14;

        // Map from atomic number -> sorted list of locants for that element
        std::map<int, std::vector<int>> locantsByZ;
        std::vector<int> allAtomicNumbers; // all heteroatom Z values, for stem selection
        for (size_t i = 0; i < bestSig.ringChain.size(); ++i) {
            int nodeIdx = bestSig.ringChain[i];
            int z = g.nodes[nodeIdx].atomicNumber;
            if (z != 6) {
                int locant = static_cast<int>(i + 1);
                locantsByZ[z].push_back(locant);
                allAtomicNumbers.push_back(z);
            }
        }
        // locants within each element are already in ring-walk order (ascending by construction)

        // Build locant prefix in citation order (P-22.2.2.1.3)
        std::vector<int> allLocs;
        for (int k = 0; k < citationOrderLen; ++k) {
            int z = citationOrder[k];
            auto it = locantsByZ.find(z);
            if (it != locantsByZ.end()) {
                for (int l : it->second) allLocs.push_back(l);
            }
        }
        QStringList locStrs;
        for (int l : allLocs) locStrs.append(QString::number(l));
        QString locantPrefix = (ringFullySaturated && allLocs.size() == 1) ? "" : (locStrs.join(",") + "-");

        // Build 'a'-replacement prefix string in citation order.
        // P-22.2.2.1.1: final 'a' of a prefix elides before the next 'a' (whether
        // from a multiplying prefix like "di-" → no, or from the next 'a'-prefix itself).
        // P-22.2.2.1.2: multiplicity indicated by di/tri/tetra before the 'a' term;
        // final letter 'a' of a multiplying prefix elides before a vowel.
        // Applied here: after appending each element's full prefix chunk, elide trailing
        // 'a' before the stem if the stem starts with a vowel (it never does for ole/ine/inine).
        // Between two consecutive 'a' prefixes: elide the trailing 'a' of the first chunk.
        QString elemPrefixes;
        for (int k = 0; k < citationOrderLen; ++k) {
            int z = citationOrder[k];
            auto it = locantsByZ.find(z);
            if (it == locantsByZ.end()) continue;
            int count = static_cast<int>(it->second.size());
            QString aPrefix = hwAPrefix(z); // e.g. "oxa", "thia", "aza"
            QString chunk;
            if (count == 1) {
                chunk = aPrefix;
            } else {
                // P-22.2.2.1.2: final 'a' of multiplying prefix elides before a vowel.
                // e.g. "tetra"+"aza" -> "tetraza" (not "tetraaza"); "tetra"+"oxa" -> "tetraoxa".
                QString mp = multiPrefix(count);
                if (mp.endsWith('a') && isVowel(aPrefix[0])) mp.chop(1);
                chunk = mp + aPrefix;
            }
            // P-22.2.2.1.1: elide trailing 'a' of previous chunk before this chunk's 'a' start
            if (!elemPrefixes.isEmpty() && elemPrefixes.endsWith('a') && chunk.startsWith('a')) {
                elemPrefixes.chop(1);
            }
            elemPrefixes += chunk;
        }

        // Determine stem: P-22.2.2.1.5.1 / Table 2.5 for all ring sizes 3-10
        QString stem = hwGeneralRingStem(ringSize, allAtomicNumbers, ringFullySaturated);
        if (stem.isEmpty()) { /* should never happen for sizes accepted by tryGeneralHeterocycle */ }

        // P-22.2.2.1.1: elide trailing 'a' of elemPrefixes before the stem
        // (stem 'ole', 'ine', 'inine' all start with a vowel)
        if (elemPrefixes.endsWith('a') && isVowel(stem[0])) {
            elemPrefixes.chop(1);
        }

        if (ringFullySaturated) {
            parentNameRoot = locantPrefix + elemPrefixes + stem;
        } else {
        // P-14.7.1: Check for indicated hydrogen (saturated ring position)
        int indicatedH = findIndicatedHydrogenLocant(g, bestSig.ringChain);
        if (indicatedH == -2) {
            // Multiple indicated hydrogen positions - not supported for general heterocycles
            return {false, "", "Multiple indicated hydrogen positions found; this general heterocycle case is not yet supported."};
        } else if (indicatedH > 0) {
            // Exactly one indicated hydrogen position - prepend "<locant>H-"
            parentNameRoot = QString("%1H-").arg(indicatedH) + locantPrefix + elemPrefixes + stem;
        } else {
            // No indicated hydrogen needed
            parentNameRoot = locantPrefix + elemPrefixes + stem;
        }
        }
    } else if (rType == RingType::LARGE_HETEROCYCLE) {
        // P-22.2.3.1/P-22.2.3.2 (fully saturated heteromonocycles, 11-20 ring members):
        // locants+prefix are grouped PER HETEROATOM KIND and citation-joined with hyphens
        // (e.g. "1-oxa-4,8,11-triazacyclotetradecane"), NOT pooled into one shared locant
        // list the way Hantzsch-Widman (GENERAL_HETEROCYCLE, above) is. Verified against
        // the real Blue Book text (BlueBookV2.md P-22.2.3), not memory. Citation-order
        // seniority reuses hwSeniorityRank/citationOrder: P-22.2.3.1's real order is
        // F > Cl > Br > I > O > S > Se > Te > N > P > As > Sb > Bi > Si > Ge > Sn > Pb >
        // B > Al > Ga > In > Tl, but among the elements this namer actually supports as
        // ring atoms (no halogens/Al/Ga/In/Tl), that's identical to hwSeniorityRank's
        // order, so reusing it here is correct, not a shortcut.
        std::map<int, std::vector<int>> locantsByZ;
        for (size_t i = 0; i < bestSig.ringChain.size(); ++i) {
            int nodeIdx = bestSig.ringChain[i];
            int z = g.nodes[nodeIdx].atomicNumber;
            if (z != 6) locantsByZ[z].push_back(static_cast<int>(i + 1));
        }

        bool omitSingle = bestSig.doubleBondLocants.empty();
        QString heteroPrefix = buildSkeletalReplacementPrefix(locantsByZ, omitSingle);
        if (!heteroPrefix.isEmpty()) {
            parentNameRoot = heteroPrefix + parentNameRoot;
        }
    }
    std::map<int, int> graphIdToLocant;
    for (size_t i = 0; i < bestSig.ringChain.size(); ++i) {
        graphIdToLocant[bestSig.ringChain[i]] = static_cast<int>(i + 1);
    }
    StereoResult ezRes = processDoubleBondStereo(mol, g, ringNodeSet, graphIdToLocant, stereoByGraphId);
    if (!ezRes.ok) {
        return {false, "", ezRes.error};
    }
    StereoResult stereoRes = formatStereoPrefix(stereoByGraphId, graphIdToLocant, bestSig.handledBranchStereoIds);
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
        pg.baseName = alphabetizationKey(pName);
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
    } else if (rType == RingType::LARGE_HETEROCYCLE) {
        std::vector<int> dbLocs = bestSig.doubleBondLocants;
        if (dbLocs.empty()) {
            rootStr += "ane";
        } else {
            rootStr += "a";
            QStringList lStrs;
            for (int l : dbLocs) lStrs.append(QString::number(l));
            rootStr += QString("-%1-%2ene").arg(lStrs.join(","), multiPrefix(static_cast<int>(dbLocs.size())));
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
            rType == RingType::ISOSELENAZOLE || rType == RingType::LARGE_HETEROCYCLE ||
            rType == RingType::PYRROLIDINE || rType == RingType::PIPERIDINE ||
            rType == RingType::TETRAHYDROFURAN || rType == RingType::TETRAHYDROTHIOPHENE) {
            fullName = prefixPart + rootStr;
        } else {
            fullName = prefixPart + rootStr + "e";
        }
    } else {
        bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE || winningType == GroupType::HYDRAZIDE ||
                            winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||
                            winningType == GroupType::ACYL_HALIDE || winningType == GroupType::ESTER);
        int pCount = static_cast<int>(bestSig.principalLocants.size());

        if (isExocyclic) {
            QString sfx;
            if (winningType == GroupType::ACID) sfx = (pCount == 1) ? "carboxylic acid" : "dicarboxylic acid";
            else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? "carboxamide" : "dicarboxamide";
            else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? "carbohydrazide" : "dicarbohydrazide";
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
            else if (winningType == GroupType::SULFONYL_HALIDE) {
                QString hName;
                int hz = sulfonylHalideZ.empty() ? 17 : sulfonylHalideZ.begin()->second;
                for (int pc : principalCarbons) {
                    if (sulfonylHalideZ.count(pc)) { hz = sulfonylHalideZ[pc]; break; }
                }
                hName = halogenSuffixWord(hz);
                sfx = (pCount == 1) ? ("sulfonyl " + hName) : ("disulfonyl " + hName);
            }
            else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";
            else if (winningType == GroupType::PHOSPHONIC_DIHALIDE) {
                QString hName;
                int hz = phosphonicDihalideZ.empty() ? 17 : phosphonicDihalideZ.begin()->second;
                for (int pc : principalCarbons) {
                    if (phosphonicDihalideZ.count(pc)) { hz = phosphonicDihalideZ[pc]; break; }
                }
                hName = "di" + halogenSuffixWord(hz);
                sfx = (pCount == 1) ? ("phosphonic " + hName) : ("diphosphonic " + hName);
            }
            else if (winningType == GroupType::PHOSPHONIC_ACID) sfx = (pCount == 1) ? "phosphonic acid" : "diphosphonic acid";
            else if (winningType == GroupType::ARSONIC_ACID) sfx = (pCount == 1) ? "arsonic acid" : "diarsonic acid";
            else if (winningType == GroupType::THIOL) sfx = (pCount == 1) ? "thiol" : "dithiol";
            else if (winningType == GroupType::SELENOL) sfx = (pCount == 1) ? "selenol" : "diselenol";
            else if (winningType == GroupType::TELLUROL) sfx = (pCount == 1) ? "tellurol" : "ditellurol";
            else if (winningType == GroupType::ALCOHOL) sfx = (pCount == 1) ? "ol" : "diol";
            else if (winningType == GroupType::HYDROPEROXIDE) sfx = (pCount == 1) ? "peroxol" : "diperoxol";
            else if (winningType == GroupType::KETONE) sfx = (pCount == 1) ? "one" : "dione";
            else if (winningType == GroupType::AMINE) sfx = (pCount == 1) ? "amine" : "diamine";
            else if (winningType == GroupType::IMINE) sfx = (pCount == 1) ? "imine" : "diimine";

            bool allCarbon = true;
            for (int n : ringNodeSet) {
                if (g.nodes[n].atomicNumber != 6) {
                    allCarbon = false;
                    break;
                }
            }
            if (rType == RingType::BENZENE && winningType == GroupType::ALCOHOL && pCount == 1) {
                fullName = prefixPart + "phenol";
            } else {
                QString stem = rootStr;
                if (stem.endsWith("e") && !sfx.isEmpty() && isVowel(sfx[0])) stem.chop(1);
                if (pCount == 1 && prefixPart.isEmpty() && allCarbon) {
                    fullName = prefixPart + stem + sfx;
                } else {
                    QStringList lStrs;
                    for (int l : bestSig.principalLocants) lStrs.append(QString::number(l));
                    fullName = prefixPart + stem + QString("-%1-%2").arg(lStrs.join(","), sfx);
                }
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







