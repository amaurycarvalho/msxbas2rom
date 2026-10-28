# ADR-002: Initial Text Screen Normalization

## Status

Accepted.

This ADR records the decision to normalize the generated ROM startup to an
explicit `SCREEN 0` 40-column text layout, independent of the machine boot
default.

## Context

The compiler configured the BIOS 40-column variables (`LINL40`, `LINLEN`, and
`CLMLST`) but delegated the initial VDP mode to the machine boot default. MSX1
machines normally boot in `SCREEN 0` (40 columns), while Japanese MSX2, MSX2+,
and turbo R machines boot in `SCREEN 1` (32 columns). The mismatch produced
40-column BIOS text written through a 32-column name-table stride, so every
`PRINT` line drifted to the right.

The apparent CR+LF defect was a symptom, not the cause: the PRINT CR+LF routine
already emits CR followed by LF. The visible drift came from the VDP/BIOS width
mismatch.

## Research: initial MSX text modes

| Machine class | Typical boot mode | Columns | Notes |
| --- | --- | --- | --- |
| MSX1 | `SCREEN 0` | 40 | `SCREEN 0` is 40x24; `SCREEN 1` is 32x24 |
| MSX2 (Japanese) | `SCREEN 1` | 32 | Japanese machines boot in `SCREEN 1`; `SCREEN 0` can use 40 or 80 columns via `WIDTH` |
| MSX2+ (Japanese) | `SCREEN 1` | 32 | Same BIOS/default rule as Japanese MSX2 |
| turbo R (Japanese) | `SCREEN 1` | 32 | Same BIOS/default rule as Japanese MSX2 |

The MSX Wiki `SCREEN` documentation states that, without `SET SCREEN`, MSX2 and
higher boot in `SCREEN 1` on Korean/Arabic MSX1 machines, Bawareth Perfect MSX2,
Daewoo CPC-300, Korean MSX2 consoles, Brazilian MSX2+ Ciel machines, and all
Japanese machines; other machines boot in `SCREEN 0`.

The regression suite uses `Sharp_HB-8000_1.2` (MSX1),
`Panasonic_FS-A1WX` (Japanese MSX2/MSX2+), and `Panasonic_FS-A1GT` (Japanese
turbo R). An inert-cartridge boot capture confirmed the MSX1 default is
`SCREEN 0` with name table `0x0000`. A test ROM containing `WIDTH 40` then
produced the same aligned 40-column rows with name table `0x0000` on every
configured machine class.

## Decision

The generated ROM startup will explicitly:

1. Configure the 40-column BIOS variables (`LINL40=39`, `LINLEN=39`,
   `CLMLST=14`, `LINL32=29`, `CRTCNT=24`).
2. Set the kernel text output mode to text (`SOMODE=0`).
3. Select `SCREEN 0` through the BIOS `CHGMOD` entry with mode `0`.

The startup no longer depends on the machine boot default or on the movable
game-graphics kernel module. The builder still patches a `C7369` call if an older
start image contains one, but the current start image does not require it.

## Alternatives

- **Keep `SCREEN 1` and configure 32 columns.** Rejected because the startup
  already configured 40-column BIOS variables, MSX1 naturally uses 40 columns,
  and 32 columns would reduce text capacity for every program.
- **Rely on the machine boot default.** Rejected because Japanese MSX2/MSX2+/
  turbo R machines boot in `SCREEN 1` and therefore render 40-column text with a
  32-column VDP stride.
- **Call the movable game-graphics mode routine at startup.** Rejected because
  the routine is not present in every ROM and only adds MSX2+ screen-mode
  handling for modes 9-12; `SCREEN 0` is fully served by BIOS `CHGMOD`.

## Consequences

- The default text layout is `SCREEN 0`, 40 columns, with the name table at
  `0x0000`.
- Existing regression baselines that captured the shifted 32-column layout are
  stale and must be regenerated. Programs that set their own `SCREEN` keep the
  layout selected by that statement.
- `PRINT/test5` and `STRING/test1` now validate the normalized layout with
  expected rows and VRAM byte probes.
- The PRINT CR+LF kernel routine is unchanged.

## References

- `openspec/changes/fix-string-temp-buffer-concat/design.md`
- `openspec/changes/fix-string-temp-buffer-concat/specs/compiler/spec.md`
- `src/infrastructure/kernel/asm/src/start.asm`
- `doc/misc/architecture.md`
