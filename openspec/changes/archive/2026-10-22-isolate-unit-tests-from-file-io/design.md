## Context

See `proposal.md` for motivation. Current relevant facts:

- The unit binary is run once per Mull mutant. The final pre-change baseline is 34.00 s excluding `Slow`; the 8 KB blob test is already tagged `Slow`.
- `test_cli_characterization.cpp` invokes `runCli` and writes sources/ROMs/symbols relative to the process CWD; running from the repository root creates `tmp/cli_characterization/`.
- `test_fs.cpp` mixes pure `Fswrapper`/logger tests with compile-based E2E suites that duplicate `tests/integration`.
- `test_file_adapters.cpp`, `test_source_boundary.cpp`, `test_resource_boundary.cpp`, and `test_lexer.cpp` contain explicit file-adapter cases and use project-relative `tmp/`.
- `scripts/check_test_inventory.py` currently rejects only `.bas` file creation in non-adapter tests; adapter tests bypass all checks.
- The project is C++11 and supports Linux and Windows; `std::filesystem` is unavailable.
- `tests/integration/Makefile` already compiles every `tests/integration/**/*.bas` through the real CLI in several mapper modes.

## Goals / Non-Goals

**Goals:**

- Keep E2E CLI and real-file compilation scenarios out of the unit/mutation binary without losing their assertions.
- Keep only explicit file-adapter tests in the unit binary and make them CWD-independent.
- Reduce the mutation baseline enough to restore the 32 s timeout.
- Automate the boundary with the test inventory check.

**Non-Goals:**

- Do not remove coverage of file-backed adapters; only isolate and relocate it.
- Do not change production code or the CLI behavior being characterized.
- Do not move pure path-helper tests out of the unit suite.

## Decisions

### Decision 1: CLI characterization moves to an integration script

The CLI scenarios move to `tests/integration/cli/run_cli_characterization.sh`, driven by fixture `.bas` files and invoked from `tests/integration/Makefile`. The script uses a unique directory under `TMPDIR`/`TEMP`, captures stdout/stderr, checks exit codes and diagnostic strings, verifies generated artifacts, and removes the directory with a trap.

- Rationale: the integration suite already owns real-file E2E behavior and is not consumed by Mull.
- Alternative: a separate doctest binary under `tests/e2e`. Rejected as unnecessary build surface; integration already runs the real CLI.
- Alternative: keep the test in the unit binary but exclude its suite from Mull. Rejected because the integration suite is the correct owner and exclusion would silently reduce unit coverage.

### Decision 2: Compile-based `test_fs` suites are removed, not migrated

The `FS suite`, `GRAPH test`, and `MEGAROM test` blocks are deleted from `tests/unit/src/test_fs.cpp`. Their `.bas` programs remain in `tests/integration` and are compiled there in ASCII8, ASCII16, ASCII16-X, and Konami4 modes by the existing integration Makefile.

- Rationale: the scenarios are already covered; moving code would duplicate the integration runner.
- Alternative: port them to the integration script. Rejected as duplication without new assertions.

### Decision 3: File-adapter tests use an RAII OS temporary directory

Add `tests/unit/src/test_temp_dir.h` with a small cross-platform `TestTempDir` helper:

- POSIX: `TMPDIR` or `/tmp`, then `mkdtemp()` with a `msxbas2rom_test_XXXXXX` template.
- Windows: `GetTempPathA()` plus a unique `msxbas2rom_test_<pid>_<counter>` directory created with `CreateDirectoryA()`.
- Destructor removes regular files and the directory; failures are ignored.
- The path is exposed through `path()` and `file(name)`.

All explicit adapter tests construct a local `TestTempDir` and use its absolute paths. Real integration fixtures that must remain read-only (AKM/AKX songs) continue to be read from `tests/integration` through the in-memory resource source, but they do not write to the project tree.

- Rationale: OS temp is unique, writable, cleaned automatically, and independent of CWD.
- Alternative: add root `tmp/` to `.gitignore`. Rejected because it only hides the pollution and keeps the CWD dependency.

### Decision 4: The inventory check enforces the boundary

Extend `scripts/check_test_inventory.py`:

- Non-adapter unit tests fail on any disk-access pattern (`ifstream`, `ofstream`, `fopen`, `freopen`, `std::remove`, `unlink`, `createPath`).
- Marked adapter tests may use disk but fail when a disk-access line uses a literal project-relative `tmp/` path.
- The existing `.bas` temporary-file rule remains.
- Add unit tests in `scripts/tests/test_check_test_inventory.py` for both failures and the adapter exception.

- Rationale: the architectural rule becomes executable instead of documentation-only.
- Alternative: rely on review. Rejected because the mutation cost is easy to reintroduce.

### Decision 5: Shrink the overflow test and restore the timeout only after measuring

`Plain-ROM overflow without auto retry fails explicitly` uses the same 200-line program as the existing auto-retry test, which already exceeds the plain-ROM limit. After the migration, measure the no-`Slow` baseline. It measured 33.11 s, still above 32 s, so `--timeout`/`--minimum-timeout` remain at 45000 ms and the rationale is recorded in `tasks.md`.

- Rationale: the timeout should reflect the measured baseline, not a one-off temporary regression.

## Risks / Trade-offs

- [Risk: integration script duplicates logic from the removed CLI test] -> Keep exact assertions in the script and reuse the same fixture names.
- [Risk: OS temp is unavailable or restricted on a build machine] -> Fall back to the current directory only as a last resort, and report the fallback; CI machines provide `TMPDIR`/`TEMP`.
- [Risk: recursive cleanup is incomplete on Windows] -> Delete regular files first, then remove the directory; ignore failures and use unique names so leftovers never collide.
- [Risk: moving tests hides behavior] -> Preserve every assertion from the CLI test in the integration script and keep the integration target part of `quality-gate`.
- [Risk: inventory check flags in-memory tests that merely contain `tmp/` strings] -> Match only lines that combine a disk-access API with the path literal.

## Migration Plan

1. Add `TestTempDir` and convert explicit adapter tests.
2. Move CLI characterization to the integration script and remove the unit test.
3. Remove the compile-based `test_fs` suites.
4. Shrink the overflow test.
5. Extend the inventory check and its tests.
6. Run the unit suite, architecture check, inventory check, and integration suite.
7. Measure the no-`Slow` baseline; restore the 32 s timeout if below the cap.
