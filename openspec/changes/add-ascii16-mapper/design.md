## Context

msxbas2rom currently supports three MegaROM mappers (ASCII8, Konami4, KonamiSCC) via the pattern established by the `add-konami4-mapper` change: all share a single kernel binary (`61_megarom.asm`) and differ only in the segment-switch address bytes, patched at build time via dispatch table entries.

The ASCII16 mapper differs fundamentally from ASCII8/Konami: it uses **16KB pages** instead of 8KB pages. This means:
- One 16KB ROM page = one segment (instead of two 8KB segments per page)
- The kernel's `MR_CHANGE_SGM` writes to both 0x7000 and 0x7800 (segment+1); ASCII16 needs only the 0x7000 write
- The kernel receives 8KB segment numbers (2, 4, 6...) but ASCII16 hardware expects 16KB page numbers (1, 2, 3...)

The project follows Clean Architecture: `domain/`, `application/`, `infrastructure/`, `cli/`.

## Goals / Non-Goals

**Goals:**
- Add `ASCII16` as a first-class `CompileMode` option
- Reuse the existing kernel binary with dispatch-table-guided patching
- Convert 8KB→16KB segment mapping entirely in the kernel via `srl a`, keeping the compiler **unchanged**
- Provide CLI flags `-6` and `--ascii16`
- Produce ROMs with `[ASCII16]` filename suffix
- Dispatch table patching (same mechanism as Konami4 mappers)
- Unit and integration test coverage

**Non-Goals:**
- No separate kernel assembly file (`61_megarom.asm` is shared)
- No compiler changes (compiler.cpp = zero diff)
- No switching of the lower bank at 0x6000 — ASCII16 kernel stays fixed at 0x4000-0x7FFF, only the upper bank at 0x8000-0xBFFF is switched
- No R-Type special case (segment 0Fh fixed)
- No auto-detection of ASCII16 vs ASCII8 at kernel level

## Decisions

### Decision 1: Kernel-level `srl a` conversion — compiler unchanged

The compiler emits 8KB-pair segment numbers (2, 4, 6...) into the generated code. Instead of modifying the compiler's segment math, the kernel's `MR_CHANGE_SGM` routine is patched to insert `srl a` (logical shift right by 1) before the `ld (0x7000),a` bank switch. This divides the 8KB segment number by 2, producing the correct 16KB page number at runtime.

To preserve the `A` register for callers (MR_JUMP depends on A being restored), the patch uses `push af` / `pop af` around the conversion+switch sequence. The patch replaces the 9 bytes starting at `konami_patch_sgm_8000` (the `ld (0x7000),a` instruction) with:

```
push af            ; F5          preserve original 8KB segment + flags
srl a              ; CB 3F       seg 2,4,6,... → page 1,2,3,...
ld (0x7000), a     ; 32 00 70    write to ASCII16 upper bank (BASE address)
pop af             ; F1          restore original A+F for caller
ret                ; C9          return (replaces dec a at offset 10)
nop                ; 00          padding (original ret at offset 11)
```

This fits exactly within the original 12-byte `MR_CHANGE_SGM` footprint. The 8KB segment numbers emitted by the compiler flow through all existing code paths and are converted only at the hardware write point.

**Alternatives considered:**
- *Option B (compiler branches)*: Rejected — 8+ sites scattered in compiler.cpp, fragile and harder to maintain.
- *NOP-only approach*: Rejected — would leave both `ld (0x7000),a` writes with wrong segment values (unshifted 2, 4, 6 instead of 1, 2, 3).
- *srl a + push/pop kernel patch*: **Chosen** — zero compiler changes, conversion at the single hardware boundary, clean separation of concerns.

### Decision 2: Dispatch-table patching with SeqReplace, ByteReplace, and NOP

The `fixAscii16Mapper()` method uses a unified `Ascii16Patch` struct supporting three patch types:

| Type | Signature | Use |
|------|-----------|-----|
| **SeqReplace** | `seqBytes != nullptr, seqLen = N` | Overwrite N bytes with explicit byte sequence (MR_CHANGE_SGM rewrite) |
| **ByteReplace** | `replaceByte != 0` | Change byte at `offset+2` (port address operand) |
| **NOP** | `replaceByte == 0, seqBytes == nullptr, seqLen = N` | Write N bytes of `0x00` |

7 patches are applied:

| # | Dispatch Index | Label | Patch | Effect |
|---|---------------|-------|-------|--------|
| 1 | `DISP_KONAMI_PATCH_SGM_8000` (207) | `konami_patch_sgm_8000` | SeqReplace 9B: `{F5, CB, 3F, 32, 00, 70, F1, C9, 00}` | Rewrites MR_CHANGE_SGM with srl-based conversion |
| 2 | `DISP_KONAMI_PATCH_BUGFIX_6800` (214) | `konami_patch_bugfix_6800` | ByteReplace: byte[+2]=0x70 | Redirects `ld (0x6800),a` → `ld (0x7000),a` |
| 3 | `DISP_KONAMI_PATCH_BUGFIX_8000` (215) | `konami_patch_bugfix_8000` | NOP 3B | Removes redundant second write |
| 4 | `DISP_ASCII16_PATCH_BUGFIX_INC1` (221) | `ascii16_patch_bugfix_inc1` | NOP 1B | Removes `inc a` after first write |
| 5 | `DISP_ASCII16_PATCH_BUGFIX_NOPSEQ` (222) | `ascii16_patch_bugfix_nopseq` | NOP 4B | Removes `inc a + ld (0x7800),a` |
| 6 | `DISP_KONAMI_PATCH_OMSX_3` (212) | `konami_patch_omsx_3` | SeqReplace 3B: `{0x32, 0xFF, 0x77}` | Rewrites 4th OpenMSX write to 0x77FF |
| 7 | `DISP_KONAMI_PATCH_OMSX_4` (213) | `konami_patch_omsx_4` | SeqReplace 3B: `{0x32, 0xFF, 0x77}` | Rewrites 5th OpenMSX write to 0x77FF |

After patching, the boot bugfix reduces to a single `ld (0x7000), 1` followed by 8 NOPs. The OPENMSX autodetection pattern becomes: 3× `ld (0x7000),a` + 2× `ld (0x77FF),a`.

### Decision 3: No lower bank (0x6000) switching; base addresses only

ASCII16 has two switch addresses: 0x6000 for 0x4000-0x7FFF and 0x7000 for 0x8000-0xBFFF. The kernel code lives at 0x4000-0x7FFF and must always be accessible. The design keeps 0x6000 unused — the kernel page is never switched. Only 0x7000 switches are used.

All segment-switch writes for normal operation SHALL target the **base addresses** (0x6000 and 0x7000) exclusively. The only exception is the OPENMSX autodetection sequence (Decision 4), which uses two writes at 0x77FF for mapper identification. Mirror addresses (0x6001-0x67FF, 0x7001-0x77FF) SHALL NOT be used for any other purpose.

### Decision 4: OPENMSX autodetection — mirror writes at 0x77FF

The kernel writes to `Seg_P8000_SW` (0x7000) five times for OPENMSX autodetection. For ASCII16, the first 3 writes stay at 0x7000 (base address), while the 4th and 5th writes are patched to 0x77FF — a mirror address within the ASCII16 0x7000-0x77FF decoding range. The mixed pattern (0x7000 ×3, 0x77FF ×2) allows openMSX to distinguish ASCII16 from ASCII8 (which only uses 0x7000) while remaining functionally equivalent (both 0x7000 and 0x77FF select the same bank on ASCII16 hardware).

The patches use SeqReplace (3 bytes each) at `DISP_KONAMI_PATCH_OMSX_3` and `DISP_KONAMI_PATCH_OMSX_4` to rewrite `ld (0x7000),a` → `ld (0x77FF),a`.

### Decision 5: Same padding, ROM size, and segment math

ROM remains padded to a multiple of 8 pages (128KB). Max ROM size remains 2048KB. `resourceSegment` stays at `pages.size() * 2` (8KB-pair convention). The compiler's segment math is **unchanged**.

### Decision 6: Dispatch table entries and DISP_ constants

Two new labels added:
- `ascii16_patch_bugfix_inc1` in `20_runtime.asm` (boot bugfix)
- `ascii16_patch_bugfix_nopseq` in `20_runtime.asm` (boot bugfix)

Corresponding `dw` entries added to `wrapper_routines_map_table`. New constants: `DISP_ASCII16_PATCH_BUGFIX_INC1` (221) and `DISP_ASCII16_PATCH_BUGFIX_NOPSEQ` (222). `DISP_ENTRIES` updated to 223.

The MR_CHANGE_SGM patch reuses the existing `konami_patch_sgm_8000` label and `DISP_KONAMI_PATCH_SGM_8000` constant — no new label needed there.

### Decision 7: CLI flag `-6` / `--ascii16`

Short flag `-6`. Long flag `--ascii16`. All other flags (`-a`, `-x`, `-8`, `-k`, `-4`) are already taken.

## Risks / Trade-offs

- **[Risk] OpenMSX may not distinguish ASCII16 from ASCII8 in auto-detection** → Mitigation: The 4th and 5th OPENMSX writes are patched to 0x77FF (mirror within ASCII16 0x7000-0x77FF range) to create a distinguishable pattern. Both addresses map to the same bank register on ASCII16 hardware.
- **[Risk] `srl a` assumes even segment numbers** → Mitigation: The compiler always emits even segment numbers (2, 4, 6...) in megaROM mode. `srl a` produces correct integer division by 2 for all even values.
- **[Risk] `push af`/`pop af` adds 2 extra stack operations per segment switch** → Mitigation: Stack usage is minimal (MR_CHANGE_SGM is leaf routine or called once per switch). The stack is in page 3 RAM with ample space.
- **[Risk] SeqReplace overwrites `dec a` and `ret` at end of MR_CHANGE_SGM** → Mitigation: The replacement explicitly includes `ret` at the correct offset. The padding NOP after `ret` is unreachable.
