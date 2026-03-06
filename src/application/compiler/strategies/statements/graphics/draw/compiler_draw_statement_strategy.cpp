#include "compiler_draw_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerDrawStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_draw();
  return context->compiled;
}
