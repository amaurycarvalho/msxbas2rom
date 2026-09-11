## Context

The kernel maintains a software sprite table `SPRTBL` (`00_constants_and_workarea.asm:460`, `SPRTBL equ PLYBUF - (32*5)`), one 5-byte entry per sprite:

```
+0  active (0=off, 0xFF=on)
+1  x0
+2  x1   (x0 + SPRSIZ, absolute)
+3  y0
+4  y1   (y0 + SPRSIZ, absolute)
```

`SPRSIZ` (`SPRTBL - 1`) holds the current sprite size (8 or 16), derived from `RG1SAV` by `SUB_GetSpriteSize` (`60_bios_helpers.asm:859`) and stored by `SUB_CLRSPR`. Collision detection lives in `SUB_SPRCOL_ALL/ONE/COUPLE/CHECK` (`60_bios_helpers.asm:714-854`): `SUB_SPRCOL_LOAD` loads sprite A's absolute bounds into `e/d/c/b`; `SUB_SPRCOL_CHECK` then compares each candidate B's absolute bounds (read directly from `SPRTBL`) using an AABB test (`bx0 < ax1`, `ax0 < bx1`, `by0 < ay1`, `ay0 < by1`). `COLLISION()` maps to three dispatch entries (`DISP_usr3_COLLISION_*`, `compiler_hooks.h:213-215`) and its BASIC API is unchanged.

The dispatch table (`20_runtime.asm`) and `compiler_hooks.h` couple each kernel routine to a `DISP_*` index resolved through `addKernelCall`.

## Goals / Non-Goals

**Goals:**

- Per-sprite collision hitbox margins (`LEFT/TOP/RIGHT/BOTTOM`) via `SET SPRITE HITBOX`.
- `ON`/`OFF`/`AUTO`/single-sprite/5-arg forms, all implemented by one kernel routine with a mode byte.
- Pre-normalized relative bounds stored in a new `HITBOX_TABLE`, persistent until explicitly changed.
- `COLLISION()` keeps its public API and the same-position rule; hitbox configuration stays outside the collision hot path.
- A robust "disabled" hitbox that truly prevents collision (including mixed disabled/enabled pairs).

**Non-Goals:**

- Pixel-perfect or arbitrarily shaped hitboxes; per-pixel masks; dynamic AUTO recalculation.
- Changes to `SPRTBL` layout, `COLLISION()` API, or sprite visuals.
- Implementing the documented-but-unimplemented "same X/Y position ⇒ same object" rule (see Decision 7).

## Decisions

### Decision 1: Disabled hitbox uses a per-sprite flag in the reserved byte (Option A)

The RFC's proposal to encode "empty" as inverted bounds (`X0=middle`, `X1=middle-1`) is **mathematically insufficient**: for a disabled sprite A at position p and a normal 16×16 sprite B at the same position, the AABB test still passes (`bx0<ax1` → `100<107`, `ax0<bx1` → `108<116`, etc.), so a disabled sprite still collides with an enabled one. Inverted bounds only suppress collision when *both* sprites are disabled.

It is also impossible to guarantee non-collision with 8-bit relative bounds added to position (any sentinel collapses under coordinate wraparound).

Therefore `HITBOX_TABLE[+4]` (the RFC's reserved byte) becomes an **enabled flag**: `0xFF` = enabled, `0x00` = disabled. `COLLISION()` checks it once per sprite (A at load, B per candidate) and skips disabled sprites. This is per-sprite *data*, not a global mode flag, so it remains compatible with the RFC's "data over mode flags" principle (§23.6).

**Alternatives considered:** inverted-bounds (rejected — wrong), per-candidate `X1<X0` detection in the hot path (works, but adds a compare per candidate and is less explicit than a flag).

### Decision 2: HITBOX_TABLE layout — 5 bytes per sprite, parallel stride

```
HITBOX_TABLE  32 × 5 bytes = 160 bytes

+0  X0  (relative, = LEFT)
+1  X1  (relative, = SPRSIZ - RIGHT)
+2  Y0  (relative, = TOP)
+3  Y1  (relative, = SPRSIZ - BOTTOM)
+4  enabled (0xFF enabled, 0x00 disabled)
```

Same 5-byte stride as `SPRTBL` so a parallel pointer (`HL' → HITBOX_TABLE`) advances in lockstep with `HL → SPRTBL`, avoiding `sprite×4` address math in the hot path (RFC §11). Costs 32 bytes vs a 4-byte table; accepted.

### Decision 3: Store relative bounds, not margins or absolute bounds

`X1 = SPRSIZ - RIGHT` and `Y1 = SPRSIZ - BOTTOM` are precomputed at configuration time; `X0/Y0` are stored directly. `COLLISION()` computes `bx0 = B.x0 + B.X0`, `bx1 = B.x0 + B.X1`, `by0 = B.y0 + B.Y0`, `by1 = B.y0 + B.Y1`. `PUT SPRITE` keeps updating only `SPRTBL`; hitbox config stays independent of movement (RFC §13).

### Decision 4: One kernel routine `set_sprite_hitbox` with a mode byte

A single dispatch entry `DISP_set_sprite_hitbox` implements all five forms:

```
A = mode:  0 = ON (all sprites, zero margins, enabled)
           1 = OFF (all sprites, disabled)
           2 = AUTO (all sprites, pattern-derived)
           3 = single sprite ON (zero margins, enabled; L = sprite number)
               — used by both `SET SPRITE HITBOX <sprite>` and `... <sprite> ON`
           4 = set margins (L = sprite number; four margin bytes on stack)
           5 = single sprite OFF (disabled; L = sprite number)
           6 = single sprite AUTO (pattern-derived; L = sprite number)
```

The compiler pushes the four margins (left/top/right/bottom, omitted ones default to zero) then the sprite number; the kernel pops them, validates, converts to relative bounds, and writes `HITBOX_TABLE`. Exact register allocation is left to the kernel implementation (RFC §29).

### Decision 5: Reset point is `SUB_CLRSPR`

`HITBOX_TABLE` is reset to default (zero margins, enabled) inside `SUB_CLRSPR` (`60_bios_helpers.asm:615`). This single hook covers the three paths that clear or re-size sprites:

- startup (`20_runtime.asm:373`)
- `CMD WRTSPR` (`31_cmd.asm:152`)
- SCREEN sprite-size change (`I70BE`/`XBASIC_SCREEN_SPRITE`, `90_support.asm:1076-1082`, ends in `JP SUB_CLRSPR`)

A plain SCREEN *mode* change (`XBASIC_SCREEN` → `C7369` → `CHGMOD`) does **not** call `SUB_CLRSPR`, but it does not alter `SPRSIZ` either, so relative bounds remain valid and no extra reset is required in the mode-change path.

### Decision 6: AUTO is pattern-only, one-shot

`AUTO` scans the sprite pattern in VRAM (via `gfxCALPAT`/`CALPAT`) and computes the bounding box of set pixels. A pixel is "visible" iff its pattern bit is set (color is ignored). Fully transparent patterns produce a disabled hitbox (flag `0x00`). `AUTO` runs only when the command executes; later pattern changes do not recalculate (RFC §14, §17).

### Decision 7: Same-position rule is out of scope and currently unimplemented

Intensive search confirms the kernel only skips the *same sprite index* (`60_bios_helpers.asm:755`, `cp b; jr z SKIP`). The documented "sprites at the same X/Y position are the same object (no collision)" rule (`sprites-handling/spec.md:106`) has **no implementation** in the collision routines. Implementing it would add a position-equality check per candidate in the hot path. **This change preserves current behavior (no same-position check) and does not implement the rule.** See the note added to RFC-001 §18.

### Decision 8: Margin validation clamps at runtime

Margins are runtime expressions, so the kernel clamps them: each margin to `[0, SPRSIZ]`, and `RIGHT` to `SPRSIZ - LEFT` (and `BOTTOM` to `SPRSIZ - TOP`) so the resulting bounds never invert. Zero-width/height (e.g. `LEFT+RIGHT = SPRSIZ`) is valid.

### Decision 9: Fix off-by-one candidate Y base in `SUB_SPRCOL_CHECK` (bug fix)

The rewritten collision hot path (Decision 6.2) read both candidate X bounds from the candidate's `x0` byte (`SPRTBL+1`) but then advanced the pointer only **once** before the Y section. Since a `SPRTBL` entry is `active, x0, x1, y0, y1` (5 bytes), a single increment from `x0` lands on `x1`, not on `y0`. The Y comparisons therefore used the candidate's `x1` (= `x0 + SPRSIZ`) as if it were `y0`, so collisions were reported at a wrong, X-dependent distance instead of the true vertical overlap. The old routine was correct because it advanced once per X byte, landing on `y0`.

Fix: advance the candidate pointer twice (`x0 -> x1 -> y0`) before the Y section, and retarget the Y-failure branches to `skip_2` and the success path to two increments so every path still advances exactly 5 bytes per candidate.

### Decision 10: Fix pre-existing X/Y swap in `SUB_SETSPRTBL_XY` (bug fix)

`SUB_SETSPRTBL_XY` does `push ix` (X) then `push iy` (Y), so the first `pop de` yields `iy` (Y); the routine stored that value into `x0` and the second pop (X) into `y0`. `SPRTBL` therefore held `x0 = display Y` and `y0 = display X`. This predates the hitbox change and is invisible to plain collision (the AABB test is invariant under exchanging both axes consistently), but it transposed the new hitbox margins: `LEFT/RIGHT` were applied to the display Y axis and `TOP/BOTTOM` to the display X axis. Fix: swap the two pushes (`push iy; push ix`) so `x0` receives X and `y0` receives Y. Collision results are unchanged for full/symmetric hitboxes; asymmetric margins now shrink the intended axis.

## Risks / Trade-offs

- **RAM cost** → 160 bytes for `HITBOX_TABLE` shifts `SPRSIZ`/`HEAPEND` down, shrinking the BASIC heap by 160 bytes. Accepted; documented in the spec.
- **Hot-path overhead** → both A and B now add position to relative bounds (2 adds per side). Mitigated by precomputed right/bottom bounds and parallel pointer traversal; benchmark against current routine (RFC §29).
- **Kernel size** → new `set_sprite_hitbox` + AUTO pattern scan must fit the kernel binary (see `kernel-size-check` capability). Verify via `make`.
- **`SPRTBL` `x1/y1` become redundant for collision** → left in place for compatibility with any other readers; `COLLISION()` now uses `x0/y0` only.
- **SPRSIZ staleness after bare mode change** → not an issue (mode change does not alter `SPRSIZ`); sprite-size change always routes through `SUB_CLRSPR`.

## Open Questions

- Whether `ON` should also reset the enabled flag for sprites previously disabled by `OFF`/`AUTO` (Decision: yes — `ON` restores full default state for all 32).
- Benchmark target: define an acceptable overhead ceiling for the new collision loop before finalizing register allocation.
