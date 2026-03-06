#include "compiler_bload_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerBloadStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_bload();
  return context->compiled;
}
