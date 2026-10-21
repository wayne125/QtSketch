# IUPAC Golden Data

This directory contains golden-data regression test files for the IUPAC naming engine.

## baseline.csv

The `baseline.csv` file contains the expected output of the IUPAC naming engine for each test case.

### Format

CSV file with the following columns:

| Column | Type | Description |
|--------|------|-------------|
| `id` | integer | Test case identifier (1-indexed) |
| `smiles` | string | SMILES string for the molecule |
| `pinned_name` | string | Expected IUPAC name from the existing test suite's pinned assertions |
| `pinned_success` | boolean | Whether the existing test expects success (true) or failure (false) |
| `actual_name` | string | Actual IUPAC name produced by `generateName()` at baseline capture time |
| `actual_success` | boolean | Whether `generateName()` succeeded at baseline capture time |
| `expected_status` | string | Reserved for future use (currently empty) |
| `notes` | string | Any notes or warnings about the test case |

### Purpose

This file establishes the **behavior fingerprint** for the IUPAC naming engine at a specific point in time. Later tasks in this plan (Task C onward) must produce identical `actual_name`/`actual_success` values to this baseline.

### Generation

The baseline was captured using:
1. `tools/iupac/extract_baseline.py` - Parses both the C++ test source and the raw test output
2. `tests/iupac_namer_test.cpp` - The existing test suite (executed to capture actual output)

To regenerate the baseline:

```bash
cd build
C:\mingw64\bin\mingw32-make.exe iupac_namer_test -j4
./iupac_namer_test.exe > baseline_raw.txt 2>&1
python ../tools/iupac/extract_baseline.py --cpp ../tests/iupac_namer_test.cpp --raw baseline_raw.txt --output ../tests/data/iupac/golden/baseline.csv
```

### Test Harness

The `tests/iupac_golden_test.cpp` program reads this CSV file and verifies that:
1. Each test's fresh output matches the `pinned_name`/`pinned_success` (existing test semantics)
2. Each test's fresh output matches the `actual_name`/`actual_success` (behavior fingerprint)

Both checks are performed separately so that a pin mismatch (expected for known-failing tests) never masks a fingerprint mismatch (a real regression).

### Determinism Check

With the `IUPAC_GOLDEN_DETERMINISM=1` environment variable, the test
harness also verifies that generating 5 seeded atom-order reorderings
(seeds 1-5) of each molecule produces byte-identical names from
`generateName()`. The reordering is a real seeded permutation of the
molecule's atom order (via `tools/iupac/AtomOrderRandomizer.{h,cpp}`,
shared with the standalone `tools/iupac/randomize_atom_order` CLI tool),
not a re-run on the unchanged SMILES. It self-checks that the reordered
molecule is still the same molecule (same Indigo canonical SMILES) and
that its raw atom order genuinely differs from the original before
returning it. Molecules with any stereocenter or stereo-flagged bond are
skipped for this check (`[DETERMINISM SKIP]` in the output) rather than
reordered, since this permutation does not re-derive stereo descriptors
for the new atom order.
