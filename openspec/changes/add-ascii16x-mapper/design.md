## Context

msxbas2rom supports five MegaROM mappers: ASCII8, ASCII16, Konami4, KonamiSCC, and (deprecated) Pcoded. The ASCII16-X mapper is an extension of ASCII16 defined by the openMSX/grauw project. It adds mirror address decoding, 12-bit bank number support (address bits A8-A11 carry upper 4 bank bits), and an autodetection signature at ROM offset 0x0010.

The ASCII16 implementation (release 1.1.0.0) uses a dispatch-table-based kernel patching approach. The kernel's `MR_CHANGE_SGM` is rewritten at build time with `push af; srl a; ld (0x7000),a; pop af; ret`, converting 8KB segment numbers from the compiler to 16KB page numbers for the mapper hardware. The compiler is mapper-agnostic — it only checks `opts->megaROM` (bool), never the specific `compileMode` enum.

For ROMs ≤4MB (≤256 banks × 16KB), which covers all msxbas2rom-generated output, the upper 4 bank bits (A8-A11) of ASCII16-X are always 0. Writing to base address 0x7000 encodes bank bits 0-7 via D0-D7 and bank bits 8-11 = 0 via A8-A11. This makes ASCII16-X functionally identical to ASCII16 for the targeted ROM size range.

The kernel binary has a `KERNEL_END_FILLER` (`DEFS 08000H-$,000H` in `90_support.asm:3867`) that pads to exactly 0x8000 bytes. This means inserting 8 bytes earlier in the kernel code is absorbed by the filler — the total binary size stays at 32768 bytes and the `BIN_HEADER_BIN_LEN` constant is unchanged.

## Goals / Non-Goals

**Goals:**
- Add `ASCII16X` as a first-class `CompileMode` option
- Reuse 100% of the ASCII16 kernel patching (same 7 dispatch-table patches)
- Insert the ASCII16-X autodetection signature `"ASCII16X"` at ROM offset 0x0010
- Reserve 8 bytes in the kernel header, shifting INIT1 from 0x4010 to 0x4018
- Update the hardcoded `"LOADER"` symbol address from `"4010"` to `"4018"`
- Provide CLI flags `-7` and `--ascii16x`
- Produce ROMs with `[ASCII16X]` filename suffix
- Unit and integration test coverage
- Zero changes to compiler

**Non-Goals:**
- No 12-bit bank number support (ROMs stay ≤4MB, upper bits always 0)
- No mirror address usage for bank switching (uses base addresses 0x6000/0x7000 only)
- No separate `fixAscii16xMapper()` method (reuses `fixAscii16Mapper()`)
- No changes to any MegaROM mode other than ASCII16X

## Decisions

### Decision 1: Extend `fixAscii16Mapper()` guard — no separate method

The existing `fixAscii16Mapper()` already contains the full ASCII16 kernel patching logic (7 patch points: 1 SeqReplace for MR_CHANGE_SGM, 1 ByteReplace for bugfix, 3 NOPs for bugfix cleanup, 2 SeqReplace for OMSX). Since ASCII16-X needs identical kernel behavior, the guard condition is extended:

```cpp
// Before:
if (opts->compileMode == BuildOptions::CompileMode::ASCII16) {

// After:
if (opts->compileMode == BuildOptions::CompileMode::ASCII16 ||
    opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
```

For ASCII16X mode, additionally write the 8-byte signature `"ASCII16X"` at fixed offset 0x0010 in `pages[0]`, and NOP out the 14-byte `AB` signature check in the boot bugfix via the dispatch table:

```cpp
if (opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
    memcpy(pages[0].data() + 0x0010, "ASCII16X", 8);

    int tableAddr = def_wrapper_routines_map_table + DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK * 2;
    int kernelAddr = bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
    int offset = kernelAddr - 0x4000;
    for (int j = 0; j < 14; j++) {
      pages[0].data()[offset + j] = 0x00;
    }
}
```

This adds a new dispatch table entry at index 223 (`DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK`) and bumps `DISP_ENTRIES` to 224. The 14-byte NOP removes the `ld a,(0x8000); cp 0x41; jr nz; ld a,(0x8001); cp 0x42; jr nz` comparison block so the ASCII8 bugfix always executes on ASCII16-X hardware.

**Alternatives considered:**
- *Separate `fixAscii16xMapper()` method*: Rejected — duplicates logic with no benefit. The 7 patch points are identical.
- *Helper method like `is16KMapper()`*: Rejected — user prefers direct enum comparisons matching the existing codebase style.

### Decision 2: No compiler changes

The compiler's `write()` and fixup resolution check only `opts->megaROM` (bool), never the specific mapper enum. Segment math (8KB pairs, `*2+2` fixup, 8-byte skip preamble, 0xA000 split) is uniform across all MegaROM modes. The kernel's `srl a` in `MR_CHANGE_SGM` transparently converts 8KB segment numbers to 16KB page numbers. ASCII16-X inherits this unchanged.

### Decision 3: Include the signature — reserve 8 bytes in kernel header

The ASCII16-X spec recommends placing the string `ASCII16X` at ROM offset 0x0010 (MSX address 0x4010). This overlaps with the kernel entry point `INIT1`. The solution:

1. **Kernel ASM** (`20_runtime.asm`): Insert `ds 8, 0x00` between the `db 'MSXB2R'` line and `INIT1:`. This shifts INIT1 from 0x4010 to 0x4018 and reserves 8 bytes at the signature location.
2. **ROM builder** (`rom.cpp`): For ASCII16X mode, write `"ASCII16X"` at `pages[0].data() + 0x0010`. For other modes, the 8 bytes remain 0x00 (from the ASM `ds 8, 0x00` default).
3. **Symbols** (`symbol_export_context.cpp`): Update hardcoded `"LOADER"` address from `"4010"` to `"4018"` to match the shifted INIT1. All export strategies (NoICE, symbol, OMDS, CDB, ELF) read this from context — no changes needed there.
4. **Regeneration**: `make header` regenerates `header.h` and `header.symbols.asm`. The `KERNEL_END_FILLER` absorbs the 8-byte displacement, keeping `BIN_HEADER_BIN_LEN` at 32768. The kernel size test (`test_kernel.cpp`) passes unchanged because the size limit check (`<= 32768`) still holds.

```
BEFORE (current)                    AFTER (with signature)
─────────────────────                ────────────────────
0x4000: db "AB"                     0x4000: db "AB"
0x4002: dw 0x4010                   0x4002: dw 0x4018
0x4004: dw 0x0000                   0x4004: dw 0x0000
0x4006: dw 0                        0x4006: dw 0
0x4008: dw 0                        0x4008: dw 0
0x400A: db 'MSXB2R'                 0x400A: db 'MSXB2R'
                                    0x4010: ds 8, 0x00  ← signature goes here
0x4010: INIT1:                      0x4018: INIT1:
  ld a, (EXPTBL)                       ld a, (EXPTBL)
  ...                                   ...
0x7FFE: KERNEL_END_FILLER           0x7FFE: KERNEL_END_FILLER
  DEFS 08000H-$,000H                   DEFS 08000H-$,000H  ← 8 bytes shorter
```

**Alternatives considered:**
- *Skip signature entirely*: Rejected — the user now wants it. The impact analysis showed it's feasible with minimal changes (4 files, ~10 lines of code).
- *Put signature at 0x000A (replace "MSXB2R")*: Rejected — only 6 bytes available (need 8), and openMSX looks at offset 0x0010.
- *Conditional ASM for 8-byte reservation*: Rejected — `pasmo` does not support conditional `DEFS`. The 8 bytes are reserved unconditionally; only the fill value differs by mode.

### Decision 4: CLI flag `-7` / `--ascii16x`

Short flag `-7` (ASCII16-X has 7 characters after "ASCII"). Long flag `--ascii16x`. These are consistent with existing conventions:
- `-6` / `--ascii16` for ASCII16
- `-4` / `--konami` for Konami4
- `-8` / `--ascii8` for ASCII8

All other short-flag characters are either taken or reserved.

### Decision 5: Same resource mapping as all other MegaROM modes

`resourceSegment` stays at `pages.size() * 2` for ASCII16X (same 8KB-pair convention as ASCII8, ASCII16, Konami4, KonamiSCC). The kernel's `srl a` conversion makes this transparent. No special case needed in `addResources()`.

### Decision 6: Same OMSX autodetection patches as ASCII16

The ASCII16 patches change the 4th and 5th OPENMSX autodetection writes from `ld (0x7000),a` to `ld (0x77FF),a`. For ASCII16-X, 0x77FF is within the 0x7000-0x7FFF base register range (not a mirror range). These are detection-only writes, not functional bank switches. The runtime `MR_CHANGE_SGM` uses 0x7000 as required. No change needed.

### Decision 7: Kernel binary regeneration via `make header`

After modifying `20_runtime.asm`, the kernel binary (`header.h`) and symbol file (`header.symbols.asm`) must be regenerated. This is done by running `make header`, which invokes `pasmo` and `xxd -i`. The new `header.h` will have all ROM addresses from 0x4010 onwards shifted by +8. All dispatch table lookups (`bin_header_bin[tableAddr]`) automatically resolve to the new correct addresses since they reference labels, not hardcoded values.

## Risks / Trade-offs

- **[Risk] Kernel binary shift affects all compile modes, not just ASCII16X** → Mitigation: The 8-byte reservation is in the shared kernel ASM. All modes see the shift. This is harmless — the kernel code is identical, just at a different address. Dispatch table entries auto-resolve. The `"LOADER"` symbol address update covers all modes uniformly. Integration tests will catch any regression.
- **[Risk] openMSX auto-detection may still not distinguish ASCII16-X from ASCII16 even with the signature** → Mitigation: The signature follows the ASCII16-X spec. Whether openMSX uses it depends on the openMSX version. The ROM functions correctly regardless.
- **[Risk] MSX BIOS slot expander detection corrupts ASCII16-X page 2 register via mirror writes** → Mitigation: The kernel boots from page 1 (0x4000-0x7FFF), which is not affected. Page 2 is explicitly initialized before use via the startup code.
- **[Risk] ASCII16X and ASCII16 share 7 identical patch points — any future ASCII16 patch changes affect ASCII16X** → Mitigation: This is intentional and desired. Both mappers share the same kernel behavior for ROMs ≤4MB.

## Open Questions

None — all design decisions are resolved.
