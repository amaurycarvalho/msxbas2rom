# Design

## Context

See `proposal.md` - Why. The compiler reserves five 256-byte temporary string slots and asks the kernel for the next slot through `GET_NEXT_TEMP_STRING_ADDRESS`. Several string functions still return their result in the fixed `BUF`, while other string strategies copy `BUF` into the requested temporary slot.

The startup also configures 40-column BIOS variables but selects the VDP screen mode through `L`, which is not initialised. The resulting display depends on the machine's boot default, producing a 32-column VDP layout with 40-column BIOS text. The CR+LF routine already emits CR then LF.

## Goals / Non-Goals

**Goals:**

- Make every string-function result in a single expression independent of later calls.
- Normalise the default ROM text layout to 40 columns on every supported machine class.
- Add probes for `PRINT/test5` and `STRING/test1` that fail on both defects.
- Review all PRINT/`?`/`STRING$` regression expectations after the layout change.
- Reuse the tier-1 function-arity hardening for `STRING$`.

**Non-Goals:**

- Changing the PRINT CR+LF kernel routine; it already emits CR then LF.
- Redesigning all MSX BASIC string functions or their ABI.
- Preserving the old 32-column shifted layout; it is the defect.

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
Japanese machines; other machines boot in `SCREEN 0`. The regression suite uses
`Sharp_HB-8000_1.2` (MSX1), `Panasonic_FS-A1WX` (Japanese MSX2/MSX2+), and
`Panasonic_FS-A1GT` (Japanese turbo R).

An inert-cartridge boot capture confirmed the MSX1 default is `SCREEN 0` with
`LINLEN=39` and name table `0x0000`. A test ROM containing `WIDTH 40` was then
run on all four configured machine classes and produced the same aligned
40-column rows with name table `0x0000` on every one.

## Decisions

### 1. Fix the temporary-slot stride to 256 bytes

`GET_NEXT_TEMP_STRING_ADDRESS` currently computes `TMPSTRADDR + index`, even though the compiler reserves `256 * 5` bytes. The allocator will advance by 256 bytes per slot, preserving its existing 0..4 rotation and AF/BC preservation.

Alternative considered: shrink the reservation to five bytes. Rejected because string results need full 255-character buffers and existing code already assumes 256-byte destinations.

### 2. Copy `BUF` results into the requested temporary slot for `STRING$`, `SPACE$`, and `INKEY$`

These strategies currently call `addTempStr(true)` and rely on the kernel to write to `HL`, but the corresponding kernel routines ignore `HL` and write `BUF`. They will follow the pattern already used by `LEFT$`, `RIGHT$`, `MID$`, `HEX$`, `OCT$`, `BIN$`, and `USING$`: call the function, request a temporary destination with `addTempStr(false)`, copy `BUF` with `DISP_XBASIC_COPY_STRING`, and leave `HL` pointing at the copy.

Alternative considered: change each kernel string routine to honor `HL`. Rejected for this change because it touches more kernel routines and changes their historical contract. The compiler-side copy keeps the kernel behavior unchanged and is consistent with the majority of string strategies.

`CHR$` and `INPUT$` already write directly to the requested destination, so the stride fix alone makes them safe.

### 3. Normalise startup to `SCREEN 0` + `WIDTH 40`

The startup must not rely on the machine's boot default. It will explicitly select
`SCREEN 0` and apply `WIDTH 40` before the program starts, using the existing
`XBASIC_WIDTH`/`XBASIC_SCREEN` kernel path or an equivalent sequence. This makes
the VDP name table, `LINLEN`, and the text stride consistent on MSX1, MSX2,
MSX2+, and turbo R.

Alternative considered: keep `SCREEN 1` and set the BIOS to 32 columns. Rejected
because the startup already sets `LINL40=39`, `LINL32=29`, and `LINLEN=39`, and
MSX1's natural default is 40 columns. Normalising to 40 columns matches the
existing intent and avoids a 32-column limitation for text programs.

`WIDTH 40` was verified on all four configured machine classes: it produces
`R#1=0x70` (or `0xF0` on MSX1), name table `0x0000`, and identical aligned rows.
The fixed default name table is therefore `0x0000`, not the old 32-column
`SCREEN 1` table at `0x1800`.

### 4. Regression probes for the split programs

`PRINT/test5` covers trailing `;`, trailing `,`, repeated commas, leading
separators, and tab spacing. Under the fixed layout its rows are:

```text
0  HELLO
1  WORLD
2  HELLO WORLD!!!
3  HELLO WORLD     !!!
4  HELLO   WORLD   !!!
5  HELLO WORLD!!!
6  HELLO           WORLD   !!!
7  HELLO   WORLD   !!!
8  HELLO   WORLD           !!!
9  HELLO   WORLD           !!!
10 HELLO   WORLD           !!!
```

Because text comparison normalises whitespace, the separator rows also need
VRAM byte probes at their exact name-table offsets to validate tab positions.

`STRING/test1` covers separate and concatenated results. Under the fixed layout
its significant rows are:

```text
0  HELLO WORLD!!!
1  AAAAA
2  BBBBBB
3  AAAAABBBBBB!!!
4  AAAAABBBBBB
5  AAAAA   BBBBBB
6  AAAAABBBBB
7  AAAAABBBBB
10   AAAAABBBBB
```

The probes target rows 3, 4, 6, 7, and 10; row 5 validates the comma-separated
form. Unlike the earlier combined test file, `LOCATE 2,10` no longer overwrites
another `STRING$` probe, so the test program itself does not need to move.

Both scenarios use `width: 40` and `name_table: "0x0000"`.

### 5. Audit all PRINT and STRING$ regression expectations

A source scan finds 123 integration programs that use `PRINT`, `?`, or
`STRING$`; 119 have baselines. Of those baselines, 94 currently record
`screen: 1`, which is the stale 32-column layout. The change will audit that
set, regenerate the affected baselines, and update manifest text checks whose
programs do not set their own `SCREEN`. Programs that set `SCREEN` explicitly
still need a `STRING$` regression check when they use string functions.

### 6. Harden extraction from `harden-cpp-*`

The `harden-cpp-parameter-validation-tier1` change contains the shared function-arity bound as task 4.4 and the corresponding requirement in its `compiler` spec delta. No `harden-cpp-*` change contains a PRINT/`?`-specific requirement. This change will apply the arity bound to `STRING$` and add a five-argument rejection test; other PRINT/`?` hardening is out of scope.

### 7. Document the screen-normalization decision

Add `doc/adr/ADR-002 Initial text screen normalization.md` following the
existing ADR style. It will record:

- the problem: BIOS 40-column setup with a machine-dependent 32-column VDP mode;
- the per-machine boot-mode research summarized above;
- the decision to normalize to `SCREEN 0` + `WIDTH 40`;
- the alternatives (keep `SCREEN 1`/32 columns, rely on the machine default) and
  why they were rejected;
- the consequence that default text baselines move from name table `0x1800` to
  `0x0000` and must be regenerated.

Update `doc/misc/architecture.md` section 5.3 (`Start code`) to state that startup
selects `SCREEN 0` and applies `WIDTH 40` explicitly, and to reference ADR-002.
This keeps the architecture guide aligned with the `compiler` spec delta and
with ADR-001's rule that architectural changes update specs and ADRs together.

## Risks / Trade-offs

- **Changing the default screen layout changes most text baselines** -> Treat the existing 94 `screen: 1` baselines as stale, regenerate them, and review the new text rather than accepting snapshots blindly.
- **Changing the temporary-slot stride could affect other allocator callers** -> Audit `GET_NEXT_TEMP_STRING_ADDRESS` callers (`MR_GET_DATA`, buffer helpers, string strategies) and verify they treat the returned address as a 256-byte string destination.
- **Compiler-side copies add code bytes to string-heavy programs** -> Reuse the existing `COPY_STRING` kernel call; verify Plain ROM and ASCII8 builds for `PRINT/test5`, `STRING/test1`, and a representative string-heavy program.
- **The apparent CR bug could be misdiagnosed** -> The regression probes assert both the 40-column name-table layout and the separator/concatenation text, so a regression in either screen selection or CR+LF fails visibly.
- **Machine defaults differ** -> Explicit `SCREEN 0` + `WIDTH 40` removes the dependency on boot defaults; the same test ROM was verified on all four configured machine classes.

## Migration Plan

1. Fix the allocator stride and string strategies; rebuild the kernel and release binary.
2. Normalise startup to `SCREEN 0` + `WIDTH 40`; verify the 40-column layout on all four machine classes.
3. Add `PRINT/test5` and `STRING/test1` manifest entries and probes; generate their baselines from the fixed ROM.
4. Audit and refresh all PRINT/`?`/`STRING$` baselines and manifest text checks.
5. Run unit, integration, regression, and architecture checks.

Rollback is limited to reverting this change; no data migration is involved.

## Open Questions

None.
