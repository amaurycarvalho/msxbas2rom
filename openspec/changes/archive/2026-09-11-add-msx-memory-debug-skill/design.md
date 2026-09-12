## Context

The skill already existed at `.opencode/skills/msx-memory-debug-skill/` but did not
work on this machine: openMSX 19.1 aborts at startup while loading the user's
`~/.openMSX/share/settings.xml`, which contains `<unbind key="keyb PrintScreen"/>`,
and `PrintScreen` is not a valid keycode (the valid name is `Print`). Because the
abort happens before the Tcl capture script runs, no artifacts were produced. When
settings are isolated, openMSX also falls back to its default `C-BIOS_MSX2+`
machine, while the project targets the Brazilian Sharp HB-8000 (Hotbit).

See `proposal.md` for motivation and `specs/msx-memory-debug/spec.md` for the
behavior contract.

## Goals / Non-Goals

**Goals:**

- Make the skill work on this machine and on machines with arbitrary user settings.
- Keep the capture deterministic: same ROM, delay, machine, and artifact names.
- Make the emulator boot the Sharp HB-8000 BIOS by default, with an override.

**Non-Goals:**

- Replacing the interactive openMSX debugger or providing live stepping.
- Capturing VRAM, mapper registers, or CPU-bus memory other than `Main RAM`.
- Managing savestates, multiple machines, or Windows shells.

## Decisions

### Isolate openMSX settings instead of repairing the user's file

Generate a minimal settings file, pass it with `-setting`, and delete it on exit,
rather than editing `~/.openMSX/share/settings.xml`.

- Why: the skill must be self-contained and reproducible regardless of the host's
  interactive configuration, and must not mutate global user state.
- Alternative considered: fix the `PrintScreen` binding in the user's settings.
  Rejected as a skill-level fix because it does not protect other machines and
  couples the skill to a path it does not own.

### Select the machine explicitly with `-machine`

Pass `-machine Sharp_HB-8000_1.2` on every run; `MSX_DEBUG_MACHINE` overrides it.

- Why: an isolated settings file has no `default_machine`, so openMSX would boot
  C-BIOS. Explicit selection guarantees the Hotbit BIOS.
- Alternative considered: copy `default_machine` out of the user's `settings.xml`.
  Rejected because it is fragile (path and file format vary) and would silently
  fall back to C-BIOS when the file is absent.

### Generate the temporary settings file beside the ROM

Write the generated settings file into the ROM's directory and remove it with an
`EXIT` trap.

- Why: sandboxed (Flatpak) openMSX builds do not see the host `/tmp`, but the ROM
  directory is guaranteed reachable because the ROM itself is passed by path.
- Alternative considered: use `$TMPDIR`. Rejected for Flatpak compatibility.

### Pass parameters through `user_setting` Tcl globals

The launcher creates `msxDebugRomPath`, `msxDebugDelay`, and
`msxDebugScreenshotMode` as openMSX user settings; the Tcl script reads them as
global variables and destroys them before exiting.

- Why: avoids shell/Tcl quoting and interpolation problems when the ROM path
  contains spaces or braces.
- Alternative considered: inline `-command` substitutions. Rejected for quoting
  fragility.

### Read memory with a single bulk debugger read

Use `debug read_block "Main RAM" 0xC000 0x4000`.

- Why: one operation returns the whole 16 KiB block; the command reference
  documents `debug read_block` for bulk reads.
- Alternative considered: a per-byte `debug read` loop. Rejected as slow and
  explicitly discouraged.

### Schedule capture with `after time` (emulated seconds)

- Why: ties the capture point to emulation time, making runs deterministic and
  independent of host performance.
- Alternative considered: `after realtime`. Rejected because it drifts with host
  load.

### Capture a frozen state, not a moving target

A fixed emulated delay does not guarantee the program is at the state of interest. The
recommended workflow is to have the debug program compute the values under inspection,
`POKE` them into fixed RAM addresses, and then halt in an infinite loop, so every capture
sees the same final state. The RAM dump is then read with a script and the probe bytes are
located at `address - 0xC000`.

- Why: a ROM that is still looping or updating when the delay elapses yields a different
  snapshot on each run, making comparisons unreliable.
- Alternative considered: raising the delay until the program ends. Rejected because the
  program may end at an unpredictable time and later state may overwrite the values.

### Map addresses with symbols before interpreting the dump

Compile the ROM with `msxbas2rom -s --noi` to obtain a `.noi` symbol file that maps BASIC
variables (for example `VAR_X%`) to RAM addresses. Kernel work-area symbols (`SPRTBL`,
`HITBOX_TABLE`, `SPRSIZ`, and similar) are read from
`src/infrastructure/kernel/asm/src/header.symbols.asm`. This makes the raw 16 KiB dump
readable without guessing offsets.

### Use openMSX breakpoints for CPU-level questions

The base skill intentionally only dumps RAM and a screenshot. When the question is about
register values, exact instruction flow, or a routine that runs many times, a companion
openMSX Tcl script is used: `debug set_bp` on the address of interest with a callback that
reads `debug read_block "CPU regs" 0 26` and `debug read_block "Main RAM" ...`, writes the
result to a file, and calls `debug cont`. Placing the breakpoint on a success/terminal path
isolates the interesting occurrence. Tcl `puts` is not reliably visible on stdout, so the
callback writes to a file.

The `CPU regs` debuggable is a 26-byte block whose layout had to be confirmed empirically:
`F, A, B, C, D, E, H, L, F', A', B', C', D', E', H', L', IXh, IXl, IYh, IYl, SPh, SPl,
PCh, PCl, I, R`.

- Why: bulk RAM/screenshot capture cannot answer register-level questions.
- Alternative considered: extend the skill's Tcl script to always set breakpoints. Rejected
  because breakpoints are question-specific and would complicate the general capture.

### Rebuild the embedded kernel header after kernel edits

The Z80 kernel is embedded in the compiler as the generated `header.h`. Editing
`src/infrastructure/kernel/asm/src/header/*.asm` requires
`make -C src/infrastructure/kernel/asm` (pasmo assembles `header.bin`, `xxd` regenerates
`header.h`) followed by `make release`. Referencing absolute RAM addresses inside kernel
code for debug stores can make pasmo emit a much larger `header.bin` (observed 32 KiB to
64 KiB); prefer existing work-area symbols or keep such instrumentation temporary.

### Keep the Sharp HB-8000 default and select an MSX2 machine explicitly

The default `Sharp_HB-8000_1.2` remains the MSX1 target. MSX2-oriented BASIC commands
(`SCREEN 7`, MSX2-only graphics/VDP features) SHALL be debugged on the Panasonic FS-A1WX
(`MSX_DEBUG_MACHINE=Panasonic_FS-A1WX`), which boots an MSX2 machine, rather than the MSX1
Hotbit.

- Why: MSX2-only commands and VDP behavior are not representative on an MSX1 machine.
- Alternative considered: change the skill default to an MSX2 machine. Rejected because the
  project's primary target is the Brazilian MSX1 Hotbit.

## Risks / Trade-offs

- [Risk] The temporary settings file is left behind on `SIGKILL` → Mitigation: the
  `EXIT` trap removes it on normal and error exits; the name is unique per run.
- [Risk] `Sharp_HB-8000_1.2` BIOS is unavailable on some machine → Mitigation:
  openMSX reports the missing ROM; the user can override with `MSX_DEBUG_MACHINE`
  or `MSX_DEBUG_SETTINGS`.
- [Risk] `Main RAM` contents are not the CPU bus when page 3 is mapped elsewhere →
  Mitigation: documented as a known limitation in `SKILL.md`/`README.md`.
- [Risk] A user-supplied `MSX_DEBUG_SETTINGS` file may be read-only, causing an
  openMSX auto-save warning on exit → Mitigation: only a warning; artifacts are
  still produced.

## Migration Plan

No migration required. The skill previously produced no artifacts on this machine;
after the change it produces them. There is no product code, build, or runtime
impact, so there is no rollback concern beyond reverting the skill files.

## Open Questions

None.

## References

openMSX manual, used as the authoritative source of information about the emulator
and the commands this skill relies on:

- <https://openmsx.org/manual/user.html> — general usage of the emulator.
- <https://openmsx.org/manual/commands.html> — console command reference for
  `after`, `debug read_block`, `screenshot`, `user_setting`, and `exit`.
- <https://openmsx.org/manual/openmsx-control.html> — controlling openMSX from
  external applications (non-interactive/remote control model).

## openMSX command-line syntax

The launcher is built from these openMSX command-line options. The skill uses
`-setting`, `-machine`, `-cart`, `-command`, and `-script`; the remaining options
are listed for reference. The syntax below is from openMSX 21.0 (the options used
by the skill are also present in openMSX 19.1).

```text
openMSX 21.0
============

usage: openmsx [arguments]
  an argument is either an option or a filename

  this is the list of supported options:
    --help, -h      Shows this text
    --version, -v   Prints openMSX version and exits
    -cart, -carta, 
    -cartb, -cartc, 
    -cartd          Insert the ROM file (cartridge) specified in
                    argument
    -cassetteplayer Put cassette image specified in argument in
                    virtual cassetteplayer
    -cda            Use iso image in argument for the CD-ROM extension
    -command        Run Tcl command at startup (see also -script)
    -control        Enable external control of openMSX process
    -diska, -diskb  Insert the disk image specified in argument
    -ext, -exta, 
    -extb, -extc, 
    -extd           Insert the extension specified in argument
    -hda            Use hard disk image in argument for the IDE or
                    SCSI extensions
    -ips            Apply the given IPS patch to the ROM or disk
                    image specified in the preceding option
    -laserdisc      Put LaserDisc image specified in argument in
                    virtual LaserDisc player
    -machine        Use machine specified in argument
    -replay         Load replay and start replaying it in view only
                    mode
    -romtype        Specify the rom type for the ROM image specified
                    in the preceding option
    -savestate      Load savestate and start emulation from there
    -script         Run extra startup script
    -setting        Load an alternative settings file
    -setup          Use setup file specified in argument
    -testconfig     Test if the specified config works and exit

  this is the list of supported file types:
    cas, tsx, wav   Cassette image, raw recording, fMSX CAS image or
                    TSX file
    di1, di2, dmk, 
    dsk, fd1, fd2, 
    xsa             Disk image
    mx1, mx2, ri, rom, 
    sg              ROM image of a cartridge
    ogv             LaserDisc image, Ogg Vorbis/Theora
    omr             openMSX replay
    oms             openMSX savestate
    tcl             Extra Tcl script to run at startup 
```
