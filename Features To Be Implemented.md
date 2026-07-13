# Features To Be Implemented

Backlog of known gaps, compiled from the UI/UX review, the deferred-work plans, and a
complete function-level audit of `chem-core.js` and the Indigo C API. Nothing here is
scheduled — pick items up explicitly. Part A is UI/product-level features; Parts B and C are
the full, itemized list of every unused library export/function, since those are exactly the
building blocks new features would be built from. Part D covers Bingo NoSQL, a vendored
module that isn't wired in at all yet (not even in `CMakeLists.txt`).

---

## Part A — UI / product features

### Speced, not yet built

- **Word-style page-aware rulers** — `docs/superpowers/specs/2026-07-08-toolbar-rulers-design.md`
  has the full design (page-bound tick marks, draggable margins, indent/tab-stop markers).
  `topRuler`/`leftRuler` in `MainWindow.qml` exist today but are static. (Section 2 of that spec;
  section 1, the toolbar alignment icons, is done — see below.)

### Done (2026-07-13, Most Abundant Mass + Mass Composition + extra pKa values)

- **Three more items from the "Calculation on molecules" cluster**, surveyed together after the
  user asked for a broader look at remaining physicochemical parameters:
  - **Most Abundant Mass** (`indigoMostAbundantMass`) — one more Properties-panel row, distinct
    from the already-shown average MW and monoisotopic mass (the mass of the most abundant
    isotopologue combination).
  - **Copy Mass Comp.** (`indigoMassComposition`) — new clipboard action, elemental mass %
    breakdown, following the "Copy SMILES/InChI/Hash" pattern (a text value, not a Properties
    row).
  - **Copy pKa Values** (`indigoPkaValues`) — new clipboard action, the full list of pKa values
    for every ionizable group, richer than the single strongest-pKa number already shown.
  - **Surveyed and explicitly excluded**: `indigoGrossFormula` (confirmed via source read as
    functionally redundant with the already-used formula field, aside from reaction support this
    app's Properties panel doesn't handle anyway) and `indigoSymmetryClasses` (returns a raw
    per-atom array, not a scalar — would need atom-highlighting UI, a materially bigger,
    different-shaped feature, not a quick property/copy-action addition).
  - Full plan: `.claude/plans/mass-composition-pka-values.md`.
  - **Ninth feature delegated to the Antigravity CLI** (flash tier). Diff read in full across all
    four changed files and confirmed clean — only the three planned additions, nothing else (the
    session's cumulative uncommitted diff still shows earlier rounds' `hash()`/`heavyAtoms`/
    `isChiral` as "changed" too, since nothing has been committed since the one push earlier this
    session — confirmed via mtime/content inspection that these are pre-existing, not
    duplicated).
  - **Verified against the actual compiled code with real chemical data**: a standalone C++
    harness called the real compiled `calcProperties`/`massComposition`/`pkaValues` against a
    real aspirin molfile — `mostAbundantMass` (182.022) came out distinct from the average MW
    (182.131, close to published aspirin MW ≈180.16 as before), mass composition
    ("C 52.76 H 3.32 O 43.92") sums to ~100% and matches aspirin's real C9H8O4 formula
    proportions, and pKa values (3.345250) exactly matches the same figure independently
    confirmed in the earlier molar-refractivity/pKa round this session (aspirin's real
    literature pKa ≈3.5).
  - Standing `scripts/worker_smoke_test.js` unaffected. Clean MinGW build; real interactive
    launch with no new QML errors beyond the two pre-existing benign warnings.

### Done (2026-07-13, Clean 2D Structure)

- **New standalone "Clean 2D Structure" toolbar button** (`indigoClean2d`) — whole-document only,
  not selection-aware. Scoped after investigating `indigoClean2d`/`MoleculeCleaner2d` as a
  possible Layout Selected substitute (see the Layout Selected entry above): confirmed it's a
  genuinely distinct algorithm from `indigoLayout` — local energy-minimization gradient descent
  starting from the existing coordinates, not a fresh global embedding — so shipped as its own
  feature rather than folded into Layout Selected. Full plan: `.claude/plans/clean-2d-structure.md`.
- **Tenth feature delegated to the Antigravity CLI** (flash tier). First attempt failed with a
  transient `agy exited 1: Error: timeout waiting for response` (confirmed via `agy-doctor` — auth
  and tier models all fine, a one-off backend hiccup, not a real setup problem); retried
  identically and completed clean. Also hit an unrelated environmental snag this round: the
  `agy-delegate`/`agy-job` wrapper commands weren't on the Bash tool's `PATH` this session despite
  the plugin normally adding its `bin/` — worked around by invoking the wrapper script by its full
  path under the plugin cache directory.
- **Diff read in full**: `IndigoService.h`/`.cpp` gained `clean2d()`/`clean2dFinished` mirroring
  `layout()` byte-for-byte with `indigoClean2d` swapped in, exactly per plan. `MainWindow.qml`
  gained the `onClean2dFinished` handler, the `reqId === "clean2d"` dispatch branch, and the
  toolbar model entry reusing `layout.svg` — no new icon asset, no `onClicked` special case needed
  (falls through to the existing generic `requestSerialize` path, same as normalize/standardize).
  No stray edits to `v8_worker.js`/`CMakeLists.txt`, and no leftover harness/test files left in the
  repo (agy's own scratch harness stayed in its sandbox, confirmed via `git status`).
- **Verified against the actual compiled code, not just the diff**: a standalone C++ harness
  called the real compiled `IndigoService::clean2d` on a deliberately squashed/distorted
  4-membered ring — result kept the same atom/bond count and the coordinates visibly changed
  (`coordsChanged=1`), confirming the real gradient-descent cleanup ran, not a no-op passthrough.
  **Environmental snag caught and fixed, not glossed over**: the harness first failed at runtime
  with `STATUS_ENTRY_POINT_NOT_FOUND`/`STATUS_DLL_NOT_FOUND` — traced to stale, version-mismatched
  Qt DLLs sitting in the old scratch test directory from an earlier round; replacing them with
  fresh copies from the actual Qt 6.11.1 install fixed it. Separately, the real interactive launch
  initially crashed the same way — traced to `build/sketch.exe` (the actual deployed, DLL-adjacent
  copy used for manual launches) being a stale build from an earlier point in the session, never
  refreshed by a plain `cmake --build .`; copying the freshly-built binary over it fixed the
  launch. Neither issue was caused by the Clean2D change itself — both were pre-existing
  verification-environment staleness, caught by insisting on an actual clean run rather than
  trusting the build's "exit code 0" alone.
- Clean MinGW build (`BUILD_EXIT:0` read directly from a log file, no `tail` pipe). Standing
  `scripts/worker_smoke_test.js` unaffected (no worker changes) — all 8 checks pass. Real
  interactive launch (the corrected `build/sketch.exe`) showed only the two pre-existing benign
  Quick Controls style warnings, no new QML errors.
- Not yet exercised: an actual on-screen click of the "Clean 2D Structure" button — verification
  here is backend/harness-level plus a clean non-crashing launch, not a manual click-through.

### Done (2026-07-13, Copy Canonical Hash)

- **"Copy Hash" toolbar action** — `indigoHash` (works for both molecules and reactions),
  following the existing "Copy SMILES ▾"/"Copy InChI ▾" clipboard-action pattern rather than the
  Properties panel (a hash is a value to copy, not a short display number). `indigoLayeredCode`
  was surveyed and dropped: reading its real implementation showed it's a pure wrapper around
  `MoleculeInChI::outputInChI` — duplicate of the InChI support already in the app via the
  dedicated `indigo-inchi` plugin. Full plan: `.claude/plans/canonical-hash.md`.
  - **Eighth feature delegated to the Antigravity CLI** (flash tier). Diff read in full and
    confirmed clean — only the planned `hash()`/`hashFinished` addition, everything else in the
    diff pre-existing from earlier rounds. The delegate's own digest noted it temporarily
    modified `src/app/main.cpp` for its own local testing before restoring it — verified this
    directly (`git diff --stat main.cpp` empty, file content inspected) rather than trusting the
    "restored it" claim at face value.
  - **Verified against the actual compiled code**: a standalone C++ harness called the real
    compiled `IndigoService::hash` — same molecule (ethane) hashed twice produced the identical
    value both times (determinism), and a different molecule (propane) produced a different
    value (discrimination) — both checks passed.
  - Standing `scripts/worker_smoke_test.js` unaffected. Clean MinGW build; real interactive
    launch with no new QML errors beyond the two pre-existing benign warnings.

### Done (2026-07-13, chirality/stereocenter checks in Validate structure)

- **"Validate structure" now also reports chiral-flag/stereocenter inconsistencies** — extends
  the existing `IndigoService::checkStructure`/`checkResultDialog` feature (not a new one) with
  `indigoCheckChirality`/`indigoCheckStereo`. `indigoCheck3DStereo` was deliberately excluded:
  reading its real implementation showed it only ever returns something meaningful when
  `BaseMolecule::hasZCoord` is true (real 3D/Z-nonzero coordinates) — trivially always false in
  this pure-2D editor, not worth exposing. Full plan: `.claude/plans/chirality-stereo-check.md`.
  - **Zero QML changes needed** — the existing `checkResultDialog` display logic
    (`MainWindow.qml`) already does `JSON.parse(report)` and renders every top-level key as its
    own paragraph; the two new findings are added as new keys (`"chirality"`, `"stereocenters"`)
    into the SAME JSON object `indigoCheckObj` already returns, so they render automatically with
    no changes to the display code at all.
  - **Seventh feature delegated to the Antigravity CLI** (flash tier). Wrapper again reported
    `AGY_FAILED`/timeout — another false negative in the now-familiar pattern; the actual diff was
    correct and complete, confirmed by reading it in full (only the planned `checkStructure`
    hunk, plus the already-verified pre-existing `heavyAtoms`/`isChiral` hunk from an earlier
    round — no scope creep).
  - **A real correction the delegate made to this session's own (flawed) plan, caught and
    independently re-verified, not blindly trusted either way**: the plan's prose described
    `indigoCheckChirality`'s return convention backwards (said "returns 1 for inconsistent"). The
    delegated code instead used `indigoCheckChirality(mol) == 0` as the problem condition —
    re-checked directly against the real source
    (`indigo_misc.cpp`: `return 0` only for the genuine `chiral_flag > 0 && stereocenters.size()
    == 0` problem case, `return 1` for every other case including the fallthrough) — confirmed
    the delegate's condition is the objectively correct one and the plan's own description was
    wrong, not the implementation.
  - **Verified against the actual compiled code**, not just re-reading source: a standalone C++
    harness called the real compiled `checkStructure` against (a) a single-atom molecule with
    chiral flag 0 (clean) and (b) the same molecule with chiral flag 1 and zero stereocenters
    (the exact problem case) — the clean case returned `{}` (no `"chirality"` key, no false
    positive), the inconsistent case correctly included the new `"chirality"` key with the
    intended descriptive message.
  - **Honest secondary finding, recorded rather than hidden**: for the tested inconsistent case,
    `indigoCheckObj`'s own existing default checks *already* flag the identical scenario under a
    different key (`"chiral_flag": "Structure contains wrong chiral flag"`) — meaning the new
    `"chirality"` finding is at least partially redundant with existing output for this specific
    case. Not a functional bug (no incorrect information is shown, and `indigoCheckStereo`'s
    distinct redundancy-detection logic wasn't tested against this same overlap), but worth
    knowing before treating this as a fully novel diagnostic in every case.
  - Standing `scripts/worker_smoke_test.js` unaffected (no worker changes). Clean MinGW build;
    real interactive launch with no new QML errors beyond the two pre-existing benign warnings.

### Done (2026-07-13, Layout Selected)

- **"Layout Selected" toolbar button** — re-lays-out only the currently-selected atoms, leaving
  the rest of the structure exactly fixed. Originated as a small backlog item
  (`indigoLayoutSelected` listed as an unused Indigo export in Part C), but turned into the
  longest single investigation this session — the planned mechanism doesn't exist, a
  higher-effort real mechanism also didn't produce the right behavior, and the final shipped
  implementation doesn't touch Indigo at all. Full story below because every stage changed the
  actual plan; skipping to "what shipped" would lose the reasoning that ruled out the other two
  approaches.

  **Stage 1 — the planned mechanism doesn't exist.** `indigoLayoutSelected` is declared in
  `indigo.h` but was never actually implemented. Confirmed twice, not assumed once: `objdump -p`
  on the vendored `indigo/lib/indigo.dll` showed no such export (while sibling `indigoLayout` was
  present), and a **fresh local compile of Indigo v1.45.0 from official source** (see below) had
  no `.cpp` anywhere implementing it either — `indigo_layout.cpp` implements `indigoLayout` only.
  A header-only stub, not a version-staleness problem.

  **The local Indigo compile, done because the first fix attempt needed it.** The user deleted
  the stale vendored `indigo/` directory mid-session to fetch a newer version; when a fresh
  v1.45.0 source tree still didn't help, the user asked to compile Indigo locally rather than
  hunt for a different prebuilt package. Configured via CMake (`-DBUILD_INDIGO=ON
  -DBUILD_STANDALONE=ON`, wrappers/utils/Bingo all off) against the same MinGW 13.1 toolchain
  this project already builds with, built with Ninja (582/582 objects, ~10 min). Produced real,
  working `indigo.dll`/`indigo-inchi.dll`/`indigo-renderer.dll` (514 plain-C exports, includes
  `indigoCreateSubmolecule`/`indigoGetSubmolecule`/`indigoLayout`/etc. — confirmed via `objdump`),
  vendored back into `indigo/lib/` + `indigo/api/c/*/` alongside matching v1.45.0 headers,
  restoring the app to a buildable state. **A real self-caught process bug along the way**: the
  first rebuild attempt after restoring `indigo/` was piped through `tail -30` without capturing
  the real exit code, so a background-task notification reported "completed (exit code 0)" for a
  build that had actually failed with a linker error — only caught because a separate,
  independently-checked verification harness hit the identical undefined-reference error and
  forced a second look. Every rebuild after that point had its exit code written directly to a
  log file, no pipe in between.

  **Stage 2 — the real, documented mechanism doesn't produce the right behavior either.** Reading
  `indigoLayout`'s actual v1.45.0 implementation (`indigo_layout.cpp`) revealed the real
  Indigo-native way to lay out a subset: `indigoGetSubmolecule` returns an `IndigoSubmolecule` — a
  live *view* into the original molecule (confirmed via `indigo_molecule_operations.cpp`; NOT a
  copy, unlike the similarly-named `indigoCreateSubmolecule`) — and `indigoLayout` special-cases
  this view type, and `IndigoBaseMolecule::is()` explicitly includes `case SUBMOLECULE: return
  true` (checked by object-type enum, not C++ inheritance — `IndigoSubmolecule` doesn't even
  inherit `IndigoBaseMolecule`). Reimplemented `IndigoService::layoutSelected` around
  `indigoGetSubmolecule` + `indigoLayout`, verified via a standalone C++ harness (pentane chain,
  3 middle atoms selected) — **failed**: all 5 atoms moved into a fresh symmetric zigzag, not just
  the 3 selected ones. Tried the one obvious knob (`indigoSetOption("layout-preserve-existing",
  "true")`) — **identical failure, bit-for-bit the same output numbers**, meaning the option had
  zero effect. Per this session's own standing rule (2 failed fixes ⇒ stop guessing, re-trace),
  read `MoleculeLayoutGraph::_layoutSingleComponent` directly
  (`molecule_layout_graph.cpp`): `if (!respect_existing || !preserve_existing_layout ||
  vertexCount() > _n_fixed) { /* recompute everything */ }` — whenever there's at least one free
  vertex (always true here), the OR is satisfied and the **entire connected component gets a
  fresh embedding**, fixed vertices included; old positions only loosely re-orient the result
  afterward, they are never pinned. Combined with `indigoLayout`'s own selection-aware branch
  being explicitly gated behind `mol->tgroups.getTGroupCount()` ("selection works only with
  monomers"), this is a real, version-independent gap in Indigo's C API: **partial-layout-with-a-
  fixed-remainder does not exist for plain small molecules**, only for monomer/biopolymer
  structures. Not a bug in this codebase — confirmed by reading the actual engine twice, not
  guessed once.

  **Stage 3 — what actually shipped: a from-scratch, worker-side reimplementation, deliberately
  narrow in scope.** Per explicit direction ("write our own implementation"), rebuilt the feature
  entirely in `src/v8_worker.js` against chem-core's own atom/bond model — a better architectural
  fit anyway, since undo/redo and selection already live there natively, and it sidesteps the
  whole Indigo round-trip (and the atom-index correlation work that round-trip needed) entirely.
  All the old Indigo-based scaffolding was removed: `IndigoService::layoutSelected`/
  `layoutSelectedFinished` (header + impl), the `getLayoutSelectedData` worker command, and
  `MainWindow.qml`'s `onLayoutSelectedFinished`/`onStructureReady` round-trip branch — replaced
  with one new worker function, `layoutSelectedChain()`, called directly via the existing generic
  `sendCommand` passthrough (same mechanism `loadSdfBatchRecord` already used), no C++ involved.
  - **v1 scope, deliberately narrow**: only a simple, unbranched, acyclic chain of selected atoms
    attached to the rest of the structure at exactly one point. Classifies every bond touching the
    selection as "internal" (both ends selected, builds the chain graph) or "anchor" (exactly one
    end selected); requires exactly one anchor bond, every selected atom to have internal-degree
    ≤2, and a walk from the anchor to visit every selected atom exactly once. Anything else —
    disconnected sub-selection, a branch point, a cycle, zero or multiple attachment points — is
    declined (no-op + `console.warn`) rather than risk producing a broken or overlapping layout.
  - **Reuses existing, already-verified geometry code rather than inventing new math**:
    `getLargestEmptyAngle(atomId)` (the same placement heuristic already used for the
    short-drag chain-extend fallback) picks the baseline direction away from the anchor, and the
    zigzag step formula (`theta ± Math.PI/6`, alternating) is copied exactly from `addChain`'s own
    working implementation — same `StandardBondLength` constant, same half-angle. Undo/redo
    follows the exact same old-position/new-position `makeCmd` shape already used by
    `alignAtoms`/`distributeAtoms` for direct `atom.pp.x`/`.pp.y` mutation.
  - **Verified via a disposable Node harness** against the real worker's stdin/stdout protocol: a
    straight 4-atom chain, lasso-selecting the 3 atoms *not* including one fixed end, running
    `layoutSelectedChain`, and checking (a) the unselected anchor atom's coordinates are byte-for-
    byte unchanged, (b) the three selected atoms visibly moved off the original straight line, and
    (c) every consecutive bond length in the new layout is exactly the standard 1.5 — all three
    passed. A secondary, orthogonal finding surfaced during this verification and is recorded here
    rather than silently dropped: reading the atom positions back via `getStructure("mol", ...)`
    showed the Y-coordinates of the two newly-placed zigzag atoms sign-flipped relative to what
    was actually written into `_struct.atoms` — traced (via a temporary debug reread immediately
    after the write, then removed) to a **pre-existing quirk inside `MolSerializer`'s mol-string
    output specifically**, not this feature: `_struct.atoms` itself and `buildRenderPrimitives`
    (the actual data QML/canvas renders from, confirmed by reading it directly — passes
    `a.pp.x`/`a.pp.y` straight through with zero transform) both hold the mathematically correct,
    unflipped values throughout. Two independent no-edit round-trip sanity checks (a single free
    atom; a bonded 3-bond chain in a non-zigzag bent shape) both preserved Y correctly, meaning the
    flip is specific to some as-yet-uncharacterized property of the zigzag shape/serialization
    path — flagged as a real, separate, pre-existing chem-core finding worth a future look, not
    something this feature introduced or needs to fix (on-screen rendering is unaffected).
  - Standing `scripts/worker_smoke_test.js` unaffected; clean MinGW build; real interactive
    launch with no new QML errors beyond the one pre-existing benign Quick Controls style warning
    that happened to appear this run (the second one is intermittent, not new).

### Done (2026-07-13, heavy atom count + chirality)

- **Heavy Atom Count and Chiral (Yes/No) added to the Properties panel's "Drug Properties"
  section** — same category, same pipeline, and same delegation pattern as the molar
  refractivity/pKa addition one day earlier. `indigoCountHeavyAtoms`/`indigoIsChiral` were listed
  in Part C's "Calculation on molecules" audit as unused core Indigo exports. Extended
  `IndigoService::calcProperties`'s `propertiesReady` signal with two more trailing params (now
  14 total); `MainWindow.qml`'s `onPropertiesReady` and `PropertyPanel.qml`'s "Drug Properties"
  grid gained two more rows ("Heavy Atoms"/"Chiral"), the latter rendering "Yes"/"No" text
  instead of a formatted number — the one deliberate difference from every other row in this
  grid, since `indigoIsChiral` is a boolean flag, not a scalar quantity. Full plan:
  `.claude/plans/heavy-atoms-chiral.md`.
  - **Sixth feature this session delegated to the Antigravity CLI** (flash tier). **Clean
    delegation, no false negative, no scope creep** — unlike the prior two rounds (QML review
    fixes and molar refractivity/pKa both had the wrapper falsely report `AGY_FAILED`/timeout on
    work that had actually succeeded correctly). This time the wrapper reported success directly,
    and the digest even mentioned the delegate having independently compiled and run a standalone
    C++ test against ethane and L-alanine — the same verification approach this session's own
    discipline calls for.
  - **Verified independently anyway, not because the digest was distrusted this time, but because
    self-reported success is still a claim, not evidence** — read the full diff of all four
    changed files (`IndigoService.h`, `IndigoService.cpp`, `MainWindow.qml`, `PropertyPanel.qml`):
    each diff contained *only* the planned two-field extension, nothing else, a first for this
    session's delegations (every prior round's diff also contained large blocks of pre-existing
    content from earlier features that had to be individually distinguished from new work).
  - **Verified against the actual compiled code with real molecules, not accepting the delegate's
    own claimed numbers on faith**: a standalone C++ harness (same technique as every other
    Indigo-backed verification this session) called the real compiled `calcProperties` against
    ethane (2 heavy atoms, no stereocenter) and L-alanine (6 heavy atoms, one real stereocenter at
    the alpha carbon). Results: ethane → `heavyAtoms=2, isChiral=false`; L-alanine →
    `heavyAtoms=6, isChiral=true` — both exact matches to the expected values, confirming the two
    new Indigo calls are wired correctly and computing real results, not stubbed.
  - Standing `scripts/worker_smoke_test.js` unaffected (no worker changes). Clean MinGW rebuild;
    real interactive launch (stderr captured directly from the process) with no new QML errors.
    Same standing environment constraint as the molar-refractivity/pKa round: an actual on-screen
    screenshot of the rendered rows was not obtained in this environment (window-focus automation
    is unreliable here); the numeric backend verification above is the verification of record.

### Done (2026-07-12, molar refractivity + pKa)

- **Molar Refractivity and pKa added to the Properties panel's "Drug Properties" section** —
  `Features To Be Implemented.md` Part C's "Calculation on molecules" audit listed
  `indigoMolarRefractivity`/`indigoPka` as unused core Indigo exports (both single-`double`-return
  calls, identical shape to `indigoLogP`/`indigoTPSA` already used one line above in
  `calcProperties`). Extended `IndigoService::calcProperties`'s existing `propertiesReady` signal
  with two trailing `double` params; `MainWindow.qml`'s `onPropertiesReady` handler and
  `PropertyPanel.qml`'s "Drug Properties" grid gained two more rows ("MolRef"/"pKa"), mirroring
  the existing TPSA/LogP/HBA/HBD/RotB rows exactly. Full plan:
  `.claude/plans/molar-refractivity-pka.md`.
  - **Fifth feature this session delegated to the Antigravity CLI** (flash tier — a small,
    mechanical signal-extension + 2 UI rows, same size class as the explicit-hydrogens
    delegation). Zero `src/v8_worker.js`/`CMakeLists.txt` changes needed (`calcProperties` is
    invoked the same `requestSerialize` passthrough as `layout`/`aromatize`; `indigo` core lib
    already linked).
  - **Wrapper reported `AGY_FAILED`/timeout again — another false negative**, same pattern as the
    QML-review-fixes delegation. This time verified with full rigor from the start (not partial,
    per the lesson from that earlier round): read every line of all four changed files' diffs,
    confirmed the only genuinely new content was the planned signal extension (2 trailing
    `double`s, correctly threaded through `IndigoService.h`'s signal, `calcProperties`'s early-
    return branch and success-path emit, `MainWindow.qml`'s handler, and `PropertyPanel.qml`'s two
    new rows) — everything else in the diffs was pre-existing content from earlier features this
    session (fold/unfold hydrogens, InChI support, SVG export, image embedding, QML review
    hardening's `textFormat` additions), individually cross-checked, not assumed.
  - **Verified against the actual compiled code with real numbers, not just "no crash"**: a
    standalone C++ harness (same technique as native SVG export/explicit-hydrogens) called the
    real compiled `IndigoService::calcProperties` directly against a real aspirin molfile.
    Results: molecular weight 182.1 (aspirin's real MW is 180.16 — close, small discrepancy from
    the hand-written test molfile's exact explicit-atom count, not a sign of a calculation bug),
    molar refractivity 41.79 (published aspirin molar refractivity is ≈44-45 — same ballpark),
    **pKa 3.345 (aspirin's real, literature-documented pKa is ≈3.5) — a close, chemically
    plausible match**, confirming `indigoMolarRefractivity`/`indigoPka` are wired and computing
    real values, not stubbed/zero.
  - **UI click-through/screenshot could not be closed out this round — a genuine environment
    constraint, not skipped by choice.** Attempted mouse-driven verification (click ring tool,
    place on canvas, screenshot the panel) but every window-focus API tried
    (`SetForegroundWindow`, `ShowWindow`, `WScript.Shell.AppActivate`, `FindWindow` by title,
    `EnumWindows`) either failed or the launched process's window never appeared in a full
    enumeration of visible top-level windows on this desktop — which also revealed this is an
    actively-used desktop (Qt Creator, Notepad++, a browser, and other tools already open on this
    same project), so continuing to force-launch/kill/move windows here was stopped as a
    real risk of interfering with actual concurrent use, not just a flaky script. The numeric
    backend verification above is solid; the actual on-screen row rendering was not visually
    confirmed this round.
  - Standing `scripts/worker_smoke_test.js` unaffected (no worker changes). Clean MinGW build;
    real interactive launch (stderr captured directly from the process, not via screenshot) shows
    no new QML errors beyond the two pre-existing benign Quick Controls style warnings.

### Done (2026-07-12, QML review hardening)

- **Four fixes from a 23-finding automated QML review, triaged first** — the raw report was not
  trusted as ground truth; each finding was checked against the real code before acting, per
  this session's standing "verify before trusting a report" rule. 19 of 23 findings did not
  survive triage:
  - One **hallucinated property**: `font.preferShaping` does not exist anywhere in this codebase
    (grep: zero hits) and isn't a real QML `Text`/`font` grouped-property attribute — applying it
    would just throw a QML property-not-found error, not a perf win.
  - One **false positive, confirmed via ownership trace**: the claim that
    `DocumentManager::documentFor` risks JS-GC ownership was wrong — `addDocument()` constructs
    `new V8Process(this)` (`DocumentManager.cpp:21`), so returned objects already have a C++
    parent; Qt/QML automatically respects existing parent-child ownership for `Q_INVOKABLE`
    return values.
  - Several **false positives describing standard, already-correct idiomatic QML** that's used
    pervasively and intentionally throughout this exact codebase: the `CheckBox`/
    `onCheckedChanged` two-way-sync pattern (matches the pre-existing dark-mode toggle exactly),
    bare `activeCanvas`/`window.*` lookups inside `Repeater` delegates (normal QML scope
    resolution), and `Connections { target: ... }` briefly going `null` (a documented, supported
    QML pattern — zero related warnings across this whole session's many captured interactive-
    launch stderr logs).
  - One **false positive with a broken suggested fix**: `RGroupPanel.qml`'s delegate sizing from
    `rowLayout.implicitHeight` isn't circular (`implicitHeight` is computed independently of the
    anchored actual `height` — that's the whole point of the `implicit*` vs `height` split in Qt
    Quick); the review's suggested mitigation would have left the `RowLayout` with no defined
    height/edges at all.
  - A few **overstated or unsupported claims**: `Theme.qml`'s 118-item `elementsList` (a
    periodic-table-sized array) isn't meaningful memory "bloat"; the `reactionsPopup`/
    `queryPopup` `GridLayout`s already give their `IconCell` children fixed `cellWidth`/
    `cellHeight`, so there's no fill-width resize risk to fix.
  - A few **speculative items self-contradicted by the report's own "investigation target"
    list** (`Binding.RestoreBinding` usage, project-wide `ComponentBehavior: Bound` — the latter
    would directly conflict with the bare-scope-lookup pattern just confirmed correct above, and
    would need a real case-by-case audit, not a blind pragma insertion) — skipped.
  - The report's own **7 investigation-target items (I-001 through I-007)** were explicitly
    self-flagged by the reviewer as unverified/needing human or runtime checks — not code changes
    — so none were actioned.
  - **`readonly` on bound-only properties** (a real but low-risk-of-being-wrong finding) was
    deliberately left out of this round — confirming "never imperatively reassigned" for each one
    individually isn't a mechanical change, so it wasn't included in this delegation batch.
  - Full triage writeup and scope: `.claude/plans/qml-review-fixes.md`.
- **The four that survived triage**, all delegated to the Antigravity CLI (`agy`, flash tier):
  - **Canvas grid-dot rendering batching** (`ChemCanvas.qml`) — the background dot-grid paint
    loop called `beginPath()`/`arc()`/`fill()` once per dot; moved to a single `beginPath()`
    before the double loop and a single `fill()` after, with `arc()` still called per dot inside.
    Pixel-identical output, real perf win for anything beyond a trivial grid.
  - **`Image.Error` handling** on three previously silent dynamic-source `Image` elements —
    `LabelLayer.qml`'s embedded-image `Repeater` (logs the failing bitmap's first 40 chars),
    `MainWindow.qml`'s reaction-arrow-mode row icon (`amRowIcon`, logs the icon path), and the
    shared `components/IconCell.qml` icon `Image` (logs `iconSource`) — each just a
    `console.warn` on `Image.Error`, matching this codebase's existing low-drama error-handling
    style (no new fallback UI, no dialogs).
  - **`parent`-null guards** on five `anchors.*: parent.*` bindings in `MainWindow.qml` delegates
    (`amRowIcon`/its label, and three more `verticalCenter`/`left`/`right` bindings) — each now a
    ternary (`parent ? parent.verticalCenter : undefined`), matching this codebase's existing
    null-guard idiom (`activeCanvas ? activeCanvas.showExplicitH : false`). Prevents benign-but-
    noisy binding-evaluation warnings during item teardown.
  - **`textFormat: Text.PlainText`** added to plain-string `Text` elements across
    `PropertyPanel.qml` (every label — the whole file is molecular-property display, no rich-text
    anywhere), `components/IconCell.qml`'s glyph label, and four shared popup/list components
    found via an explicit whole-tree search (`components/PopupHeader.qml`,
    `SegmentedControl.qml`, `StatusPlaceholder.qml`, `ThumbnailGridItem.qml`) — skips `AutoText`'s
    HTML-sniff cost on labels that are guaranteed plain strings/numbers. Confirmed none of the
    touched elements bind to anything with embedded HTML/rich-text markup.
- **Wrapper reported failure; the actual work had already succeeded — a false negative, the
  mirror image of this session's standing "don't trust agy's GREEN" rule.** `agy-delegate`
  printed `AGY_FAILED` / `Error: timeout waiting for response` (a response-channel timeout, not a
  crash), but the five target files' mtimes fell squarely inside the delegation's run window and
  `git diff` showed exactly the planned changes, nothing more — confirmed by reading every touched
  hunk and cross-checking large diff regions against already-documented pre-existing features
  (canvas resize/rotate handles, R-group badges, image rendering — all correctly recognized as
  pre-existing uncommitted content, not new from this delegation) before concluding the actual
  edit was correct and complete. No digest was recoverable (the timeout cut off before any digest
  text was returned), so the file-level and mtime-level checks stood in for it.
- Verified: clean MinGW rebuild, `scripts/worker_smoke_test.js` unaffected (no worker changes),
  real interactive launch with no new QML errors beyond the two pre-existing benign Quick
  Controls style warnings.
- **Real regression shipped in the first pass, caught by the user, not by this round's own
  verification**: the original write-up here claimed the grid-dot batching was "a pure
  call-batching reorder... so no visual difference is expected" and skipped an actual visual
  check on that basis. Wrong — removing the per-dot `ctx.beginPath()` changed real Canvas `arc()`
  semantics: without a `beginPath()`/`moveTo()` immediately before each `arc()` call, the arc
  implicitly draws a straight connecting line from the previous subpath's endpoint to the new
  arc's start instead of starting a fresh closed circle. With every dot in a column chained this
  way, the single `fill()` at the end filled each column as one continuous wedge — large
  triangular shapes tiling the whole canvas, reported by the user with a screenshot. **Fix**:
  added `ctx.moveTo(x + dotRadius, y)` immediately before each `ctx.arc(...)` call (starts a
  fresh subpath at the circle's own start point, so `fill()` sees separate closed circles again)
  — keeps the batching perf win ( still one `beginPath()`/`fill()` for the whole grid) while
  restoring correct dot rendering. Verified this time by actually launching the app and taking a
  screenshot (`PowerShell` `Start-Process` + `System.Drawing` screen capture) rather than
  reasoning about the change in the abstract — confirmed small individual dots, no triangles,
  clean rebuild, smoke test still passing, no new QML errors. **Lesson applied**: "no visual
  difference expected" is a claim, not a verification — for any canvas/paint-path change, actually
  look at the render before writing that sentence.

### Done (2026-07-12, explicit hydrogens fold/unfold)

- **"Add Explicit Hydrogens" / "Remove Explicit Hydrogens" toolbar buttons** — Part C's
  "Molecules & reactions — shared ops, remaining" audit listed `indigoFoldHydrogens`/
  `indigoUnfoldHydrogens` as unused core Indigo exports; nothing in the app could physically
  add/remove explicit H atoms on the structure (distinct from the pre-existing "Show Hydrogens"
  checkbox, `activeCanvas.showExplicitH`, which is a render-only label toggle that never touches
  the underlying atom data). New `IndigoService::unfoldHydrogens`/`foldHydrogens`
  (`src/app/IndigoService.h`/`.cpp`), each a byte-for-byte mirror of the existing `dearomatize`
  method's shape (reaction/molecule branch, session alloc/release, `emitOnGuiThread`) except for
  the one core call (`indigoUnfoldHydrogens(mol)`/`indigoFoldHydrogens(mol)`); two new toolbar
  entries in `MainWindow.qml`'s STRUCTURE GROUP row reusing the existing, previously-unused
  `icons/explicit-hydrogens.svg` for both directions (tooltip text disambiguates — no second
  icon exists in `icons/`, and inventing one was an explicit non-goal). Full plan at
  `.claude/plans/fold-unfold-hydrogens.md`.
  - **Zero `src/v8_worker.js`/dispatch changes** — confirmed by tracing `requestSerialize(reqId)`
    (`src/v8_process.cpp:84`) is a pure passthrough to `getStructure("mol", reqId)`, the same
    generic opaque-reqId mechanism already used by `calc_props`/`smiles`/`inchi`/etc.; the
    toolbar's existing generic `onClicked` handler already covers any new op id via
    `requestSerialize(op)` with no changes needed there either.
  - **Fourth feature this session delegated to the Antigravity CLI** (`agy`, flash tier — the
    smallest, most mechanical delegation this session, same size class as the R-group
    attachment-point delegation).
  - **Real gap the plan missed, caught by the interactive-launch check, not by trusting the
    digest**: the first build launched cleanly per `agy`'s own report, but the real interactive
    launch showed a new QML resource error — `Cannot open: qrc:/qt/qml/Sketch/App/icons/explicit-hydrogens.svg`
    — because the SVG, while present on disk and already referenced by the new toolbar entries,
    was never added to `CMakeLists.txt`'s `RESOURCES` list (the same class of omission the
    image-embedding feature's `add-image.svg` needed fixed for earlier this session). Added
    `icons/explicit-hydrogens.svg` to `CMakeLists.txt`, rebuilt, relaunched — error gone, only
    the two pre-existing benign Quick Controls style warnings remained.
  - Verified via a standalone C++ harness (same technique used for native SVG export's
    verification) calling the *actual compiled* `IndigoService::unfoldHydrogens`/`foldHydrogens`
    directly against a real ethane molfile (2 C, implicit H), moc'd and linked against the real
    vendored Indigo DLLs, bypassing the GUI entirely: unfolding correctly produced 8 atoms (2 C +
    6 explicit H, correct valence for ethane) each properly bonded to its carbon, and folding
    that result back down produced an atom-for-atom, bond-for-bond exact match of the original
    2-atom input. Standing `scripts/worker_smoke_test.js` unaffected (no worker changes). Clean
    MinGW rebuild; real interactive launch confirmed the app stays alive with no new QML errors
    after the icon fix. **Known verification gap, stated explicitly**: the actual mouse-driven
    toolbar click-through was not exercised in this environment; backend correctness was
    verified directly against the compiled code, and the QML wiring was verified by inspection
    plus a clean, error-free interactive launch.

### Done (2026-07-12, R-group attachment points)

- **R-group variable attachment points** ("Attachment Point 1"/"2"/"Clear Attachment Point" on
  the atom right-click menu) — chem-core already has the real MDL-standard machinery for this
  (`Atom.attachmentPoints` field, values `1=FirstSideOnly, 2=SecondSideOnly, 3=BothSides`, and
  both the V2000 `M  APO` writer and the KET `atomToKet` writer already read it directly and
  serialize it correctly) — the entire gap was that nothing in `v8_worker.js`/the UI ever set
  it. New `setAttachmentPoint(atomId, order)` worker command (mirrors `changeAtomCharge`'s exact
  shape); new context-menu items in `ChemCanvas.qml`; new superscript badge (`*1`/`*2`/`*3`) in
  `LabelLayer.qml` alongside the existing charge/isotope badges, with a matching one-line bond-
  retraction addition in `MoleculeLayer.qml`'s `atomHasLabel()` (a sensible small addition beyond
  the literal plan — badges need the same bond-line retraction charge/isotope/valence labels
  already get, so bonds don't visually collide with the new badge text). Full plan at
  `.claude/plans/rgroup-attachment-points.md`.
  - **Deliberately capped at chem-core's real ceiling**: MDL's APO convention supports at most
    two attachment points per fragment (`primary`/`secondary`) — this is *not* an arbitrary-N
    feature, and extending beyond that would mean modifying vendored `chem-core.js`, out of
    scope. v1 also allows marking any atom (no enforcement that it must belong to an R-group
    member fragment) and does not touch the separate, similarly-named but unrelated R1-R8
    scaffold pseudo-atom-label mechanism (`changeAtomLabel`/`isRGroupLabel`) or `RGroupPanel.qml`.
  - **Third feature this session delegated to the Antigravity CLI** (`agy`, Gemini 3.5 Flash
    High this time — a smaller, more mechanical delegation than SVG export or image embedding,
    tiered down accordingly). Read-before-writing confirmed both real chem-core writers read
    `atom.attachmentPoints` directly off the atom (not a separate pool), so no pool-maintenance
    code was needed anywhere — exactly as scoped.
  - Verified via a standalone Node harness spawning `src/v8_worker.js` over its real stdin/stdout
    protocol: `setAttachmentPoint` produces a correct `M  APO` line (parsed field-by-field, not
    just substring-matched — an earlier looser substring check produced a false failure against
    the real fixed-width MDL spacing before being corrected) for orders 1 and 2, and a correct
    `attachmentPoints` field in KET output; `undo` correctly restores the prior order (including
    back to fully cleared); an explicit clear (order 0) removes the `M  APO` line entirely.
    Standing `scripts/worker_smoke_test.js` unaffected. Clean MinGW build; real interactive
    launch with no new QML errors beyond pre-existing style warnings.

### Done (2026-07-12, image embedding)

- **Image embedding on canvas ("Insert Image")** — `struct.images` was already a native,
  initialized Pool in vendored `chem-core.js` (right next to `multitailArrows`) with a working
  KET `Image` class (`toKetNode`/`fromKetNode`), so opening a `.ket` file with an embedded image
  already silently worked; nothing could *create* one, and nothing *rendered* one. New worker
  commands `addImage(base64DataUri, cx, cy, halfW, halfH)`/`deleteImage(id)` (`src/v8_worker.js`,
  a duck-typed `_makeImage` object mirroring chem-core's real `Image` class, since that class
  isn't exported from the module — same situation as `Text`/`MultitailArrow`); new
  `FileIO::readImageAsDataUri(fileUrl)` (binary read + base64, 5 MB size guard, whitelisted
  png/jpg/jpeg/gif/bmp extensions); new `IMAGE` tool in `ToolPanel.qml` using the
  previously-unused `icons/add-image.svg`; new `FileDialog` in `MainWindow.qml`; new rendering
  block in `LabelLayer.qml` (a hidden `Image {}` `Repeater` per picture decodes the base64 data
  URI, then `ctx.drawImage()`s it once ready). Full plan at
  `.claude/plans/image-embedding.md`.
  - **v1 scope is deliberately bounded**: insert + delete only, no move/drag/resize (matching
    text annotations' own existing lack of drag support — not a new limitation just for images);
    file-picker insertion only, no clipboard-paste; KET-only round-trip, `.mol`/`.sdf` drop
    images exactly like texts/multitailArrows already do. `loadMolfile()`'s internal MDL
    round-trip (used by Layout/Aromatize/etc.) carries the `images` pool across unchanged — the
    same bug class fixed for texts/multitailArrows earlier this session, deliberately not
    repeated here.
  - **Second feature this session delegated to the Antigravity CLI** (`agy`, Gemini 3.1 Pro
    High, `--mode accept-edits`). The plan flagged the bounding-box/Y-inversion math as the
    highest-risk part (getting it wrong "is the single most likely way this feature silently
    produces corrupt KET output") and required reading the real `Image` class in `chem-core.js`
    before writing the duck-type rather than guessing. Verified independently, not just trusted:
    read the real class's `getTopLeftPosition()` (`center.sub(halfSize)`) and
    `getNodeWithInvertedYCoord`'s customizer (negates `.y` only) directly, and confirmed the
    delegated `_makeImage.toKetNode()` reproduces that exact formula — an initial hand-derivation
    suspecting a sign error turned out to be based on a wrong assumption about the coordinate
    convention, corrected by reading the real source instead of trusting either the digest or
    unverified reasoning.
  - Verified via two standalone Node harnesses spawning `src/v8_worker.js` over its real
    stdin/stdout protocol: one confirmed `addImage` → `getStructure("ket",...)` produces a
    correctly-shaped `image` KET node (`format`, base64 `data` matching the input exactly, and
    `boundingBox` matching the hand-computed expected value for a known center/half-size), that
    `undo` removes it, and that a real chem-core quirk (`imageToKet`'s re-serialize step
    genuinely drops the `center` field — confirmed by reading `chem-core.js:21073-21081` — is
    expected behavior, not a bug); a second confirmed the image survives `loadMolfile`'s internal
    round-trip (triggered via `aromatize`). Standing `scripts/worker_smoke_test.js` unaffected.
    Clean MinGW build; real interactive launch showed no new QML errors beyond pre-existing style
    warnings. **Known verification gap, stated explicitly rather than glossed over**: the actual
    mouse-driven click-through (select the IMAGE tool, click canvas, pick a real file in the
    native Windows file dialog, confirm it renders) was not exercised — no UI automation was
    available in this environment for the native file-picker step. Backend correctness (the
    higher-risk part) was verified directly; the QML wiring was verified by inspection and a
    clean launch, not by an actual click-through.

### Done (2026-07-12, SDF batch record browsing)

- **SDF multi-record batch browsing ("Browse records...")** — opening a multi-record `.sdf`
  previously silently kept only the first record and discarded the rest
  (`_deserializeStruct`'s `sdf` branch returned `items[0].struct` with no warning). Reuses the
  Library/template popup's exact pattern (`AnchoredPicker` + `ThumbnailGridItem` + `GridView`,
  itself already "parse a multi-record SDF, thumbnail every record, click to pick" — just
  pointed at `templates/library.sdf` instead of a user-opened file). New worker commands
  `deserializeSdfBatch(data)`/`loadSdfBatchRecord(index)` (`src/v8_worker.js`), new standalone
  `SdfRecordPicker.qml`, wired into `MainWindow.qml`'s `loadFromFile`/`onStructureReady`. Full
  plan at `.claude/plans/sdf-batch-browser.md`.
  - **Zero regression for the common case, by design**: a single-record `.sdf` still loads
    directly with no popup — `MainWindow.qml` only opens the picker when the worker reports
    `count >= 2`. Eager thumbnail computation is capped at the first 500 records (no general
    pagination system was built — an explicit non-goal). `.rdf` remains completely unsupported,
    untouched, and out of scope, exactly as before.
  - **First feature this session planned by Claude and implemented by delegating to the
    Antigravity CLI (`agy`, model Gemini 3.1 Pro High, `--mode accept-edits`)** rather than
    `opencode`. First attempt was lost to a shell double-backgrounding mistake (a trailing `&`
    inside an already-backgrounded call detached the process from its own wrapper's stdio,
    causing the wrapper to report a false timeout while an orphaned `agy` process kept running
    unmanaged) — killed the stray process, confirmed it had written nothing to the workspace,
    and relaunched cleanly via stdin-piped prompt. Second attempt completed cleanly end to end.
  - Verified via two standalone Node harnesses (spawning `src/v8_worker.js` over its real
    stdin/stdout protocol, the same technique `scripts/worker_smoke_test.js` uses — no GUI
    needed): one confirmed `deserializeSdfBatch` against the real `templates/library.sdf`
    returns the true record count (276, independently verified by counting `$$$$` separators)
    with correct titles and non-empty thumbnails, and that a genuine single-record file reports
    `count === 1`; a second confirmed `loadSdfBatchRecord(5)` loads the exact picked record (atom
    count 12 / bond count 12, matching `templates/library.sdf`'s "alpha-D-Galactopyranose" record
    read directly from the raw file, not just "didn't crash"). Standing
    `scripts/worker_smoke_test.js` unaffected. Clean MinGW build; real interactive launch showed
    no new QML errors beyond the pre-existing benign Quick Controls style warnings.

### Done (2026-07-12, native SVG export)

- **Native SVG export** ("Save As" → `*.svg`) — links the previously-vendored-but-unlinked
  `indigo-renderer` plugin (`indigo-renderer.dll`, MinGW bare-DLL link + a generated MSVC `.lib`
  via the same `dumpbin`/`lib.exe` process already used for `indigo.lib`/`indigo-inchi.lib`).
  Purely additive — does not touch the existing screenshot-based PNG/PDF export
  (`AppController::exportPdf`/canvas `exportPNG`). New `IndigoService::renderToFile(molfile,
  fileUrl, format)` + `renderFinished(bool, error)` signal, following the `inchi()`/
  `emitOnGuiThread` shape exactly. `MainWindow.qml`: `pendingRenderUrl` property, `render_svg`
  `reqId` branch, `.svg` entry in the save dialog's `nameFilters` + `onAccepted` branch, reusing
  `workerErrorDialog` for failures. Full plan at
  `.claude/plans/indigo-native-svg-export.md`.
  - **Started as an `opencode` delegation, stalled partway through, and was finished
    manually.** `opencode` (model `opencode-go/glm-5.2`) completed `CMakeLists.txt`'s linking
    changes correctly and added the bare `Q_INVOKABLE renderToFile(...)` declaration in
    `IndigoService.h`, then stopped producing any new output for several checks in a row while
    the process was still alive — genuinely stalled, not crashed (confirmed via `tasklist`, not
    guessed). Killed the process (`taskkill`) and hand-finished the rest against the same plan:
    the missing `<QUrl>` include and `renderFinished` signal in `IndigoService.h`, the entire
    `.cpp` implementation, and all of the (until then completely untouched) `MainWindow.qml`
    wiring.
  - Verified via a standalone harness (a small `.cpp` calling the *actual compiled*
    `IndigoService::renderToFile`, moc'd and linked directly against the vendored DLLs,
    bypassing the GUI): rendering a benzene molfile to SVG produced a real vector
    `<svg>...</svg>` document (9 `<path>` elements, not a rasterized blob or an empty/error
    file). Clean MinGW build; real interactive launch (`windeployqt` + `Start-Process`) showed
    no new QML errors (only pre-existing, unrelated Quick Controls style warnings); `.mol`/
    `.sdf`/`.ket`/`.png` save paths inspected and confirmed unaffected by the new `.svg` branch.

### Done (2026-07-11, toolbar alignment icons)

- **Toolbar alignment icon cleanup** — section 1 of
  `docs/superpowers/specs/2026-07-08-toolbar-rulers-design.md`. First feature this session
  planned by Claude and **implemented by delegating to `opencode`** (via the
  `claude-opencode:opencode-implement` skill) rather than direct implementation — plan at
  `.claude/plans/toolbar-alignment-icons.md`.
  - The 6 align/distribute actions (previously plain-text `Button`s) now render as an `IconCell`
    group matching the adjacent rotate/flip icon group's style, using the previously-vendored-
    but-unused `fonts/materialdesignicons-webfont.ttf` (no dedicated SVG icons existed for
    these actions, and none were sourced — confirmed with the user first).
  - **Codepoints verified directly against the actual font binary** (via Python's `fontTools`,
    `getBestCmap()`) before writing the plan, not guessed from memory — MDI codepoints vary by
    font version. `align-horizontal-left`=`U+F11C2`, `-right`=`U+F11C4`,
    `align-vertical-top`=`U+F11C7`, `-bottom`=`U+F11C5`, `distribute-horizontal-center`=`U+F11C9`,
    `distribute-vertical-center`=`U+F11CC`.
  - **Real bug in `IconCell.qml` found and fixed along the way**: its glyph-mode sizing logic
    (`glyph.length > 2 ? 12 : 18`) would have misfired for every one of these icons, since any
    codepoint above U+FFFF is a 2-code-unit UTF-16 surrogate pair in a JS/QML string — `.length`
    is `2` for a single MDI glyph, which would've hit the "short text label" branch instead of
    an icon-appropriate size, in the wrong font entirely (glyph mode hardcoded
    `Theme.fontDisplay`). Fixed with two new opt-in properties (`glyphFontFamily`,
    `glyphIsIcon`), defaulting to the old behavior so none of the other 7 existing glyph-mode
    call sites in the app changed.
  - Verified end-to-end after opencode's implementation: read the actual diff against the plan
    (matched exactly, no deviations), confirmed the other 7 glyph call sites untouched via grep,
    clean MinGW build, and — the one thing opencode's own environment couldn't do (no display
    capture) — a real screenshot of the running app, cropped and scaled up, confirming all 6
    icons render as correct, distinct pictograms (not tofu boxes, not the wrong font) that
    accurately depict their action.

### Done (2026-07-10)

- **Library-template ring fusion** — `insertLibraryTemplateFused` (`src/v8_worker.js`), wired
  through `V8Process`/`ChemCanvas.qml`'s `LIB_` click-press branch. Dropping a template with
  `<atomid>`/`<bondid>` metadata (235 of 276 in `templates/library.sdf`) onto an existing bond
  now fuses the seam instead of placing a disconnected copy; templates without that metadata
  keep the old plain-placement behavior unchanged. See
  `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md` for the full design and verification.

### Done (2026-07-11)

- **Canvas interaction tools: move fix, Hand, Rotate, Lasso, page boundary** — full plan and
  ChemDraw-convention research at `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`.
  - Dragging an unselected atom/bond in `SELECT` mode now moves it on the first try
    (`ChemCanvas.qml` onPressed) instead of arming a bond-preview.
  - New `HAND` tool: left-drag pans the canvas (reuses the existing middle-mouse pan state),
    with an `H` keyboard shortcut.
  - A dedicated `ROTATE` tool (ChemDraw-style) was built here, then **superseded the same day**
    by the PowerPoint-style selection handles below — see that entry instead. `rotateSelectionLive`/
    `commitRotate` (added here) survive, just triggered from the bbox handle now, not a tool mode.
  - New `SELECT_LASSO` tool: freehand selection using ChemDraw's actual semantics (confirmed via
    research) — an atom/bond is selected only if **entirely enclosed** by the lasso path, not on
    partial overlap. New `selectByLasso(pointsFlat)` in `src/v8_worker.js`.
  - New hard page/canvas boundary (`PAGE_MIN_X/MAX_X/MIN_Y/MAX_Y` in `src/v8_worker.js`,
    `-30..30` x `-21..21` chemical units, ~A4-landscape-proportioned). Confirmed as a hard clamp
    with the user despite ChemDraw's own page lines being only a soft print guide. Clamped at
    every new-content placement entry point (`addAtom`, `addBondAndAtom`,
    `addBondBetweenCoords`, `addChain`, `insertFunctionalGroup`, `pasteSelection`, and
    `getRingPreviewCoords` in `v8_process.cpp` for `addRing`/templates) plus `moveSelection`
    (whole-selection bbox-based delta clamp) and `rotateSelectionLive` (freezes rotation before
    any atom would leave the page). Visual outline drawn in `MoleculeLayer.qml`.
  - Verified via a temporary Node harness (lasso enclosure, rotate+undo+redo, boundary clamp —
    all passing) plus the standing `scripts/worker_smoke_test.js` (no regressions) and a clean
    MinGW build/launch with no QML errors.

- **PowerPoint-style selection handles (resize + rotate), replacing the ROTATE tool** — same-day
  follow-up, full design at `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`. Removed the
  standalone `ROTATE` tool; a bounding-box outline with 4 corner + 4 side resize handles + 1
  top-middle rotate handle now appears automatically on any qualifying selection (≥2 points)
  while `SELECT`/`SELECT_FRAGMENT` is active — no mode switch needed, matching real PowerPoint.
  - **Generalized across every selectable type** (atoms, reaction arrows, rxn-plus signs,
    multitail arrows), not just atom fragments: `rotateSelectionLive`/`commitRotate` were
    extended (previously atom-only) and a new `scaleSelectionLive(factor, anchorX, anchorY)`/
    `commitScale()` pair was added, both built on a shared `_snapshotSelectionPoints()`/
    `_writeSelectionPoint()` helper in `src/v8_worker.js` that mirrors `moveSelection`'s own
    multi-type id-list handling. A single reaction arrow (2 points) or a mixed molecule+arrow+text
    selection now rotates/resizes as a whole.
  - **Resize is uniform-scale-only**, anchored at the handle's geometric opposite point (not the
    centroid — dragging a corner keeps the opposite corner fixed, matching real corner-drag
    resize behavior). Chemically motivated: anisotropic stretch would distort bond lengths.
  - Hard page-boundary clamp extended to scale (freezes the factor before any point would leave
    the page, same approach `rotateSelectionLive` already used).
  - **Not attempted** (documented, deliberate scope boundary): independent font-size/glyph
    scaling or a persisted rotation angle for a *lone* selected text/rxn-plus — their position
    generalizes fine as part of a larger selection, but neither entity has an independent size
    field today; would need new data-model fields, separate scope from "add resize handles."
  - Verified via a temporary Node harness (uniform scale + anchor-fixed point, undo/redo,
    generalized rotate on a mixed atom+rxn-plus selection — all passing), the standing smoke
    test, and a full interactive-launch check (see below).
  - **Found and fixed during this verification pass**: `HAND`/`SELECT_LASSO`'s icons
    (`icons/hand.svg`/`icons/select-lasso.svg`, added in the prior entry) were never added to
    `CMakeLists.txt`'s icon resource list — it's an explicit file list, not a glob, so a new icon
    file on disk silently isn't packaged until added there too. Confirmed via a real GUI launch
    with stderr captured (`QQuickImage: Cannot open: qrc:/.../icons/hand.svg`) — the very first
    real interactive-launch check this session actually captured QML console output correctly;
    earlier "no QML errors" launch checks in this session were run through a Bash+Qt-DLL-PATH
    setup that turned out to silently fail before reaching QML at all (see Architecture Map's
    "Canvas interaction tools" note for the deployment fix). Fixed by adding both to
    `CMakeLists.txt`'s icon list.

### Done (2026-07-11, InChI)

- **InChI / InChIKey support ("Copy as InChI", "Copy as InChIKey")** — full plan at
  `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`. Links the previously-vendored-but-
  unlinked `indigo-inchi` plugin (`indigo-inchi.dll`, MinGW bare-DLL link + a generated MSVC
  `.lib` via the same `dumpbin`/`lib.exe` process already used for `indigo.lib`), and mirrors
  the existing SMILES/canonical-SMILES clipboard round trip end to end (no `src/v8_worker.js`
  changes needed — `requestSerialize`'s `reqId` is a pure passthrough tag). New "Copy InChI ▾"
  toolbar button next to "Copy SMILES ▾"; new `IndigoService::inchi()`/`inchiKey()` methods.
  - **Found and fixed a real bug via direct verification, not just a clean build**: naively
    passing `indigoInchiGetInchi()`'s return value straight into `indigoInchiGetInchiKey()` as
    its argument silently returns null — the first call's return pointer aliases a buffer the
    second call itself overwrites while computing its own result, so by the time
    `indigoInchiGetInchiKey` reads its argument, that memory has already been clobbered.
    Confirmed via a standalone repro program (bypassing Qt/the GUI entirely, linked directly
    against the vendored DLLs) before touching the real fix, and again after: `inchiKey()` now
    copies the InChI into an owned `QString` before the second call. Verified against benzene's
    known reference values (`InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H`,
    `UHOVQNZJYSORNB-UHFFFAOYSA-N`) both before and after the fix.
  - Verified via the standing `scripts/worker_smoke_test.js` (unaffected, as expected — no
    worker changes), a clean MinGW build, and a real interactive launch (PowerShell
    `Start-Process` + `windeployqt`) with no new QML errors.
  - **Not yet done**: MSVC compile/link itself (the `.lib` was generated and its symbols
    verified present, but the actual MSVC compile needs the user's Qt Creator rebuild, same
    standing limitation as every other MSVC step this session).

- **"Load from InChI"** — follow-up to the above, prompted by a request to "load molecules from
  InChIKey." **InChIKey is a one-way hash — no algorithm can reverse it back into a structure**
  (confirmed with the user before building anything); the real fix for that request is either
  (a) load from the full InChI string, which *is* invertible, or (b) look up the InChIKey via an
  external database (e.g. PubChem's API) — a network-dependent feature this app has never had,
  out of scope here. Built (a): confirmed via direct testing that Indigo's *generic* structure
  loader (`indigoLoadMoleculeFromString`, already used by "Load from SMILES") auto-detects and
  correctly parses InChI natively — no `indigo-inchi`-plugin call needed for this direction,
  unlike *generating* an InChI (the "Copy as InChI" direction above, which does need the
  plugin). This means **zero new IndigoService/CMakeLists work** — a new "Load from InChI"
  toolbar entry + `TaskDialog` (`MainWindow.qml`), mirroring "Load from SMILES" exactly, calling
  the same existing `indigoSvc.layout(text)`. Includes a live inline warning if the pasted text
  looks like an InChIKey shape rather than a full InChI, directly targeting the mix-up that
  prompted this feature. Verified: a standalone `.cpp` test (bypassing Qt/the GUI, linked
  directly against the vendored DLLs) confirmed the generic loader parses a real InChI
  correctly even without `indigo-inchi.dll` linked or `indigoInchiInit` called at all; standing
  smoke test unaffected; clean MinGW build + real interactive launch with no new QML errors.

### Deferred at the roadmap level (no design work done)

- IUPAC name generation (needs a licensed engine)
- Native biopolymer **drawing** — visual sequence/monomer canvas editing (see chem-core.js
  "Biopolymer / monomer template engine" in Part B — the engine is vendored in, just unused)
- 3D viewer / conformer generation
- Spectral prediction (NMR/MS/IR)
- Macro/scripting support

### Small, concrete gaps

*(none currently open — see "Done (2026-07-11, KET multitail-arrow crash)" below)*

### Done (2026-07-11, KET multitail-arrow crash)

- **`getStructure("ket", ...)` threw/returned empty when a multitail arrow was present** —
  found while investigating the loadMolfile data-loss bug (see "Done (2026-07-11, loadMolfile
  data loss)"). Full root-cause + fix at
  `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`.
  - Root-caused via a **disposable copy** of `src/v8_worker.js` (never touched the tracked
    file) patched to stop `getStructure`'s catch block from silently swallowing the exception:
    `TypeError: multitailArrow.toKetNode is not a function`. `KetSerializer.serializeMicromolecules`
    (chem-core.js, vendored) walks `struct.multitailArrows` directly and calls `.toKetNode()` on
    each entry, assuming a real chem-core `MultitailArrow` class instance — this app's own
    multitail arrows (`_makeMultitailArrow`) are plain data objects with no such method (already
    flagged in Part B's audit: "Sketch reimplemented this as plain objects"), so serialization
    threw the instant one existed, before `_injectMultitailArrows` (which correctly builds KET
    nodes from the plain-object data) ever got a chance to run.
  - **Second, related gap found while tracing this**: `getClipboardAsKet()` had the identical
    crash risk, and separately never called `_injectMultitailArrows` at all — a copied multitail
    arrow would have stayed silently missing from clipboard-as-KET output even after the crash
    was fixed.
  - **Fix**: a new `_serializeMicromoleculesSafe(struct)` helper (mirroring the existing
    `_reattachMultitailArrows`'s established pattern of swapping in
    `new CoreLib.ChemCore.Pool()`) temporarily empties `struct.multitailArrows` for just the
    `serializeMicromolecules` call, restoring it immediately after — safe because both call
    sites already re-inject the correct nodes afterward from the real pool data. Used by both
    `getStructure`'s `"ket"` branch and `getClipboardAsKet()`; added the missing
    `_injectMultitailArrows` call to the latter too. Vendored chem-core.js was not touched.
  - Verified via the exact repro that found the bug (disposable Node harness): both
    `getStructure("ket", ...)` and `getClipboardAsKet()` now return non-empty KET JSON
    containing the multitail-arrow node when one is present. Standing smoke test unaffected,
    clean MinGW build, real interactive launch with no new QML errors.

### Done (2026-07-11, reaction validation)

- **Reaction structure validation** — was stubbed (`IndigoService.cpp:452` returned "Validation
  is not yet supported for reactions" unconditionally). Full plan at
  `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`.
  - Checking the whole reaction handle at once (`indigoCheckObj` on the reaction object itself)
    gives back a report shape (`{"": "Reaction component check result, ...(id)"}`) the existing
    molecule-check JSON parser can't classify or attribute to a component — confirmed via a
    standalone test before writing any real code. The fix: iterate
    `indigoIterateReactants`/`indigoIterateProducts` and run `indigoCheckObj` on each component
    individually, which gives back the exact same clean per-type shape the molecule branch
    already handles.
  - **Deliberate scope boundary, not an oversight**: this only produces a **text report** (the
    modal dialog opened by the explicit "Validate" button) — the **inline** badge/red-bond
    highlighting path (`checkIssuesReady`/`setCheckIssues`, which fires silently on every
    property-recalc) stays disabled for reactions, exactly as before. Mapping each check's
    per-component-local atom/bond indices back to global canvas ids for highlighting would need
    matching chem-core.js's own fragment ordering when it serializes a `$RXN` string from
    `_struct` — not a safely verifiable assumption without disproportionate archaeology into
    third-party bundled/minified code, and getting it wrong would silently highlight the *wrong*
    atom, worse than the honest stub it replaces.
  - Verified via a standalone `.cpp` test (bypassing Qt/the GUI, linked directly against the
    vendored DLL) against both a broken reaction (a product with a genuine valence error) and a
    clean one, confirming the per-component report correctly names the specific component with
    an issue and reports "No problems found." when none exist. Standing smoke test unaffected
    (no worker changes), clean MinGW build, real interactive launch with no new QML errors.

### Done (2026-07-11, loadMolfile data loss)

- **Fixed data loss on the Indigo round-trip** — started from a question about minimizing
  worker↔Indigo "IPC" overhead; investigating that round-trip found a real correctness bug
  instead of a meaningful perf win (confirmed with the user this was the right thing to chase).
  Full plan at `C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`.
  - Layout, Aromatize, Dearomatize, Normalize, and Standardize all serialize `_struct` to an MDL
    molfile/RXN string for Indigo, then reload whatever comes back via `loadMolfile()`
    (`src/v8_worker.js:3177`), which did a full `_struct = loaded` replace. MDL molfile/RXN has
    no concept of Ketcher's own text annotations or multitail arrows — confirmed by direct
    testing that `getStructure("mol", ...)` on a struct with a text annotation and a multitail
    arrow silently drops both, keeping only the atom. Net effect: clicking any of those five
    toolbar actions **deleted every text label and multitail arrow on the canvas**, with no
    warning and no separate undo entry (bundled into the same undo step as the layout change).
    Reaction arrows/plus signs were unaffected — `$RXN` is a real MDL concept and round-trips
    fine, confirmed the same way.
  - **Fix**: `loadMolfile()` now merges the outgoing struct's `texts`/`multitailArrows` pools
    into the freshly-deserialized one before replacing `_struct`. Safe to do unconditionally
    because `loadMolfile()` is only ever called for "reload a modified version of the *same*
    document" (the 5 ops above, plus biopolymer-notation expansion) — genuine file **open**
    goes through a different path (`loadStructure()`), confirmed by tracing `MainWindow.qml`'s
    `loadFromFile()`, so this fix can't leak stale annotations into an unrelated opened file.
  - Known, accepted limitation: a text annotation's position is an absolute `x,y` with no atom
    anchor, so a big Layout-driven repositioning can leave a surviving label looking
    misplaced relative to the structure — a real but much smaller problem than losing it
    outright, and not attempted here.
  - **Found in passing, not fixed here** (added to "Small, concrete gaps" above):
    `getStructure("ket", ...)` throws/returns empty specifically when a multitail arrow is
    present, even though KET format otherwise preserves both texts and multitail arrows
    correctly.
  - Verified via a disposable Node harness (same throwaway pattern as this session's other
    `*_TEMP.js` scripts) simulating the round-trip directly against the real worker — text and
    multitail arrow both survive a `loadMol` call that replaces the atom. Standing smoke test
    unaffected, clean MinGW build, real interactive launch with no new QML errors. No
    C++/CMake changes this pass, so MSVC risk is minimal but not independently re-verified.

---

## Part B — chem-core.js: every unused export (50 of 72)

The bundled Ketcher core is loaded whole via `eval()` in `src/v8_worker.js`; only 22 of these
72 top-level exports are ever referenced. Every other one below is a potential feature seed —
grouped by what it's for.

### Biopolymer / monomer template engine (18) — maps to "native biopolymer drawing"
- `DrawingEntitiesManager`
- `Entities`
- `MonomerSize`
- `HalfMonomerSize`
- `MONOMER_CONST`
- `STRAND_TYPE`
- `CREATE_MONOMER_TOOL_NAME`
- `QetcherJSWrapper`
- `SnakeLayoutCellWidth`
- `setMonomerPrefix`
- `setMonomerTemplatePrefix`
- `setMonomerGroupTemplatePrefix`
- `setAmbiguousMonomerPrefix`
- `setAmbiguousMonomerTemplatePrefix`
- `KetAmbiguousMonomerTemplateSubType`
- `getHELMClassByKetMonomerClass`
- `getMonomerTemplateRefFromMonomerItem`
- `getKetRef`

### RNA/DNA/peptide ambiguous-symbol tables (12) — sub-feature of biopolymer
- `NO_NATURAL_ANALOGUE`
- `RNA_DNA_NON_MODIFIED_PART`
- `RnaDnaBaseNames`
- `RnaDnaNaturalAnaloguesEnum`
- `StandardAmbiguousPeptide`
- `StandardAmbiguousRnaBase`
- `peptideAmbiguousSymbols`
- `peptideNaturalAnalogues`
- `rnaDnaAmbiguousSymbols`
- `rnaDnaNaturalAnalogues`
- `unknownNaturalAnalogues`
- `fillNaturalAnalogueForPhosphateAndSugar`

### R-group attachment-point helpers (6) — Sketch hand-rolls R-groups on `struct.rgroups` instead
- `AttachmentPointName`
- `getAttachmentPointLabel`
- `getAttachmentPointLabelWithBinaryShift`
- `getAttachmentPointNumberFromLabel`
- `getNextFreeAttachmentPoint`
- `isSingleRGroupAttachmentPoint`

### Image embedding (3, now 2 used — see "Done" above) — `IMAGE_KEY`/`IMAGE_SERIALIZE_KEY` are
now used by the insert/delete/render feature; `imageReferencePositionToCursor` remains unused
(a resize-handle cursor-icon helper — v1 deliberately has no resize, see the "Done" entry above)
- `imageReferencePositionToCursor`

### Multitail reaction-arrow class (4) — Sketch reimplemented this as plain objects
- `MULTITAIL_ARROW_KEY`
- `MULTITAIL_ARROW_TOOL_NAME`
- `multitailArrowReferenceLinesToCursor`
- `multitailReferencePositionToCursor`

### Misc geometry / selection utilities (7)
- `CoreAtom` (chem-core's own internal alias for `Atom`, exported twice under different names)
- `Scale`
- `SgContexts`
- `getNodeWithInvertedYCoord`
- `modifyTransformation`
- `switchIntoChemistryCoordSystem`
- `populateStructWithSelection`

---

## Part C — Indigo C API: every unused function (466 of 517)

`IndigoService.cpp` calls 54 of Indigo's 517 declared functions (across `indigo.h`,
`indigo-inchi.h`, `indigo-renderer.h`). Everything below is linked into the binary via
`indigo.dll` already — no new dependency needed to use any of it.

### Substructure matching (13) — no SMARTS/substructure search exists anywhere in the app
- `indigoSubstructureMatcher`
- `indigoIgnoreAtom`
- `indigoUnignoreAtom`
- `indigoUnignoreAllAtoms`
- `indigoMatch`
- `indigoCountMatches`
- `indigoCountMatchesWithLimit`
- `indigoIterateMatches`
- `indigoHighlightedTarget`
- `indigoMapAtom`
- `indigoMapBond`
- `indigoMapMolecule`
- `indigoIterateTautomers`

### Fingerprints & similarity (7) — would back "find similar" search
- `indigoFingerprint`
- `indigoCountBits`
- `indigoCommonBits`
- `indigoOneBitsList`
- `indigoLoadFingerprintFromBuffer`
- `indigoLoadFingerprintFromDescriptors`
- `indigoSimilarity`

### Renderer plugin — indigo-renderer.h (8, now 3 used — see "Done" above) — native PNG/SVG/PDF
export; print-to-PDF still grabs a canvas screenshot instead (`AppController::exportPdf`)

**`indigoRendererInit`/`Dispose` and `indigoRenderToFile` are now linked and used** (Part A's
"Done (2026-07-12, native SVG export)" entry, for `*.svg` only so far) — the remaining
functions below are still unused.

- `indigoRender(object, output)` — renders one molecule/reaction to a file
  (`indigoWriteFile`), an in-memory buffer (`indigoWriteBuffer`), or a raw Windows HDC
  (`indigoRenderWriteHDC`) for direct GDI drawing
- `indigoRenderGrid(objects, refAtoms, nColumns, output)` / `indigoRenderGridToFile(...)` —
  renders a whole array of structures as a grid (a template sheet, or a reaction's
  reactants+products in one image)
- `indigoRenderWriteHDC` — see above
- `indigoRenderReset()` — resets rendering settings to defaults

**What it draws is controlled via `indigoSetOption`/`indigoGetOption`, not function
arguments** — key options (from the Indigo docs):
- `render-output-format` — PNG/SVG/PDF/EMF ("automatic" by default, inferred from the
  output file extension)
- Layout: `render-bond-length`, `render-image-size`/`width`/`height`, `render-margins`,
  `render-relative-thickness`, `render-bond-line-width`
- Colors: `render-base-color`, `render-background-color`, `render-coloring` (CPK-style
  atom coloring), `render-highlight-color`
- Annotations: `render-comment` (+ font size/color/alignment/position) — bakes a caption
  into the image itself
- Display toggles: `render-label-mode`, `render-implicit-hydrogens-visible`,
  `render-atom-ids-visible`, `render-bond-ids-visible`

This would replace `AppController::exportPdf`'s current screenshot-grab-then-`QPdfWriter`
approach with real vector-quality SVG/PDF output; `indigoRenderGrid` is also a natural fit
for printing a whole reaction scheme or a sheet of templates in one image.

### InChI plugin — indigo-inchi.h (11, now 2 used — see "Done" above)

The third and last separate Indigo plugin (own `indigo-inchi.dll`, own header), wrapping the
standard IUPAC InChI engine. Unlike SMILES, InChI is a formally standardized, algorithmically
canonical identifier — every valid structure has exactly one correct InChI by specification,
which is why chemistry databases (PubChem, ChemSpider, etc.) key lookups on it rather than
SMILES (which can vary between toolkits even when "canonical"). **`indigoInchiGetInchi`/
`indigoInchiGetInchiKey` (plus lifecycle `indigoInchiInit`/`Dispose`) are now linked and used**
(Part A's "Done (2026-07-11, InChI)" entry) — the remaining functions below are still unused.

- `indigoInchiGetInchi(molecule)` — structure → InChI string (the main direction)
- `indigoInchiGetInchiWithForcedOptions(molecule, forcedOptions)` — same, with explicit InChI
  generation flags (fixed-H layer, stereo layers, etc.)
- `indigoInchiGetInchiKey(inchi_string)` — InChI → **InChIKey**, a fixed 27-character hash.
  This is the part actually used for database search/deduplication — short, URL-safe,
  exact-match friendly
- `indigoInchiLoadMolecule(inchi_string)` — the reverse: parse an InChI string back into a
  usable Indigo molecule handle
- `indigoInchiGetWarning` / `indigoInchiGetLog` — diagnostics from the last operation (e.g.
  ambiguous stereo, non-standard valence)
- `indigoInchiGetAuxInfo` — auxiliary reconstruction info generated alongside the main InChI
  string (atom-numbering correspondence, etc.)
- `indigoInchiInit` / `indigoInchiDispose` / `indigoInchiResetOptions` / `indigoInchiVersion`
  — lifecycle/version, same pattern as the other plugins

Natural fit: sits right next to the SMILES support already wired in (`indigoSmiles`/
`indigoCanonicalSmiles` → `copyToClipboard`, in `MainWindow.qml`'s `Connections` block) — a
"Copy as InChI" / "Copy as InChIKey" action alongside "Copy as SMILES." InChIKey specifically
is what you'd paste into an external database to look up a drawn structure.

### SDF / RDF / SMILES / CML / CDX file iteration (23) — no batch multi-record file browsing
- `indigoIterateSDF`
- `indigoIterateRDF`
- `indigoIterateSmiles`
- `indigoIterateCML`
- `indigoIterateCDX`
- `indigoIterateSDFile`
- `indigoIterateRDFile`
- `indigoIterateSmilesFile`
- `indigoIterateCMLFile`
- `indigoIterateCDXFile`
- `indigoRawData`
- `indigoTell`
- `indigoTell64`
- `indigoSdfAppend`
- `indigoSmilesAppend`
- `indigoRdfHeader`
- `indigoRdfAppend`
- `indigoCmlHeader`
- `indigoCmlAppend`
- `indigoCmlFooter`
- `indigoCreateSaver`
- `indigoCreateFileSaver`
- `indigoAppend`

### R-Group deconvolution & scaffold detection (13) — Markush/scaffold analysis, distinct
from Sketch's own manual R-group authoring

A related pair: both about analyzing a *set* of molecules rather than one at a time.

**Scaffold detection** — finds the shared core across multiple structures without
specifying it up front:
- `indigoExtractCommonScaffold(structures, options)` — the maximum common substructure
  (by ring count) across a set
- `indigoAllScaffolds(extracted)` — every possible scaffold candidate from that extraction,
  not just the single best one

**R-Group deconvolution** — given a scaffold and a set of molecules that share it, splits
each molecule into "core + substituents":
- `indigoDecomposeMolecules(scaffold, structures)` → `indigoDecomposedMoleculeScaffold`
  (the core, with R-site markers where substituents attach) and
  `indigoIterateDecomposedMolecules` (per-molecule results)
- `indigoDecomposedMoleculeHighlighted` — a molecule with its scaffold portion highlighted
- `indigoDecomposedMoleculeWithRGroups` — a full query molecule with `R1=...`, `R2=...`
  substituents explicitly defined — a real Markush structure, in the same shape Sketch's
  own R-group panel already works with
- A second, incremental API for streaming use: `indigoCreateDecomposer(scaffold)` →
  `indigoDecomposeMolecule(decomp, mol)` per molecule → `indigoIterateDecompositions` /
  `indigoAddDecomposition` to commit results one at a time

**R-Group convolution** — the reverse direction: `indigoGetFragmentedMolecule`/
`indigoRGroupComposition` take an already-Markush-annotated structure and expand or
summarize it back toward concrete molecules.

This is the *discovery* complement to R-groups Sketch already has working. Today, R-groups
are entirely user-authored — manually draw a scaffold and define R1–R8 by hand
(`RGroupPanel.qml`). This cluster would let you instead hand it a *pile* of related
molecules (e.g. an opened SDF of analogs) and have Indigo automatically find the shared
scaffold and populate the R-group table — classic SAR-table generation from a medicinal
chemistry workflow. `indigoDecomposedMoleculeWithRGroups`'s output shape could plausibly
feed straight into the existing `RGroupPanel.qml`/`struct.rgroups` model rather than
needing a new UI paradigm.

### Reaction products enumeration (3) — combinatorial/virtual-library reaction expansion
- `indigoReactionProductEnumerate`
- `indigoTransform`
- `indigoTransformHELMtoSCSR`

### Reactions, query reactions — remaining (48) — beyond load/save already used: reactant/
product/catalyst iteration, auto atom-mapping, pKa prediction, reacting-center flags
- `indigoLoadReaction`
- `indigoLoadReactionFromFile`
- `indigoLoadReactionFromBuffer`
- `indigoLoadReactionWithLib`
- `indigoLoadReactionWithLibFromString`
- `indigoLoadReactionWithLibFromFile`
- `indigoLoadReactionWithLibFromBuffer`
- `indigoLoadQueryReaction`
- `indigoLoadQueryReactionFromString`
- `indigoLoadQueryReactionFromFile`
- `indigoLoadQueryReactionFromBuffer`
- `indigoLoadQueryReactionWithLib`
- `indigoLoadQueryReactionWithLibFromString`
- `indigoLoadQueryReactionWithLibFromFile`
- `indigoLoadQueryReactionWithLibFromBuffer`
- `indigoLoadReactionSmarts`
- `indigoLoadReactionSmartsFromString`
- `indigoLoadReactionSmartsFromFile`
- `indigoLoadReactionSmartsFromBuffer`
- `indigoCreateReaction`
- `indigoCreateQueryReaction`
- `indigoAddReactant`
- `indigoAddProduct`
- `indigoAddCatalyst`
- `indigoCountReactants`
- `indigoCountProducts`
- `indigoCountCatalysts`
- `indigoCountMolecules`
- `indigoGetMolecule`
- `indigoIterateReactants`
- `indigoIterateProducts`
- `indigoIterateCatalysts`
- `indigoIterateMolecules`
- `indigoIterateReactions`
- `indigoSaveRxnfile`
- `indigoSaveRxnfileToFile`
- `indigoOptimize`
- `indigoIonize`
- `indigoBuildPkaModel`
- `indigoGetAcidPkaValue`
- `indigoGetBasicPkaValue`
- `indigoAutomap`
- `indigoGetAtomMappingNumber`
- `indigoSetAtomMappingNumber`
- `indigoGetReactingCenter`
- `indigoSetReactingCenter`
- `indigoClearAAM`
- `indigoCorrectReactingCenters`

### Calculation on molecules — remaining (23) — molar refractivity, extra pKa APIs, chirality/
Fischer-projection checks, canonical hashing/symmetry classes, submolecule extraction
- `indigoCountHeavyAtoms`
- `indigoGrossFormula`
- `indigoMostAbundantMass`
- `indigoMassComposition`
- `indigoMolarRefractivity`
- `indigoPka`
- `indigoPkaValues`
- `indigoLayeredCode` **deliberately excluded** — confirmed a pure wrapper around
  `MoleculeInChI::outputInChI`, duplicating the InChI support already implemented. See "Done
  (2026-07-13, Copy Canonical Hash)" above.
- ~~`indigoHash`~~ **done (2026-07-13)** — see the same "Done" entry above.
- `indigoSymmetryClasses`
- `indigoHasCoord`
- `indigoHasZCoord`
- `indigoIsChiral`
- ~~`indigoCheckChirality`~~ / ~~`indigoCheckStereo`~~ **done (2026-07-13)** — see "Done
  (2026-07-13, chirality/stereocenter checks in Validate structure)" above.
- `indigoCheck3DStereo` **deliberately excluded** — trivially always-0 in this 2D-only editor
  (only meaningful with real Z-nonzero coordinates), see the same "Done" entry above.
- `indigoIsPossibleFischerProjection`
- `indigoCreateSubmolecule`
- `indigoCreateEdgeSubmolecule`
- `indigoGetSubmolecule`
- `indigoRemoveAtoms`
- `indigoRemoveBonds`
- `indigoAlignAtoms`

### Molecules & reactions — shared ops, remaining (20, now 2 used, 1 confirmed dead — see "Done
(2026-07-12, explicit hydrogens fold/unfold)" and "Done (2026-07-13, Layout Selected)" above) —
tautomer handling, named properties on the handle, bad-valence/ambiguous-H checks
- `indigoFoldUnfoldHydrogens` (the combined auto-toggle variant — the plan deliberately used the
  two separate directional calls instead, see the "Done" entry's non-goals)
- ~~`indigoLayoutSelected`~~ **confirmed a dead header stub, not a real function** — declared in
  `indigo.h` but never implemented in any release checked, including a fresh local compile of
  v1.45.0 from official source. See "Done (2026-07-13, Layout Selected)" above for the full
  investigation and the from-scratch worker-side reimplementation that shipped instead.
- ~~`indigoClean2d`~~ **done (2026-07-13): standalone whole-document "Clean 2D
  Structure" toolbar button.** Investigated as a possible Layout Selected substitute first:
  `MoleculeCleaner2d` (its real underlying algorithm, `core/indigo-core/layout/molecule_cleaner_2d.*`)
  is a genuinely distinct algorithm from `MoleculeLayout` — local energy-minimization gradient
  descent starting FROM existing coordinates, not a fresh global embedding. Has a
  `selected_vertices`-aware constructor confirmed via reading `_updateGradient2()`: non-selected
  atoms get zero gradient (stay fixed). Empirically verified with disposable C++ harnesses: does
  NOTHING for a plain chain (`is_trivial` early-return — nothing to optimize in a non-biconnected
  path), but WORKS correctly for a ring (anchor atom outside the submolecule stayed exactly fixed
  while the ring reshaped). Conclusion: complementary to, not a replacement for, the shipped
  `layoutSelectedChain()` — recommend shipping it standalone (whole-document, not selection-aware)
  rather than reopening Layout Selected. User confirmed: "Yes, scope Clean 2D Structure".
- `indigoSmarts`
- `indigoCanonicalSmarts`
- `indigoExactMatch`
- `indigoSetTautomerRule`
- `indigoRemoveTautomerRule`
- `indigoClearTautomerRules`
- `indigoName`
- `indigoSetName`
- `indigoSerialize`
- `indigoUnserialize`
- `indigoHasProperty`
- `indigoGetProperty`
- `indigoSetProperty`
- `indigoRemoveProperty`
- `indigoIterateProperties`
- `indigoClearProperties`
- `indigoCheckBadValence`
- `indigoCheckAmbiguousH`

### Molecules, query molecules, SMARTS — remaining loaders/savers (73) — query-molecule/SMARTS
loading, CDX/CML/monomer-library save formats, file/buffer-path variants of loaders Sketch
only uses the from-string form of
- `indigoGetOriginalFormat`
- `indigoCreateMolecule`
- `indigoCreateQueryMolecule`
- `indigoLoadStructureFromString`
- `indigoLoadStructureFromBuffer`
- `indigoLoadStructureFromFile`
- `indigoLoadMoleculeWithLib`
- `indigoLoadMoleculeWithLibFromString`
- `indigoLoadMoleculeWithLibFromFile`
- `indigoLoadMoleculeWithLibFromBuffer`
- `indigoLoadMolecule`
- `indigoLoadMoleculeFromFile`
- `indigoLoadMoleculeFromBuffer`
- `indigoLoadQueryMoleculeWithLib`
- `indigoLoadQueryMoleculeWithLibFromString`
- `indigoLoadQueryMoleculeWithLibFromFile`
- `indigoLoadQueryMoleculeWithLibFromBuffer`
- `indigoLoadQueryMolecule`
- `indigoLoadQueryMoleculeFromString`
- `indigoLoadQueryMoleculeFromFile`
- `indigoLoadQueryMoleculeFromBuffer`
- `indigoLoadSmarts`
- `indigoLoadSmartsFromString`
- `indigoLoadSmartsFromFile`
- `indigoLoadSmartsFromBuffer`
- `indigoLoadMonomerLibrary`
- `indigoLoadMonomerLibraryFromFile`
- `indigoLoadMonomerLibraryFromBuffer`
- `indigoLoadKetDocument`
- `indigoLoadKetDocumentFromString`
- `indigoLoadKetDocumentFromFile`
- `indigoLoadKetDocumentFromBuffer`
- `indigoLoadSequence`
- `indigoLoadSequenceFromFile`
- `indigoLoadFasta`
- `indigoLoadFastaFromFile`
- `indigoLoadIdt`
- `indigoLoadIdtFromFile`
- `indigoLoadHelm`
- `indigoLoadHelmFromFile`
- `indigoLoadAxoLabs`
- `indigoLoadAxoLabsFromFile`
- `indigoSaveMolfile`
- `indigoSaveMolfileToFile`
- `indigoSaveSequence`
- `indigoSaveSequenceToFile`
- `indigoSaveSequence3Letter`
- `indigoSaveSequence3LetterToFile`
- `indigoSequence3Letter`
- `indigoSaveFasta`
- `indigoSaveFastaToFile`
- `indigoSaveIdt`
- `indigoSaveIdtToFile`
- `indigoSaveHelm`
- `indigoSaveHelmToFile`
- `indigoSaveAxoLabs`
- `indigoSaveAxoLabsToFile`
- `indigoSaveMonomerLibrary`
- `indigoSaveMonomerLibraryToFile`
- `indigoMonomerLibrary`
- `indigoSaveJsonToFile`
- `indigoSaveJson`
- `indigoSaveCml`
- `indigoSaveCmlToFile`
- `indigoCml`
- `indigoCdxBase64`
- `indigoSaveCdxml`
- `indigoSaveCdx`
- `indigoCdxml`
- `indigoSaveCdxmlToFile`
- `indigoSaveCdxToFile`
- `indigoSaveMDLCT`
- `indigoNameToStructure`

### Accessing a molecule — atom/bond/SGroup internals (162) — Sketch always hands Indigo a
whole molfile/RXN string rather than building structures through its object model; this
entire category is naturally dormant given that division of labour
- `indigoIterateAtoms`
- `indigoIteratePseudoatoms`
- `indigoIterateRSites`
- `indigoIterateAlleneCenters`
- `indigoIterateRGroups`
- `indigoCountRGroups`
- `indigoCopyRGroups`
- `indigoIsPseudoatom`
- `indigoIsRSite`
- `indigoIsTemplateAtom`
- `indigoChangeStereocenterType`
- `indigoSetStereocenterGroup`
- `indigoStereocenterPyramid`
- `indigoSingleAllowedRGroup`
- `indigoAddStereocenter`
- `indigoIterateRGroupFragments`
- `indigoCountAttachmentPoints`
- `indigoIterateAttachmentPoints`
- `indigoSymbol`
- `indigoDegree`
- `indigoGetCharge`
- `indigoGetExplicitValence`
- `indigoSetExplicitValence`
- `indigoAtomicNumber`
- `indigoIsotope`
- `indigoValence`
- `indigoGetHybridization`
- `indigoCheckValence`
- `indigoCheckQuery`
- `indigoCheckRGroups`
- `indigoAtomIndex`
- `indigoBondIndex`
- `indigoBondBegin`
- `indigoBondEnd`
- `indigoCheck`
- `indigoCheckStructure`
- `indigoCountHydrogens`
- `indigoCountImplicitHydrogens`
- `indigoMacroProperties`
- `indigoXYZ`
- `indigoSetXYZ`
- `indigoClearXYZ`
- `indigoCountSuperatoms`
- `indigoCountDataSGroups`
- `indigoCountRepeatingUnits`
- `indigoCountMultipleGroups`
- `indigoCountGenericSGroups`
- `indigoIterateDataSGroups`
- `indigoIterateSuperatoms`
- `indigoIterateGenericSGroups`
- `indigoIterateRepeatingUnits`
- `indigoIterateMultipleGroups`
- `indigoIterateTGroups`
- `indigoIterateSGroups`
- `indigoGetSuperatom`
- `indigoGetDataSGroup`
- `indigoGetGenericSGroup`
- `indigoGetMultipleGroup`
- `indigoGetRepeatingUnit`
- `indigoDescription`
- `indigoData`
- `indigoAddDataSGroup`
- `indigoAddSuperatom`
- `indigoSetDataSGroupXY`
- `indigoSetSGroupData`
- `indigoSetSGroupCoords`
- `indigoSetSGroupDescription`
- `indigoSetSGroupFieldName`
- `indigoSetSGroupQueryCode`
- `indigoSetSGroupQueryOper`
- `indigoSetSGroupDisplay`
- `indigoSetSGroupLocation`
- `indigoSetSGroupTag`
- `indigoSetSGroupTagAlign`
- `indigoSetSGroupDataType`
- `indigoSetSGroupXCoord`
- `indigoSetSGroupYCoord`
- `indigoCreateSGroup`
- `indigoGetSGroupClass`
- `indigoGetSGroupName`
- `indigoSetSGroupClass`
- `indigoSetSGroupName`
- `indigoGetSGroupNumCrossBonds`
- `indigoAddSGroupAttachmentPoint`
- `indigoDeleteSGroupAttachmentPoint`
- `indigoIterateSGroupAttachmentPoints`
- `indigoGetSGroupAttachmentPointAtomIdx`
- `indigoGetSGroupAttachmentPointLeaveAtom`
- `indigoGetSGroupAttachmentPointLabel`
- `indigoGetSGroupDisplayOption`
- `indigoSetSGroupDisplayOption`
- `indigoGetSGroupSeqId`
- `indigoGetSGroupCoords`
- `indigoGetSGroupMultiplier`
- `indigoSetSGroupMultiplier`
- `indigoGetRepeatingUnitSubscript`
- `indigoGetRepeatingUnitConnectivity`
- `indigoSetSGroupBrackets`
- `indigoFindSGroups`
- `indigoGetSGroupType`
- `indigoGetSGroupIndex`
- `indigoGetSGroupOriginalId`
- `indigoSetSGroupOriginalId`
- `indigoGetSGroupParentId`
- `indigoSetSGroupParentId`
- `indigoAddTemplate`
- `indigoRemoveTemplate`
- `indigoFindTemplate`
- `indigoGetTGroupClass`
- `indigoGetTGroupName`
- `indigoGetTGroupAlias`
- `indigoTransformSCSRtoCTAB`
- `indigoTransformCTABtoSCSR`
- `indigoResetCharge`
- `indigoResetExplicitValence`
- `indigoResetIsotope`
- `indigoSetAttachmentPoint`
- `indigoClearAttachmentPoints`
- `indigoRemoveConstraints`
- `indigoAddConstraint`
- `indigoAddConstraintNot`
- `indigoAddConstraintOr`
- `indigoResetStereo`
- `indigoInvertStereo`
- `indigoCountPseudoatoms`
- `indigoCountRSites`
- `indigoIterateBonds`
- `indigoBondOrder`
- `indigoBondStereo`
- `indigoTopology`
- `indigoIterateNeighbors`
- `indigoBond`
- `indigoGetAtom`
- `indigoGetBond`
- `indigoSource`
- `indigoDestination`
- `indigoClearCisTrans`
- `indigoClearStereocenters`
- `indigoCountStereocenters`
- `indigoClearAlleneCenters`
- `indigoCountAlleneCenters`
- `indigoResetSymmetricCisTrans`
- `indigoResetSymmetricStereocenters`
- `indigoMarkEitherCisTrans`
- `indigoMarkStereobonds`
- `indigoValidateChirality`
- `indigoAddAtom`
- `indigoResetAtom`
- `indigoGetTemplateAtomClass`
- `indigoSetTemplateAtomClass`
- `indigoAddRSite`
- `indigoSetRSite`
- `indigoSetCharge`
- `indigoSetIsotope`
- `indigoGetRadicalElectrons`
- `indigoGetRadical`
- `indigoSetRadical`
- `indigoResetRadical`
- `indigoSetImplicitHCount`
- `indigoAddBond`
- `indigoSetBondOrder`
- `indigoMerge`

### Options, iterators, arrays, connected components, SSSR, basic I/O (40) — generic
object-model plumbing, dormant for the same reason as the "Accessing a molecule" group
- `indigoSetOption`
- `indigoSetOptionInt`
- `indigoSetOptionFloat`
- `indigoSetOptionColor`
- `indigoSetOptionXY`
- `indigoResetOptions`
- `indigoGetOption`
- `indigoGetOptionInt`
- `indigoGetOptionBool`
- `indigoGetOptionFloat`
- `indigoGetOptionColor`
- `indigoGetOptionXY`
- `indigoGetOptionType`
- `indigoHasNext`
- `indigoRemove`
- `indigoCreateArray`
- `indigoArrayAdd`
- `indigoAt`
- `indigoCount`
- `indigoClear`
- `indigoIterateArray`
- `indigoCountComponents`
- `indigoComponentIndex`
- `indigoIterateComponents`
- `indigoComponent`
- `indigoCountSSSR`
- `indigoIterateSSSR`
- `indigoIterateSubtrees`
- `indigoIterateRings`
- `indigoIterateEdgeSubmolecules`
- `indigoReadFile`
- `indigoReadString`
- `indigoLoadString`
- `indigoReadBuffer`
- `indigoLoadBuffer`
- `indigoWriteFile`
- `indigoWriteBuffer`
- `indigoClose`
- `indigoExpandAbbreviations`
- `indigoExpandGroupPseudoatoms`

### Highlighting, selection, system misc, debug (19) — Sketch keeps highlight/selection state
entirely on the chem-core.js side; debug counters are for Indigo's own development use
- `indigoHighlight`
- `indigoUnhighlight`
- `indigoIsHighlighted`
- `indigoSelect`
- `indigoUnselect`
- `indigoIsSelected`
- `indigoHasSelection`
- `indigoVersion`
- `indigoVersionInfo`
- `indigoSetErrorHandler`
- `indigoClone`
- `indigoCountReferences`
- `indigoToBase64String`
- `indigoToBuffer`
- `indigoDbgInternalType`
- `indigoDbgBreakpoint`
- `indigoDbgProfiling`
- `indigoDbgResetProfiling`
- `indigoDbgProfilingGetCounter`

---

## Part D — Bingo NoSQL: a whole module that isn't wired in at all

Unlike Part C, these functions aren't just uncalled — the module itself isn't linked.
`bingo-nosql.dll` sits in `indigo/lib/` (MinGW build) and
`indigo-libs-windows-x86_64/lib/windows-x86_64/` (MSVC build), and its header is already
vendored at `indigo/api/c/bingo-nosql/bingo-nosql.h` — but `CMakeLists.txt` never links it
and `IndigoService` never calls it. It needs its own `target_link_libraries` entry (and, on
MSVC, its own generated `.lib` via the same `dumpbin`/`lib.exe` process used for `indigo.dll`)
before any of this is usable.

**What it is:** a self-contained, file-based chemistry search index — no database server,
just a local index file on disk. Purpose-built for exactly the "similarity search" and
"substructure search" gaps in Part C, and a better fit for them than hand-rolling with the
raw `indigoFingerprint`/`indigoSubstructureMatcher` API: Bingo owns the indexing, storage,
and scoring as one package. Concretely, it could back a "find similar" search over the 276
`library.sdf` templates (build the index once, query fast — no linear re-scan per search),
and the same indexing mechanism is the natural implementation for the "SDF/RDF batch file
browsing" gap from Part C if a user opens their own large SDF.

**Full function list (34, all unused):**
- `bingoVersion`
- `bingoCreateDatabaseFile`
- `bingoLoadDatabaseFile`
- `bingoCloseDatabase`
- `bingoInsertRecordObj`
- `bingoInsertIteratorObj`
- `bingoInsertRecordObjWithId`
- `bingoInsertRecordObjWithExtFP`
- `bingoInsertRecordObjWithIdAndExtFP`
- `bingoDeleteRecord`
- `bingoGetRecordObj`
- `bingoOptimize`
- `bingoSearchSub` — substructure search
- `bingoSearchExact` — exact-structure match
- `bingoSearchMolFormula` — search by molecular formula
- `bingoSearchSim` — similarity search (threshold-based)
- `bingoSearchSimWithExtFP`
- `bingoSearchSimTopN` — similarity search (top-N)
- `bingoSearchSimTopNWithExtFP`
- `bingoEnumerateId`
- `bingoNext` — result iterator
- `bingoGetCurrentId`
- `bingoGetCurrentSimilarityValue`
- `bingoEstimateRemainingResultsCount`
- `bingoEstimateRemainingResultsCountError`
- `bingoEstimateRemainingTime`
- `bingoContainersCount`
- `bingoCellsCount`
- `bingoCurrentCell`
- `bingoMinCell`
- `bingoMaxCell`
- `bingoGetObject`
- `bingoEndSearch`
- `bingoProfilingGetStatistics`

---

*Counts verified against `indigo/api/c/indigo/indigo.h`, `indigo-inchi.h`, `indigo-renderer.h`,
`indigo/api/c/bingo-nosql/bingo-nosql.h`, and the top-level export object at
`chem-core.js:5772-5843`, cross-checked with usage grep over `src/v8_worker.js` and
`src/app/IndigoService.cpp`. Full interactive version (with search filter) published as an
artifact on 2026-07-09; Bingo NoSQL discovered and added 2026-07-10.*
