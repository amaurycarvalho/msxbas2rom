#include "compiler_dim_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerDimStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_dim();
  return context->compiled;
}
