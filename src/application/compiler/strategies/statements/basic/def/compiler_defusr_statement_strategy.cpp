#include "compiler_defusr_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerDefUsrStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_defusr();
  return context->compiled;
}
