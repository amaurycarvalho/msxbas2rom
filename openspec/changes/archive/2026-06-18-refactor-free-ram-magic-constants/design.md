## Context

The `ram_memory_perc` calculation in `compiler.cpp:313-322` uses three inline magic constants:

```
5931  — free variable RAM when file I/O is enabled
10534 — free variable RAM without file I/O
0x538 — kernel internal RAM overhead (always added)
```

These were introduced during sprint-3 file-handling (May 2026), with the disk value adjusted from 5690→5931 during development — suggesting empirical calibration rather than derivation from first principles. The values sit within a `//! @todo` block calling for refactoring.

The memory layout is defined in two places:
- **C++ constants**: `compiler_hooks.h` defines `def_RAM_BOTTOM` (0xC038), `def_RAM_TOP` (0xEE5E), `def_RAM_SIZE` (11814)
- **Assembly**: `00_constants_and_workarea.asm` defines the same layout; `20_runtime.asm` sets `HIMEM` at runtime (VARWRK=0xF380 for non-disk, read from BIOS sysvar for disk)

No exact first-principles derivation of 5931/10534/0x538 was found in the codebase. The relationship is not a simple linear function of `def_RAM_SIZE` — the non-disk value plus overhead (11870) exceeds `def_RAM_SIZE` (11814) by 56 bytes, suggesting the percentage intentionally accounts for kernel structures above `HEAPEND`.

## Goals / Non-Goals

**Goals:**
- Replace 3 inline magic constants in `compiler.cpp` with named, documented constants in `compiler_hooks.h`
- Preserve exact numeric behavior (identical `ram_memory_perc` for all inputs)
- Document each constant's semantic meaning and relationship to kernel memory layout

**Non-Goals:**
- Improve or correct the percentage calculation (behavioral change deferred)
- Derive values from assembly first-principles (requires build-system changes, separate effort)
- Change test expectations or user-facing output

## Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Location of constants | `compiler_hooks.h` | Adjacent to existing `def_RAM_*` constants; single source of truth for memory layout |
| Naming pattern | `def_VAR_RAM_NON_DISK`, `def_VAR_RAM_DISK`, `def_KERNEL_RAM_OVERHEAD` | Consistent with existing `def_` prefix; names describe what each value represents, not where it's used |
| Value preservation | Exact copy of 5931, 10534, 0x538 | No evidence the values are wrong; a behavioral change would require separate validation effort |
| Specs | No new or modified specs | This is pure implementation refactoring with zero user-facing behavioral change |

## Risks / Trade-offs

- **Stale values if kernel layout changes** → Constants are now named and documented, making it obvious where to update and what each value means; a future improvement could add a compile-time assertion comparing derived vs. actual
- **No first-principles derivation** → Documented as accepted; full derivation requires exporting assembly EQU values to C++ headers, which is a build-system change beyond this scope
