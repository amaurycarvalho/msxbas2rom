#include "compiler_put_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPutStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_put();
  return context->compiled;
}
