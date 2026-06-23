## Context

msxbas2rom currently supports three MegaROM mappers (ASCII8, Konami4, KonamiSCC) via the pattern established by the `add-konami4-mapper` change: all share a single kernel binary (`61_megarom.asm`) and differ only in the segment-switch address bytes, patched at build time via dispatch table entries.

The ASCII16 mapper differs fundamentally from ASCII8/Konami: it uses **16KB pages** instead of 8KB pages. This means:
- One 16KB ROM page = one segment (instead of two 8KB segments per page)
- The kernel's `MR_CHANGE_SGM` writes to both 0x7000 and 0x7800 (segment+1); ASCII16 needs only the 0x7000 write
- The compiler's segment numbering, skip-preamble size, and fixup resolution all assume 8KB-pair math

The project follows Clean Architecture: `domain/`, `application/`, `infrastructure/`, `cli/`.

## Goals / Non-Goals

**Goals:**
- Add `ASCII16` as a first-class `CompileMode` option
- Reuse the existing kernel binary with dispatch-table-guided NOP patching
- Adjust compiler segment math for 16KB pages via inline conditional branches (Option B)
- Provide CLI flags `-6` and `--ascii16`
- Produce ROMs with `[ASCII16]` filename suffix
- Dispatch table patching (same mechanism as Konami4 mappers)
- Unit and integration test coverage

**Non-Goals:**
- No separate kernel assembly file (`61_megarom.asm` is shared)
- No parameterization of segment size (avoids refactoring the compiler's segment model)
- No switching of the lower bank at 0x6000 — ASCII16 kernel stays fixed at 0x4000-0x7FFF, only the upper bank at 0x8000-0xBFFF is switched
- No R-Type special case (segment 0Fh fixed)
- No auto-detection of ASCII16 vs ASCII8 at kernel level

## Decisions

### Decision 1: Kernel sharing with dispatch-table NOP patching (like Konami4)

The ASCII16 mapper reuses the same `61_megarom.asm` kernel binary. The `fixAscii16Mapper()` method patches it at build time via dispatch table entries, just as `fixKonamiMapper()` does for Konami modes.

While Konami patching replaces the high byte of port addresses (byte-level replace at `offset+2`), ASCII16 patching needs to **NOP out entire instruction sequences**. The dispatch table labels point to the first byte of each sequence to NOP. The patch writes `0x00` for N bytes starting at the labeled address.

**Alternatives considered:**
- *Separate kernel assembly file*: Rejected — would add a new kernel variant, binary, and include to maintain. The shared-kernel approach is proven and simpler.
- *Byte-scanning (pre-dispatch-table approach)*: Rejected — fragile, and the dispatch table mechanism already exists for this purpose.

### Decision 2: Option B — inline conditionals for compiler segment math

The compiler's `write()` method and fixup loop contain ~8 sites where segment/page math assumes 8KB-pair semantics. Each site branches on `compileMode == ASCII16`:

| Site | ASCII8 (current) | ASCII16 (new) |
|------|------------------|---------------|
| `segm_last` initial | `2` | `1` |
| `segm_last` increment | `+= 2` | `+= 1` |
| `segm_total` rounding | `((last+1)/16+1)*16` | `((last+1)/8+1)*8` |
| Skip preamble size (code) | 8 bytes | 4 bytes |
| Skip preamble size (data) | 3 bytes | 0 (implicitly from changed preamble) |
| `segm` assignment split | `if addr >= 0xA000: segm++` | never splits |
| Fixup `segm_from` | `((addr)/0x4000)*2+2` | `((addr)/0x4000)*1+1` |
| Fixup `segm_to` | `((addr)/0x4000)*2+2` | `((addr)/0x4000)*1+1` |
| `resourceSegment` | `pages.size() * 2` | `pages.size()` |

**Alternatives considered:**
- *Option A: Parameterize with segment size variable*: Rejected — requires refactoring the segment model, introduces new abstraction surface, and doesn't match the existing codebase style which already uses `if (opts->megaROM)` branches.
- *Option B: Inline conditionals*: **Chosen** — each change is a 1-2 line conditional, mechanically verifiable, and matches the existing pattern (`if (opts->megaROM)` branches throughout the codebase).

### Decision 3: Mix byte-replace and NOP-sequence patching

ASCII16 needs two types of patch operations on the kernel:

1. **Byte replace** (like Konami): In the boot bugfix, change `ld (0x6800),a` → `ld (0x7000),a` by patching byte `0x68`→`0x70` at the `konami_patch_bugfix_6800` label location. After the NOP that follows (see below), both writes go to 0x7000 with the correct segment value.

2. **NOP sequences**: New labels point to instruction sequences that must be replaced with `0x00`:
   - MR_CHANGE_SGM: `inc a` (1 byte) + `ld (0x7800),a` (3 bytes) + `dec a` (1 byte) = **5 bytes NOP**
   - Boot bugfix: `inc a` (1 byte) before `ld (0x7000),a` = **1 byte NOP** (the 0x6800 write already diverted to 0x7000, so a incs to 2 with the NOP in place, then writes 1 again — redundant but harmless)
   - Boot bugfix: `inc a` (1 byte) + `ld (0x7800),a` (3 bytes) = **4 bytes NOP** (after the valid 0x7000 write)
   - Verify slots: writes to 0x7800 that need NOPping

The patch uses a struct `Ascii16Patch { int dispIndex; bool isNopSeq; int nopLen; unsigned char replaceByte; }` to encode both operation types.

### Decision 4: No lower bank (0x6000) switching; base addresses only

ASCII16 has two switch addresses: 0x6000 for 0x4000-0x7FFF and 0x7000 for 0x8000-0xBFFF. The kernel code lives at 0x4000-0x7FFF and must always be accessible. The design keeps 0x6000 unused — the kernel page is never switched. Only 0x7000 switches are used for the code/data area at 0x8000-0xBFFF.

All segment-switch writes SHALL target the **base addresses** (0x6000 and 0x7000) exclusively. Mirror addresses (0x6001-0x67FF, 0x7001-0x77FF) SHALL NOT be used, as they may not be implemented on all ASCII16 cartridge hardware and could cause unpredictable behavior.

**Alternatives considered:**
- *Support lower bank switching for data access*: Rejected — would require new kernel dispatch routines to coordinate both banks, adding complexity disproportionate to benefit. The current resource system uses segment numbers for addressing, and 16KB segments already cover the entire 0x8000-0xBFFF region.

### Decision 5: OPENMSX autodetection unchanged

The kernel writes to 0x7000 five times for OPENMSX autodetection. ASCII16 uses 0x7000 as a valid switch address, so the existing OpenMSX detection pattern works. OpenMSX may auto-detect as ASCII8 rather than ASCII16, but both mappers are functionally compatible with the generated ROM format.

### Decision 6: Same padding and ROM size constraints

ROM remains padded to a multiple of 8 pages (128KB), same as all other MegaROM modes. Max ROM size remains 2048KB. The `pageCount % 8` padding logic in `writeRom()` is unchanged.

### Decision 7: New dispatch table entries and DISP_ constants

Add labels in `61_megarom.asm` and `20_runtime.asm` at each patch point, add corresponding `dw label_name` entries in the `wrapper_routines_map_table`, and add `DISP_ASCII16_PATCH_*` constants in `compiler_hooks.h`. This follows the exact pattern established by `DISP_KONAMI_PATCH_*`.

### Decision 8: Compiler skip preamble for ASCII16

When a 16KB segment fills up, the compiler emits a jump preamble to continue in the next segment:

ASCII8 (8KB pairs): 8-byte preamble (code) / 3-byte marker (data)
```
ld a, segm        ; 2 bytes
ld hl, 0x8000     ; 3 bytes
jp MR_JUMP        ; 3 bytes
```

ASCII16 (16KB single): 4-byte preamble
```
ld a, segm        ; 2 bytes
jp MR_JUMP        ; 2 bytes
```

The `ld hl, 0x8000` is unnecessary because the target address is always 0x8000 in the next segment. The compiled code at 0x8000 naturally continues execution. The `jp MR_JUMP` is 2 bytes instead of 3 because... wait, `jp nn` is 3 bytes (`C3 nn nn`), not 2. That's correct.

So the ASCII16 preamble is: `{0x3E, segm, 0xC3, mr_jump_target_lo, mr_jump_target_hi}` = 5 bytes. Let me reconsider.

Actually, the ASCII8 preamble is 8 bytes total: 2 (ld a) + 3 (ld hl) + 3 (jp) = 8.
For ASCII16, we only need: 2 (ld a, segm) + 3 (jp MR_JUMP) = 5 bytes.

After the preamble, filler bytes pad to 0x4000 boundary. The preamble step is subtracted from available space.

For data items in ASCII8, the filler marker is `0xFF, 0xFF, 0xFF` (3 bytes). For ASCII16, data items don't need a separate marker since segment boundaries are aligned to 16KB page boundaries and data always starts at 0x8000. Actually no — when crossing a segment boundary for data, we still need to jump to the next 0x8000 location. The MR_JUMP destination does `jp (hl)` which expects HL to point to the target address. For data reads, MR_GET_DATA handles this differently — it switches segments, reads, and switches back.

Wait, looking at the compiler code more carefully, the skip preamble for data is `{0xFF, 0xFF, 0xFF}` which is a marker in the compiled code, not an executable instruction. The fixup resolver later handles this. For ASCII16, since the segment is 16KB instead of 8KB+8KB, there's no "second half of the page" data item concern. Data items always have `addr_within_segm < 0x4000` because a single 16KB segment fills the entire 0x8000-0xBFFF range.

Actually, for the simplest approach: when a data item crosses the 16KB boundary in ASCII16 mode, we still need a marker since we need a new segment. But the marker just needs to signal "new segment" — and the data filler can be simpler since there's no 0xA000 split.

Hmm, actually looking at the code more carefully, the data filler is used when code also fills the same segment. For ASCII16 with 16KB segments, the data filler for crossing segment boundary would be:
- `0xFF` (2 bytes? Or maybe 1 byte?)

Let me just note this in the design at a higher level and leave exact byte counts for implementation, since the actual values will be verified against the kernel's MR_JUMP conventions.

**Correction on preamble size**: Looking at the actual compiler code, the ASCII16 code-skip preamble would be:
```
ld a, segm_last    ; 2 bytes
jp MR_JUMP         ; 3 bytes (C3 nn nn)
```
Total: 5 bytes. The `ld hl, 0x8000` is not needed because after MR_JUMP switches to the new segment and does `ret`, the code at 0x8000 is the entry point. The compiled code starts at 0x8000 in every segment.

### Decision 9: CLI flag `-6` / `--ascii16`

Short flag `-6` (mnemonic: "6" looks like "16" compressed, or ASCII "6" = ASCII16). Long flag `--ascii16`.
- `-a` is taken by `--auto`
- `-x` and `-8` are taken by ASCII8 variants
- `-k` is taken by `--scc`
- `-4` is taken by `--konami`
- `-6` is unused

## Risks / Trade-offs

- **[Risk] OpenMSX may not distinguish ASCII16 from ASCII8 in auto-detection** → Mitigation: The ROM functions correctly on both mapper types since only 0x7000 writes are used (valid for both). Users targeting specific ASCII16 flash cartridges should test on hardware.
- **[Risk] NOP-patching alters instruction sequence lengths** → Mitigation: NOP positions are chosen so that no code path is skipped or entered mid-sequence. `MR_CHANGE_SGM` is the only routine affected, and NOP lengths match exactly the replaced instruction lengths.
- **[Risk] Compiler segment math changes are scattered across ~8 sites** → Mitigation: Each site is a small conditional branch, mechanically verifiable. Integration tests will validate the full pipeline.
- **[Risk] ASCII8 bugfix initialization sets wrong segment on ASCII16** → Mitigation: The dispatch table patch diverts the 0x6800 write to 0x7000, and NOPs prevent the value from being incremented incorrectly. The first code page is accessed via segment 1 (0x7000=1), which is correct.
- **[Risk] `inc a` NOP in bugfix leaves A=2 for the subsequent valid 0x7000 write** → Mitigation: The `inc a` between the diverted 0x6800→0x7000 write and the original 0x7000 write is NOPped, so A stays at 1. Both writes put 1 in 0x7000 (segment 1), redundant but correct.

## Open Questions

None — all design decisions are resolved.
