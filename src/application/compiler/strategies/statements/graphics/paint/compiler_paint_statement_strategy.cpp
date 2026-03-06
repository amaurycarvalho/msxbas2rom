#include "compiler_paint_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPaintStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_paint();
  return context->compiled;
}
