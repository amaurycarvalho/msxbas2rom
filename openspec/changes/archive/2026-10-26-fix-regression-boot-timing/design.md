# Design

## Context

See `proposal.md` - Why. The runner currently uses a single
`fallback_seconds` value (default 10) for all machines. Local probes showed
8-second captures could still be in boot, 10 seconds was borderline, and 15
seconds was reliable for `Panasonic_FS-A1WX` and `Panasonic_FS-A1GT`.

## Goals / Non-Goals

**Goals:**

- Prevent fallback captures from occurring before the machine has booted.
- Keep the behavior configurable per machine class and per program.
- Make the effective time visible in capture metadata.

**Non-Goals:**

- Detecting a program-specific "first output" signal.
- Replacing the terminal-state capture for programs that end early.
- Guaranteeing a fixed boot time for every BIOS/machine combination.

## Decisions

### D1: Add a machine boot-delay map

Add `machine_boot_seconds` defaults keyed by machine class:

- `msx1`: 10
- `msx2`: 15
- `msx2plus`: 15
- `turboR`: 15

The manifest may override individual entries.

### D2: Effective fallback is the maximum

The runner composes the program/scenario `fallback_seconds` as today, then
uses `max(fallback_seconds, machine_boot_seconds[class])`. A program that needs a
longer wait keeps its value; a program cannot accidentally capture before the
machine boot minimum.

### D3: Keep the fallback and terminal capture separate

This change only affects fallback captures. Programs that reach their terminal
breakpoint still capture at the pre-cleanup END entry from
`fix-regression-end-pgm-capture`.

### D4: Record the effective value

Write the effective fallback and machine class to the capture metadata/run
report so a baseline review can see why a capture happened at that emulated
time.

## Risks / Trade-offs

- **[Risk] Regression becomes slower** -> Only fallback captures are affected;
  programs that end early still capture immediately.
- **[Risk] 15 seconds is still empirical** -> Make it configurable and document
  that a slow local host or a different BIOS may need more.
- **[Risk] A program-specific shorter fallback is ignored** -> This is
  intentional to avoid pre-boot captures; document `machine_boot_seconds` as the
  override path.

## Migration Plan

- Additive defaults and schema fields; existing manifests remain valid.
- Re-run fallback-based baselines; terminal-state baselines are unaffected.
- Roll back by removing the machine boot-delay map.
