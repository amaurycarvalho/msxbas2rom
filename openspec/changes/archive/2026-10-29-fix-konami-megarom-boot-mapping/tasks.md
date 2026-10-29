# Tasks

## 1. Builder fix

- [x] 1.1 Add the mapper-neutral alias `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` at the builder call site and make it resolve to the shared guard dispatch index used by ASCII16X.
- [x] 1.2 In `Rom::fixKonamiMapper()`, NOP the 14 bytes at `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` for Konami4 and KonamiSCC, reusing the existing offset validation and failure reporting; verify ASCII8 and ASCII16 still keep the guard.
- [x] 1.3 Confirm no kernel source or generated kernel artifact change is needed, and run the kernel size and architecture checks.

## 2. Builder unit tests

- [x] 2.1 Add a unit test asserting Konami4 and KonamiSCC ROMs contain `0x00` in all 14 bytes at the shared guard offset.
- [x] 2.2 Add assertions that the Konami `0x7000`, `0x8000`, and `0xA000` bank writes remain present after the NOP patch.
- [x] 2.3 Add negative checks that ASCII8 and ASCII16 ROMs still contain the original 14-byte `AB` guard sequence.
- [x] 2.4 Run the unit test target and verify the new and existing ROM builder tests pass.

## 3. Regression runner matrices

- [x] 3.1 Extend the manifest schema and loader with optional `compile_modes` and `machines` arrays, defaulting to `ascii8` and the existing machine inference.
- [x] 3.2 Expand each program into one variant per compile-mode and machine-class combination, with distinct labels and baseline paths.
- [x] 3.3 Map each compile mode to the correct compiler flag and ROM/`.noi` suffix (`[Konami]`, `[KonamiSCC]`, and the existing ASCII8/ASCII16 names).
- [x] 3.4 Correct the true MSX2 default to `Panasonic_FS-A1`, keep `Panasonic_FS-A1WX` for MSX2+, and update the runner unit test that asserts the old MSX2 default.
- [x] 3.5 Add runner unit tests for mode/flag/suffix mapping, matrix expansion, unique labels/baselines, and the corrected MSX2 default.
- [x] 3.6 Run the regression runner unit tests and verify they pass.

## 4. Regression probes

- [x] 4.1 Create `tests/integration/MISC/megaboot.bas` that exercises FOR/NEXT, integer math, and PRINT and prints `MEGAROM BOOT OK` and `SUM=5050`; verify it compiles in Konami4 and KonamiSCC modes.
- [x] 4.2 Create `tests/integration/MISC/turbo_konami.bas` that switches turbo off and on and prints `TURBO KONAMI OK`; do not reuse or modify `turbo_test1`.
- [x] 4.3 Add a `MEGAROM_BOOT` manifest entry for `megaboot` with `compile_modes: ["konami", "konamiscc"]`, `machines: ["msx1", "msx2", "msx2plus", "turboR"]`, a long enough fallback, `volatile: true`, and expected text `MEGAROM BOOT OK` plus `SUM=5050`.
- [x] 4.4 Add a turboR-focused manifest entry for `turbo_konami` with `compile_modes: ["konami", "konamiscc"]`, `machines: ["turboR"]`, `volatile: true`, and expected text `TURBO KONAMI OK`.
- [x] 4.5 Verify the corrected MSX2 default selects `Panasonic_FS-A1`, MSX2+ selects `Panasonic_FS-A1WX`, and turbo R selects `Panasonic_FS-A1GT`.

## 5. Integration verification

- [x] 5.1 Run the Konami/KonamiSCC boot probe matrix on MSX1, MSX2, MSX2+, and turbo R and verify every variant passes with the patch.
- [x] 5.2 Run the turboR Konami/KonamiSCC probe and verify `TURBO KONAMI OK` is captured for both modes.
- [x] 5.3 Run `make test-integration` and verify the new programs compile in the existing integration matrix.
- [x] 5.4 Run `make test-architecture` and verify the dependency and composition checks pass.
- [x] 5.5 Review the generated artifacts and baselines and confirm the machine classes and mapper modes are recorded correctly.
- [x] 5.6 Validate the change with `openspec validate fix-konami-megarom-boot-mapping`.

## Workflow follow-up

- Run a final acceptance check on real `Panasonic_FS-A1GT` hardware when available.
- Archive the change after the matrix and baselines are reviewed.
