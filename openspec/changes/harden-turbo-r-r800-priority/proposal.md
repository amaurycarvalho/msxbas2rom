# Proposal

## Why

The `spike-turbo-r-r800-fragility` spike reproduced one confirmed R800/turbo R breakage: `CMD TURBO 2` enters R800 DRAM mode and `TURBO()` reports 2, which the runtime does not support. The other static-review candidates either passed in openMSX or still need hardware evidence, so only this breakage belongs in the priority change.

## What Changes

- Clamp `CMD TURBO` on turbo R to Z80 mode (0) and R800 ROM mode (1) before calling `CHGCPU`; values outside 0/1 SHALL NOT enter R800 DRAM mode.
- Keep the existing turbo R RAM canary behavior intact and verify `TURBO()` never reports 2 after `CMD TURBO 2`.
- Promote the spike's `CMD TURBO 2` diagnostic into the permanent `MISC/turbo_mode_clamp` regression program with `requires: turboR`, `volatile: true`, and stable text expectations.
- Document R800 DRAM mode as unsupported.

## Capabilities

### New Capabilities
- `turbo-r-compatibility`: behavior of the compiled ROM and Z80 kernel when an MSX turbo R runs in Z80 mode or R800 ROM mode, starting with explicit CPU-mode selection.

### Modified Capabilities
- (none)

## Impact

- Kernel ASM: `src/infrastructure/kernel/asm/src/header/31-cmd-game-graphics.asm`.
- Generated kernel artifacts (`header.h`, `generated_kernel_image.cpp`) after the kernel rebuild.
- Tests: `tests/integration/MISC/turbo_mode_clamp.bas` and `tests/integration/regression/manifest.json`.
- Specs: `openspec/specs/turbo-r-compatibility/spec.md` (new).
- Prerequisite: `spike-turbo-r-r800-fragility` (the spike report is the evidence source).
