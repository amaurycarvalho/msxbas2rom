#include "compiler_maxfiles_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerMaxfilesStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_maxfiles();
  return context->compiled;
}
