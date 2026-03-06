#include "compiler_set_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerSetStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_set();
  return context->compiled;
}
