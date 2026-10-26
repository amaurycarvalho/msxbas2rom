# Running Pre-compiled Z80 Binary Code from MSX-BASIC

MSXBAS2ROM compiles MSX-BASIC sources into a ROM image, but it also lets your
program embed and execute **already assembled Z80 machine code**. This guide
shows the supported ways to inject a raw binary blob (produced by an assembler
such as `pasmo`, `z80asm`, `sjasmplus`, etc.) and call it from compiled BASIC.

All examples below were assembled and validated on a real emulated MSX1
(openMSX, Sharp HB-8000/Hotbit).

## 1. Prerequisites

- An assembler that can output a flat binary (`.bin`), for example:
  `pasmo --bin routine.asm routine.bin`.
- The MSXBAS2ROM compiler: `msxbas2rom -c program.bas` (plain ROM) or
  `-x` / `-6` for larger MegaROM programs.

The binary must be **position independent** (see section 3) and, when used with
`CMD RUNASM`, **uncompressed**.

## 2. Example routines

The guide reuses two tiny routines.

### 2.1 `usr.bin` — takes a parameter and returns a value

This routine reads the integer passed by `USR()`, adds one and stores the result
back so that `USR()` returns it.

```asm
; usr.asm
DAC_PARAM equ 0xF7F8        ; XBASIC USR() input/output parameter (DAC+2)
CHPUT     equ 0x00A2

            org 0           ; will be relocated at runtime -> keep it relocatable
            ld   hl,(DAC_PARAM)
            inc  hl
            ld   (DAC_PARAM),hl
            ret
```

```text
$ pasmo --bin usr.asm usr.bin
$ xxd -g1 usr.bin
2a f8 f7 23 22 f8 f7 c9        ; 8 bytes
```

| Offset | Byte | Value | Instruction   |
| ------ | ---- | ----- | ------------- |
| 0      | 42   | $2A   | `ld hl,(...)` |
| 1      | 248  | $F8   |               |
| 2      | 247  | $F7   |               |
| 3      | 35   | $23   | `inc hl`      |
| 4      | 34   | $22   | `ld (...),hl` |
| 5      | 248  | $F8   |               |
| 6      | 247  | $F7   |               |
| 7      | 201  | $C9   | `ret`         |

### 2.2 `hello.bin` — prints without parameters

Used to demonstrate `CMD RUNASM`, which passes no useful parameter. It writes
`HI!` through the BIOS `CHPUT` routine and returns.

```asm
; hello.asm
CHPUT equ 0x00A2

        org 0
        ld a,'H' : call CHPUT
        ld a,'I' : call CHPUT
        ld a,'!' : call CHPUT
        ret
```

```text
3e 48 cd a2 00 3e 49 cd a2 00 3e 21 cd a2 00 c9   ; 16 bytes
```

> `call CHPUT` is allowed because `CHPUT` is a **fixed external address** in the
> BIOS. The restriction is on `CALL`/`JP` to addresses *inside your own blob*.

## 3. Rules for the machine code

Because the routines are relocated (copied into a RAM buffer or executed from a
fixed RAM address), any address computed at assembly time is likely wrong:

- Use only **relative jumps** (`JR`, `DJNZ`, conditional `JR`) instead of
  `JP`.
- Do **not** `CALL`/`JP` to labels inside your own code (absolute addresses).
- Calls to fixed hardware addresses (BIOS entries such as `CHPUT`, kernel
  routines) are fine.
- `LD HL,label` to a label inside your blob is also unsafe; unroll the code or
  compute addresses at runtime.

The `usr.bin` routine is fully relocatable: it only touches the fixed address
`0xF7F8`.

## 4. How `DEF USR` / `USR()` work

- `DEF USR = <address>` installs the routine address in `USRTAB[0]`.
- `DEF USR1 = <address>` ... `DEF USR9 = <address>` install slots 1..9.
  (`DEF USR0 = <address>` is the same as `DEF USR = <address>`.)
- `USR(x)` / `USR0(x)` calls slot 0, `USR1(x)` slot 1, and so on.
- The **input** integer is written to `0xF7F8` (`DAC+2`) before the call; the
  routine must write the **output** integer to the same address before `RET`.
- `USR(x)` requires exactly one argument.

When **no** `DEF USR` is present in the program, `USR0()`/`USR1()`/`USR2()`/
`USR3()` are predefined kernel helpers (resource address, resource size,
multi-function, sprite collision). Defining any `DEF USR` switches all `USRn()`
calls to the user-defined dispatch table.

## 5. Resources and numbering

- `FILE "file"` embeds an external file as a **resource**; resources are numbered
  from `0` in the order the `FILE` directives appear (put them at the top of the
  source, before numbered lines).
- Inline `IDATA` and `DATA` blocks are also compiler-managed resources and are
  numbered **after** all `FILE` resources, `IDATA` before `DATA`.
  Example: with `FILE "a.bin"`, `FILE "b.bin"`, `IDATA ...`, `DATA ...` the
  indexes are `0=a.bin`, `1=b.bin`, `2=_IDATA_`, `3=_DATA_`.
- In `-d` (debug) mode the compiler prints the exact list:
  `Resource #0: a.bin`, `Resource #1: b.bin`, ...
- With plain ROM (`-c`) the whole image is 48 KB (16 KB usable for resources).
  Use a MegaROM mode for larger resource sets.

## 6. Alternative 1 — `DATA` + `READ` + `POKE` + `DEF USR` + `USR()`

The classic approach: store the code bytes as `DATA` values, `POKE` them into
free RAM, then point `DEF USR` at that RAM and call it.

```basic
10 FOR I=0 TO 7
20   READ B
30   POKE HEAP()+I, B
40 NEXT I
50 DEF USR = HEAP()
60 PRINT USR(41)                       ' prints 42
70 DATA 42,248,247,35,34,248,247,201   ' usr.bin bytes
```

- `HEAP()` returns the first free RAM address, which is a safe place to build the
  routine.
- `POKE` writes a single byte; the loop copies all 8 bytes.
- `DATA` values are decimal byte values (`42`=$2A ... `201`=$C9).
- `DEF USR = HEAP()` points the routine to the first byte written.

Pros: standard MSX-BASIC only, no external file needed.
Cons: the most RAM-hungry and slowest method; `DATA` uses text resources.

Compile and run:

```text
$ msxbas2rom -c alt1.bas
$ alt1.rom
```

## 7. Alternative 2 — `IDATA` + `IREAD` + `IPOKE` + `DEF USR` + `USR()`

Same idea, but the bytes are stored as 16-bit integers, which uses less ROM and
is faster.

```basic
10 FOR I=0 TO 3
20   IREAD V%
30   IPOKE HEAP()+I*2, V%
40 NEXT I
50 DEF USR = HEAP()
60 PRINT USR(41)                       ' prints 42
70 IDATA &hF82A, &h23F7, &hF822, &hC9F7
```

- `IDATA` stores 16-bit little-endian words. Pair the bytes of `usr.bin`:
  `2A F8` -> `&hF82A`, `F7 23` -> `&h23F7`, `22 F8` -> `&hF822`,
  `F7 C9` -> `&hC9F7`.
- `IREAD` reads the next integer from the `IDATA` resource.
- `IPOKE address, value` writes a 16-bit little-endian word, so the address
  advances by 2 each iteration.
- `IPEEK(address)` is the matching 16-bit read.

Pros: smaller and faster than `DATA`; still no external file.
Cons: requires grouping bytes into words.

## 8. Alternative 3 — `FILE` + `CMD RESTORE` + `IREAD` + `IPOKE` + `DEF USR` + `USR()`

Embed the raw `.bin` file and copy it to RAM word by word.

```basic
FILE "usr.bin"
10 CMD RESTORE 0        ' select binary resource #0
20 IRESTORE 0          ' rewind to byte 0 (required before IREAD)
30 FOR I=0 TO 3
40   IREAD V%
50   IPOKE HEAP()+I*2, V%
60 NEXT I
70 DEF USR = HEAP()
80 PRINT USR(41)        ' prints 42
```

- `FILE "usr.bin"` embeds the file as resource `0`.
- `CMD RESTORE <n>` selects the resource used by `IREAD` (and by `READ`).
- `IRESTORE <byte>` sets the read position **in bytes**; it must be used right
  after `CMD RESTORE` before reading a binary resource.
- `IREAD` reads one 16-bit word and advances 2 bytes.

Pros: the code lives in an external file; smaller BASIC listing; works well with
a build pipeline.
Cons: needs `IRESTORE` positioning; odd-length binaries must be padded.

## 9. Alternative 4 — `FILE` + `CMD RUNASM`

The simplest path: `CMD RUNASM` copies the resource into an internal buffer and
jumps to it.

```basic
FILE "hello.bin"
10 CMD RUNASM 0        ' executes hello.bin -> prints "HI!"
20 PRINT "BACK FROM ASM"
```

The example prints `HI!BACK FROM ASM`.

### `CMD RUNASM` limitations

- The resource must be **uncompressed**.
- All jumps must be **relative** (`JR`), and `CALL`/`JP` to addresses inside the
  routine are not allowed (the code is relocated).
- **Size limit:** the kernel copies the resource through its internal `BUF`
  buffer (`0xF55E`, 258 bytes) and caps the copy at **255 bytes**
  (`resource.get_data` in `80_resources.asm`). A routine larger than 255 bytes
  executes only its first 255 bytes and then runs into unrelated memory. The
  wiki's reference text mentions "256 KB", but the current implementation cannot
  honor that; treat **255 bytes** as the effective limit. For larger routines use
  `CMD RSCTORAM` (next section).

## 10. Other viable alternatives

### 10.1 `CMD RSCTORAM` + `DEF USR` (recommended for larger code)

`CMD RSCTORAM` copies a resource, or a bounded range of it, into a RAM address
of your choice, with no 255-byte cap. Then call it with `DEF USR` + `USR()`.

```basic
FILE "usr.bin"
10 CMD RSCTORAM 0, HEAP()   ' copy resource #0 to free RAM
20 DEF USR = HEAP()
30 PRINT USR(41)            ' prints 42
```

- Syntax:
  `CMD RSCTORAM <resource>, <RAM dest> [, <pletter: 0=no, 1=yes> [, <offset> [, <size>]]]`.
- `offset` defaults to `0` (first resource byte). `size` defaults to the number
  of bytes remaining after `offset`; a `size` of `0` also means "full remaining".
  When `size` is non-zero, the kernel copies
  `min(size, resource_size - offset)` bytes and never reads past the resource end.
- `pletter = 1` decompresses the whole compressed resource and ignores `offset`
  and `size`.
- The kernel routine returns the destination address in `HL` on success, or `0`
  when the resource does not exist or nothing was copied. `CMD RSCTORAM` used as
  a statement ignores this value.
- This is the best option for routines bigger than 255 bytes, or when you want
  to avoid the word-by-word `IPOKE` loop.
- Because the code executes at `HEAP()` (unknown at assembly time), the same
  relocation rules apply: relative jumps only, or assemble the routine for the
  runtime address.

### 10.2 `RESOURCE()` / `RESOURCESIZE()`

`RESOURCE(n)` returns a resource start address and `RESOURCESIZE(n)` its size.
They are useful to pass a resource to memory commands such as `COPY` or
`CMD RAMTORAM`. Executing a resource in place with
`DEF USR = RESOURCE(n)` is **not recommended**: in the current build
`RESOURCE()` did not return the expected data address on plain ROM (verified),
so use `CMD RSCTORAM` to move the code to RAM first.

### 10.3 `USR0`/`USR1` (deprecated)

Before `RESOURCE()`/`RESOURCESIZE()` existed, `USR0(n)` returned the resource
address and `USR1(n)` the resource size. They are deprecated and share the same
caveat as `RESOURCE()`.

## 11. Command and function syntax summary

| Item | Syntax | Notes |
| ---- | ------ | ----- |
| `FILE` | `FILE "path"` | Embeds a resource; place before numbered lines. |
| `DATA` | `DATA v1, v2, ...` | Values read with `READ` (classic resource). |
| `IDATA` | `IDATA v1, v2, ...` | 16-bit integer resource, read with `IREAD`. |
| `READ` | `READ var[, var...]` | Reads `DATA`/text resources. |
| `IREAD` | `IREAD var[, var...]` | Reads 16-bit integers from `IDATA`/binary `FILE`. |
| `RESTORE` | `RESTORE [line]` | Repositions text/CSV reads by line. |
| `IRESTORE` | `IRESTORE [byte]` | Repositions integer/binary reads by byte offset. |
| `CMD RESTORE` | `CMD RESTORE <resource>` | Selects the resource for `READ`/`IREAD`. |
| `POKE` | `POKE address, byte` | Writes an 8-bit value. |
| `IPOKE` | `IPOKE address, integer` | Writes a 16-bit little-endian value. |
| `IPEEK` | `IPEEK(address)` | Reads a 16-bit little-endian value. |
| `DEF USR` | `DEF USR = address` | Installs slot 0 (`USR`/`USR0`). |
| `DEF USRn` | `DEF USRn = address` | Installs slot n (1..9). |
| `USR` | `USR(x)` / `USRn(x)` | Calls installed routine; param/return at `0xF7F8`. |
| `HEAP()` | `HEAP()` | First free RAM address. |
| `VARPTR()` | `VARPTR(var)` | Address of a variable. |
| `CMD RUNASM` | `CMD RUNASM <resource>` | Copies (max 255 bytes) and executes. |
| `CMD RSCTORAM` | `CMD RSCTORAM <res>, <dest>[, <pletter>[, <offset>[, <size>]]]` | Copies a resource range to RAM; `size=0` means full remaining. |
| `RESOURCE()` | `RESOURCE(n)` | Resource address (with caveats, see 10.2). |
| `RESOURCESIZE()` | `RESOURCESIZE(n)` | Resource size. |

## 12. Choosing an alternative

| Method | External file | Size limit | Speed | Typical use |
| ------ | ------------- | ---------- | ----- | ----------- |
| 1. `DATA`/`POKE` | no | RAM/ROM | slow | small patches, no assets |
| 2. `IDATA`/`IPOKE` | no | RAM/ROM | medium | small patches, inline |
| 3. `FILE`/`CMD RESTORE`/`IPOKE` | yes | RAM | medium | build pipeline, many routines |
| 4. `CMD RUNASM` | yes | 255 bytes | fast to write | tiny helpers |
| 5. `CMD RSCTORAM`/`DEF USR` | yes | RAM | fast | larger routines |

## 13. Gotchas

- Always position binary `FILE` reads with `IRESTORE` after `CMD RESTORE`;
  otherwise execution reads from the wrong address (verified: it crashes).
- `DEF USR` must appear **before** the first `USR()` call.
- A routine that never reaches `RET` will crash the program.
- With plain ROM (`-c`), keep the total resource size within ~16 KB; use a
  MegaROM mode for larger assets.
- Do not call `CMD RUNASM` with a compressed resource.
- If you use several `DEF USRn` slots, remember `USR`/`USR0` share slot 0.
