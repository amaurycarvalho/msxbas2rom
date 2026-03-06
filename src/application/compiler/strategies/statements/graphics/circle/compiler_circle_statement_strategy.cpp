#include "compiler_circle_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerCircleStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_circle();
  return context->compiled;
}
