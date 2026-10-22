# generateNameImpl Internal Structure Map

This document maps the internal structure of `IupacNamer::generateNameImpl`
in `src/app/IupacNamer.cpp`. `generateNameImpl` spans **lines 8745-23967
(15,223 lines)**: declaration at line 8745, closing brace at line 23967,
blank line 23968, then the next top-level function,
`computePeripheralNumbering3Ring`, at line 23969.

**Note on entry points:** The public entry point is the thin wrapper
`IupacNamer::generateName` (comment starts line 8726, declaration line
8730, closing brace line 8737), which calls `generateNameImpl` and
normalizes the `status` field. This document maps the real logic in
`generateNameImpl`.

**Correction history:** this is the fourth version of this document.
- v1 (`baeae90`) mis-stated the function's end by ~800 lines and
  fabricated the content of the final section.
- v2 (`79f51bb`) fixed those but described two separate lambdas as one
  combined block, omitted an entire branch of a fusion-dispatch section,
  and had a systematic off-by-one from computing numbers via offset
  arithmetic from an earlier commit instead of re-reading the file.
- v3 (`a01d275`) fixed the structural errors (both lambdas, the missing
  branch) but the SAME commit also edited the source pointer comment
  above `generateNameImpl` (growing it from 3 lines to 5), and every line
  number in that version's doc was read from the tree BEFORE that source
  edit landed — so the doc shipped already stale by +2 lines relative to
  its own commit's actual final state. This is the same root-cause
  mistake as v2, recurring in a new form: computing/reading numbers
  against a tree that is not the one actually being committed.
- This version (v4) was corrected against `a01d275`'s actual committed
  `HEAD:src/app/IupacNamer.cpp` (via `git show HEAD:...`, not the working
  tree, not a prior commit), and **does not touch `src/app/IupacNamer.cpp`
  at all** — so this commit's own numbers cannot go stale the same way
  again. If a future task edits that file (including the pointer comment
  itself), every line number below will need re-verifying against
  whatever becomes the new committed HEAD at that time, not derived by
  adding/subtracting an offset.

---

## Major Dispatch Sections

### Section 1: Entry and Preprocessing (lines 8745-11790)

**Purpose:** Input validation, structural normalization, early-exit
special cases, Indigo-to-graph index construction.

**Real order (corrected — a prior version had this sequence wrong):**
1. **Validation and early reject** (~8745-8749): `if (mol < 0) return {false, "", "Invalid molecule handle."};`
2. **Sulfoxide normalization** (~8750-8838): charge-separated sulfoxide
   R2S(+1)-O(-1) → neutral R2S=O. Mutates `mol` in place via Indigo calls.
   Isotope-label handling (`elementIsotope`) appears inside this block,
   roughly lines 8851-8874.
3. **Bare inorganic (zero-carbon) ion lookup** (~8840-~8925): exact-match
   lookup for zero-carbon molecules only (halides, oxides, metals,
   polyatomic ions). Returns immediately on match.
4. **Aromatization and ring count**: `indigoAromatize(mol)`, then
   `int ringCount = indigoCountSSSR(mol);` at line 8928 — computed exactly
   once, read (never recomputed) by every later ring-count-guarded
   section.
5. **Salt naming (P-77)** (~8932-9779): 2-component salts starting ~8932,
   3-component salts starting ~9380. Uses `indigoComponent()`,
   `indigoClone()`, and recursive `generateName()` calls on cloned
   components.
6. **Standalone, carbon-containing ion blocks** (~9779-11786) — NOT
   zero-carbon (unlike block 3 above); each is its own guarded lookup:
   standalone carboxylate anion (~9779, isotope handling also appears
   here, roughly 9787-9826), alkoxide (~10001), ammonium (~10233),
   N-oxide (~10424), pyridinium (~10616), saturated N-heterocyclic
   quaternary ammonium (~10875), symmetric quaternary ammonium (~11162).
7. **Indigo-to-graph index construction** (line 11790):
   `std::map<int, int> indigoToGraphIdx;` — atom iteration starts ~11786.

**Data produced:** `indigoToGraphIdx` (line 11790); `ringCount` (line
8928); early-exit `IupacResult` for ions/salts; a normalized (possibly
mutated) `mol`.

**Data consumed:** None (entry point).

---

### Section 2: Graph Construction (lines ~11791-12069)

**Purpose:** Build the `Graph g` object the rest of the function (and
every function it calls) operates on.

- `Graph g;` — line 12018, nodes built from heavy atoms with implicit +
  explicit hydrogen counts.
- Bond iteration populates `g.nodes[*].neighbors`/`bondOrders` from
  `indigoIterateBonds(mol)`.

**Data produced:** `Graph g` (line 12018) — read by every subsequent
section.

---

### Section 3: Organometallic Compound Handling (lines 12070-~12228)

**Real header (line 12070):** `// --- Organometallic compounds, Group 1-2 metals (P-69.3) ---`

**Purpose:** Name organometallic compounds with exactly one Group 1 or
Group 2 metal atom.

**Indigo call density:** measured ~17 calls across this section.

---

### Section 4: Functional-Group Narrow Cases (lines ~12229-15142)

**Purpose:** Specific, individually-recognized functional-group patterns.
Each sub-block is self-contained, returns `IupacResult` directly on
match, falls through otherwise.

**Real sub-block header lines (read directly from committed HEAD):**

| Sub-block | Real line |
|---|---|
| P-67.2 dinuclear phosphorus oxoacids | 12229 |
| P-67.2 dinuclear arsenic oxoacids | 12324 |
| P-67.2 dinuclear boron oxoacids | 12419 |
| P-67.2 dinuclear antimony oxoacids | 12503 |
| Amidoximes (P-66.4.4) | 12598 |
| N-substituted diamines (P-62.2.4) | 12675 |
| N-substituted amines (P-62.2.4) | 13137 |
| N-substituted exocyclic amines on saturated N-heterocycle | 13583 |
| Carbamate esters (P-65.2.1.1) | 13919 |
| N-substituted amides (P-66.1.1.3.1), incl. imide rejection | 14002 |
| N-substituted sulfonamides/sulfinamides | 14146 |
| N-substituted imines (P-62.3) | 14362 |
| N'-substituted amidines (P-66.4) | 14444 |
| N'-substituted guanidine (P-66.4.1.2.1) | 14514 |
| Ureas | 14595 |
| N-substituted thioamides (P-66.1.1.3.1), incl. imide rejection | 14695 |
| N-substituted hydrazides (P-66.3.3.1) | 14824 |
| P-67.2 dinuclear sulfur oxoacids | 15051 |

**Indigo call density:** measured ~39 calls across this section.

---

### Section 5: Ring-Substituent Pre-pass, then TWO Lambdas (lines ~15143-18368)

**Pre-pass (~15143-16088):** functional-group detection for ring
substituents (builds maps like `carbonSulfonicAcid` starting ~15143),
then `struct RingSubstituentInfo` (line 15471) and
`std::vector<RingSubstituentInfo> ringSubstituentInfos;` (line 15476,
read by Section 6's guard below), then the two "forced ring" variables
used to let later sections force a specific ring into the first lambda's
monocyclic-parent role:
```cpp
std::set<int> phase2ForcedRingNodeSet;                       // line 16089
std::map<int, QString> phase2ForcedRingSubstituentNames;      // line 16090
```

**Lambda 1 — `namePhase2Monocyclic`** (declared line 16092; its own
header comment, `// --- Phase 2: Monocyclic Ring Path (ringCount == 1) ---`,
is on the line immediately AFTER the `auto` line, not on the same line;
closes `};` at line 17563 — **1,472 lines**):
```cpp
auto namePhase2Monocyclic = [&]() -> IupacResult { ... };
```
Called from: line 23781 (Section 12's disjoint branch), lines
23875/23884/23898/23908 (Section 13 — **4 call sites**, not 3), and the
function's own final line, 23966.

**Lambda 2 — `nameNaphthaleneFusion`** (declared line 17565, closes `};`
at line 18368 — **804 lines**):
```cpp
auto nameNaphthaleneFusion = [&](const std::set<int> &ring1Nodes,
                                  const std::set<int> &ring2Nodes,
                                  const std::vector<std::pair<int, QString>> &extraSubstituents = {}) -> IupacResult { ... };
```
An inline lambda, not a separately-defined function, despite the name.
Measured ZERO direct Indigo calls. Called from: line 22927 (Section 11)
and line 23963 (Section 13).

**Data produced:** both lambdas return `IupacResult` when called;
`namePhase2Monocyclic` reads `phase2ForcedRingNodeSet`/
`phase2ForcedRingSubstituentNames`.

---

### Section 6: Acyclic Naming Path (Phase 1) (lines 18370-20164)

**Guard (line 18370):** `if (ringCount == 0 || !ringSubstituentInfos.empty())`

**Purpose:** Name acyclic compounds and ring-substituted chains, via a
`carbonGroup` map built from pure graph traversal, principal-group
suffix selection, and chain parent selection via
`nameAcyclicChainParentWithSubstituents`.

**Indigo call density: measured ZERO.** This section does all
functional-group detection via pure traversal of the already-built `g`.

---

### Section 7: Phase 26 — Retained Names for Adamantane and Cubane (lines ~20165-20467)

**Real header (line 20165):** `// --- Phase 26: Retained Names for Adamantane and Cubane ---`

---

### Section 8: Phase 27 — Saturated Unsubstituted Bicyclic Hydrocarbons (lines 20468-20657)

**Real header (line 20468):** `// --- Phase 27: Saturated Unsubstituted Bicyclic Hydrocarbon Path (von Baeyer: bicyclo[a.b.c]alkane) ---`

**Guard:** `if (ringCount == 2)`.

---

### Section 9: Spiro and Dispiro Hydrocarbon Paths (Phases 28, 29, 29.5) (lines 20658-21323)

**Real header lines, in actual file order:**

| Phase | Real line |
|---|---|
| Phase 29.5 (Dispiroter[bicyclo[a.b.c]alkane], P-24.4.1) | 20658 |
| Phase 29 (Spirobi[bicyclo[a.b.c]alkane]) | 20802 |
| Phase 28 (general spiro[a.b]alkane) | 20985 |

---

### Section 10: Special Fused/Heterocyclic Narrow Cases (Phases 30.5-35) (lines 21324-22746)

**Real header lines, in actual file order:**

| Phase | Real line |
|---|---|
| Phase 30.5 (1,4-epoxynaphthalene, P-25.4; guard `allSSSRRings.size() == 3`) | 21324 |
| Detect unsupported fusion topologies (bare guarded block, no phase number) | 21550 |
| Phase 31 (Benzo-fused Heterobicyclic Ring Systems, ringCount==2) | 21639 |
| Phase 32 (Purine) | 21780 |
| Phase 33 (Pentalene) | 21897 |
| Phase 34 (3H-Pyrrolizine) | 22002 |
| Phase 35 (General Two-Heterocycle Fusion Nomenclature) | 22121 |

This section ends right where Section 11 begins (line 22747) — no
overlap.

---

### Section 11: Multi-Component Ring Dispatch — Fusion-Dispatch Site 2 (lines 22747-23376)

**Real header (line 22747):** `// --- Multi-component ring dispatch: a molecule with exactly one real, ...`

**Purpose:** Molecules with exactly one "real" fused ring system plus one
or more separate, plain single rings elsewhere. Characterized in Task
G's `docs/iupac/fusion-dispatch-characterization.md` as **Site 2** (that
document's own line citations for this site have drifted a few lines
from successive edits to this file since it was written — not fixed
here, since Task G's document is already reviewed and closed).

**Structure:** extract SSSR rings; union-find to separate the "real"
component from isolated single rings; name each isolated ring as a
substituent via `nameRingAsSubstituent`, collecting results into
`extraSubs`; also directly calls the Section 5 lambda
`nameNaphthaleneFusion(ring1G, ring2G, extraSubs)` at line 22927 for a
2-ring real component.

**Fusion-dispatch Site 2 calls (real lines 23349, 23355):**
```cpp
auto chainRes  = nameLinearChainFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
auto branchRes = nameBranchingFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
```
Only `result[0].success` is checked; returns early on either success.

---

### Section 12: Fusion-Dispatch Site 3, Retained-Registry Pre-Check, AND the Disjoint-Ring Branch (lines 23378-23786)

**Real header (line 23377, one line before the guard):** `// --- Phase 44 & 48: N-Heterocycle Fusion Chain Nomenclature ---`

**Guard (line 23378):** `if (ringCount >= 3)`

**Two branches on `allDisjoint`:**

- **`!allDisjoint` branch** (the fused case, starts ~23414): extract SSSR
  rings; build the `shared` matrix and `degree`/`validFusion`/`ends`/
  `centers`/`hasBranching`; call `tryRetainedBaseDecomposition(mol)`
  (line ~23498) — on success, check the Task C InChIKey override set and
  return a `PreferredIupacName`-/`SystematicIupacName`-tagged result;
  otherwise fall through to fusion-dispatch **Site 3**'s
  `nameLinearChainFusion` (line ~23572) / `nameBranchingFusion` (line
  ~23640) calls (default parameters, also characterized in Task G's
  document).
- **`allDisjoint == true` branch** (starts ~23700): map the SSSR rings to
  graph indices; a local `isParentEligible` (line ~23714) determines
  whether exactly one ring is "parent-eligible" (heteroatom, or exocyclic
  double/triple bond to O/N/S); if so, name every OTHER ring as a
  substituent via `nameRingAsSubstituent` (line ~23769), write
  `phase2ForcedRingSubstituentNames`/`phase2ForcedRingNodeSet`, and call
  `namePhase2Monocyclic()` at **line 23781** — this is the FIRST of that
  lambda's 5 real call sites, not inside Section 13.
- **Both branches end the same way**: an UNCONDITIONAL
  `return {false, "", "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."};`
  at **line 23785** if nothing above matched. Section 12 therefore ALWAYS
  returns — it never falls through to Section 13, which has its own
  separate, mutually-exclusive `ringCount == 2` guard anyway.

**Data written:** `chainResults`/`branchResults` (fused branch);
`phase2ForcedRingSubstituentNames`/`phase2ForcedRingNodeSet` (disjoint
branch — the SAME two variables Section 13 also writes; see the
data-flow table below).

---

### Section 13: Ortho-Fused Bicyclic Path — Final Fallback (lines 23788-23967)

**Real header (line 23788):** `// --- Phase 3: Ortho-fused Bicyclic Aromatic Path (ringCount == 2: Naphthalene) ---`

**Guard:** `if (ringCount == 2)`

Content (read in full, re-confirmed across 3 review rounds now):

1. Extract exactly 2 SSSR rings (reject if not exactly 2).
2. Check whether the two rings are disjoint (share zero atoms) or fused.
3. **If disjoint:** the same `isParentEligible` criteria as Section 12's
   disjoint branch (defined again locally here). Exactly one eligible →
   name the other as a substituent, force it via
   `phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames`, call
   `namePhase2Monocyclic()` — **4 call sites here**, at lines 23875,
   23884, 23898, and 23908 for the various eligibility branches. Both
   eligible → break the tie via `compareRingSeniority` (P-44.2.1 criteria
   a-g); a complete tie returns
   `"Two ring systems of equal seniority (P-44.2.1) are not yet supported as parent vs. substituent."`.
   Neither eligible → check the **biphenyl special case** (two
   unsubstituted, all-carbon, all-aromatic 6-rings) and return
   `"biphenyl"` directly if it matches.
4. **Fallback for everything else, including the fused case:**
   `return nameNaphthaleneFusion(ring1Nodes, ring2Nodes);` at **line
   23963** — calls the Section 5 lambda, not a separate function.
5. **The function's absolute final line, 23966:**
   `return namePhase2Monocyclic();` — the last-resort fallback if nothing
   in Sections 1-13 matched, including this section's own guard not
   firing.

---

## Cross-Section Data Flow

| State | Produced at | Consumed by |
|---|---|---|
| `indigoToGraphIdx` | Line 11790 | Every subsequent section |
| `Graph g` | Line 12018 | Every subsequent section |
| `int ringCount` | Line 8928 | Every ring-count-guarded section (3, 6, 7, 8, 9, 10, 11, 12, 13) |
| `ringSubstituentInfos` | Section 5's pre-pass (line 15476) | Section 6's guard; Section 5's `namePhase2Monocyclic` lambda |
| `phase2ForcedRingNodeSet` / `phase2ForcedRingSubstituentNames` | Declared empty in Section 5's pre-pass (lines 16089-16090); **WRITTEN by BOTH Section 12's disjoint branch (line 23781's surrounding code) AND Section 13** | Read by Section 5's `namePhase2Monocyclic` lambda |
| `extraSubs`, `realComponentIndigo` | Section 11 | Section 11's own Site 2 calls and its `nameNaphthaleneFusion` call |
| `chainResults`/`branchResults` | Section 12's fused branch | Section 12's own Site 3 handling only |

**Critical dependency:** `mol` is mutated in place by Section 1 (sulfoxide
normalization, aromatization) before `g`/`indigoToGraphIdx` are built.

**Note on SSSR re-extraction:** SSSR rings are independently re-extracted
by Indigo in Sections 8, 10, 11, 12, and 13 — each extraction is local to
its own section, never shared or cached.

---

## Indigo Call-Site Density by Section (measured)

| Section | Lines | Indigo calls |
|---|---|---|
| 1-2 (Entry + graph construction) | 8745-12069 | ~81% of the function's total |
| 3 (Organometallic) | 12070-~12228 | ~17 |
| 4 (Narrow cases) | ~12229-15142 | ~39 |
| 6 (Acyclic, Phase 1) | 18370-20164 | **0** |
| 13 (Phase 3 final fallback) | 23788-23967 | 9 |
| **Whole function** | 8745-23967 | **865** (line-count basis; 866 raw occurrences) |

Re-run `grep -c "indigo[A-Z][A-Za-z]*("` over any of these ranges yourself
to re-verify if the line ranges shift in a future task.

---

## Fusion-Dispatch Call Sites Summary

This function contains **two of the three** fusion-dispatch call sites
characterized in Task G (Site 1 is in `nameRingSystemAsSubstituent`, a
different function). See `docs/iupac/fusion-dispatch-characterization.md`
for the full, already-reviewed characterization.

| Site | Section (above) | Key parameters | Result handling |
|---|---|---|---|
| **Site 2** | Section 11 (real lines 23349, 23355) | `ringSubsetOverride=&realComponentIndigo`, `extraSubstituents=extraSubs` | `result[0].success` only |
| **Site 3** | Section 12's `!allDisjoint` branch | all defaults | `result[0]`, checks `.success` + sentinel strings |

---

## Key Observations for Future Extraction

1. **`g` and `indigoToGraphIdx` are the core shared state** (lines 12018,
   11790), read by virtually every section.
2. **`mol` is mutated in-place** in Section 1, before `g` is built.
3. **SSSR rings are independently re-extracted** in Sections 8, 10, 11,
   12, and 13 — never cached or shared.
4. **Section 5 contains the function's two largest reusable units**:
   `namePhase2Monocyclic` (1,472 lines, 5 call sites across Sections 12
   and 13 plus the function's own final line) and `nameNaphthaleneFusion`
   (804 lines, 2 call sites across Sections 11 and 13). Both are already
   lambda-isolated — the most natural extraction candidates if this
   function is ever decomposed — but they are two DIFFERENT units with
   different signatures and purposes, not one combined block.
5. **`phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames` cross a
   THREE-way boundary**: declared in Section 5, written by BOTH Section
   12's disjoint branch and Section 13, read by Section 5's
   `namePhase2Monocyclic` lambda.
6. **Ring-count guards are the dominant dispatch mechanism** (`ringCount
   == 0`, `== 1` (via the lambda), `== 2` (Sections 8, 10 parts, 13),
   `>= 3` (Section 12)).
7. **Section 12 always returns; it is not a fall-through section.**
   Execution cannot reach Section 13 from Section 12 — they are mutually
   exclusive via their guards (`>= 3` vs `== 2`), and Section 12's block
   has no path that doesn't end in a `return`.
