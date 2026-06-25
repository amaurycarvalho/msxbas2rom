## ADDED Requirements

### Requirement: Compiler emits resource number via HL without intermediate DAC write

The C++ compiler SHALL NOT emit `cpu.addLdiiHL(def_DAC)` for any resource-consuming CMD handler or function. Since the Z80 expression evaluator places integer results in HL, resource numbers SHALL flow directly from HL into the kernel routine via `addKernelCall(DISP_*)` without an intermediate write to the DAC work area (`0xF7F6`). The kernel routines (`cmd_play`, `cmd_draw`, `cmd_restore`, `cmd_screen_load`, `cmd_rsctoram`, `cmd_wrtscr`, `cmd_wrtspr`, `cmd_wrtfnt`, `cmd_wrtclr`, `cmd_wrtchr`, `cmd_wrtspratr`, `cmd_wrtsprclr`, `cmd_wrtsprpat`, `cmd_wrtvram`, `cmd_mtf`, `cmd_plyload`, `cmd_setfnt`, `cmd_runasm`, and the BLOAD/SCREEN/SPRITE/START compiler strategies) SHALL receive the resource number in HL directly.

#### Scenario: CMD PLAY compiler handler omits DAC write

- **WHEN** the compiler processes `CMD PLAY <resource expr>`
- **THEN** the generated code SHALL NOT contain `ld (0xF7F6), hl`
- **AND** the generated code SHALL contain `call <cmd_play>` directly after expression evaluation

#### Scenario: CMD RESTORE compiler handler omits DAC write

- **WHEN** the compiler processes `CMD RESTORE <resource expr>`
- **THEN** the generated code SHALL NOT contain `ld (0xF7F6), hl`

#### Scenario: BLOAD statement omits DAC write

- **WHEN** the compiler processes `BLOAD "<filename>"`
- **THEN** the generated code SHALL embed the resource number as `ld hl, <literal>`
- **AND** the generated code SHALL NOT contain `ld (0xF7F6), hl`

### Requirement: Non-resource CMD parameters use PARM1 work area

The C++ compiler SHALL emit `cpu.addLdiiHL(def_PARAM1)` (writing to `0xF6E8`) instead of `cpu.addLdiiHL(def_DAC)` for CMD handlers whose parameter is NOT a resource number: `cmd_turbo`, `cmd_plyloop`, `cmd_plysong`, `cmd_plysound`. The `def_PARAM1` constant (`0xF6E8`) is already defined in `compiler_hooks.h` line 440.

#### Scenario: CMD TURBO compiler handler writes to PARM1

- **WHEN** the compiler processes `CMD TURBO <flag expr>`
- **THEN** the generated code SHALL contain `ld (0xF6E8), hl`
- **AND** the generated code SHALL NOT contain `ld (0xF7F6), hl`

#### Scenario: CMD PLYSONG compiler handler writes to PARM1

- **WHEN** the compiler processes `CMD PLYSONG <subsong expr>`
- **THEN** the generated code SHALL contain `ld (0xF6E8), hl`
- **AND** the generated code SHALL NOT contain `ld (0xF7F6), hl`

### Requirement: No resource-related DAC references remain in generated code

After migration, the only uses of the DAC work area (`0xF7F6`) in generated code SHALL be those required by the MSX BASIC math pack and string temporary operations (i.e., `addCallBDOSWE`, float-to-BCD conversion). No resource-consuming or non-resource CMD handler SHALL reference `(DAC)` for parameter passing.

#### Scenario: Generated ROM code has zero resource-related DAC writes

- **WHEN** a compiled ROM is disassembled
- **THEN** the instruction `ld (0xF7F6), hl` SHALL NOT appear before any `call <cmd_play>`, `call <cmd_draw>`, `call <cmd_restore>`, `call <cmd_screen_load>`, `call <cmd_rsctoram>`, `call <cmd_wrtscr>`, `call <cmd_wrtspr>`, `call <cmd_wrtfnt>`, `call <cmd_wrtclr>`, `call <cmd_wrtchr>`, `call <cmd_wrtspratr>`, `call <cmd_wrtsprclr>`, `call <cmd_wrtsprpat>`, `call <cmd_wrtvram>`, `call <cmd_mtf>`, `call <cmd_plyload>`, `call <cmd_setfnt>`, `call <cmd_runasm>`, or `call <usr0>`, `call <usr1>`
