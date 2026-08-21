import sys

def apply_patch(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    old_sfx1 = """    } else if (winningType == GroupType::SULFINYL_HALIDE) {
        QString hName;
        int hz = sulfinylHalideZ.empty() ? 17 : sulfinylHalideZ.begin()->second;
        for (int pc : principalCarbons) {
            if (sulfinylHalideZ.count(pc)) { hz = sulfinylHalideZ[pc]; break; }
        }
        hName = halogenSuffixWord(hz);
        if (pCount == 1) {
            sfx = (k <= 2) ? QString("sulfinyl %1").arg(hName)
                           : QString("-%1-sulfinyl %2").arg(principalLocants[0], hName);
        } else {
            sfx = QString("-%1-%2sulfinyl %3").arg(lStrs.join(","), multiPrefix(pCount), hName);
        }
    } else if (winningType == GroupType::SULFINIC_ACID) {"""
    
    new_sfx1 = """    } else if (winningType == GroupType::SULFINYL_HALIDE) {
        QString hName = halogenSuffixWord(acylHalideHalogenZ);
        if (pCount == 1) {
            sfx = (k <= 2) ? QString("sulfinyl %1").arg(hName)
                           : QString("-%1-sulfinyl %2").arg(principalLocants[0]).arg(hName);
        } else {
            QStringList lStrs;
            for (int l : principalLocants) lStrs.append(QString::number(l));
            sfx = QString("-%1-%2sulfinyl %3").arg(lStrs.join(","), multiPrefix(pCount), hName);
        }
    } else if (winningType == GroupType::SULFINIC_ACID) {"""

    content = content.replace(old_sfx1, new_sfx1)

    with open("src/app/IupacNamer.cpp", "w", encoding="utf-8") as f:
        f.write(content)

if __name__ == "__main__":
    apply_patch("src/app/IupacNamer.cpp")
