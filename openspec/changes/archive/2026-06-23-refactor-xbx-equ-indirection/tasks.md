## 1. Remover EQU redundante WriteParamBCD

- [x] 1.1 Remover linha `WriteParamBCD: equ 0x6C0B` de `00_constants_and_workarea.asm`
- [x] 1.2 Verificar que `WriteParamBCD:` label em `90_support.asm:190` continua definido e compila

## 2. Redirecionar XBASIC_* EQUs para labels nativos (parte 1: funções matemáticas e de comparação)

- [x] 2.1 XBASIC_MULTIPLY_INTEGERS → J7631
- [x] 2.2 XBASIC_DIVIDE_INTEGERS → C7643
- [x] 2.3 XBASIC_ADD_FLOATS → C76D7
- [x] 2.4 XBASIC_SUBTRACT_FLOATS → C76D3
- [x] 2.5 XBASIC_MULTIPLY_FLOATS → C7748
- [x] 2.6 XBASIC_DIVIDE_FLOATS → C782C
- [x] 2.7 XBASIC_SIN → C79EA
- [x] 2.8 XBASIC_COS → C79E2
- [x] 2.9 XBASIC_TAN → I7A41
- [x] 2.10 XBASIC_ATN → I7A63
- [x] 2.11 XBASIC_EXP → J7AA8
- [x] 2.12 XBASIC_LOG → C7B01
- [x] 2.13 XBASIC_SQR → I7B60
- [x] 2.14 XBASIC_RND → I768E
- [x] 2.15 XBASIC_INT → C7999
- [x] 2.16 XBASIC_FIX → C798C
- [x] 2.17 XBASIC_SGN_INT → J_SGN_INT
- [x] 2.18 XBASIC_SGN_FLOAT → J_SGN_FLOAT
- [x] 2.19 XBASIC_ABS_INT → J_ABS_INT
- [x] 2.20 XBASIC_POWER_FLOAT_TO_INTEGER → C7878
- [x] 2.21 XBASIC_POWER_FLOAT_TO_FLOAT → I78BF
- [x] 2.22 XBASIC_COMPARE_FLOATS_EQ → C792A
- [x] 2.23 XBASIC_COMPARE_FLOATS_NE → C7933
- [x] 2.24 XBASIC_COMPARE_FLOATS_GT → C793C
- [x] 2.25 XBASIC_COMPARE_FLOATS_GE → C7946
- [x] 2.26 XBASIC_COMPARE_FLOATS_LT → C794F
- [x] 2.27 XBASIC_COMPARE_FLOATS_LE → C7958

## 3. Redirecionar XBASIC_* EQUs para labels nativos (parte 2: strings e conversão)

- [x] 3.1 XBASIC_LEFT → I7E42
- [x] 3.2 XBASIC_RIGHT → I7E49
- [x] 3.3 XBASIC_MID → C7E5A
- [x] 3.4 XBASIC_MID_ASSIGN → C7E81
- [x] 3.5 XBASIC_INSTR → C7F15
- [x] 3.6 XBASIC_VAL → C7EB0
- [x] 3.7 XBASIC_STRING → C7EF2
- [x] 3.8 XBASIC_OCT_HEX_BIN → I7EC9
- [x] 3.9 XBASIC_INKEY → C7F01
- [x] 3.10 XBASIC_COPY_STRING → C7F44 (label a ser criado no passo 5)
- [x] 3.11 XBASIC_COPY_STRING_TO_NULBUF → C7F40
- [x] 3.12 XBASIC_CONCAT_STRINGS → C7F99
- [x] 3.13 XBASIC_COMPARE_STRING_WITH_NULBUF_EQ → C7F55
- [x] 3.14 XBASIC_COMPARE_STRING_WITH_NULBUF_NE → C7F67
- [x] 3.15 XBASIC_COMPARE_STRING_WITH_NULBUF_GT → C7F4B
- [x] 3.16 XBASIC_COMPARE_STRING_WITH_NULBUF_GE → C7F5B
- [x] 3.17 XBASIC_COMPARE_STRING_WITH_NULBUF_LT → C7F61
- [x] 3.18 XBASIC_COMPARE_STRING_WITH_NULBUF_LE → C7F6D
- [x] 3.19 XBASIC_CAST_INTEGER_TO_FLOAT → C78DF
- [x] 3.20 XBASIC_CAST_INTEGER_TO_STRING → C7BD1
- [x] 3.21 XBASIC_CAST_FLOAT_TO_INTEGER → C7901
- [x] 3.22 XBASIC_CAST_FLOAT_TO_STRING → C7C2B
- [x] 3.23 XBASIC_CAST_STRING_TO_FLOAT → C7EB0

## 4. Redirecionar XBASIC_* EQUs para labels nativos (parte 3: gráficos e VDP)

- [x] 4.1 XBASIC_POINT → I6FA9
- [x] 4.2 XBASIC_LINE → C6DA9
- [x] 4.3 XBASIC_BOX → I6D4B
- [x] 4.4 XBASIC_BOXF → I6E29
- [x] 4.5 XBASIC_PSET → I6F73
- [x] 4.6 XBASIC_PAINT → C74BC
- [x] 4.7 XBASIC_CIRCLE → I74DC
- [x] 4.8 XBASIC_CIRCLE2 → J74EE
- [x] 4.9 XBASIC_COLOR_RGB → C710E
- [x] 4.10 XBASIC_COLOR_SPRITE → C7189
- [x] 4.11 XBASIC_COLOR_SPRSTR → C7194
- [x] 4.12 XBASIC_SCREEN_SPRITE → I70BE
- [x] 4.13 XBASIC_SPRITE_ASSIGN → C7145
- [x] 4.14 XBASIC_SET_PAGE → C70CC
- [x] 4.15 XBASIC_SET_SCROLL → C7004
- [x] 4.16 XBASIC_VDP → I7339
- [x] 4.17 XBASIC_VPEEK → I70A3
- [x] 4.18 XBASIC_VPOKE → I70B7

## 5. Redirecionar XBASIC_* EQUs para labels nativos (parte 4: traps, swap, print)

- [x] 5.1 XBASIC_TRAP_ON → I6C8B
- [x] 5.2 XBASIC_TRAP_OFF → I6C9E
- [x] 5.3 XBASIC_TRAP_STOP → C6CA7
- [x] 5.4 XBASIC_TRAP_CHECK → C6C27
- [x] 5.5 XBASIC_SWAP_INTEGER → I6BF7
- [x] 5.6 XBASIC_SWAP_STRING → I6BFB
- [x] 5.7 XBASIC_SWAP_FLOAT → I6BFF
- [x] 5.8 XBASIC_PRINT_TAB → C74F3
- [x] 5.9 XBASIC_PRINT_INT → C7508
- [x] 5.10 XBASIC_PRINT_FLOAT → C750D
- [x] 5.11 XBASIC_PRINT_CRLF → C74F7

## 6. Criar label C7F44 para XBASIC_COPY_STRING

- [x] 6.1 Adicionar `C7F44:` em `90_support.asm`, na linha após `LD C,(HL)` dentro da rotina `C7F40`
- [x] 6.2 Verificar que `XBASIC_COPY_STRING: equ C7F44` resolve corretamente

## 7. Verificação final

- [x] 7.1 Compilar kernel: `make` no diretório `src/infrastructure/kernel/asm` deve ter sucesso
- [x] 7.2 Verificar `header.symbols.asm`: todos os símbolos `XBASIC_*` nos mesmos endereços de antes
- [x] 7.3 Verificar binário: `stat -c%s bin/header.bin` = 32768 bytes (tamanho inalterado)
- [x] 7.4 Executar `make test-kernel` — todos os testes devem passar (11/11)
- [x] 7.5 Verificar que `WriteParamBCD` não causa mais erro de compilação "Invalid definition"

