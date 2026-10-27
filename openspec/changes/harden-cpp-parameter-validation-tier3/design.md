# Design

## Context

See `proposal.md`. Tiers 1 and 2 fix individual handlers. The compiler still has no single entry point for "evaluate a numeric command parameter" or "validate a fixed-table index", so new handlers can repeat implicit conversions and ad-hoc ranges.

## Goals / Non-Goals

**Goals:**
- Define one parameter-evaluation API and migrate the highest-risk handlers.
- Make fixed-table index behavior explicit and consistent.
- Preserve expression and `LET` conversion semantics.

**Non-Goals:**
- Reimplementing the expression evaluator.
- Runtime validation of variable values (Z80 changes).
- Changing the MSX BASIC language surface beyond rejecting documented type mismatches.

## Decisions

### Decision 1: Add parameter helper functions

Introduce helpers in the compiler semantic helper layer:
- numeric parameter: evaluate, reject `subtype_string`, cast numeric/float;
- string parameter: evaluate, reject numeric unless explicitly allowed;
- fixed-table index: evaluate, require numeric, enforce literal range, preserve sentinels.

Handlers call these helpers instead of open-coding `evalExpression` + `addCast`.

Alternative: enforce types inside `addCast`. Rejected because `addCast` is also used by expression operators and `LET`, where implicit conversion is valid.

### Decision 2: Migrate by risk, not all at once

Tier 3 migrates command families with known failures first: graphics coordinates, sprite/tile indices, MTF, Arkos, file numbers, and string buffers. Remaining handlers are migrated in follow-up changes if needed.

Alternative: rewrite every strategy in one change. Rejected because the compatibility surface is large and test coverage is uneven.

### Decision 3: Explicit sentinel registry

The fixed-table index helper receives a documented allowed sentinel (for example `-1` for `COLLISION`) rather than globally allowing negatives. This prevents a blanket range check from breaking `COLLISION(-1)`.

Alternative: special-case `COLLISION` in the helper. Rejected because future sentinels should be explicit at the call site.

## Risks / Trade-offs

- **BREAKING** for programs using string expressions where a numeric command parameter is required → document and add negative tests.
- Migration may reveal handlers that intentionally relied on implicit conversion → audit failures and document exceptions.
- Helper introduction touches many files → keep behavior changes isolated to parameter evaluation.

## Migration Plan

1. Add helpers and unit tests.
2. Migrate graphics/sprite/tile/MTF/Arkos/file handlers.
3. Run integration build and fix programs that depend on rejected forms.
4. Roll back by handler family if needed.

## Open Questions

None for the first migration set.
