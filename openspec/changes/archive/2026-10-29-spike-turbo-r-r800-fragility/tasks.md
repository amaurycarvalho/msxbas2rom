# Tasks

## 1. Diagnostic harness

- [x] 1.1 Create `tests/integration/MISC/turbo_spike_vdp_copy.bas` and a small `turbo_spike_pattern.bin` resource; verify the program compiles with `bin/Release/msxbas2rom -q -a` and prints a stable completion marker.
- [x] 1.2 Create `tests/integration/MISC/turbo_spike_vdp_graphics.bas` using `SCREEN 5`, `PSET`, `LINE`, `CIRCLE`, `SET SCROLL`, and `POINT`; verify it compiles and returns to `SCREEN 0`.
- [x] 1.3 Create `tests/integration/MISC/turbo_spike_player.bas` using `../ARKTRK/songs.akm` and `../ARKTRK/effects.akx`; verify `CMD PLYLOAD`, `CMD PLYSONG`, `CMD PLYPLAY`, and `CMD PLYMUTE` complete.
- [x] 1.4 Create `tests/integration/MISC/turbo_spike_pad.bas` calling `PAD(0)`, `PAD(12)`, `PAD(13)`, and `PAD(14)` with turbo off and on; verify it compiles.
- [x] 1.5 Create `tests/integration/MISC/turbo_spike_math.bas` covering multiply, divide, modulo, and `TURBO()` values with turbo off and on; verify it compiles.
- [x] 1.6 Create `tests/integration/MISC/turbo_spike_turbo2.bas` that runs `CMD TURBO 2` in isolation and checks `TURBO()` and a RAM canary; verify it compiles.
- [x] 1.7 Create `tests/integration/MISC/turbo_spike_psg_timing.bas` plus `psg_probe.asm`/`psg_probe.bin`; load the probe with `CMD RSCTORAM` and `DEF USR`, write PSG registers in a tight loop, read back through BIOS `RDPSG`, and verify the program compiles.
- [x] 1.8 Add temporary `MISC` manifest entries with `requires: turboR`, `volatile: true`, and stable text expectations; verify them with `make -C tests/integration regression-list`.

## 2. Execution and evidence

- [x] 2.1 Run each diagnostic on openMSX `Panasonic_FS-A1GT` with `make -C tests/integration regression SCENARIO=MISC PROGRAM=<name>`; verify each produces a run artifact under `tests/integration/regression/artifacts/runs/`.
- [x] 2.2 Repeat the runs on a real MSX turbo R when one is available; record machine model, BIOS version, and evidence paths in the report.
- [x] 2.3 Rerun every failing path at least three times; verify the failure is reproducible and capture RAM/VRAM differences or audio/register evidence.
- [x] 2.4 For each path, classify the result as `broken`, `not reproduced`, or `safe`; verify every candidate from the design table has a classification.

## 3. Spike report

- [x] 3.1 Write `openspec/changes/spike-turbo-r-r800-fragility/spike-report.md` with the breakage matrix, evidence links, suspected mechanism, and priority recommendation for each path; verify every candidate has a row.
- [x] 3.2 Add a short note on openMSX fidelity limits, especially for PSG sub-instruction timing, and state which rows require hardware confirmation.

## 4. Split the harden change

- [x] 4.1 Create `openspec new change "harden-turbo-r-r800-priority"` and `openspec new change "harden-turbo-r-r800-optional"`; verify both changes exist and name the paths that belong to each.
- [x] 4.2 Move the confirmed-breakage recommendations into `harden-turbo-r-r800-priority` and the remaining hardening into `harden-turbo-r-r800-optional`; verify `openspec validate` passes for both.
- [x] 4.3 Update `openspec/changes/harden-turbo-r-r800-compatibility/` proposal, design, and tasks to point at the two split changes and mark it superseded; verify its status still parses.
- [x] 4.4 Decide which diagnostic programs become permanent regression tests in the priority change; move or rename them and verify `make -C tests/integration build` still compiles every `.bas`.
- [x] 4.5 Remove temporary diagnostic programs or manifest entries that are not promoted; verify `make -C tests/integration regression-list` still resolves every remaining program.

## Workflow follow-up

- Archive the spike after the report and the split are reviewed.
- Keep the spike report as the reference for the priority and optional changes.
