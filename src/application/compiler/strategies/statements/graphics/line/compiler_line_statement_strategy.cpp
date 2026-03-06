#include "compiler_line_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerLineStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_line();
  return context->compiled;
}
