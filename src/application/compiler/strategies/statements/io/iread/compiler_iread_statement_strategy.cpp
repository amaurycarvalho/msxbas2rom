#include "compiler_iread_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerIreadStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_iread();
  return context->compiled;
}
