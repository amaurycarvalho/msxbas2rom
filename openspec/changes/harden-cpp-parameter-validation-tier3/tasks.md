# Tasks

## 1. Parameter helper API

- [ ] 1.1 Add numeric, string, and fixed-table index parameter helpers with explicit sentinel support; verify helper unit tests for numeric, string, and sentinel cases.
- [ ] 1.2 Keep `addCast` behavior unchanged for expressions and `LET`; verify existing expression/assignment tests still pass.

## 2. Fixed-table policy

- [ ] 2.1 Implement one range policy for sprite, tile, KEY, STRIG, and resource index literals, including documented sentinels such as `COLLISION(-1)`.
- [ ] 2.2 Reject string and non-scalar index expressions for fixed-table parameters; verify representative pass/fail cases.

## 3. Handler migration

- [ ] 3.1 Migrate graphics coordinate commands (`LOCATE`, `PSET`, `LINE`, `CIRCLE`, `PAINT`, `DRAW`, `PUT`, `GET`, `SET`) to the numeric parameter helper; verify existing graphics tests and new type-mismatch tests.
- [ ] 3.2 Migrate sprite, tile, MTF, Arkos, and file-number handlers; verify the table-driven parameter matrix.
- [ ] 3.3 Remove remaining silent no-op parameter paths or replace them with explicit syntax errors; verify no command accepts unsupported parameters silently.

## 4. Integration and documentation

- [ ] 4.1 Add CLI/integration negative cases for string-where-numeric parameters and unsupported forms.
- [ ] 4.2 Update the affected specs and user-facing documentation with the strict type policy and exceptions; verify `openspec validate --change harden-cpp-parameter-validation-tier3`.
- [ ] 4.3 Run `make test-unit`, `make test-integration`, and `make test-architecture`; record results.

## Workflow follow-up

- Archive the change after review.
