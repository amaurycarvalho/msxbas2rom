## Why

The unit-test binary currently contains end-to-end CLI and real-file compilation scenarios, so every Mull mutant run repeats filesystem I/O and multi-line compilations. The post-Round-6 mutation baseline is 34.00 s, close to 24 hours for a full run, and running the binary from the repository root creates an untracked `tmp/cli_characterization/` tree. Test execution cost is multiplied by the number of mutants, so E2E file work must not live in the unit binary.

## What Changes

- Move `test_cli_characterization.cpp` from `tests/unit/src/` to the integration suite, preserving its CLI output/exit-code assertions in a real-file E2E script.
- Remove the compile-based `FS suite`, `GRAPH test`, and `MEGAROM test` blocks from `tests/unit/src/test_fs.cpp`; those programs are already covered by `tests/integration`.
- Keep only explicit file-adapter tests in the unit binary, and make them use a unique OS temporary directory through a shared RAII helper instead of project-relative `tmp/`.
- Reduce the `Plain-ROM overflow without auto retry` unit test so it no longer adds ~4 s to every mutant run.
- Strengthen `scripts/check_test_inventory.py`: non-adapter unit tests may not perform filesystem I/O; adapter tests may use the OS temp directory but not project-relative `tmp/`.
- Re-measure the mutation baseline after the migration; if it falls below 32 s, restore the Mull timeout to 32000 ms.

## Capabilities

### New Capabilities

<!-- None. -->

### Modified Capabilities

- `unit-test-coverage`: require E2E file-backed CLI scenarios to live in integration, allow explicit file-adapter tests only through OS temporary directories, and require the test inventory check to enforce the boundary.
