#include "compiler_out_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerOutStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_out();
  return context->compiled;
}
