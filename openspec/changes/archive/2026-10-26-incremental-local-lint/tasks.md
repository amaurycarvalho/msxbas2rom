# Tasks

## 1. Makefile targets

- [x] 1.1 Add `lint-all` to the `.PHONY` list and verify `make -n lint-all` resolves without errors.
- [x] 1.2 Replace the `lint` recipe with the CI/no-Git full-tree branch and the local changed-`.cpp` branch; verify `CI=true make lint SRC_FILES='src/application/compiler/compiler.cpp'` takes the full-tree branch and a single changed `.cpp` is the only file selected by `make lint` (inject a temporary diagnostic in that file, observe that only it is reported, then revert).
- [x] 1.3 Add the `lint-all` target using `SRC_FILES` without `-fanalyzer`; verify `make lint-all SRC_FILES='src/application/compiler/compiler.cpp'` lints that file and that `make -n lint-full` still contains `-fanalyzer`.
- [x] 1.4 Change `quality-gate` to depend on `lint-all` and verify `make -n quality-gate` lists `lint-all` instead of `lint`.
- [x] 1.5 Verify skip and failure states: with only the `Makefile` or a header changed, `make lint` prints the `make lint-all` guidance and no pass message; with a changed `.cpp` containing a temporary diagnostic, `make lint` exits non-zero and does not print the pass message.

## 2. Documentation

- [x] 2.1 Update `CONTRIBUTING.md` with the local/CI split (`make lint`, `make lint-all`, `make quality-gate`) and verify the documented commands match the implemented targets.
- [x] 2.2 Update `doc/misc/architecture.md` section 10 with `lint-all` and the local/CI behavior, and verify the command list matches the `Makefile`.

## 3. Integration verification

- [x] 3.1 Verify the no-Git fallback with `GIT_DIR=/nonexistent make lint SRC_FILES='src/application/compiler/compiler.cpp'` and confirm it prints the full-tree message.
- [x] 3.2 Run `make lint-all` once over the real source tree and confirm it passes; run `make test-architecture` and confirm it passes.
- [x] 3.3 Run `openspec validate "incremental-local-lint" --strict` and confirm the change validates.

## Workflow follow-up

- Archive the change after review and run the changelog skill.
