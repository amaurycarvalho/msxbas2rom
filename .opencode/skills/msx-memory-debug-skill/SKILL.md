---
name: msx-memory-debug
description: Launch an MSX ROM in openMSX, wait a configurable amount of emulated time, capture the Z80-visible RAM range 0xC000-0xFFFF and a screenshot beside the ROM, then exit openMSX so a coding agent can inspect the artifacts.
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
4. Lets openMSX run normally for the configured delay.
5. Captures the CPU-visible RAM range `0xC000..0xFFFF` (16 KiB) from the `Main RAM` debuggable.
6. Captures a PNG screenshot.
7. Exits openMSX automatically.
8. Leaves both artifacts in the same directory as the ROM.

Default artifacts:

- `<rom-basename>.ram-c000-ffff.bin` — exactly 16384 bytes.
- `<rom-basename>.screen.png` — screenshot captured at the same point in emulated time.

## Configuration

Environment variables:

- `OPENMSX_EXECUTABLE` — executable used to launch openMSX. Default: `openmsx`.
- `OPENMSX_ARGS` — optional extra launcher arguments, parsed as shell words. Default: empty.
- `OPENMSX_FLATPAK_APP` — if set, the launcher becomes `flatpak run <value>` and `OPENMSX_EXECUTABLE`/`OPENMSX_ARGS` are ignored. Example: `org.openmsx.openMSX`.
- `MSX_DEBUG_DELAY` — delay in emulated seconds before capture. Default: `5`.
- `MSX_DEBUG_MACHINE` — openMSX machine name passed with `-machine`. Default: `Sharp_HB-8000_1.2` (the Brazilian Hotbit), so the emulator never falls back to C-BIOS.
- `MSX_DEBUG_SETTINGS` — optional path to an openMSX `settings.xml` to load with `-setting`. When unset, an isolated settings file is generated automatically.
- `MSX_DEBUG_SCREENSHOT_MODE` — `scaled` (default) or `raw`.

Examples:

```bash
MSX_DEBUG_DELAY=10 ./scripts/debug-msx-memory.sh build/program.rom
```

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

## Important behavior

The launcher does not load the user's interactive `settings.xml`. openMSX aborts at startup when that file contains a key binding the installed version does not recognise (for example an unknown `PrintScreen` keycode), which would prevent the capture from running. Instead, the launcher writes a minimal settings file next to the ROM, passes it with `-setting`, and deletes it afterwards.

The machine is selected explicitly with `-machine Sharp_HB-8000_1.2`, so the emulator boots the Sharp HB-8000 (Hotbit) BIOS rather than falling back to C-BIOS. Set `MSX_DEBUG_MACHINE` to use a different machine, or `MSX_DEBUG_SETTINGS` to supply a custom settings file.

The delay uses openMSX's Tcl `after time`, which is based on emulated MSX seconds rather than host wall-clock time. This makes the capture point tied to emulation time.

The script exits openMSX after the artifacts have been written. The launcher returns the openMSX exit status to the calling agent.

Do not use `debug read` one byte at a time. The Tcl script uses `debug read_block` to retrieve the complete 16 KiB range in one operation.
