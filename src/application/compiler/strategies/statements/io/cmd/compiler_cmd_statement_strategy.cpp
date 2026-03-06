#include "compiler_cmd_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerCmdStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_cmd();
  return context->compiled;
}
