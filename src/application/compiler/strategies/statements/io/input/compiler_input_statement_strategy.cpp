#include "compiler_input_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerInputStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_input(true);
  return context->compiled;
}
