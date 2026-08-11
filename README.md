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

## Tech stack

- **UI**: Qt 6 Quick / QML (`Sketch.App` QML module), Fluent Windows 11 style
- **Backend**: C++20 (`src/app/`), a pure in-process document model (`DocumentState`,
  `EditableMolecule`) with no scripting layer — exposed to QML via `Q_INVOKABLE`
- **Chemistry**: Indigo toolkit (`indigo.dll`), Imago (`imago.dll`)
- **Build**: CMake 3.24+, `qt_add_executable` / `qt_add_qml_module`
- **Platform**: Windows only (the build links `.dll`/`.lib` files directly; no
  Linux/macOS path exists in `CMakeLists.txt` today)

An earlier version of this app ran its chemistry logic through an embedded QuickJS-ng
script engine (`chem-core.js`, `src/worker/*.js`). That engine has been fully retired —
every document now runs on the C++ `DocumentState` model exclusively — and its files were
relocated (not deleted) to `archive/js-engine/` for reference.

## Installation

This project only builds on Windows today, with the MinGW toolchain (not MSVC — the
vendored Indigo/Imago DLLs are MinGW-built and their exports are not directly linkable
by `link.exe`).

### 1. Prerequisites

Install these first, in any order:

- **Qt 6.6 or newer** (developed against 6.11.1) with the **MinGW 64-bit** kit, via the
  [Qt online installer](https://www.qt.io/download-qt-installer) or the `aqtinstall`
  command-line tool. You need at minimum the Core, Gui, Qml, Quick, QuickControls2, and
  Concurrent modules — these are included in a standard Qt Quick installation.
- **The MinGW toolchain that ships with Qt's MinGW kit** (e.g. `C:\Qt\Tools\mingw1310_64`,
  or a standalone `C:\mingw64` install using the same GCC major version as your Qt kit).
  This must be a real, separate MinGW install on disk — do not rely on a different
  MinGW/MSYS2 copy (e.g. one bundled with Git Bash, Conda, or MSYS2) being first on
  `PATH`, since a mismatched `libstdc++`/`libgcc` ABI causes runtime crashes
  (`STATUS_ENTRYPOINT_NOT_FOUND`) that look unrelated to any code change.
- **CMake 3.24 or newer** — https://cmake.org/download/ (or `winget install Kitware.CMake`).

### 2. Vendor Indigo and Imago

Indigo and Imago are not committed to this repository (they're large binary SDKs — see
`.gitignore` and `THIRD-PARTY-NOTICES.md`) and must be placed on disk yourself, at the
repo root, before configuring the build:

```
indigo/
    LICENSE
    api/c/indigo/            (indigo.h and friends)
    api/c/indigo-inchi/
    api/c/indigo-renderer/
    lib/
        indigo.dll
        indigo-inchi.dll
        indigo-renderer.dll

imago/
    LICENSE
    include/
        imago_c.h
    lib/
        imago.dll
```

Both are Apache-2.0 (EPAM Systems). Build them yourself from source with your MinGW
toolchain — https://github.com/epam/indigo and https://github.com/epam/Imago — matching
the directory layout above, or obtain a prebuilt MinGW-compatible distribution from
elsewhere and copy the headers/DLLs into the same layout. `CMakeLists.txt` expects these
exact paths (`indigo/api/c/...`, `indigo/lib/*.dll`, `imago/include/imago_c.h`,
`imago/lib/imago.dll`); nothing else about their internal build process matters to this
project.

### 3. Configure

From the repo root, in **PowerShell** (not Git Bash's own shell — it may resolve a
different `mingw32-make`/compiler than the one Qt's CMake integration expects):

```powershell
cmake -B build -G "MinGW Makefiles" `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64" `
    -DCMAKE_C_COMPILER="C:\mingw64\bin\gcc.exe" `
    -DCMAKE_CXX_COMPILER="C:\mingw64\bin\g++.exe"
```

Adjust `CMAKE_PREFIX_PATH` to your actual Qt kit path and the compiler paths to your
actual MinGW install. Without an explicit `-G "MinGW Makefiles"`, CMake may default to
a Visual Studio generator it finds on the machine and then fail to locate Qt6 at all
(`Qt6Config.cmake` not found) — the generator and the Qt kit must be the same toolchain
family.

### 4. Build

```powershell
$env:PATH = "C:\mingw64\bin;$env:PATH"
C:\mingw64\bin\mingw32-make.exe -C build -j4
```

This builds the `sketch` executable plus 10 standalone test executables (see Testing
below). A post-build step copies `indigo.dll`/`indigo-inchi.dll`/`indigo-renderer.dll`/
`imago.dll` from step 2 next to `sketch.exe`, and copies `assets/monomer_library.ket`
into `indigo/data/molecules/basic/` — both automatic, no manual copying needed.

### 5. Run

```powershell
.\run_sketch.bat
```

`run_sketch.bat` (not committed to git — create it yourself if missing) puts Qt's
`mingw_64\bin` and `indigo\lib` on `PATH` before launching, since Windows resolves DLLs
from `PATH` when they aren't next to the `.exe`:

```bat
@echo off
set PATH=C:\Qt\6.11.1\mingw_64\bin;%~dp0indigo\lib;%PATH%
start "" "%~dp0build\sketch.exe"
```

Or run `build\sketch.exe` directly, provided the same two directories are already on
`PATH` in that shell.

### Reconfiguring after moving/removing vendored files

If you ever delete or move `build/` and reconfigure, or if CMake reports it can't find a
subdirectory that used to exist, delete the stale cache first — CMake refuses to switch
generators or toolchains in place:

```powershell
Remove-Item build\CMakeCache.txt, build\CMakeFiles -Recurse -Force
```

Then repeat step 3.

## Testing

Ten standalone C++/Qt test executables are registered with CTest — built automatically
by the same `mingw32-make -j4` from step 4 above:

```powershell
cd build
ctest --output-on-failure
```

Expected: `100% tests passed, 0 tests failed out of 10`.

| Test | Covers |
|---|---|
| `iupac_namer_test` | The IUPAC namer's own growing regression suite |
| `fused_ring_orientation_test` | Geometric ring-orientation module (standalone; not yet wired into the naming pipeline) |
| `fused_ring_direction_detector_test` | Geometric ring-direction module (standalone; not yet wired into the naming pipeline) |
| `editable_molecule_test` | Core molecule-editing operations |
| `document_state_test` | Document-level state (undo/redo, selection, commands) |
| `render_primitives_test` | Geometry → render-primitive conversion |
| `sdf_batch_test` | SDF/RDF batch import |
| `clipboard_preview_test` | Clipboard paste-preview geometry |
| `biopolymer_sequence_view_test` | Biopolymer sequence view snapshot model |
| `render_primitives_to_variant_test` | Render-primitive → QML-variant conversion |

## Project layout

```
src/app/          C++ backend: DocumentState (document model), EditableMolecule,
                   IndigoService, ImagoService, IupacNamer, AppController
src/v8_process.*  QML-facing command surface (name is historical — no JS engine
                   behind it anymore; every command runs on DocumentState directly)
*.qml             UI: MainWindow, ChemCanvas, panels, dialogs, toolbars
js/               Small live QML-side JS utility modules (Selection, ToolLabels,
                   ThumbnailPainter) — unrelated to the retired scripting engine below
tests/            C++ test suites (see Testing above)
archive/js-engine/  The retired QuickJS-ng-based chemistry engine (chem-core.js,
                   the old worker/*.js command scripts, qjs_engine.*, the vendored
                   quickjs-ng source) — kept for reference, not built
indigo/, imago/   Vendored Indigo/Imago headers and DLLs (not committed — see
                   Installation above)
docs/             Design/analysis notes
```

## Documentation

- `IUPAC Blue Book Coverage.md` — IUPAC namer coverage tracked against the Blue Book
- `THIRD-PARTY-NOTICES.md` — third-party licenses (Ketcher, Indigo, Imago)
- `Architecture Map.md` / `Features To Be Implemented.md` — kept locally as working
  notes, not committed to this repository (see `.gitignore`)

## License and third-party notices

This repository does not currently declare its own license. See
`THIRD-PARTY-NOTICES.md` for the licenses of bundled third-party components
(Ketcher, Indigo, Imago — all Apache License 2.0).
