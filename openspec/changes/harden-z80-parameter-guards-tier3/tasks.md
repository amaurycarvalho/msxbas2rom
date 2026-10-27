# Tasks

## 1. Shared guard layer

- [ ] 1.1 Add reusable assembly helpers/macros for table index normalization, resource open-or-fail, resource size checks, and mode/channel normalization; verify each helper with focused emulator/probe tests.
- [ ] 1.2 Define and document the uniform failure policy (no-op, clamp, or skip) per parameter class.

## 2. Handler migration

- [ ] 2.1 Migrate all Tier 1 and Tier 2 guarded handlers to the shared helpers; verify equivalent behavior in the existing canary/regression probes.
- [ ] 2.2 Audit remaining command handlers for unguarded fixed-table/resource/mode parameters and migrate or explicitly document exceptions.

## 3. Malformed-resource regression

- [ ] 3.1 Add fixtures for truncated/zero MTF maps, truncated `.spr`, short fonts, invalid RUNASM, and missing resources.
- [ ] 3.2 Add volatile manifest scenarios with bounded fallback times and RAM/VRAM canary probes; verify hangs and canary changes fail the scenario.

## 4. Size and performance

- [ ] 4.1 Run the kernel size/placement checks and fix fixed-page overflow by compacting helpers or relocating non-fixed modules; verify `make test-kernel`.
- [ ] 4.2 Run the graphics performance check and verify the guard layer stays within the established budget.

## 5. Documentation and verification

- [ ] 5.1 Document the shared guard policy and malformed-input contract in the kernel specs and user-facing documentation; verify `openspec validate --change harden-z80-parameter-guards-tier3`.
- [ ] 5.2 Run the full unit, kernel, integration, and regression suites; record results.

## Workflow follow-up

- Archive the change after review.
