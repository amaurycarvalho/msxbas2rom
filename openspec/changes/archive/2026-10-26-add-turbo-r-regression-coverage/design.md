# Design

## Context

See `proposal.md` - Why. Current turbo-related programs:

- Feature tests: `MISC/test1` (`TURBO()`/`CMD TURBO`), `MISC/test64`
  (`MSX`/`VDP`/`TURBO`/timing), `MISC/test65` (`CMD TURBO` + timing).
- Performance/speed usage: `GRAPH/tree`, `GRAPH/mexhat`, `GRAPH/test78`,
  `IO/test48`, `MATH/cos`.

Their baselines currently record `Sharp_HB-8000_1.2` or `Panasonic_FS-A1WX`;
`MISC/test1`/`test64` baselines do not show turbo values. `TURBO()`, `VDP()`, and
`MAKER()` hang until `fix-turbo-function-runtime` lands.

## Goals / Non-Goals

**Goals:**

- Run the turbo R feature programs on the turbo R machine.
- Keep MSX2+ coverage for programs that only use turbo for speed.
- Make the expected turbo R output visible in regression checks.

**Non-Goals:**

- Classifying programs by heuristic `CMD TURBO` detection.
- Running every turbo-capable program on turbo R.
- Asserting exact elapsed-time values.

## Decisions

### D1: Explicit `requires: turboR` for feature tests

Add `requires: "turboR"` to `MISC/test1`, `MISC/test64`, and `MISC/test65`.
Do not infer turbo R from `CMD TURBO`/`CALL TURBO`/`TURBO()` because those are
valid on MSX2+ too.

### D2: Performance-only programs stay on MSX2+

`GRAPH/tree`, `GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, and `MATH/cos` remain
on `Panasonic_FS-A1WX` (MSX2+). They still exercise the MSX2+ turbo path and
preserve MSX2+ compatibility coverage.

### D3: Assert stable output, keep timing volatile

Add text expectations for `MSX = 3` and the `TURBO OFF/ON ELAPSED TIME` labels
for `MISC/test65`; mark timing/`TURBO()` numeric values volatile. `MISC/test1`
and `test64` require `fix-turbo-function-runtime` before they can complete.

### D4: Add a turbo R gate independent of TURBO()

Add a small program that checks `MSX() = 3` and executes `CMD TURBO 1`/`0`,
then prints a stable marker. This validates the turbo R machine and command path
even if `TURBO()` is temporarily unavailable.

## Risks / Trade-offs

- **[Risk] `MISC/test1`/`test64` hang until the USR2 fix lands** -> Make the
  dependency explicit and do not enable them in this change before the fix.
- **[Risk] Timing values are volatile** -> Assert labels and machine identity,
  not elapsed numbers.
- **[Risk] Baseline churn** -> `refresh-regression-baselines` handles the
  regenerated MISC baselines and review.
- **[Risk] `MSX()`/`VDP()` values are firmware-dependent** -> Assert `MSX = 3`
  on FS-A1GT and keep VDP output informational unless a stable value is known.

## Migration Plan

- Add manifest classifications and expectations after the dependencies land.
- Regenerate MISC baselines and verify the selected machine is FS-A1GT.
- Roll back by removing the `requires` entries and restoring the previous
  baselines.
