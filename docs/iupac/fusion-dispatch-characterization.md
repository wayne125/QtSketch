# Fusion-Dispatch Call Site Characterization

This document characterizes all three call sites for the fusion-naming functions
`nameLinearChainFusion` and `nameBranchingFusion` in the IUPAC namer codebase
(as of commit `0a2f2d5` on branch `dev`).

Both functions share the same signature:

```cpp
std::vector<RingSystemNameResult> nameLinearChainFusion(
    const Graph &g,
    int mol,
    const std::map<int, int> &indigoToGraphIdx,
    int ringCount,
    int freeValenceAtom = -1,
    const std::vector<std::set<int>> *ringSubsetOverride = nullptr,
    const std::vector<std::pair<int, QString>> &extraSubstituents = {});

std::vector<RingSystemNameResult> nameBranchingFusion(
    const Graph &g,
    int mol,
    const std::map<int, int> &indigoToGraphIdx,
    int ringCount,
    int freeValenceAtom = -1,
    const std::vector<std::set<int>> *ringSubsetOverride = nullptr,
    const std::vector<std::pair<int, QString>> &extraSubstituents = {});
```

Both functions start by creating a `FusionGraphContext` (introduced in an earlier task),
which wraps the ~51-line setup block that was previously duplicated at the top of
each function. The context computes SSSR rings (either from `ringSubsetOverride` or
via Indigo) and checks pairwise disjointness, providing `sssrRings` and `allDisjoint`
to the rest of the function logic.

---

## Call Site 1: `nameRingSystemAsSubstituent` (ring-as-substituent context)

**Location:** `IupacNamer.cpp:2870` and `2887`

**Function:** `nameBranchGraph` (line 3098, NOT its forward declaration at 1790) calls
`nameRingSystemAsSubstituent` (line 3122), which contains the actual fusion-dispatch
calls.

**Call expressions:**
```cpp
// Line 2870
std::vector<RingSystemNameResult> chainResults = 
    nameLinearChainFusion(g, mol, indigoToGraphIdx, ringCount, rootAtom);

// Line 2887
std::vector<RingSystemNameResult> branchResults = 
    nameBranchingFusion(g, mol, indigoToGraphIdx, ringCount, rootAtom);
```

### Input Construction

- **`g`**: The `Graph` object passed as parameter to `nameRingSystemAsSubstituent`
- **`mol`**: The molecule handle passed as parameter to `nameRingSystemAsSubstituent`
- **`indigoToGraphIdx`**: Built locally from `g.nodes` (lines 2812-2815):
  ```cpp
  std::map<int, int> indigoToGraphIdx;
  for (const auto &node : g.nodes) {
      indigoToGraphIdx[node.indigoIdx] = node.id;
  }
  ```
- **`ringCount`**: Computed via BFS traversal of ortho-fused rings (lines 2827-2849).
  Starts from the ring directly containing `rootAtom` (`ringSystemNodes`), then walks
  adjacency (two rings sharing exactly 2 atoms) to find the full transitively-connected
  fused system. Result is the size of `fusedSystemRingIdx` (line 2850).
- **`freeValenceAtom` (5th param)**: `rootAtom` (the attachment point of this ring
  system as a substituent)
- **`ringSubsetOverride` (6th param)**: NOT passed (defaults to `nullptr`)
- **`extraSubstituents` (7th param)**: NOT passed (defaults to `{}`)

### Fusion Function Assumptions

Both functions use `FusionGraphContext` to obtain SSSR rings and check disjointness.
With `ringSubsetOverride == nullptr`, they compute SSSR rings from `mol` via Indigo
(locally, not using any caller-provided ring information).

The functions' early logic checks:
- `nameLinearChainFusion`: If `ringCount < 3`, returns "Linear chain: no match" sentinel
- `nameBranchingFusion`: No explicit ringCount check, but internally checks for
  branching topology (degree > 2)

Both functions then build the `shared` matrix and `sharedNodesPairs` to identify
fusion connections, mirroring the logic in Site 1's own BFS traversal.

### Result Handling

Results from both functions are collected into a `candidates` vector (lines 2871-2873,
2888-2890). Each result is checked:
- If `result.success`: added to `candidates`
- If `result.error` is a "no match" sentinel: skipped (fall through to next mechanism)
- If `result.error` is a specific rejection message (e.g., "All-carbon fused ring
  systems..." for linear chain): skipped (fall through)
- Otherwise: returns empty string `""` immediately (hard failure)

After both mechanisms are tried, if `candidates` is non-empty, the candidate
minimizing `rootAtom`'s locant is selected (lines 3052-3074):
```cpp
RingSystemNameResult *bestCandidate = nullptr;
int bestLocant = INT_MAX;
for (auto &candidate : candidates) {
    // Check if rootAtom has a valid numeric locant
    auto it = candidate.locantMap.find(rootAtom);
    if (it == candidate.locantMap.end()) continue;
    QString locantStr = it->second;
    // Reject letter-suffixed locants (fusion carbons can't be attachment points)
    if (!locantStr.isEmpty() && locantStr.back().isLetter()) continue;
    bool ok = false;
    int locantValue = locantStr.toInt(&ok);
    if (ok && locantValue < bestLocant) {
        bestLocant = locantValue;
        bestCandidate = &candidate;
    }
}
```

If no valid candidate found, returns empty string.

The selected name is then formatted (lines 3081-3095) and returned wrapped in parentheses
with locant and "-yl" suffix: `"(" + stem + "-" + locantStr + "-yl" + ")"`

### Failure Handling

- Hard failures (non-sentinel errors) cause immediate `return ""`
- "No match" sentinels allow fall-through to next mechanism
- Empty candidates after all mechanisms: `return ""` (line 3049)

---

## Call Site 2: `generateNameImpl` (multi-component handling)

**Location:** `IupacNamer.cpp:23343` and `23349`

**Context:** Deep inside multi-component molecule handling, specifically for
molecules with exactly one real, non-trivial ring system (fused pair or ortho-fused
chain/branch) plus one or more separate, plain single rings elsewhere.

**Call expressions:**
```cpp
// Line 23343
auto chainRes = nameLinearChainFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, 
                                       &realComponentIndigo, extraSubs);

// Line 23349
auto branchRes = nameBranchingFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, 
                                       &realComponentIndigo, extraSubs);
```

### Input Construction

- **`g`**: The `Graph` object passed as parameter to `generateNameImpl`
- **`mol`**: The molecule handle passed as parameter to `generateNameImpl`
- **`indigoToGraphIdx`**: Passed as parameter to `generateNameImpl` (built at a higher
  level, typically from the molecule's atoms)
- **`NC`**: Set to `realComponentSize` (line 23310), which is the size of
  `realComponentIndigo` vector (line 22825: `int realComponentSize = 
  static_cast<int>(realComponentIndigo.size());`)
- **`freeValenceAtom`**: Computed as first element of `attachNodesSeen` or -1 (line 23337):
  ```cpp
  int freeValAtom = attachNodesSeen.empty() ? -1 : *attachNodesSeen.begin();
  ```
  Where `attachNodesSeen` contains the attachment points from isolated rings to the
  real component (lines 22896-22900).
- **`ringSubsetOverride`**: `&realComponentIndigo` (pointer to the vector of ring sets
  for the real component)
- **`extraSubstituents`**: `extraSubs` (line 22903: `std::vector<std::pair<int, QString>> 
  extraSubs;`), populated by naming each directly-attached isolated ring via
  `nameRingAsSubstituent` (lines 22905-22910):
  ```cpp
  for (size_t k = 0; k < isolatedAttachPairs.size(); ++k) {
      int attachReal = isolatedAttachPairs[k].first;
      int attachIso = isolatedAttachPairs[k].second;
      QString subName = nameRingAsSubstituent(g, isolatedGraphRings[k], attachIso, 
                                               attachReal, allSSSRRings, 
                                               realComponentGraphUnion, ...);
      if (subName.isEmpty()) { namesOk = false; break; }
      extraSubs.push_back({attachReal, subName});
  }
  ```

### Fusion Function Assumptions

With `ringSubsetOverride == &realComponentIndigo`, the `FusionGraphContext` uses the
provided ring subset instead of computing SSSR from `mol`. This is critical: the
multi-component context wants to name only the REAL component (the fused system),
not the entire molecule (which includes isolated rings).

The `extraSubstituents` parameter provides pre-named substituent information for
the isolated rings attached to the real component. This allows the fusion functions
to incorporate these substituents into their numbering without having to re-detect
and re-name them.

### Result Handling

Results are checked directly for success (lines 23344-23346, 23350-23352):
```cpp
// For linear chain
if (!chainRes.empty() && chainRes[0].success) {
    mcSuccess = true;
    mcName = chainRes[0].baseName;
}

// For branching
if (!branchRes.empty() && branchRes[0].success) {
    mcSuccess = true;
    mcName = branchRes[0].baseName;
}
```

Only the first element of the result vector is used (`chainRes[0]` or `branchRes[0]`).
The `baseName` is used directly as the component name.

If either mechanism succeeds, `mcSuccess` is set to true and `mcName` to the result,
and the function returns early (line 23361: `return {true, mcName, ""};`).

### Failure Handling

- If neither mechanism succeeds, falls through to later checks (line 23354-23362)
- The calls are guarded by topology checks (lines 23339-23354):
  - Linear chain: only called if `validFusionC && endsC == 2 && centersC == NC - 2 && NC == 3`
  - Branching: only called if `validFusionC && hasBranchingC`
- No sentinel error checking - uses direct success/failure

---

## Call Site 3: `generateNameImpl` (retained-registry pre-check)

**Location:** `IupacNamer.cpp:23566` and `23634`

**Context:** Inside `generateNameImpl`, near the retained-registry pre-check, immediately
after the Task C InChIKey-override return falls through.

**Call expressions:**
```cpp
// Line 23566
chainResults = nameLinearChainFusion(g, mol, indigoToGraphIdx, ringCount);

// Line 23634
branchResults = nameBranchingFusion(g, mol, indigoToGraphIdx, ringCount);
```

### Input Construction

- **`g`**: The `Graph` object passed as parameter to `generateNameImpl`
- **`mol`**: The molecule handle passed as parameter to `generateNameImpl`
- **`indigoToGraphIdx`**: Passed as parameter to `generateNameImpl`
- **`ringCount`**: Computed from `sssrRings.size()` where `sssrRings` is built by
  iterating SSSR via Indigo (lines 23372-23392):
  ```cpp
  int sssrIter = indigoIterateSSSR(mol);
  std::vector<std::set<int>> sssrRings;
  // ... iterate and build sssrRings ...
  int ringCount = static_cast<int>(sssrRings.size());
  ```
- **`freeValenceAtom` (5th param)**: NOT passed (defaults to -1)
- **`ringSubsetOverride` (6th param)**: NOT passed (defaults to `nullptr`)
- **`extraSubstituents` (7th param)**: NOT passed (defaults to `{}`)

### Fusion Function Assumptions

With all optional parameters at defaults, the functions compute SSSR from `mol`
via `FusionGraphContext` and proceed with standard logic.

These calls are guarded by topology checks computed in the same scope (lines
23372-23410):
```cpp
int N = sssrRings.size();
std::vector<std::vector<int>> shared(N, std::vector<int>(N, 0));
// ... build shared matrix ...
std::vector<int> degree(N, 0);
bool validFusion = true;
for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
        if (shared[i][j] == 2) degree[i]++;
        else if (shared[i][j] != 0) validFusion = false;
    }
}
int ends = 0, centers = 0;
bool hasBranching = false;
for (int i = 0; i < N; ++i) {
    if (degree[i] == 1) ends++;
    else if (degree[i] == 2) centers++;
    else if (degree[i] > 2) hasBranching = true;
}
```

### Result Handling

Results are assigned to local variables and checked (lines 23566-23568, 23634-23636):
```cpp
// For linear chain
std::vector<RingSystemNameResult> chainResults;
if (validFusion && ends == 2 && centers == N - 2) {
    chainResults = nameLinearChainFusion(g, mol, indigoToGraphIdx, ringCount);
    linearChainApplies = !chainResults.empty() && 
                        chainResults[0].error != "Linear chain: no match";
}

// For branching
std::vector<RingSystemNameResult> branchResults;
if (validFusion && hasBranching) {
    branchResults = nameBranchingFusion(g, mol, indigoToGraphIdx, ringCount);
    branchingApplies = !branchResults.empty() && 
                      branchResults[0].error != "Branching fusion: no match";
}
```

Note the sentinel checking: `chainResults[0].error != "Linear chain: no match"` and
`branchResults[0].error != "Branching fusion: no match"`. This differs from Site 1,
which checks each element in the vector.

If linear chain applies and succeeds (line 23621-23623):
```cpp
// linearChainApplies guarantees chainResults[0] exists and isn't "no match"
// the only remaining case is success.
return {true, chainResults[0].baseName, ""};
```

For branching, if applies and succeeds (lines 23637-23649), returns the baseName.

Special error handling for exocyclic substituents (lines 23573-23619 for linear,
23641-23659 for branching): if the error is "Substituents on fused ring systems
named via this mechanism are not yet supported.", it performs chain-vs-ring
seniority check and may fall back to chain naming.

Other errors cause immediate return (line 23619: `return {false, "", chainResults[0].error};`)

### Failure Handling

- Sentinel "no match" errors allow fall-through
- Specific rejection errors may trigger fallback logic
- Other errors cause immediate rejection
- Unhandled cases fall through to later mechanism checks

---

## Contract Comparison

### Parameter Passing Summary

| Parameter | Site 1 | Site 2 | Site 3 |
|-----------|--------|--------|--------|
| `g` | From function param | From function param | From function param |
| `mol` | From function param | From function param | From function param |
| `indigoToGraphIdx` | Built locally from `g.nodes` | From function param | From function param |
| `ringCount` | BFS-computed fused system size | `realComponentSize` | `sssrRings.size()` |
| `freeValenceAtom` | `rootAtom` (explicit) | `freeValAtom` (explicit) | -1 (default) |
| `ringSubsetOverride` | `nullptr` (default) | `&realComponentIndigo` (explicit) | `nullptr` (default) |
| `extraSubstituents` | `{}` (default) | `extraSubs` (explicit) | `{}` (default) |

### Key Differences

1. **`indigoToGraphIdx` construction**:
   - Site 1: Built locally from `g.nodes` (lines 2812-2815)
   - Site 2 & 3: Passed from caller (built at higher level)
   - **Impact**: Site 1 is self-contained; Sites 2 & 3 rely on external construction

2. **`ringSubsetOverride` usage**:
   - Site 1: Uses `nullptr` → functions compute SSSR from `mol`
   - Site 2: Uses `&realComponentIndigo` → functions use provided ring subset
   - Site 3: Uses `nullptr` → functions compute SSSR from `mol`
   - **Impact**: Site 2 constrains the functions to a specific ring subset (the real
     component); Sites 1 & 3 let functions detect all rings in `mol`

3. **`extraSubstituents` usage**:
   - Site 1: Uses `{}` → functions detect all substituents internally
   - Site 2: Uses `extraSubs` → functions incorporate pre-named isolated ring substituents
   - Site 3: Uses `{}` → functions detect all substituents internally
   - **Impact**: Site 2 provides pre-computed substituent names; Sites 1 & 3 require
     functions to name substituents themselves

4. **`freeValenceAtom` usage**:
   - Site 1: Uses `rootAtom` (the substituent attachment point)
   - Site 2: Uses first attachment point or -1
   - Site 3: Uses -1 (no specific free valence atom)
   - **Impact**: Sites 1 & 2 provide a free valence atom for the functions to skip
     in substituent detection; Site 3 does not

5. **Result handling**:
   - Site 1: Collects ALL results, filters sentinels, picks best by locant minimization
   - Site 2: Uses only `result[0]`, checks `.success` directly
   - Site 3: Uses only `result[0]`, checks both `.success` and sentinel errors

### Structural Similarities

- Sites 1 and 3 both use default `ringSubsetOverride` and `extraSubstituents`
- Sites 1 and 3 both let functions compute SSSR from `mol`
- Sites 1 and 2 both provide explicit `freeValenceAtom` values
- Sites 2 and 3 are both in `generateNameImpl` with similar topology-checking logic

### Blocking Differences

**Site 2 differs from Sites 1 and 3 in two critical ways:**

1. **`ringSubsetOverride`**: Site 2 passes `&realComponentIndigo` to constrain the
   fusion functions to only the real component's rings. Sites 1 and 3 pass `nullptr`,
   causing the functions to compute SSSR from the entire `mol` and consider all rings.
   This is a fundamental difference in scope: Site 2 is naming a specific subset of
   the molecule's rings, while Sites 1 and 3 name all rings in the molecule (or the
   provided graph).

2. **`extraSubstituents`**: Site 2 passes pre-computed substituent names for isolated
   rings attached to the real component. Sites 1 and 3 pass empty defaults, requiring
   the fusion functions to detect and name all substituents internally.

These two differences mean Site 2's contract is **fundamentally different** from
Sites 1 and 3. Site 2 is operating in a multi-component naming context where:
- Only a subset of rings (the "real component") should be named
- Isolated ring substituents have already been named and must be incorporated

In contrast, Sites 1 and 3 operate in single-ring-system contexts where:
- All rings in the relevant scope (entire graph or entire molecule) should be considered
- All substituents must be detected and named by the fusion functions themselves

### Sites 1 and 3: Can They Be Unified?

Sites 1 and 3 have the same parameter pattern (using defaults for params 5-7), but
they differ in one important way:

- **Site 1**: `indigoToGraphIdx` is built locally from `g.nodes`
- **Site 3**: `indigoToGraphIdx` is passed from the caller

However, this difference is superficial: both provide the same mapping, just
constructed at different scopes. The functions only use `indigoToGraphIdx` for
looking up graph indices from Indigo atom indices, and the mapping is identical
in both cases.

More importantly, both sites:
- Use `nullptr` for `ringSubsetOverride`
- Use `{}` for `extraSubstituents`
- Pass an explicit value for `freeValenceAtom` (Site 1) or rely on default -1 (Site 3)
- Operate on a single, coherent ring system

**The freeValenceAtom difference**:
- Site 1: Passes `rootAtom` as the free valence atom (the attachment point for
  this ring system as a substituent)
- Site 3: Passes -1 (no free valence atom)

In `nameLinearChainFusion` and `nameBranchingFusion`, `freeValenceAtom` is used
to skip that atom during substituent detection (to avoid treating the
attachment point as a substituent). In Site 1's context (ring-as-substituent),
this is correct: `rootAtom` is where the ring system attaches to the parent
molecule, so it should not be treated as a substituent on the ring system itself.

In Site 3's context (whole-molecule naming), there is no single attachment point
-to-skip, so -1 is appropriate.

**Conclusion**: Sites 1 and 3 have **different semantics for freeValenceAtom**.
Site 1 needs to exclude the attachment point; Site 3 has no attachment point to
exclude. This is a real, functional difference, not just a parameter-passing
convention.

However, looking at the actual usage: Site 1 passes `rootAtom` as the 5th positional
parameter, which maps to `freeValenceAtom`. Site 3 does NOT pass a 5th parameter,
so it defaults to -1. This is a **parameter count mismatch** that happens to work
because of default parameters.

If we wanted to unify Sites 1 and 3, we would need to either:
- Make Site 3 explicitly pass -1 as the 5th parameter, OR
- Create a wrapper that always passes all parameters explicitly

But this would be a cosmetic change, not a functional unification.

---

## Final Conclusion

**Can the call sites be unified?**

**NO** - Site 2 has a fundamentally different contract from Sites 1 and 3:

1. Site 2 passes `ringSubsetOverride` to constrain the fusion to a specific ring subset
2. Site 2 passes `extraSubstituents` with pre-named isolated ring substituents

These parameters change the semantic behavior of the fusion functions in ways
that cannot be reconciled without breaking Site 2's multi-component naming logic.

**Sites 1 and 3 have matching contracts in terms of ring detection and substituent
handling** (both use defaults for `ringSubsetOverride` and `extraSubstituents`),
but they differ in:
- The value of `freeValenceAtom` (Site 1: `rootAtom`, Site 3: -1)
- The construction of `indigoToGraphIdx` (local vs. passed)

These differences are **context-appropriate** rather than arbitrary:
- Site 1 needs to exclude the attachment point (`rootAtom`) from substituent detection
- Site 3 has no attachment point to exclude

A forced unification would require either:
- Adding a parameter to control whether to skip a free valence atom (but it
  already exists - it's `freeValenceAtom`)
- Or always passing -1 and having Site 1's caller filter out `rootAtom` from
  results (which would break the existing logic)

**Recommendation**: Keep all three call sites separate and documented as-is.

The specific blocking difference is:
- Site 2's use of `ringSubsetOverride` and `extraSubstituents` creates a fundamentally
  different naming context (multi-component with pre-named substituents) that
  cannot be unified with the single-ring-system contexts of Sites 1 and 3 without
  breaking the multi-component naming functionality.

Even between Sites 1 and 3, the `freeValenceAtom` parameter serves different purposes
(root atom exclusion vs. no exclusion), making unification awkward and potentially
confusing without clear benefit.
