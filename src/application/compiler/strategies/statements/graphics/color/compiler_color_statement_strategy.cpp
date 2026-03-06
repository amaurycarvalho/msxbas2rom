#include "compiler_color_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerColorStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_color();
  return context->compiled;
}
