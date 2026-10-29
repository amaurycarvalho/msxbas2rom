# Tasks

## 1. Hardware evidence

- [ ] 1.1 Recreate the spike PSG timing probe (`psg_probe.asm`/`psg_probe.bin` and a BASIC caller) and run it on a real MSX turbo R; record machine model, BIOS version, and read-back values for Z80 and R800 modes.
- [ ] 1.2 Recreate the spike ARKOS player diagnostic on the same machine; record audio/register evidence for dropped PSG writes.
- [ ] 1.3 Add a dedicated partial-MTF probe that copies a known 16x4 window and compares VRAM rows across CPU modes; verify it compiles and runs.
- [ ] 1.4 Recreate the spike mouse diagnostic and exercise PSG reads with movement and button input; compare `PAD(12..14)` across CPU modes.

## 2. Optional fixes

- [ ] 2.1 Add an R800-aware delay to direct PSG/ARKOS writes only if hardware evidence shows dropped writes; verify the probe reads back the written values with turbo on.
- [ ] 2.2 Fix the partial-MTF path only if the new probe shows corruption; verify the known window matches in both modes.
- [ ] 2.3 Replace the remaining observable `SLL` use with `SLA` and correct the R800 detection comment; verify the kernel assembles and math results are identical across modes.
- [ ] 2.4 Regenerate affected kernel artifacts and verify the kernel size check passes.

## 3. Regression and documentation

- [ ] 3.1 Promote the optional probes that become stable into `tests/integration/MISC/` with `requires: turboR`, `volatile: true`, and stable text expectations; verify `make -C tests/integration regression-list`.
- [ ] 3.2 Generate baselines for the promoted programs and review the recorded machine.
- [ ] 3.3 Document openMSX PSG fidelity limits and which rows require hardware confirmation; verify the documented commands are accurate.
- [ ] 3.4 Run `make test-integration` and `make test-architecture`; verify all checks pass.

## Workflow follow-up

- Apply after `harden-turbo-r-r800-priority`.
- Leave rows `not reproduced` when no hardware evidence is available; do not add speculative delays.
- Archive after the hardware evidence and optional fixes are reviewed.
