# generateNameImpl Internal Structure Map

This document maps the internal structure of `IupacNamer::generateNameImpl` in `src/app/IupacNamer.cpp` (lines 8739-24799, 16,061 lines total as of commit 835fca3).

**Note on entry points:** The public entry point is the thin wrapper `IupacNamer::generateName` at line 8729-8735, which calls `generateNameImpl` and normalizes the `status` field. This document maps the real logic in `generateNameImpl`.

---

## Major Dispatch Sections

### Section 1: Entry and Preprocessing (lines 8739-12071)

**Purpose:** Input validation, structural normalization, early-exit special cases, graph construction.

**Subsections:**
- **Validation and early reject** (8739-8742): Check `mol < 0`, return error.
- **Sulfoxide normalization** (8744-8832): Convert charge-separated sulfoxide R2S(+1)-O(-1) to neutral sulfoxide R2S=O form. Mutates `mol` directly via Indigo API calls.
- **Bare inorganic ion lookup** (8834-8917): Exact match lookup for zero-carbon molecules (halides, oxides, metals, polyatomic ions). Returns immediately on match.
- **Aromatization** (8919-8920): `indigoAromatize(mol)` to ensure aromatic bonds have reliable order=4.
- **Ring count** (8922): `ringCount = indigoCountSSSR(mol)`.
- **Salt naming (P-77)** (8924-9769):
  - 2-component salts (8924-9370): hydrohalide salts, Group 1 metal carboxylate/alkoxide salts, ammonium salts.
  - 3-component salts (9372-9768): Group 2 metal salts, metal + 2 identical acid/alcohol components.
  - Uses `indigoComponent()`, `indigoClone()`, recursive `generateName()` calls, and suffix transformation.
  - Each component classified as MAIN, HALIDE, METAL, or AMMONIUM.
- **Standalone carboxylate anion naming** (9771-10200+): Single-component molecules with charged oxygen atoms on carboxylate groups.
- **Graph construction** (11776-12071):
  - Atom iteration and filtering (11776-12006): Build `heavyAtomIndices`, `explicitHCounts`, `indigoToGraphIdx` from `mol`. Handle isotope labeling.
  - `Graph g` construction (12012-12049): Build graph nodes from heavy atoms with implicit + explicit hydrogen counts.
  - Bond iteration (12050-12071): Populate `g.nodes[*].neighbors` and `bondOrders` from `indigoIterateBonds(mol)`.

**Data produced:**
- `Graph g` (local, built from heavy atoms of `mol`)
- `std::map<int, int> indigoToGraphIdx` (local, maps Indigo atom index to graph node ID)
- `int ringCount` (local, from `indigoCountSSSR(mol)`)
- Early-exit results for special cases (ions, salts)

**Data consumed:** None (entry point)

**Indigo call density:** High - dozens of Indigo calls in sulfoxide normalization, ion lookup, salt detection, and graph construction. Notable: `indigoIterateAtoms`, `indigoAtomicNumber`, `indigoGetCharge`, `indigoCountImplicitHydrogens`, `indigoIterateNeighbors`, `indigoBond`, `indigoBondOrder`, `indigoIndex`, `indigoAromatize`, `indigoCountSSSR`, `indigoComponent`, `indigoClone`, `indigoFree`.

---

### Section 2: Organometallic Compound Handling (lines 12072-12171)

**Purpose:** Name organometallic compounds with Group 1-2 metals.

**Structure:**
- Metal atom detection (12106-12112): Find Group 1 (Li, Na, K) and Group 2 (Mg, Ca) metal atoms.
- Single metal atom check (12114-12170): Reject if >1 metal atom.
- Group 1 metals (12133-12142): Must have exactly one carbon neighbor. Name alkyl group via `nameBranchGraph` + metal name.
- Group 2 metals (12143-12169): Must have exactly one carbon + one halogen neighbor. Name alkyl group + metal name + halide suffix.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:** `g`, `mol`, `indigoToGraphIdx`

**Indigo call density:** Low - primarily graph traversal, minimal Indigo calls.

---

### Section 3: Functional-Group Narrow Cases (lines 12172-18363)

**Purpose:** Handle specific functional group patterns before main dispatch.

**Subsections (all with Guard pattern `if (g.nodes.size() >= X && g.nodes.size() <= Y)`):**
- **Organometallic fallback** (12064-12071): Already handled above, this is a continuation guard.
- **P-67.2 Dinuclear phosphorus oxoacids** (12172-12412): diphosphoric, hypodiphosphoric, diphosphorous, hypodiphosphorous.
- **P-67.2 Dinuclear arsenic oxoacids** (12413-12580): diarsoric, hypodiarsoric, diarsorous, hypodiarsorous.
- **P-67.2 Dinuclear boron oxoacids** (12581-12668): diboric acid, hypodiboric acid.
- **P-67.2 Dinuclear antimony oxoacids** (12669-12805): distiboric, hypodistiboric, distiborous, hypodistiborous.
- **Amidoximes (P-66.4.4)** (12806-12894)
- **N-substituted diamines (P-62.2.4)** (12895-13130)
- **N-substituted amines (P-62.2.4)** (13131-13576)
- **N-substituted exocyclic amines on saturated N-heterocycle** (13577-13912)
- **Carbamate esters (P-65.2.1.1)** (13913-13995)
- **N-substituted amides (P-66.1.1.3.1)** (13996-14355): Includes imide rejection.
- **N-substituted sulfonamides/sulfinamides** (14356-14507)
- **N-substituted imines (P-62.3)** (14508-14588)
- **N'-substituted amidines (P-66.4)** (14589-14688)
- **N'-substituted guanidine (P-66.4.1.2.1)** (14689-14817)
- **Ureas (Phase 2)** (14818-15044)
- **N-substituted thioamides (P-66.1.1.3.1)** (15045-15044 correction: 15045-15264)
- **N-substituted hydrazides (P-66.3.3.1)** (15265-15480)
- **P-67.2 Dinuclear sulfur oxoacids** (15481-16086): disulfuric, dithionic, dithionous.

**Data produced:** Returns `IupacResult` directly on match for each narrow case.

**Data consumed:** `g`, `mol`

**Indigo call density:** Low - primarily graph traversal and pattern matching on `g.nodes`, minimal Indigo calls.

---

### Section 4: Acyclic Naming Path (Phase 1) (lines 18364-20158)

**Purpose:** Name acyclic compounds and ring-substituted chains.

**Guard:** `if (ringCount == 0 || !ringSubstituentInfos.empty())` (line 18364)

**Subsections:**
- **Functional group detection for Phase 1** (18364-18410): Detect nitro, nitroso, isocyanate nitrogen patterns.
- **Functional-group pattern matching** (18412-20158):
  - Build `carbonGroup` map (carbon node -> GroupType) by scanning all carbon atoms and their neighbors.
  - Detect: carboxylic acid (via COOH pattern: C with double O, single O, and H or connection), ester, acyl halide, amide, thioamide, sulfinic acid, sulfonic acid, etc.
  - Build prefix/suffix via `principalGroupSuffix()` helper.
  - Handle various functional group classes with `classifyFunctionalGroups` and `nameAcyclicChainParentWithSubstituents`.
  - Chain parent selection and locant assignment.
  - Stereochemistry handling for acyclic chains.

**Data produced:** Returns `IupacResult` directly for acyclic compounds.

**Data consumed:** `g`, `mol`, `ringCount`, `ringSubstituentInfos`

**Indigo call density:** Medium - uses `indigoGetAtom`, `indigoAtomicNumber`, `indigoCountImplicitHydrogens`, `indigoIterateNeighbors`, `indigoBond`, `indigoBondOrder` for functional group detection. Dozens of Indigo calls throughout.

---

### Section 5: Monocyclic Ring Path (Phase 2) (lines 20159-20461)

**Purpose:** Name monocyclic ring compounds (ringCount == 1).

**Guard:** `if ((ringCount == 3 && allSSSRNodes.size() == 10) || (ringCount == 5 && allSSSRNodes.size() == 8))` for adamantane/cubane, but main Phase 2 starts after functional group checks.

**Actually:** Phase 2 marker at line 20159 (`// --- Phase 2: Monocyclic Ring Path (ringCount == 1) ---`) but the guard is for retained names. The actual monocyclic handling is distributed across multiple specialized blocks.

**Subsections:**
- **Retained Names for Adamantane and Cubane** (20159-20461): Phase 26 marker here, handles cage structures.

**Note:** The monocyclic ring naming is interspersed with functional group handling. The actual Phase 2 content appears to start at line 20159 but is followed by Phase 26, then Phase 27.

---

### Section 6: Saturated Unsubstituted Bicyclic Hydrocarbon (Phase 27) (lines 20462-20500)

**Purpose:** Name von Baeyer bicyclo[a.b.c]alkane systems.

**Guard:** `if (ringCount == 2)` (line 20463)

**Logic:**
- Extract SSSR rings via Indigo.
- Build shared atom matrix between rings.
- Check for valid bicyclic topology (exactly 2 shared atoms for von Baeyer).
- Build bridge size array [a, b, c] from ring pair connectivity.
- Name as "bicyclo[a.b.c]alkane" with appropriate stem.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:** `g`, `mol`, `ringCount`

**Indigo call density:** Medium - uses `indigoIterateSSSR`, `indigoIterateAtoms`, `indigoIndex`, `indigoFree`.

---

### Section 7: Spiro Hydrocarbon Paths (Phases 28-29.5) (lines 20501-21240)

**Purpose:** Name spiro compounds.

**Subsections:**
- **Spirobi[bicyclo[a.b.c]alkane] (Phase 29)** (20501-20590): 4-ring spiro systems.
- **Spiro[alkane-...] (Phase 28)** (20591-21240): General spiro compound naming.
- **Dispiroter[bicyclo[a.b.c]alkane] (Phase 29.5)** (21241-...): Multi-spiro systems.

**Guards:** Ring count and ring size specific checks.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:** `g`, `mol`, `ringCount`, `allSSSRRings`

**Indigo call density:** Medium - SSSR iteration, atom/neighbor traversal.

---

### Section 8: Special Narrow Cases (Phases 30.5-35) (lines 21241-23370)

**Purpose:** Handle specific fused ring systems and special cases.

**Subsections:**
- **1,4-epoxynaphthalene (Phase 30.5, P-25.4)** (21241-21580): Specific fused heterocycle.
- **Benzo-fused Heterobicyclic Ring Systems (Phase 31, ringCount == 2)** (21581-22410): Handles benzofused systems like benzofuran, indole, etc.
- **Purine (Phase 32)** (22411-22800): Imidazole + pyrimidine ortho-fused system.
- **Pentalene (Phase 33)** (22801-23000): 5-5 fused bicyclic hydrocarbon C8H6.
- **3H-Pyrrolizine (Phase 34)** (23001-23200): 5-5 fused bicyclic with bridgehead nitrogen.
- **General Two-Heterocycle Fusion Nomenclature (Phase 35)** (23201-23370): Generic handling for two-heterocycle fused systems.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:** `g`, `mol`, `ringCount`, `allSSSRRings`

**Indigo call density:** Medium - SSSR iteration, atom classification, ring pattern matching.

---

### Section 9: Multi-Component Ring Dispatch with Fusion Site 2 (lines 22741-23369)

**Purpose:** Handle molecules with exactly one real fused ring system plus isolated single rings (multi-component fused systems). Characterized in Task G as Site 2.

**Guard:** Implicit - entered when previous specialized phases don't match.

**Structure:**
- Extract all SSSR rings from `mol` via Indigo (lines 22754-22773).
- Build ring connectivity graph using union-find (lines 22775-22820).
- Identify connected components: one "real" component (fused rings) and one or more isolated single rings.
- For each isolated ring, name it as substituent via `nameRingAsSubstituent` (lines 22896-22910), collecting results into `extraSubs`.
- Build `realComponentIndigo` (vector of ring sets for the real component) and `realComponentGraphUnion` (graph node set).
- Compute topology of real component: `sharedC` matrix, `degreeC`, `validFusionC`, `endsC`, `centersC`, `hasBranchingC` (lines 23310-23335).
- **Fusion Site 2 - Multi-component context** (lines 23343, 23349):
  - Line 23343: `auto chainRes = nameLinearChainFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);`
  - Line 23349: `auto branchRes = nameBranchingFusion(g, mol, indigoToGraphIdx, NC, freeValAtom, &realComponentIndigo, extraSubs);`
  - Parameters: `g` (local Graph), `mol` (input), `indigoToGraphIdx` (local map), `NC` = `realComponentSize`, `freeValAtom` (first attachment point or -1), `ringSubsetOverride = &realComponentIndigo` (CRITICAL: constrains to real component rings), `extraSubstituents = extraSubs` (CRITICAL: pre-named isolated ring substituents).
  - Result handling: Uses only `result[0]`, checks `.success` directly (lines 23344-23346, 23350-23352). Returns early if either succeeds (line 23361).
- Falls through to later checks if neither fusion mechanism succeeds.

**Data produced:**
- `realComponentIndigo` (vector of ring atom sets for real component)
- `extraSubs` (vector of pre-named isolated ring substituent names with attachment points)
- Fusion results that may trigger early return

**Data consumed:**
- `g` (local Graph)
- `mol` (input molecule)
- `indigoToGraphIdx` (local map from graph construction)
- SSSR rings from Indigo

**Indigo call density:** High - extensive SSSR iteration, atom classification, ring component analysis. ~30-40 Indigo calls in this block.

**Contract notes:** Site 2 has fundamentally different contract from Sites 1 and 3 due to `ringSubsetOverride` and `extraSubstituents` parameters. Cannot be unified without breaking multi-component naming.

---

### Section 10: Fusion Chain Nomenclature with Retained-Registry Pre-Check and Fusion Site 3 (lines 23371-23781)

**Purpose:** Main fusion chain/branching nomenclature entry point for non-multi-component cases. Includes retained-registry pre-check (Task C) and Fusion Site 3.

**Guard:** `if (ringCount >= 3)` (line 23372)

**Subsections:**
- **Phase 44 & 48 header** (line 23371): N-Heterocycle Fusion Chain Nomenclature.
- **SSSR extraction and topology checking** (lines 23372-23490):
  - Extract SSSR rings via Indigo (lines 23373-23392).
  - Build `shared` matrix counting shared atoms between ring pairs (lines 23393-23405).
  - Compute `degree`, `validFusion`, `ends`, `centers`, `hasBranching` (lines 23407-23410).
  - Check ring disjointness (lines 23393-23399).
- **Retained-registry pre-check** (lines 23492-23559):
  - Call `tryRetainedBaseDecomposition(mol)` (line 23492).
  - On success: check for heteroatoms, exocyclic substituents, hydro prefix.
  - Task C InChIKey override check (lines 23537-23555): match against pinned InChIKeys for specific molecules (1,2,3,4-tetrahydroacridine, octahydrobenz[b]acridine, octahydrotetracene, naphtho[2,3-g]isoquinoline).
  - Return retained name with `PreferredIupacName` status if InChIKey matches.
  - Falls through if check fails.
- **Fusion Site 3 - Retained-registry pre-check context** (lines 23566, 23634):
  - Line 23566: `chainResults = nameLinearChainFusion(g, mol, indigoToGraphIdx, ringCount);`
  - Line 23634: `branchResults = nameBranchingFusion(g, mol, indigoToGraphIdx, ringCount);`
  - Parameters: `g` (local Graph), `mol` (input), `indigoToGraphIdx` (local map), `ringCount` (from line 8922). All optional parameters use defaults: `freeValenceAtom = -1`, `ringSubsetOverride = nullptr`, `extraSubstituents = {}`.
  - Guarded by topology checks: linear chain only if `validFusion && ends == 2 && centers == N - 2` (line 23565). Branching only if `validFusion && hasBranching` (line 23633).
  - Result handling: Collects only `result[0]`, checks both `.success` and specific sentinel errors (lines 23567, 23635). Sentinel strings: "Linear chain: no match" and "Branching fusion: no match".
  - Exocyclic substituent rejection handling (lines 23573-23619 for linear, 23641-23659 for branching): if error is the specific exocyclic substituent rejection, perform chain-vs-ring seniority check via `classifyFunctionalGroups` and potentially fall back to chain naming.
  - Returns fusion result directly if applies and succeeds (lines 23623, 23638).
- Falls through to Phase 3 if no fusion match.

**Data produced:**
- Fusion results that may trigger early return
- `chainResults`, `branchResults` (std::vector<RingSystemNameResult>)

**Data consumed:**
- `g` (local Graph)
- `mol` (input molecule)
- `indigoToGraphIdx` (local map from graph construction)
- `ringCount` (computed at line 8922)
- `sssrRings` (local SSSR rings extracted in this block)

**Indigo call density:** High - SSSR iteration, topology analysis. ~25-35 Indigo calls.

**Contract notes:** Site 3 uses default parameters for `freeValenceAtom`, `ringSubsetOverride`, and `extraSubstituents`. Different from Site 2 (multi-component) which passes explicit values. Different result-handling logic from Site 1 (in `nameRingSystemAsSubstituent`).

---

### Section 11: Ortho-Fused Bicyclic Aromatic Path (Phase 3) (lines 23782-24799)

**Purpose:** Name ortho-fused bicyclic aromatic systems (e.g., naphthalene, anthracene, phenanthrene).

**Guard:** `if (ringCount == 2)` (line 23783)

**Structure:**
- Extract SSSR rings via Indigo.
- Verify exactly 2 rings with ortho-fusion (exactly 2 shared atoms).
- Build ring node sets, identify fusion atoms.
- Determine ring sizes and fusion pattern.
- Name as "naphthalene", "anthracene", "phenanthrene" based on ring size combination.
- Handle substituents on ortho-fused systems:
  - Classify functional groups via `classifyFunctionalGroups`.
  - Build locant map for fusion atoms (shared atoms get letter suffixes: a, b, c, ...).
  - Handle locant assignment for shared vs. non-shared positions.
  - Special handling for naphthalene (well-known retained name).
- Seniority checks between chain and ring parents.
- Substituent naming via `nameBranchGraph` for each substituent.
- Assemble final name with proper locants and seniority ordering.

**Data produced:** Returns `IupacResult` directly on match.

**Data consumed:**
- `g` (local Graph)
- `mol` (input molecule)
- `indigoToGraphIdx` (local map)
- `ringCount`

**Indigo call density:** Medium - SSSR iteration, atom classification. ~20-30 Indigo calls.

---

## Cross-Section Data Flow

### Key Local State Produced and Consumed

| State | Produced at | Consumed by | Lifetime |
|-------|-------------|-------------|----------|
| `Graph g` | Line 12012 | All subsequent sections | Function scope |
| `std::map<int, int> indigoToGraphIdx` | Line 11784 | All sections needing Indigo-to-graph mapping | Function scope |
| `int ringCount` | Line 8922 | All ring-count guarded sections | Function scope |
| `std::vector<std::set<int>> allSSSRRings` (or `localSSSRRings`) | Lines 12081-12104, 22754-22773, 23373-23392, etc. | Ring analysis sections | Multiple local instances |
| `std::vector<RingSystemNameResult> chainResults/branchResults` | Lines 23566, 23634 (Site 3); Lines 23343, 23349 (Site 2) | Fusion result handling | Local to fusion blocks |
| `std::vector<std::pair<int, QString>> extraSubs` | Lines 22903, 22905-22910 | Site 2 fusion calls | Multi-component block scope |
| `std::vector<std::set<int>> realComponentIndigo` | Lines 22825, 22827-22870 | Site 2 fusion calls | Multi-component block scope |
| `FunctionalGroupClassification fgc` | Lines 16250, 17703, 23585, 23653 | Substituent-aware naming, seniority checks | Local to each call site |

### Data Flow Between Major Sections

1. **Entry/Preprocessing → All other sections**:
   - Produces: `g`, `indigoToGraphIdx`, `ringCount`, normalized `mol`
   - All subsequent sections consume these

2. **Salt handling (Section 1) → Exit**:
   - Produces: `IupacResult` directly
   - No data flow to other sections (early return)

3. **Organometallic → Exit**:
   - Produces: `IupacResult` directly
   - No data flow to other sections (early return)

4. **Functional-Group Narrow Cases → Exit**:
   - Produces: `IupacResult` directly on match
   - Falls through to next check on no match

5. **Acyclic Path → Exit**:
   - Guarded by: `ringCount == 0 || !ringSubstituentInfos.empty()`
   - Produces: `IupacResult` for acyclic compounds
   - Falls through if guard not met

6. **Multi-Component Ring Dispatch (Site 2) → Exit or Phase 44/48**:
   - Produces: `extraSubs` (pre-named isolated ring substituents)
   - Produces: `realComponentIndigo` (ring subset for real component)
   - Calls: `nameLinearChainFusion`/`nameBranchingFusion` with `ringSubsetOverride=&realComponentIndigo, extraSubstituents=extraSubs`
   - On success: returns `IupacResult` directly (early exit at line 23361)
   - On failure: falls through to Phase 44/48

7. **Phase 44/48 → Exit or Phase 3**:
   - Produces: `sssrRings`, topology metrics (`validFusion`, `ends`, `centers`, `hasBranching`)
   - Retained-registry pre-check: may return early with retained name
   - Fusion Site 3: calls `nameLinearChainFusion`/`nameBranchingFusion` with default parameters
   - On fusion match: returns `IupacResult` directly
   - Falls through to Phase 3

8. **Phase 3 → Exit**:
   - Produces: `IupacResult` for ortho-fused bicyclic aromatics
   - No further fallback

### Critical Data Dependencies

- **`g` and `indigoToGraphIdx`**: Built once at lines 12012 and 11784. Shared by all sections that need graph-based analysis. Never modified after construction.
- **`mol`**: Input parameter. Modified in-place by sulfoxide normalization (lines 8744-8832) and aromatization (line 8920). All subsequent Indigo calls use the modified molecule.
- **`ringCount`**: Computed once at line 8922. Used by all ring-count guarded sections. Not recomputed.
- **SSSR rings**: Extracted multiple times in different sections (lines 12081, 22754, 23373, 23495). Each extraction is independent and local to its section.

---

## Indigo Call-Site Density by Section

| Section | Lines | Indigo Call Density | Notes |
|---------|-------|---------------------|-------|
| Entry/Preprocessing | 8739-12071 | Very High (~100+) | Sulfoxide norm, ion lookup, salt handling, graph construction |
| Organometallic | 12072-12171 | Low (~5) | Minimal Indigo, mostly graph traversal |
| Functional-Group Narrow Cases | 12172-18363 | Low-Medium (~20-30) | Pattern matching on graph, minimal Indigo |
| Acyclic Path (Phase 1) | 18364-20158 | High (~50-70) | Functional group detection, chain analysis |
| Monocyclic Ring (Phase 2) | 20159-20461 | Medium (~15-20) | Ring detection, validation |
| Saturated Bicyclic (Phase 27) | 20462-20500 | Medium (~10-15) | SSSR iteration, ring analysis |
| Spiro Paths (Phases 28-29.5) | 20501-21240 | Medium (~15-20) | SSSR, ring connectivity |
| Special Narrow Cases (Phases 30.5-35) | 21241-23370 | Medium (~20-30) | SSSR, atom classification |
| Multi-Component Ring (Site 2) | 22741-23369 | High (~30-40) | SSSR, union-find, ring component analysis |
| Fusion Chain + Retained Check (Site 3) | 23371-23781 | High (~25-35) | SSSR, topology analysis, fusion calls |
| Ortho-Fused Aromatic (Phase 3) | 23782-24799 | Medium (~20-30) | SSSR, functional group classification |

**Total estimated Indigo calls:** 400-500+ across the entire function.

**Density observation:** The entry/preprocessing and acyclic sections have the highest Indigo call density. The functional-group narrow cases have the lowest (mostly graph-based pattern matching). Fusion-related sections have high density due to SSSR extraction and topology analysis.

---

## Fusion-Dispatch Call Sites Summary

This function contains **two of the three** fusion-dispatch call sites characterized in Task G (Site 1 is in `nameRingSystemAsSubstituent`, not in this function).

| Site | Lines | Context | Key Parameters | Result Handling |
|------|-------|---------|-----------------|------------------|
| **Site 2** | 23343, 23349 | Multi-component (one real fused system + isolated rings) | `ringSubsetOverride=&realComponentIndigo`, `extraSubstituents=extraSubs`, `freeValenceAtom=freeValAtom` | Uses `result[0].success` only, early return on success |
| **Site 3** | 23566, 23634 | Retained-registry pre-check (non-multi-component) | All defaults (`freeValenceAtom=-1`, `ringSubsetOverride=nullptr`, `extraSubstituents={}`) | Uses `result[0]`, checks both `.success` and sentinel errors, exocyclic fallback logic |

See `docs/iupac/fusion-dispatch-characterization.md` (Task G) for complete characterization of all three sites including Site 1 in `nameRingSystemAsSubstituent`.

---

## Key Observations for Future Extraction

1. **`g` and `indigoToGraphIdx` are the core shared state**: Built once at lines 12012 and 11784, used by virtually all subsequent sections. These would be natural parameters for any extracted component.

2. **`mol` is mutated in-place**: Sulfoxide normalization (lines 8744-8832) and aromatization (line 8920) modify the input molecule. Any extraction must either:
   - Accept that the molecule is pre-normalized
   - Handle normalization internally
   - Clone the molecule before normalization

3. **SSSR rings are recomputed multiple times**: Different sections extract SSSR rings independently (lines 12081, 22754, 23373, 23495). A caching mechanism could reduce redundant computation.

4. **Ring-based guards are common**: Many sections guard on `ringCount == X` or `ringCount >= X`. This suggests a natural decomposition point at ring-count dispatch.

5. **Fusion sites have different contracts**: Site 2 (multi-component) cannot be unified with Site 3 (retained-registry pre-check) due to `ringSubsetOverride` and `extraSubstituents` parameters. See Task G characterization.

6. **Functional group classification is reused**: `classifyFunctionalGroups` is called at lines 16250, 17703, 23585, 23653, and 23147. This is a candidate for extraction.

7. **Graph traversal helpers are needed**: `nameBranchGraph` is called extensively throughout. This is a natural candidate for a shared helper service.

8. **Early returns are common**: Most sections return `IupacResult` directly on match, with fall-through on no match. This makes the function a chain of responsibility pattern.
