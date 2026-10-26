# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [add-16bit-segments-ascii16-4mb-ascii16x-8mb](openspec/changes/add-16bit-segments-ascii16-4mb-ascii16x-8mb) 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB ROMs
- [add-define-def-fn-preprocessor](openspec/changes/add-define-def-fn-preprocessor) Compile-time macro support via DEFINE and DEF FN preprocessor
- [fix-dim-position-order](openspec/changes/fix-dim-position-order) Fix DIM position order so array references before DIM work correctly
- [implement-double-via-float-float](openspec/changes/implement-double-via-float-float) Double precision operations via float emulation
- [refactor-resource-number-hl-protocol](openspec/changes/refactor-resource-number-hl-protocol) Resource number passing standardized to HL register protocol
- [set-page-screen4](openspec/changes/set-page-screen4) SET PAGE support for screen 4 compatibility
- [wishlist](openspec/changes/wishlist) Explore and track potential future improvements not yet scheduled for any sprint

### [add-rsctoram-offset-size](openspec/changes/archive/2026-10-26-add-rsctoram-offset-size) Add optional offset/size parameters to CMD RSCTORAM for partial resource copies

#### Added
- Add optional `offset` and `size` parameters to `CMD RSCTORAM`

#### Changed
- Clamp the effective copy length to `min(size, resource_size - offset)`
- Treat an omitted `size` as the full remaining resource
- Ignore `offset` and `size` when `pletter = 1` and decompress the whole resource as before
- Return `HL = dest` on success and `HL = 0` when the resource is missing or nothing is copied
- Pass command parameters through the `PARM1` work area
- Preserve existing 2- and 3-argument `CMD RSCTORAM` behavior

### [redefine-resource-function](openspec/changes/archive/2026-10-26-redefine-resource-function) Redefine RESOURCE(n) to load the resource into free RAM and return a usable pointer

#### Changed
- Redefine `RESOURCE(n)` to copy the resource into `HEAP()` via `CMD RSCTORAM` and return a directly usable RAM pointer
- Return `0` when the resource does not exist and truncate the copy to `FRE()` bytes when the resource is larger
- Keep `RESOURCESIZE(n)` and the existing `DISP_cmd_rsctoram` dispatch unchanged

## [1.4.0.0] - 2026-10-26

### [2026-10-23-expand-kernel-central-jump-table](openspec/changes/archive/2026-10-23-expand-kernel-central-jump-table) Introduce a runtime central jump table and relocatable module packer for the kernel

#### Added
- Add a runtime central jump table in the fixed kernel page, with `jp` stubs for resident entries and `{segment, address}` dispatcher stubs for paged entries
- Add a module manifest (embedded in `msxbas2rom`) describing module ids, exports, imports, dependencies, placement policy, fixed regions, and patch points
- Add embedded relocation metadata for pre-assembled modules so BASIC compilation never invokes an external assembler, linker, or data file
- Add cross-module import fixups for calls and address-of pointers so fixed and relocatable modules never embed a movable module's address
- Add a build-time packer that reserves boot/dispatcher/traps/player regions, computes the used-module closure, fills kernel free holes first, and sends only overflow to the user page or MegaROM segments
- Add unit, kernel-regression, and emulator tests for dispatching, nested calls, interrupts, traps, relocations, and every supported mapper

#### Changed
- Enforce module isolation and page safety: modules that map resource data into `0x8000` while executing stay fixed, and forced overflow of a fixed module fails with a diagnostic
- Track the used `DISP_*` set during compilation and defer kernel call resolution to dispatch fixups, with the packer generating selective stubs and patching call sites
- Classify routines into fixed resident, flex high priority, flex low priority, and overflow
- Keep player/Arkos fixed-resident at `0x6481-0x6BD5` and keep the timer interrupt handler resident
- Split `90_support.asm` into graphics, math, strings, print, logic, and trap subgroups, pinning trap/interrupt-critical routines resident
- Keep `DISP_*` indices stable while replacing direct absolute kernel calls with stubs/fixups
- Update Plain ROM and MegaROM placement, budget validation, and diagnostics
- Internal breaking change: kernel routines may move out of the fixed page, flex-routine addresses in `header.symbols.asm` are no longer build constants, and `header.bin` is no longer the sole runtime kernel image

### [2026-10-26-accelerate-test-emulator](openspec/changes/archive/2026-10-26-accelerate-test-emulator) Accelerate openMSX regression captures with pre-boot breakpoints and conditional checks

#### Added
- Add a shared helper for `tests/dispatcher/` runtime checks that runs openMSX accelerated when no host input script is present, installs pre-boot breakpoints, and preserves the emulated-time delay as a fallback
- Add an explicit acceleration option to the `msx-memory-debug` skill
- Add optional breakpoint conditions (`LABEL=ADDRESS@CONDITION`) so MegaROM checks can require the correct mapper block before capturing

#### Changed
- Compile BASIC runtime-check programs with `-s --noi`, resolve `LIN_<line>` from `.noi` symbols, mask addresses with `& 0xFFFF`, and derive the mapper block from the symbol's segment
- Install the terminal-state breakpoint on the first line of the marker loop after the program has written its RAM marker
- Export and parse the pasmo `loop` symbol for the hand-written dispatcher prototype
- Preserve every existing check, marker address, machine selection, headless SDL behavior, and pass/fail result
- Keep `make test-regression`, product runtime behavior, and parallel emulator execution unchanged

### [2026-10-26-add-emulator-memory-regression-tests](openspec/changes/archive/2026-10-26-add-emulator-memory-regression-tests) Add an openMSX-based memory regression workflow for integration test programs

#### Added
- Add a `regression` Make target in `tests/integration/Makefile`, independent from the existing `run`/`test-integration` flow
- Add a regression runner and manifest under `tests/integration/regression/`
- Group programs into scenarios by feature folder and allow running a whole scenario or a single program
- Compile selected programs in ASCII8 mode by default, with `--noi` symbols and a dedicated output directory
- Execute ROMs in openMSX with deterministic capture: `END_PGM`/`END_STMT + 2` breakpoints, configurable emulated-time fallback, and automated keyboard/joystick input
- Validate RAM probes and stable RAM snapshots, VRAM text/name-table checks, and VRAM region hashes, using screenshots only as diagnostics
- Add baseline generation/refresh with stable-byte masking and volatile-program handling

#### Changed
- Extend the `msx-memory-debug` skill with VRAM capture, pre-boot breakpoints, input injection, and fallback capture
- Keep the regression workflow out of CI/CD

### [2026-10-26-add-regression-vram-probes](openspec/changes/archive/2026-10-26-add-regression-vram-probes) Add declarative VRAM probes to the regression manifest

#### Added
- Add `vram.probes` to the regression manifest schema and runner
- Add unit tests and document the manifest field

#### Changed
- Let each probe declare an absolute VRAM offset and expected `value` or `bytes`
- Evaluate VRAM probes before the volatile short-circuit so volatile programs can still assert exact VRAM state
- Report probe failures with the VRAM offset and expected/actual bytes

### [2026-10-26-add-turbo-r-regression-coverage](openspec/changes/archive/2026-10-26-add-turbo-r-regression-coverage) Add turbo R regression coverage and machine requirements

#### Added
- Add stable text expectations for `MSX()` and turbo output labels, marking timing-dependent values volatile
- Add a turbo R gate that verifies `MSX() = 3` and that `CMD TURBO` continues

#### Changed
- Mark the turbo R feature programs (`MISC/test1`, `MISC/test64`, `MISC/test65`) with `requires: turboR` so they run on `Panasonic_FS-A1GT`
- Keep performance-only users of `CMD TURBO`/`CALL TURBO` on MSX2+ unless they declare a turbo R requirement

### [2026-10-26-fix-put-sprite-or-color](openspec/changes/archive/2026-10-26-fix-put-sprite-or-color) Preserve MSX2 sprite color bits in PUT SPRITE

#### Added
- Add `tests/integration/GRAPH/sprite3.bas` to the regression workflow with a VRAM probe for sprite 1's 16 color-table bytes at `0x6C`

#### Changed
- Preserve bits 5 and 6 of the PUT SPRITE color parameter when writing the sprite color table on MSX2 and later
- Keep the MSX1 path limited to the 4-bit sprite color
- Generate a `GRAPH/sprite3` baseline after the fix

### [2026-10-26-fix-regression-boot-timing](openspec/changes/archive/2026-10-26-fix-regression-boot-timing) Enforce a per-machine-class minimum boot delay for regression captures

#### Added
- Add a per-machine-class minimum boot delay for fallback captures

#### Changed
- Default MSX2+, MSX2, and turbo R to at least 15 emulated seconds
- Keep program-specific `fallback_seconds` as the capture delay but use `max(fallback_seconds, machine_boot_seconds)` for the effective fallback
- Allow the machine boot minimum to be overridden in the manifest
- Document the timing rule and add runner tests for the effective value

### [2026-10-26-fix-regression-end-pgm-capture](openspec/changes/archive/2026-10-26-fix-regression-end-pgm-capture) Capture terminal state at the pre-cleanup END entry

#### Added
- Add regression checks that a graphics-mode program captured at END reports the graphics mode and uses the correct VRAM region layout

#### Changed
- Capture terminal state at `END_STMT + 2` for both explicit and implicit program termination
- Use `END_PGM` only as a fallback when `END_STMT` is unavailable
- Report a clear diagnostic when the captured VDP mode conflicts with the source-declared screen mode
- Keep the source fallback only for SCREEN 9-12 where VDP registers cannot distinguish the modes

### [2026-10-26-fix-regression-machine-selection](openspec/changes/archive/2026-10-26-fix-regression-machine-selection) Select regression machines by class and VDP screen mode

#### Added
- Introduce machine classes `msx1`, `msx2`, `msx2plus`, and `turboR` with configurable defaults; MSX2/MSX2+ use `Panasonic_FS-A1WX`
- Add an explicit `requires` manifest field while keeping `machine` as the highest-precedence override
- Add unit tests for parsing, machine selection, and VDP mode decoding

#### Changed
- Parse `SCREEN` literals with or without whitespace
- Infer MSX2+ for `SCREEN >= 4` or turbo-capable commands and require an explicit `requires: turboR` for turbo R programs
- Decode V9938/V9958 screen modes from VDP registers, using the source's last SCREEN for SCREEN 9-12
- Fail a regression run when the baseline machine differs from the selected machine
- Expose each program's selected machine and class in the audit/list output and run report

### [2026-10-26-fix-turbo-function-runtime](openspec/changes/archive/2026-10-26-fix-turbo-function-runtime) Route TURBO(), VDP(), and MAKER() through kernel dispatch

#### Added
- Add runtime regressions proving `TURBO()`, `VDP()`, and `MAKER()` return numeric values on MSX2+ and turbo R without hanging

#### Changed
- Route `TURBO()`, `VDP()`, and `MAKER()` through the kernel dispatch mechanism instead of a direct absolute call to `USR2 + 1`
- Pass the function selector in `HL` so the `USR2` dispatcher receives the intended function number
- Keep MSX1 behavior: `TURBO()` returns 0 and `CMD TURBO` remains a no-op

### [2026-10-26-incremental-local-lint](openspec/changes/archive/2026-10-26-incremental-local-lint) Run local lint incrementally while keeping CI full-tree

#### Added
- Add `make lint-all` for a full local lint run

#### Changed
- Keep `make lint` full-tree when `CI=true` so the CI workflow is unchanged
- Locally, lint only changed `src/**/*.cpp` files (working tree versus `HEAD`, including untracked files) and exit successfully with guidance when no C++ source changed
- Point `make quality-gate` at `lint-all` so the local quality gate remains full
- Keep `make lint-full` (full tree with `-fanalyzer`) unchanged
- Document the local/CI split in the contributing guide and the build command reference

#### Removed
- Remove the automatic fallback to full lint when a header or the `Makefile` changes; header and build-flag validation becomes an explicit `make lint-all` or `make quality-gate`

### [2026-10-26-refresh-regression-baselines](openspec/changes/archive/2026-10-26-refresh-regression-baselines) Refresh regression baselines for the new machine classes and capture fixes

#### Added
- Add the new `GRAPH/sprite3` baseline after the OR-color fix

#### Changed
- Regenerate baselines for programs whose machine class changed to MSX2+ or turbo R
- Regenerate baselines whose captured screen mode or VRAM regions changed because of the V9958 decoder and END-capture fixes
- Raise `MEGAROM/test51` fallback time so its 500-line program reaches the pre-cleanup END entry before capture, then regenerate its baseline
- Review every regenerated baseline for selected machine, screen mode, and region/layout changes and reject unexplained diffs
- Run the full local regression and the machine audit

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.4.0.0...HEAD
[1.4.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.4.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
