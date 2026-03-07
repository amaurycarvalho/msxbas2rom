#include "compiler_randomize_statement_strategy.h"

#include "compiler_context.h"

void CompilerRandomizeStatementStrategy::cmd_randomize(
    CompilerContext* context) {
  auto& cpu = *context->cpu;
  // ld hl, 0x3579      ; RANDOMIZE 1 - FIX
  cpu.addLdHL(0x3579);
  // ld (0xF7BC+0), hl  ; SWPTMP+0
  cpu.addLdiiHL(0xF7BC);

  // ld hl, (0xFC9E)    ; RANDOMIZE 2 - JIFFY
  cpu.addLdHLii(0xFC9E);
  // ld (0xF7BC+2), hl  ; SWPTMP+2
  cpu.addLdiiHL(0xF7BE);
}

bool CompilerRandomizeStatementStrategy::execute(CompilerContext* context) {
  cmd_randomize(context);
  return context->compiled;
}
