# Proposal

## Why

The kernel trusts several runtime parameter values and resource metadata. Invalid sprite numbers index past fixed tables into unrelated RAM, out-of-range directions produce inconsistent transformations, short or missing resources are used without checking the resource loader's failure signal, and `CMD RUNASM` can execute a truncated routine. These are low-effort guards with high memory-safety impact.

## What Changes

- Mask/validate sprite numbers in `SUB_GETSPRTBL_ADDR` and `SUB_GET_HITBOX_ADDR` so `PUT SPRITE`, `SET SPRITE HITBOX`, and collision helpers cannot index past their 32-entry tables.
- Normalize tile and sprite flip/rotate directions to the documented `0..2` range; out-of-range runtime values become a safe no-op or documented default instead of an arbitrary transformation.
- Check `BC >= 7` in `CMD WRTFNT` before subtracting the header, and bail safely when the resource is shorter.
- Check the carry/failure result of `resource.open_and_get_address` in MTF, WRTFNT, WRTSPR, RESTORE, PLYLOAD, and WRTVRAM paths before using `HL`/`BC`.
- Harden `CMD RUNASM` against resources larger than 255 bytes or invalid resources; do not execute a truncated copy.
- Guard `cmd_plysound` against effect `0`, channel above `2`, and inverted volume above `16`, preserving the documented `0..16` range.

## Capabilities

### New Capabilities

- `kernel-parameter-guards`: runtime validation of kernel command parameters and resource metadata, with safe failure behavior instead of table overruns, truncation, or undefined execution.

### Modified Capabilities

- _(none)_

## Impact

- Z80 kernel routines in `31-cmd-tile-sprite.asm`, `31-cmd-game-graphics.asm`, `33_player.asm`, `60_bios_helpers.asm`, and resource-using command modules.
- Compiled programs that pass invalid dynamic values now receive safe no-op/failure behavior instead of RAM corruption.
- Emulator regression coverage for sprite-table canaries, short/invalid resources, and `RUNASM` size limits.
