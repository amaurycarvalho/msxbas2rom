# Design

## Context

See `proposal.md`. Tier 1 introduced the basic validation helpers and fixed direct parameter-loss bugs. Tier 2 needs metadata that is not in a single action: whether an identifier is a declared array, whether a literal resource exists, and whether a resource can be executed directly. The existing `compiler_tile_parameter.h` helper already implements declared-array detection for tiles; it can be reused for sprites.

## Goals / Non-Goals

**Goals:**
- Reuse existing array/resource metadata instead of adding global state.
- Keep compile-time checks limited to literals; variables remain runtime values.
- Preserve documented MTF negative map-coordinate wrapping.
- Make the validation policy reusable for Tier 3.

**Non-Goals:**
- Strict typing of all numeric/string parameters (Tier 3).
- Runtime validation of variable indices/resources (Z80 changes).
- Changing resource compression or RUNASM behavior in the kernel.

## Decisions

### Decision 1: Reuse `getDeclaredArrayIdentifier`

Sprite buffer validation will use the existing declared-array helper from `compiler_tile_parameter.h`. This keeps one definition of "declared array" for tile and sprite commands. Minimum element-count checks will only be added where the compiler can determine the declared bounds without adding a new symbol-layout subsystem.

Alternative: duplicate array detection in the sprite handlers. Rejected because it would diverge over time.

### Decision 2: Resource validation uses the compiler resource manager

Literal resource numbers will be checked against `ResourceManager` at compile time. Variable resource numbers remain runtime values and are covered by the Z80 guard changes.

Alternative: postpone all resource checks to runtime. Rejected because literal mistakes are deterministic and should fail before ROM generation.

### Decision 3: RUNASM requires resource metadata

For a literal resource, the compiler will inspect the resource kind and size. Compressed resources and resources larger than 255 bytes are rejected. Runtime guards in the Z80 change cover variable resource numbers and malformed metadata.

Alternative: rely on the kernel's 255-byte truncation. Rejected because it executes a truncated routine.

### Decision 4: MTF coordinates split by role

`map_x`/`map_y` remain unrestricted for operations 1 and 2 because negative values are a documented modulo-wrap feature. `width`/`height` and destination `screen_x`/`screen_y` are validated separately. Destination values above the screen remain valid and are clipped at runtime.

Alternative: reject all negative MTF coordinates. Rejected because it breaks documented scrolling behavior.

## Risks / Trade-offs

- **BREAKING** for programs using scalar sprite buffers, out-of-range literal sprite/key/strig indices, missing literal resources, or oversized RUNASM resources → document and add negative tests.
- Array dimension validation may be incomplete for multi-dimensional arrays → require only a declared array in the first pass and record dimension checks as a follow-up.
- Negative destination MTF coordinates were previously accepted with a wrap-like result → reject literal negatives and let runtime clipping own variables.

## Migration Plan

1. Add the shared helpers and resource introspection.
2. Migrate sprite, tile, MTF, resource, and RUNASM handlers.
3. Update unit/integration tests and documentation.
4. Roll back per handler if a compatibility problem is found.

## Open Questions

- Whether sprite buffer dimensions should be enforced strictly in this change or only declared-array status. Default: declared-array status now, dimension checks if the symbol table exposes dimensions.
