# Design

## Context

See `proposal.md`. The kernel receives dynamic values in registers or in the `DAC`/`ARG`/`PARM1` work areas. Fixed sprite tables have 32 entries, but address helpers multiply the raw byte. Resource helpers return a carry flag on failure, but most command handlers do not inspect it. `CMD RUNASM` uses a 255-byte copy buffer.

## Goals / Non-Goals

**Goals:**
- Prevent table overruns and undefined resource pointers with minimal cycles and size.
- Keep documented valid values, including negative MTF map coordinates, unchanged.
- Make failure behavior deterministic (ignore, clamp, or skip).

**Non-Goals:**
- Full MTF header/window validation (Tier 2).
- Central guard macros (Tier 3).
- Compiler-side literal rejection (C++ changes).

## Decisions

### Decision 1: Normalize sprite indices in the address helpers

`SUB_GETSPRTBL_ADDR` and `SUB_GET_HITBOX_ADDR` will mask the raw sprite number to five bits before multiplying by five. This protects every caller at once. Literal compile-time rejection is handled by the C++ change.

Alternative: validate at each caller. Rejected because there are multiple call paths and future callers can miss the check.

### Decision 2: Normalize directions in the flip/rotate entries

Each entry executes a small normalization: values above `2` become a documented safe value (or no-op), and negative values are handled explicitly. The policy will be identical for tile and sprite variants.

Alternative: rely on compiler literal validation. Rejected because direction can come from a variable.

### Decision 3: Check resource carry at the command boundary

Resource consumers will branch on carry immediately after `resource.open_and_get_address`. On failure they close/skip and return without using `HL`/`BC`. `cmd_wrtfnt` additionally checks `BC >= 7`.

Alternative: make `resource.address` return sentinel values. Rejected because callers already have a documented carry contract and RSCTORAM already uses it.

### Decision 4: RUNASM checks the real resource size

`cmd_runasm` will use `resource.address` to obtain the real size before copying, instead of relying on `resource.get_data`'s 255-byte cap. Missing, oversized, or non-executable resources return without a jump.

Alternative: keep truncation and document it. Rejected because executing a truncated routine is unsafe.

### Decision 5: PLYSOUND guards the Arkos ABI

The kernel will reject effect `0`, channel above `2`, and volume above `16` before calling `PLY_AKM_PlaySoundEffect`.

Alternative: sanitize in the compiler only. Rejected because values can be dynamic.

## Risks / Trade-offs

- Extra branches add size and cycles in hot paths → keep guards small and measure with the kernel size check.
- Masking sprite numbers changes behavior for callers that relied on accidental wraparound → desirable safety change; document.
- Resource carry checks can change previously "working by accident" invalid-resource behavior → desirable.
- PLYSOUND volume clamping must not alter valid `0..16` values.

## Migration Plan

1. Add sprite index masks and direction normalization.
2. Add resource/size checks and RUNASM validation.
3. Add PLYSOUND guards.
4. Build kernel and run emulator regression probes.
5. Roll back per routine if compatibility issues appear.

## Open Questions

- Whether invalid dynamic directions should no-op or clamp to `2`. Default: no-op for defined behavior; confirm during implementation.
