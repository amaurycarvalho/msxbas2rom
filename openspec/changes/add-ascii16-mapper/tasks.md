## 1. Domain Layer — CompileMode Enum and Output Naming

- [ ] 1.1 Add `ASCII16` to `CompileMode` enum after `ASCII8` (`src/domain/options/build_options.h`)
- [ ] 1.2 Add `ASCII16` to `setInputFilename()` condition for `[ASCII16]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp`)
- [ ] 1.3 Add `ASCII16` case to `getCompileModeShortName()` returning `"ASCII16"` (`src/domain/options/build_options.cpp`)
- [ ] 1.4 Add `ASCII16` case to `getCompileModeLongName()` returning `"ASCII16 MegaROM"` (`src/domain/options/build_options.cpp`)

## 2. Kernel Assembly — Dispatch Table Labels and Entries

- [ ] 2.1 Add `ascii16_patch_*` labels at each NOP-sequence and byte-replace point in kernel ASM:
  - `ascii16_patch_sgm_nopseq` before `inc a` in MR_CHANGE_SGM (`61_megarom.asm`): 5-byte NOP sequence
  - `ascii16_patch_bugfix_inc1` before `inc a` after patched `ld (0x6800),a` in boot bugfix (`20_runtime.asm`): 1-byte NOP
  - `ascii16_patch_bugfix_nopseq` before `inc a` after valid `ld (0x7000),a` in boot bugfix (`20_runtime.asm`): 4-byte NOP sequence
- [ ] 2.2 Add `dw ascii16_patch_*` entries in `wrapper_routines_map_table` (`20_runtime.asm`)
- [ ] 2.3 Add `DISP_ASCII16_PATCH_*` constants in `compiler_hooks.h` (`src/application/compiler/helpers/hooks/compiler_hooks.h`)
- [ ] 2.4 Update `DISP_ENTRIES` to reflect new entries (207 → 210)
- [ ] 2.5 Regenerate `header.h` by running `make header` (pasmo + xxd -i)

## 3. CLI Layer — Flags, Help Text, and Status Messages

- [ ] 3.1 Add `-6`/`--ascii16` CLI flags setting `compileMode = CompileMode::ASCII16` (`src/cli/options/build_options_setup.cpp`)
- [ ] 3.2 Add `-6 or --ascii16 = ASCII16 MegaROM compile mode` entry to help text in `src/cli/appinfo.h` (info_help, under compile options)
- [ ] 3.3 Update compile-mode info message in `src/cli/main.cpp` to show `"ASCII16"` for ASCII16 mode
- [ ] 3.4 Update summary message in `src/cli/main.cpp` to distinguish ASCII16 from other MegaROM modes

## 4. Application Layer — ROM Builder (fixAscii16Mapper)

- [ ] 4.1 Declare `fixAscii16Mapper()` method in `src/application/builder/rom.h`
- [ ] 4.2 Implement `fixAscii16Mapper()` in `src/application/builder/rom.cpp`:
  - Define `Ascii16Patch` struct with `dispIndex`, `isNopSeq`, `nopLen`, `replaceByte`
  - Define `patches[]` array covering all dispatch table entries
  - For each patch: resolve kernel address from dispatch table, compute page offset
  - For byte-replace patches: write `replaceByte` at appropriate offset
  - For NOP-sequence patches: write `0x00` for `nopLen` bytes from label address
  - Verify all patches applied (patched count matches expected)
  - Log error if patch count mismatch
- [ ] 4.3 Add `resourceSegment` conditional for ASCII16: `pages.size()` (not `pages.size() * 2`) in `addResources()` (`src/application/builder/rom.cpp`)
- [ ] 4.4 Call `fixAscii16Mapper()` in `addKernel()` when `compileMode == ASCII16`

## 5. Application Layer — Compiler Segment Math (ASCII16)

- [ ] 5.1 Initialize `segm_last = 1` (not 2) for ASCII16 in `compiler.cpp` write() method
- [ ] 5.2 Initialize `segm_total = 3` (not 4) for ASCII16 (segments 0, 1, and initial space for 2)
- [ ] 5.3 Branch on ASCII16 for `segm_last += 1` (instead of `+= 2`) when crossing 16KB boundary
- [ ] 5.4 Conditional segment-skip preamble: 5 bytes (`ld a,seg; jp MR_JUMP`) for ASCII16, 8 bytes (`ld a,seg; ld hl,0x8000; jp MR_JUMP`) for others
- [ ] 5.5 Remove `if (addr_within_segm >= 0xA000) codeItem->segm++` for ASCII16 (no 8KB split)
- [ ] 5.6 Branch on ASCII16 for `segm_total` rounding: `((last+1)/8+1)*8` for ASCII16, `((last+1)/16+1)*16` for others
- [ ] 5.7 Branch on ASCII16 for fixup `segm_from` calculation: `((address + start_address - 0x8000) / 0x4000) * 1 + 1`
- [ ] 5.8 Branch on ASCII16 for fixup `segm_to` calculation: same formula with target address

## 6. Unit Tests

- [ ] 6.1 Add test cases for `-6` and `--ascii16` CLI flags in `tests/unit/src/test_options.cpp` (verify compileMode = ASCII16, megaROM = true, output filename contains `[ASCII16]`)
- [ ] 6.2 Add test case for ASCII16 ROM build with patched kernel (verify `fixAscii16Mapper` patches all locations via dispatch table, NOP sequences present) in `tests/unit/src/test_rom.cpp`

## 7. Integration Tests

- [ ] 7.1 Update `tests/integration/Makefile` to compile MEGAROM test programs with `--ascii16` flag
- [ ] 7.2 Run integration tests and verify no regressions

## 8. Verification

- [ ] 8.1 Run `make test-unit` and verify all unit tests pass
- [ ] 8.2 Run `make test-integration` and verify all integration tests pass
- [ ] 8.3 Run `make release` and verify build succeeds without warnings
- [ ] 8.4 Manual verification: compile a test program with `--ascii16` and verify output ROM byte pattern at known segment-switch offsets (NOPs in MR_CHANGE_SGM, patched boot bugfix)
- [ ] 8.5 Manual verification: test ASCII16 ROM in openMSX emulator
