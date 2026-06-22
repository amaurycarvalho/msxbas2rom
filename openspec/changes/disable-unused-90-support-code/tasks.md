## 1. Desativar tabela de erros BASIC (J4DFA - I4E28)

- [ ] 1.1 Comentar código de J4DFA até I4E28 (linhas 60-87): 12 entradas da jump table de erros + rotina I4E28 de erro de modo de tela
- [ ] 1.2 Adicionar `DEFS 47, 0x00` (0x6455 - 0x6426 = 47 bytes) com comentário explicativo
- [ ] 1.3 Verificar que `PAINT_FIX.2` permanece no endereço 0x6455

## 2. Desativar Get Sprite Definition (C716E - J7183)

- [ ] 2.1 Comentar código de C716E, J717D e J7183 (linhas 1228-1241): handler de leitura de definição de sprite
- [ ] 2.2 Adicionar `DEFS 25, 0x00` (0x7189 - 0x716E = 27 bytes?) — verificar tamanho exato com `pasmo --msx header.asm header.bin --map`
- [ ] 2.3 Verificar que `C7189` (change sprite color) permanece no endereço 0x7189

## 3. Desativar rotinas de INPUT (I7517 - C75BE) e J761C

- [ ] 3.1 Comentar código de I7517 até C75BE (linhas 1899-2078): prompt de input numérico/string, busca de DATA, mensagem `I7530 "?Redo from start"`
- [ ] 3.2 Comentar código de J761C (linhas 2073-2078): helper de skip de bytes por tipo de token
- [ ] 3.3 Adicionar `DEFS` com tamanho calculado: `C7624 - I7517` bytes, com comentário
- [ ] 3.4 Verificar que `C7624` (multiply integers) permanece no endereço 0x7624

## 4. Desativar C76CE (remainder de divisão)

- [ ] 4.1 Comentar código de C76CE (linhas 2223-2225): rotina que calcula resto com `CALL C7643; EX DE,HL; RET`
- [ ] 4.2 Adicionar `DEFS 5, 0x00` (0x76D3 - 0x76CE = 5 bytes)
- [ ] 4.3 Verificar que `C76D3` (subtract floats) permanece no endereço 0x76D3

## 5. Desativar C78CD (unsigned int to float)

- [ ] 5.1 Comentar código de C78CD (linhas 2604-2612): conversão unsigned int→float
- [ ] 5.2 Adicionar `DEFS` com tamanho calculado: `C78DF - C78CD` bytes
- [ ] 5.3 Verificar que `C78DF` (int to float) permanece no endereço 0x78DF

## 6. Desativar C7962 (float <==>)

- [ ] 6.1 Comentar código de C7962 (linhas 2749-2750): função trivial `LD HL,-1; RET`
- [ ] 6.2 Adicionar `DEFS 2, 0x00` (0x7966 - 0x7962 = 4 bytes? verificar)
- [ ] 6.3 Verificar que `C7966` (compare floats) permanece no endereço 0x7966

## 7. Desativar concatenação de strings (C7F99 - J7FF6)

- [ ] 7.1 Comentar código de C7F99 até J7FF6 (linhas 3999-4076): concatenação de strings + helpers internos
- [ ] 7.2 Adicionar `DEFS` com tamanho calculado: `BASIC_KUN_END_FILLER - C7F99` bytes
- [ ] 7.3 Verificar que `BASIC_KUN_END_FILLER` permanece no endereço 0x7FFE

## 8. Verificação final

- [ ] 8.1 Compilar header: `pasmo --msx header.asm header.bin --map header.map`
- [ ] 8.2 Comparar binário com original: `xxd header.bin` vs `xxd header_original.bin` — devem ser idênticos
- [ ] 8.3 Verificar que `header.symbols.asm` não foi alterado (todos os símbolos nos mesmos endereços)
- [ ] 8.4 Executar `make` completo para garantir que o build do projeto não quebra
- [ ] 8.5 Executar testes (se houver) relacionados ao runtime/kernel
