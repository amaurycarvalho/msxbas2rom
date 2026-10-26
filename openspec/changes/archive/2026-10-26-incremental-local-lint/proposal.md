# Proposal

## Why

`make lint` currently compiles all ~300 translation units in a single `g++ -fsyntax-only` invocation. On a 4-CPU development machine a 10-file sample took ~26 seconds and the full target exceeded 5 minutes, making the local edit/lint loop unnecessarily slow. CI must keep linting the whole tree because it is the authoritative warning gate, but local developers only need feedback on what they changed.

## What Changes

- `make lint` keeps full-tree behavior when `CI=true`, so the CI workflow is unchanged.
- Locally, `make lint` lints only changed `src/**/*.cpp` files (working tree versus `HEAD`, including untracked files) and exits successfully with guidance when no C++ source changed.
- Remove the automatic fallback to full lint when a header or the `Makefile` changes. Local lint becomes a fast, best-effort check; header and build-flag validation is an explicit `make lint-all` or `make quality-gate`.
- Add `make lint-all` for a full local lint run.
- Keep `make lint-full` (full tree with `-fanalyzer`) unchanged.
- Point `make quality-gate` at `lint-all` so the local quality gate remains full.
- Document the local/CI split in the contributing guide and the build command reference.

## Capabilities

### New Capabilities

- `lint-workflow`: Local and CI selection rules for the `make lint` target, including the explicit full local lint target and the local quality-gate contract.

### Modified Capabilities

None.

## Impact

- **Build tooling**: `Makefile` — `lint` target behavior, new `lint-all` target, `quality-gate` dependency, and `.PHONY` update.
- **Documentation**: `CONTRIBUTING.md`, `doc/misc/architecture.md` (build and quality command reference).
- **CI**: no workflow changes; `.github/workflows/ci.yml` continues to call `make lint` and receives full-tree behavior through `CI=true`.
- **Runtime behavior**: none; no compiler or generated-ROM changes.
- **Release**: 1.4.0.0
