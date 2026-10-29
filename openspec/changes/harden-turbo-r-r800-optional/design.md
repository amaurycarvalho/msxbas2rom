# Design

## Context

See `spike-report.md`. openMSX passed the PSG, player, mouse, and VDP paths, but the spike could not prove PSG sub-instruction timing, did not exercise a partial MTF window, and had no mouse input. Those rows are `not reproduced`, not `safe`.

## Goals / Non-Goals

**Goals:**

- Establish hardware evidence for the remaining turbo R candidates.
- Add only the fixes the hardware evidence requires.
- Keep optional hardening separate from the confirmed mode-selection fix.

**Non-Goals:**

- Supporting R800 DRAM mode.
- Replacing the spike harness with a CI gate before it is stable.
- Fixing candidates that hardware proves safe.

## Decisions

### D1: Hardware evidence gates PSG changes

Use the spike's `psg_probe.asm`/`turbo_spike_psg_timing.bas` pattern on a real turbo R. Record the machine model and BIOS version. Add an R800-aware delay only if read-back values differ between Z80 and R800 modes. Prefer the existing `ED 55` idiom or a one-time `SLL A`/`SLA A` CPU check.

### D2: ARKOS audio is validated separately

The player test asserts completion only. Add an audio capture or register probe on hardware before claiming the ARKOS path safe.

### D3: Add a dedicated partial-MTF probe

Load the existing MTF resources, copy a known 16x4 window, read the destination name-table rows back with `CMD VRAMTORAM`, and compare Z80/R800 results. Do not infer MTF safety from the VDP resource-copy result.

### D4: Mouse reads need input

Inject mouse movement and button events before sampling `PAD(12..14)`. Idle values are not evidence that the PSG read path is timing-safe.

### D5: Fix only observable `SLL` behavior

Replace the reviewed `SLL` use with `SLA` where the result is observable, and correct the comment describing the `SLL A`/`SLA A` CPU detection trick.

## Risks / Trade-offs

- **Hardware availability** -> Without a real turbo R, leave the rows `not reproduced` and do not add speculative delays.
- **PSG delay cost** -> A delay in the ARKOS player must fit the fixed layout and not disturb the timer interrupt; measure playback timing.
- **MTF probe fragility** -> Use a known window and stable row-by-row copies rather than a full-screen hash.
- **Test churn** -> Keep optional probes volatile and assert stable markers only.

## Migration Plan

1. Obtain hardware evidence for PSG, ARKOS, MTF, and mouse.
2. Implement only confirmed fixes, then regenerate affected kernel artifacts.
3. Add or promote the optional regression programs and baselines.
4. Run `make test-integration`, `make test-architecture`, and the turbo R scenario.
5. Roll back by reverting the optional kernel ASM and regenerating the artifacts.

## Open Questions

- Is a real MSX turbo R available, and which BIOS version does it run?
- Does openMSX audio capture suffice for the ARKOS row, or is hardware audio required?
- Which optional probes should become permanent regression tests?
