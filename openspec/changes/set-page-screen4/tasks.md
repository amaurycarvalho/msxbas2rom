## 1. Kernel Assembly — Work Area Constants

- [ ] 1.1 Add page layout constants to `00_constants_and_workarea.asm`: `SCREEN4_PAGE_SIZE` (0x3400), `SCREEN4_PAGE_START` (0x3900), `SCREEN4_PAGE_COUNT` (8), `SCREEN4_ACTIVE_PAGE` work area variable, `SCREEN4_DISPLAY_PAGE` work area variable
- [ ] 1.2 Allocate work area variables for page tracking in WRKARE space

## 2. Kernel Assembly — CMD MTF Dynamic VRAM Fix

- [ ] 2.1 Read and understand `cmd_mtf` in `31_cmd.asm` (lines ~1654-1927) — identify all hardcoded VRAM addresses
- [ ] 2.2 Replace tileset bank addresses (0x0000, 0x0800, 0x1000) with `(GRPCGP)` + offset calculations
- [ ] 2.3 Replace colorset bank addresses (0x2000, 0x2800, 0x3000) with `(GRPCOL)` + offset calculations
- [ ] 2.4 Replace name table address (0x1800) with `(GRPNAM)`
- [ ] 2.5 Verify MTF palette handling (S.SETPLT) does not use VRAM addresses and needs no changes
- [ ] 2.6 Ensure bank offsets (0x800) are computed relative to BIOS variables, not hardcoded

## 3. Kernel Assembly — screen4_set_page Routine

- [ ] 3.1 Create new `screen4_set_page` routine in `90_support.asm` — input: A = display page, B = active page (or A if single param)
- [ ] 3.2 Implement VRAM backup logic: if current page changed and page > 0, LDIRMV from active area to page storage
- [ ] 3.3 Implement VRAM restore logic: LDIRVM from new page storage to active area (patterns 3 banks + colors 3 banks + name table)
- [ ] 3.4 Implement VDP R#2 update for display page via WRTVDP (name table base = 0x1800 for all pages)
- [ ] 3.5 Update BIOS variables: GRPNAM=0x1800, GRPCOL=0x2000, GRPCGP=0x0000, NAMBAS, CGPBAS
- [ ] 3.6 Store display page and active page in work area variables
- [ ] 3.7 Add MSX1 guard: check VERSION, skip on MSX1 (return silently)
- [ ] 3.8 Add invalid page guard: if page >= SCREEN4_PAGE_COUNT, set error and return

## 4. Kernel Assembly — Symbol Export

- [ ] 4.1 Add `screen4_set_page` symbol to `header.symbols.asm` with its runtime address
- [ ] 4.2 Add jump table entry in `header.asm` or `20_runtime.asm` if needed

## 5. Compiler C++ — SET PAGE Redirection

- [ ] 5.1 Add `def_screen4_set_page` define to `compiler_hooks.h`
- [ ] 5.2 Modify `cmd_set_page` in `compiler_set_statement_strategy.cpp`: add runtime SCRMOD check (ld a, (SCRMOD); cp 5)
- [ ] 5.3 For SCRMOD=5: load display page into A, active page into B (or A if single param), call `screen4_set_page`
- [ ] 5.4 For SCRMOD≠5: preserve existing behavior (call `XBASIC_SET_PAGE`, write `ACPAGE`)
- [ ] 5.5 Ensure single-parameter form (SET PAGE n) passes same value for both display and active

## 6. Compiler C++ — Context Tracking (Optional Enhancement)

- [ ] 6.1 Add `screen4CurrentPage` field to `CompilerContext` if needed for compile-time optimizations
- [ ] 6.2 Track page state in compiler context for potential future use (e.g., warning if SET PAGE called without SCREEN 4)

## 7. CMD MTF Integration Fix

- [ ] 7.1 Test CMD MTF after kernel changes (Section 2) to verify tilesets load correctly
- [ ] 7.2 Verify `cmd_wrtscr` uses `(GRPNAM)` correctly when GRPNAM is modified by SET PAGE
- [ ] 7.3 Verify `cmd_wrtchr` uses `(GRPCGP)` correctly after SET PAGE
- [ ] 7.4 Verify `cmd_wrtclr` uses `(GRPCOL)` correctly after SET PAGE

## 8. Unit Tests (tests/unit/)

- [ ] 8.1 Add test for `cmd_set_page` with SCREEN 4 — verify correct kernel dispatch (screen4_set_page vs C70CC)
- [ ] 8.2 Add test for `cmd_set_page` single parameter — verifies both display and active use same value
- [ ] 8.3 Add test for `cmd_set_page` invalid parameter count — verifies error handling
- [ ] 8.4 Add test for `cmd_set_page` screen 5+ backward compatibility — verifies C70CC path unchanged
- [ ] 8.5 Run existing unit test suite to verify no regressions

## 9. Integration Test (tests/integration/GRAPH/)

- [ ] 9.1 Create test .bas file: SCREEN 4, SET TILE ON, load font, define test tiles
- [ ] 9.2 Test SET PAGE display page only: set page 0, draw pattern A; set page 1, draw pattern B; set page 0, verify pattern A still exists
- [ ] 9.3 Test SET PAGE with active page: set page display=0 active=1, CLS and PRINT should NOT affect page 0 display
- [ ] 9.4 Test PUT TILE across pages: SET PAGE 0,2; PUT TILE at (5,5); SET PAGE 0,0; verify tile NOT at (5,5)
- [ ] 9.5 Test TILE() across pages
- [ ] 9.6 Test CMD MTF with SET PAGE: load MTF to page 2, verify tiles NOT on page 0
- [ ] 9.7 Test BLOAD S / SCREEN LOAD across pages
- [ ] 9.8 Test COPY with source/dest pages
- [ ] 9.9 Test invalid page number error handling
- [ ] 9.10 Verify expected output against rom checksum or known-good behavior

## 10. Build & Verification

- [ ] 10.1 Build release binary with `make release` — verify compilation succeeds
- [ ] 10.2 Run `make test-unit` — verify all unit tests pass
- [ ] 10.3 Run `make test-integration` — verify all integration tests pass
- [ ] 10.4 Build debug binary — verify no warnings
- [ ] 10.5 Run `make test-clean` and rebuild to verify clean build
