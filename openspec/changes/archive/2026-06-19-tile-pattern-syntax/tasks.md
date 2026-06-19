## 1. Z80 Kernel

### 1.1 Implement `get_tile_pattern` routine in `31_cmd.asm`

- Replace the `ret` stub at `get_tile_pattern` with a full implementation
- Input: A=tile, HL=buffer, B=bank (0-2, default 0)
- Calculate VRAM address: `pattern_base + tile×8 + bank×0x0800`
- Screen mode handling: check SCRMOD, return if ≥5, handle mode 0 specially
- Use LDIRMV to read 8 bytes from VRAM to buffer
- Existing label `get_tile_pattern` at `31_cmd.asm:658` — edit in place

- [x] 1.1 Replace `get_tile_pattern` stub with LDIRMV-based implementation

### 1.2 Add `set_tile_pattern_buffer` routine in `31_cmd.asm`

- New routine for bulk buffer write, placed near `set_tile_pattern`
- Input: A=tile, HL=buffer (8 bytes), B=bank (0-2 specific, 3=all)
- Calculate VRAM address same formula as `set_tile_pattern`
- For B=3: loop 3 passes (bank 0, 1, 2) incrementing VRAM address by 0x0800
- Use LDIRVM for each pass
- Handle screen mode 0 (force B=0, use 0x0800 base)

- [x] 1.2 Add `set_tile_pattern_buffer` routine after `set_tile_pattern` in `31_cmd.asm`

### 1.3 Update runtime jump table in `20_runtime.asm`

- Add `jp set_tile_pattern_buffer` entry after `jp get_sprite_pattern`
- The existing `jp set_tile_pattern` and `jp get_tile_pattern` stay unchanged
- Shift subsequent entries accordingly

- [x] 1.3 Add `jp set_tile_pattern_buffer` to `20_runtime.asm`

### 1.4 Update compiler hooks in `compiler_hooks.h`

- Add `#define def_set_tile_pattern_buffer (def_get_sprite_pattern + 3)`
- Update all `def_usr*` defines:
  - `def_usr0` → `def_set_tile_pattern_buffer + 3`
  - `def_usr1` → `def_usr0 + 3`
  - etc.

- [x] 1.4 Add `def_set_tile_pattern_buffer` hook constant and shift USR defines

### 1.5 Update symbol table in `header.symbols.asm`

- Add symbol `set_tile_pattern_buffer EQU <address>`
- The address depends on the code layout — determined during assembly

- [x] 1.5 Add `set_tile_pattern_buffer` symbol

## 2. Compiler (C++)

### 2.1 Update SET TILE PATTERN compiler in `compiler_set_statement_strategy.cpp`

Current dispatch logic at lines 635-736 needs restructuring:

```
Current:
  if (t == 2 && lexeme != "ARRAY") → buffer form (broken call to set_tile_pattern)
  else if (t <= 3)                 → inline array form

New:
  if (lexeme == "ARRAY")           → inline form (existing per-line loop, keep as-is)
  else                             → buffer form:
    - Evaluate tile number → A
    - Evaluate buffer identifier → HL (via fixup)
    - If (t == 3) evaluate bank → B; else B = 3
    - Call def_set_tile_pattern_buffer
```

The condition check should examine `action->actions[1]->lexeme->value == "ARRAY"` regardless of t value.

- [x] 2.1 Restructure SET TILE PATTERN dispatch in compiler

### 2.2 Update GET TILE PATTERN compiler in `compiler_get_statement_strategy.cpp`

Current code at lines 175-207 only handles t==2. Extend to:

```
if (lexeme->value == "PATTERN") {
  if (t == 2 || t == 3) {
    // Evaluate tile number → A
    // Evaluate buffer → HL (via fixup)
    if (t == 3) evaluate bank → B; else B = 0
    // Call def_get_tile_pattern
    //   A = tile, HL = buffer, B = bank
  }
}
```

The `get_tile_pattern` calling convention changes from `A=tile, HL=buffer` to `A=tile, HL=buffer, B=bank`. The existing `def_get_tile_pattern` entry stays; the Z80 routine reads B for bank.

- [x] 2.2 Extend GET TILE PATTERN to handle t==3 and pass bank

## 3. Testing

### 3.1 Unit tests

- Verify parser produces correct ActionNode trees for both inline and array syntaxes
- Verify compiler emits correct Z80 opcodes for each syntax variant

- [x] 3.1 Add unit tests for parser and compiler changes

### 3.2 Integration tests

- Test SET TILE PATTERN inline form across screen modes 0, 1, 2, 3
- Test SET TILE PATTERN buffer form (4-int array) with and without bank
- Test GET TILE PATTERN buffer form (4-int array) with and without bank
- Test backward compatibility — existing inline syntax still works
- Test screen mode validation (mode ≥5 = no-op)

- [x] 3.2 Add integration test BAS files for all syntax variants
- [x] 3.3 Verify all integration tests pass

## 4. Build & Verify

- [x] 4.1 Build project in release mode (`make release`)
- [x] 4.2 Run unit tests (`make test-unit`)
- [x] 4.3 Run integration tests (`make test-integration`)
