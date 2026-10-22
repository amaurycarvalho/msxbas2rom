# Tasks

## 1. OS temporary directory helper

- [x] 1.1 Add `tests/unit/src/test_temp_dir.h` with a cross-platform RAII `TestTempDir` using `TMPDIR`/`TEMP` and `mkdtemp`/`CreateDirectoryA`; verify it compiles on the unit target.
- [x] 1.2 Convert `tests/unit/src/test_file_adapters.cpp` to use `TestTempDir` and verify the File adapters suite passes.
- [x] 1.3 Convert the file-writing paths in `test_source_boundary.cpp`, `test_resource_boundary.cpp`, and `test_lexer.cpp` to `TestTempDir`; verify their suites pass.

## 2. Move E2E scenarios out of the unit binary

- [x] 2.1 Add `tests/integration/cli/run_cli_characterization.sh` with normal, quiet, debug, symbol, missing-input, and ASCII8-retry cases, using a unique temporary directory and a cleanup trap; verify the script exit code is zero.
- [x] 2.2 Invoke the CLI characterization script from `tests/integration/Makefile` and verify `make -C tests/integration run` exercises it.
- [x] 2.3 Delete `tests/unit/src/test_cli_characterization.cpp` and verify the unit suite no longer contains CLI characterization.
- [x] 2.4 Remove the `FS suite`, `GRAPH test`, and `MEGAROM test` blocks from `tests/unit/src/test_fs.cpp`, keep the pure `Fswrapper`/logger tests, and verify the remaining suite passes.

## 3. Reduce mutation cost and restore timeout

- [x] 3.1 Shrink `Plain-ROM overflow without auto retry fails explicitly` to the same 200-line fixture used by the auto-retry test and verify it still fails with the plain-ROM limit.
- [x] 3.2 Measure the post-migration baseline with `tests/unit/bin/test_unit --test-suite-exclude=Slow`: 612 tests / 8,843 assertions in **33.11 s**, still above the 32 s cap.
- [x] 3.3 The measured baseline is 33.11 s, so `--timeout`/`--minimum-timeout` remain at 45000 ms. Recorded rationale: the suite is still above the former 32 s cap even after removing the CLI E2E and shortening the overflow test.

## 4. Enforce the boundary

- [x] 4.1 Extend `scripts/check_test_inventory.py` to reject disk access in non-adapter tests and project-relative `tmp/` paths in adapter tests.
- [x] 4.2 Add success/failure cases to `scripts/tests/test_check_test_inventory.py` and verify `python3 -m unittest discover -s scripts/tests` passes.
- [x] 4.3 Run `make test-architecture` and verify the inventory and architecture checks reject the old patterns and accept the migrated tree.

## 5. Verification and handoff

- [x] 5.1 Run `make -C tests/unit run`: 613 tests / 8,846 assertions pass, down from 624 / 8,991 after moving the CLI E2E scenarios out of the unit binary.
- [x] 5.2 Run `make -C tests/integration run`: all real-file compilation modes and the CLI characterization script pass.
- [x] 5.3 Verified the unit binary passes from the repository root and creates no `tmp/` directory.
- [x] 5.4 Recorded: unit baseline 33.11 s excluding `Slow`, unit suite 613 / 8,846, integration green, timeout remains 45000 ms. The final mutation run remains user-managed.
