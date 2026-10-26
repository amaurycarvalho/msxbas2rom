---
name: msx-memory-debug
description: Launch an MSX ROM in openMSX, capture the Z80-visible RAM range 0xC000-0xFFFF, optional VRAM and a screenshot beside the ROM, optionally at a pre-boot breakpoint or with scripted keyboard/joystick input, then exit openMSX so a coding agent can inspect the artifacts.
---

# MSX Memory Debug

Use this skill when an MSX ROM needs post-startup memory inspection without requiring an interactive debugger session.

## Workflow

Run:

```bash
./scripts/debug-msx-memory.sh path/to/program.rom
```

The launcher:

1. Validates the ROM path.
2. Starts openMSX with the ROM inserted.
3. Passes the ROM path and capture delay to the Tcl support script.
4. Installs any configured breakpoints before the ROM starts executing.
5. Lets openMSX run normally for the configured delay, or captures immediately
   when a breakpoint fires.
6. Captures the CPU-visible RAM range `0xC000..0xFFFF` (16 KiB) from the `Main RAM` debuggable.
7. Optionally captures raw physical VRAM and the VDP register block.
8. Captures a PNG screenshot (unless disabled).
9. Exits openMSX automatically.
10. Leaves the artifacts in the same directory as the ROM.

Default artifacts:

- `<rom-basename>.ram-c000-ffff.bin` — exactly 16384 bytes.
- `<rom-basename>.screen.png` — screenshot captured at the same point in emulated time.

Opt-in artifacts:

- `<rom-basename>.vram.bin` — physical VRAM, sized by the emulated VDP
  (16 KiB on the default Sharp HB-8000, 128 KiB on a V9938 MSX2 machine),
  or exactly the configured `START:LENGTH` range.
- `<rom-basename>.vdp-regs.bin` — the 64-byte VDP register debug block.
- `<rom-basename>.capture.txt` — trigger, emulated capture time, machine, and
  artifact sizes. Written only when VRAM, breakpoints, or input are configured.

## Configuration

Environment variables:

- `OPENMSX_EXECUTABLE` — executable used to launch openMSX. Default: `openmsx`.
- `OPENMSX_ARGS` — optional extra launcher arguments, parsed as shell words. Default: empty.
- `OPENMSX_FLATPAK_APP` — if set, the launcher becomes `flatpak run <value>` and `OPENMSX_EXECUTABLE`/`OPENMSX_ARGS` are ignored. Example: `org.openmsx.openMSX`.
- `MSX_DEBUG_DELAY` — delay in emulated seconds before fallback capture. The fallback runs at the next VDP frame boundary after the delay so the captured state is deterministic. Default: `5`.
- `MSX_DEBUG_MACHINE` — openMSX machine name passed with `-machine`. Default: `Sharp_HB-8000_1.2` (the Brazilian Hotbit, MSX1), so the emulator never falls back to C-BIOS. Use `Panasonic_FS-A1WX` when debugging MSX2-oriented BASIC commands.
- `MSX_DEBUG_SETTINGS` — optional path to an openMSX `settings.xml` to load with `-setting`. When unset, an isolated settings file is generated automatically.
- `MSX_DEBUG_SCREENSHOT_MODE` — `scaled` (default) or `raw`.
- `MSX_DEBUG_SCREENSHOT` — `1` (default) or `0` to skip the screenshot artifact.
- `MSX_DEBUG_VRAM` — `1`/`full` to capture physical VRAM, or `START:LENGTH` for a range.
- `MSX_DEBUG_VRAM_START`, `MSX_DEBUG_VRAM_LENGTH` — optional range when `MSX_DEBUG_VRAM` is enabled.
- `MSX_DEBUG_BREAKPOINTS` — comma-separated `LABEL=ADDRESS`, `ADDRESS`,
  `LABEL=ADDRESS@CONDITION`, or `ADDRESS@CONDITION` items installed before the ROM
  starts. Addresses come from a `.noi` file. The optional condition is a Tcl
  expression evaluated before capture, for example
  `[pc_in_slot X X 2]`; it must not contain commas or braces.
- `MSX_DEBUG_ACCELERATE` — `1`/`on` disables throttling when no input script is
  configured. With an input script, real-time throttling stays enabled. An
  explicit `MSX_DEBUG_THROTTLE` value overrides this setting.
- `MSX_DEBUG_INPUT` — path to a timed input script (keyboard and joystick).
- `MSX_DEBUG_THROTTLE` — `on` or `off`; `off` runs emulation as fast as possible.
  Prefer `MSX_DEBUG_ACCELERATE=1` over setting this directly.

Examples:

```bash
MSX_DEBUG_DELAY=10 ./scripts/debug-msx-memory.sh build/program.rom
```

VRAM capture (16 KiB on the default MSX1 machine, 128 KiB on an MSX2 machine):

```bash
MSX_DEBUG_VRAM=1 ./scripts/debug-msx-memory.sh build/program.rom
```

Capture at a program-end breakpoint (addresses from the compiled `.noi`):

```bash
MSX_DEBUG_ACCELERATE=1 \
MSX_DEBUG_DELAY=20 \
MSX_DEBUG_VRAM=1 \
MSX_DEBUG_BREAKPOINTS='END_PGM=0x80a0,END_STMT+2=0x8065' \
  ./scripts/debug-msx-memory.sh build/program.rom
```

Capture only when the MegaROM block containing the line is selected:

```bash
MSX_DEBUG_ACCELERATE=1 \
MSX_DEBUG_BREAKPOINTS='LIN_80=0x8152@[pc_in_slot X X 2]' \
  ./scripts/debug-msx-memory.sh build/program.rom
```

### Timed input scripts

`MSX_DEBUG_INPUT` points to a line-oriented file. Each line starts with an
emulated time in seconds:

```text
# type a line of text, then press RETURN
type 6.5 JOHN
key 6.8 RETURN 0.05
# hold the joystick left for four emulated seconds
joy 8.0 left down
joy 12.0 left up
# press joystick button A
joybutton 8.5 a down
joybutton 8.7 a up
```

Supported commands:

- `type TIME TEXT` — types text through openMSX's keyboard input.
- `key TIME KEY [DURATION]` — presses a key matrix key (`SPACE`, `RETURN`,
  `BACKSPACE`, `TAB`, `ESC`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `SHIFT`, `CTRL`,
  `GRAPH`, `CAPS`). A single printable character is typed instead.
- `joy TIME DIRECTION down|up` — drives openMSX's `keyjoystick1` pluggable on
  joystick port A through host XTEST events (`DIRECTION` is `left`, `right`,
  `up`, or `down`). Requires an X display and `python-xlib`.
- `joybutton TIME a|b down|up` — joystick trigger A or B.

Direct executable:

```bash
OPENMSX_EXECUTABLE=openmsx ./scripts/debug-msx-memory.sh build/program.rom
```

Flatpak:

```bash
OPENMSX_FLATPAK_APP=org.openmsx.openMSX \
  ./scripts/debug-msx-memory.sh build/program.rom
```

With extra openMSX arguments:

```bash
OPENMSX_ARGS='-ext fmpac' \
  ./scripts/debug-msx-memory.sh build/program.rom
```

## Output interpretation

The `.bin` file is a raw 16 KiB memory dump. Byte `0` corresponds to CPU address `0xC000`; byte `0x3FFF` corresponds to `0xFFFF`.

The dump is intended for deterministic post-run inspection by an agent. It should not be interpreted as a dump of a particular physical RAM chip or slot unless the selected MSX machine's memory mapping makes that interpretation valid. The Tcl script reads openMSX's `Main RAM` debuggable at CPU addresses `0xC000..0xFFFF`.

The screenshot is a visual snapshot of the emulated MSX screen at the same capture event.

## Debugging strategies

The launcher captures one RAM dump and one screenshot at a fixed emulated time. Use these
strategies to turn that into an effective debugging session.

### Freeze the state before capture

A fixed delay does not guarantee the program is at the state of interest. Have the debug
program compute the values under inspection, `POKE` them into fixed RAM addresses, and then
halt (for example `GOTO` itself) so every capture sees the same final state. Locate a probe
byte in the dump at offset `address - 0xC000`.

```basic
100 POKE 57344, X%
110 GOTO 110
```

### Map addresses with symbols

Compile the ROM with `msxbas2rom -s --noi` to get a `.noi` file mapping BASIC variables
(for example `VAR_X%`) to RAM addresses. Kernel work-area symbols (`SPRTBL`,
`HITBOX_TABLE`, `SPRSIZ`, ...) are listed in
`src/infrastructure/kernel/asm/src/header.symbols.asm`.

```bash
./bin/Release/msxbas2rom -q -s --noi program.bas
```

### Inspect the artifacts

- RAM: 16384 bytes, byte `0` = `0xC000`. Parse it with a small script (e.g. Python) and
  search for printable strings to identify variable contents.
- Screenshot: open the PNG to confirm what is on screen at the capture point.

### Go to CPU level with openMSX breakpoints

When registers or exact instruction flow matter, drive openMSX directly with a Tcl script
instead of the bulk capture: set a breakpoint with `debug set_bp`, read registers with
`debug read_block "CPU regs" 0 26`, read memory with
`debug read_block "Main RAM" <addr> <len>`, and resume with `debug cont`. Callbacks pause
the CPU, so always call `debug cont`. Write callback output to a file because Tcl `puts` is
not reliably visible on stdout. Prefer a breakpoint on a success/terminal path to capture
only the interesting occurrence.

The `CPU regs` block is 26 bytes: `F, A, B, C, D, E, H, L, F', A', B', C', D', E', H', L',
IXh, IXl, IYh, IYl, SPh, SPl, PCh, PCl, I, R`.

### Rebuild the kernel when debugging kernel behavior

The Z80 kernel is embedded in the compiler as `src/infrastructure/kernel/header.h`. After
editing `src/infrastructure/kernel/asm/src/header/*.asm`, run:

```bash
make -C src/infrastructure/kernel/asm
make release
```

Referencing absolute RAM addresses inside kernel code for debug stores can make pasmo emit
a much larger `header.bin`; prefer existing work-area symbols and remove temporary
instrumentation afterwards.

### MSX2 commands use an MSX2 machine

MSX2-oriented BASIC commands (for example `SCREEN 7`) must be debugged on an MSX2 machine:

```bash
MSX_DEBUG_MACHINE=Panasonic_FS-A1WX ./scripts/debug-msx-memory.sh program.rom
```

The default `Sharp_HB-8000_1.2` is an MSX1 machine and remains correct for MSX1 commands.

## Important behavior

The launcher does not load the user's interactive `settings.xml`. openMSX aborts at startup when that file contains a key binding the installed version does not recognise (for example an unknown `PrintScreen` keycode), which would prevent the capture from running. Instead, the launcher writes a minimal settings file next to the ROM, passes it with `-setting`, and deletes it afterwards.

The machine is selected explicitly with `-machine Sharp_HB-8000_1.2`, so the emulator boots the Sharp HB-8000 (Hotbit) BIOS rather than falling back to C-BIOS. Set `MSX_DEBUG_MACHINE` to use a different machine, or `MSX_DEBUG_SETTINGS` to supply a custom settings file. For MSX2-oriented BASIC commands, set `MSX_DEBUG_MACHINE=Panasonic_FS-A1WX`.

The delay uses openMSX's Tcl `after time`, which is based on emulated MSX seconds rather than host wall-clock time. This makes the capture point tied to emulation time.

The script exits openMSX after the artifacts have been written. The launcher returns the openMSX exit status to the calling agent.

Do not use `debug read` one byte at a time. The Tcl script uses `debug read_block` to retrieve the complete 16 KiB range in one operation.
