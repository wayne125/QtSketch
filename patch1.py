import sys
import re

with open('src/app/IupacNamer.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Add buildSkeletalReplacementPrefix helper before hwSixMemberStem (around line 295)
helper_code = '''
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
'''
code = code.replace('static QString hwSixMemberStem', helper_code + '\nstatic QString hwSixMemberStem')

# 2. Refactor LARGE_HETEROCYCLE to use it (around line 8923)
large_het_old = '''        int totalHeteroCount = 0;
        for (const auto &kv : locantsByZ) totalHeteroCount += static_cast<int>(kv.second.size());

        if (totalHeteroCount == 1) {
            // P-22.2.3.2.1: locant '1' for a sole heteroatom is omitted entirely in the
            // saturated ('-ane') form (e.g. "thiacyclododecane") -- confirmed against the
            // Blue Book's own example, unlike the mancude form which always shows it
            // (not implemented in this phase; see classifyMonocyclicHeteroRing).
            if (bestSig.doubleBondLocants.empty()) {
                int z = locantsByZ.begin()->first;
                parentNameRoot = hwAPrefix(z) + parentNameRoot;
            } else {
                int z = locantsByZ.begin()->first;
                parentNameRoot = "1-" + hwAPrefix(z) + parentNameRoot;
            }
        } else {
            QStringList chunks;
            for (int k = 0; k < citationOrderLen; ++k) {
                int z = citationOrder[k];
                auto it = locantsByZ.find(z);
                if (it == locantsByZ.end()) continue;
                const std::vector<int> &locs = it->second;
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
            parentNameRoot = chunks.join("-") + parentNameRoot;
        }'''
large_het_new = '''        bool omitSingle = bestSig.doubleBondLocants.empty();
        QString heteroPrefix = buildSkeletalReplacementPrefix(locantsByZ, omitSingle);
        if (!heteroPrefix.isEmpty()) {
            // buildSkeletalReplacementPrefix returns e.g. "oxa" or "1-oxa". We just prepend it.
            // Wait, does "oxa" need a hyphen? No, "oxa" is added directly. "1-oxa" already has a hyphen.
            // Oh, wait, chunks.join("-") returns "1-oxa-4-thia". When appending "cyclotetradecane", it doesn't need a hyphen if the prefix ends in a letter.
            // Wait, the original code did: chunks.join("-") + parentNameRoot. (e.g. "1-oxa" + "cyclotetradecane" = "1-oxacyclotetradecane")
            parentNameRoot = heteroPrefix + parentNameRoot;
        }'''
code = code.replace(large_het_old, large_het_new)

with open('src/app/IupacNamer.cpp', 'w', encoding='utf-8') as f:
    f.write(code)
