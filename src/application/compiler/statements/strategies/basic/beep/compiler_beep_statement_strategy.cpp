#include "compiler_beep_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"

void CompilerBeepStatementStrategy::cmd_beep(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  // call beep
  cpu.addCall(def_BEEP);
}

bool CompilerBeepStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_beep(context);
  return context->compiled;
}
