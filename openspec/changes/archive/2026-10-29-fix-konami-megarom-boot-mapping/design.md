# Design

## Context

See `proposal.md` - Why. The kernel's `start` routine (`20_runtime.asm:297`)
runs `megarom_ascii8_bug_fix` before jumping to `0x8010`. That block reads
`0x8000` and `0x8001`, expects `AB`, and only then writes the mapper registers.
The guard exists to handle machines whose mapper reset exposes segment 0 at
`0x8000`; on turbo R the same window can expose an empty segment, so the guard
takes the skip branch and leaves the user-code segment unmapped.

The builder already NOPs the same 14-byte guard for ASCII16X via
`DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK` (`rom.cpp:696`). The fix reuses that
mechanism for Konami4 and KonamiSCC.

## Goals / Non-Goals

**Goals:**

- Make Konami4/KonamiSCC boot mapping independent of the mapper's reset state.
- Keep ASCII8 and ASCII16 behavior unchanged.
- Cover the fix with builder unit tests and an emulator matrix that includes
  MSX1, MSX2, MSX2+, and turbo R.
- Keep the new probes independent of `MISC/turbo_test1` and the existing
  turbo-R changes.

**Non-Goals:**

- Changing the kernel assembly or regenerating kernel headers.
- Making ASCII8 boot mapping unconditional (not confirmed broken).
- Changing the mapper layout or the 8 KB segment model.
- Reusing or modifying `MISC/turbo_test1`, `spike-turbo-r-r800-fragility`, or
  `harden-turbo-r-r800-compatibility`.

## Decisions

### D1: Patch the guard in the builder, not the kernel

Add the 14-byte NOP to `Rom::fixKonamiMapper()` for `Konami4` and `KonamiSCC`.
This mirrors the ASCII16X implementation and avoids changing shared kernel code
that ASCII8/ASCII16 still use. Alternative: remove the guard from
`20_runtime.asm`; rejected because it would change every mapper and require
kernel regeneration.

### D2: Use a mapper-neutral local alias

Define `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` as an alias of the generated
`DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK` dispatch index at the builder call sites.
The generated dispatch table keeps index 223, so no kernel or manifest
regeneration is needed. Alternative: rename the dispatch symbol end-to-end;
rejected for this fix because it touches generated headers, symbol baselines,
and the ASCII16X path for no behavioral gain. The alias keeps the intent clear
without that churn.

### D3: Patch only Konami4 and KonamiSCC

ASCII8 and ASCII16 keep the guard because they are not confirmed broken and
their boot paths already work on turbo R in the spike. ASCII16X keeps its
existing NOP. If ASCII8 later shows the same reset-state dependency, it can be
made unconditional in a separate change.

### D4: Add compile-mode and machine matrices to the regression manifest

Extend the program config with:

- `compile_modes`: list, default `["ascii8"]`, mapping to the existing CLI flags
  and ROM suffixes.
- `machines`: list of machine classes, default to the existing
  inference/`requires` behavior.

The runner expands the Cartesian product into variants. Each variant gets a
unique label (for example `MISC/megaboot [konami@msx1]`) and a unique baseline
path. Alternative: duplicate one `.bas` file per machine/mapper; rejected
because it multiplies files and hides the intent.

### D5: Correct the MSX2 default

Set `msx2_machine` to `Panasonic_FS-A1` and keep `msx2plus_machine` as
`Panasonic_FS-A1WX` (`runner.py:54-55`). No existing manifest program declares
`requires: msx2`, so this only affects the new matrix and the runner unit test
that currently expects the old value. Alternative: per-program `machine`
overrides; rejected because the default is semantically wrong and the matrix is
the right place to fix it.

### D6: Two new probes, no reuse of `turbo_test1`

- `MISC/megaboot.bas`: machine-independent, prints `MEGAROM BOOT OK` and
  `SUM=5050`. It exercises the boot mapping, FOR/NEXT, integer math, and PRINT,
  and its output is identical on MSX1 through turbo R.
- `MISC/turbo_konami.bas`: turboR-focused; runs the same workload with
  `CMD TURBO 0` and `CMD TURBO 1` and prints `TURBO KONAMI OK`.

Both are compiled in Konami4 and KonamiSCC modes. `megaboot` runs on the full
four-machine matrix; `turbo_konami` runs on `Panasonic_FS-A1GT` only. This keeps
the probes separate from `MISC/turbo_test1` and the other turbo-R changes.

### D7: Split verification by layer

- Builder unit tests assert the patched bytes directly: 14 NOPs for both Konami
  modes, guard intact for ASCII8/ASCII16, and Konami bank writes still present.
- Runner unit tests assert compile-mode argument/suffix mapping, matrix
  expansion, unique labels/baselines, and the corrected MSX2 default.
- Emulator regression asserts the stable text on each matrix variant.

## Risks / Trade-offs

- **Real-hardware differences** -> The spike used openMSX turbo R machines; the
  regression runs there too. Keep a real `Panasonic_FS-A1GT` validation as a
  manual acceptance step before release.
- **Other Konami ROM reset states** -> Always writing the banks is idempotent
  for segment 0, segment 2, and empty reset states. Existing MSX1/MSX2/MSX2+
  behavior was verified with a patched build.
- **Runner matrix complexity** -> Variant labels and baseline paths must include
  mode and machine. Add runner unit tests before wiring the probes.
- **MSX2 default change** -> Only the new matrix and the runner test are
  affected today. Review the manifest for any future `requires: msx2` entries.
- **Fallback captures** -> A broken boot never reaches `END`; the matrix probes
  use a fallback long enough to capture the crashed state and fail on missing
  text.

## Migration Plan

1. Patch `Rom::fixKonamiMapper()` and add the local alias.
2. Add builder unit tests and run the unit suite.
3. Extend the runner manifest schema, config expansion, and unit tests.
4. Correct the MSX2 default and update the runner test.
5. Add `MISC/megaboot.bas` and `MISC/turbo_konami.bas` plus manifest entries.
6. Run the matrix regressions and review the artifacts.
7. Run `make test`, `make test-integration`, and `make test-architecture`.
8. Roll back by reverting the builder patch and removing the manifest entries;
   no kernel artifacts are involved.
