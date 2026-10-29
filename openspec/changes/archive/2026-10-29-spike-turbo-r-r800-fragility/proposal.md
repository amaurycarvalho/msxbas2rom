# Proposal

## Why

Static review identified candidate R800/turbo R failure points, but there is no evidence of which paths actually break. Fixing all candidates would be speculative; the project first needs a controlled spike that toggles turbo on a turbo R class machine, exercises each candidate path, and records what changes.

## What Changes

- Add a temporary diagnostic harness under `tests/integration/MISC/` (BASIC and, where needed, small ASM resources) that runs the same workload with `CMD TURBO 0` and `CMD TURBO 1`.
- Cover: VDP resource copy (`CMD WRTVRAM`/`VRAMTORAM`), VDP graphics/scroll (`PSET`/`LINE`/`CIRCLE`/`SET SCROLL`), ARKOS player/direct PSG, mouse PSG read (`PAD`), integer math, and `CMD TURBO 2`.
- Run on openMSX `Panasonic_FS-A1GT` and on real hardware when available; capture RAM/VRAM artifacts, screenshots, logs, and audio observations.
- Produce a breakage matrix/report under the change directory that states, per path, whether it breaks, the evidence, and the suspected mechanism.
- Final task: rewrite `harden-turbo-r-r800-compatibility` into two new changes: `harden-turbo-r-r800-priority` (confirmed breakage) and `harden-turbo-r-r800-optional` (remaining hardening), and mark the original change superseded.

## Capabilities

### New Capabilities
- (none; this spike is investigative and does not change durable behavior)

### Modified Capabilities
- (none)

## Impact

- Temporary diagnostics in `tests/integration/MISC/` and possibly `tests/integration/regression/manifest.json`.
- Change-local report and evidence under `openspec/changes/spike-turbo-r-r800-fragility/`.
- No production kernel or compiler behavior changes.
- The spike must complete before `harden-turbo-r-r800-compatibility` is applied.
