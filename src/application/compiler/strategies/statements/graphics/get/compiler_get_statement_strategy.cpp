#include "compiler_get_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerGetStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_get();
  return context->compiled;
}
