#include "RetainedRingMatcher.h"
#include "RetainedRingTypes.h"
#include "retained/HydrocarbonFamily.h"
#include "retained/QuinolineFamily.h"
#include "retained/IndoleFamily.h"
#include "retained/TricyclicNFamily.h"
#include "retained/PhenoxazineFamily.h"
#include "retained/ChromeneXanthineFamily.h"
#include "retained/PhosphorusArsenicFamily.h"
#include "NamingTypes.h"
#include "indigo.h"
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <functional>

// Build the full registry once, on first use.
static const std::vector<RetainedRingSystem>& fullRegistry() {
    static std::vector<RetainedRingSystem> registry = [] {
        std::vector<RetainedRingSystem> r;
        registerHydrocarbonFamily(r);
        registerQuinolineFamily(r);
        registerIndoleFamily(r);
        registerTricyclicNFamily(r);
        registerPhenoxazineFamily(r);
        registerChromeneXanthineFamily(r);
        registerPhosphorusArsenicFamily(r);
        return r;
    }();
    return registry;
}

// Returns true and fills outAtomMap (position within sys.referenceSmiles's
// own atom order -> candidate-molecule atom index) if sys matches the
// induced subgraph of `ringAtoms` (a set of candidate-molecule atom
// indices) EXACTLY -- same topology AND same heteroatom identities at
// each position.
//
// REDESIGNED (found independently during Task 8's own verification, not
// caught by the implementer's self-report): the original approach here
// built an indigoSubstructureMatcher on `mol`, masked out non-subset atoms
// via indigoIgnoreAtom, and called indigoMatch(matcher, refMol) where
// refMol was loaded via indigoLoadMoleculeFromString (a plain molecule).
// indigoMatch's "query" parameter REQUIRES a query molecule -- passing a
// plain molecule makes indigoMatch fail with the real Indigo error "core:
// <molecule> is not a query molecule" and return -1, which the old code's
// `if (match != 0)` treated as SUCCESS (confirmed via a fresh scratch
// Indigo run reproducing this exact error string). This is why every test
// that should have correctly REJECTED a wrong base (chromene matching a
// naphthalene target, phenoxazine matching the ground-truth molecule's
// isoquinoline-containing ring subset) instead reported a false match.
//
// Switching the query to indigoLoadQueryMoleculeFromString does make
// indigoMatch succeed/fail correctly by heteroatom identity, but a second,
// deeper problem remains: several registry referenceSmiles mix an
// uppercase uncharged heteroatom symbol into an otherwise lowercase-
// aromatic ring (e.g. isoquinoline's own "n1ccc2ccccc2c1" style entries,
// before THIS fix also quinoline/cinnoline/etc, and chromene/phenoxazine-
// family entries with deliberate uppercase N-H or explicit-H bracket
// atoms). Loaded as a QUERY molecule, Indigo does not reconcile that
// representation against a target's fully-aromatic bonds even after an
// explicit indigoAromatize() call -- confirmed via fresh Indigo runs that
// an uppercase-N query molecule fails to self-match an identical target.
//
// The fix that empirically works for both problems (confirmed via fresh
// Indigo runs on the ground-truth naphtho[2,3-g]isoquinoline molecule, its
// correct isoquinoline k=2 subset, and deliberately-wrong chromene/
// phenoxazine/naphthalene/quinoline rejections): extract the ring subset
// into its own standalone molecule via indigoGetSubmolecule + indigoClone
// (NOT a query), aromatize both sides explicitly, and compare with
// indigoExactMatch (which returns a real mapping object on success, zero
// otherwise, and takes two ordinary molecules -- no query-molecule
// representational issues at all).
static bool matchesRetainedSystem(int mol, const std::set<int> &ringAtoms,
                                    const RetainedRingSystem &sys,
                                    std::map<int,int> &outAtomMap) {
    if (static_cast<int>(ringAtoms.size()) != static_cast<int>(sys.peripheralLocants.size())) {
        return false; // atom count must match exactly -- cheap reject before
                       // paying for submolecule extraction
    }

    int refMol = indigoLoadMoleculeFromString(sys.referenceSmiles.toUtf8().constData());
    if (refMol < 0) return false;
    indigoAromatize(refMol);

    int refAtomCount = 0;
    int refIter = indigoIterateAtoms(refMol);
    while (indigoNext(refIter)) refAtomCount++;
    indigoFree(refIter);
    if (static_cast<int>(ringAtoms.size()) != refAtomCount) {
        indigoFree(refMol);
        return false;
    }

    // Vertex order is the sorted order of ringAtoms (std::set iterates
    // ascending) -- fixed and reproducible, used below to translate the
    // clone's own atom indices back to real `mol` atom indices.
    std::vector<int> verts(ringAtoms.begin(), ringAtoms.end());
    int sub = indigoGetSubmolecule(mol, static_cast<int>(verts.size()), verts.data());
    int clone = indigoClone(sub);
    indigoAromatize(clone);

    int mapping = indigoExactMatch(clone, refMol, "");
    bool success = (mapping > 0);
    if (success) {
        outAtomMap.clear();
        int qIter = indigoIterateAtoms(refMol);
        int qPos = 0;
        while (int qAtom = indigoNext(qIter)) {
            int tAtom = indigoMapAtom(mapping, qAtom);
            if (tAtom == 0) { success = false; indigoFree(qAtom); break; }
            int cloneIdx = indigoIndex(tAtom);
            outAtomMap[qPos] = verts[cloneIdx]; // clone index -> real mol atom index
            ++qPos;
            indigoFree(qAtom);
        }
        indigoFree(qIter);
    }
    if (mapping > 0) indigoFree(mapping);

    indigoFree(clone);
    indigoFree(sub);
    indigoFree(refMol);
    return success;
}

// FR-2.3 heteroatom priority order for tie-breaking (a) criterion.
// Lower rank = more senior. This list follows Blue Book FR-2.3(a):
// "The heteroatom which occurs earliest in the following sequence: F, Cl, Br, I,
// O, S, Se, Te, P, As, Sb, Bi, Si, Ge, Sn, Pb, B, Hg, N" determines seniority.
static int getHeteroatomRank(const QString &atomicSymbol) {
    static const std::vector<QString> order = {
        "F", "Cl", "Br", "I",
        "O", "S", "Se", "Te",
        "P", "As", "Sb", "Bi",
        "Si", "Ge", "Sn", "Pb",
        "B", "Hg",
        "N"
    };
    for (size_t i = 0; i < order.size(); ++i) {
        if (atomicSymbol == order[i]) return static_cast<int>(i);
    }
    return static_cast<int>(order.size()); // Not a heteroatom
}

// Computes the seniority rank for a candidate registry entry based on
// the heteroatoms present in its referenceSmiles.
static int computeHeteroatomRank(const RetainedRingSystem &sys) {
    int refMol = indigoLoadMoleculeFromString(sys.referenceSmiles.toUtf8().constData());
    if (refMol < 0) return 999; // Very low priority if can't parse
    
    int bestRank = 999;
    int atomIter = indigoIterateAtoms(refMol);
    while (int atom = indigoNext(atomIter)) {
        // Get atomic symbol
        const char *sym = indigoSymbol(atom);
        if (sym && sym[0] != '\0') {
            QString atomicSymbol = QString::fromUtf8(sym);
            int rank = getHeteroatomRank(atomicSymbol);
            if (rank < bestRank) bestRank = rank;
        }
        indigoFree(atom);
    }
    indigoFree(atomIter);
    indigoFree(refMol);
    return bestRank;
}



// Implements FR-2.3(a)-(j)'s ordered criteria for choosing among multiple
// candidate base-component matches at the SAME winning ring-subset size k.
// Returns true if `a` is preferred over `b`.
static bool isPreferredBase(const RetainedRingSystem &a, int aHeteroatomRank, int aK,
                           const RetainedRingSystem &b, int bHeteroatomRank, int bK) {
    // (a) heteroatom occurring earliest in priority list -- OUTRANKS ring
    // count. This must be compared across candidates at DIFFERENT k, not
    // just within one k -- see the fix note on tryRetainedBaseDecomposition
    // below for why the search loop can no longer stop at the first
    // non-empty k before this comparator ever sees a smaller-k, higher-
    // heteroatom-rank candidate.
    if (aHeteroatomRank != bHeteroatomRank) return aHeteroatomRank < bHeteroatomRank;

    // (b) Greater number of rings (only reached when heteroatom rank ties)
    if (aK != bK) return aK > bK;

    // (c) Larger rings at first point of difference -- skip for now, same k
    // (d) Greater number of heteroatoms -- not implemented yet
    // (e) Heteroatoms in order of FR-2.3(a) priority -- not implemented yet
    // (f) Lower locants -- not implemented yet
    // (g) Seniority rank from Appendix (lower = more senior)
    return a.seniorityRank < b.seniorityRank;
}

// Computes all graph automorphisms of a standalone reference molecule
// (returned as permutations over its own indigoIterateAtoms position
// order, i.e. the same "qPos" space peripheralLocants/atomMap use), via
// plain backtracking isomorphism search (refMol is always small -- a
// retained ring system's own atom count -- so this is cheap).
//
// Needed because a SYMMETRIC retained system (e.g. naphthalene, which has
// a point-symmetry automorphism) can match a real molecule's ring-subset
// via more than one equally-valid atom correspondence, and Blue Book
// FR-4.1's "locants as low as possible" rule means the CORRECT citation
// for an attached component's fusion bond may require a different
// automorphic relabeling than whichever one indigoExactMatch happened to
// return (e.g. isoquinoline's own numbering calls the physical fusion
// bond "6,7", and naphthalene's canonical registry numbering ALSO calls
// that exact physical bond "6,7" under the identity labeling -- but
// naphthalene's point-symmetry automorphism relabels the very same bond
// "2,3", which is lower and is the real published citation
// "naphtho[2,3-g]isoquinoline", not "naphtho[6,7-g]isoquinoline").
static std::vector<std::vector<int>> computeAutomorphisms(int refMol) {
    std::vector<int> atomHandles;
    int iter = indigoIterateAtoms(refMol);
    int h;
    while ((h = indigoNext(iter)) != 0) atomHandles.push_back(h);
    indigoFree(iter);

    int n = static_cast<int>(atomHandles.size());
    std::vector<int> atomicNum(n);
    for (int i = 0; i < n; ++i) atomicNum[i] = indigoAtomicNumber(atomHandles[i]);

    std::vector<std::vector<bool>> adjMat(n, std::vector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        int neiIter = indigoIterateNeighbors(atomHandles[i]);
        int nei;
        while ((nei = indigoNext(neiIter)) != 0) {
            int j = indigoIndex(nei);
            if (j >= 0 && j < n) adjMat[i][j] = true;
            indigoFree(nei);
        }
        indigoFree(neiIter);
    }
    for (int i = 0; i < n; ++i) indigoFree(atomHandles[i]);

    std::vector<std::vector<int>> results;
    std::vector<int> perm(n, -1);
    std::vector<bool> used(n, false);

    std::function<void(int)> backtrack = [&](int i) {
        if (i == n) { results.push_back(perm); return; }
        for (int j = 0; j < n; ++j) {
            if (used[j]) continue;
            if (atomicNum[j] != atomicNum[i]) continue;
            bool ok = true;
            for (int k = 0; k < i && ok; ++k) {
                if (adjMat[i][k] != adjMat[j][perm[k]]) ok = false;
            }
            if (!ok) continue;
            perm[i] = j; used[j] = true;
            backtrack(i + 1);
            used[j] = false; perm[i] = -1;
        }
    };
    backtrack(0);
    return results;
}

// Blue Book "lowest locants" comparison for a single locant string: numeric
// part first, then letter suffix (e.g. "4" < "4a" < "5").
static bool locantLess(const QString &a, const QString &b) {
    auto parse = [](const QString &s) -> std::pair<int, QChar> {
        int i = 0;
        while (i < s.size() && s[i].isDigit()) ++i;
        int num = s.left(i).toInt();
        QChar letter = (i < s.size()) ? s[i] : QChar(QChar::Null);
        return {num, letter};
    };
    auto pa = parse(a);
    auto pb = parse(b);
    if (pa.first != pb.first) return pa.first < pb.first;
    return pa.second < pb.second;
}

// Build ring adjacency information for the molecule's SSSR.
// Returns false if the ring system is not a simple linear/ortho-fused chain.
static bool buildRingAdjacency(int mol, 
                              std::vector<std::set<int>> &sssrRings,
                              std::vector<std::vector<int>> &shared,
                              std::vector<int> &degree,
                              int &N) {
    sssrRings.clear();
    
    int sssrIter = indigoIterateSSSR(mol);
    if (sssrIter < 0) return false;
    
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
    
    N = static_cast<int>(sssrRings.size());
    if (N < 2) return false; // Need at least 2 rings for fusion
    
    shared.assign(N, std::vector<int>(N, 0));
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            int count = 0;
            for (int a : sssrRings[i]) {
                if (sssrRings[j].count(a)) count++;
            }
            shared[i][j] = shared[j][i] = count;
        }
    }
    
    degree.assign(N, 0);
    bool validFusion = true;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (shared[i][j] == 2) degree[i]++;
            else if (shared[i][j] != 0) validFusion = false;
        }
    }
    
    return validFusion;
}

std::optional<RingSystemNameResult> tryRetainedBaseDecomposition(int mol) {
    // Step 1: Build ring adjacency
    std::vector<std::set<int>> sssrRings;
    std::vector<std::vector<int>> shared;
    std::vector<int> degree;
    int N = 0;
    
    if (!buildRingAdjacency(mol, sssrRings, shared, degree, N)) {
        // std::cerr << "DEBUG: buildRingAdjacency returned false" << std::endl;
        return std::nullopt; // Not a valid fusion or can't detect SSSR
    }
    
    // Step 2: Check if it's a simple linear/ortho-fused chain
    int ends = 0, centers = 0;
    bool hasBranching = false;
    for (int i = 0; i < N; ++i) {
        if (degree[i] == 1) ends++;
        else if (degree[i] == 2) centers++;
        else if (degree[i] > 2) hasBranching = true;
    }
    
    // A valid LINEAR chain: every shared[i][j] is either 0 or 2 (never 1 or >2),
    // AND ends==2 AND centers==N-2 (i.e. exactly a straight path through all N rings, no branching).
    // Also, validFusion from buildRingAdjacency must be true (no spiro/bridged fusions).
    bool isLinearChain = (ends == 2 && centers == N - 2 && !hasBranching);
    // std::cerr << "DEBUG: N=" << N << " ends=" << ends << " centers=" << centers << " hasBranching=" << hasBranching << " isLinearChain=" << isLinearChain << std::endl;
    if (!isLinearChain) {
        return std::nullopt; // Branching systems fall through to nameBranchingFusion
    }
    
    // Step 3: Order rings along the chain
    // Find the two end rings (degree == 1)
    std::vector<int> endRings;
    for (int i = 0; i < N; ++i) {
        if (degree[i] == 1) endRings.push_back(i);
    }
    
    if (endRings.size() != 2) return std::nullopt;
    
    // Build the chain order by walking from one end to the other
    std::vector<int> chainOrder;
    int current = endRings[0];
    std::set<int> visited;
    visited.insert(current);
    chainOrder.push_back(current);
    
    while (chainOrder.size() < static_cast<size_t>(N)) {
        int next = -1;
        for (int j = 0; j < N; ++j) {
            if (!visited.count(j) && shared[current][j] == 2) {
                next = j;
                break;
            }
        }
        if (next == -1) return std::nullopt; // Chain is broken
        visited.insert(next);
        chainOrder.push_back(next);
        current = next;
    }
    
    // Step 4: For k = N down to 2, search for contiguous adjacent-ring subsets
    const auto &registry = fullRegistry();
    
    // Precompute heteroatom ranks for all registry entries
    std::map<const RetainedRingSystem*, int> heteroatomRankCache;
    for (const auto &sys : registry) {
        heteroatomRankCache[&sys] = computeHeteroatomRank(sys);
    }
    
    // Candidate base: the best match at the largest k
    struct Candidate {
        int k;
        int startIdx; // start index in chainOrder for this contiguous subset
        const RetainedRingSystem *system;
        std::map<int, int> atomMap; // query atom pos -> target atom index
    };
    
    std::vector<Candidate> bestCandidates;

    // FIX (found independently during Task 8's own mandatory ground-truth
    // check -- the implementer's own test 4 "passed" by returning a
    // non-nullopt result without checking it was the RIGHT one): this loop
    // used to `break` as soon as any k produced a match, on the theory that
    // FR-2.3 always prefers more rings. That is backwards: isPreferredBase's
    // own documented criteria list (a) heteroatom rank ABOVE (b) ring count
    // -- a nitrogen-containing 2-ring base must beat an all-carbon 3-ring
    // base, but breaking early on the 3-ring match (e.g. an anthracene
    // match for naphtho[2,3-g]isoquinoline's 3 carbocyclic rings) never
    // lets the comparator see the 2-ring isoquinoline candidate at all.
    // Confirmed via the ground-truth molecule c1ccc2cc3cc4ccncc4cc3cc2c1
    // (real name naphtho[2,3-g]isoquinoline): the old code returned
    // baseName="anthracene" here, which is wrong. Fix: search every k from
    // N down to 2 WITHOUT breaking early, collect every valid candidate
    // across all k, then let isPreferredBase (now also comparing k as its
    // criterion (b), only after heteroatom rank ties) pick the true winner
    // across the whole set.
    for (int k = N; k >= 2; --k) {
        // For each contiguous subset of size k
        for (int start = 0; start <= N - k; ++start) {
            // Collect all atoms in rings [start, start+k)
            std::set<int> ringAtoms;
            for (int r = start; r < start + k; ++r) {
                int ringIdx = chainOrder[r];
                for (int a : sssrRings[ringIdx]) {
                    ringAtoms.insert(a);
                }
            }
            
            // Try each registry entry
            for (const auto &sys : registry) {
                
                // Quick size check
                if (static_cast<int>(ringAtoms.size()) != static_cast<int>(sys.peripheralLocants.size())) {
                    continue;
                }
                
                std::map<int, int> atomMap;
                if (matchesRetainedSystem(mol, ringAtoms, sys, atomMap)) {
                    // Found a match at this k
                    Candidate cand;
                    cand.k = k;
                    cand.startIdx = start;
                    cand.system = &sys;
                    cand.atomMap = atomMap;
                    bestCandidates.push_back(cand);
                }
            }
        }
        
        // Do NOT break here -- see the fix note above the loop. Every k
        // from N down to 2 is searched; isPreferredBase picks the true
        // winner across the whole collected set afterward.
    }

    // Step 5: If no candidate at any k, return nullopt
    if (bestCandidates.empty()) {
        return std::nullopt;
    }

    // Step 6: Pick the best candidate across ALL k via the full FR-2.3
    // comparator (heteroatom rank first, ring count second).
    Candidate *best = &bestCandidates[0];
    for (size_t i = 1; i < bestCandidates.size(); ++i) {
        int aRank = heteroatomRankCache[best->system];
        int bRank = heteroatomRankCache[bestCandidates[i].system];
        if (isPreferredBase(*best->system, aRank, best->k, *bestCandidates[i].system, bRank, bestCandidates[i].k)) {
            // best remains best
        } else {
            best = &bestCandidates[i];
        }
    }
    
    // Step 7 (Task 10): the winning base may cover only k < N rings. The
    // remaining rings (the "remainder") are a contiguous prefix and/or
    // suffix of chainOrder, since isLinearChain was already enforced above.
    // If there's no remainder (k == N), return the same minimal base-only
    // result Task 8/9 already relied on -- unchanged behavior for every
    // already-passing k==N test (plain isoquinoline, plain naphthalene,
    // etc.).
    int remPrefixEnd = best->startIdx;              // chainOrder[0, remPrefixEnd)
    int remSuffixStart = best->startIdx + best->k;   // chainOrder[remSuffixStart, N)
    bool hasPrefixRemainder = remPrefixEnd > 0;
    bool hasSuffixRemainder = remSuffixStart < N;

    if (!hasPrefixRemainder && !hasSuffixRemainder) {
        RingSystemNameResult result;
        result.success = true;
        result.baseName = best->system->name;
        for (const auto &kv : best->atomMap) {
            result.locantMap[kv.first] = QString::number(kv.second);
        }
        return result;
    }

    // Two-sided remainder (rings left over on BOTH the prefix and suffix
    // side of the winning base): no test molecule exercising this shape
    // was found in this task's search budget, and the correct citation
    // assembly rule for "two different attached components, one on each
    // side of the base, each needing its own bracketed citation" was not
    // derived or verified against a real Blue Book example. Rather than
    // guess, bail out to the existing nameLinearChainFusion path (same as
    // if no retained match had been found at all).
    if (hasPrefixRemainder && hasSuffixRemainder) {
        // TODO(Task 10 follow-on): implement two-sided remainder citation
        // once a real ground-truth molecule needing it is identified.
        return std::nullopt;
    }

    // One-sided remainder: exactly one contiguous run of leftover rings,
    // either entirely before or entirely after the base in chainOrder.
    std::vector<int> remainderRingIndices;
    int boundaryRemainderRing;   // the remainder ring adjacent to the base
    int boundaryBaseRing;        // the base ring adjacent to the remainder
    if (hasPrefixRemainder) {
        for (int r = 0; r < remPrefixEnd; ++r) remainderRingIndices.push_back(chainOrder[r]);
        boundaryRemainderRing = chainOrder[remPrefixEnd - 1];
        boundaryBaseRing = chainOrder[remPrefixEnd];
    } else {
        for (int r = remSuffixStart; r < N; ++r) remainderRingIndices.push_back(chainOrder[r]);
        boundaryRemainderRing = chainOrder[remSuffixStart];
        boundaryBaseRing = chainOrder[remSuffixStart - 1];
    }

    // The two real-molecule atom indices shared between the boundary
    // remainder ring and the boundary base ring -- the actual fusion bond
    // between the attached component and the base. buildRingAdjacency's
    // `shared` matrix already confirmed this count is exactly 2 for every
    // adjacent pair in a valid linear chain.
    std::vector<int> fusionAtoms;
    for (int a : sssrRings[boundaryRemainderRing]) {
        if (sssrRings[boundaryBaseRing].count(a)) fusionAtoms.push_back(a);
    }
    if (fusionAtoms.size() != 2) {
        // Shouldn't happen for a valid linear chain, but don't guess.
        return std::nullopt;
    }

    // Collect the remainder's own ring atoms, then try matching the WHOLE
    // remainder ring-set against the registry (same approach as the main
    // k-loop above, but for this one fixed ring-set).
    std::set<int> remainderRingAtoms;
    for (int ringIdx : remainderRingIndices) {
        for (int a : sssrRings[ringIdx]) remainderRingAtoms.insert(a);
    }

    const RetainedRingSystem *remainderSys = nullptr;
    std::map<int, int> remainderAtomMap;
    bool remainderAmbiguous = false;
    for (const auto &sys : registry) {
        if (static_cast<int>(remainderRingAtoms.size()) != static_cast<int>(sys.peripheralLocants.size())) {
            continue;
        }
        std::map<int, int> candidateMap;
        if (matchesRetainedSystem(mol, remainderRingAtoms, sys, candidateMap)) {
            if (remainderSys != nullptr && remainderSys->name != sys.name) {
                remainderAmbiguous = true; // two different registry entries both match -- don't guess
            }
            remainderSys = &sys;
            remainderAtomMap = candidateMap;
        }
    }

    // No registry match for the remainder ring-set, more than one distinct
    // match (ambiguous), or a match whose fusionPrefix hasn't been verified
    // yet (see RetainedRingTypes.h's doc comment on fusionPrefix): fall
    // back to the existing monocyclic per-ring attached-component path.
    //
    // TODO(Task 10 follow-on): that monocyclic fallback (classify each
    // remainder ring's own RingType via the existing per-ring
    // classification logic elsewhere in IupacNamer.cpp, then cite each via
    // getFusionPrefixShared(RingType)) is NOT implemented here. No test
    // molecule in the current corpus was confirmed to need it (every
    // molecule that reaches this point either resolves via the naphthalene
    // case below or safely falls through to nameLinearChainFusion exactly
    // as it did before this task, since returning std::nullopt here is
    // behaviorally identical to the pre-Task-10 code's own partial,
    // unconsumed result for every such molecule). Implementing it without
    // a concrete ground-truth molecule to verify against risks emitting a
    // confidently wrong name instead of correctly falling through.
    if (remainderSys == nullptr || remainderAmbiguous || remainderSys->fusionPrefix.isEmpty()) {
        return std::nullopt;
    }

    // Translate the two real fusion-atom indices into each system's OWN
    // locant numbering via the reverse of their respective atomMaps
    // (query-position -> real-atom-index, inverted here to real-atom-index
    // -> that system's own locant string).
    auto reverseLocant = [](const std::map<int,int> &atomMap, const RetainedRingSystem &sys, int realAtomIdx) -> QString {
        for (const auto &kv : atomMap) {
            if (kv.second == realAtomIdx) {
                if (kv.first >= 0 && kv.first < static_cast<int>(sys.peripheralLocants.size())) {
                    return sys.peripheralLocants[kv.first];
                }
                return QString();
            }
        }
        return QString();
    };

    QString baseLocA = reverseLocant(best->atomMap, *best->system, fusionAtoms[0]);
    QString baseLocB = reverseLocant(best->atomMap, *best->system, fusionAtoms[1]);
    if (baseLocA.isEmpty() || baseLocB.isEmpty()) {
        return std::nullopt;
    }

    // Find the BASE's own bond-letter for this fusion bond (its
    // peripheralBondLetters pair, in either order, matching the two base
    // locants just computed). The base (isoquinoline in the ground-truth
    // case) is never symmetric when it contains the qualifying heteroatom,
    // so a single atomMap-derived labeling is sufficient here -- no
    // automorphism search needed on this side.
    char baseFaceLetter = 0;
    for (const auto &kv : best->system->peripheralBondLetters) {
        const QString &p1 = kv.second.first;
        const QString &p2 = kv.second.second;
        if ((p1 == baseLocA && p2 == baseLocB) || (p1 == baseLocB && p2 == baseLocA)) {
            baseFaceLetter = kv.first;
            break;
        }
    }
    if (baseFaceLetter == 0) {
        return std::nullopt; // No matching bond letter found -- don't guess.
    }

    // For the REMAINDER, a single atomMap-derived labeling is NOT enough:
    // the remainder system may have its own symmetry (naphthalene does --
    // see computeAutomorphisms' doc comment), so the single mapping
    // matchesRetainedSystem happened to return may label the fusion bond
    // with a HIGHER locant pair than Blue Book's "as low as possible"
    // rule requires. Find the real fusion atoms' query-positions under
    // the ORIGINAL remainder atomMap, then try every automorphism of the
    // remainder's own reference molecule to find every other equally-valid
    // labeling of those same two atoms, and keep the lowest.
    auto reverseQPos = [](const std::map<int,int> &atomMap, int realAtomIdx) -> int {
        for (const auto &kv : atomMap) {
            if (kv.second == realAtomIdx) return kv.first;
        }
        return -1;
    };
    int remQPos0 = reverseQPos(remainderAtomMap, fusionAtoms[0]);
    int remQPos1 = reverseQPos(remainderAtomMap, fusionAtoms[1]);
    if (remQPos0 < 0 || remQPos1 < 0) {
        return std::nullopt;
    }

    int remRefMol = indigoLoadMoleculeFromString(remainderSys->referenceSmiles.toUtf8().constData());
    if (remRefMol < 0) {
        return std::nullopt;
    }
    indigoAromatize(remRefMol);
    std::vector<std::vector<int>> remAutos = computeAutomorphisms(remRefMol);
    indigoFree(remRefMol);

    QString remCiteLoc1, remCiteLoc2;
    bool foundRemainderBond = false;
    for (const auto &perm : remAutos) {
        if (remQPos0 >= static_cast<int>(perm.size()) || remQPos1 >= static_cast<int>(perm.size())) continue;
        int altQ0 = perm[remQPos0];
        int altQ1 = perm[remQPos1];
        if (altQ0 < 0 || altQ0 >= static_cast<int>(remainderSys->peripheralLocants.size())) continue;
        if (altQ1 < 0 || altQ1 >= static_cast<int>(remainderSys->peripheralLocants.size())) continue;
        QString altLocA = remainderSys->peripheralLocants[altQ0];
        QString altLocB = remainderSys->peripheralLocants[altQ1];
        for (const auto &kv : remainderSys->peripheralBondLetters) {
            const QString &p1 = kv.second.first;
            const QString &p2 = kv.second.second;
            if ((p1 == altLocA && p2 == altLocB) || (p1 == altLocB && p2 == altLocA)) {
                // Compare candidates by their sorted-ascending locant pair
                // (Blue Book's own low-locants comparison), but CITE using
                // the stored pair order (already correct per-bond
                // convention, including the special wrap-bond case).
                QString sortedFirst = locantLess(p1, p2) ? p1 : p2;
                QString sortedSecond = locantLess(p1, p2) ? p2 : p1;
                if (!foundRemainderBond) {
                    remCiteLoc1 = p1; remCiteLoc2 = p2;
                    foundRemainderBond = true;
                } else {
                    QString curSortedFirst = locantLess(remCiteLoc1, remCiteLoc2) ? remCiteLoc1 : remCiteLoc2;
                    QString curSortedSecond = locantLess(remCiteLoc1, remCiteLoc2) ? remCiteLoc2 : remCiteLoc1;
                    bool better = locantLess(sortedFirst, curSortedFirst) ||
                                  (sortedFirst == curSortedFirst && locantLess(sortedSecond, curSortedSecond));
                    if (better) { remCiteLoc1 = p1; remCiteLoc2 = p2; }
                }
                break;
            }
        }
    }
    if (!foundRemainderBond) {
        return std::nullopt;
    }

    // Assemble: remainderPrefix + "[" + loc1 + "," + loc2 + "-" + letter + "]" + baseName
    // (same two-locants-then-letter convention IupacNamer.cpp's own
    // buildBlock lambda uses for a depth-1 attached component -- see its
    // `QString locsStr = QString("%1-%2").arg(formatLocants(...` call).
    RingSystemNameResult result;
    result.success = true;
    result.baseName = remainderSys->fusionPrefix + "[" + remCiteLoc1 + "," + remCiteLoc2 + "-" + QString(QChar(baseFaceLetter)) + "]" + best->system->name;
    for (const auto &kv : best->atomMap) {
        result.locantMap[kv.first] = QString::number(kv.second);
    }
    for (const auto &kv : remainderAtomMap) {
        // Offset remainder keys well clear of the base's own query-position
        // key space (small ints, bounded by the base registry entry's own
        // atom count) so the two maps' keys never collide -- nothing reads
        // these keys' actual values, only locantMap.size() downstream.
        result.locantMap[100000 + kv.first] = QString::number(kv.second);
    }

    return result;
}
