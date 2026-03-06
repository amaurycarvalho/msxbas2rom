#include "compiler_ipoke_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerIpokeStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_ipoke();
  return context->compiled;
}
