#include "compiler_let_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerLetStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_let();
  return context->compiled;
}
