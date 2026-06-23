## 1. Core Implementation

- [x] 1.1 Alterar `COMPILE_MAX_PAGES` em `compiler.h` de `(16 * 4)` para `(16 * 8)`
- [x] 1.2 Verificar que `COMPILE_CODE_SIZE` foi automaticamente dobrado para 2MB (2097152 bytes)

## 2. Build and Verification

- [x] 2.1 Compilar o projeto (`make clean && make`)
- [x] 2.2 Executar a suíte de testes unitários (`make test-unit`)
- [x] 2.3 Executar os testes de integração (`make test-integration`)
- [x] 2.4 Verificar que nenhum teste falha com a nova constante

## 3. Validation

- [x] 3.1 Verificar que a mensagem de erro de buffer overflow em `compiler.cpp:362-364` referencia os novos valores corretamente
- [x] 3.2 Validar que o buffer `vector<unsigned char> buf(COMPILE_CODE_SIZE)` em `rom.cpp:166` aloca 2MB
