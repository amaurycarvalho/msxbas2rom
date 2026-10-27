# Design

## Context

See `proposal.md` for motivation. The compiler evaluates command parameters into an `ActionNode` tree and each statement strategy emits Z80 code directly. Several handlers accept optional parameters, and the Arkos handlers share the `DAC`/`ARG` work-area convention. The parser has a no-op strategy for parameterless statements.

## Goals / Non-Goals

**Goals:**
- Make the Arkos command ABI explicit and testable.
- Enforce optional/default values at compile time with minimal code changes.
- Reject invalid literals and unsupported arity before code emission.
- Keep the existing kernel ABI unchanged.

**Non-Goals:**
- Broad strict typing of all command parameters (Tier 3).
- Runtime validation of variable values (Z80 changes).
- MTF window/header hardening (C++ Tier 2).

## Decisions

### Decision 1: Fix Arkos defaults in the compiler handlers

`PLYSOUND` will emit channel `2` and inverted volume `0` for omitted arguments; the third argument will be read from `actions[2]`. `PLYLOAD` keeps the current one-argument behavior and documents that the omitted second parameter equals the first. `PLYSONG` accepts zero or one argument and emits `0` when omitted.

Alternative: change the kernel ABI to infer defaults. Rejected because the kernel contract is already register/work-area based and the compiler owns defaults.

### Decision 2: FRE requires one argument

The `FRE` strategy will require `parmCount == 1`. Existing integration programs using bare `FRE()` will be updated to `FRE(0)`, and the `resource-functions`/`compiler` wording will use the one-argument form.

Alternative: keep accepting zero arguments for compatibility. Rejected because the requested language contract requires the argument and silent acceptance hides errors.

### Decision 3: Signed literal helper

A helper will consume a direct numeric literal or a unary `+`/`-` over a numeric literal. It will be used by tile/sprite direction checks and literal range checks. Computed expressions remain runtime values.

Alternative: evaluate constant expressions. Rejected for Tier 1 because it adds a constant-folding dependency; direct literals and signed literals cover the documented compile-time cases.

### Decision 4: Reject extras instead of ignoring

Argument-less statements will stop discarding trailing tokens. `RANDOMIZE` with an argument will fail because only the parameterless form is documented.

Alternative: warn and ignore. Rejected because silent wrong code is the problem being fixed.

## Risks / Trade-offs

- **BREAKING** for sources using bare `FRE()`, numeric `COLOR SPRITE$`, out-of-range literals, or ignored trailing parameters → document in the proposal/spec and update integration fixtures.
- Signed literal validation only covers literal forms; computed directions still reach the kernel → covered by the Z80 Tier 1/Tier 2 guard changes.
- Changing Arkos defaults can alter audio behavior for previously accepted programs → add exact-byte tests for default channel/volume.

## Migration Plan

1. Add the helpers and handler fixes.
2. Update unit tests and integration programs.
3. Run unit and CLI characterization tests.
4. Roll back by reverting the compiler handlers; no persistent data migration.

## Open Questions

None for Tier 1.
