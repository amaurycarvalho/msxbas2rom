#include "compiler_irestore_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerIrestoreStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_irestore();
  return context->compiled;
}
