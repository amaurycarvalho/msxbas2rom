#include "compiler_pset_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPsetStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_pset(true);
  return context->compiled;
}
