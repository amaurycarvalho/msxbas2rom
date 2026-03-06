#include "compiler_close_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerCloseStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_close();
  return context->compiled;
}
