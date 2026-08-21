import sys

def apply_patch(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    # 4. Exocyclic/ring-substituent exclusion checks
    # Line ~4478
    old_cNode = """            if (carbonSulfinicAcid.count(cNode) && winningType != GroupType::SULFINIC_ACID) {
                locantSubstituents[locant].append("sulfino");
            }"""
    new_cNode = """            if (carbonSulfinylHalide.count(cNode) && winningType != GroupType::SULFINYL_HALIDE) {
                locantSubstituents[locant].append(halogenPrefix(sulfinylHalideZ[cNode]) + "sulfinyl");
            }
            if (carbonSulfinicAcid.count(cNode) && winningType != GroupType::SULFINIC_ACID) {
                locantSubstituents[locant].append("sulfino");
            }"""
    content = content.replace(old_cNode, new_cNode)

    # Line ~4542
    old_cNode_nei = "if (carbonSulfinicAcid.count(cNode) && carbonSulfinicAcid[cNode] == nei) continue;"
    new_cNode_nei = "if (carbonSulfinylHalide.count(cNode) && carbonSulfinylHalide[cNode] == nei) continue;\n                if (carbonSulfinicAcid.count(cNode) && carbonSulfinicAcid[cNode] == nei) continue;"
    content = content.replace(old_cNode_nei, new_cNode_nei)

    # Line ~8638
    old_rNode = """                        } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                            subName = "sulfino";"""
    new_rNode = """                        } else if (carbonSulfinylHalide.count(rNode) && carbonSulfinylHalide[rNode] == nei && winningType != GroupType::SULFINYL_HALIDE) {
                            subName = halogenPrefix(sulfinylHalideZ[rNode]) + "sulfinyl";
                        } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                            subName = "sulfino";"""
    content = content.replace(old_rNode, new_rNode)

    # Line ~9728
    old_rNode2 = """                    } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                        subName = "sulfino";"""
    new_rNode2 = """                    } else if (carbonSulfinylHalide.count(rNode) && carbonSulfinylHalide[rNode] == nei && winningType != GroupType::SULFINYL_HALIDE) {
                        subName = halogenPrefix(sulfinylHalideZ[rNode]) + "sulfinyl";
                    } else if (carbonSulfinicAcid.count(rNode) && carbonSulfinicAcid[rNode] == nei && winningType != GroupType::SULFINIC_ACID) {
                        subName = "sulfino";"""
    content = content.replace(old_rNode2, new_rNode2)

    # 5. Seniority arrays
    # Line ~8400
    old_sen1 = """                case GroupType::SULFONIC_ACID: return 1;
                case GroupType::SULFINIC_ACID: return 2;
                case GroupType::ACID: return 3;
                case GroupType::PHOSPHONIC_ACID: return 4;
                case GroupType::PHOSPHONIC_DIHALIDE: return 5;
                case GroupType::ARSONIC_ACID: return 6;
                case GroupType::ARSONIC_DIHALIDE: return 7;
                case GroupType::BORONIC_ACID: return 8;
                case GroupType::ESTER: return 9;
                case GroupType::ACYL_HALIDE: return 10;
                case GroupType::SULFONYL_HALIDE: return 11;
                case GroupType::AMIDE: return 12;
                case GroupType::HYDRAZIDE: return 13;
                case GroupType::NITRILE: return 14;
                case GroupType::ALDEHYDE: return 15;
                case GroupType::THIAL: return 16;
                case GroupType::KETONE: return 17;
                case GroupType::THIONE: return 18;
                case GroupType::ALCOHOL: return 19;
                case GroupType::THIOL: return 20;
                case GroupType::SELENOL: return 21;
                case GroupType::TELLUROL: return 22;
                case GroupType::HYDROPEROXIDE: return 23;
                case GroupType::AMINE: return 24;
                case GroupType::IMINE: return 25;
                case GroupType::PHOSPHINE: return 26;
                default: return 27;"""
    new_sen1 = """                case GroupType::SULFONIC_ACID: return 1;
                case GroupType::SULFINIC_ACID: return 2;
                case GroupType::SULFINYL_HALIDE: return 3;
                case GroupType::ACID: return 4;
                case GroupType::PHOSPHONIC_ACID: return 5;
                case GroupType::PHOSPHONIC_DIHALIDE: return 6;
                case GroupType::ARSONIC_ACID: return 7;
                case GroupType::ARSONIC_DIHALIDE: return 8;
                case GroupType::BORONIC_ACID: return 9;
                case GroupType::ESTER: return 10;
                case GroupType::ACYL_HALIDE: return 11;
                case GroupType::SULFONYL_HALIDE: return 12;
                case GroupType::AMIDE: return 13;
                case GroupType::HYDRAZIDE: return 14;
                case GroupType::NITRILE: return 15;
                case GroupType::ALDEHYDE: return 16;
                case GroupType::THIAL: return 17;
                case GroupType::KETONE: return 18;
                case GroupType::THIONE: return 19;
                case GroupType::ALCOHOL: return 20;
                case GroupType::THIOL: return 21;
                case GroupType::SELENOL: return 22;
                case GroupType::TELLUROL: return 23;
                case GroupType::HYDROPEROXIDE: return 24;
                case GroupType::AMINE: return 25;
                case GroupType::IMINE: return 26;
                case GroupType::PHOSPHINE: return 27;
                default: return 28;"""
    content = content.replace(old_sen1, new_sen1)

    # Line ~9296
    old_sen2 = """        switch (gt) {
            case GroupType::SULFONIC_ACID: return 1;
            case GroupType::SULFINIC_ACID: return 2;
            case GroupType::ACID: return 3;
            case GroupType::PHOSPHONIC_ACID: return 4;
            case GroupType::PHOSPHONIC_DIHALIDE: return 5;
            case GroupType::ARSONIC_ACID: return 6;
            case GroupType::ARSONIC_DIHALIDE: return 7;
            case GroupType::BORONIC_ACID: return 8;
            case GroupType::ESTER: return 9;
            case GroupType::ACYL_HALIDE: return 10;
            case GroupType::SULFONYL_HALIDE: return 11;
            case GroupType::AMIDE: return 12;
            case GroupType::HYDRAZIDE: return 13;
            case GroupType::NITRILE: return 14;
            case GroupType::ALDEHYDE: return 15;
            case GroupType::THIAL: return 16;
            case GroupType::KETONE: return 17;
            case GroupType::THIONE: return 18;
            case GroupType::ALCOHOL: return 19;
            case GroupType::THIOL: return 20;
            case GroupType::SELENOL: return 21;
            case GroupType::TELLUROL: return 22;
            case GroupType::HYDROPEROXIDE: return 23;
            case GroupType::AMINE: return 24;
            case GroupType::IMINE: return 25;
            case GroupType::PHOSPHINE: return 26;
            default: return 27;
        }"""
    new_sen2 = """        switch (gt) {
            case GroupType::SULFONIC_ACID: return 1;
            case GroupType::SULFINIC_ACID: return 2;
            case GroupType::SULFINYL_HALIDE: return 3;
            case GroupType::ACID: return 4;
            case GroupType::PHOSPHONIC_ACID: return 5;
            case GroupType::PHOSPHONIC_DIHALIDE: return 6;
            case GroupType::ARSONIC_ACID: return 7;
            case GroupType::ARSONIC_DIHALIDE: return 8;
            case GroupType::BORONIC_ACID: return 9;
            case GroupType::ESTER: return 10;
            case GroupType::ACYL_HALIDE: return 11;
            case GroupType::SULFONYL_HALIDE: return 12;
            case GroupType::AMIDE: return 13;
            case GroupType::HYDRAZIDE: return 14;
            case GroupType::NITRILE: return 15;
            case GroupType::ALDEHYDE: return 16;
            case GroupType::THIAL: return 17;
            case GroupType::KETONE: return 18;
            case GroupType::THIONE: return 19;
            case GroupType::ALCOHOL: return 20;
            case GroupType::THIOL: return 21;
            case GroupType::SELENOL: return 22;
            case GroupType::TELLUROL: return 23;
            case GroupType::HYDROPEROXIDE: return 24;
            case GroupType::AMINE: return 25;
            case GroupType::IMINE: return 26;
            case GroupType::PHOSPHINE: return 27;
            default: return 28;
        }"""
    content = content.replace(old_sen2, new_sen2)

    # 6. Suffix assembly
    # Line ~1767
    old_sfx1 = """    } else if (winningType == GroupType::SULFINIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("sulfinic acid")
                                        : QString("-%1-sulfinic acid").arg(principalLocants[0]);"""
    new_sfx1 = """    } else if (winningType == GroupType::SULFINYL_HALIDE) {
        QString hName;
        int hz = sulfinylHalideZ.empty() ? 17 : sulfinylHalideZ.begin()->second;
        for (int pc : principalCarbons) {
            if (sulfinylHalideZ.count(pc)) { hz = sulfinylHalideZ[pc]; break; }
        }
        hName = halogenSuffixWord(hz);
        if (pCount == 1) {
            sfx = (k <= 2) ? QString("sulfinyl %1").arg(hName)
                           : QString("-%1-sulfinyl %2").arg(principalLocants[0]).arg(hName);
        } else {
            sfx = QString("-%1-%2sulfinyl %3").arg(lStrs.join(","), multiPrefix(pCount), hName);
        }
    } else if (winningType == GroupType::SULFINIC_ACID) {
        if (pCount == 1) sfx = (k <= 2) ? QStringLiteral("sulfinic acid")
                                        : QString("-%1-sulfinic acid").arg(principalLocants[0]);"""
    content = content.replace(old_sfx1, new_sfx1)

    # Line ~8876
    old_sfx2 = """                else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";"""
    new_sfx2 = """                else if (winningType == GroupType::SULFINYL_HALIDE) {
                    QString hName;
                    int hz = sulfinylHalideZ.empty() ? 17 : sulfinylHalideZ.begin()->second;
                    for (int pc : principalCarbons) {
                        if (sulfinylHalideZ.count(pc)) { hz = sulfinylHalideZ[pc]; break; }
                    }
                    hName = halogenSuffixWord(hz);
                    sfx = (pCount == 1) ? ("sulfinyl " + hName) : ("disulfinyl " + hName);
                }
                else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";"""
    content = content.replace(old_sfx2, new_sfx2)

    # Line ~10160
    old_sfx3 = """            else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";"""
    new_sfx3 = """            else if (winningType == GroupType::SULFINYL_HALIDE) {
                QString hName;
                int hz = sulfinylHalideZ.empty() ? 17 : sulfinylHalideZ.begin()->second;
                for (int pc : principalCarbons) {
                    if (sulfinylHalideZ.count(pc)) { hz = sulfinylHalideZ[pc]; break; }
                }
                hName = halogenSuffixWord(hz);
                sfx = (pCount == 1) ? ("sulfinyl " + hName) : ("disulfinyl " + hName);
            }
            else if (winningType == GroupType::SULFINIC_ACID) sfx = (pCount == 1) ? "sulfinic acid" : "disulfinic acid";"""
    content = content.replace(old_sfx3, new_sfx3)

    # Long lists
    content = content.replace(
        "gt == GroupType::SULFINIC_ACID",
        "gt == GroupType::SULFINYL_HALIDE || gt == GroupType::SULFINIC_ACID"
    )

    content = content.replace(
        "GroupType::SULFONIC_ACID, GroupType::SULFINIC_ACID",
        "GroupType::SULFONIC_ACID, GroupType::SULFINIC_ACID, GroupType::SULFINYL_HALIDE"
    )
    
    content = content.replace(
        "winningType == GroupType::SULFINIC_ACID || winningType == GroupType::THIOL",
        "winningType == GroupType::SULFINYL_HALIDE || winningType == GroupType::SULFINIC_ACID || winningType == GroupType::THIOL"
    )

    # Seniority check in ring/group rank (just in case there's another occurrence)
    content = content.replace(
        "winningType == GroupType::SULFINIC_ACID) {",
        "winningType == GroupType::SULFINYL_HALIDE || winningType == GroupType::SULFINIC_ACID) {"
    )

    with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
        f.write(content)

if __name__ == "__main__":
    apply_patch("src/app/IupacNamer.cpp")
