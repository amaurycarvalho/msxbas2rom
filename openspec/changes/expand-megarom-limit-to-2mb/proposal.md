## Why

O compilador aloca um buffer de saída de 1MB (`COMPILE_MAX_PAGES = 64` × 16KB) para o código Z80 gerado, limitando efetivamente qualquer MegaROM ao mesmo teto de 1MB — embora o pipeline de resource manager e o armazenamento de segmentos de 8 bits já suportem 2MB (256 segmentos × 8KB = 2048KB). O gargalo está exclusivamente no `COMPILE_CODE_SIZE`, impedindo que todos os formatos MegaROM (ASCII8, ASCII16, ASCII16X, Konami4, KonamiSCC) atinjam o limite arquitetural de 2048KB disponível com segmentos de 8 bits.

## What Changes

- `#define COMPILE_MAX_PAGES` em `compiler.h` passa de `(16 * 4)` para `(16 * 8)`, dobrando `COMPILE_CODE_SIZE` de 1MB para 2MB
- Nenhuma mudança estrutural no pipeline de segmentos — os 8 bits atuais (0-255) já cobrem exatamente 2MB
- Nenhuma mudança no kernel ASM, resource map, ROM header ou bridges cross-segment

## Capabilities

### New Capabilities

- `megarom-2mb-max-size`: O compilador permite gerar código Z80 de até 2MB (128 páginas de 16KB), suficiente para que todos os formatos MegaROM (ASCII8, ASCII16, ASCII16X, Konami4, KonamiSCC) atinjam o limite arquitetural de 2048KB com segmentos de 8 bits

### Modified Capabilities

<!-- Nenhuma — os requisitos existentes do builder e compiler não especificam valores de limite, apenas comportamentos -->

## Impact

- **C++ source**: `src/application/compiler/compiler.h` (constante `COMPILE_MAX_PAGES`)
- **Memória em tempo de compilação**: +1MB no buffer de código (`vector<unsigned char>` em `rom.cpp:166` e `CpuWorkspaceContext` em `compiler.cpp:54`)
- **Target**: Release 1.1.0.0
