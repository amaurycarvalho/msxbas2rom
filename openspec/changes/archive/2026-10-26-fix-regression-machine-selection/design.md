# Design

## Context

See `proposal.md` - Why. Relevant current behavior:

- `screen_modes()` at `runner.py:194` matches `SCREEN\s+(\d+)`, so `SCREEN8` is
  missed.
- `machine_for()` at `runner.py:200` uses `maximum >= 5`.
- `screen_from_vdp()` at `runner.py:384` only understands MSX1 register pairs.
- `baseline["machine"]` is written but never compared.
- `Panasonic_FS-A1WX` is an MSX2+ machine with V9958; `Panasonic_FS-A1GT` is an
  MSXturboR machine with V9958.

## Goals / Non-Goals

**Goals:**

- Choose MSX2/MSX2+ for all V9958-capable programs, including SCREEN 4.
- Keep turbo R explicit; do not infer it from turbo-capable commands.
- Decode the modes the runner observes into the correct SCREEN number.
- Make stale baseline machines fail loudly.

**Non-Goals:**

- Selecting a pure V9938 MSX2 machine; MSX2 and MSX2+ intentionally share
  `Panasonic_FS-A1WX`.
- Guessing turbo R from source text when no explicit requirement exists.
- Fixing capture timing or boot delays; those are separate changes.

## Decisions

### D1: Least-capable machine classes

Use `msx1`, `msx2`, `msx2plus`, and `turboR`. Defaults are
`Sharp_HB-8000_1.2`, `Panasonic_FS-A1WX`, `Panasonic_FS-A1WX`, and
`Panasonic_FS-A1GT`. MSX2 and MSX2+ share the FS-A1WX because it is the project's
chosen MSX2-family regression machine.

### D2: Precedence and inference

Order: explicit `machine` > explicit `requires` > inference. Inference chooses
`msx2plus` when the highest literal `SCREEN` is at least 4 or when a
turbo-capable command is present; otherwise `msx1`. Turbo R is never inferred.

### D3: SCREEN parsing

Use `\bSCREEN\s*(\d+)\b` so `SCREEN8` is found. Keep the current heuristic that
ignores non-literal screen expressions; programs with dynamic modes can declare
`requires` or `machine` explicitly.

### D4: V9958 mode table

Use the empirically verified register pairs from `Panasonic_FS-A1WX`:

| SCREEN | R0 | R1 |
| --- | --- | --- |
| 0 | 00 | 70 |
| 1 | 00 | 60 |
| 2 | 02 | 60 |
| 3 | 00 | 68 |
| 4 | 04 | 60 |
| 5 | 06 | 60 |
| 6 | 08 | 60 |
| 7 | 0A | 60 |
| 8 | 0E | 60 |
| 9-12 | 0E | 60 |

Mask `R0 & 0x0E` and `R1 & 0x18` to ignore sprite-size, interrupt, and blank
bits. For `0x0E`, return the source's last SCREEN when it is 9-12; otherwise
return 8. Preserve the existing MSX1 results for the register pairs the current
tests use.

### D5: Baseline machine consistency

Load the baseline for non-volatile programs as today, then compare
`baseline["machine"]` with the selected machine before snapshot and VRAM
comparison. On mismatch, fail with a regenerate instruction. This converts the
baseline churn caused by this change into explicit, actionable failures.

### D6: Audit visibility

Extend `runner.py list` with machine class/machine output (or `--json`). The run
report includes the selected machine. This lets an operator verify that no
MSX2+ program selects MSX1 and that turbo R programs select FS-A1GT.

## Risks / Trade-offs

- **[Risk] Baseline churn** -> `refresh-regression-baselines` handles the
  regeneration and review after this change and its dependents.
- **[Risk] `line1` capture is inconsistent** -> The screen decoder change may
  expose the END-capture problem; `fix-regression-end-pgm-capture` handles it.
  Do not silently fall back to the source mode for all END captures.
- **[Risk] FS-A1WX is not a pure MSX2** -> Document that MSX2 coverage uses
  MSX2+ hardware by design; a V9938 machine can be added later.
- **[Risk] Regex matches comments/strings** -> Keep the heuristic, document it,
  and prefer explicit `requires` for ambiguous programs.

## Migration Plan

- Add defaults and schema fields; existing manifests remain valid.
- Regenerate affected baselines after the dependent fixes.
- Roll back by restoring the old `machine_for`/decoder and keeping the schema
  additive.
