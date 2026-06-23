## 1. Kernel ASM — 16-bit segment switch routines

- [ ] 1.1 Add `MR_CHANGE_SGM_16` routine to `61_megarom.asm` (accepts HL=16-bit segm, SRL HL, writes L to 0x7000)
- [ ] 1.2 Add `MR_CHANGE_SGM_16X` routine to `61_megarom.asm` (extends 1.1 with address encoding: H=0→0x7000, H=1→0x7100)
- [ ] 1.3 Add `MR_JUMP_16` wrapper routine (`push hl; call MR_CHANGE_SGM_16; ret`)
- [ ] 1.4 Add `MR_CALL_16` wrapper routine (saves HL, calls MR_CHANGE_SGM_16, indirect call, restores HL)
- [ ] 1.5 Add `MR_GET_DATA_16` wrapper routine (saves HL, calls MR_CHANGE_SGM_16, LDIR, restores HL)

## 2. Kernel ASM — Workarea and resource layout

- [ ] 2.1 Expand `RSCMAPSG` from 1 to 2 bytes in `00_constants_and_workarea.asm`
- [ ] 2.2 Shift `RSCMAPT1`, `PLYSGTM`, `BASMEM` by +1 byte to accommodate 2.1
- [ ] 2.3 Update `resource.map.segment` from `db` to `dw` in `start.asm` (0x800D now 2 bytes)
- [ ] 2.4 Shift `BASTEXT` from 0x800E to 0x800F in `start.asm` and `00_constants_and_workarea.asm`

## 3. Kernel ASM — Resource map reader (2-byte segments)

- [ ] 3.1 Update `resource.address` in `80_resources.asm` to read 2-byte segment from map entries
- [ ] 3.2 Change entry size calculation from ×5 to ×6 bytes per resource
- [ ] 3.3 Ensure segment value in DE is passed correctly to `MR_CHANGE_SGM_16`/`MR_CHANGE_SGM_16X`

## 4. Kernel ASM — Dispatch table

- [ ] 4.1 Add `dw` entries for `MR_CHANGE_SGM_16`, `MR_CHANGE_SGM_16X`, `MR_JUMP_16`, `MR_CALL_16`, `MR_GET_DATA_16` to `20_runtime.asm`
- [ ] 4.2 Add corresponding Z80 labels (e.g., `mr_change_sgm_16:`) in `20_runtime.asm` at the correct dispatch positions

## 5. C++ BuildOptions — Helper method

- [ ] 5.1 Add `bool uses16BitSegments() const` method to `BuildOptions` class
- [ ] 5.2 Implement returning `true` for `ASCII16` and `ASCII16X`, `false` otherwise
- [ ] 5.3 Add declaration in `build_options.h`

## 6. C++ Resource Manager — 16-bit segments

- [ ] 6.1 Change resource map entry format from 5 to 6 bytes (WORD segment instead of BYTE)
- [ ] 6.2 Write both bytes of `resourceBlockSegment` to the map in `buildMap()`
- [ ] 6.3 Change MegaROM limit check from `> 255` to `> 65535` for 16-bit modes, keep `> 255` for 8-bit modes
- [ ] 6.4 Update error message to report mode-specific limit (2048K / 4096K / 8192K)

## 7. C++ ROM Builder — Header and kernel patching

- [ ] 7.1 Change `pages[1][13]` to write 2 bytes of `resourceSegment` in `setResourceMapStartAddress()`
- [ ] 7.2 Update `Rom::fixAscii16Mapper()` to apply 16-bit patches for ASCII16 mode (SeqReplace with `MR_CHANGE_SGM_16`)
- [ ] 7.3 Add ASCII16-X 16-bit patching (SeqReplace with `MR_CHANGE_SGM_16X`) in `fixAscii16Mapper()`
- [ ] 7.4 Update ASCII16 boot bugfix patch to use 16-bit compatible NOP sizes
- [ ] 7.5 Update OMSX autodetection patches for 16-bit modes

## 8. C++ Compiler — 16-bit bridges and fixups

- [ ] 8.1 Make `step` variable conditional: `step = opts->uses16BitSegments() ? 9 : 8` for code blocks
- [ ] 8.2 Emit 9-byte bridge for 16-bit modes: `LD HL,<segm>; LD DE,0x8000; JP MR_JUMP`
- [ ] 8.3 Keep 8-byte bridge for 8-bit modes unchanged
- [ ] 8.4 Update `CompilerFixupResolver::addFix()` preamble for 16-bit modes (emit `LD HL, 0` instead of `LD A, 0`)
- [ ] 8.5 Write 2-byte segment in cross-segment fixups for 16-bit modes: `dest[addr-5]` and `dest[addr-4]`
- [ ] 8.6 Resolve MR_JUMP target: use `DISP_MR_JUMP_16` for 16-bit modes, `DISP_MR_JUMP` for 8-bit

## 9. C++ Constants — compiler_hooks.h

- [ ] 9.1 Add `DISP_MR_CHANGE_SGM_16`, `DISP_MR_CHANGE_SGM_16X`, `DISP_MR_JUMP_16`, `DISP_MR_CALL_16`, `DISP_MR_GET_DATA_16` constants
- [ ] 9.2 Update `DISP_ENTRIES` from 223 to 228
- [ ] 9.3 Update `def_BASMEM` if shifted by workarea changes

## 10. Kernel binary regeneration

- [ ] 10.1 Run `make header` to regenerate `header.h` and `header.symbols.asm`
- [ ] 10.2 Verify `BIN_HEADER_BIN_LEN` remains 32768 (filler absorbs new code)
- [ ] 10.3 Verify all dispatch table address offsets resolve correctly

## 11. Unit Tests — Compiler and builder

- [ ] 11.1 Test `BuildOptions::uses16BitSegments()` returns correct values for each compile mode
- [ ] 11.2 Test ASCII16 mode emits 9-byte bridges with `LD HL, <segm16>` opcode prefix (0x21)
- [ ] 11.3 Test ASCII8 mode emits 8-byte bridges with `LD A, <segm8>` opcode prefix (0x3E)
- [ ] 11.4 Test cross-segment fixup writes 2 bytes of segment for 16-bit modes
- [ ] 11.5 Test resource map entry is 6 bytes with WORD segment for 16-bit modes
- [ ] 11.6 Test ROM header has 2-byte resource map segment at 0x800D
- [ ] 11.7 Test MegaROM limit rejects > 255 segments for ASCII8 and > 65535 for ASCII16
- [ ] 11.8 Test 4MB ASCII16 ROM builds without size limit error
- [ ] 11.9 Test 8MB ASCII16-X ROM builds without size limit error

## 12. Unit Tests — Kernel patching

- [ ] 12.1 Test ASCII16 kernel patch contains `MR_CHANGE_SGM_16` byte sequence (SRL H; RR L; ...)
- [ ] 12.2 Test ASCII16-X kernel patch contains `MR_CHANGE_SGM_16X` byte sequence with JR conditional
- [ ] 12.3 Test ASCII8 kernel is NOT patched with 16-bit routines
- [ ] 12.4 Test Konami4 kernel is NOT patched with 16-bit routines

## 13. Integration Tests

- [ ] 13.1 Test full pipeline: BASIC source → ASCII16 4MB ROM → verify kernel patch in output
- [ ] 13.2 Test full pipeline: BASIC source → ASCII16-X 8MB ROM → verify signature + kernel patch
- [ ] 13.3 Test ASCII8 ROM regression: same BASIC source produces identical output as before
- [ ] 13.4 Test Konami4 ROM regression: same BASIC source produces identical output as before
