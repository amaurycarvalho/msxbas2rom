# Design

## Context

The tile command family is implemented in three layers:

- The parser builds a generic action list for the command parameters and does not know the command grammar.
- The compiler in `compiler_set_statement_strategy.cpp` and `compiler_get_statement_strategy.cpp` decides the syntax form from the parameter count and action types.
- The Z80 routines in `31-cmd-tile-sprite.asm` receive the bank in `B` and compute VRAM addresses by adding `0x0800` per bank.

The current compiler uses the second parameter type to distinguish tile color buffers from scalar colors. Because every identifier is treated as a buffer, scalar variables in the FC/BC positions are silently routed to the buffer routines. The GET commands likewise accept any expression as a destination. The kernel only treats exactly `3` as "all" in the SET routines, and the GET routines have no guard at all.

See `proposal.md` for the motivation and `specs/tile-commands/spec.md` for the required behavior.

## Goals / Non-Goals

**Goals:**

- Make the bank rule uniform and safe in both the compiler and the kernel.
- Disambiguate buffer arrays from scalar color and bank expressions using declared-array information.
- Reject parameter shapes that cannot be represented correctly.
- Keep the fast compile-only unit tests and add emulator VRAM probes for runtime behavior.

**Non-Goals:**

- Redesigning the tile VRAM layout or SCREEN 4 paging.
- Fixing SCREEN 1 color-table addressing (the color table is 32 bytes in SCREEN 1, while the current color routines assume 8 bytes per tile). This change only prevents invalid bank arithmetic.
- Changing the existing partial-row behavior of tuples with fewer than eight entries.
- Adding a general-purpose expression range checker for every numeric command.

## Decisions

### Decision 1: Normalize SET banks in the kernel with `jr nc`

In `set_tile.paste`, `set_tile_pattern`, `set_tile_color_buf`, and `set_tile_pattern_buffer`, replace:

```z80
cp 3
jr z, <all_label>
```

with:

```z80
cp 3
jr nc, <all_label>
```

After `cp 3`, carry is set only when `B < 3`. Therefore `B=0,1,2` keeps the single-bank path and `B=3..255` takes the all-banks path. The change is byte-for-byte the same size as the current instruction.

Alternative considered: normalize in the compiler only. Rejected because the kernel is the stable ABI and must also protect callers that pass a variable bank, and because `B=3` already has this meaning in the documented contract.

### Decision 2: Normalize GET banks to bank 0

`get_tile_pattern` and `get_tile_color` cannot represent "all", so before their bank loop they clamp the requested bank:

```z80
ld a, b
cp 3
jr c, .bank_valid
xor a              ; bank >= 3 -> bank 0
.bank_valid:
```

The existing zero-bank fast path remains. Bank 0 was chosen over bank 2 or a no-op because it is the documented default for GET and is the least surprising deterministic result.

Alternative considered: clamp to bank 2. Rejected because "all" starts at bank 0 and a caller that passes the all sentinel to a GET is more likely to expect the default bank. Alternative considered: no-op. Rejected because it leaves the caller buffer in an undefined state.

### Decision 3: Single-bank screen modes

For `SCRMOD < 2`, the pattern routines force `B=0` before the bank loops, and `set_tile.paste` writes only the base address. This extends the existing SCREEN 0 handling to SCREEN 1 and prevents a bank request from writing into the name or color tables.

`SET TILE COLOR` already ignores the bank in SCREEN 1 and is a no-op in SCREEN 0, so no color-routine change is required for this decision.

### Decision 4: Identify buffers through the declared-array flag

Add a compiler helper that returns the underlying identifier only when the action is one of:

- an identifier whose lexeme has `isArray` set by `DIM`, or
- an `ARRAY` action with exactly one child identifier whose lexeme has `isArray` set.

Use this helper for the buffer form of `SET TILE PATTERN`, `SET TILE COLOR`, `GET TILE PATTERN`, and `GET TILE COLOR`. An action that is not a declared array is not a buffer.

This fixes the `SET TILE COLOR 5, F%` and `SET TILE COLOR 5, F%, G%` misclassification while keeping `C%()` and `C%` working when `C%` was declared with `DIM`.

Alternative considered: require parentheses for every buffer. Rejected because the parser can represent `C%()` in more than one action shape, and the declared-array flag is the information that actually distinguishes a scalar from an array. If `isArray` is not set on a reference in some construction, the helper falls back to the compiler symbol resolver and reports a syntax error rather than guessing.

### Decision 5: Scalar bank parameters and literal normalization

A bank parameter must be a scalar expression. The compiler rejects an `ARRAY` action or a declared buffer in the bank position. A scalar identifier is accepted and is used as the bank.

When the bank parameter is a numeric literal whose value is greater than 2, the compiler emits bank 3 instead of the literal value. This covers 16-bit literals such as `256` and `300` that would otherwise be truncated to a low byte. Runtime expressions still reach the kernel as 8-bit bank values; values `3..255` mean all and values above 255 are truncated by the existing `LD B`/`LD H` ABI. That limitation is documented rather than solved with extra runtime clamping code.

Alternative considered: emit a runtime 16-bit clamp for every bank expression. Rejected for code size, because the kernel already handles the complete 8-bit range and tile banks are conventionally small.

### Decision 5a: Preserve color operands when a bank parameter is present

The current `SET TILE COLOR` implementation reuses the same `sub_action` variable for the bank expression. In the `t=3` FC-array-with-bank and `t=4` scalar-or-array forms this overwrites the FC/BC operand reference, so the bank value is used as color data or the FC array loop is skipped entirely. The fix is to keep the color operand actions in separate local variables from the bank action and to build the bank only after the color operands have been identified.

This is observable: `SET TILE COLOR 66, 2, 8, 1` currently writes `0x18` instead of `0x28`, and `SET TILE COLOR 69, (8,7,6,5,4,3,2,1), 2` writes nothing to tile 69. The deterministic `test97` and `tile_bank_all` probes assert the corrected values.

### Decision 6: Arity and row-count validation

- Inline `SET TILE PATTERN` accepts only the tile, the tuple, and an optional bank. The current `if (t <= 3)` block gets an explicit error path for `t > 3`.
- Pattern tuples and per-row color arrays are capped at eight entries. Fewer entries remain valid and keep the existing partial-update behavior. `null` entries keep their existing skip behavior.
- BC arrays use the same eight-row cap.

### Decision 7: Mask color nibbles

The compiler masks every foreground and background value to four bits before packing. Foreground values are already effectively masked by the existing shift-and-mask sequence; add `and 0x0F` to the background value in both the scalar and per-row array paths before the `or`.

Alternative considered: mask in the kernel. Rejected because the kernel receives an already packed `FC<<4|BC` byte and can no longer tell which nibble was invalid.

### Decision 8: Validate literal flip/rotate directions

When the direction parameter is a numeric literal, `SET TILE FLIP` and `SET TILE ROTATE` reject values outside `0..2`. Non-literal direction expressions keep the kernel's current fallback behavior. This avoids changing the ABI while catching the common typo case.

### Decision 9: Deterministic probes on affected integration programs

The affected integration programs are `test87`, `test88`, `test94`, `test97`, and `tile_flip_rotate`; `base2` and `test93` only use `SET TILE ON` and do not exercise the changed paths. Every program that exercises the changed behavior is marked `volatile: true` and receives deterministic `vram.probes`, so the runner validates exact bytes without a baseline.

- `test87` keeps its SCREEN 0/1/2 sequence but removes blocking `INPUT$` waits and ends in SCREEN 1 with a deterministic tile 72 pattern/color section. Probes assert that the final pattern write touches bank 0 only while banks 1 and 2 remain zero, covering the single-bank SCREEN 1 rule.
- `test88` removes its waits, stays in SCREEN 2, and probes tile 65 pattern/color in all three banks for the default bank=3 path.
- `test97` removes its waits, stays in SCREEN 2, and probes every color form: FC/BC default all, bank 1 only, FC array, FC+BC arrays, FC array bank 2, buffer all, and buffer bank 1.
- `tile_bank_all` is the dedicated bank-boundary program. It probes banks 3, 9, and 200 for inline pattern and scalar color; bank 9 and 200 for FLIP/ROTATE; and GET with bank 3 and 200 followed by writes that prove the reads came from bank 0.
- `test94` and `tile_flip_rotate` remain in the GRAPH regression run; they are already volatile and do not need baselines.

### Decision 10: Verification strategy

- Compiler and parser tests cover emission, rejection, buffer/scalar classification, bank literals 3 and 200, and tuple caps.
- `make test-kernel` rebuilds the kernel and checks the symbol/filler layout after the assembly edits.
- The GRAPH regression run is the authoritative runtime check: `runner.py run --scenario GRAPH` executes the affected programs and evaluates their probes.

## Risks / Trade-offs

- **`isArray` may not be set on every reference shape** -> The helper falls back to the symbol resolver and then to a syntax error. Tests cover scalar, `C%()`, `(C%)`, and undeclared identifiers.
- **Rejecting previously accepted programs** -> The proposal marks the affected rejection paths as breaking; the error messages name the command so programs fail loudly instead of corrupting VRAM or RAM.
- **Kernel size growth** -> Use the same-size `jr nc` replacement where possible and keep the GET clamp small. `make test-kernel` validates the filler and relocation layout.
- **Literal bank normalization changes generated bytes** -> Existing tests only use banks 0, 1, and 2. New tests assert that literals above 2 emit bank 3.
- **Emulator regression timing** -> The regression program keeps running instead of returning to BASIC, and the manifest uses exact VRAM probes with `volatile: true`.
- **Partial tuples are still accepted** -> This is intentional and matches the existing tests; only tuples longer than eight rows are rejected.
- **SCREEN 1 color addressing remains separate** -> This change only protects bank selection. The SCREEN 1 color table stride is documented as a non-goal.
