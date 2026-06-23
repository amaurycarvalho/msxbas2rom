## Context

O compilador mantém um buffer de saída para o código Z80 gerado, definido por `COMPILE_CODE_SIZE = COMPILE_MAX_PAGES * 0x4000`. Atualmente `COMPILE_MAX_PAGES = 64` (1MB). Esse buffer é usado em:

- `compiler.cpp:54`: inicialização do `CpuWorkspaceContext` com `COMPILE_CODE_SIZE` como limite
- `compiler.cpp:360-366`: verificação de estouro — se `cpu->context->error` é `true`, emite erro de limite excedido
- `rom.cpp:166`: `vector<unsigned char> buf(COMPILE_CODE_SIZE)` como buffer temporário para o código compilado
- `tests/unit/test_compiler.cpp`: inicialização de contexto nos testes

Enquanto isso, o resource manager (`resource_manager.cpp:158`) já permite segmentos até 255 (2048KB), e todo o armazenamento de segmentos opera com 8 bits (0-255). O gargalo está exclusivamente no buffer do compilador.

O projeto tem um change ativo `add-16bit-segments-ascii16-4mb-ascii16x-8mb` que expande ASCII16 para 4MB e ASCII16X para 8MB usando segmentos de 16 bits. Este change é independente — lida com o teto de 2MB que os formatos de 8 bits já podem atingir arquiteturalmente.

## Goals / Non-Goals

**Goals:**
- Permitir que código Z80 compilado ocupe até 2MB (128 páginas de 16KB) no buffer do compilador
- Garantir que todos os formatos MegaROM (ASCII8, ASCII16, ASCII16X, Konami4, KonamiSCC) possam atingir o limite de 2048KB com segmentos de 8 bits

**Non-Goals:**
- Expandir formatos específicos além de 2MB (isso é tratado pelo change `add-16bit-segments-ascii16-4mb-ascii16x-8mb`)
- Alterar a largura de segmentos (permanece 8 bits)
- Modificar kernel ASM, resource map, ROM header ou bridges cross-segment
- Adicionar limites por formato de mapper

## Decisions

### Decisão 1: Aumentar COMPILE_MAX_PAGES de 64 para 128

**Alternativa considerada:** Tornar o limite configurável via CLI (`--max-pages`).

**Decisão:** Aumentar a constante diretamente, sem flag CLI.

**Racional:** O custo de memória (+1MB) é irrelevante em hardware moderno (máquinas de desenvolvimento têm gigabytes). Um flag CLI adicionaria complexidade sem benefício prático — o usuário sempre quer o máximo possível de espaço. Se um limite menor for necessário no futuro (ex.: sistemas embarcados para compilação cruzada), pode-se adicionar o flag depois.

### Decisão 2: Não modificar o resource manager

O resource manager já verifica `resourceBlockSegment > 255` (2048K). Nenhuma mudança necessária — o limite de 255 segmentos de 8 bits cobre exatamente 2MB para todos os formatos.

### Decisão 3: Manter independência do change 16-bit segments

Este change altera apenas `compiler.h`. O change `add-16bit-segments` altera `compiler.cpp`, `resource_manager.cpp`, `rom.cpp` e kernel ASM — áreas completamente disjuntas. Os dois changes podem ser aplicados em qualquer ordem ou combinados.

## Risks / Trade-offs

- **Memória +1MB em tempo de compilação**: O `vector<unsigned char>` em `rom.cpp:166` e o buffer interno do `CpuWorkspaceContext` alocam 2MB em vez de 1MB. Em sistemas com <4GB RAM compilando múltiplas instâncias em paralelo, pode haver pressão de memória. Mitigação: o uso típico é single-thread (CLI), e 1MB adicional é insignificante.
- **Mensagem de erro desatualizada nos testes**: Testes unitários que esperam a mensagem "1MB" podem quebrar se referenciarem `COMPILE_MAX_PAGES` literalmente. Mitigação: a mensagem usa `to_string(COMPILE_CODE_SIZE)` dinamicamente, então deve se ajustar automaticamente. Verificar durante implementação.
