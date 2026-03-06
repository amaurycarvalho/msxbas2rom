#include "compiler_vpoke_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerVpokeStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_vpoke();
  return context->compiled;
}
