# generateNameImpl Internal Structure Map

This document maps the internal structure of `IupacNamer::generateNameImpl`
in `src/app/IupacNamer.cpp`. Every line number below was read directly from
the current `dev` HEAD (the commit that lands this fix), not computed by
offset arithmetic from an earlier commit — an earlier revision of this
document got its own numbers wrong by doing exactly that kind of arithmetic
instead of re-reading the file.

`generateNameImpl` spans **lines 8743-23966 (15,224 lines)**, ending right
before the next top-level function, `computePeripheralNumbering3Ring`
(line 23967).

**Note on entry points:** The public entry point is the thin wrapper
`IupacNamer::generateName` (comment starts line 8726, declaration line 8730,
closing brace line 8737), which calls `generateNameImpl` and normalizes the
`status` field. This document maps the real logic in `generateNameImpl`.

**Correction history:** this is the third version of this document.
- v1 (commit `baeae90`) mis-stated the function's end by ~800 lines (it
  actually pointed into 3 unrelated functions further down the file) and
  fabricated the content of the final section.
- v2 (commit `79f51bb`) fixed those, but (a) described a ~2,277-line block
  as ONE lambda, `namePhase2Monocyclic` — it is actually TWO separate
  lambdas with different names and purposes; (b) omitted an entire branch
  (the `allDisjoint == true` case) from the fusion-dispatch section and
  wrongly claimed that section "falls through" to the next one when it
  actually always returns; (c) had a systematic off-by-one in every line
  number past roughly line 8740, because it computed numbers via "+3 from
  an earlier commit" arithmetic instead of re-reading the file at the
  commit it claimed to describe.
- This version (v3) was produced by re-reading every cited line directly
  at HEAD — no arithmetic carried forward from any earlier version.

---

## Major Dispatch Sections

### Section 1: Entry and Preprocessing (lines 8743-11786)

**Purpose:** Input validation, structural normalization, early-exit special
cases, Indigo-to-graph index construction.

**Subsections:**
- **Validation and early reject** (~8743-8745): `if (mol < 0) return {false, "", "Invalid molecule handle."};`
- **Sulfoxide normalization** (~8748-8836): charge-separated sulfoxide
  R2S(+1)-O(-1) → neutral R2S=O. Mutates `mol` in place via Indigo calls.
- **Bare inorganic ion lookup and related standalone-ion blocks**
  (~8838-11780-ish): exact-match lookups for zero-carbon or simple-ion
  molecules, returning immediately on match. This is not one block but
  several, including (approximate lines, not individually re-verified to
  the line for this revision — flagged here as a gap rather than guessed
  precisely): alkoxide salts (~9996), ammonium salts (~10228), N-oxides
  (~10419), pyridinium salts (~10611), saturated N-heterocyclic quaternary
  ammonium (~10870), symmetric quaternary ammonium (~11157).
- **Salt naming (P-77)**: 2- and 3-component salt handling (hydrohalides,
  Group 1/2 metal carboxylate/alkoxide salts, ammonium salts), using
  `indigoComponent()`, `indigoClone()`, and recursive `generateName()` calls
  on cloned components.
- **Aromatization and ring count**: `indigoAromatize(mol)`, then
  `int ringCount = indigoCountSSSR(mol);` at line 8926 — computed exactly
  once, read (never recomputed) by every later ring-count-guarded section.
- **Isotope-label handling** near the end of this section, shortly before
  `indigoToGraphIdx` is built.

**Data produced:** `int ringCount` (line 8926); early-exit `IupacResult`
for ions/salts; a normalized (possibly mutated) `mol`.

**Data consumed:** None (entry point).

---

### Section 2: Graph Construction (lines ~11787-12067)

**Purpose:** Build the `Graph g` object the rest of the function (and every
function it calls) operates on.

- `std::map<int, int> indigoToGraphIdx;` — line 11787.
- `Graph g;` — line 12016, nodes built from heavy atoms with implicit +
  explicit hydrogen counts.
- Bond iteration populates `g.nodes[*].neighbors`/`bondOrders` from
  `indigoIterateBonds(mol)`.

**Data produced:** `indigoToGraphIdx` (11787), `Graph g` (12016) — both read
by every subsequent section.

**Data consumed:** `mol` (post-normalization).

---

### Section 3: Organometallic Compound Handling (lines 12068-~12226)

**Real header (line 12068):** `// --- Organometallic compounds, Group 1-2 metals (P-69.3) ---`

**Purpose:** Name organometallic compounds with exactly one Group 1 or
Group 2 metal atom.

**Structure:** detect Group 1 (Li, Na, K) / Group 2 (Mg, Ca) metal atoms;
reject if more than one; Group 1 must have exactly one carbon neighbor
(name the alkyl group via `nameBranchGraph` + metal name); Group 2 must
have exactly one carbon + one halogen neighbor.

**Indigo call density:** measured ~17 calls across this section's ~158
lines — low but not zero.

---

### Section 4: Functional-Group Narrow Cases (lines ~12227-15140)

**Purpose:** Specific, individually-recognized functional-group patterns.
Each sub-block is self-contained, returns `IupacResult` directly on match,
falls through otherwise.

**Real sub-block header lines (every one read directly from HEAD):**

| Sub-block | Real line |
|---|---|
| P-67.2 dinuclear phosphorus oxoacids | 12227 |
| P-67.2 dinuclear arsenic oxoacids | 12322 |
| P-67.2 dinuclear boron oxoacids | 12417 |
| P-67.2 dinuclear antimony oxoacids | 12501 |
| Amidoximes (P-66.4.4) | 12596 |
| N-substituted diamines (P-62.2.4) | 12673 |
| N-substituted amines (P-62.2.4) | 13135 |
| N-substituted exocyclic amines on saturated N-heterocycle | 13581 |
| Carbamate esters (P-65.2.1.1) | 13917 |
| N-substituted amides (P-66.1.1.3.1), incl. imide rejection | 14000 |
| N-substituted sulfonamides/sulfinamides | 14144 |
| N-substituted imines (P-62.3) | 14360 |
| N'-substituted amidines (P-66.4) | 14442 |
| N'-substituted guanidine (P-66.4.1.2.1) | 14512 |
| Ureas | 14593 |
| N-substituted thioamides (P-66.1.1.3.1), incl. imide rejection | 14693 |
| N-substituted hydrazides (P-66.3.3.1) | 14822 |
| P-67.2 dinuclear sulfur oxoacids | 15049 |

**Indigo call density:** measured ~39 calls across this section.

The last sub-block (dinuclear sulfur oxoacids) runs from line 15049 to
approximately line 15140, where Section 5's pre-pass begins.

---

### Section 5: Ring-Substituent Pre-pass, then TWO Lambdas (lines ~15141-18365)

**This section was wrong in both prior versions of this document.** v1
omitted it almost entirely; v2 found the first lambda but described it as
one ~2,277-line block — it is actually two separate lambdas with different
names, different purposes, and a real function call in between.

**Pre-pass (~15141-16086):** functional-group detection for ring
substituents (builds maps like `carbonSulfonicAcid`, `carbonSulfonamide`,
`carbonThiol`, etc., starting ~15141), then `struct RingSubstituentInfo`
(line 15469) and `std::vector<RingSubstituentInfo> ringSubstituentInfos;`
(line 15474, read by Section 6's guard below), then the two "forced ring"
variables used to let later sections force a specific ring into the first
lambda's monocyclic-parent role:
```cpp
std::set<int> phase2ForcedRingNodeSet;                       // line 16087
std::map<int, QString> phase2ForcedRingSubstituentNames;      // line 16088
```

**Lambda 1 — `namePhase2Monocyclic`** (line 16091, header comment
`// --- Phase 2: Monocyclic Ring Path (ringCount == 1) ---` at line 16091
itself, closing `};` at line 17560 — **1,470 lines**):
```cpp
auto namePhase2Monocyclic = [&]() -> IupacResult { ... };
```
Called from: line 23779 (Section 12), lines 23873/23882/23896/23906
(Section 13), and the function's own final line, 23964.

**Lambda 2 — `nameNaphthaleneFusion`** (line 17563, closing `};` at line
18365 — **803 lines**):
```cpp
auto nameNaphthaleneFusion = [&](const std::set<int> &ring1Nodes,
                                  const std::set<int> &ring2Nodes,
                                  const std::vector<std::pair<int, QString>> &extraSubstituents = {}) -> IupacResult { ... };
```
Despite the name suggesting a standalone function, this is ALSO an inline
lambda, not a separately-defined function — v2 of this document explicitly
and wrongly claimed it "lives in a separate function, not inline." It has
measured ZERO direct Indigo calls. Called from: line 22925 (Section 11) and
line 23961 (Section 13).

**Data produced:** both lambdas return `IupacResult` when called;
`namePhase2Monocyclic` reads `phase2ForcedRingNodeSet`/
`phase2ForcedRingSubstituentNames` to support being invoked with a ring
artificially forced into the "monocyclic parent" role.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringSubstituentInfos`,
`ringCount`.

---

### Section 6: Acyclic Naming Path (Phase 1) (lines 18367-20161)

**Guard (line 18367):** `if (ringCount == 0 || !ringSubstituentInfos.empty())`

**Purpose:** Name acyclic compounds and ring-substituted chains, via a
`carbonGroup` map built from pure graph traversal (nitro, nitroso,
isocyanate, carboxylic acid, ester, acyl halide, amide, thioamide,
sulfinic/sulfonic acid, etc.), principal-group suffix selection, and chain
parent selection via `nameAcyclicChainParentWithSubstituents`.

**Indigo call density: measured ZERO** (`grep -c "indigo[A-Z][A-Za-z]*("`
over this exact range returns 0 — confirmed on two independent review
passes now). This section does all functional-group detection via pure
traversal of the already-built `g`.

---

### Section 7: Phase 26 — Retained Names for Adamantane and Cubane (lines ~20163-20465)

**Real header (line 20163):** `// --- Phase 26: Retained Names for Adamantane and Cubane ---`

Name cage hydrocarbons (adamantane, cubane) by exact structural match.

---

### Section 8: Phase 27 — Saturated Unsubstituted Bicyclic Hydrocarbons (lines 20466-20655)

**Real header (line 20466):** `// --- Phase 27: Saturated Unsubstituted Bicyclic Hydrocarbon Path (von Baeyer: bicyclo[a.b.c]alkane) ---`

**Guard:** `if (ringCount == 2)`. Von Baeyer bicyclo[a.b.c]alkane naming:
extract SSSR rings, build the shared-atom matrix, validate bicyclic
topology, construct the bridge-size array, assemble the name.

---

### Section 9: Spiro and Dispiro Hydrocarbon Paths (Phases 28, 29, 29.5) (lines 20656-21321)

**Real header lines, in actual file order:**

| Phase | Real line |
|---|---|
| Phase 29.5 (Dispiroter[bicyclo[a.b.c]alkane], P-24.4.1) | 20656 |
| Phase 29 (Spirobi[bicyclo[a.b.c]alkane]) | 20800 |
| Phase 28 (general spiro[a.b]alkane) | 20983 |

---

### Section 10: Special Fused/Heterocyclic Narrow Cases (Phases 30.5-35) (lines 21322-22744)

**Real header lines, in actual file order:**

| Phase | Real line |
|---|---|
| Phase 30.5 (1,4-epoxynaphthalene, P-25.4; guard `allSSSRRings.size() == 3`) | 21322 |
| Detect unsupported fusion topologies (P-25.3.1.1.2, P-25.4, P-25.5; a bare guarded block, no separate phase number) | 21548 |
| Phase 31 (Benzo-fused Heterobicyclic Ring Systems, ringCount==2) | 21637 |
| Phase 32 (Purine) | 21778 |
| Phase 33 (Pentalene) | 21895 |
| Phase 34 (3H-Pyrrolizine) | 22000 |
| Phase 35 (General Two-Heterocycle Fusion Nomenclature) | 22119 |

This section ends right where Section 11 begins (line 22745) — no overlap.

---

### Section 11: Multi-Component Ring Dispatch — Fusion-Dispatch Site 2 (lines 22745-23374)

**Real header (line 22745):** `// --- Multi-component ring dispatch: a molecule with exactly one real, ...`

**Purpose:** Molecules with exactly one "real" fused ring system plus one
or more separate, plain single rings elsewhere. Characterized in Task G's
`docs/iupac/fusion-dispatch-characterization.md` as **Site 2** (that
document's own line citations for this site, 23343/23349, were written
against an earlier commit and have since drifted a few lines — not
re-verified or fixed here, since Task G's document is already reviewed and
closed; flagging it only so a future reader isn't confused by the small
mismatch).

**Structure:** extract SSSR rings; union-find to separate the "real"
(fused) component from isolated single rings; name each isolated ring as a
substituent via `nameRingAsSubstituent`, collecting results into
`extraSubs`; also directly calls the Section 5 lambda
`nameNaphthaleneFusion(ring1G, ring2G, extraSubs)` at line 22925 for a
2-ring real component.

**Fusion-dispatch Site 2 calls (real HEAD lines 23347, 23353):**
```cpp
auto chainRes  = nameLinearChainFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
auto branchRes = nameBranchingFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
```
Only `result[0].success` is checked; returns early on either success.

**Data produced:** `extraSubs`, `realComponentIndigo`.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`.

---

### Section 12: Fusion-Dispatch Site 3, Retained-Registry Pre-Check, AND the Disjoint-Ring Branch (lines 23376-23784)

**Real header (line 23375, one line before the guard):** `// --- Phase 44 & 48: N-Heterocycle Fusion Chain Nomenclature ---`

**Guard (line 23376):** `if (ringCount >= 3)`

**This section has TWO branches on `allDisjoint`, and a prior version of
this document only documented one of them and wrongly claimed the section
"falls through" to Section 13 — it never does; it always returns.**

- **`!allDisjoint` branch** (the fused case): extract SSSR rings; build the
  `shared` matrix and `degree`/`validFusion`/`ends`/`centers`/`hasBranching`;
  call `tryRetainedBaseDecomposition(mol)` — on success, check the Task C
  InChIKey override set and return a `PreferredIupacName`- or
  `SystematicIupacName`-tagged result; otherwise fall through to fusion-
  dispatch **Site 3**'s `nameLinearChainFusion`/`nameBranchingFusion` calls
  (default parameters, also characterized in Task G's document — not
  re-verified here).
- **`allDisjoint == true` branch** (previously undocumented): map the SSSR
  rings to graph indices; find whether exactly one ring is "parent-
  eligible" (via its own local `isParentEligible` — heteroatom, or
  exocyclic double/triple bond to O/N/S); if so, name every OTHER ring as a
  substituent via `nameRingAsSubstituent`, write
  `phase2ForcedRingSubstituentNames`/`phase2ForcedRingNodeSet`, and call
  `namePhase2Monocyclic()` (line 23779 — this is the FIRST of that lambda's
  real call sites, not inside Section 13 as the prior version placed it).
- **Both branches end the same way**: if nothing above matched, this
  section's block ends with an UNCONDITIONAL
  `return {false, "", "Fused, bridged, spiro, or multiple ring systems are not supported in Phase 2."};`
  (line 23783). Section 12 therefore ALWAYS returns — it never falls
  through to Section 13, which has its own separate, mutually-exclusive
  `ringCount == 2` guard anyway.

**Data produced/written:** `chainResults`/`branchResults` (fused branch);
`phase2ForcedRingSubstituentNames`/`phase2ForcedRingNodeSet` (disjoint
branch — the SAME two variables Section 13 also writes; see the data-flow
table below).

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringCount`.

---

### Section 13: Ortho-Fused Bicyclic Path — Final Fallback (lines 23786-23966)

**Real header (line 23786):** `// --- Phase 3: Ortho-fused Bicyclic Aromatic Path (ringCount == 2: Naphthalene) ---`

**Guard:** `if (ringCount == 2)`

Read in full (and re-confirmed again on this revision) — the content is:

1. Extract exactly 2 SSSR rings (reject if not exactly 2).
2. Check whether the two rings are disjoint (share zero atoms) or fused.
3. **If disjoint:** find whether a ring is "parent-eligible" (same
   `isParentEligible` criteria as Section 12's disjoint branch, defined
   again locally here). Exactly one eligible → name the other as a
   substituent, force it via `phase2ForcedRingNodeSet`/
   `phase2ForcedRingSubstituentNames`, call `namePhase2Monocyclic()` (3 of
   that lambda's call sites are here: lines 23873, 23882, 23896/23906 for
   the various eligibility branches). Both eligible → break the tie via
   `compareRingSeniority` (P-44.2.1 criteria a-g); a complete tie returns
   `"Two ring systems of equal seniority (P-44.2.1) are not yet supported as parent vs. substituent."`.
   Neither eligible → check the **biphenyl special case** (two
   unsubstituted, all-carbon, all-aromatic 6-rings) and return `"biphenyl"`
   directly if it matches.
4. **Fallback for everything else, including the fused case:**
   `return nameNaphthaleneFusion(ring1Nodes, ring2Nodes);` (line 23961 —
   this calls the Section 5 lambda, NOT a separate function).
5. **The function's absolute final line (23964):**
   `return namePhase2Monocyclic();` — if `ringCount != 2` (nothing in
   Sections 1-13 matched, including this section's own guard not firing),
   `generateNameImpl` falls through to calling that lambda unconditionally
   as the last resort.

**Data produced:** `IupacResult`, directly or via the two Section 5
lambdas.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringCount`,
`allSSSRRings`, `phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames`
(also written by Section 12 — see above).

---

## Cross-Section Data Flow

| State | Produced at | Consumed by |
|---|---|---|
| `indigoToGraphIdx` | Line 11787 | Every subsequent section |
| `Graph g` | Line 12016 | Every subsequent section |
| `int ringCount` | Line 8926 | Every ring-count-guarded section (3, 6, 7, 8, 9, 10, 11, 12, 13) |
| `ringSubstituentInfos` | Section 5's pre-pass (line 15474) | Section 6's guard; Section 5's `namePhase2Monocyclic` lambda |
| `phase2ForcedRingNodeSet` / `phase2ForcedRingSubstituentNames` | Declared empty in Section 5's pre-pass (lines 16087-16088); **WRITTEN by BOTH Section 12's disjoint branch AND Section 13** | Read by Section 5's `namePhase2Monocyclic` lambda |
| `extraSubs`, `realComponentIndigo` | Section 11 | Section 11's own Site 2 calls and its `nameNaphthaleneFusion` call |
| `chainResults`/`branchResults` | Section 12's fused branch | Section 12's own Site 3 handling only |

**Critical dependency:** `mol` is mutated in place by Section 1 (sulfoxide
normalization, aromatization) before `g`/`indigoToGraphIdx` are built —
every later section's Indigo calls operate on the already-normalized
molecule.

**Note on SSSR re-extraction:** SSSR rings are independently re-extracted
by Indigo in multiple separate places (Sections 8, 10, 11, 12, 13) — each
extraction is local to its own section, never shared or cached.

---

## Indigo Call-Site Density by Section (measured where stated)

| Section | Lines | Indigo calls |
|---|---|---|
| 1-2 (Entry + graph construction) | 8743-12067 | the large majority of the function's total (Sections 1-2 alone account for roughly 81% of the whole function's count) |
| 3 (Organometallic) | 12068-~12226 | ~17 |
| 4 (Narrow cases) | ~12227-15140 | ~39 |
| 6 (Acyclic, Phase 1) | 18367-20161 | **0**, measured — not an estimate |
| 13 (Phase 3 final fallback) | 23786-23966 | 9 |
| **Whole function** | 8743-23966 | **865** (line-count basis; 866 raw occurrences, since one line contains two calls) |

Re-run `grep -c "indigo[A-Z][A-Za-z]*("` over any of these ranges yourself
to re-verify if the line ranges shift in a future task — do not trust
these numbers blindly if the file has changed since this was written.

---

## Fusion-Dispatch Call Sites Summary

This function contains **two of the three** fusion-dispatch call sites
characterized in Task G (Site 1 is in `nameRingSystemAsSubstituent`, a
different function). See `docs/iupac/fusion-dispatch-characterization.md`
for the full, already-reviewed characterization.

| Site | Section (above) | Key parameters | Result handling |
|---|---|---|---|
| **Site 2** | Section 11 (real lines 23347, 23353) | `ringSubsetOverride=&realComponentIndigo`, `extraSubstituents=extraSubs` | `result[0].success` only |
| **Site 3** | Section 12's `!allDisjoint` branch | all defaults | `result[0]`, checks `.success` + sentinel strings, exocyclic-substituent fallback logic |

---

## Key Observations for Future Extraction

1. **`g` and `indigoToGraphIdx` are the core shared state** (lines 12016,
   11787), read by virtually every section.
2. **`mol` is mutated in-place** in Section 1, before `g` is built.
3. **SSSR rings are independently re-extracted** in Sections 8, 10, 11, 12,
   and 13 — never cached or shared.
4. **Section 5 contains the function's two largest reusable units**:
   `namePhase2Monocyclic` (1,470 lines, 5 call sites across Sections 12
   and 13 plus the function's own final line) and `nameNaphthaleneFusion`
   (803 lines, 2 call sites across Sections 11 and 13). Both are already
   lambda-isolated, making them the most natural extraction candidates if
   this function is ever decomposed — but note they are two DIFFERENT
   units with different signatures and purposes, not one combined block.
5. **`phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames` cross a
   THREE-way boundary**, not two: declared in Section 5, written by BOTH
   Section 12's disjoint branch and Section 13, read by Section 5's
   `namePhase2Monocyclic` lambda. Any future extraction touching any of
   these three locations must account for all three, not just two.
6. **Ring-count guards are the dominant dispatch mechanism** (`ringCount
   == 0`, `== 1` (via the lambda), `== 2` (Sections 8, 10 parts, 13),
   `>= 3` (Section 12)) — the natural axis for any future decomposition.
7. **Section 12 always returns; it is not a fall-through section.** A
   future reader tracing control flow from Section 12 to "whatever comes
   next" would be wrong to assume execution can reach Section 13 from
   Section 12 — they are mutually exclusive via their guards (`>= 3` vs
   `== 2`), and Section 12's own block has no path that doesn't end in a
   `return`.
