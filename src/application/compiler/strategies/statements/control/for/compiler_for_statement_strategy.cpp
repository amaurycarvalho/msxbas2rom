#include "compiler_for_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerForStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_for();
  return context->compiled;
}
