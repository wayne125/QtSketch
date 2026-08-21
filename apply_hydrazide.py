import re

with open('src/app/IupacNamer.cpp', 'r') as f:
    content = f.read()

# 1. GroupType
content = content.replace("""AMIDE,         // Amide\n    NITRILE,""", """AMIDE,         // Amide\n    HYDRAZIDE,     // Hydrazide\n    NITRILE,""")

# 2. isChainParentWithRingSubstituentSupported
content = content.replace("""gt == GroupType::ACID || gt == GroupType::AMIDE || gt == GroupType::NITRILE""", """gt == GroupType::ACID || gt == GroupType::AMIDE || gt == GroupType::HYDRAZIDE || gt == GroupType::NITRILE""")

# 3. isPrincipalGroupHeteroNeighbor
content = content.replace("""        if (nz == 7 && order == 1) return true;                         // -N<\n    } else if (winningType == GroupType::NITRILE) {""", """        if (nz == 7 && order == 1) return true;                         // -N<\n    } else if (winningType == GroupType::HYDRAZIDE) {\n        if (nz == 8 && order == 2) return true;                         // =O\n        if (nz == 7 && order == 1) return true;                         // -N<\n    } else if (winningType == GroupType::NITRILE) {""")

# 4. Acyclic Suffix
content = content.replace("""sfx = (pCount == 2) ? QStringLiteral("diamide") : QStringLiteral("amide");\n    } else if (winningType == GroupType::NITRILE) {""", """sfx = (pCount == 2) ? QStringLiteral("diamide") : QStringLiteral("amide");\n    } else if (winningType == GroupType::HYDRAZIDE) {\n        sfx = (pCount == 2) ? QStringLiteral("dihydrazide") : QStringLiteral("hydrazide");\n    } else if (winningType == GroupType::NITRILE) {""")

# 5. Seniority Order array
content = content.replace("""GroupType::ESTER, GroupType::ACYL_HALIDE, GroupType::AMIDE, GroupType::NITRILE""", """GroupType::ESTER, GroupType::ACYL_HALIDE, GroupType::AMIDE, GroupType::HYDRAZIDE, GroupType::NITRILE""")

# 6. myGroupRank
content = content.replace("""if (gt == GroupType::AMIDE) return 2;\n                if (gt == GroupType::KETONE)""", """if (gt == GroupType::AMIDE) return 2;\n                if (gt == GroupType::HYDRAZIDE) return 3;\n                if (gt == GroupType::KETONE)""")
content = content.replace("""if (gt == GroupType::KETONE) return 3;""", """if (gt == GroupType::KETONE) return 4;""")
content = content.replace("""if (gt == GroupType::ALCOHOL) return 4;""", """if (gt == GroupType::ALCOHOL) return 5;""")
content = content.replace("""if (gt == GroupType::AMINE) return 5;""", """if (gt == GroupType::AMINE) return 6;""")

# 7. Cage detection
content = content.replace("""else if (hasDblO && hasSglN) myCarbonGroup[rIdx] = GroupType::AMIDE;""", """else if (hasDblO && hasSglN) {
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
                }""")

# 8. Cage principal atom
content = content.replace("""if (winningType == GroupType::AMIDE && (z == 8 || z == 7)) isPrincipalAtom = true;""", """if (winningType == GroupType::AMIDE && (z == 8 || z == 7)) isPrincipalAtom = true;\n                        if (winningType == GroupType::HYDRAZIDE && (z == 8 || z == 7)) isPrincipalAtom = true;""")

# 9. Detection (3 places: acyclic, ring, naphthalene)
detect_old = """                } else if (!doubleO.empty() && !singleN.empty()) {
                    carbonGroup[i] = GroupType::AMIDE;
                } else if (!tripleN.empty()) {"""

detect_new = """                } else if (!doubleO.empty() && !singleN.empty()) {
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
                } else if (!tripleN.empty()) {"""

content = content.replace(detect_old, detect_new)

# 10. switch statements (2 places)
switch_old = """                case GroupType::AMIDE: return 8;
                case GroupType::NITRILE: return 9;
                case GroupType::ALDEHYDE: return 10;
                case GroupType::THIAL: return 11;
                case GroupType::KETONE: return 12;
                case GroupType::THIONE: return 13;
                case GroupType::ALCOHOL: return 14;
                case GroupType::THIOL: return 15;
                case GroupType::SELENOL: return 16;
                case GroupType::TELLUROL: return 17;
                case GroupType::HYDROPEROXIDE: return 18;
                case GroupType::AMINE: return 19;
                case GroupType::IMINE: return 20;
                case GroupType::PHOSPHINE: return 21;"""

switch_new = """                case GroupType::AMIDE: return 8;
                case GroupType::HYDRAZIDE: return 9;
                case GroupType::NITRILE: return 10;
                case GroupType::ALDEHYDE: return 11;
                case GroupType::THIAL: return 12;
                case GroupType::KETONE: return 13;
                case GroupType::THIONE: return 14;
                case GroupType::ALCOHOL: return 15;
                case GroupType::THIOL: return 16;
                case GroupType::SELENOL: return 17;
                case GroupType::TELLUROL: return 18;
                case GroupType::HYDROPEROXIDE: return 19;
                case GroupType::AMINE: return 20;
                case GroupType::IMINE: return 21;
                case GroupType::PHOSPHINE: return 22;"""

content = content.replace(switch_old, switch_new)

switch2_old = """            case GroupType::AMIDE: return 8;
            case GroupType::NITRILE: return 9;
            case GroupType::ALDEHYDE: return 10;
            case GroupType::THIAL: return 11;
            case GroupType::KETONE: return 12;
            case GroupType::THIONE: return 13;
            case GroupType::ALCOHOL: return 15;
            case GroupType::THIOL: return 16;
            case GroupType::SELENOL: return 17;
            case GroupType::TELLUROL: return 18;
            case GroupType::HYDROPEROXIDE: return 19;
            case GroupType::AMINE: return 20;
            case GroupType::IMINE: return 21;
            case GroupType::PHOSPHINE: return 22;"""

switch2_new = """            case GroupType::AMIDE: return 8;
            case GroupType::HYDRAZIDE: return 9;
            case GroupType::NITRILE: return 10;
            case GroupType::ALDEHYDE: return 11;
            case GroupType::THIAL: return 12;
            case GroupType::KETONE: return 13;
            case GroupType::THIONE: return 14;
            case GroupType::ALCOHOL: return 15;
            case GroupType::THIOL: return 16;
            case GroupType::SELENOL: return 17;
            case GroupType::TELLUROL: return 18;
            case GroupType::HYDROPEROXIDE: return 19;
            case GroupType::AMINE: return 20;
            case GroupType::IMINE: return 21;
            case GroupType::PHOSPHINE: return 22;"""

content = content.replace(switch2_old, switch2_new)

# 11. isExocyclic
exo_old = """bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE ||\n                                winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||"""
exo_new = """bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE || winningType == GroupType::HYDRAZIDE ||\n                                winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||"""
content = content.replace(exo_old, exo_new)

exo2_old = """bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE ||\n                            winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||"""
exo2_new = """bool isExocyclic = (winningType == GroupType::ACID || winningType == GroupType::AMIDE || winningType == GroupType::HYDRAZIDE ||\n                            winningType == GroupType::NITRILE || winningType == GroupType::ALDEHYDE ||"""
content = content.replace(exo2_old, exo2_new)

# 12. exocyclic suffix
sfx_old = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? QStringLiteral("carboxamide") : QStringLiteral("dicarboxamide");\n                else if (winningType == GroupType::NITRILE)"""
sfx_new = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? QStringLiteral("carboxamide") : QStringLiteral("dicarboxamide");\n                else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? QStringLiteral("carbohydrazide") : QStringLiteral("dicarbohydrazide");\n                else if (winningType == GroupType::NITRILE)"""
content = content.replace(sfx_old, sfx_new)

sfx2_old = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? QStringLiteral("carboxamide") : QStringLiteral("dicarboxamide");\n            else if (winningType == GroupType::NITRILE)"""
sfx2_new = """else if (winningType == GroupType::AMIDE) sfx = (pCount == 1) ? QStringLiteral("carboxamide") : QStringLiteral("dicarboxamide");\n            else if (winningType == GroupType::HYDRAZIDE) sfx = (pCount == 1) ? QStringLiteral("carbohydrazide") : QStringLiteral("dicarbohydrazide");\n            else if (winningType == GroupType::NITRILE)"""
content = content.replace(sfx2_old, sfx2_new)

with open('src/app/IupacNamer.cpp', 'w') as f:
    f.write(content)
