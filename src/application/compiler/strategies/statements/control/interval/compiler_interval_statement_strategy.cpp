#include "compiler_interval_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerIntervalStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_interval();
  return context->compiled;
}
