#include "compiler_restore_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerRestoreStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_restore();
  return context->compiled;
}
