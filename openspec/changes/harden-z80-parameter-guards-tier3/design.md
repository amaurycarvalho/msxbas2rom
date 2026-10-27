# Design

## Context

See `proposal.md`. Tiers 1 and 2 add command-specific guards. The kernel is a fixed 16 KB page with overflow modules, so duplicated validation code is costly and inconsistent behavior is easy to reintroduce. Regression currently uses openMSX with RAM/VRAM probes and bounded fallback times.

## Goals / Non-Goals

**Goals:**
- One guard policy and reusable helpers/macros.
- Uniform, documented failure behavior.
- Malformed-resource regression coverage with bounded execution.
- Keep fixed-page size and hot-path performance within existing checks.

**Non-Goals:**
- Compiler-side literal validation (C++ changes).
- New user-facing commands.
- Rewriting the kernel module layout.

## Decisions

### Decision 1: Shared guard helpers/macros

Add small assembly helpers/macros for:
- sprite/table index normalization;
- resource open-or-fail;
- resource size minimum check;
- mode/channel bit normalization.

Handlers call the shared helper instead of duplicating instructions.

Alternative: keep per-command guards. Rejected because inconsistency and size duplication already caused the original bugs.

### Decision 2: Uniform failure policy

The policy is:
- fixed-table indices: normalize (mask) or no-op;
- directions: documented safe value/no-op;
- resources: close and return without using undefined registers;
- sizes: skip the operation;
- modes/channels: mask or reject.

The choice is recorded per parameter class in the spec.

Alternative: return error codes. Rejected because most commands have no error channel and MSX BASIC callers do not check one.

### Decision 3: Malformed-resource regression suite

Add volatile regression programs with canaries/probes and short fallback times. A hang fails the scenario; a canary change fails the probe.

Alternative: unit-test assembly in isolation. Rejected as the primary gate because the real risk is interaction with resource loading and VRAM/RAM layout; a small isolated harness can be added later.

### Decision 4: Size/performance budget

Run the existing kernel size check and graphics performance checks. If the shared guard layer exceeds the fixed page, move non-fixed modules or compact helpers before relaxing the guard.

Alternative: accept size growth. Rejected because fixed-page overflow is a hard build constraint.

## Risks / Trade-offs

- Shared helpers add call overhead in hot paths → inline macros for the smallest checks and measure.
- Masking invalid indices can hide upstream compiler bugs → compiler still rejects literals; runtime masking is the last line of defense.
- Regression fixtures with malformed binaries can be brittle → generate them from documented byte layouts and keep probes minimal.
- Uniform policy may differ from historical accidental behavior → document as bug fixes.

## Migration Plan

1. Introduce helpers/macros and convert Tier 1/Tier 2 call sites.
2. Add malformed-resource fixtures and manifest scenarios.
3. Run kernel size, unit, integration, and regression checks.
4. Roll back helper-by-helper if a size or compatibility problem appears.

## Open Questions

None that change the approach; exact macro versus call choices are implementation details guided by size measurements.
