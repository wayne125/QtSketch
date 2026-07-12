# Architecture Map — Sketch

Where the next features actually plug in: the current three-layer hybrid, and the two
structurally separate integration seams every item in `Features To Be Implemented.md`
falls into.

---

## The current architecture

### QML UI · Qt Quick

- **MainWindow.qml** — shell: tabs, toolbars, menus, dialogs, and the single shared
  `IndigoService` instance
- **ChemCanvas.qml** — one per open document; owns tool state, zoom/pan, keyboard shortcuts
- **Rendering layers** — `MoleculeLayer.qml`, `SelectionLayer.qml`, `LabelLayer.qml`,
  `ToolOverlay.qml`
- **ToolPanel.qml / PropertyPanel.qml** — left tool grid, right property editor
- **components/** — `IconCell`, `AnchoredPicker`, `TaskDialog`, `SegmentedControl`, etc.
- **Theme.qml / StyleSheets.qml** — design-token singletons

↓↑ Q_INVOKABLE calls · property bindings · signals (to V8Process)
↓↑ Q_INVOKABLE calls · `*Finished` signals (to IndigoService)

### C++ Host · Qt6

- **DocumentManager** — `QML_SINGLETON`; owns `docId → V8Process*`
- **V8Process** — one per document; the `Sketch` bridge (~90 `Q_INVOKABLE` methods). Owns one
  in-process `QjsEngine` (a QuickJS-ng `JSRuntime`+`JSContext` pair) per document — **no child
  process, no Node.js dependency at all** (this replaced the original Node.js/`QProcess`
  transport; see "Engine migration" below)
- **AppController / PlacementEngines / PlacementPreviewManager** — drag-placement preview,
  synchronous, no IPC
- **IndigoService** — `QML_ELEMENT`, a single instance for the whole window, always acts on
  `activeCanvas`. Every heavy call runs via `QtConcurrent::run` on the thread pool with its
  own `indigoAllocSessionId()`

↓↑ direct JS function calls, in-process, no IPC (V8Process → QjsEngine → `_dispatchCommand`,
one JSContext per open tab)
↓↑ direct C calls, in-process (IndigoService ↔ indigo.dll)

### Runtime layer

- **src/v8_worker.js** — one `JSContext` per document; holds the live `_struct`, dispatches
  `(cmd, args)` via an if/else chain (`_dispatchCommand`, `src/v8_worker.js`), wraps every
  mutation in `makeCmd` for undo/redo, then calls `buildRenderPrimitives`. Node-specific globals
  (`fs`/`path`/`readline`/`process`/`console`) are shimmed by `QjsEngine` under `__native`; a
  `_hasNative` check keeps a Node.js fallback path working for `scripts/worker_smoke_test.js`
  and local dev, but the shipped app never spawns Node at all
- **chem-core.js** — `eval()`'d once per `JSContext`; 72 exports, 22 used (see the coverage audit)
- **quickjs-ng (`qjs` library)** — the in-process JS engine itself. Vendored as plain C source
  in `third_party/quickjs-ng`, compiled directly into the `sketch` binary via `add_subdirectory`
  — no prebuilt-binary/toolchain mismatch to fight (unlike Indigo), since CMake just compiles
  it fresh under whichever compiler is active. Bridge code: `src/qjs_engine.h`/`.cpp`
- **indigo.dll** — 517 C functions, 51 used (see the coverage audit). Linked only into the
  C++ host — the worker's JS code never touches it
- **bingo-nosql.dll** — a separate, self-contained chemistry search index (substructure/
  similarity/exact/formula search, file-based, no DB server). Already vendored (DLL + header
  at `indigo/api/c/bingo-nosql/bingo-nosql.h`) but **not linked into the build at all** — no
  `CMakeLists.txt` entry, no service wrapping it yet

**The re-entry loop:** when `IndigoService` finishes a job (e.g. `layoutFinished`),
`MainWindow.qml`'s `Connections` block calls `activeCanvas.loadMolfile(...)` — which
re-enters the V8Process → worker path above. Indigo never holds state; every result gets
funneled back into the worker's `_struct` as the one source of truth.

### Engine migration (2026-07-10)

The worker transport was originally a spawned Node.js child process talking newline-delimited
JSON over stdin/stdout (`QProcess`). That's been fully replaced with `QjsEngine`
(`src/qjs_engine.h`/`.cpp`), an in-process QuickJS-ng `JSRuntime`/`JSContext` per document —
`V8Process::sendCommand`'s public shape is unchanged, only its internals differ (direct
`JS_Call` instead of a pipe write). This removed the "Node.js must be on PATH or bundled next
to the binary" deployment requirement entirely. V8 was evaluated and passed over: MSVC (needed
for V8 anyway) is now available, but V8's build system (`depot_tools`/GN/Ninja, no CMake
integration, a 10-20GB checkout, hours-long builds) was judged not worth it since this app's
actual bottleneck was IPC overhead, not JS execution speed — QuickJS-ng removes the IPC hop
for a fraction of the setup cost.

**Argument marshaling gotcha (found post-migration):** `QjsEngine::variantToJs`
(`src/qjs_engine.cpp`) converts each `QVariant` in `sendCommand`'s `args` list into a real JS
value for `_dispatchCommand`. It handles scalars (bool/int/double/string) directly, and
**`QVariantList`/`QVariantMap` recursively into real JS arrays/objects** — the latter case
was originally missing, which silently broke every `TEMPLATE_*` ring tool (`addRing`'s
`coords` argument is a `QVariantList` wrapped inside the args list) while scalar-only
commands kept working fine. `scripts/worker_smoke_test.js` never would have caught this: it
drives the worker via the Node fallback path, never through `QjsEngine` at all. Any new
`Q_INVOKABLE` that takes a `QVariantList`/`QVariantMap` argument (not just scalars) is safe
now, but a manual in-app test is the only real verification for this seam — there's no
automated test exercising the QjsEngine transport itself yet.

**Verification pass (2026-07-10):** a systematic bug-hunt across the whole project after the
migration found and fixed 9 issues — a deferred-error-signal bug in `V8Process`, a
cross-thread `QPointer` race across all 20 `QtConcurrent` call sites in `IndigoService.cpp`
(see the Indigo seam's step 1 below), two undo/redo asymmetries in `v8_worker.js`'s
sgroup/functional-group handling (`pasteSelection`, `deleteAtomById`/`deleteSelection`), and
four QML bugs including a **recurring pattern** worth remembering: several call sites forgot
to resolve a contracted sgroup pill id to a real atom id before touching the backend (the
established fix is `ChemCanvas.qml`'s own `resolveHitAtom()` helper — check any new
atom-targeting code path against it).

### Canvas interaction tools (2026-07-11)

Added a `HAND` (pan) tool, a `SELECT_LASSO` tool, a hard page/canvas boundary, and (same day,
as a follow-up) PowerPoint-style selection handles that **replaced** an initial standalone
`ROTATE` tool — full design and research in
`C:\Users\jp18b\.claude\plans\calm-launching-kettle.md`; implementation summary in
`Features To Be Implemented.md`'s "Done (2026-07-11)" entries. Notable patterns for future tools
built the same way:

- **Drag-tool state lives on `ChemCanvas.qml`'s `MouseArea`**, not `overlayState` — rotate/resize
  added `rotatingSelection`/`rotateCenterX/Y`/`rotateLastAngle`/`rotateRawTotal`/
  `rotateAppliedTotal` plus `resizingSelection`/`resizeAnchorCanvasX/Y`/`resizeAnchorChemX/Y`/
  `resizeOrigDist` alongside the existing `movingSelection`/`isDragging`, since this state is
  purely local UI-drag bookkeeping, never needed by the backend or other layers. `SELECT_LASSO`'s
  in-progress path, by contrast, went into `overlayState.lassoPath` (like the existing
  `dragRect`/`bondPreview`) since `ToolOverlay.qml` needs to read it to paint the live outline.
- **No separate tool for rotate/resize — handles appear directly on the selection**, matching
  PowerPoint rather than ChemDraw's own separate-tool convention (an earlier `ROTATE` tool was
  built first, then explicitly replaced the same day per user direction). `ChemCanvas.qml`'s
  `selectionBBoxCanvas()`/`selectionHandles()` compute the bbox + 9 handle positions (4 corner +
  4 side resize, 1 rotate) whenever `SELECT`/`SELECT_FRAGMENT` has a qualifying selection (≥2
  points, generalized across every selectable type — see below); `ToolOverlay.qml` renders them
  and `onPressed` hit-tests them **before** the normal atom/bond hit-test, since handles sit on
  top and take priority.
- **Rotate/resize generalized to every selectable type, not just atoms**: `_snapshotSelectionPoints()`/
  `_writeSelectionPoint()` (`v8_worker.js`) are shared helpers that flatten the four selection
  id-lists (atoms, rxnArrows, rxnPluses, multitailArrows) into a tagged point list and write
  transformed positions back by type — mirroring `moveSelection`'s own multi-type handling. Both
  `rotateSelectionLive`/`commitRotate` and the new `scaleSelectionLive`/`commitScale` build on
  this, so a mixed selection (a molecule fragment + a reaction arrow + a text label) rotates/
  resizes together, and a single reaction arrow (2 points) qualifies on its own.
- **Resize scales uniformly about a fixed anchor point (the handle's geometric opposite), not the
  centroid** — deliberately different from rotate's centroid-based pivot. Dragging a corner keeps
  the opposite corner fixed, matching real corner-drag resize behavior; uniform-only (no
  independent width/height stretch) since anisotropic scale would distort bond lengths into
  chemically meaningless values — even the 4 side handles do the same uniform scale, just
  anchored at their own opposite edge.
- **Live-preview + separate commit, now used by rotate, resize, and move**:
  `rotateSelectionLive`/`commitRotate` and `scaleSelectionLive`/`commitScale` in `v8_worker.js`
  are structurally identical to `moveSelection`/`commitMove` — non-undoable repeated calls during
  the drag (both rotate and scale re-apply from a snapshot taken at drag-start to avoid drift,
  rather than compounding), one undoable command on release. Any future continuous-drag transform
  should follow this same shape.
- **Lasso selection reuses `selectByRect`'s structure** (`v8_worker.js`) but is **not** a drop-in
  copy: ChemDraw's lasso only selects objects **entirely enclosed** by the path (an atom inside
  the polygon; a bond only if *both* atoms are), whereas `selectByRect`'s rectangle marquee also
  selects bonds/arrows that merely cross the rect edge (`lineIntersectsRect`). Don't assume the
  two selection tools share a membership rule — confirmed via research this is a real ChemDraw
  distinction, not an oversight to "fix" into consistency.
- **Page boundary is a hard clamp, by explicit user decision, not ChemDraw parity** — ChemDraw's
  own page lines are a soft print/layout guide with no movement constraint; this app's boundary
  (`PAGE_MIN_X/MAX_X/MIN_Y/MAX_Y` in `v8_worker.js`, `±30`/`±21` chemical units) actually stops
  content. Clamping is applied per-call-site (`addAtom`, `addBondAndAtom`,
  `addBondBetweenCoords`, `addChain`, `insertFunctionalGroup`, `pasteSelection`) rather than
  inside `chem-core.js`'s primitives, and deliberately **skipped** for fusion-sensitive geometry
  (`addRing`'s per-vertex coords, `insertLibraryTemplateFused`'s bond-snap path) where clamping
  could misalign a ring/template seam from the existing structure it's fusing onto — only the
  *anchor point* feeding those is clamped. `moveSelection` clamps the whole selection's
  bbox-delta together (so a multi-atom drag stops as one block, not atom-by-atom);
  `rotateSelectionLive`/`scaleSelectionLive` freeze before any point would exit the page rather
  than solving for an exact boundary angle/factor.
- **CMakeLists.txt's icon resource list is an explicit file list, not a glob** — a new icon file
  added to `icons/` on disk (e.g. `hand.svg`, `select-lasso.svg` for the tools above) is silently
  **not** packaged into the app until also added to the `qt_add_qml_module`/resource list in
  `CMakeLists.txt`, and requires a full CMake reconfigure (not just a rebuild) to pick up. Missed
  on first pass here; only surfaced once a real interactive launch captured QML stderr (see next
  point) — worth checking whenever a new icon is referenced from QML.
- **Bash-launched GUI verification needs the app's runtime DLLs actually deployed, not just Qt's
  `bin/` on `PATH`** — earlier passes this session "verified" a clean launch via Git Bash with
  `PATH="/c/Qt/.../mingw_64/bin:$PATH"`, but that POSIX-style path prefix doesn't reliably reach
  the native Windows loader's DLL search through MSYS's `CreateProcess` call, and produced a
  silent `STATUS_DLL_NOT_FOUND` crash with **zero output** (not even a caught error) for this
  pass's launch checks — a false-negative that could just as easily have been a false-positive
  reported success in earlier passes. Found via PowerShell's `Start-Process` (its exit code,
  `-1073741511` = `0xC0000135`, is unambiguous), fixed by running the Qt-provided
  `windeployqt.exe --qmldir <src>` against `sketch.exe` once per build tree (copies every runtime
  DLL/QML plugin next to the exe) plus copying the matching `libstdc++-6.dll`/
  `libgcc_s_seh-1.dll`/`libwinpthread-1.dll` from the *same* MinGW toolchain CMake actually
  compiled with (`C:\mingw64`, per `CMAKE_CXX_COMPILER` in `CMakeCache.txt` — not a different
  vendored MinGW, which can silently mismatch the runtime ABI). PowerShell + `windeployqt` is now
  the reliable path for launch verification from this environment; don't trust a quiet Bash
  launch as proof of a clean run without checking `Start-Process`'s exit code too.
- Verified via a temporary Node harness (deleted after use, same pattern as the ring-fusion
  feature's `test_fusion_TEMP.js`) plus the standing `scripts/worker_smoke_test.js`, a clean
  MinGW build, **and this time a real interactive-launch check with captured stderr** (via
  PowerShell + `windeployqt`, see above) showing zero QML errors beyond the pre-existing benign
  "native style customization" warnings. **Manual mouse-driven interaction (does the handle feel
  right to grab, drag smoothness) and the MSVC build were still not verified this pass** — pending
  manual check, same "ask the user to rebuild via Qt Creator" pattern noted in the
  engine-migration section above (this Bash shell lacks the MSVC `INCLUDE`/`LIB` environment).

### InChI / InChIKey support (2026-07-11)

Linked the previously-vendored-but-unlinked `indigo-inchi` plugin — third confirmation of the
Indigo-plugin linking pattern (after `indigo.dll` itself; `indigo-renderer` linked next, for
native SVG export, see below; Bingo still unlinked). Full design: `calm-launching-kettle.md`;
implementation summary: `Features To Be Implemented.md`'s "Done (2026-07-11, InChI)" entry.

- **Zero `src/v8_worker.js` changes** — confirms `V8Process::requestSerialize(reqId)`
  (`v8_process.cpp:84`) is a pure passthrough: it's always `requestStructure("mol", reqId)`, so
  any new clipboard/export action that just needs the current molfile (as SMILES/InChI/anything
  else already does) is purely an `IndigoService` + `MainWindow.qml` addition — check this seam
  before assuming a new "compute X from the current structure" feature needs worker changes.
- **The MSVC `.lib` generation step for a vendored plugin DLL is a standalone tool invocation,
  not a full build** — `dumpbin.exe`/`lib.exe` (found under
  `C:\Program Files (x86)\Microsoft Visual Studio\...\VC\Tools\MSVC\<ver>\bin\Hostx64\x64\`)
  don't need the MSVC `INCLUDE`/`LIB` compiler environment this Bash shell otherwise lacks —
  only an actual `.cpp` compile does. `indigo-inchi.lib` was generated and its symbols verified
  present (`dumpbin /linkermember:1`) directly from this shell, same process as `indigo.lib`:
  `dumpbin /exports indigo-inchi.dll` → extract the name column → `LIBRARY indigo-inchi` +
  `EXPORTS` + one name per line → `lib.exe /def: /machine:x64 /out:`.
- **Found and fixed a real bug via direct chemistry-level verification, not just a clean
  build**: `indigoInchiGetInchi()`'s return value is a pointer into a buffer
  `indigoInchiGetInchiKey()` itself overwrites while computing its own result — passing the
  first call's raw pointer straight into the second as its argument reads already-clobbered
  memory and silently returns null. Confirmed with a standalone repro (a tiny `.cpp` compiled
  directly against the vendored DLLs, bypassing Qt/the GUI entirely) before touching the fix,
  then again with the exact `QString`-copy pattern used in the real fix, both against benzene's
  known reference values (`InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H`,
  `UHOVQNZJYSORNB-UHFFFAOYSA-N`). **General lesson for any Indigo API returning `const char*`**:
  treat the pointer as valid only until the *next* Indigo call of any kind, on any function —
  copy it to an owned string immediately if it needs to survive past that, don't just check for
  null and assume the pointer stays good.
- Verified via the standing `scripts/worker_smoke_test.js` (unaffected, confirming the "zero
  worker changes" claim above), a clean MinGW build, and a real interactive launch (PowerShell +
  `windeployqt`, per the pattern established in the previous section) with no new QML errors.
  MSVC compile/link itself still needs the user's Qt Creator rebuild to fully confirm.

### "Load from InChI" (2026-07-11) — and why not InChIKey

Follow-up prompted by a request to "load molecules from InChIKey" — **confirmed with the user
before building anything** that InChIKey is a one-way hash with no reverse algorithm, so the
request actually meant one of two different things: load from the full InChI string (locally
invertible), or resolve an InChIKey via an external database like PubChem (network-dependent,
never done in this app). Built the first; the second remains a real, distinct, unbuilt idea if
ever wanted — it would need a new networked service layer, not an `IndigoService` extension.

- **Key discovery, worth remembering**: Indigo's *generic* structure loader
  (`indigoLoadMoleculeFromString`, the same one "Load from SMILES" already calls via
  `IndigoService::layout()`) auto-detects and correctly parses InChI strings **natively** —
  confirmed via a standalone test that this works even without `indigo-inchi.dll` linked or
  `indigoInchiInit` called at all. The `indigo-inchi` plugin (linked in the previous section) is
  only needed for the opposite direction — *generating* an InChI/InChIKey from a structure.
  Loading FROM an InChI needed **zero new backend code** — purely `MainWindow.qml`: a new "Load
  from InChI" toolbar entry + `TaskDialog`, mirroring `smilesDialog` exactly, calling the exact
  same `indigoSvc.layout(text)`.
- Added a live inline warning (regex-matches the InChIKey shape, `^[A-Z]{14}-[A-Z]{10}-[A-Z]$`)
  in the new dialog, directly targeting the mix-up that prompted this whole feature — shown as
  the user types, doesn't block submission (consistent with "Load from SMILES" also not
  validating input before calling `layout()`).
- Verified via a standalone `.cpp` repro (bypassing Qt/the GUI, compiled directly against the
  vendored DLLs) confirming the generic-loader behavior above, the standing smoke test
  (unaffected — no worker changes), a clean MinGW build, and a real interactive launch with no
  new QML errors.

### Reaction structure validation (2026-07-11)

`IndigoService::checkStructure` had an explicit reaction guard returning a placeholder message
instead of ever checking anything — the "Small, concrete gaps" entry in
`Features To Be Implemented.md`, now resolved there too.

- **`indigoCheckObj` on a whole reaction handle isn't usable the same way as on a molecule** —
  confirmed via a standalone test before writing any real code: it returns an empty-string key
  with no component association (`{"": "Reaction component check result, ...(id)"}`), which the
  existing `atomChecks`/regex-based JSON parser (written for `{"checkType": "...(ids)"}` pairs)
  can't classify. **Fix**: iterate `indigoIterateReactants`/`indigoIterateProducts` and run
  `indigoCheckObj` on each component individually — confirmed this gives back the exact same
  clean per-type shape the molecule branch already parses, just with atom/bond indices local to
  that one component. The shared parsing logic was factored into a small file-local
  `parseCheckReport()` function so both branches (molecule, and each reaction component) call
  the same code instead of duplicating the regex/`atomChecks` block.
- **Deliberate scope boundary, decided during planning rather than discovered as a limitation
  later**: this only produces a **text report** — the inline badge/red-bond highlighting path
  (`checkIssuesReady`/`setCheckIssues`, `v8_worker.js:1800`) stays off for reactions. Those
  per-component-local ids would need to be re-offset against however
  `CoreLib.ChemCore.MolSerializer` actually orders reactant/product fragments' atoms when it
  serializes a `$RXN` string from `_struct` — third-party bundled/minified code not safely
  verifiable without disproportionate effort, and a wrong mapping would silently highlight the
  *wrong* atom, which is worse than the honest "not yet supported" stub it replaces. If inline
  reaction highlighting is ever wanted, that fragment-ordering question is the thing to nail
  down first, not an incremental extension of this pass's approach.
- Verified via a standalone `.cpp` test (bypassing Qt/the GUI, linked directly against the
  vendored DLL) against both a broken reaction (product with a genuine valence error) and a
  clean one — confirmed the per-component report correctly names the specific component and
  reports "No problems found." when none exist. Standing smoke test unaffected, clean MinGW
  build, real interactive launch with no new QML errors.

### loadMolfile data loss on the Indigo round-trip (2026-07-11)

Started from "how do we minimize worker↔Indigo IPC overhead" — investigating that round-trip
found the actual overhead is negligible (MDL molfile parsing at this app's structure sizes is
microsecond-scale, dwarfed by the `QtConcurrent` thread-hop and debounce timers already in
place) but surfaced a real correctness bug instead. Full design/investigation:
`calm-launching-kettle.md`; implementation summary: `Features To Be Implemented.md`'s "Done
(2026-07-11, loadMolfile data loss)" entry.

- **MDL molfile/RXN — what every one of Layout/Aromatize/Dearomatize/Normalize/Standardize
  round-trips through — has no concept of Ketcher's own text annotations or multitail arrows.**
  `loadMolfile()` (`v8_worker.js:3177`) did a full `_struct = loaded` replace with whatever came
  back, so both were silently deleted on every one of those five actions. Reaction arrows/plus
  signs were fine (`$RXN` is a real MDL concept). Confirmed via direct calls to
  `getStructure("mol", ...)` before writing any fix, same discipline as every other bug found
  this session.
- **Fix**: merge the outgoing struct's `texts`/`multitailArrows` pools into the freshly-
  deserialized one inside `loadMolfile()` itself, rather than at each of the five call sites —
  safe specifically because `loadMolfile()` is *only* ever called for "reload a modified version
  of the same document" (those five ops, plus biopolymer expansion); genuine file **open** goes
  through the separate `loadStructure()` path (traced via `MainWindow.qml`'s `loadFromFile()`),
  so this can't leak stale annotations into an unrelated newly-opened file. One fix, every caller
  benefits.
- **Found in passing, fixed as a same-day follow-up** (see the next section): `getStructure("ket", ...)`
  threw/returned empty specifically when a multitail arrow was present, even though KET format
  otherwise correctly preserves both texts and multitail arrows.
- **Pattern worth repeating**: when a "let's optimize X" question comes up, testing the actual
  behavior directly (a disposable Node harness against the real worker, same pattern as every
  other verification this session) before assuming the premise is worth more than reasoning
  about it in the abstract — the real finding here was orthogonal to the original question.
- Verified via a disposable Node harness simulating the round-trip directly against the real
  worker (text and multitail arrow both survive a `loadMol` call that replaces the atom),
  standing smoke test unaffected, clean MinGW build, real interactive launch with no new QML
  errors. No C++/CMake changes this pass.

### KET multitail-arrow crash (2026-07-11)

Same-day follow-up to the gap found above.

- **Root-caused, not guessed**: patched a *disposable copy* of `src/v8_worker.js` (in `/tmp`,
  the tracked file was never touched) so `getStructure`'s catch block would stop silently
  swallowing the exception, then reran the exact repro that found the symptom. Real error:
  `TypeError: multitailArrow.toKetNode is not a function`, thrown **inside**
  `KetSerializer.serializeMicromolecules` (chem-core.js, vendored/minified) — it walks
  `struct.multitailArrows` directly and calls `.toKetNode()` on each entry, assuming a real
  chem-core `MultitailArrow` class instance. This app's own multitail arrows
  (`_makeMultitailArrow`, `v8_worker.js:3942`) are plain data objects with no such method — the
  exact situation `Features To Be Implemented.md`'s Part B audit already flagged ("Sketch
  reimplemented this as plain objects"). The crash happens *before* `_injectMultitailArrows`
  (which correctly builds KET nodes from the plain-object data — never the actual bug) gets a
  chance to run.
- **Second call site with the identical problem, found while tracing this**:
  `getClipboardAsKet()` (`v8_worker.js:1571`) calls the same `serializeMicromolecules` and would
  crash the same way; it also never called `_injectMultitailArrows` at all, a separate smaller
  gap (a copied multitail arrow would stay silently missing from clipboard-as-KET output even
  once the crash was fixed).
- **Fix**: a new `_serializeMicromoleculesSafe(struct)` helper — mirroring the existing
  `_reattachMultitailArrows`'s established pattern (`v8_worker.js:3159`) of swapping in
  `new CoreLib.ChemCore.Pool()` for a struct-shape mismatch — temporarily empties
  `struct.multitailArrows` for just the `serializeMicromolecules` call, restoring it right after
  in a `finally`. Safe because both call sites already re-inject the correct multitail-arrow
  nodes afterward from the *real* pool data. Vendored chem-core.js was not touched, consistent
  with the standing "leave it" rule from earlier this session about editing vendored source.
- **Pattern worth repeating**: when a caught exception is suspiciously silent (`catch (e) {
  return "" }`), patch a *disposable copy* of the file to unmask it rather than guessing at the
  cause from the symptom alone — this is the second bug this session (after the InChIKey
  buffer-aliasing bug) found by refusing to trust "it fails" without seeing the actual exception.
- Verified via the exact repro that found the bug: both `getStructure("ket", ...)` and
  `getClipboardAsKet()` now return non-empty KET JSON containing the multitail-arrow node.
  Standing smoke test unaffected, clean MinGW build, real interactive launch with no new QML
  errors. No C++/CMake changes.

### Toolbar alignment icons (2026-07-11) — first delegated-to-opencode implementation

Section 1 of `docs/superpowers/specs/2026-07-08-toolbar-rulers-design.md`. First feature this
session where Claude planned but did **not** directly implement — delegated to `opencode` via
the `claude-opencode:opencode-implement` skill, then verified the result independently. Plan:
`.claude/plans/toolbar-alignment-icons.md`.

- **Planning still required real investigation before handoff, same rigor as every self-
  implemented feature this session** — no dedicated align/distribute icons existed anywhere in
  `icons/`, and the vendored `fonts/materialdesignicons-webfont.ttf` was completely unused (no
  `FontLoader`, no codepoint mapping). Confirmed the MDI-font route with the user, then
  extracted the *actual* codepoints from the font binary via Python's `fontTools`
  (`getBestCmap()`) rather than guessing from memory — MDI codepoints are version-dependent and
  wrong ones would render as blank tofu boxes with no build-time signal.
- **Found and specified the fix for a real `IconCell.qml` bug before delegating, not left for
  opencode to discover**: its glyph-mode sizing (`glyph.length > 2 ? 12 : 18`) would silently
  misfire for any codepoint above U+FFFF (a 2-code-unit UTF-16 surrogate pair in a JS/QML
  string), landing every one of these icons in the wrong font at the wrong size. New
  `glyphFontFamily`/`glyphIsIcon` properties, defaulting to old behavior, fix this without
  touching any of the other 7 existing glyph-mode call sites in the app.
- **Delegation mechanics**: preflight (`opencode --version`, `opencode auth list`) confirmed
  `opencode` installed and authed (OpenCode Go + OpenAI); plan approved via `AskUserQuestion`
  before any model/permission questions (per the skill's phase ordering); model
  `opencode-go/glm-5.2` and the `build` agent (full permissions) chosen by the user. The first
  `opencode run --agent build ...` invocation was blocked by Claude Code's own auto-mode safety
  classifier (flagged as granting a third-party CLI unrestricted write/shell access from an
  abstract permission-tier answer, not explicit confirmation) — not worked around; the user
  explicitly asked for a retry, which then ran to completion.
- **Verification after opencode returned did not stop at reading its own report** — read the
  actual file diffs directly (matched the plan exactly, zero deviations), grepped for the other
  7 `glyph:` call sites to confirm none were touched, and — the one item opencode's own report
  flagged it *couldn't* verify (no display capture in its environment) — launched the real app
  and took an actual screenshot (via `System.Drawing`/Win32 `PrintWindow`, cropped and 4x-scaled
  for clarity), confirming all 6 icons render as correct, distinct pictograms matching their
  intended action, not tofu/wrong-font fallback. This is the load-bearing lesson from this
  delegation: a sub-agent's "I verified X" claim is only as good as what its environment can
  actually observe — check what it says it *couldn't* check.

### Native SVG export (2026-07-12)

Linked the previously-vendored-but-unlinked `indigo-renderer` plugin — fourth confirmation of
the Indigo-plugin linking pattern. Purely additive: does not touch the existing screenshot-based
PNG/PDF export (`AppController::exportPdf`/canvas `exportPNG`). New
`IndigoService::renderToFile(molfile, fileUrl, format)` + `renderFinished(bool, error)` signal,
following the `inchi()`/`emitOnGuiThread` shape exactly; `MainWindow.qml` gained a
`pendingRenderUrl` property, a `render_svg` `reqId` branch, and a `*.svg` entry in the save
dialog. See `Features To Be Implemented.md`'s "Done (2026-07-12, native SVG export)" entry and
`.claude/plans/indigo-native-svg-export.md`.

- **Started as an `opencode` delegation that stalled partway through.** The process (model
  `opencode-go/glm-5.2`) finished `CMakeLists.txt`'s linking changes and the bare `Q_INVOKABLE`
  declaration in `IndigoService.h`, then produced no new output for several checks in a row
  while still alive (confirmed via `tasklist`, not guessed). Killed it and hand-finished the
  rest against the same plan: the missing `<QUrl>` include and `renderFinished` signal, the
  `.cpp` implementation, and all of the `MainWindow.qml` wiring.
- Verified via a standalone harness — a small `.cpp` calling the *actual compiled*
  `IndigoService::renderToFile`, moc'd and linked directly against the vendored DLLs, bypassing
  the GUI — rendering a benzene molfile to SVG and confirming the output was a real vector
  `<svg>` document (9 `<path>` elements), not a rasterized blob or an empty/error file. Clean
  MinGW build; real interactive launch showed no new QML errors beyond pre-existing style
  warnings; `.mol`/`.sdf`/`.ket`/`.png` save paths inspected and confirmed unaffected.

### SDF multi-record batch browsing (2026-07-12)

Opening a multi-record `.sdf` previously silently kept only the first record
(`_deserializeStruct`'s `sdf` branch returned `items[0].struct`, discarding the rest with no
warning — `v8_worker.js:3026-3028`). Fixed by reusing the Library/template popup's exact
pattern (`AnchoredPicker`+`ThumbnailGridItem`+`GridView`) — that popup is itself already "parse
a multi-record SDF, thumbnail every record, click to pick," just pointed at
`templates/library.sdf` instead of a user-opened file. New worker commands
`deserializeSdfBatch(data)`/`loadSdfBatchRecord(index)`, new standalone `SdfRecordPicker.qml`.
Full plan: `.claude/plans/sdf-batch-browser.md`; implementation summary: `Features To Be
Implemented.md`'s "Done (2026-07-12, SDF batch record browsing)" entry.

- **Built entirely via seam ①** (the worker's own `SdfSerializer`), not seam ② as the roadmap
  had originally scoped it — no new `IndigoService`/Indigo C API surface was needed, since the
  worker already had everything required.
- **Zero regression for the common case, by design**: a single-record `.sdf` still loads
  directly with no popup (`MainWindow.qml` only opens the picker when the worker reports
  `count >= 2`). Eager thumbnail computation is capped at 500 records; no general pagination
  system was built (explicit non-goal). `.rdf` remains completely unsupported.
- **First feature this session delegated to the Antigravity CLI (`agy`, Gemini 3.1 Pro High,
  `--mode accept-edits`) rather than `opencode`.** First launch attempt was lost to a shell
  double-backgrounding mistake (a trailing `&` inside an already-backgrounded call detached the
  process from its own wrapper's stdio, so the wrapper reported a false timeout while an
  orphaned `agy` process kept running unmanaged with no workspace writes) — killed the stray
  process and relaunched cleanly via a stdin-piped prompt; the second attempt completed cleanly.
- Verified via two standalone Node harnesses spawning `src/v8_worker.js` over its real
  stdin/stdout protocol (the same technique `scripts/worker_smoke_test.js` uses, no GUI needed):
  one confirmed `deserializeSdfBatch` against the real `templates/library.sdf` returns the true
  record count (276, independently cross-checked by counting `$$$$` separators in the raw file)
  with correct titles/thumbnails, and that a genuine single-record file reports `count === 1`;
  a second confirmed `loadSdfBatchRecord(5)` loads the exact picked record (atom/bond counts
  matching that record read directly from the raw file). Standing smoke test unaffected; clean
  MinGW build; real interactive launch with no new QML errors.
- **Re-verified on request, fresh (not recalled from the original pass)**: reran a new Node
  harness against the current code and the real `templates/library.sdf` — worker-reported count
  still exactly matches the raw file's `$$$$`-delimited record count (276/276), and
  `loadSdfBatchRecord(10)`'s loaded structure title (`alpha-D-Lyxopyranose`) matches that record's
  picker-list label exactly. Code unchanged since the original implementation; standing smoke
  test still passes; app still launches cleanly. **Attempted to close the standing click-through
  gap this round and could not**: tried driving the real UI (`Ctrl+O` → native file dialog → type
  path → `SdfRecordPicker` should open) via `SetForegroundWindow`, `ShowWindow`, and
  `WScript.Shell.AppActivate` — all three failed to bring the app window into focus in this
  environment, confirmed by a screenshot showing the keystrokes never reached the app (canvas
  stayed on the empty untitled doc). This is an environment constraint (no window-focus control
  for another process' window), not a code issue — backend correctness is solid; the actual
  mouse-driven open → picker → click flow remains an honest, stated verification gap, same class
  as the earlier image-file-picker gap.

### Image embedding on canvas (2026-07-12)

`struct.images` was already a native, initialized Pool in vendored `chem-core.js` with a working
KET `Image` class (`toKetNode`/`fromKetNode`) — opening a `.ket` file with an embedded image
already silently worked; nothing could create one, nothing rendered one. New worker commands
`addImage`/`deleteImage` (a duck-typed `_makeImage` object mirroring chem-core's real, unexported
`Image` class — same situation as `Text`/`MultitailArrow`); new `FileIO::readImageAsDataUri`;
new `IMAGE` tool (using the previously-unused `icons/add-image.svg`) + `FileDialog` +
`LabelLayer.qml` rendering block (a hidden `Image {}` `Repeater` decodes each base64 data URI,
then `ctx.drawImage()`s it once ready). Full plan: `.claude/plans/image-embedding.md`.

- **v1 deliberately bounded**: insert/delete only (no move/resize — matching text annotations'
  own existing lack of drag support), file-picker only (no clipboard-paste), KET-only round-trip.
  `loadMolfile()`'s internal MDL round-trip carries `images` across unchanged — the same bug
  class fixed for texts/multitailArrows earlier this session, deliberately not repeated here.
- **Second feature delegated to the Antigravity CLI** (`agy`, Gemini 3.1 Pro High,
  `--mode accept-edits`). The plan flagged the bounding-box/Y-inversion math as highest-risk and
  required reading the real `Image` class before writing the duck-type. **Verified
  independently, not just trusted**: read `getTopLeftPosition()` (`center.sub(halfSize)`) and
  `getNodeWithInvertedYCoord`'s customizer (negates `.y` only) directly in `chem-core.js`, and
  confirmed the delegated `toKetNode()` reproduces that exact formula — an initial
  hand-derivation suspecting a sign error turned out to rest on a wrong coordinate-convention
  assumption, corrected by reading the real source rather than trusting either the digest or the
  unverified reasoning.
- Verified via two standalone Node harnesses spawning `src/v8_worker.js` over its real
  stdin/stdout protocol: one confirmed `addImage` → `getStructure("ket",...)` produces a
  correctly-shaped node (base64 `data` exactly matching input, `boundingBox` matching the
  hand-computed expected value), `undo` removes it, and that `imageToKet`'s real re-serialize
  step genuinely drops the `center` field (confirmed by reading `chem-core.js:21073-21081` —
  expected chem-core behavior, not a bug); a second confirmed the image survives `loadMolfile`'s
  internal round-trip (via `aromatize`). Standing smoke test unaffected; clean MinGW build; real
  interactive launch with no new QML errors. **Known gap, stated explicitly**: the actual
  mouse-driven click-through (IMAGE tool → canvas click → native file-picker → render) was not
  exercised — no UI automation was available for the native file-dialog step in this
  environment. Backend correctness was verified directly; QML wiring was verified by inspection
  and a clean launch, not an actual click-through.

### R-group variable attachment points (2026-07-12)

Chem-core already has the real MDL-standard machinery for this — `Atom.attachmentPoints`
(`1=FirstSideOnly, 2=SecondSideOnly, 3=BothSides`), and both the V2000 `M  APO` writer and the
KET `atomToKet` writer already read it directly and serialize it correctly. The entire gap was
that nothing in `v8_worker.js`/the UI ever set it. New `setAttachmentPoint(atomId, order)`
worker command (mirrors `changeAtomCharge`'s exact shape); new "Attachment Point 1/2/Clear"
context-menu items in `ChemCanvas.qml`; new superscript badge (`*1`/`*2`/`*3`) in
`LabelLayer.qml` alongside the existing charge/isotope badges, plus a matching one-line
bond-retraction addition in `MoleculeLayer.qml`'s `atomHasLabel()`. Full plan:
`.claude/plans/rgroup-attachment-points.md`.

- **Deliberately capped at chem-core's real ceiling** (2 attachment points per fragment, not
  arbitrary-N — extending further means touching vendored `chem-core.js`, out of scope). v1
  allows marking any atom (no fragment-membership enforcement) and does not touch the separate,
  similarly-named R1-R8 scaffold pseudo-atom-label mechanism or `RGroupPanel.qml`.
- **Third feature delegated to the Antigravity CLI** — Gemini 3.5 Flash High this time (a
  smaller, more mechanical delegation than SVG export or image embedding, tiered down
  accordingly). Read-before-writing confirmed both real chem-core writers read
  `atom.attachmentPoints` directly off the atom, so no pool-maintenance code was needed anywhere.
- Verified via a standalone Node harness spawning `src/v8_worker.js` over its real stdin/stdout
  protocol: correct `M  APO` line (parsed field-by-field, not just substring-matched — an
  earlier looser check produced a false failure against the real fixed-width MDL spacing, caught
  and corrected) for orders 1 and 2, correct `attachmentPoints` in KET output, `undo` correctly
  restores the prior order including back to fully cleared, explicit clear removes the line
  entirely. Standing smoke test unaffected; clean MinGW build; real interactive launch with no
  new QML errors.

### Explicit hydrogens fold/unfold (2026-07-12)

Part C's Indigo-export audit listed `indigoFoldHydrogens`/`indigoUnfoldHydrogens` as unused;
nothing in the app could physically add/remove explicit H atoms — distinct from the pre-existing
"Show Hydrogens" checkbox (`activeCanvas.showExplicitH`), which only toggles whether implicit H
are *drawn* and never touches the atom data. New `IndigoService::unfoldHydrogens`/`foldHydrogens`
(`src/app/IndigoService.h`/`.cpp`), each a byte-for-byte mirror of the existing `dearomatize`
method's shape except for the one core call; two new toolbar entries in `MainWindow.qml`'s
STRUCTURE GROUP row reusing the existing, previously-unused `icons/explicit-hydrogens.svg` for
both directions. Full plan: `.claude/plans/fold-unfold-hydrogens.md`.

- **Zero `src/v8_worker.js` changes** — confirmed `requestSerialize(reqId)`
  (`src/v8_process.cpp:84`) is a pure passthrough to `getStructure("mol", reqId)`, the same
  generic opaque-reqId mechanism already used by `calc_props`/`smiles`/`inchi`; the toolbar's
  existing generic `onClicked` handler already covers any new op id with no changes needed.
- **Fourth feature delegated to the Antigravity CLI** — flash tier, the smallest and most
  mechanical delegation this session (same size class as the R-group attachment-point one).
- **Real gap caught by the interactive-launch check, not by trusting the digest**: the real
  launch showed a new QML resource error — `Cannot open: qrc:/qt/qml/Sketch/App/icons/explicit-hydrogens.svg`
  — because the SVG, though present on disk and already referenced by the new toolbar entries,
  was never added to `CMakeLists.txt`'s `RESOURCES` list (the same class of omission
  `add-image.svg` needed fixed for earlier this session). Added it, rebuilt, relaunched — error
  gone, only the two pre-existing benign style warnings remained.
- Verified via a standalone C++ harness (same technique used for native SVG export) calling the
  *actual compiled* `IndigoService::unfoldHydrogens`/`foldHydrogens` directly against a real
  ethane molfile, moc'd and linked against the real vendored Indigo DLLs, bypassing the GUI:
  unfolding correctly produced 8 atoms (2 C + 6 explicit H, correct valence) each properly
  bonded to its carbon, and folding back down reproduced the original 2-atom input exactly.
  Standing smoke test unaffected (no worker changes); clean MinGW rebuild; real interactive
  launch with no new QML errors after the icon fix. **Known verification gap, stated
  explicitly**: the actual mouse-driven toolbar click-through was not exercised in this
  environment.

### Molar refractivity + pKa (2026-07-12)

Part C's Indigo-export audit listed `indigoMolarRefractivity`/`indigoPka` as unused; both are
single-`double`-return calls, identical shape to `indigoLogP`/`indigoTPSA` already used in
`IndigoService::calcProperties`. Extended `propertiesReady`'s signal with two trailing `double`
params; `MainWindow.qml`/`PropertyPanel.qml` gained two more "Drug Properties" grid rows.
Full plan: `.claude/plans/molar-refractivity-pka.md`.

- **Zero worker/CMake changes** — same `requestSerialize` passthrough as `layout`/`aromatize`;
  `indigo` core lib already linked.
- **Fifth feature delegated to the Antigravity CLI** — flash tier, small mechanical signal
  extension. **Wrapper reported `AGY_FAILED`/timeout again — another false negative**, verified
  with full rigor from the start this time (every line of all four changed files' diffs read;
  the only genuinely new content was the planned two-param signal extension, everything else
  individually cross-checked against already-documented earlier features, not assumed).
- **Verified against the actual compiled code with real numbers**: a standalone C++ harness
  called the real compiled `calcProperties` against a real aspirin molfile — molar refractivity
  41.79 (published ≈44-45) and **pKa 3.345 (aspirin's real literature pKa is ≈3.5)**, both close,
  chemically plausible matches confirming the values are computed, not stubbed.
- **UI screenshot/click-through could not be closed out — a real environment constraint.**
  Every window-focus API tried (`SetForegroundWindow`, `ShowWindow`, `WScript.Shell.AppActivate`,
  `FindWindow`, `EnumWindows`) failed to reliably surface the launched window; `EnumWindows` also
  revealed this is an actively-used desktop (Qt Creator, Notepad++, a browser already open on
  this project), so further window manipulation was stopped as a real interference risk, not
  abandoned by choice. Backend correctness is solid; on-screen row rendering wasn't visually
  confirmed this round. Standing smoke test unaffected; clean MinGW build; real interactive
  launch (stderr captured directly) shows no new QML errors.

### QML review hardening (2026-07-12)

An automated 23-finding QML review was triaged against the real code before acting — 19
findings did not survive: a hallucinated property (`font.preferShaping`, zero hits anywhere in
the codebase), a false ownership claim (`DocumentManager`'s `V8Process` objects are constructed
`new V8Process(this)`, already have a C++ parent, so QML respects existing ownership), several
false positives describing already-correct idiomatic QML used pervasively and intentionally
throughout this codebase (the `CheckBox` two-way-sync pattern, bare `activeCanvas`/`window.*`
delegate lookups, `Connections.target` going briefly `null`), one false positive with a broken
suggested fix (`RGroupPanel.qml`'s `implicitHeight` sizing isn't circular), a couple of overstated
claims (`Theme.qml`'s 118-item array, fixed-size popup grids), some speculative items the report
itself flagged as unverified, and the report's own 7 self-flagged investigation-only targets.
Full triage: `.claude/plans/qml-review-fixes.md`.

- **Four findings survived and were delegated to the Antigravity CLI** (flash tier): batching
  `ChemCanvas.qml`'s per-dot `beginPath()`/`fill()` grid-paint calls into one call for the whole
  grid; `Image.Error` logging on three previously-silent dynamic-source `Image` elements
  (`LabelLayer.qml`, `MainWindow.qml`'s `amRowIcon`, `components/IconCell.qml`); `parent`-null
  guards on five `anchors.*: parent.*` bindings in `MainWindow.qml` delegates; and
  `textFormat: Text.PlainText` on plain-string `Text` elements across `PropertyPanel.qml`,
  `IconCell.qml`, and four shared components found via an explicit whole-tree search.
- **False negative from the delegation wrapper — the mirror image of "never trust agy's
  GREEN."** `agy-delegate` reported `AGY_FAILED`/timeout, but the five target files' mtimes fell
  inside the delegation's run window and `git diff` showed exactly the planned four fixes and
  nothing more — confirmed by reading every hunk and distinguishing genuinely-new content from
  large pre-existing uncommitted diffs (canvas resize/rotate handles, R-group badges, image
  rendering — all cross-checked against already-documented earlier features) before concluding
  the work was correct and complete despite the wrapper's failure report.
- Verified: clean MinGW rebuild, standing smoke test unaffected, real interactive launch with no
  new QML errors beyond the two pre-existing benign style warnings.
- **Real regression shipped and then caught by the user, not by this pass's own verification.**
  The grid-dot batching fix removed the per-dot `ctx.beginPath()` on the assumption that
  reordering `beginPath()`/`fill()` calls around an unchanged `arc()` loop couldn't change the
  render — wrong: without a `beginPath()`/`moveTo()` immediately before each `arc()`, Canvas
  draws a straight line connecting the previous subpath's end to the new arc's start instead of
  opening a fresh circle, so the whole grid rendered as large triangular wedges (one per column)
  instead of dots — reported by the user with a screenshot showing exactly that. Fixed by adding
  `ctx.moveTo(x + dotRadius, y)` before each `arc()` call, keeping the single `beginPath()`/
  `fill()` batching while restoring correct per-dot subpaths. This time verified by actually
  launching the app and screenshotting the canvas (not just reasoning about the diff) before
  calling it fixed — the gap the first pass skipped.

---

## Two integration seams

Every backlog item is chemistry-model work (touches `_struct`, wants undo/redo, needs to
render) or toolkit work (a computed result handed back once) — never both. That split is
exactly the chem-core.js / Indigo divide, and it decides which files a new feature touches.

### ① The worker seam

For anything built on an unused **chem-core.js** export — biopolymer drawing, image
embedding, native multitail arrows, R-group attachment helpers.

1. Add the operation to `src/v8_worker.js`, calling the already-loaded `CoreLib.ChemCore.X`
   — follow the `addText`/`addChain` shape (build the object, wrap the mutation in
   `makeCmd`, call `executeCommand`)
2. Register the new `cmd` string in the if/else dispatch inside `_dispatchCommand`
   (`src/v8_worker.js:4026`)
3. Add a matching `Q_INVOKABLE` to `v8_process.h`/`.cpp` that calls `sendCommand(...)` —
   array/object-shaped arguments are fine (`QjsEngine::variantToJs` converts
   `QVariantList`/`QVariantMap` recursively into real JS arrays/objects, e.g. `addRing`'s
   `coords` list), not just scalars
4. Extend `buildRenderPrimitives` if the new entity needs to reach the canvas, then teach
   `MoleculeLayer`/`LabelLayer`/`SelectionLayer` to draw and hit-test it
5. Add the tool/menu entry in `ToolPanel.qml` or `MainWindow.qml`, reusing
   `IconCell`/`AnchoredPicker`/`TaskDialog`

### ② The Indigo seam

For anything built on an unused **Indigo C function** — similarity search, SMARTS matching,
InChI, native rendering, SDF/RDF batch browsing, scaffold decomposition, reaction
enumeration, auto-AAM/pKa.

1. Add a method to `IndigoService.h`/`.cpp` following the existing `layout()`/
   `calcProperties()` shape — `QtConcurrent::run`, own `indigoAllocSessionId()`, then
   **`emitOnGuiThread(self, [...](IndigoService *s) { emit s->xyzFinished(...); })`** to emit
   the result — not a raw `if (self) emit self->...` from the background thread. `QPointer`'s
   guard isn't thread-safe for a concurrent check-then-use; `emitOnGuiThread` posts the
   check-and-emit onto the GUI thread instead, where it's serialized against
   `~IndigoService()` by the same event loop (found and fixed across all 20 existing call
   sites in the 2026-07-10 verification pass — this is now the required pattern, not the old
   inline `QPointer` check)
2. Wire a handler in `MainWindow.qml`'s `Connections { target: indigoSvc }` block
3. Feed the result back to the model either via `activeCanvas.loadMolfile(...)` (structural
   result) or straight into a QML panel (a computed value, e.g. a similarity score or
   InChIKey string)
4. This path **never touches** `v8_worker.js` — the worker is the passive supplier of the
   molfile string Indigo is handed

### ③ The Bingo seam (new — not wired in at all yet)

For similarity search, substructure search, exact-match search, and formula search over a
collection (the template library, or a user-opened SDF/RDF).

1. **Prerequisite, not yet done:** link `bingo-nosql.dll` in `CMakeLists.txt` (mirroring the
   `indigo.dll` pattern — on MSVC this needs its own generated `.lib`, same `dumpbin`/`lib.exe`
   process already used for `indigo.dll`)
2. Add a new `BingoService` (or extend `IndigoService`) following the same
   `QtConcurrent::run` + own-session shape
3. Build the index once (e.g. over `templates/library.sdf`'s 276 entries) via
   `bingoCreateDatabaseFile`/`bingoInsertRecordObj`, then query with `bingoSearchSub`/
   `bingoSearchSim`/`bingoSearchExact`/`bingoSearchMolFormula`
4. Like the Indigo seam, this never touches `v8_worker.js` directly — results (a list of
   matching template names/ids) feed a QML popup, not the live `_struct`

---

## Backlog → integration point

| Feature | Seam | Touches |
|---|---|---|
| Word-style rulers (~~alignment icons done 2026-07-11~~) | QML-only | `MainWindow.qml` (`topRuler`/`leftRuler` still static — margins/indents/tab-stops not yet built). Alignment icons: done, see the "Toolbar alignment icons" section above and `.claude/plans/toolbar-alignment-icons.md`. No backend change either way — `alignAtoms`/`distributeAtoms` Q_INVOKABLEs already existed. |
| ~~Library-template ring fusion~~ **(done 2026-07-10)** | ① Worker | Implemented: `insertLibraryTemplateFused()` in `v8_worker.js` + `V8Process::insertLibraryTemplateFused` + `ChemCanvas.qml`'s `hitBond`-gated `LIB_` branch. See `calm-launching-kettle.md` for the verified design/implementation notes. |
| ~~Canvas interaction tools: move fix, Hand, PowerPoint-style resize/rotate handles, Lasso, page boundary~~ **(done 2026-07-11, pending manual/MSVC verification)** | ① Worker + QML | `HAND`/`SELECT_LASSO` tools in `ToolPanel.qml`; selection handles + drag handling in `ChemCanvas.qml`/`ToolOverlay.qml`; `rotateSelectionLive`/`commitRotate`/`scaleSelectionLive`/`commitScale`/`selectByLasso` + `PAGE_MIN_X/MAX_X/MIN_Y/MAX_Y` clamp in `v8_worker.js`; boundary outline in `MoleculeLayer.qml`. See the "Canvas interaction tools (2026-07-11)" section above and `calm-launching-kettle.md`. |
| ~~Reaction structure validation~~ **(done 2026-07-11)** | ② Indigo | Implemented: `IndigoService::checkStructure` now runs `indigoCheckObj` per reactant/product component via `indigoIterateReactants`/`indigoIterateProducts`, text-report only (inline highlighting deliberately stays off for reactions). See the "Reaction structure validation (2026-07-11)" section above and `calm-launching-kettle.md`. |
| Native biopolymer drawing engine | ① Worker | `DrawingEntitiesManager`/`Entities`/monomer helpers already sit unused in `chem-core.js`; would need a new worker-side monomer struct, new canvas render layer, new tool set. The biggest single item in the backlog. |
| ~~Image embedding on canvas~~ **(done 2026-07-12)** | ① Worker | Implemented: new `addImage`/`deleteImage` worker commands (a duck-typed `_makeImage` object mirroring chem-core's real, unexported `Image` class), new `FileIO::readImageAsDataUri`, new `IMAGE` tool + `FileDialog` + `LabelLayer.qml` rendering. v1 is insert/delete only — no move/resize, no clipboard-paste, KET-only round-trip. See `Features To Be Implemented.md`'s "Done (2026-07-12, image embedding)" entry and `.claude/plans/image-embedding.md`. |
| ~~R-group variable attachment points~~ **(done 2026-07-12)** | ① Worker | Implemented via the existing native `Atom.attachmentPoints` field (MDL `M  APO`), not the `getAttachmentPointLabel`/`getNextFreeAttachmentPoint` bitmask helpers originally guessed here — those operate on a different namespace (the R1-R8 scaffold label system) and remain unused. New `setAttachmentPoint` worker command, `ChemCanvas.qml` context-menu items, `LabelLayer.qml` badge rendering. See `Features To Be Implemented.md`'s "Done (2026-07-12, R-group attachment points)" entry and `.claude/plans/rgroup-attachment-points.md`. |
| ~~Explicit hydrogens fold/unfold~~ **(done 2026-07-12)** | ② Indigo | Implemented: `IndigoService::unfoldHydrogens()`/`foldHydrogens()` mirroring `dearomatize`'s exact shape around `indigoUnfoldHydrogens`/`indigoFoldHydrogens`; two new STRUCTURE GROUP toolbar buttons reusing the previously-unused `icons/explicit-hydrogens.svg`. Distinct from the pre-existing render-only "Show Hydrogens" checkbox. See `Features To Be Implemented.md`'s "Done (2026-07-12, explicit hydrogens fold/unfold)" entry and `.claude/plans/fold-unfold-hydrogens.md`. |
| ~~Molar refractivity + pKa~~ **(done 2026-07-12)** | ② Indigo | Implemented: extended `IndigoService::calcProperties`'s `propertiesReady` signal with `indigoMolarRefractivity`/`indigoPka` (two more trailing scalar doubles, same shape as the existing TPSA/LogP fields); two new "Drug Properties" rows in `PropertyPanel.qml`. Verified against real aspirin data (pKa 3.345 vs. literature ≈3.5). See `Features To Be Implemented.md`'s "Done (2026-07-12, molar refractivity + pKa)" entry and `.claude/plans/molar-refractivity-pka.md`. |
| IUPAC nomenclature, 3D viewer, spectral prediction, macros | External | None of these are latent in either vendored library — all need a new dependency or licensed engine, so no existing seam applies yet. |
| Similarity / fingerprint search | ③ Bingo (preferred) or ② Indigo | Better done via Bingo NoSQL's `bingoSearchSim`/`bingoSearchSimTopN` over a pre-built index than hand-rolled `indigoFingerprint`/`indigoSimilarity` calls — Bingo owns indexing + scoring as one package. Natural UI home is the Library popup (compare against `templates/library.sdf` entries). |
| Substructure / SMARTS search | ③ Bingo (preferred) or ② Indigo | `bingoSearchSub` over the same Bingo index, vs. hand-rolling `indigoSubstructureMatcher`/`indigoMatch` yourself. Result would highlight atoms back on the active canvas (needs a "highlight by atom id list" hook — `_struct` already tracks per-atom `checkWarning` flags the same way). |
| Bingo NoSQL indexed search (the prerequisite for both rows above) | ③ Bingo | Not yet linked at all — needs a `CMakeLists.txt` entry for `bingo-nosql.dll` (+ a generated `.lib` on MSVC, same process used for `indigo.lib`) and a new `BingoService` before either search feature above can be built. |
| ~~InChI / InChIKey generation~~ **(done 2026-07-11)** | ② Indigo | Implemented: `indigo-inchi` linked in `CMakeLists.txt` (MinGW bare-DLL + generated MSVC `.lib`); `IndigoService::inchi()`/`inchiKey()`; "Copy InChI ▾" button in `MainWindow.qml`. See the "InChI / InChIKey support (2026-07-11)" section above and `calm-launching-kettle.md`. |
| ~~Load from InChI~~ **(done 2026-07-11)** | QML-only | Implemented: `indigoSvc.layout(text)` (Indigo's generic loader auto-detects InChI natively — no plugin call needed for this direction) + a new "Load from InChI" toolbar entry/dialog in `MainWindow.qml`, mirroring "Load from SMILES." Not InChIKey — that's a one-way hash with no reverse algorithm; see the "Load from InChI" section above. |
| ~~Native SVG export~~ **(done 2026-07-12)** | ② Indigo | Implemented: `indigo-renderer` linked in `CMakeLists.txt` (MinGW bare-DLL + generated MSVC `.lib`); `IndigoService::renderToFile()`; new `*.svg` entry in the save dialog. Purely additive alongside the existing screenshot-based PNG/PDF export. Started as an `opencode` delegation that stalled partway (finished `CMakeLists.txt` only) and was completed manually. See `Features To Be Implemented.md`'s "Done (2026-07-12, native SVG export)" entry and `.claude/plans/indigo-native-svg-export.md`. |
| Native PNG/PDF rendering via indigo-renderer | ② Indigo | Would replace `AppController::exportPdf`'s canvas-screenshot approach with `indigoRenderToFile` (same plugin the SVG export above now uses) — same print/export menu entry in `MainWindow.qml`, different implementation underneath. Format/style is set entirely via `indigoSetOption` (`render-output-format`, `render-bond-length`, `render-coloring`, `render-comment`, etc.), not function args. `indigoRenderGrid`/`indigoRenderGridToFile` is a bonus fit for printing a whole reaction scheme or a template sheet in one image. |
| ~~SDF batch file browsing~~ **(done 2026-07-12)** | ① Worker | Implemented via seam ① (`src/v8_worker.js`'s existing `SdfSerializer`), not seam ② as originally scoped here — no new `IndigoService`/Indigo C API surface needed, since the worker already had everything required to parse and thumbnail multi-record SDFs (it does exactly that for `templates/library.sdf` at startup). New `deserializeSdfBatch`/`loadSdfBatchRecord` worker commands + new `SdfRecordPicker.qml` reusing `AnchoredPicker`+`ThumbnailGridItem`. `.rdf` remains unsupported — separate, larger feature. See `Features To Be Implemented.md`'s "Done (2026-07-12, SDF batch record browsing)" entry and `.claude/plans/sdf-batch-browser.md`. |
| R-Group deconvolution / scaffold analysis | ② Indigo | New `IndigoService` method around `indigoExtractCommonScaffold` + `indigoDecomposeMolecules`; a batch-input feature (multiple molecules in, e.g. an opened SDF of analogs), so it doesn't fit the single-`activeCanvas` model as cleanly — likely wants its own dialog rather than reusing the toolbar. `indigoDecomposedMoleculeWithRGroups`'s output shape could feed straight into the existing `RGroupPanel.qml`/`struct.rgroups` model — the discovery complement to Sketch's manual R-group authoring. |
| Reaction product enumeration | ② Indigo | New `IndigoService` method around `indigoReactionProductEnumerate`, taking the active canvas's reaction (reactants/products already split via `rxnArrows`) plus a reactant-set input dialog. |
| Automatic atom-mapping / pKa prediction | ② Indigo | `indigoAutomap` could replace/augment the worker's manual click-to-increment AAM (`atom.aam` field) — this one is unusual in that its *result* (mapping numbers) would need to flow back through the ① worker seam via `setAtomMapping`, one of the few features that crosses both seams. |

---

*Signatures verified against `src/v8_process.h` (90 Q_INVOKABLE methods),
`src/app/IndigoService.h`, `src/app/DocumentManager.h`, and the dispatch chain in
`src/v8_worker.js:4026+`. Companion documents: `Features To Be Implemented.md` and the
chem-core.js/Indigo coverage audit.*
