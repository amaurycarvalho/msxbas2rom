# MSX Memory Debug Skill

A small coding-agent skill for post-run inspection of MSX ROMs with openMSX.

## Files

- `SKILL.md` — agent-facing instructions.
- `scripts/debug-msx-memory.sh` — launcher and environment configuration.
- `scripts/msx-memory-capture.tcl` — openMSX capture/exit script.

## Quick start

```bash
chmod +x scripts/debug-msx-memory.sh
./scripts/debug-msx-memory.sh ./program.rom
```

The ROM directory receives:

```text
program.ram-c000-ffff.bin
program.screen.png
```

The RAM dump is exactly 16 KiB and starts at CPU address `0xC000`.

## Flatpak

```bash
OPENMSX_FLATPAK_APP=org.openmsx.openMSX \
  ./scripts/debug-msx-memory.sh ./program.rom
```

This expands to the equivalent of:

```text
flatpak run org.openmsx.openMSX ...
```

## Native executable

```bash
OPENMSX_EXECUTABLE=openmsx \
  ./scripts/debug-msx-memory.sh ./program.rom
```

## Delay

The delay is measured in openMSX emulated seconds:

```bash
MSX_DEBUG_DELAY=10 ./scripts/debug-msx-memory.sh ./program.rom
```

## Why `Main RAM`

openMSX exposes memory and RAM through its debugger as debuggables. The script uses `debug read_block "Main RAM" 0xC000 0x4000`, rather than repeatedly reading individual bytes. This produces a compact, raw binary dump suitable for automated analysis.

For an MSX program where the CPU-visible page 3 is mapped to another device instead of main RAM, this dump should be understood as the `Main RAM` device contents at those addresses, not necessarily as the currently visible CPU bus contents. If exact CPU-bus mapping is required for a particular machine/mapper, the skill should be extended to capture the relevant mapped memory debuggable as well.

## Debugging strategies

The launcher produces one RAM dump and one screenshot at a fixed emulated time. The
following practices make that capture useful.

### Freeze the state before capture

Have the debug program compute the values of interest, `POKE` them into fixed RAM
addresses, and then halt in an infinite loop so every capture sees the same final state:

```basic
100 POKE 57344, X%
110 GOTO 110
```

A probe byte is at offset `address - 0xC000` in the RAM dump.

### Map addresses with symbols

Compile with `-s --noi` to map BASIC variables (for example `VAR_X%`) to RAM addresses:

```bash
./bin/Release/msxbas2rom -q -s --noi program.bas
```

Kernel work-area symbols (`SPRTBL`, `HITBOX_TABLE`, `SPRSIZ`, ...) are listed in
`src/infrastructure/kernel/asm/src/header.symbols.asm`.

### Inspect the artifacts

Parse the 16 KiB RAM dump with a small script (byte `0` is `0xC000`) and search for
printable strings to identify variable contents. Open the PNG screenshot to confirm the
on-screen state at the capture point.

### CPU-level debugging with breakpoints

For register or instruction-flow questions, drive openMSX with a custom Tcl script:
`debug set_bp` to break, `debug read_block "CPU regs" 0 26` for registers,
`debug read_block "Main RAM" <addr> <len>` for memory, and `debug cont` to resume. Write
callback output to a file because Tcl `puts` is not reliably visible on stdout. Breakpoints
pause the CPU, so always call `debug cont`.

The `CPU regs` block is 26 bytes: `F, A, B, C, D, E, H, L, F', A', B', C', D', E', H', L',
IXh, IXl, IYh, IYl, SPh, SPl, PCh, PCl, I, R`.

### Kernel changes need a kernel rebuild

The Z80 kernel is embedded as `src/infrastructure/kernel/header.h`. After editing
`src/infrastructure/kernel/asm/src/header/*.asm`:

```bash
make -C src/infrastructure/kernel/asm
make release
```

Referencing absolute RAM addresses inside kernel code for debug stores can make pasmo emit
a much larger `header.bin`; prefer existing work-area symbols and remove temporary
instrumentation afterwards.

## Settings isolation

By default the launcher does **not** load the user's `settings.xml`. openMSX aborts
at startup when that file contains a key binding the installed version does not
recognise, for example:

```text
Uncaught exception: Invalid keycode: PrintScreen
```

That crash happens before the capture script runs, so the skill would produce no
artifacts. To avoid it, the launcher generates a minimal settings file next to the
ROM, passes it with `-setting`, and removes it on exit.

Because the isolated settings file carries no `default_machine`, the launcher also
selects the target machine explicitly:

```text
-machine Sharp_HB-8000_1.2
```

This boots the Brazilian Sharp HB-8000 (Hotbit) BIOS, an MSX1 machine. Without it openMSX
would fall back to C-BIOS. For MSX2-oriented BASIC commands (for example `SCREEN 7`), use an
MSX2 machine such as the Panasonic FS-A1WX. Overrides:

```bash
# Debug MSX2 commands on a Panasonic FS-A1WX
MSX_DEBUG_MACHINE=Panasonic_FS-A1WX ./scripts/debug-msx-memory.sh ./program.rom

# Use a custom settings file instead of the generated one
MSX_DEBUG_SETTINGS=/path/to/settings.xml ./scripts/debug-msx-memory.sh ./program.rom
```

## openMSX behavior

The capture is implemented entirely through the openMSX Tcl interface. The official command reference documents `debug read_block` for bulk debugger reads, `screenshot` for PNG capture, and Tcl `after time` for scheduling commands in emulated seconds.

After writing both artifacts, the Tcl script calls `exit`, so the calling coding agent regains control.
