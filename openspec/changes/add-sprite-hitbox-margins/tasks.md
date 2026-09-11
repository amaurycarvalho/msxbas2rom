## 1. Lexer — keywords

- [x] 1.1 Add `HITBOX` keyword to `src/domain/lexer/lexeme.cpp` keyword list (alphabetical order)
- [x] 1.2 Add `AUTO` keyword to `src/domain/lexer/lexeme.cpp` keyword list (alphabetical order)

## 2. Parser — SET SPRITE HITBOX

- [x] 2.1 Extend `parseSetSprite` (`set_statement_strategy.cpp`) to route `HITBOX` to a new parser function
- [x] 2.2 Implement `parseSetSpriteHitbox` accepting `ON`/`OFF`/`AUTO` (0 params), `<sprite>` (1 param), `<sprite> ON/OFF/AUTO` (2 params), and `<sprite>,<l>,<t>,<r>,<b>` (2..6 params)
- [x] 2.3 Add parser unit tests in `tests/unit/src/test_parser.cpp` for all forms (global ON/OFF/AUTO, bare sprite, per-sprite ON/OFF/AUTO, margins) and invalid parameter counts

## 3. Compiler — SET SPRITE HITBOX

- [x] 3.1 Add `DISP_set_sprite_hitbox` constant to `compiler_hooks.h`
- [x] 3.2 Implement `SET SPRITE HITBOX` emission in `compiler_set_statement_strategy.cpp` (mode byte in A; sprite in L; margins pushed onto stack; omitted margins default to zero)
- [x] 3.3 Add compiler unit tests in `tests/unit/src/test_compiler.cpp` for all forms (global ON/OFF/AUTO, bare sprite, per-sprite ON/OFF/AUTO, margins)

## 4. Kernel — HITBOX_TABLE allocation and symbols

- [x] 4.1 Allocate `HITBOX_TABLE` (32 x 5 bytes = 160 bytes) in `00_constants_and_workarea.asm`; shift `SPRSIZ`/`HEAPEND` accordingly
- [x] 4.2 Add `set_sprite_hitbox` dispatch entry to `20_runtime.asm`
- [x] 4.3 Regenerate `header.symbols.asm` with new symbols

## 5. Kernel — set_sprite_hitbox routine

- [x] 5.1 Implement `set_sprite_hitbox` in `31_cmd.asm` for modes ON (0), OFF (1), single-sprite ON (3), margins (4), single-sprite OFF (5), and single-sprite AUTO (6)
- [x] 5.2 Implement margin validation/clamping (0..SPRSIZ, LEFT+RIGHT <= SPRSIZ, TOP+BOTTOM <= SPRSIZ) and conversion to relative bounds (X1 = SPRSIZ - RIGHT, Y1 = SPRSIZ - BOTTOM)
- [x] 5.3 Implement AUTO (modes 2 and 6): read sprite pattern via `gfxCALPAT`, compute visible-pixel bounding box per sprite (or the single specified sprite), disable fully transparent sprites

## 6. Kernel — collision hot path

- [x] 6.1 Rewrite `SUB_SPRCOL_LOAD` to read position + relative bounds and produce effective absolute bounds for sprite A
- [x] 6.2 Rewrite `SUB_SPRCOL_CHECK` (and its skip ladder) to combine candidate position with relative bounds and check the per-sprite enabled flag (skip disabled candidates)
- [x] 6.3 Maintain parallel `HL' -> HITBOX_TABLE` pointer advancing in lockstep with `HL -> SPRTBL` (5-byte stride)

## 7. Kernel — reset on sprite clear

- [x] 7.1 Reset `HITBOX_TABLE` to default (zero margins, enabled) inside `SUB_CLRSPR`

## 8. Tests — integration

- [x] 8.1 Add integration test in `tests/integration/GRAPH/` covering ON/OFF/AUTO/single-sprite ON/OFF/AUTO/margins, collision disappearance, movement persistence, and sprite-size reset

## 9. Verification

- [x] 9.1 Build kernel and confirm it fits (kernel-size-check); run `make test-unit`
- [x] 9.2 Run lint/typecheck per project Makefile
