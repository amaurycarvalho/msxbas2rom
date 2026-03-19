#include "compiler_beep_statement_strategy.h"

#include "compiler_context.h"

void CompilerBeepStatementStrategy::cmd_beep(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  // call beep
  cpu.addCall(0x00c0);
}

bool CompilerBeepStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_beep(context);
  return context->compiled;
}
