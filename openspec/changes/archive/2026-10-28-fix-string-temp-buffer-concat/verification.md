# Verification

## Temporary-string allocator (tasks 1.1 and 1.2)

`GET_NEXT_TEMP_STRING_ADDRESS` returns `TMPSTRADDR + index * 256` by loading the
0..4 rotation index into `B` and zero into `C` before `add hl, bc`. The 0..4
rotation and the `AF`/`BC` preservation are unchanged. The kernel was rebuilt
with `make -C src/infrastructure/kernel/asm all`.

A two-`CHR$` probe (`PRINT CHR$(65);CHR$(66);CHR$(67)`) captured `ABC`, proving
that each result occupies a distinct slot.

Caller audit:

| Caller | Destination use | Result |
| --- | --- | --- |
| `CompilerVariableEmitter::addTempStr()` | string destination for direct writes or `DISP_XBASIC_COPY_STRING` | 256-byte slot, safe |
| `XBASIC_GET_BUFFER` | stores the next address in `TMPSTRBUF` for PRINT#/INPUT#/PLAY/TAB | 256-byte buffer, safe |
| `XBASIC_TAB` | fills the buffer with spaces through the STRING$ handler | up to 255 bytes, safe |
| `XBASIC_READ` / `XBASIC_IREAD` | copies the next DATA string (length byte + payload) | at most 256 bytes, safe |
| `MR_GET_DATA` | copies a string from a MegaROM segment | at most 256 bytes, safe |

No caller assumes a one-byte stride or places more than 256 bytes in a slot.

## String strategies (tasks 2.1 to 2.3)

`STRING$`, `SPACE$`, and `INKEY$` now follow the `LEFT$` pattern: the kernel
writes `BUF`, then the compiler calls `addTempStr(false)`,
`DISP_XBASIC_COPY_STRING`, and leaves `HL` at the copied temporary string.

- Unit coverage in `tests/unit/src/test_compiler_functions.cpp` checks the copy
  count for `STRING$` concatenation, `SPACE$` concatenation with `CHR$`, and
  `INKEY$`, and checks that the `INKEY$` kernel call precedes the copy.
- `SPACE$` runtime probe: `PRINT "["+SPACE$(3)+CHR$(66)+"]"` produced `[   B]`.
- `STRING/test1` produced `AAAAABBBBBB` for the concatenated result rows.

## Startup screen mode (tasks 3.1 to 3.3)

`start.asm` now configures the 40-column BIOS variables, sets the kernel text
mode, and selects `SCREEN 0` through BIOS `CHGMOD`. The builder no longer
requires a `C7369` call in the start image, though it still patches one if an
older image contains it.

A `PRINT "HELLO"` / `PRINT "WORLD"` probe was captured on all three configured
machine classes:

| Machine | VDP screen | VDP name-table base | `LINLEN` | Rows |
| --- | --- | --- | --- | --- |
| `Sharp_HB-8000_1.2` | 0 | `0x0000` | 39 | aligned 40-column rows |
| `Panasonic_FS-A1WX` | 0 | `0x0000` | 39 | aligned 40-column rows |
| `Panasonic_FS-A1GT` | 0 | `0x0000` | 39 | aligned 40-column rows |

The PRINT CR+LF routine was not changed. Consecutive PRINTs land on consecutive
rows at a consistent left margin with no rightward drift.

## STRING$ arity (task 4.1)

`CompilerExpressionEvaluator::evalFunction` rejects more than four arguments
before evaluating any argument. `STRING$(3,65)` compiles; 
`STRING$(3,65,CHR$(66),2,3)` fails with `Too many function arguments` and emits
no `DISP_XBASIC_STRING` or temporary-string allocation.

## Regression probes (tasks 5.1 to 5.5)

`PRINT/test5` and `STRING/test1` are registered in
`tests/integration/regression/manifest.json` with `name_table: "0x0000"` and
`width: 40`. Both select the MSX1 machine (`Sharp_HB-8000_1.2`) through
`regression-list`. Their baselines were generated from the fixed ROM and both
pass:

```text
make -C tests/integration regression SCENARIO=PRINT PROGRAM=test5
PASS  PRINT/test5

make -C tests/integration regression SCENARIO=STRING PROGRAM=test1
PASS  STRING/test1
```

Pre-fix comparison using the HEAD build (`/tmp/opencode/old-msx`) captured
`SCREEN 1` with name table `0x1800`: `PRINT/test5` wrapped at 32 columns, and
`STRING/test1` rendered the concatenated result as repeated `B` characters. The
fixed ROM produces screen 0, name table `0x0000`, 40-column rows, and
`AAAAABBBBBB`.

## Additional PRINT probes (tasks 9.1 to 9.3)

Added manifest coverage for the remaining PRINT parameter variants:

| Program | Mode | Capture and probes |
| --- | --- | --- |
| `PRINT/test6` | explicit `SCREEN 0` | 40-column text at name table `0x0000`, including the separator rows and exact tab/space bytes |
| `PRINT/test8` | explicit `SCREEN 1` | 32-column text at name table `0x1800`, including the separator rows and exact tab/space bytes |
| `PRINT/test9` | `SCREEN 2` | pattern-generator probes at `0x00`-`0xa8` for the rendered PRINT glyphs |
| `PRINT/test10` | `SCREEN 3` | multicolour pattern-generator probes at `0x00`-`0x50` for the rendered PRINT glyphs |
| `PRINT/test11` | `SCREEN 4` | pattern-generator probes at `0x00`-`0xa8` for the rendered PRINT glyphs |

Each program was captured three times to produce a new baseline. All five pass
their regression check:

```text
PRINT/test6   PASS
PRINT/test8   PASS
PRINT/test9   PASS
PRINT/test10  PASS
PRINT/test11  PASS
```

## Baseline audit (tasks 6.1 to 6.4)

Inventory before the fix:

- 123 integration programs use `PRINT`, `?`, or `STRING$`; 121 had baselines.
- 88 of those baselines recorded `screen: 1`; 89 programs used the default
  layout, and 80 of those recorded `screen: 1`.
- The repository had 94 `screen: 1` baselines in total.

Regenerated 100 baselines with three runs each (98 existing, 2 new): every
program with no explicit `SCREEN`, every `PRINT/*` and `STRING/*` baseline, the
text-bearing entries listed in task 6.3, and the explicit-screen `STRING$`
programs from task 6.4. `GRAPH/mexhat`, `GRAPH/test89`, `MATH/cos`,
`MTF/mtf2`, and `MTF/mtf4` were also regenerated because the initial mode change
affected their captured state. After regeneration, no default-layout program
records `screen: 1`; the 11 remaining `screen: 1` baselines belong to programs
that explicitly select `SCREEN 1`.

Manifest text checks for `INPUT/test8`, `ON/test2`, `ON/test4`,
`MISC/test1`, `MISC/test64`, `MISC/test65`, `MISC/rsc_offset`,
`MISC/rscfunc`, and `MISC/turbo_gate` now use `name_table: "0x0000"` and
`width: 40`. `GRAPH/mouse2` keeps its explicit `SCREEN 1` baseline.

## Integration checks (tasks 7.1 and 7.2)

- Full local regression: 174 programs, 174 passed, 0 failed, 0 errors.
- `make test-unit`: 686 test cases, 12305 assertions, all passed.
- `make test-integration`: passed, including CLI characterization, self-contained
  compilation, forced overflow, and game-graphics dispatch performance.
- `make test-architecture`: passed.
- `make lint`: passed.

## Documentation (tasks 8.1 and 8.2)

Added `doc/adr/ADR-002 Initial text screen normalization.md` following the
ADR-001 structure and updated `doc/misc/architecture.md` section 5.3 to describe
the explicit `SCREEN 0` / `WIDTH 40` startup and link to ADR-002.
