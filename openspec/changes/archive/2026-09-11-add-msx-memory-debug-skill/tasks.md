## 1. Settings isolation

- [x] 1.1 Generate a minimal settings file beside the ROM and pass it with `-setting`; verify the launcher no longer aborts when `~/.openMSX/share/settings.xml` contains an invalid key binding (e.g. `PrintScreen`)
- [x] 1.2 Remove the generated settings file on exit via an `EXIT` trap; verify no `.msx-memory-debug.*.xml` file remains after a run
- [x] 1.3 Support `MSX_DEBUG_SETTINGS` to load a custom settings file; verify a non-existent path fails with a clear error before launching openMSX

## 2. Machine selection

- [x] 2.1 Select `Sharp_HB-8000_1.2` by default via `-machine` and allow `MSX_DEBUG_MACHINE` to override it; verify the boot screenshot shows the HOTBIT (Sharp) BIOS and not C-BIOS

## 3. Memory and screenshot capture

- [x] 3.1 Read `Main RAM` at `0xC000..0xFFFF` with `debug read_block` and write the raw `.ram-c000-ffff.bin`; verify the artifact is exactly 16384 bytes
- [x] 3.2 Capture a PNG screenshot at the same capture point and honor `MSX_DEBUG_SCREENSHOT_MODE`; verify `raw` produces a raw-resolution PNG and `scaled` the default
- [x] 3.3 Schedule capture with `after time` in emulated seconds and validate `MSX_DEBUG_DELAY`; verify an invalid delay is rejected before launching openMSX
- [x] 3.4 Exit openMSX automatically after writing both artifacts and return its exit status to the caller

## 4. Documentation

- [x] 4.1 Document the skill purpose, workflow, configuration, and settings isolation in `SKILL.md`
- [x] 4.2 Document usage, settings isolation, and machine selection in `README.md`

## 5. End-to-end verification

- [x] 5.1 Run the skill on a real ROM and verify both artifacts are produced beside the ROM, the Sharp BIOS boots, and the temporary settings file is removed

## 6. Debugging strategies documentation

- [x] 6.1 Document the frozen-state probe workflow (write results to fixed RAM addresses with `POKE`, then halt) in `SKILL.md` and `README.md`
- [x] 6.2 Document symbol-based address mapping (`msxbas2rom -s --noi` for BASIC variables; `header.symbols.asm` for kernel work area) in `SKILL.md` and `README.md`
- [x] 6.3 Document CPU-level debugging with openMSX breakpoints (`debug set_bp`, `debug read_block "CPU regs"`, `debug read_block "Main RAM"`, `debug cont`, callback output to a file) and the 26-byte `CPU regs` layout in `SKILL.md` and `README.md`
- [x] 6.4 Document the kernel rebuild workflow (`make -C src/infrastructure/kernel/asm` then `make release`) and the pasmo absolute-address pitfall
- [x] 6.5 Document that MSX2-oriented BASIC commands use `MSX_DEBUG_MACHINE=Panasonic_FS-A1WX` while the MSX1 default stays `Sharp_HB-8000_1.2`

