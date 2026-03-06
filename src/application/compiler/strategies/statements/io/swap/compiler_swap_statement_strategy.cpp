#include "compiler_swap_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerSwapStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_swap();
  return context->compiled;
}
