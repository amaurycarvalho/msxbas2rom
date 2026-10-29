# Tasks

## 1. Turbo mode clamp

- [ ] 1.1 Clamp `CMD TURBO` on turbo R to modes 0 and 1 before calling `CHGCPU`; verify `TURBO()` never reports 2 after `CMD TURBO 2`.
- [ ] 1.2 Keep the RAM canary and mapper state intact across `CMD TURBO 2`; verify the canary check still passes.
- [ ] 1.3 Regenerate `header.h`, `generated_kernel_image.cpp`, and the kernel manifest; verify the kernel size check passes.

## 2. Regression coverage

- [ ] 2.1 Add `tests/integration/MISC/turbo_mode_clamp.bas` from the spike diagnostic, expecting `TURBO2 AFTER=OK`, `TURBO2 CANARY=OK`, and `TURBO2 DONE`.
- [ ] 2.2 Register `MISC/turbo_mode_clamp` in `tests/integration/regression/manifest.json` with `requires: turboR`, `fallback_seconds: 30`, `volatile: true`, and the stable text expectations; verify `make -C tests/integration regression-list` selects `Panasonic_FS-A1GT`.
- [ ] 2.3 Generate the baseline with `make -C tests/integration regression-baseline SCENARIO=MISC PROGRAM=turbo_mode_clamp` and review the recorded machine.
- [ ] 2.4 Run `make -C tests/integration regression SCENARIO=MISC PROGRAM=turbo_mode_clamp` and verify it passes with turbo mode 2 rejected.

## 3. Documentation and verification

- [ ] 3.1 Document R800 DRAM mode as unsupported and state the supported modes 0/1; verify the documented commands are accurate.
- [ ] 3.2 Run `make test-integration` and `make test-architecture`; verify all checks pass.
- [ ] 3.3 Review the kernel size report and verify fixed addresses, including the ARKOS player, are unchanged.

## Workflow follow-up

- Do not apply this change before the spike report exists.
- Archive after the clamp, regression program, and documentation are reviewed.
