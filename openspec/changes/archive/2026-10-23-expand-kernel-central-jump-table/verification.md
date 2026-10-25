# Verification Notes

## Self-contained BASIC compilation (task 8.6)

Command used:

```bash
tmpdir=$(mktemp -d)
printf '10 PRINT "SELF"\n20 END\n' > "$tmpdir/probe.bas"
strace -f -e trace=execve,openat \
  -o "$tmpdir/trace.txt" \
  ./bin/Release/msxbas2rom -q "$tmpdir/probe.bas"
```

Result: `strace` recorded a single `execve` for `msxbas2rom` itself. There were
no `execve` calls for `pasmo`, `rasm`, a linker, or any other process, and no
`openat` of kernel module, manifest, relocation, or `.asm` files during BASIC
compilation. The compiler uses only the embedded kernel headers.

Repeatable check: `python3 tests/integration/check_self_contained.py`.

## MR segment save/restore audit (task 4.5)

`MR_CALL`, `MR_GET_DATA`, and `MR_GET_BYTE` save the current segment with
`ld bc, (SGMADR-1)`, push `bc`, and later `pop af`. In little-endian order the
low byte at `SGMADR-1` is `MAPPER` and the high byte is `SGMADR`, so `pop af`
places the exact previous segment in `A` before `jp`/`call MR_CHANGE_SGM`.
No use of `SGMADR-1` was found that restores a different segment.

## Page-safety audit and decisions (tasks 8.1, 8.2, 9.3-9.9)

A module that executes at `0x8000` cannot call a routine that maps resource
data into `0x8000` and then continue executing in the module. The module's code
is replaced by resource data. The same restriction applies to code invoked with
compressed data mapped at `0x8000`.

| Module | Reason | Decision |
| --- | --- | --- |
| `32_usr` | `usr0`/`usr1` call `resource.open` + `resource.address` and continue | fixed |
| `31-cmd-mtf` | `resource.open_and_get_address`, then header/data access in the module | fixed |
| `70_pletter` | BLOAD/`resource.ram.unpack` pass compressed data mapped at `0x8000` | fixed |
| `31-cmd-game-graphics` | `cmd_restore` and `cmd_rsctoram` continue after resource mapping | fixed |
| `31-cmd-cold-graphics` | `cmd_screen_load` (SCREEN LOAD) continues after resource mapping | fixed |
| `90-graphics` | Atomic group with game graphics; start code also calls `C7369` | fixed |
| `30_basic_helpers` | BLOAD, DATA/RESTORE/READ, resource management | already fixed |
| `80_resources`, `60_bios_helpers`, `33_player`, `61_megarom`, `90-traps` | Core/resource/player/interrupt support | already fixed |
| `34_file_handling` | Disk I/O, no `0x8000` resource mapping | flex-low/overflow |
| `31-cmd-tile-sprite` | No resource access | flex-high/overflow |
| `90-math`, `90-math-early`, `90-strings`, `21_logic_pack`, `90-print` | Pure computation/strings/print | flex/overflow |
| `31-cmd-input` | Joystick/pad | flex-low |

DATA/RESTORE/READ handlers live in fixed `30_basic_helpers`;
`cmd_restore` lives in fixed `31-cmd-game-graphics` and jumps into the fixed
handler. Both are covered by the resident MegaROM test.

## Hardcoded kernel address audit (task 9.8)

- `XBASIC_TAB` used `jp 0x7EF5`; the current `STRING$` handler is `C7EF2`.
  The address was stale and is now `jp C7EF2`, recorded as a module import.
- `start.asm` calls `C7369`. With `90-graphics` fixed, the start-code patch is
  skipped because the original absolute address remains valid. The builder
  still patches it if the module later becomes relocatable.
- No other numeric kernel-routine addresses were found outside the build-time
  virtual dispatch table.

## Dispatcher ABI correction (task 9.9)

The previous dispatcher saved registers around the target but did not restore
argument registers before entering it, and it restored caller registers after
the target, discarding result registers. The corrected dispatcher:

1. saves `AF`, `BC`, `DE`, `HL`, and `IX` in a 16-byte kernel scratch frame;
2. reads the inline segment/address and restores the argument registers;
3. switches segment, preserving caller `A`;
4. enters the target with normal call semantics;
5. preserves the target `A` across the segment restore and returns target
   results to the caller.

`BASMEM` moved from `0xC038` to `0xC048` to reserve the scratch frame.
`tests/dispatcher/prototype.asm` now verifies that the target receives
`A/BC/DE/HL/IX` and that result registers reach the caller.

## Plain ROM overflow

`31-cmd-tile-sprite` is generated as a relocatable module from a dual-base
assembly. In Plain ROM it is forced to overflow so the path is exercised by
`tests/dispatcher/check_plain_overflow.py`. The module is copied after user code
in `0x8000-0xBFFF`, the central table uses a direct `jp` stub, and the program
writes its marker in openMSX.

## MegaROM status

The packer and composer place the dispatcher module and generate paged stubs for
MegaROM overflow entries. `tests/dispatcher/check_megarom_overflow.py` exercises
a forced tile/sprite overflow module in openMSX for ASCII8, Konami4, KonamiSCC,
ASCII16, and ASCII16-X, and `check_megarom_resources.py` covers resource/trap
metadata.

`tests/dispatcher/check_page_safe_modules.py` verifies:

- overflow runtime for `34_file_handling`, `31-cmd-input`, `90-math`,
  `90-strings`, `21_logic_pack`, `90-print`, and `31-cmd-tile-sprite` in
  Plain ROM and all five MegaROM mappers;
- resident runtime and forced-overflow rejection for `32_usr`,
  `31-cmd-mtf`, `70_pletter`, `31-cmd-cold-graphics`,
  `31-cmd-game-graphics`, and `90-graphics`;
- resident DATA/RESTORE/READ in MegaROM.

## Game graphics performance (task 8.3)

`tests/integration/check_game_graphics_perf.py` compares the resident and
forced-overflow layouts of `GRAPH/test94.bas` in ASCII8 MegaROM and writes
`performance-notes.md`. The measured overflow group is `31-cmd-tile-sprite`;
game graphics is fixed for page safety. The report records the resident
baseline (0 paged calls) and forced layout (3 paged calls), the stub size
difference, and the dispatcher instruction path.

## Final verification status

- `make lint`: passed.
- `make test`: 673 unit cases / 11345 assertions passed; 37 architecture tests passed.
- `make test-kernel`: kernel regression, relocation prototype, dispatcher ABI,
  Plain overflow, all five MegaROM mappers, MegaROM resource/MR handling,
  page-safe overflow matrix, fixed resource modules, and DATA/RESTORE/READ
  passed in openMSX.
- `make test-integration`: all ASCII8/ASCII16/ASCII16-X/Konami4 builds,
  self-contained trace, forced-overflow migration checks, and the graphics
  dispatch performance check passed.
- `openspec validate` was not re-run because the CLI is not installed in this
  environment.

Tasks 8.1, 8.2, and section 9 are complete. The remaining workflow item is
archive/release.
