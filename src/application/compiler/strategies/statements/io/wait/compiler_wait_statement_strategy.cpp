#include "compiler_wait_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerWaitStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_wait();
  return context->compiled;
}
