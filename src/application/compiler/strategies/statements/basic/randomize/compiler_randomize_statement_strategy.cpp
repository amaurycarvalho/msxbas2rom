#include "compiler_randomize_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerRandomizeStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_randomize();
  return context->compiled;
}
