# Proposal

## Why

The compiler accepts several invalid or inconsistent command/function parameters and silently emits wrong code: `CMD PLYSOUND` loses its third argument, `PLYSONG` rejects the documented optional subsong, `FRE()` accepts a missing argument, `CLOSE` does not emit the documented close-all value, `RANDOMIZE` ignores an unsupported argument, and several literal ranges are unchecked. These are compile-time contract bugs that are cheap to fix and prevent wrong ROM behavior before it reaches the kernel.

## What Changes

- Fix the Arkos command ABI:
  - `CMD PLYSOUND <effect>[, <channel>[, <volume>]]`: emit the third argument from `actions[2]`, default channel to `2`, default inverted volume to `0`, and reject more than three arguments.
  - `CMD PLYLOAD <AKM>[, <AKX>]`: keep the documented optional second resource; when omitted, emit the first resource number in `ARG` (current behavior becomes an explicit tested contract) and reject more than two arguments.
  - `CMD PLYSONG [subsong]`: accept zero or one argument, defaulting the subsong to `0`; reject more than one.
  - `CMD PLYLOOP`: accept a literal `0` or `1`.
- Fix `CLOSE` without parameters to emit `A=0xFF`, matching the documented close-all behavior.
- Require exactly one argument for `FRE()`; update compiler error handling, tests, and programs that currently use bare `FRE()`. **BREAKING** for sources using `FRE()` without an argument.
- Reject `RANDOMIZE` with an argument; only the parameterless form is supported.
- Reject `COLOR SPRITE$` when the value is not a string; reject `COLOR SPRITE` when the first parameter is empty.
- Validate literal directions for `SET TILE FLIP/ROTATE` and `SET SPRITE FLIP/ROTATE`, including signed literals such as `-1`.
- Validate literal ranges documented by the wiki: `CMD TURBO 0/1`, `CMD PAGE mode 0/1` with delays `0..15`, `CMD CLIP 0/1`, `CMD PLAY channel 0/1`, `SET FONT style 0..3` and bank `0..2` or all, `PLYLOOP 0/1`, and non-negative tile banks.
- Reject extra parameters for no-argument statements and commands such as `BEEP`, `CLS`, `END`, `PLYMUTE`, `PLYPLAY`, `PLYREPLAY`, `SCREEN ON/OFF`, and `SET TILE ON/OFF`.
- Cap function calls at four arguments before writing the compiler's fixed result buffer, returning a clear syntax error instead of invoking undefined behavior.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `compiler`: add compile-time parameter contracts for Arkos commands, `FRE`, `RANDOMIZE`, literal ranges, function arity, and parameter counts.
- `parser`: reject trailing tokens for statements that have no arguments instead of discarding them.
- `sprites-handling`: require a string value for `COLOR SPRITE$` and validate literal sprite flip/rotate directions.
- `resource-functions`: document that `FRE` requires exactly one argument and that bare `FRE()` is rejected.

## Impact

- C++ parser/compiler strategies under `src/application/parser` and `src/application/compiler`.
- Existing unit tests and integration programs that use `FRE()` without an argument.
- No kernel ABI change: default channel/volume and literal validation are emitted by the compiler.
- **BREAKING** for programs that relied on silently ignored parameters, bare `FRE()`, numeric `COLOR SPRITE$`, or out-of-range literal directions.
