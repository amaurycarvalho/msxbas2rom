## Why

O sistema atual armazena números de segmento como valores de 8 bits em múltiplos pontos críticos — ROM header, resource map, kernel `MR_CHANGE_SGM`, bridges de código e fixups — limitando o tamanho máximo do MegaROM a 2MB (255 segmentos × 8KB). Para suportar ROMs de 4MB no mapper ASCII16 (256 páginas de 16KB = 512 segmentos de 8KB) e 8MB no mapper ASCII16-X (512 páginas de 16KB = 1024 segmentos de 8KB), é necessário migrar o tratamento interno de segmentos para 16 bits com o mínimo de alteração de código.

## What Changes

- **Segmentos como variáveis de 16 bits** em todo o pipeline (compilador, builder, kernel) substituindo o armazenamento atual de 8 bits
- **Resource map** expandido de 5 para 6 bytes por entrada (segmento de 1→2 bytes)
- **ROM header** (`start.asm`) com segmento do resource map de 2 bytes (desloca `BASTEXT`)
- **Kernel workarea** (`RSCMAPSG`, `RSCMAPT1`) expandido de 1 para 2 bytes cada (desloca `PLYSGTM` e `BASMEM`)
- **Kernel `MR_CHANGE_SGM`** reescrito para receber segmento em registrador de 16 bits (HL); versão ASCII16 faz shift right de 16 bits (/2); versão ASCII16-X adicionalmente codifica bits altos via address encoding (A8-A11)
- **Compiler bridges** expandidos de 8 para 9 bytes (`LD A,<8>` → `LD HL,<16>`) apenas para modos ASCII16/ASCII16X
- **Cross-segment fixups** escrevem 2 bytes de segmento ao invés de 1
- **Limite de resource manager** ampliado de 255 para 65535 segmentos
- **Mantém convivência ASCII8 ↔ ASCII16**: compilador segue calculando segmentos no estilo ASCII8 (dois por página de 16KB), kernel converte via patching
- **Compatibilidade total** com mappers existentes (ASCII8, Konami4, KonamiSCC) que continuam usando 8 bits
- **Testes unitários** para validação dos novos limites, formatos e patches de kernel

## Capabilities

### New Capabilities

- `16bit-segments`: Tratamento interno de segmentos como valores de 16 bits em todos os pontos do pipeline (compilador, builder, resource manager, kernel), com fallback condicional a 8 bits para mappers legados
- `ascii16-4mb-roms`: Suporte a ROMs de até 4MB no mapper ASCII16, com kernel patch que converte segmentos 16-bit do estilo ASCII8 para páginas de 16KB via shift right de 16 bits
- `ascii16x-8mb-roms`: Suporte a ROMs de até 8MB no mapper ASCII16-X, com kernel patch usando address encoding (A8-A11) para bits altos do número de banco de 12 bits

### Modified Capabilities

- `builder`: Resource map passa de 5 para 6 bytes por entrada; ROM header (`start.asm`) com resource.map.segment de 2 bytes; limite de MegaROM ampliado condicionalmente por mapper
- `compiler`: Bridges de segmento e fixups cross-segment emitem valores de 16 bits para modos ASCII16/ASCII16X; mantém 8 bits para ASCII8/Konami
- `kernel-call-routing`: Novas rotinas `MR_CHANGE_SGM_16` e `MR_CHANGE_SGM_16X` no kernel; novos labels de dispatch table; resource.address lê segmento de 2 bytes

## Impact

- **C++ source**: `compiler.cpp` (bridge + fixup + segm_total), `compiler_fixup_resolver.cpp` (preamble), `rom.cpp` (header + patch + limits), `resource_manager.cpp` (map entry + limit), `rom.h`, `compiler_hooks.h` (DISP entries)
- **ASM kernel**: `61_megarom.asm` (MR_CHANGE_SGM_16/16X), `80_resources.asm` (resource.address 2-byte), `00_constants_and_workarea.asm` (workarea layout), `20_runtime.asm` (dispatch table), `start.asm` (header layout)
- **Regeneração**: `make header` necessário após alterações ASM; `header.h`, `header.symbols.asm`, constantes `BIN_HEADER_BIN_LEN`
- **Definições C++**: `compiler_hooks.h` — `def_BASMEM`, `def_MR_TRAP_SEGMS`, novos `DISP_*` constants
- **Testes**: `test_rom.cpp`, `test_resources.cpp`, `test_kernel.cpp`
- **Target**: Release 1.5.0.0
