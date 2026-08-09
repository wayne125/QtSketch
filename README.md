# sketch

A desktop chemical structure editor (Qt 6 / QML) for drawing, editing, and analyzing
molecules — 2D structure drawing, reactions, biopolymer sequences, and a growing
IUPAC name generator, backed by the Indigo cheminformatics toolkit.

## Features

- 2D molecule drawing and editing (atoms, bonds, rings, templates, R-groups)
- Reaction drawing and biopolymer sequence support
- Structure cleanup/layout, aromatization, InChI/SMILES conversion, and CIP
  stereodescriptors via [Indigo](https://github.com/epam/indigo)
- Chemical-structure-image recognition (OCR) via [Imago](https://github.com/epam/indigo)
  for the Insert Image feature
- A from-scratch IUPAC name generator (`src/app/IupacNamer.cpp`) built incrementally
  against the official Blue Book text — see `IUPAC Blue Book Coverage.md` for current
  section-by-section coverage
- An in-process chemistry scripting engine: `chem-core.js` (a Ketcher-derived
  cheminformatics core) runs inside the app via an embedded QuickJS-ng interpreter
  (`src/qjs_engine.*`), reached through the same command-in / JSON-line-out protocol
  a legacy Node.js child-process worker (`src/v8_process.*`, `src/worker/*.js`) used to
  speak — the worker JS itself is unchanged, only the host process changed

## Tech stack

- **UI**: Qt 6 Quick / QML (`Sketch.App` QML module), Fluent Windows 11 style
- **Backend**: C++20 (`src/app/`) — services exposed to QML via `Q_INVOKABLE`
- **Chemistry**: Indigo toolkit (`indigo.dll`), Imago (`imago.dll`)
- **Scripting core**: `chem-core.js` + `src/v8_worker.js`, executed in-process via
  QuickJS-ng (`third_party/quickjs-ng`)
- **Build**: CMake 3.24+, `qt_add_executable` / `qt_add_qml_module`

## Building

Requires Qt 6.6+ (developed against 6.11.1) and a C++20 compiler. On Windows this
project is built with MinGW, not MSVC:

```powershell
cmake -B build -G "MinGW Makefiles"
C:\mingw64\bin\mingw32-make.exe -C build -j4
```

The `sketch` target is the main application executable. `indigo.dll` / `imago.dll`
and their lib directories must be on `PATH` at runtime (see `run_sketch.bat` for the
expected layout).

## Running

```powershell
.\run_sketch.bat
```

or run `build\sketch.exe` directly with `indigo\lib` on `PATH`.

## Testing

Three standalone C++ test executables are registered with CTest:

```powershell
C:\mingw64\bin\mingw32-make.exe -C build iupac_namer_test fused_ring_orientation_test fused_ring_direction_detector_test -j4
ctest --test-dir build
```

- `tests/iupac_namer_test.cpp` — the IUPAC namer's own growing regression suite
- `tests/fused_ring_orientation_test.cpp` / `fused_ring_direction_detector_test.cpp` —
  geometric ring-orientation modules built and tested standalone (not yet wired into
  the main app's naming pipeline)
- `tests/worker_smoke.mjs` — a Node-based smoke test for the worker JS logic itself
  (`node tests/worker_smoke.mjs`), independent of the in-process QuickJS host

## Project layout

```
src/app/          C++ backend: services (Indigo, Imago, IUPAC namer), app controller
src/worker/       Worker-side JS command handlers (state, edit, templates, serialize, ...)
src/qjs_engine.*  In-process QuickJS-ng host for chem-core.js / v8_worker.js
src/v8_process.*  Legacy Node.js child-process transport (same protocol, alternate host)
*.qml             UI: MainWindow, ChemCanvas, panels, dialogs, toolbars
tests/            C++ and JS test suites
indigo/, imago/   Vendored Indigo/Imago headers and DLLs
docs/             Design/analysis notes
```

## Documentation

- `Architecture Map.md` — architecture and module map
- `Features To Be Implemented.md` — roadmap / feature backlog
- `IUPAC Blue Book Coverage.md` — IUPAC namer coverage tracked against the Blue Book
- `THIRD-PARTY-NOTICES.md` — third-party licenses (Ketcher, Indigo, Imago)

## License and third-party notices

This repository does not currently declare its own license. See
`THIRD-PARTY-NOTICES.md` for the licenses of bundled third-party components
(Ketcher, Indigo, Imago — all Apache License 2.0).
