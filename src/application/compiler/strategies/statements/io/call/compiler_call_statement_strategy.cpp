#include "compiler_call_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerCallStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_call();
  return context->compiled;
}
