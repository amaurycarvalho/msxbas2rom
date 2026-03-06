#include "compiler_data_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerDataStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_data();
  return context->compiled;
}
