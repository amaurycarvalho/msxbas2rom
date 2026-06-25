## 1. Kernel — Rotinas resource.* em 60_bios_helpers.asm e 80_resources.asm

- [ ] 1.1 Atualizar `resource.open` (`60_bios_helpers.asm:5-22`): trocar comentário `in: DAC = resource number` por `in: hl = resource number (reserved)`
- [ ] 1.2 Atualizar `resource.open_and_get_address` (`60_bios_helpers.asm:45-48`): substituir `ld bc, (DAC)` por `push hl` + `call resource.open` + `pop hl`, remover comentário `in: DAC = resource number`, adicionar `in: hl = resource number`
- [ ] 1.3 Atualizar `resource.address` (`80_resources.asm:24-63`): trocar entrada de `bc` para `hl`; adicionar `push hl` / `call resource.count` / `pop bc` no início; trocar `cp b`/`cp c` para usar BC (já preservado); atualizar comentário do contrato
- [ ] 1.4 Atualizar `resource.get_data` (`80_resources.asm:67-86`): trocar `push bc` por `push hl` e `pop bc` por `pop hl`; atualizar comentário de entrada

## 2. Kernel — CMD handlers em 31_cmd.asm

- [ ] 2.1 `cmd_play` (linha 8): remover `ld bc, (DAC)` — HL já tem o resource number
- [ ] 2.2 `cmd_draw` (linha 48): remover `ld bc, (DAC)`
- [ ] 2.3 `cmd_runasm` (linha 59): remover `ld bc, (DAC)`
- [ ] 2.4 `cmd_rsctoram` (linha 307): remover `ld (DAC), hl` (HL já é passado direto para `resource.open_and_get_address`)
- [ ] 2.5 `cmd_restore` (linhas 549-558): sem alteração no corpo (usa `resource.open_and_get_address` que já migrou para HL); verificar que HL chega corretamente
- [ ] 2.6 `cmd_screen_load` (linhas 1634-1648): trocar `ld (DAC), hl` por `ld (PARM1), hl` e `ld (DAC+2), a` por `ld (PARM1+2), a`; atualizar comentários
- [ ] 2.7 `cmd_mtf` (linhas 1688-1690): remover `ld (DAC), hl` (HL de `MTF_RESN_PARM` já está pronto); passar HL direto para `resource.open_and_get_address`
- [ ] 2.8 `cmd_turbo` (linhas 537, 543): trocar `ld a, (DAC)` por `ld a, (PARM1)`
- [ ] 2.9 `cmd_wrtvram`, `cmd_wrtfnt`, `cmd_wrtspr`, `cmd_wrtchr`, `cmd_wrtclr`, `cmd_wrtscr`, `cmd_wrtsprpat`, `cmd_wrtsprclr`, `cmd_wrtspratr`: verificar que o resource number chega via HL implicitamente (via `resource.copy_to_ram_on_page_*` que chama `resource.open_and_get_address` já migrado)

## 3. Kernel — 30_basic_helpers.asm (XBASIC_BLOAD scratch)

- [ ] 3.1 `XBASIC_BLOAD` (linhas 1045-1049): atualizar comentários de `DAC`/`DAC+2` para `PARM1`/`PARM1+2`
- [ ] 3.2 `XBASIC_BLOAD.get_next_block` (linha 1110): trocar `ld hl, (DAC)` por `ld hl, (PARM1)`
- [ ] 3.3 `XBASIC_BLOAD.get_next_block` (linha 1111): trocar `ld a, (DAC+2)` por `ld a, (PARM1+2)`
- [ ] 3.4 `XBASIC_BLOAD.get_next_block` (linha 1128): trocar `ld a, (DAC+2)` por `ld a, (PARM1+2)`
- [ ] 3.5 `XBASIC_BLOAD.get_next_block` (linha 1131): trocar `ld (DAC+2), a` por `ld (PARM1+2), a`
- [ ] 3.6 `XBASIC_BLOAD.get_next_block` (linha 1135): trocar `ld (DAC), de` por `ld (PARM1), de`
- [ ] 3.7 Verificar `XBASIC_READ` (linha 652), `XBASIC_IREAD` (linha 672), `XBASIC_RESTORE` (linha 712): estas rotinas chamam `resource.open` para slot selection mas não precisam do resource number (usam contexto DATLIN/DORES). Nenhuma alteração necessária.

## 4. Kernel — 32_usr.asm (USR functions)

- [ ] 4.1 `usr0` (linhas 10-13): trocar `pop bc` por `pop hl` (resource number permanece em HL para `resource.address`)
- [ ] 4.2 `usr1` (linhas 26-29): trocar `pop bc` por `pop hl`

## 5. Kernel — 33_player.asm (Arkos Tracker)

- [ ] 5.1 `cmd_plyload` (linha 118): verificar que HL chega com resource number do caller (DAC não é mais usado)
- [ ] 5.2 `cmd_plyload` (linha 130): trocar `ld bc, (ARG)` por `ld hl, (ARG)` (agora `resource.address` aceita HL); linha 131 já chama `resource.address`
- [ ] 5.3 `cmd_plyloop` (linha 224): trocar `ld a, (DAC)` por `ld a, (PARM1)`
- [ ] 5.4 `cmd_plysong` (linha 165): trocar `ld a, (DAC)` por `ld a, (PARM1)`
- [ ] 5.5 `cmd_plysound` (linha 250): trocar `ld a, (DAC)` por `ld a, (PARM1)`
- [ ] 5.6 `player.int.play.50hz` (linha 81), `player.int.mute` (linha 95): chamam `resource.open` para slot selection sem resource number. Nenhuma alteração necessária (HL pode ser qualquer valor).

## 6. C++ Compiler — Resource-consuming handlers (remover ld (DAC),hl)

- [ ] 6.1 `compiler_bload_statement_strategy.cpp`: remover `cpu.addLdiiHL(def_DAC)` da estratégia BLOAD (linha ~50)
- [ ] 6.2 `compiler_cmd_restore_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.3 `compiler_cmd_rsctoram_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)` (já usa HL via push/pop)
- [ ] 6.4 `compiler_cmd_wrtscr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.5 `compiler_cmd_wrtspr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.6 `compiler_cmd_wrtfnt_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.7 `compiler_cmd_wrtclr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.8 `compiler_cmd_wrtchr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.9 `compiler_cmd_wrtspratr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.10 `compiler_cmd_wrtsprclr_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.11 `compiler_cmd_wrtsprpat_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.12 `compiler_cmd_wrtvram_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)` (resource number via HL; dest address ainda vai em ARG)
- [ ] 6.13 `compiler_cmd_draw_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.14 `compiler_cmd_setfnt_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)` (ambas as ocorrências — linhas ~21 e ~37)
- [ ] 6.15 `compiler_cmd_play_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)` (ambas as ocorrências)
- [ ] 6.16 `compiler_cmd_plyload_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.17 `compiler_cmd_runasm_handler.cpp`: remover `cpu.addLdiiHL(def_DAC)`
- [ ] 6.18 `compiler_cmd_ramtoram_handler.cpp`: verificar se usa DAC para resource number; remover se aplicável
- [ ] 6.19 `compiler_cmd_ramtovram_handler.cpp`: verificar; remover se aplicável
- [ ] 6.20 `compiler_cmd_vramtoram_handler.cpp`: verificar; remover se aplicável
- [ ] 6.21 `compiler_screen_statement_strategy.cpp`: remover `cpu.addLdiiHL(def_DAC)` da estratégia SCREEN LOAD (linha ~319)
- [ ] 6.22 `compiler_sprite_statement_strategy.cpp`: remover `cpu.addLdiiHL(def_DAC)` da estratégia SPRITE LOAD (linha ~72)
- [ ] 6.23 `compiler_start_statement_strategy.cpp`: remover `cpu.addLdiiHL(def_DAC)` da estratégia START (linha ~78)
- [ ] 6.24 `compiler_set_statement_strategy.cpp`: verificar se o `ld (DAC), hl` com valor 0 em SET TILE ON (linha ~566) é resource-related; se for reset de DAC, manter mas trocar para PARM1

## 7. C++ Compiler — Non-resource handlers (DAC → PARM1)

- [ ] 7.1 `compiler_cmd_turbo_handler.cpp`: trocar `cpu.addLdiiHL(def_DAC)` por `cpu.addLdiiHL(def_PARAM1)`
- [ ] 7.2 `compiler_cmd_plyloop_handler.cpp`: trocar `cpu.addLdiiHL(def_DAC)` por `cpu.addLdiiHL(def_PARAM1)`
- [ ] 7.3 `compiler_cmd_plysong_handler.cpp`: trocar `cpu.addLdiiHL(def_DAC)` por `cpu.addLdiiHL(def_PARAM1)`
- [ ] 7.4 `compiler_cmd_plysound_handler.cpp`: trocar `cpu.addLdiiHL(def_DAC)` por `cpu.addLdiiHL(def_PARAM1)`

## 8. Verificação e testes

- [ ] 8.1 Recompilar o kernel (`make kernel` ou equivalente) e verificar que o binário não excede 32768 bytes
- [ ] 8.2 Verificar que `wrapper_routines_map_table` não mudou de tamanho ou ordem (índices DISP_* estáveis)
- [ ] 8.3 Executar `make build` e verificar que o executável compila sem erros
- [ ] 8.4 Executar suíte de testes unitários (`make test` ou equivalente) e verificar zero regressões
- [ ] 8.5 Executar testes de integração (todos os `.bas` em `tests/integration/`) e verificar que geram ROMs válidas
- [ ] 8.6 Inspecionar desassemblagem de uma ROM de teste para confirmar ausência de `ld (0xF7F6), hl` antes de chamadas resource-related
- [ ] 8.7 Inspecionar desassemblagem para confirmar `ld (0xF6E8), hl` presente antes de `call <cmd_turbo>`, `call <cmd_plyloop>`, `call <cmd_plysong>`, `call <cmd_plysound>`
