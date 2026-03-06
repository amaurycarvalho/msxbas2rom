#include "compiler_screen_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerScreenStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_screen();
  return context->compiled;
}
