#include "compiler_def_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerDefStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_def();
  return context->compiled;
}
