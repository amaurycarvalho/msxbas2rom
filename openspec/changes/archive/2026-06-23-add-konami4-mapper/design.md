## Context

msxbas2rom currently supports three ROM formats: Plain ROM, ASCII8 MegaROM, and KonamiSCC MegaROM. The ASCII8 and KonamiSCC mappers share a common kernel (`61_megarom.asm`) with two segment-switch addresses (`Seg_P8000_SW` and `Seg_PA000_SW`). The difference between ASCII8 and KonamiSCC is handled by a binary patch (`fixIfKonamiSCC()`) that replaces 0x7000/0x7800 with 0x8000/0xA000 in the compiled kernel.

The Konami4 mapper (Konami without SCC) uses the SAME upper-bank switch addresses as KonamiSCC (0x8000 for Bank 3 at 0x8000-0x9FFF, 0xA000 for Bank 4 at 0xA000-0xBFFF). This means the existing kernel patch is directly reusable.

The project follows Clean Architecture with 4 layers: `domain/`, `application/`, `infrastructure/`, `cli/`. Dependencies flow inward. The `fixIfKonamiSCC()` method lives in `src/application/builder/rom.cpp`.

## Goals / Non-Goals

**Goals:**
- Add `Konami4` as a first-class `CompileMode` option
- Reuse the existing binary kernel patch (no new kernel code needed)
- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` to reflect generic Konami support
- Provide CLI flags `-4` and `--konami`
- Produce ROMs with `[Konami]` filename suffix
- Unit and integration test coverage

**Non-Goals:**
- No SCC sound chip support (that's KonamiSCC)
- No new kernel assembly or Z80 routines
- No changes to ROM page layout or alignment logic
- No changes to resource mapping or MegaROM detection
- No auto-detection of Konami4 vs KonamiSCC at the kernel level (future concern)

## Decisions

### Decision 1: Reuse `fixIfKonamiSCC()` patch, rename to `fixKonamiMapper()`

Both KonamiSCC and Konami4 use identical segment-switch addresses for the upper two banks (0x8000 and 0xA000). The existing binary patch logic is the same for both. The patch is applied via dispatch table entries (DISP_KONAMI_PATCH_*) that resolve to exact instruction addresses in the kernel, covering all 14 segment-switch points. The rename is mechanical — method name, declaration comment in rom.h, error message string in rom.cpp, and the call site in `addKernel()` all change.

**Alternatives considered:**
- *Duplicate the method as `fixIfKonami4()`*: Rejected — causes code duplication without benefit.
- *Keep name `fixIfKonamiSCC()` and just add `|| Konami4` condition*: Rejected — name becomes misleading ("SCC" in the name but not SCC-specific).
- *Rename to `fixKonamiMapper()`*: **Chosen** — communicates the method handles all Konami mappers that share this switch scheme.

### Decision 2: Where to add Konami4 in the `CompileMode` enum

Add `Konami4` before `KonamiSCC` in the enum, maintaining alphabetical order ('4' < 'S' in ASCII).

```cpp
enum class CompileMode { Plain, ASCII8, ASCII16, ASCII16X, Konami4, KonamiSCC, Pcoded };
```

### Decision 3: CLI flag design

Short flag `-4` (mnemonic: Konami4) and long flag `--konami` (descriptive).
- `-k` is already taken by `--scc` (KonamiSCC).
- `-4` is unused and intuitive (4 banks, Konami4).

### Decision 4: Integration test approach

Rather than creating new test programs, reuse existing programs in `tests/integration/MEGAROM/` by compiling them with `--konami` in addition to the default `-a` (auto mode). This validates that existing MegaROM-capable programs work correctly in Konami4 mode.

### Decision 5: No kernel OpenMSX autodetection changes

The kernel's `OPENMSX_EMULATOR_AUTODETECTION` section writes to the segment switch address to help openMSX identify the mapper. For ASCII8, the original writes go to 0x7000; after the Konami patch, they go to 0x8000. Both KonamiSCC and Konami4 use 0x8000, so no kernel change is needed. OpenMSX may detect the mapper as KonamiSCC rather than Konami4, but this is acceptable — both mappers function identically for the generated ROM.

### Decision 6: Dispatch table-based patching instead of byte scanning

Instead of scanning the binary kernel for byte patterns (`LD (0x7000),A` / `LD A,(0x7000)`), the patch uses wrapper_routines_map_table entries. Each segment-switch write instruction has a label (e.g., `konami_patch_sgm_8000`) in the kernel ASM, a `dw` entry in the dispatch table, and a `DISP_KONAMI_PATCH_*` constant in compiler_hooks.h. The `fixKonamiMapper()` function iterates over these constants, resolves each label's kernel address from the dispatch table, and patches the high byte of the address operand directly.

**Advantages:**
- No fragile byte-pattern scanning; patch locations are explicit and compile-time verified
- Catches all 14 segment-switch points including `megarom_ascii8_bug_fix` (previously outside the scanner's start offset 0xDB)
- Patch locations are maintained alongside the kernel code, not derived from binary layout

## Risks / Trade-offs

- **[Risk] OpenMSX may not distinguish Konami4 from KonamiSCC in auto-detection** → Mitigation: The ROM will still function correctly on both mapper types. Users targeting specific Konami4 flash cartridges should test on hardware.
- **[Risk] Method rename `fixIfKonamiSCC` → `fixKonamiMapper` touches rom.h/rom.cpp** → Mitigation: The rename is mechanical and verified by compiler. All callers are in the same class.
- **[Risk] Accidental clean rebuild of all source files** → Mitigation: Avoid touching the kernel header or infrastructure files; only compile-domain code changes trigger re-linking, not recompilation of everything.

## Open Questions

None — all design decisions are resolved.
