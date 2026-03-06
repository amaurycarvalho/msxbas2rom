#include "compiler_width_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerWidthStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_width();
  return context->compiled;
}
