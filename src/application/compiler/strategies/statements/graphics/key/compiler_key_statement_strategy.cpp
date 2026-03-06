#include "compiler_key_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerKeyStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_key();
  return context->compiled;
}
