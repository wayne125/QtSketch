# generateNameImpl Internal Structure Map

This document maps the internal structure of `IupacNamer::generateNameImpl`
in `src/app/IupacNamer.cpp`. All line numbers in this document are relative
to the current `dev` HEAD at the time this map was committed (commit
`baeae90`'s fix, not the original `baeae90`): `generateNameImpl` spans
**lines 8742-23963 (15,222 lines)**, ending right before the next top-level
function, `computePeripheralNumbering3Ring` (line 23965).

**Note on entry points:** The public entry point is the thin wrapper
`IupacNamer::generateName` (lines 8726-8733 as of this writing), which calls
`generateNameImpl` and normalizes the `status` field. This document maps the
real logic in `generateNameImpl`.

**Correction note:** an earlier version of this document (commit `baeae90`)
stated the function ends at line 24799. That was wrong — it misidentified
the function's end by matching a line that looked like a new top-level
function signature but was actually three SEPARATE functions further down
the file (`computePeripheralNumbering3Ring`, `computeSpiroNumbering`,
`computePeripheralNumbering`), none of which are part of `generateNameImpl`.
That version also mis-stated several section boundaries and fabricated the
content of the function's final section. This version was rewritten from a
line-by-line scan of every `// ---` section-divider comment inside the real
8742-23963 range, cross-checked twice independently, plus a direct read of
the function's actual final ~180 lines.

---

## Major Dispatch Sections

### Section 1: Entry and Preprocessing (lines 8742-11786)

**Purpose:** Input validation, structural normalization, early-exit special
cases, Indigo-to-graph index construction.

**Subsections (verified real line numbers):**
- **Validation and early reject** (8742-8744): `if (mol < 0) return {false, "", "Invalid molecule handle."};`
- **Sulfoxide normalization** (~8747-8835): Convert charge-separated
  sulfoxide R2S(+1)-O(-1) to neutral R2S=O. Mutates `mol` in place via
  Indigo API calls.
- **Bare inorganic ion lookup** (~8837-8920): Exact-match lookup for
  zero-carbon molecules. Returns immediately on match.
- **Aromatization**: `indigoAromatize(mol)` shortly before `ringCount` is
  computed.
- **Ring count** (line 8925): `int ringCount = indigoCountSSSR(mol);` —
  computed exactly once, used (not recomputed) by every ring-count-guarded
  section for the rest of the function.
- **Salt naming (P-77)** (~8927-9772): 2-component and 3-component salt
  handling (hydrohalides, Group 1/2 metal carboxylate/alkoxide salts,
  ammonium salts). Uses `indigoComponent()`, `indigoClone()`, and recursive
  `generateName()` calls on cloned components.
- **Standalone carboxylate anion naming** (~9774+): single-component
  molecules with charged carboxylate oxygens.
- **Indigo-to-graph index construction** (line 11787):
  `std::map<int, int> indigoToGraphIdx;` — built from atom iteration over
  `mol`, handling isotope labeling and explicit/implicit hydrogen counts.

**Data produced:** `indigoToGraphIdx` (line 11787); `ringCount` (line 8925);
early-exit `IupacResult` for ions/salts; a normalized (possibly mutated)
`mol`.

**Data consumed:** None (entry point).

**Indigo call density:** very high — this section alone accounts for the
large majority of the function's Indigo calls (sulfoxide normalization, ion
lookup, salt detection all call Indigo repeatedly). See the density table
below for a measured count.

---

### Section 2: Graph Construction (lines ~11787-12067)

**Purpose:** Build the `Graph g` object the rest of the function (and
every function it calls) operates on.

**Subsections:**
- `indigoToGraphIdx` built (line 11787, see Section 1).
- **`Graph g` construction** (line 12015): `Graph g;` — nodes built from
  heavy atoms with implicit + explicit hydrogen counts.
- **Bond iteration**: populates `g.nodes[*].neighbors`/`bondOrders` from
  `indigoIterateBonds(mol)`.

**Data produced:** `Graph g` (line 12015) — used, read-only, by every
subsequent section in the function.

**Data consumed:** `mol` (post-normalization), `indigoToGraphIdx`.

---

### Section 3: Organometallic Compound Handling (lines 12067-~12222)

**Real section header (line 12067):** `// --- Organometallic compounds, Group 1-2 metals (P-69.3) ---`

**Purpose:** Name organometallic compounds with exactly one Group 1 or
Group 2 metal atom.

**Structure:** Detect Group 1 (Li, Na, K) / Group 2 (Mg, Ca) metal atoms;
reject if more than one metal atom; Group 1 metals must have exactly one
carbon neighbor (name the alkyl group via `nameBranchGraph` + metal name);
Group 2 metals must have exactly one carbon + one halogen neighbor.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`.

**Indigo call density:** low — primarily graph traversal.

---

### Section 4: Functional-Group Narrow Cases (lines ~12223-15047)

**Purpose:** Handle specific, individually-recognized functional-group
patterns before the main dispatch chain. Each sub-block is its own
self-contained guarded check that returns `IupacResult` directly on match
and falls through otherwise.

**Real sub-block header lines (verified via a full scan of `// ---`
comments in this range — these are the actual line numbers, not estimates):**

| Sub-block | Real line |
|---|---|
| P-67.2 dinuclear phosphorus oxoacids | 12226 |
| P-67.2 dinuclear arsenic oxoacids | 12321 |
| P-67.2 dinuclear boron oxoacids | 12416 |
| P-67.2 dinuclear antimony oxoacids | 12500 |
| Amidoximes (P-66.4.4) | 12595 |
| N-substituted diamines (P-62.2.4) | 12672 |
| N-substituted amines (P-62.2.4) | 13134 |
| N-substituted exocyclic amines on saturated N-heterocycle | 13580 |
| Carbamate esters (P-65.2.1.1) | 13916 |
| N-substituted amides (P-66.1.1.3.1), incl. imide rejection | 13999 |
| N-substituted sulfonamides/sulfinamides | 14143 |
| N-substituted imines (P-62.3) | 14359 |
| N'-substituted amidines (P-66.4) | 14441 |
| N'-substituted guanidine (P-66.4.1.2.1) | 14511 |
| Ureas | 14592 |
| N-substituted thioamides (P-66.1.1.3.1), incl. imide rejection | 14692 |
| N-substituted hydrazides (P-66.3.3.1) | 14821 |
| P-67.2 dinuclear sulfur oxoacids | 15048 |

**Data produced:** Returns `IupacResult` directly on match for each case.

**Data consumed:** `g`, `mol`.

**Indigo call density:** low-medium — mostly graph-based pattern matching
on `g.nodes`.

---

### Section 5: Ring-Substituent Pre-pass, then the `namePhase2Monocyclic` Lambda (lines ~15048-18365)

**This entire section was MISSING from the original version of this
document** — it was folded incorrectly into "Section 3" and "Section 5"
labels that didn't match the real code. This is actually one coherent,
large unit: a short pre-pass, followed by a ~2,277-line lambda that is
called from FIVE different places later in the function (not inlined
once).

**Pre-pass (~15048-16088):** builds `ringSubstituentInfos`,
`allRingSubstituentNodes`, and the forcing variables
`phase2ForcedRingNodeSet` / `phase2ForcedRingSubstituentNames` (both
initially empty; later sections — see Section 11 — can populate these to
force the lambda below to treat a specific ring as the monocyclic parent
with a specific substituent already named).

**The lambda itself (line 16089):**
```cpp
auto namePhase2Monocyclic = [&]() -> IupacResult {
    // --- Phase 2: Monocyclic Ring Path (ringCount == 1) ---
    ...
};
```
Closes at line 18365. This is the function's single largest contiguous
block of ring-naming logic for monocyclic systems.

**Call sites (all within `generateNameImpl`, further down the file):**
lines 23778, 23872, 23881, 23895, 23905, and the function's own final
fallback return at line 23963 (`return namePhase2Monocyclic();`).

**Data produced:** `IupacResult` when called; the lambda itself reads
`phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames` to support
being invoked with a ring artificially forced into the "monocyclic parent"
role (used by Section 11, below).

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringSubstituentInfos`
(from the pre-pass), `ringCount`.

---

### Section 6: Acyclic Naming Path (Phase 1) (lines 18367-20161)

**Guard (line 18367):** `if (ringCount == 0 || !ringSubstituentInfos.empty())`

**Purpose:** Name acyclic compounds and ring-substituted chains.

**Structure:** functional-group pattern matching (nitro, nitroso,
isocyanate, carboxylic acid, ester, acyl halide, amide, thioamide,
sulfinic/sulfonic acid, etc.) via a `carbonGroup` map; principal-group
suffix selection; chain parent selection and locant assignment via
`nameAcyclicChainParentWithSubstituents`; stereochemistry handling.

**Data produced:** Returns `IupacResult` directly for acyclic compounds.

**Data consumed:** `g`, `mol`, `ringCount`, `ringSubstituentInfos`.

**Indigo call density: measured as ZERO.** `grep -c "indigo[A-Z][A-Za-z]*("`
over this exact line range returns 0. This section does all of its
functional-group detection via pure graph traversal on the already-built
`g` — it makes no direct Indigo API calls at all. (The original version of
this document claimed "High (~50-70)" for this section — that was
fabricated; this is actually one of the LEAST Indigo-dependent sections in
the whole function, the opposite of what was claimed.)

---

### Section 7: Phase 26 — Retained Names for Adamantane and Cubane (lines ~20165-20464)

**Real header (line 20162, +3 for comment markers added since = ~20165):**
`// --- Phase 26: Retained Names for Adamantane and Cubane ---`

**Purpose:** Name cage hydrocarbons (adamantane, cubane) by exact
structural match.

(The original document mislabeled this whole block as "Section 5:
Monocyclic Ring Path (Phase 2)" — Phase 2 is the lambda in Section 5
above, a completely different and much larger piece of code. This
section is Phase 26 only.)

---

### Section 8: Phase 27 — Saturated Unsubstituted Bicyclic Hydrocarbons (lines ~20465-20655)

**Real header:** `// --- Phase 27: Saturated Unsubstituted Bicyclic Hydrocarbon Path (von Baeyer: bicyclo[a.b.c]alkane) ---`

**Guard:** `if (ringCount == 2)`

**Purpose:** Name von Baeyer bicyclo[a.b.c]alkane systems: extract SSSR
rings, build the shared-atom matrix, check valid bicyclic topology,
construct the bridge-size array, assemble the "bicyclo[a.b.c]alkane" name.

---

### Section 9: Spiro and Dispiro Hydrocarbon Paths (Phases 28, 29, 29.5) (lines ~20655-21321)

**Real header lines, in the order they actually appear (NOT the order the
original document claimed):**

| Phase | Real line |
|---|---|
| Phase 29.5 (Dispiroter[bicyclo[a.b.c]alkane], P-24.4.1) | 20655 |
| Phase 29 (Spirobi[bicyclo[a.b.c]alkane]) | 20799 |
| Phase 28 (general spiro[a.b]alkane) | 20982 |

(The original document had these in a different order with ranges that
overlapped Section 8's territory — e.g. it placed "Phase 29" before
"Phase 28" with a range that didn't match either real block.)

---

### Section 10: Special Fused/Heterocyclic Narrow Cases (Phases 30.5-35) (lines ~21321-23374)

**Real header lines (verified, in actual file order):**

| Phase | Real line |
|---|---|
| Phase 30.5 (1,4-epoxynaphthalene, P-25.4) | 21321 |
| Detect unsupported fusion topologies (P-25.3.1.1.2, P-25.4, P-25.5) | 21547 |
| Phase 31 (Benzo-fused Heterobicyclic Ring Systems, ringCount==2) | 21636 |
| Phase 32 (Purine) | 21777 |
| Phase 33 (Pentalene) | 21894 |
| Phase 34 (3H-Pyrrolizine) | 21999 |
| Phase 35 (General Two-Heterocycle Fusion Nomenclature) | 22118 |

(The original document's line ranges for this group were off by up to
~1,100 lines in places and had Section 8/9-equivalent ranges overlapping
each other. These are the real, scanned header positions.)

---

### Section 11: Multi-Component Ring Dispatch — Fusion-Dispatch Site 2 (lines ~22744-23374)

**Real header (line 22744):** `// --- Multi-component ring dispatch: a molecule with exactly one real, ...`

**Purpose:** Handle molecules with exactly one "real" fused ring system
plus one or more separate, plain single rings elsewhere. Characterized in
Task G's `docs/iupac/fusion-dispatch-characterization.md` as **Site 2**.

**Structure:** Extract all SSSR rings; build ring connectivity via
union-find to identify the one "real" (fused) component vs. isolated
single rings; name each isolated ring as a substituent via
`nameRingAsSubstituent`, collecting results into `extraSubs`; build
`realComponentIndigo` (the real component's own ring set) and compute its
topology (`sharedC`, `degreeC`, `validFusionC`, `endsC`, `centersC`,
`hasBranchingC`).

**Fusion-dispatch Site 2 calls (lines 23343, 23349 per Task G's
characterization doc, which already has verified line numbers for this —
not re-derived here):**
```cpp
auto chainRes  = nameLinearChainFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
auto branchRes = nameBranchingFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);
```
Only `result[0].success` is checked; returns early on either success.

**Data produced:** `extraSubs`, `realComponentIndigo`; an early-return
`IupacResult` on fusion success.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`.

**Contract note (from Task G):** Site 2's `ringSubsetOverride`/
`extraSubstituents` arguments give it a fundamentally different contract
from Sites 1 and 3 — see `fusion-dispatch-characterization.md` for the
full analysis; not re-derived here to avoid disagreeing with the
already-reviewed, already-approved source of truth for this specific
topic.

---

### Section 12: Retained-Registry Pre-Check + Fusion-Dispatch Site 3 (lines ~23374-23785)

**Real header (line 23374):** `// --- Phase 44 & 48: N-Heterocycle Fusion Chain Nomenclature ---`

**Guard:** `if (ringCount >= 3)`

**Purpose:** Main fusion chain/branching entry point for non-multi-
component cases. Includes the retained-registry pre-check (Task C's
InChIKey-override logic) and fusion-dispatch **Site 3** (also
characterized in Task G's document — not re-derived here).

**Structure:** extract SSSR rings; build the `shared` matrix and compute
`degree`/`validFusion`/`ends`/`centers`/`hasBranching`; call
`tryRetainedBaseDecomposition(mol)` — on success, check the Task C
InChIKey override set (see `src/app/IupacNamer.cpp`'s own comment at that
call site for the exact 4 molecules) and return a `PreferredIupacName`-
or `SystematicIupacName`-tagged result; otherwise fall through to Site 3's
`nameLinearChainFusion`/`nameBranchingFusion` calls (default parameters);
on no match, fall through to Section 13 below.

**Data produced:** `chainResults`/`branchResults`; an early-return
`IupacResult` on retained-registry or fusion success.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringCount`.

---

### Section 13: Ortho-Fused Bicyclic Path — Final Fallback (lines 23785-23963)

**Real header (line 23785):** `// --- Phase 3: Ortho-fused Bicyclic Aromatic Path (ringCount == 2: Naphthalene) ---`

**Guard:** `if (ringCount == 2)`

**This section's real content is substantially different from what the
original version of this document claimed.** The original claimed this
section names "naphthalene, anthracene, phenanthrene" directly using
`classifyFunctionalGroups`, fusion-locant letter assignment, and
`nameBranchGraph` — none of that appears anywhere in the real 178-line
body (read in full to write this entry, not inferred). What the code
actually does:

1. Extract exactly 2 SSSR rings (reject if not exactly 2:
   `"Fused ring systems other than naphthalene are not supported in this phase."`).
2. Check whether the two rings are **disjoint** (share zero atoms) or
   fused (share atoms).
3. **If disjoint:** determine which ring (if either) is "parent-eligible"
   (has a heteroatom, or an exocyclic double/triple bond to O/N/S). If
   exactly one is eligible, name the other as a substituent via
   `nameRingAsSubstituent`, force that ring into the monocyclic-parent
   lambda from Section 5 via `phase2ForcedRingNodeSet`/
   `phase2ForcedRingSubstituentNames`, and call `namePhase2Monocyclic()`.
   If BOTH are eligible, break the tie via `compareRingSeniority` (P-44.2.1
   criteria a-g); a complete tie returns
   `"Two ring systems of equal seniority (P-44.2.1) are not yet supported as parent vs. substituent."`.
   If NEITHER is eligible, check for the **biphenyl special case**
   (two unsubstituted all-carbon aromatic 6-rings) and return `"biphenyl"`
   directly if it matches.
4. **Fallback for everything else (including the actually-fused case):**
   `return nameNaphthaleneFusion(ring1Nodes, ring2Nodes);` (line 23960) —
   the real naphthalene/fused-bicyclic-aromatic naming lives in this
   separate function, not inline in `generateNameImpl`.
5. **The function's absolute final line (23963):**
   `return namePhase2Monocyclic();` — if `ringCount != 2` at all (i.e. none
   of Sections 1-12 matched and this section's own `if (ringCount == 2)`
   guard didn't even fire), `generateNameImpl` falls all the way through to
   calling the Section 5 lambda unconditionally as the last resort.

**Data produced:** Returns `IupacResult` — either directly, or via
`namePhase2Monocyclic()`/`nameNaphthaleneFusion()`.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`, `ringCount`,
`allSSSRRings`, `phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames`
(written here, read by Section 5's lambda).

---

## Cross-Section Data Flow

| State | Produced at | Consumed by |
|---|---|---|
| `indigoToGraphIdx` | Line 11787 | Every subsequent section |
| `Graph g` | Line 12015 | Every subsequent section |
| `int ringCount` | Line 8925 | Every ring-count-guarded section (3, 6, 7, 8, 9, 10, 11, 12, 13) |
| `ringSubstituentInfos` | Section 5's pre-pass (~15048-16088) | Section 6's guard; Section 5's lambda |
| `phase2ForcedRingNodeSet` / `phase2ForcedRingSubstituentNames` | Declared empty in Section 5's pre-pass; WRITTEN by Section 13 | Read by Section 5's `namePhase2Monocyclic` lambda |
| `extraSubs`, `realComponentIndigo` | Section 11 | Section 11's own fusion-dispatch Site 2 calls only |
| `chainResults`/`branchResults` | Section 12 | Section 12's own fusion-dispatch Site 3 handling only |

**Critical dependency:** `mol` is mutated in place by Section 1's
sulfoxide normalization and aromatization, before `g`/`indigoToGraphIdx`
are ever built — every later section's Indigo calls operate on the
already-normalized molecule.

**Note on SSSR re-extraction:** SSSR rings are independently re-extracted
by Indigo in multiple separate places (Sections 8, 10, 11, 12, 13) — each
extraction is local to its own section and not shared or cached across
sections.

---

## Indigo Call-Site Density by Section (measured, not estimated)

| Section | Lines | Indigo calls (measured via `grep -c "indigo[A-Z][A-Za-z]*("`) |
|---|---|---|
| 1-2 (Entry + graph construction) | 8742-12067 | the large majority of the function's total |
| 6 (Acyclic, Phase 1) | 18367-20161 | **0** — pure graph traversal, no direct Indigo calls |
| 13 (Phase 3 final fallback) | 23785-23963 | 9 |
| **Whole function** | 8742-23963 | **865** |

(The original version of this document estimated per-section counts for
every section without measuring; several of those estimates were
contradicted by a direct count, most notably claiming the acyclic section
was one of the HIGHEST-density sections when it is measurably zero. The
counts above are from an actual `grep -c` over the cited ranges at the
commit this fix was made, not an estimate — re-run the same command
yourself if the line ranges shift in a future task.)

---

## Fusion-Dispatch Call Sites Summary

This function contains **two of the three** fusion-dispatch call sites
characterized in Task G (Site 1 is in `nameRingSystemAsSubstituent`, a
different function entirely). See
`docs/iupac/fusion-dispatch-characterization.md` for the full, already-
reviewed characterization — summarized here only for convenience:

| Site | Section (above) | Key parameters | Result handling |
|---|---|---|---|
| **Site 2** | Section 11 | `ringSubsetOverride=&realComponentIndigo`, `extraSubstituents=extraSubs` | `result[0].success` only |
| **Site 3** | Section 12 | all defaults | `result[0]`, checks `.success` + sentinel strings, exocyclic-substituent fallback logic |

---

## Key Observations for Future Extraction

1. **`g` and `indigoToGraphIdx` are the core shared state** (lines 12015,
   11787), read by virtually every section. Natural parameters for any
   future extracted component.
2. **`mol` is mutated in-place** in Section 1, before `g` is built. Any
   future extraction must either accept a pre-normalized molecule or
   handle normalization itself.
3. **SSSR rings are independently re-extracted** in Sections 8, 10, 11,
   12, and 13 — never cached or shared. A caching layer could reduce
   redundant Indigo calls, but is out of scope for this plan.
4. **Section 5's `namePhase2Monocyclic` lambda is the single largest
   reusable unit** (2,277 lines), already structured as a self-contained
   callable with 5 internal call sites plus the function's own final
   fallback — the most natural extraction candidate if this function is
   ever decomposed, since it already has lambda-boundary isolation.
5. **Section 13 and Section 5 are coupled** via
   `phase2ForcedRingNodeSet`/`phase2ForcedRingSubstituentNames` — any
   extraction touching either must account for this one shared piece of
   mutable state crossing the boundary between them.
6. **Ring-count guards are the dominant dispatch mechanism** (`ringCount
   == 0`, `== 1` (via the lambda), `== 2` (Sections 8, 10 parts, 13),
   `>= 3` (Section 12)) — this is the natural axis along which a future
   decomposition would likely split.
