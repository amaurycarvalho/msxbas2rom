# Design

## Context

See `proposal.md` and `openspec/changes/spike-turbo-r-r800-fragility/spike-report.md`. The spike reproduced `TURBO2 AFTER=2` in three identical openMSX runs on `Panasonic_FS-A1GT`. The RAM canary survived, but mode 2 reserves the last four mapper pages and conflicts with the runtime's memory model, so it must never be selected.

## Goals / Non-Goals

**Goals:**

- Clamp `CMD TURBO` on turbo R to modes 0 and 1.
- Keep `TURBO()` from ever reporting 2 after `CMD TURBO 2`.
- Add a permanent, stable turbo R regression program for the clamp.
- Document R800 DRAM mode as unsupported.

**Non-Goals:**

- Fixing unconfirmed PSG, MTF, mouse, or graphics candidates.
- Supporting R800 DRAM mode.
- Changing turbo behavior on MSX2+ machines.

## Decisions

### D1: Clamp before `CHGCPU`

In the turbo R branch of the `CMD TURBO` handler, reject or normalize values other than 0 and 1 before calling `CHGCPU`. Clamping to a supported mode is preferred over forwarding an unsupported value. The exact normalized value (0 or 1) is chosen when implementing, but `TURBO()` must never observe 2.

### D2: Preserve MSX2+ behavior

The MSX2+ turbo path uses port `0x41` and is unrelated to the R800 DRAM mode. Keep that path unchanged.

### D3: Assert stable markers, not timing

The permanent regression program checks `TURBO()` and a RAM canary, then prints stable text markers. It is volatile and does not compare RAM snapshots. It runs in isolation so a hang cannot mask the other MISC diagnostics.

### D4: Promote the spike diagnostic

Copy/rename the spike's `turbo_spike_turbo2.bas` behavior into `MISC/turbo_mode_clamp.bas`. Before the fix it reports `TURBO2 AFTER=BAD`; after the fix it must report `TURBO2 AFTER=OK`, `TURBO2 CANARY=OK`, and `TURBO2 DONE`.

## Risks / Trade-offs

- **Choosing a clamp target** -> Prefer the safest supported mode and document it; the spec only requires that mode 2 is never selected.
- **Regression before the fix** -> The new program intentionally fails until the clamp lands; do not wire it into a release gate before the fix.
- **Kernel size** -> The clamp is a small branch in an existing handler; verify the kernel size gate after the rebuild.

## Migration Plan

1. Implement the clamp in `31-cmd-game-graphics.asm`.
2. Regenerate `header.h`, `generated_kernel_image.cpp`, and the kernel manifest.
3. Build `MISC/turbo_mode_clamp.bas` and run it on `Panasonic_FS-A1GT`.
4. Run the MISC regression scenario and review the turbo R baseline/probe.
5. Roll back by reverting the ASM change and regenerating the kernel artifacts.

## Open Questions

- Should an out-of-range argument clamp to 0, clamp to 1, or leave the current CPU mode unchanged? The spec only requires that mode 2 is never entered.
