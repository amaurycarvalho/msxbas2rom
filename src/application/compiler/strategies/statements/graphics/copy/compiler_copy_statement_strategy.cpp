#include "compiler_copy_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerCopyStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_copy();
  return context->compiled;
}
