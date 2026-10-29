# Proposal

## Why

The spike confirmed only `CMD TURBO 2` as broken. The remaining turbo R candidates either passed in openMSX without stress or still need hardware evidence: ARKOS/direct PSG timing, partial MTF windows, mouse PSG reads, and the remaining `SLL` use. These are hardening items, not blockers for the confirmed mode-selection fix.

## What Changes

- Validate direct PSG and ARKOS playback timing on a real MSX turbo R; add an R800-aware delay only where hardware evidence proves writes are dropped.
- Add a dedicated partial-MTF probe that copies a known window and compares VRAM in Z80 and R800 modes.
- Exercise mouse PSG reads with mouse movement and button input, then compare `PAD()` results across CPU modes.
- Replace the remaining observable `SLL` use with `SLA` and correct the R800 detection comment.
- Document openMSX fidelity limits and the hardware evidence required before closing each row.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- `turbo-r-compatibility`: adds slow-I/O, VDP block-transfer, and R800 instruction-parity requirements that are not implemented by the priority change.

## Impact

- Kernel ASM: ARKOS player, BIOS helpers, MTF, graphics, input, and math modules as hardware evidence dictates.
- Tests: additional turbo R regression programs and probes under `tests/integration/MISC/`.
- Documentation: turbo R limitations and hardware-validation notes.
- Dependency: apply after `harden-turbo-r-r800-priority`; the priority change owns the `turbo-r-compatibility` capability.
