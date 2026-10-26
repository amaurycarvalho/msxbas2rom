# Spec Delta

## Purpose

Defines how the `make lint` target selects source files locally and in CI, and how the full local lint gate is invoked.

## ADDED Requirements

### Requirement: Local lint selects changed C++ sources

When running outside CI, `make lint` SHALL lint only `src/**/*.cpp` files changed in the working tree relative to `HEAD`, plus untracked `.cpp` files. Deleted `.cpp` files SHALL NOT be passed to the compiler. If no `.cpp` file changed, the target SHALL succeed without compiling anything.

#### Scenario: Tracked source change

- **WHEN** a tracked `.cpp` file is modified and `make lint` runs locally
- **THEN** only that changed `.cpp` file is passed to the compiler

#### Scenario: Untracked source file

- **WHEN** a new untracked `.cpp` file exists and `make lint` runs locally
- **THEN** the new `.cpp` file is included in the lint selection

#### Scenario: Deleted source file

- **WHEN** a `.cpp` file is deleted and no other `.cpp` file changed
- **THEN** `make lint` succeeds without passing the deleted file to the compiler

#### Scenario: No changed source file

- **WHEN** only files outside `src/**/*.cpp` changed and `make lint` runs locally
- **THEN** `make lint` succeeds without compiling anything

### Requirement: CI lint remains full-tree

When the `CI` environment value is `true`, `make lint` SHALL lint every `src/**/*.cpp` file, independent of git status. The CI workflow SHALL continue to invoke `make lint` without supplying a changed-file range.

#### Scenario: CI with a small change

- **WHEN** `CI=true` and only one `.cpp` file changed
- **THEN** `make lint` lints all source files, not only the changed file

#### Scenario: CI with a clean tree

- **WHEN** `CI=true` and the working tree has no changes
- **THEN** `make lint` lints all source files

#### Scenario: CI workflow call is unchanged

- **WHEN** the CI quality-gate workflow runs
- **THEN** it invokes `make lint` and relies on the CI environment for full-tree selection

### Requirement: Full local lint targets

`make lint-all` SHALL lint every `src/**/*.cpp` file locally. `make lint-full` SHALL continue to lint every source file with `-fanalyzer` enabled.

#### Scenario: Explicit full local lint

- **WHEN** `make lint-all` runs locally
- **THEN** every source file is passed to the compiler without `-fanalyzer`

#### Scenario: Full analyzer lint

- **WHEN** `make lint-full` runs locally
- **THEN** every source file is linted with `-fanalyzer`

### Requirement: Local quality gate remains full

`make quality-gate` SHALL depend on `lint-all`, so the local quality gate lints the full source tree rather than only changed files.

#### Scenario: Quality gate includes full lint

- **WHEN** `make quality-gate` runs locally
- **THEN** the full-tree lint target runs as part of the gate

### Requirement: Header and Makefile changes do not force full local lint

Local `make lint` SHALL NOT switch to full-tree lint solely because a header, another non-source file, or the `Makefile` changed. It SHALL lint only changed `.cpp` files; when none changed, it SHALL skip and print guidance to use `make lint-all`.

#### Scenario: Header-only change

- **WHEN** only a `.h` or `.hpp` file changed and `make lint` runs locally
- **THEN** no full-tree lint is triggered and the target prints guidance to use `make lint-all`

#### Scenario: Header and source change

- **WHEN** a header and one `.cpp` file changed and `make lint` runs locally
- **THEN** only the changed `.cpp` file is linted

#### Scenario: Makefile-only change

- **WHEN** only the `Makefile` changed and `make lint` runs locally
- **THEN** no full-tree lint is triggered and the target prints guidance to use `make lint-all`

#### Scenario: Makefile and source change

- **WHEN** the `Makefile` and one `.cpp` file changed and `make lint` runs locally
- **THEN** only the changed `.cpp` file is linted

### Requirement: Lint output distinguishes skip from pass

When local lint skips because no `.cpp` file changed, it SHALL NOT print the pass message and SHALL print guidance to run `make lint-all`. When selected files pass, it SHALL print the pass message. When a selected file fails, the target SHALL exit non-zero.

#### Scenario: Skip output

- **WHEN** local lint skips an empty changed-source set
- **THEN** it prints guidance to use `make lint-all` and does not print the pass message

#### Scenario: Passing selected files

- **WHEN** all selected source files compile cleanly
- **THEN** the target prints the pass message and exits zero

#### Scenario: Failing selected file

- **WHEN** a selected source file produces a lint diagnostic
- **THEN** the target exits non-zero and does not print the pass message

### Requirement: No-version-control fallback

If the working directory is not a Git repository, `make lint` SHALL fall back to linting every source file, even outside CI.

#### Scenario: Source archive without Git metadata

- **WHEN** `make lint` runs in a directory without a `.git` repository
- **THEN** every source file is linted
